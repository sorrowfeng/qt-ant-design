#include "AntEmpty.h"

#include <QPainter>
#include <QResizeEvent>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

AntEmpty::AntEmpty(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

QString AntEmpty::description() const { return m_description; }

void AntEmpty::setDescription(const QString& description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    updateGeometry();
    update();
    Q_EMIT descriptionChanged(m_description);
}

bool AntEmpty::imageVisible() const { return m_imageVisible; }

void AntEmpty::setImageVisible(bool visible)
{
    if (m_imageVisible == visible)
    {
        return;
    }
    m_imageVisible = visible;
    updateGeometry();
    update();
    Q_EMIT imageVisibleChanged(m_imageVisible);
}

bool AntEmpty::isSimple() const { return m_simple; }

void AntEmpty::setSimple(bool simple)
{
    if (m_simple == simple)
    {
        return;
    }
    m_simple = simple;
    updateGeometry();
    update();
    Q_EMIT simpleChanged(m_simple);
}

QSize AntEmpty::imageSize() const { return m_imageSize; }

void AntEmpty::setImageSize(const QSize& size)
{
    const QSize clamped(qMax(48, size.width()), qMax(36, size.height()));
    if (m_imageSize == clamped)
    {
        return;
    }
    m_imageSize = clamped;
    updateGeometry();
    update();
    Q_EMIT imageSizeChanged(m_imageSize);
}

QWidget* AntEmpty::extraWidget() const
{
    return m_extraWidget.data();
}

void AntEmpty::setExtraWidget(QWidget* widget)
{
    if (m_extraWidget == widget)
    {
        return;
    }
    if (m_extraWidget)
    {
        m_extraWidget->setParent(nullptr);
    }
    m_extraWidget = widget;
    if (m_extraWidget)
    {
        m_extraWidget->setParent(this);
        m_extraWidget->show();
    }
    syncExtraGeometry();
    updateGeometry();
    update();
}

QSize AntEmpty::sizeHint() const
{
    const auto& token = antTheme->tokens();
    QSize image = effectiveImageSize();
    int height = token.padding;

    if (m_imageVisible)
    {
        height += image.height();
        height += token.marginSM;
    }

    QFont descFont = font();
    descFont.setPixelSize(token.fontSize);
    const int descHeight = QFontMetrics(descFont).boundingRect(QRect(0, 0, 360, 120),
                                                               Qt::AlignCenter | Qt::TextWordWrap,
                                                               m_description)
                               .height();
    height += qMax(token.fontSize, descHeight);

    if (m_extraWidget)
    {
        height += token.margin;
        height += m_extraWidget->sizeHint().height();
    }
    height += token.padding;

    return QSize(qMax(220, image.width() + token.paddingXL), height);
}

QSize AntEmpty::minimumSizeHint() const
{
    return QSize(160, 120);
}

void AntEmpty::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (m_imageVisible)
    {
        const QRect r = imageRect();
        const QColor primary = AntPalette::alpha(token.colorTextTertiary, antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.5 : 0.32);
        const QColor fill = AntPalette::alpha(token.colorFillQuaternary, antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.78 : 1.0);
        const QColor line = AntPalette::alpha(token.colorTextTertiary,
                                              antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.68 : 0.45);

        painter.save();
        painter.translate(r.topLeft());
        painter.scale(r.width() / 128.0, r.height() / 80.0);

        painter.setPen(Qt::NoPen);
        painter.setBrush(AntPalette::alpha(token.colorPrimary, m_simple ? 0.12 : 0.08));
        painter.drawEllipse(QRectF(16, 58, 96, 14));

        painter.setBrush(fill);
        painter.drawRoundedRect(QRectF(34, 10, 60, 46), 10, 10);
        painter.setBrush(AntPalette::alpha(token.colorBgContainer, 0.88));
        painter.drawRoundedRect(QRectF(42, 18, 44, 30), 6, 6);

        painter.setPen(QPen(primary, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawArc(QRectF(10, 20, 26, 26), 35 * 16, 260 * 16);
        painter.drawArc(QRectF(92, 18, 22, 22), 220 * 16, 220 * 16);

        painter.setPen(QPen(line, 2.2, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(49, 27), QPointF(79, 27));
        painter.drawLine(QPointF(49, 34), QPointF(73, 34));
        if (!m_simple)
        {
            painter.drawLine(QPointF(49, 41), QPointF(67, 41));
            painter.setBrush(AntPalette::alpha(token.colorPrimary, 0.16));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(QRectF(20, 12, 8, 8));
            painter.drawEllipse(QRectF(100, 42, 6, 6));
        }
        painter.restore();
    }

    QFont descFont = painter.font();
    descFont.setPixelSize(token.fontSize);
    descFont.setWeight(QFont::Normal);
    painter.setFont(descFont);
    painter.setPen(token.colorTextSecondary);
    painter.drawText(descriptionRect(), Qt::AlignCenter | Qt::TextWordWrap, m_description);
}

void AntEmpty::resizeEvent(QResizeEvent* event)
{
    syncExtraGeometry();
    QWidget::resizeEvent(event);
}

QRect AntEmpty::imageRect() const
{
    const auto& token = antTheme->tokens();
    const QSize image = effectiveImageSize();
    return QRect((width() - image.width()) / 2, token.padding, image.width(), image.height());
}

QRect AntEmpty::descriptionRect() const
{
    const auto& token = antTheme->tokens();
    const int top = m_imageVisible ? imageRect().bottom() + token.marginSM : token.padding;
    const int bottom = m_extraWidget ? extraRect().top() - token.marginSM : height() - token.padding;
    return QRect(token.paddingSM, top, qMax(40, width() - token.paddingSM * 2), qMax(token.fontSize + 4, bottom - top));
}

QRect AntEmpty::extraRect() const
{
    if (!m_extraWidget)
    {
        return {};
    }
    const auto& token = antTheme->tokens();
    const QSize size = m_extraWidget->sizeHint();
    return QRect((width() - size.width()) / 2,
                 height() - token.padding - size.height(),
                 size.width(),
                 size.height());
}

void AntEmpty::syncExtraGeometry()
{
    if (m_extraWidget)
    {
        m_extraWidget->setGeometry(extraRect());
        m_extraWidget->show();
    }
}

QSize AntEmpty::effectiveImageSize() const
{
    if (m_imageSize.isValid() && !m_imageSize.isEmpty())
    {
        return m_imageSize;
    }
    return m_simple ? QSize(96, 56) : QSize(128, 80);
}
