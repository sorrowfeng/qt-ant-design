#include "AntAvatar.h"

#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

#include <algorithm>

#include "core/AntTheme.h"
#include "styles/AntAvatarStyle.h"

AntAvatar::AntAvatar(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntAvatarStyle(style()));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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

// ── AntAvatarGroup ──

AntAvatarGroup::AntAvatarGroup(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

int AntAvatarGroup::maxCount() const { return m_maxCount; }

void AntAvatarGroup::setMaxCount(int maxCount)
{
    maxCount = qMax(0, maxCount);
    if (m_maxCount == maxCount)
        return;
    m_maxCount = maxCount;
    relayout();
    Q_EMIT maxCountChanged(m_maxCount);
}

Ant::AvatarSize AntAvatarGroup::avatarSize() const { return m_avatarSize; }

void AntAvatarGroup::setAvatarSize(Ant::AvatarSize size)
{
    if (m_avatarSize == size)
        return;
    m_avatarSize = size;
    for (auto* av : m_avatars)
        av->setAvatarSize(size);
    relayout();
    Q_EMIT avatarSizeChanged(m_avatarSize);
}

void AntAvatarGroup::addAvatar(AntAvatar* avatar)
{
    if (!avatar || m_avatars.contains(avatar))
        return;
    avatar->setParent(this);
    avatar->setAvatarSize(m_avatarSize);
    m_avatars.append(avatar);
    relayout();
}

void AntAvatarGroup::removeAvatar(AntAvatar* avatar)
{
    if (m_avatars.removeOne(avatar))
    {
        avatar->setParent(nullptr);
        relayout();
    }
}

QList<AntAvatar*> AntAvatarGroup::avatars() const { return m_avatars; }

QSize AntAvatarGroup::sizeHint() const
{
    if (m_avatars.isEmpty())
        return QSize(0, 0);
    const int sz = m_avatars.first()->sizeHint().width();
    const int visible = (m_maxCount > 0 && m_avatars.size() > m_maxCount)
                            ? m_maxCount + 1 // +1 for overflow
                            : m_avatars.size();
    const int overlap = sz * 2 / 5;
    const int totalW = sz + (visible - 1) * (sz - overlap);
    return QSize(totalW, sz);
}

QSize AntAvatarGroup::minimumSizeHint() const
{
    return QSize(24, 24);
}

void AntAvatarGroup::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntAvatarGroup::resizeEvent(QResizeEvent* event)
{
    relayout();
    QWidget::resizeEvent(event);
}

void AntAvatarGroup::relayout()
{
    const int total = m_avatars.size();
    if (total == 0)
        return;

    const int sz = m_avatars.first()->sizeHint().width();
    const int overlap = sz * 2 / 5;
    const int step = sz - overlap;

    int visibleCount = total;
    bool showOverflow = false;
    if (m_maxCount > 0 && total > m_maxCount)
    {
        visibleCount = m_maxCount;
        showOverflow = true;
    }

    // Hide overflow avatars
    for (int i = visibleCount; i < total; ++i)
        m_avatars[i]->hide();

    // Position visible avatars (left to right, later on top)
    for (int i = 0; i < visibleCount; ++i)
    {
        m_avatars[i]->setFixedSize(sz, sz);
        m_avatars[i]->move(i * step, 0);
        m_avatars[i]->show();
        m_avatars[i]->raise();
    }

    updateGeometry();
    update();
}
