#include "AntTag.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "styles/AntTagStyle.h"

AntTag::AntTag(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntTagStyle>(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

AntTag::AntTag(const QString& text, QWidget* parent)
    : AntTag(parent)
{
    m_text = text;
}

QString AntTag::text() const { return m_text; }

void AntTag::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    invalidateSizeHintCache();
    updateGeometry();
    update();
    Q_EMIT textChanged(m_text);
}

QString AntTag::color() const { return m_color; }

void AntTag::setColor(const QString& color)
{
    if (m_color == color)
    {
        return;
    }
    m_color = color;
    invalidateColorCache();
    update();
    Q_EMIT colorChanged(m_color);
}

QString AntTag::iconText() const { return m_iconText; }

void AntTag::setIconText(const QString& iconText)
{
    if (m_iconText == iconText)
    {
        return;
    }
    m_iconText = iconText;
    invalidateSizeHintCache();
    updateGeometry();
    update();
    Q_EMIT iconTextChanged(m_iconText);
}

bool AntTag::isClosable() const { return m_closable; }

void AntTag::setClosable(bool closable)
{
    if (m_closable == closable)
    {
        return;
    }
    m_closable = closable;
    invalidateSizeHintCache();
    updateGeometry();
    update();
    Q_EMIT closableChanged(m_closable);
}

bool AntTag::isCheckable() const { return m_checkable; }

void AntTag::setCheckable(bool checkable)
{
    if (m_checkable == checkable)
    {
        return;
    }
    m_checkable = checkable;
    update();
    Q_EMIT checkableChanged(m_checkable);
}

bool AntTag::isChecked() const { return m_checked; }

void AntTag::setChecked(bool checked)
{
    if (m_checked == checked)
    {
        return;
    }
    m_checked = checked;
    update();
    Q_EMIT checkedChanged(m_checked);
}

Ant::TagVariant AntTag::variant() const { return m_variant; }

void AntTag::setVariant(Ant::TagVariant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    update();
    Q_EMIT variantChanged(m_variant);
}

QSize AntTag::sizeHint() const
{
    if (!m_sizeHintDirty)
    {
        return m_cachedSizeHint;
    }

    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSizeSM);
    const int textWidth = QFontMetrics(f).horizontalAdvance(m_text);
    const int iconWidth = m_iconText.isEmpty() ? 0 : 18;
    const int closeWidth = m_closable ? 18 : 0;
    m_cachedSizeHint = QSize(textWidth + iconWidth + closeWidth + 18, 24);
    m_sizeHintDirty = false;
    return m_cachedSizeHint;
}

QSize AntTag::minimumSizeHint() const
{
    return QSize(32, 22);
}

void AntTag::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::FontChange
        || event->type() == QEvent::ApplicationFontChange
        || event->type() == QEvent::StyleChange)
    {
        invalidateSizeHintCache();
        updateGeometry();
    }
    QWidget::changeEvent(event);
}

void AntTag::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTag::mouseMoveEvent(QMouseEvent* event)
{
    const bool hovered = rect().contains(event->pos());
    const bool closeHovered = m_closable && closeRect().contains(event->pos());
    const bool hoveredChanged = m_hovered != hovered;
    const bool closeChanged = m_closeHovered != closeHovered;
    if (hoveredChanged)
    {
        m_hovered = hovered;
    }
    if (m_closeHovered != closeHovered)
    {
        m_closeHovered = closeHovered;
    }
    if (hoveredChanged)
    {
        update();
    }
    else if (closeChanged)
    {
        update(closeRect().adjusted(-2, -2, 2, 2));
    }
    QWidget::mouseMoveEvent(event);
}

void AntTag::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        update();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntTag::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        const bool inside = rect().contains(event->pos());
        if (inside && m_closable && closeRect().contains(event->pos()))
        {
            Q_EMIT closeRequested();
            hide();
            event->accept();
            return;
        }
        bool stateUpdated = false;
        if (inside)
        {
            if (m_checkable)
            {
                setChecked(!m_checked);
                stateUpdated = true;
            }
            Q_EMIT clicked();
        }
        if (!stateUpdated)
        {
            update();
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntTag::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_closeHovered = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

QRect AntTag::closeRect() const
{
    return QRect(width() - 19, height() / 2 - 8, 16, 16);
}

bool AntTag::isPressedState() const
{
    return m_pressed;
}

bool AntTag::isCloseHoveredState() const
{
    return m_closeHovered;
}

QColor AntTag::baseColor() const
{
    ensureColorCache();
    return m_cachedBaseColor;
}

QColor AntTag::backgroundColor() const
{
    const auto& token = antTheme->tokens();
    if (m_checkable)
    {
        if (m_checked)
        {
            return m_hovered ? token.colorPrimaryHover : token.colorPrimary;
        }
        return m_hovered ? token.colorFillTertiary : Qt::transparent;
    }
    if (!hasCustomColor())
    {
        return token.colorFillQuaternary;
    }
    const QColor base = baseColor();
    if (m_variant == Ant::TagVariant::Solid)
    {
        return base;
    }
    return AntPalette::backgroundColor(base, antTheme->themeMode());
}

QColor AntTag::borderColor() const
{
    if (m_checkable)
    {
        return Qt::transparent;
    }
    if (!hasCustomColor())
    {
        return antTheme->tokens().colorBorder;
    }
    return m_variant == Ant::TagVariant::Solid
        ? baseColor()
        : AntPalette::borderColor(baseColor(), antTheme->themeMode());
}

QColor AntTag::textColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorTextDisabled;
    }
    if (m_checkable && m_checked)
    {
        return token.colorTextLightSolid;
    }
    if (m_checkable)
    {
        return m_hovered ? token.colorPrimary : token.colorText;
    }
    if (hasCustomColor())
    {
        return m_variant == Ant::TagVariant::Solid ? token.colorTextLightSolid : baseColor();
    }
    return token.colorText;
}

bool AntTag::hasCustomColor() const
{
    ensureColorCache();
    return m_cachedHasCustomColor;
}

void AntTag::invalidateSizeHintCache()
{
    m_sizeHintDirty = true;
    m_cachedSizeHint = QSize();
}

void AntTag::invalidateColorCache()
{
    m_colorCacheDirty = true;
    m_cachedBaseColor = QColor();
    m_cachedHasCustomColor = false;
}

void AntTag::ensureColorCache() const
{
    if (!m_colorCacheDirty)
    {
        return;
    }

    const QString color = m_color.trimmed();
    m_cachedHasCustomColor = !color.isEmpty();
    if (color.compare(QStringLiteral("success"), Qt::CaseInsensitive) == 0)
    {
        m_cachedBaseColor = antTheme->tokens().colorSuccess;
    }
    else if (color.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
    {
        m_cachedBaseColor = antTheme->tokens().colorWarning;
    }
    else if (color.compare(QStringLiteral("error"), Qt::CaseInsensitive) == 0)
    {
        m_cachedBaseColor = antTheme->tokens().colorError;
    }
    else if (color.compare(QStringLiteral("processing"), Qt::CaseInsensitive) == 0)
    {
        m_cachedBaseColor = antTheme->tokens().colorPrimary;
    }
    else
    {
        const QColor preset = AntPalette::presetColor(color);
        if (preset.isValid())
        {
            m_cachedBaseColor = preset;
        }
        else
        {
            const QColor parsed(color);
            m_cachedBaseColor = parsed.isValid() ? parsed : antTheme->tokens().colorText;
        }
    }
    m_colorCacheDirty = false;
}
