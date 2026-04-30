#include "AntCascader.h"

#include <QApplication>
#include <QFocusEvent>
#include <QFrame>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScreen>
#include <QScrollBar>

#include <algorithm>

#include "../styles/AntCascaderStyle.h"
#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

#include "CascaderPopup.h"

AntCascader::AntCascader(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_popup = new CascaderPopup(this);

    m_arrowAnimation = new QPropertyAnimation(this, "arrowRotation", this);
    m_arrowAnimation->setDuration(160);
    m_arrowAnimation->setEasingCurve(QEasingCurve::OutCubic);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        updateGeometry();
        update();
    });

    auto* cascaderStyle = new AntCascaderStyle(style());
    cascaderStyle->setParent(this);
    setStyle(cascaderStyle);

    updateCursor();
}

AntCascader::~AntCascader()
{
    delete m_popup;
    m_popup = nullptr;
}

QVector<AntCascaderOption> AntCascader::options() const
{
    return m_options;
}

void AntCascader::setOptions(const QVector<AntCascaderOption>& options)
{
    m_options = options;
    resolveLeafFlags(m_options);
    update();
    emit optionsChanged();
}

QStringList AntCascader::value() const
{
    return m_value;
}

void AntCascader::setValue(const QStringList& path)
{
    if (m_value == path)
    {
        return;
    }

    m_value = path;
    update();
    emit valueChanged(m_value);
}

QString AntCascader::placeholder() const
{
    return m_placeholder;
}

void AntCascader::setPlaceholder(const QString& placeholder)
{
    if (m_placeholder == placeholder)
    {
        return;
    }
    m_placeholder = placeholder;
    update();
    emit placeholderChanged(m_placeholder);
}

bool AntCascader::allowClear() const
{
    return m_allowClear;
}

void AntCascader::setAllowClear(bool allowClear)
{
    if (m_allowClear == allowClear)
    {
        return;
    }
    m_allowClear = allowClear;
    update();
    emit allowClearChanged(m_allowClear);
}

Ant::Size AntCascader::cascaderSize() const
{
    return m_cascaderSize;
}

void AntCascader::setCascaderSize(Ant::Size size)
{
    if (m_cascaderSize == size)
    {
        return;
    }
    m_cascaderSize = size;
    updateGeometry();
    update();
    emit cascaderSizeChanged(m_cascaderSize);
}

Ant::Status AntCascader::status() const
{
    return m_status;
}

void AntCascader::setStatus(Ant::Status status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    update();
    emit statusChanged(m_status);
}

Ant::Variant AntCascader::variant() const
{
    return m_variant;
}

void AntCascader::setVariant(Ant::Variant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    update();
    emit variantChanged(m_variant);
}

Ant::Trigger AntCascader::expandTrigger() const
{
    return m_expandTrigger;
}

void AntCascader::setExpandTrigger(Ant::Trigger trigger)
{
    if (m_expandTrigger == trigger)
    {
        return;
    }
    m_expandTrigger = trigger;
    emit expandTriggerChanged(m_expandTrigger);
}

bool AntCascader::isOpen() const
{
    return m_open;
}

void AntCascader::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }

    m_open = open;
    animateArrow(m_open);

    if (m_open)
    {
        auto* popup = static_cast<CascaderPopup*>(m_popup);
        popup->rebuildColumns();
        popup->updateSizeAndPosition();
        AntPopupMotion::show(m_popup);
    }
    else
    {
        if (m_popup->isVisible())
        {
            AntPopupMotion::hide(m_popup);
        }
    }

    update();
    emit openChanged(m_open);
}

qreal AntCascader::arrowRotation() const
{
    return m_arrowRotation;
}

void AntCascader::setArrowRotation(qreal rotation)
{
    m_arrowRotation = rotation;
    update();
}

QString AntCascader::displayText() const
{
    if (m_value.isEmpty())
    {
        return QString();
    }

    QStringList labels;
    if (buildLabelPath(m_options, m_value, labels, 0))
    {
        return labels.join(QStringLiteral(" / "));
    }

    return m_value.join(QStringLiteral(" / "));
}

bool AntCascader::isHoveredState() const
{
    return m_hovered;
}

bool AntCascader::isPressedState() const
{
    return m_pressed;
}

QSize AntCascader::sizeHint() const
{
    const Metrics m = metrics();
    return QSize(220, m.height);
}

QSize AntCascader::minimumSizeHint() const
{
    const Metrics m = metrics();
    return QSize(120, m.height);
}

void AntCascader::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntCascader::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void AntCascader::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        const Metrics m = metrics();
        if (canClear() && clearButtonRect(m).contains(event->position()))
        {
            setValue(QStringList());
            event->accept();
            return;
        }

        m_pressed = true;
        setFocus(Qt::MouseFocusReason);
        setOpen(!m_open);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntCascader::focusInEvent(QFocusEvent* event)
{
    update();
    QWidget::focusInEvent(event);
}

void AntCascader::focusOutEvent(QFocusEvent* event)
{
    if (!m_popup->isVisible())
    {
        setOpen(false);
    }
    update();
    QWidget::focusOutEvent(event);
}

void AntCascader::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateCursor();
        if (!isEnabled())
        {
            setOpen(false);
        }
        update();
    }
    QWidget::changeEvent(event);
}

AntCascader::Metrics AntCascader::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.height = token.controlHeight;
    m.fontSize = token.fontSize;
    m.radius = token.borderRadius;
    m.paddingX = token.paddingSM - token.lineWidth;
    m.arrowWidth = token.fontSize + token.paddingXS * 2;

    if (m_cascaderSize == Ant::Size::Large)
    {
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
    }
    else if (m_cascaderSize == Ant::Size::Small)
    {
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        m.radius = token.borderRadiusSM;
        m.paddingX = token.paddingXS;
    }
    return m;
}

QRectF AntCascader::controlRect() const
{
    const Metrics m = metrics();
    return QRectF(1, (height() - m.height) / 2.0, width() - 2, m.height);
}

QRectF AntCascader::clearButtonRect(const Metrics& m) const
{
    const QRectF control = controlRect();
    const qreal size = std::min<qreal>(18, control.height() - 8);
    return QRectF(control.right() - m.paddingX - size,
                  control.center().y() - size / 2.0,
                  size,
                  size);
}

QColor AntCascader::borderColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (m_status == Ant::Status::Error)
    {
        return (m_hovered || hasFocus() || m_open) ? token.colorErrorHover : token.colorError;
    }
    if (m_status == Ant::Status::Warning)
    {
        return (m_hovered || hasFocus() || m_open) ? token.colorWarningHover : token.colorWarning;
    }
    if (m_hovered || hasFocus() || m_open)
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor AntCascader::backgroundColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (m_variant == Ant::Variant::Filled)
    {
        return m_hovered ? token.colorFillTertiary : token.colorFillQuaternary;
    }
    if (m_variant == Ant::Variant::Borderless || m_variant == Ant::Variant::Underlined)
    {
        return QColor(0, 0, 0, 0);
    }
    return token.colorBgContainer;
}

bool AntCascader::canClear() const
{
    return isEnabled() && m_allowClear && m_hovered && !m_value.isEmpty();
}

void AntCascader::updateCursor()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void AntCascader::animateArrow(bool open)
{
    m_arrowAnimation->stop();
    m_arrowAnimation->setStartValue(m_arrowRotation);
    m_arrowAnimation->setEndValue(open ? 180.0 : 0.0);
    m_arrowAnimation->start();
}

void AntCascader::updatePopupPosition()
{
    if (m_popup && m_popup->isVisible())
    {
        auto* popup = static_cast<CascaderPopup*>(m_popup);
        popup->rebuildColumns();
        popup->updateSizeAndPosition();
    }
}

void AntCascader::resolveLeafFlags(QVector<AntCascaderOption>& options)
{
    for (auto& opt : options)
    {
        if (opt.children.isEmpty())
        {
            opt.isLeaf = true;
        }
        else
        {
            opt.isLeaf = false;
            resolveLeafFlags(opt.children);
        }
    }
}

const AntCascaderOption* AntCascader::findOptionByValue(const QVector<AntCascaderOption>& options, const QVariant& value)
{
    for (const auto& opt : options)
    {
        if (opt.value == value)
        {
            return &opt;
        }
    }
    return nullptr;
}

bool AntCascader::buildLabelPath(const QVector<AntCascaderOption>& options, const QStringList& valuePath, QStringList& labelPath, int depth)
{
    if (depth >= valuePath.size())
    {
        return true;
    }

    const QString& val = valuePath.at(depth);
    for (const auto& opt : options)
    {
        if (opt.value.toString() == val)
        {
            labelPath.push_back(opt.label);
            if (depth + 1 == valuePath.size())
            {
                return true;
            }
            return buildLabelPath(opt.children, valuePath, labelPath, depth + 1);
        }
    }
    return false;
}

bool AntCascader::buildValuePathFromLabels(const QVector<AntCascaderOption>& options, const QStringList& labelPath, QStringList& valuePath, int depth)
{
    if (depth >= labelPath.size())
    {
        return true;
    }

    const QString& lbl = labelPath.at(depth);
    for (const auto& opt : options)
    {
        if (opt.label == lbl)
        {
            valuePath.push_back(opt.value.toString());
            if (depth + 1 == labelPath.size())
            {
                return true;
            }
            return buildValuePathFromLabels(opt.children, labelPath, valuePath, depth + 1);
        }
    }
    return false;
}
