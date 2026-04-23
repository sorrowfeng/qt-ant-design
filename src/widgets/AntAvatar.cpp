#include "AntAvatar.h"

#include <QPainter>
#include <QPainterPath>

#include <algorithm>

#include "core/AntTheme.h"

AntAvatar::AntAvatar(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

AntAvatar::AntAvatar(const QString& text, QWidget* parent)
    : AntAvatar(parent)
{
    m_text = text;
}

QString AntAvatar::text() const { return m_text; }

void AntAvatar::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    update();
    Q_EMIT textChanged(m_text);
}

QString AntAvatar::iconText() const { return m_iconText; }

void AntAvatar::setIconText(const QString& iconText)
{
    if (m_iconText == iconText)
    {
        return;
    }
    m_iconText = iconText;
    update();
    Q_EMIT iconTextChanged(m_iconText);
}

QString AntAvatar::imagePath() const { return m_imagePath; }

void AntAvatar::setImagePath(const QString& imagePath)
{
    if (m_imagePath == imagePath)
    {
        return;
    }
    m_imagePath = imagePath;
    m_pixmap = QPixmap(m_imagePath);
    update();
    Q_EMIT imagePathChanged(m_imagePath);
}

int AntAvatar::gap() const { return m_gap; }

void AntAvatar::setGap(int gap)
{
    gap = std::max(0, gap);
    if (m_gap == gap)
    {
        return;
    }
    m_gap = gap;
    update();
    Q_EMIT gapChanged(m_gap);
}

int AntAvatar::customSize() const { return m_customSize; }

void AntAvatar::setCustomSize(int size)
{
    size = std::max(0, size);
    if (m_customSize == size)
    {
        return;
    }
    m_customSize = size;
    updateGeometry();
    update();
    Q_EMIT customSizeChanged(m_customSize);
}

Ant::AvatarShape AntAvatar::shape() const { return m_shape; }

void AntAvatar::setShape(Ant::AvatarShape shape)
{
    if (m_shape == shape)
    {
        return;
    }
    m_shape = shape;
    update();
    Q_EMIT shapeChanged(m_shape);
}

Ant::AvatarSize AntAvatar::avatarSize() const { return m_avatarSize; }

void AntAvatar::setAvatarSize(Ant::AvatarSize size)
{
    if (m_avatarSize == size)
    {
        return;
    }
    m_avatarSize = size;
    updateGeometry();
    update();
    Q_EMIT avatarSizeChanged(m_avatarSize);
}

QSize AntAvatar::sizeHint() const
{
    const int extent = avatarExtent();
    return QSize(extent, extent);
}

QSize AntAvatar::minimumSizeHint() const
{
    return QSize(antTheme->tokens().controlHeightSM, antTheme->tokens().controlHeightSM);
}

void AntAvatar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    const auto& token = antTheme->tokens();
    const QRectF avatarRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const QPainterPath path = clipPath(avatarRect);
    if (!m_pixmap.isNull())
    {
        painter.save();
        painter.setClipPath(path);
        painter.drawPixmap(avatarRect, m_pixmap, imageSourceRect(m_pixmap, avatarRect.size()));
        painter.restore();
        return;
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(isEnabled() ? token.colorTextPlaceholder : token.colorBgContainerDisabled);
    painter.drawPath(path);

    const QString content = !m_iconText.isEmpty() ? m_iconText : m_text;
    if (content.isEmpty())
    {
        return;
    }

    QFont f = painter.font();
    f.setPixelSize(!m_iconText.isEmpty() ? iconFontSize() : textFontSize());
    f.setWeight(QFont::DemiBold);
    if (m_iconText.isEmpty())
    {
        const int available = std::max(1, width() - m_gap * 2);
        const int textWidth = QFontMetrics(f).horizontalAdvance(content);
        if (textWidth > available)
        {
            const qreal scale = static_cast<qreal>(available) / textWidth;
            f.setPixelSize(std::max(8, static_cast<int>(f.pixelSize() * scale)));
        }
    }

    painter.setFont(f);
    painter.setPen(isEnabled() ? token.colorTextLightSolid : token.colorTextDisabled);
    painter.drawText(avatarRect, Qt::AlignCenter, content.left(4));
}

int AntAvatar::avatarExtent() const
{
    if (m_customSize > 0)
    {
        return m_customSize;
    }
    const auto& token = antTheme->tokens();
    switch (m_avatarSize)
    {
    case Ant::AvatarSize::Large:
        return token.controlHeightLG;
    case Ant::AvatarSize::Small:
        return token.controlHeightSM;
    case Ant::AvatarSize::Middle:
        return token.controlHeight;
    }
    return token.controlHeight;
}

int AntAvatar::textFontSize() const
{
    const auto& token = antTheme->tokens();
    if (m_avatarSize == Ant::AvatarSize::Small)
    {
        return token.fontSizeSM;
    }
    return token.fontSize;
}

int AntAvatar::iconFontSize() const
{
    const auto& token = antTheme->tokens();
    if (m_avatarSize == Ant::AvatarSize::Large || m_customSize >= token.controlHeightLG)
    {
        return token.fontSizeXL;
    }
    if (m_avatarSize == Ant::AvatarSize::Small && m_customSize == 0)
    {
        return token.fontSize;
    }
    return (token.fontSizeLG + token.fontSizeXL) / 2;
}

QPainterPath AntAvatar::clipPath(const QRectF& rect) const
{
    QPainterPath path;
    if (m_shape == Ant::AvatarShape::Circle)
    {
        path.addEllipse(rect);
    }
    else
    {
        path.addRoundedRect(rect, antTheme->tokens().borderRadius, antTheme->tokens().borderRadius);
    }
    return path;
}

QRectF AntAvatar::imageSourceRect(const QPixmap& pixmap, const QSizeF& targetSize) const
{
    const QSizeF source = pixmap.size();
    const qreal sourceRatio = source.width() / source.height();
    const qreal targetRatio = targetSize.width() / targetSize.height();
    if (sourceRatio > targetRatio)
    {
        const qreal width = source.height() * targetRatio;
        return QRectF((source.width() - width) / 2.0, 0, width, source.height());
    }
    const qreal height = source.width() / targetRatio;
    return QRectF(0, (source.height() - height) / 2.0, source.width(), height);
}
