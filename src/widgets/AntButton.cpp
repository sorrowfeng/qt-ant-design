#include "AntButton.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

namespace
{
struct ButtonColors
{
    QColor background;
    QColor border;
    QColor text;
    Qt::PenStyle borderStyle = Qt::SolidLine;
};
}

AntButton::AntButton(QWidget* parent)
    : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover, true);
    setFlat(true);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometryFromState();
        update();
    });
    connect(&m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle + 30) % 360;
        update();
    });

    updateGeometryFromState();
}

AntButton::AntButton(const QString& text, QWidget* parent)
    : AntButton(parent)
{
    setText(text);
}

Ant::ButtonType AntButton::buttonType() const { return m_buttonType; }

void AntButton::setButtonType(Ant::ButtonType type)
{
    if (m_buttonType == type)
        return;
    m_buttonType = type;
    updateGeometryFromState();
    update();
    Q_EMIT buttonTypeChanged(m_buttonType);
}

Ant::ButtonSize AntButton::buttonSize() const { return m_buttonSize; }

void AntButton::setButtonSize(Ant::ButtonSize size)
{
    if (m_buttonSize == size)
        return;
    m_buttonSize = size;
    updateGeometryFromState();
    update();
    Q_EMIT buttonSizeChanged(m_buttonSize);
}

Ant::ButtonShape AntButton::buttonShape() const { return m_buttonShape; }

void AntButton::setButtonShape(Ant::ButtonShape shape)
{
    if (m_buttonShape == shape)
        return;
    m_buttonShape = shape;
    updateGeometryFromState();
    update();
    Q_EMIT buttonShapeChanged(m_buttonShape);
}

bool AntButton::isLoading() const { return m_loading; }

void AntButton::setLoading(bool loading)
{
    if (m_loading == loading)
        return;
    m_loading = loading;
    m_loading ? m_spinnerTimer.start(80) : m_spinnerTimer.stop();
    updateGeometryFromState();
    update();
    Q_EMIT loadingChanged(m_loading);
}

bool AntButton::isDanger() const { return m_danger; }

void AntButton::setDanger(bool danger)
{
    if (m_danger == danger)
        return;
    m_danger = danger;
    update();
    Q_EMIT dangerChanged(m_danger);
}

bool AntButton::isGhost() const { return m_ghost; }

void AntButton::setGhost(bool ghost)
{
    if (m_ghost == ghost)
        return;
    m_ghost = ghost;
    update();
    Q_EMIT ghostChanged(m_ghost);
}

bool AntButton::isBlock() const { return m_block; }

void AntButton::setBlock(bool block)
{
    if (m_block == block)
        return;
    m_block = block;
    setSizePolicy(block ? QSizePolicy::Expanding : QSizePolicy::Preferred, QSizePolicy::Fixed);
    updateGeometry();
    Q_EMIT blockChanged(m_block);
}

QSize AntButton::sizeHint() const
{
    const Metrics m = metrics();
    QFont f = font();
    f.setPixelSize(m.fontSize);
    QFontMetrics fm(f);
    int width = fm.horizontalAdvance(text()) + m.paddingX * 2;
    if (m_loading)
        width += m.iconSize + 8;
    if (m_buttonShape == Ant::ButtonShape::Circle)
        width = m.height;
    return QSize(std::max(width, m.height), m.height);
}

QSize AntButton::minimumSizeHint() const
{
    return sizeHint();
}

void AntButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    const QRectF outer = rect().adjusted(0, 0, -1, -1);
    const int radius = cornerRadius(m);
    const bool active = m_pressed || isDown();
    const bool plainType = m_buttonType == Ant::ButtonType::Text || m_buttonType == Ant::ButtonType::Link;
    const QColor accent = semanticColor();

    ButtonColors colors;
    colors.background = plainType || m_ghost ? QColor(Qt::transparent) : token.colorBgContainer;
    colors.border = plainType ? QColor(Qt::transparent) : token.colorBorder;
    colors.text = token.colorText;

    if (m_buttonType == Ant::ButtonType::Primary)
    {
        colors.background = accent;
        colors.border = accent;
        colors.text = token.colorTextLightSolid;
        if (m_hovered)
            colors.background = colors.border = antTheme->hoverColor(accent);
        if (active)
            colors.background = colors.border = antTheme->activeColor(accent);
    }
    else if (m_buttonType == Ant::ButtonType::Default || m_buttonType == Ant::ButtonType::Dashed)
    {
        colors.borderStyle = m_buttonType == Ant::ButtonType::Dashed ? Qt::DashLine : Qt::SolidLine;
        if (m_danger)
            colors.text = colors.border = token.colorError;
        if (m_hovered)
            colors.text = colors.border = m_danger ? token.colorErrorHover : token.colorPrimaryHover;
        if (active)
            colors.text = colors.border = m_danger ? token.colorErrorActive : token.colorPrimaryActive;
    }
    else if (m_buttonType == Ant::ButtonType::Text)
    {
        colors.text = m_danger ? token.colorError : token.colorText;
        if (m_hovered)
            colors.background = token.colorFillTertiary;
        if (active)
            colors.background = token.colorFillQuaternary;
    }
    else if (m_buttonType == Ant::ButtonType::Link)
    {
        colors.text = m_danger ? token.colorError : token.colorLink;
        if (m_hovered)
            colors.text = m_danger ? token.colorErrorHover : token.colorLinkHover;
        if (active)
            colors.text = m_danger ? token.colorErrorActive : token.colorLinkActive;
    }

    if (m_ghost && !plainType)
    {
        colors.background = QColor(Qt::transparent);
        if (m_buttonType == Ant::ButtonType::Primary)
            colors.text = colors.border = accent;
    }

    if (!isEnabled())
    {
        colors.background = plainType || m_ghost ? QColor(Qt::transparent) : token.colorBgContainerDisabled;
        colors.border = plainType ? QColor(Qt::transparent) : token.colorBorderDisabled;
        colors.text = token.colorTextDisabled;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (!plainType && isEnabled() && !m_ghost && !active)
        antTheme->drawEffectShadow(&painter, rect(), 4, radius, m_buttonType == Ant::ButtonType::Primary ? 0.45 : 0.25);

    painter.setBrush(colors.background);
    painter.setPen(colors.border.alpha() == 0 ? Qt::NoPen : QPen(colors.border, token.lineWidth, colors.borderStyle));
    painter.drawRoundedRect(outer, radius, radius);

    if (hasFocus() && isEnabled())
    {
        QColor focus = AntPalette::alpha(m_danger ? token.colorError : token.colorPrimary, 0.18);
        painter.setPen(QPen(focus, token.controlOutlineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(outer.adjusted(2, 2, -2, -2), radius, radius);
    }

    QRectF textRect = contentRect(m);
    painter.setFont(font());
    painter.setPen(colors.text);
    if (m_loading)
    {
        QRectF spinnerRect(textRect.left(), textRect.center().y() - m.iconSize / 2.0, m.iconSize, m.iconSize);
        drawSpinner(painter, spinnerRect, colors.text);
        textRect.adjust(m.iconSize + 8, 0, 0, 0);
    }
    painter.drawText(textRect, Qt::AlignCenter, text());
}

void AntButton::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QPushButton::enterEvent(event);
}

void AntButton::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QPushButton::leaveEvent(event);
}

void AntButton::mousePressEvent(QMouseEvent* event)
{
    m_pressed = true;
    update();
    QPushButton::mousePressEvent(event);
}

void AntButton::mouseReleaseEvent(QMouseEvent* event)
{
    m_pressed = false;
    update();
    QPushButton::mouseReleaseEvent(event);
}

void AntButton::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange)
    {
        updateGeometryFromState();
        update();
    }
    QPushButton::changeEvent(event);
}

AntButton::Metrics AntButton::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    switch (m_buttonSize)
    {
    case Ant::ButtonSize::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.paddingX = token.padding;
        m.radius = token.borderRadiusLG;
        m.iconSize = 16;
        break;
    case Ant::ButtonSize::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS;
        m.radius = token.borderRadiusSM;
        m.iconSize = 12;
        break;
    case Ant::ButtonSize::Middle:
        m.height = token.controlHeight;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingSM + token.lineWidth * 3;
        m.radius = token.borderRadius;
        m.iconSize = 14;
        break;
    }
    return m;
}

int AntButton::cornerRadius(const Metrics& metrics) const
{
    if (m_buttonShape == Ant::ButtonShape::Circle || m_buttonShape == Ant::ButtonShape::Round)
        return metrics.height / 2;
    return metrics.radius;
}

QRectF AntButton::contentRect(const Metrics& metrics) const
{
    if (m_buttonShape == Ant::ButtonShape::Circle)
        return rect();
    return rect().adjusted(metrics.paddingX, 0, -metrics.paddingX, 0);
}

QColor AntButton::semanticColor() const
{
    return m_danger ? antTheme->tokens().colorError : antTheme->tokens().colorPrimary;
}

void AntButton::updateGeometryFromState()
{
    const Metrics m = metrics();
    QFont f = font();
    f.setPixelSize(m.fontSize);
    setFont(f);
    setMinimumHeight(m.height);
    setMaximumHeight(m.height);
    setSizePolicy(m_block ? QSizePolicy::Expanding : QSizePolicy::Preferred, QSizePolicy::Fixed);
    updateGeometry();
}

void AntButton::drawSpinner(QPainter& painter, const QRectF& rect, const QColor& color) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(color, 2, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, m_spinnerAngle * 16, 270 * 16);
    painter.restore();
}
