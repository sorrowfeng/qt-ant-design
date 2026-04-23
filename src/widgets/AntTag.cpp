#include "AntTag.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

AntTag::AntTag(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
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
    const auto& token = antTheme->tokens();
    QFont f = font();
    f.setPixelSize(token.fontSizeSM);
    const int textWidth = QFontMetrics(f).horizontalAdvance(m_text);
    const int iconWidth = m_iconText.isEmpty() ? 0 : 18;
    const int closeWidth = m_closable ? 18 : 0;
    return QSize(textWidth + iconWidth + closeWidth + 18, 24);
}

QSize AntTag::minimumSizeHint() const
{
    return QSize(32, 22);
}

void AntTag::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRectF pill = rect().adjusted(0.5, 1.5, -0.5, -1.5);
    painter.setPen(QPen(borderColor(), token.lineWidth));
    painter.setBrush(backgroundColor());
    painter.drawRoundedRect(pill, token.borderRadiusSM, token.borderRadiusSM);

    QFont f = painter.font();
    f.setPixelSize(token.fontSizeSM);
    painter.setFont(f);
    painter.setPen(textColor());

    int x = token.paddingXS;
    if (!m_iconText.isEmpty())
    {
        painter.drawText(QRect(x - 2, 0, 16, height()), Qt::AlignCenter, m_iconText.left(2));
        x += 18;
    }
    const int rightReserve = m_closable ? 18 : token.paddingXXS;
    painter.drawText(QRect(x, 0, width() - x - rightReserve, height()), Qt::AlignLeft | Qt::AlignVCenter, m_text);

    if (m_closable)
    {
        const QRect close = closeRect();
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_closeHovered ? token.colorFillTertiary : Qt::transparent);
        painter.drawRoundedRect(close, token.borderRadiusXS, token.borderRadiusXS);
        painter.setPen(QPen(textColor(), 1.3, Qt::SolidLine, Qt::RoundCap));
        const QPoint c = close.center();
        painter.drawLine(QPoint(c.x() - 3, c.y() - 3), QPoint(c.x() + 3, c.y() + 3));
        painter.drawLine(QPoint(c.x() + 3, c.y() - 3), QPoint(c.x() - 3, c.y() + 3));
    }
}

void AntTag::mouseMoveEvent(QMouseEvent* event)
{
    m_hovered = rect().contains(event->pos());
    const bool closeHovered = m_closable && closeRect().contains(event->pos());
    if (m_closeHovered != closeHovered)
    {
        m_closeHovered = closeHovered;
    }
    update();
    QWidget::mouseMoveEvent(event);
}

void AntTag::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_closable && closeRect().contains(event->pos()))
        {
            Q_EMIT closeRequested();
            hide();
            event->accept();
            return;
        }
        if (m_checkable)
        {
            setChecked(!m_checked);
        }
        Q_EMIT clicked();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntTag::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_closeHovered = false;
    update();
    QWidget::leaveEvent(event);
}

QRect AntTag::closeRect() const
{
    return QRect(width() - 19, height() / 2 - 8, 16, 16);
}

QColor AntTag::baseColor() const
{
    if (m_color.compare(QStringLiteral("success"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorSuccess;
    }
    if (m_color.compare(QStringLiteral("warning"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorWarning;
    }
    if (m_color.compare(QStringLiteral("error"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorError;
    }
    if (m_color.compare(QStringLiteral("processing"), Qt::CaseInsensitive) == 0 || m_color.compare(QStringLiteral("blue"), Qt::CaseInsensitive) == 0)
    {
        return antTheme->tokens().colorPrimary;
    }
    const QColor parsed(m_color);
    return parsed.isValid() ? parsed : antTheme->tokens().colorText;
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
    return AntPalette::tint(base, 0.88);
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
    if (m_variant == Ant::TagVariant::Filled)
    {
        return Qt::transparent;
    }
    return m_variant == Ant::TagVariant::Solid ? baseColor() : AntPalette::tint(baseColor(), 0.65);
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
    return !m_color.trimmed().isEmpty();
}
