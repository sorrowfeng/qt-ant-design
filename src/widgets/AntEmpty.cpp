#include "AntEmpty.h"

#include <QPainter>
#include <QResizeEvent>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "styles/AntEmptyStyle.h"

AntEmpty::AntEmpty(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntEmptyStyle(style()));
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
