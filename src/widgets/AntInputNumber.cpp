#include "AntInputNumber.h"

#include <QEasingCurve>
#include <QEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QRegion>
#include <QSizePolicy>
#include <QStyleOptionSpinBox>

#include "../styles/AntInputNumberStyle.h"
#include "core/AntTheme.h"
#include "styles/AntIconPainter.h"

namespace
{
int handlerWidthForInput(const AntInputNumber* input)
{
    const auto& token = antTheme->tokens();
    int handlerWidth = token.fontSize + token.paddingSM;
    if (!input)
    {
        return handlerWidth;
    }

    switch (input->inputSize())
    {
    case Ant::Size::Large:
        handlerWidth = token.fontSizeLG + token.padding;
        break;
    case Ant::Size::Small:
        handlerWidth = token.fontSize + token.paddingXS + token.paddingXXS;
        break;
    case Ant::Size::Middle:
        break;
    }
    return handlerWidth;
}

class InputNumberControlsOverlay : public QWidget
{
public:
    explicit InputNumberControlsOverlay(AntInputNumber* input)
        : QWidget(input)
        , m_input(input)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        if (!m_input || !m_input->controlsVisible() || !m_input->isEnabled() || m_input->controlsProgress() <= 0.01)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        const qreal progress = m_input->controlsProgress();
        const int panelWidth = handlerWidthForInput(m_input);
        const QRect panelRect(width() - panelWidth, 0, panelWidth, height());
        const QRect upRect(panelRect.left(), 1, panelRect.width(), height() / 2 - 1);
        const QRect downRect(panelRect.left(), height() / 2, panelRect.width(), height() / 2 - 1);
        const bool upHovered = m_input->activeSubControl() == QStyle::SC_SpinBoxUp;
        const bool downHovered = m_input->activeSubControl() == QStyle::SC_SpinBoxDown;

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.setOpacity(progress);
        painter.setClipRect(rect());
        painter.translate((1.0 - progress) * panelWidth, 0);

        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        painter.drawLine(QPointF(panelRect.left() + 0.5, 3),
                         QPointF(panelRect.left() + 0.5, height() - 3));
        painter.drawLine(QPointF(panelRect.left() + 2, height() / 2.0),
                         QPointF(panelRect.right() - 2, height() / 2.0));

        const QColor hoverColor = m_input->isStepPressed() ? token.colorFillQuaternary : token.colorFillTertiary;
        painter.setPen(Qt::NoPen);
        if (upHovered)
        {
            painter.fillRect(upRect.adjusted(1, 0, -1, 0), hoverColor);
        }
        if (downHovered)
        {
            painter.fillRect(downRect.adjusted(1, 0, -1, 0), hoverColor);
        }

        const QColor iconColor = token.colorTextTertiary;
        const int iconInsetX = qMin(6, qMax(2, panelWidth / 4));
        AntIconPainter::drawIcon(painter, Ant::IconType::Up, upRect.adjusted(iconInsetX, 2, -iconInsetX, -2), iconColor);
        AntIconPainter::drawIcon(painter, Ant::IconType::Down, downRect.adjusted(iconInsetX, 2, -iconInsetX, -2), iconColor);
    }

private:
    AntInputNumber* m_input = nullptr;
};
}

AntInputNumber::AntInputNumber(QWidget* parent)
    : QDoubleSpinBox(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setFrame(false);
    setAccelerated(true);
    setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
    setDecimals(0);
    setRange(-999999, 999999);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    auto* inputStyle = new AntInputNumberStyle(style());
    inputStyle->setParent(this);
    setStyle(inputStyle);

    m_controlsAnimation = new QPropertyAnimation(this, "controlsProgress", this);
    m_controlsAnimation->setDuration(160);
    m_controlsAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_controlsOverlay = new InputNumberControlsOverlay(this);
    m_controlsOverlay->hide();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        const Metrics oldMetrics = metrics();
        invalidateMetricsCache();
        const Metrics newMetrics = metrics();
        updateEditStyle();
        if (oldMetrics.height != newMetrics.height || oldMetrics.fontSize != newMetrics.fontSize ||
            oldMetrics.paddingX != newMetrics.paddingX || oldMetrics.radius != newMetrics.radius)
        {
            updateGeometry();
        }
        invalidateGeometryCache();
        updateControlsOverlayGeometry();
        update();
    });

    updateEditStyle();
    if (lineEdit())
    {
        lineEdit()->installEventFilter(this);
        lineEdit()->setMouseTracking(true);
    }
    syncInputNumberPerfCounters();
}

Ant::Size AntInputNumber::inputSize() const
{
    return m_inputSize;
}

void AntInputNumber::setInputSize(Ant::Size size)
{
    if (m_inputSize == size)
    {
        return;
    }
    m_inputSize = size;
    invalidateMetricsCache();
    updateEditStyle();
    invalidateGeometryCache();
    updateControlsOverlayGeometry();
    updateGeometry();
    update();
    Q_EMIT inputSizeChanged(m_inputSize);
}

Ant::Status AntInputNumber::status() const
{
    return m_status;
}

void AntInputNumber::setStatus(Ant::Status status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    updateFrameChrome();
    Q_EMIT statusChanged(m_status);
}

Ant::Variant AntInputNumber::variant() const
{
    return m_variant;
}

void AntInputNumber::setVariant(Ant::Variant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    updateEditStyle();
    update();
    Q_EMIT variantChanged(m_variant);
}

bool AntInputNumber::controlsVisible() const
{
    return m_controlsVisible;
}

void AntInputNumber::setControlsVisible(bool visible)
{
    if (m_controlsVisible == visible)
    {
        return;
    }
    m_controlsVisible = visible;
    setButtonSymbols(visible ? QAbstractSpinBox::UpDownArrows : QAbstractSpinBox::NoButtons);
    invalidateGeometryCache();
    animateControls(shouldShowControls());
    updateLineEditControlsInset();
    updateControlsOverlayGeometry();
    updateGeometry();
    updateControlsRegion();
    Q_EMIT controlsVisibleChanged(m_controlsVisible);
}

qreal AntInputNumber::controlsProgress() const
{
    return m_controlsProgress;
}

void AntInputNumber::setControlsProgress(qreal progress)
{
    progress = qBound<qreal>(0.0, progress, 1.0);
    if (qFuzzyCompare(m_controlsProgress, progress))
    {
        return;
    }
    const QRect oldDirty = controlsDirtyRect();
    m_controlsProgress = progress;
    invalidateGeometryCache();
    updateLineEditControlsInset();
    updateControlsOverlayGeometry();
    if (m_controlsOverlay)
    {
        m_controlsOverlay->update();
    }
    updateControlsRegion(oldDirty);
}

QString AntInputNumber::placeholderText() const
{
    return lineEdit() ? lineEdit()->placeholderText() : QString();
}

void AntInputNumber::setPlaceholderText(const QString& text)
{
    if (lineEdit())
    {
        lineEdit()->setPlaceholderText(text);
    }
}

QString AntInputNumber::prefixText() const
{
    return prefix();
}

void AntInputNumber::setPrefixText(const QString& text)
{
    if (prefix() == text)
    {
        return;
    }
    setPrefix(text);
    updateEditStyle();
    updateGeometry();
    update();
    Q_EMIT prefixTextChanged(prefix());
}

QString AntInputNumber::suffixText() const
{
    return suffix();
}

void AntInputNumber::setSuffixText(const QString& text)
{
    if (suffix() == text)
    {
        return;
    }
    setSuffix(text);
    updateEditStyle();
    updateGeometry();
    update();
    Q_EMIT suffixTextChanged(suffix());
}

QString AntInputNumber::addonAfterText() const
{
    return m_addonAfterText;
}

void AntInputNumber::setAddonAfterText(const QString& text)
{
    if (m_addonAfterText == text)
    {
        return;
    }
    m_addonAfterText = text;
    invalidateGeometryCache();
    updateEditStyle();
    updateGeometry();
    update();
}

int AntInputNumber::precision() const
{
    return QDoubleSpinBox::decimals();
}

void AntInputNumber::setPrecision(int decimals)
{
    setDecimals(decimals);
}

void AntInputNumber::setDecimals(int decimals)
{
    decimals = qMax(0, decimals);
    if (QDoubleSpinBox::decimals() == decimals)
    {
        return;
    }
    QDoubleSpinBox::setDecimals(decimals);
    updateEditStyle();
    updateGeometry();
    update();
    Q_EMIT precisionChanged(QDoubleSpinBox::decimals());
}

QSize AntInputNumber::sizeHint() const
{
    if (m_sizeHintDirty || !m_cachedSizeHint.isValid())
    {
        const Metrics m = metrics();
        m_cachedSizeHint = QSize(140, m.height);
        m_cachedMinimumSizeHint = QSize(96, m.height);
        m_sizeHintDirty = false;
        ++m_sizeHintResolveCount;
        syncInputNumberPerfCounters();
    }
    return m_cachedSizeHint;
}

QSize AntInputNumber::minimumSizeHint() const
{
    if (m_sizeHintDirty || !m_cachedMinimumSizeHint.isValid())
    {
        const Metrics m = metrics();
        m_cachedSizeHint = QSize(140, m.height);
        m_cachedMinimumSizeHint = QSize(96, m.height);
        m_sizeHintDirty = false;
        ++m_sizeHintResolveCount;
        syncInputNumberPerfCounters();
    }
    return m_cachedMinimumSizeHint;
}

bool AntInputNumber::isHoveredState() const
{
    return m_hovered;
}

QStyle::SubControl AntInputNumber::activeSubControl() const
{
    return m_activeSubControl;
}

bool AntInputNumber::isStepPressed() const
{
    return m_stepPressed;
}

QAbstractSpinBox::StepEnabled AntInputNumber::stepEnabledFlags() const
{
    return stepEnabled();
}

void AntInputNumber::enterEvent(AntEnterEvent* event)
{
    if (m_hovered)
    {
        QDoubleSpinBox::enterEvent(event);
        return;
    }
    m_hovered = true;
    animateControls(true);
    updateFrameChrome();
    QDoubleSpinBox::enterEvent(event);
}

void AntInputNumber::leaveEvent(QEvent* event)
{
    const QRect oldControls = controlsDirtyRect();
    m_hovered = false;
    const bool controlsChanged = m_activeSubControl != QStyle::SC_None || m_stepPressed;
    if (controlsChanged)
    {
        m_activeSubControl = QStyle::SC_None;
        m_stepPressed = false;
    }
    animateControls(shouldShowControls());
    updateFrameChrome();
    if (controlsChanged)
    {
        updateControlsRegion(oldControls);
    }
    QDoubleSpinBox::leaveEvent(event);
}

void AntInputNumber::mouseMoveEvent(QMouseEvent* event)
{
    const QStyle::SubControl nextControl = hitSubControl(event->pos());
    if (m_activeSubControl != nextControl)
    {
        const QRect oldControls = controlsDirtyRect();
        m_activeSubControl = nextControl;
        if (m_controlsOverlay)
        {
            m_controlsOverlay->update();
        }
        updateControlsRegion(oldControls);
    }
    QDoubleSpinBox::mouseMoveEvent(event);
}

void AntInputNumber::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const QRect oldControls = controlsDirtyRect();
        const bool oldPressed = m_stepPressed;
        const QStyle::SubControl oldControl = m_activeSubControl;
        m_activeSubControl = hitSubControl(event->pos());
        m_stepPressed = m_activeSubControl == QStyle::SC_SpinBoxUp || m_activeSubControl == QStyle::SC_SpinBoxDown;
        if (oldControl != m_activeSubControl || oldPressed != m_stepPressed)
        {
            if (m_controlsOverlay)
            {
                m_controlsOverlay->update();
            }
            updateControlsRegion(oldControls);
        }
    }
    QDoubleSpinBox::mousePressEvent(event);
}

void AntInputNumber::mouseReleaseEvent(QMouseEvent* event)
{
    const QRect oldControls = controlsDirtyRect();
    const bool oldPressed = m_stepPressed;
    const QStyle::SubControl oldControl = m_activeSubControl;
    m_activeSubControl = hitSubControl(event->pos());
    m_stepPressed = false;
    if (oldControl != m_activeSubControl || oldPressed != m_stepPressed)
    {
        if (m_controlsOverlay)
        {
            m_controlsOverlay->update();
        }
        updateControlsRegion(oldControls);
    }
    QDoubleSpinBox::mouseReleaseEvent(event);
}

void AntInputNumber::focusInEvent(QFocusEvent* event)
{
    animateControls(true);
    updateFrameChrome();
    QDoubleSpinBox::focusInEvent(event);
}

void AntInputNumber::focusOutEvent(QFocusEvent* event)
{
    animateControls(m_hovered);
    updateFrameChrome();
    QDoubleSpinBox::focusOutEvent(event);
}

void AntInputNumber::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        animateControls(shouldShowControls());
        updateEditStyle();
        updateFrameChrome();
    }
    QDoubleSpinBox::changeEvent(event);
}

void AntInputNumber::resizeEvent(QResizeEvent* event)
{
    QDoubleSpinBox::resizeEvent(event);
    invalidateGeometryCache();
    updateControlsOverlayGeometry();
}

bool AntInputNumber::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == lineEdit())
    {
        switch (event->type())
        {
        case QEvent::Enter:
        case QEvent::FocusIn:
            animateControls(true);
            updateFrameChrome();
            break;
        case QEvent::Leave:
            animateControls(shouldShowControls());
            updateFrameChrome();
            break;
        case QEvent::FocusOut:
            animateControls(m_hovered);
            updateFrameChrome();
            break;
        default:
            break;
        }
    }
    return QDoubleSpinBox::eventFilter(watched, event);
}

AntInputNumber::Metrics AntInputNumber::metrics() const
{
    if (!m_metricsDirty)
    {
        return m_cachedMetrics;
    }

    const auto& token = antTheme->tokens();
    Metrics m;
    switch (m_inputSize)
    {
    case Ant::Size::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.radius = token.borderRadiusLG;
        m.paddingX = token.paddingSM;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        m.radius = token.borderRadiusSM;
        m.paddingX = token.paddingXS;
        break;
    case Ant::Size::Middle:
        m.height = token.controlHeight;
        m.fontSize = token.fontSize;
        m.radius = token.borderRadius;
        m.paddingX = token.paddingSM - token.lineWidth;
        break;
    }
    m_cachedMetrics = m;
    m_metricsDirty = false;
    ++m_metricsResolveCount;
    syncInputNumberPerfCounters();
    return m_cachedMetrics;
}

void AntInputNumber::invalidateMetricsCache() const
{
    m_metricsDirty = true;
    invalidateSizeHintCache();
    invalidateGeometryCache();
    syncInputNumberPerfCounters();
}

void AntInputNumber::invalidateSizeHintCache() const
{
    m_sizeHintDirty = true;
    syncInputNumberPerfCounters();
}

void AntInputNumber::invalidateGeometryCache() const
{
    m_controlsGeometryDirty = true;
    syncInputNumberPerfCounters();
}

void AntInputNumber::updateEditStyle()
{
    if (!lineEdit())
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    bool changed = false;

    QFont font = lineEdit()->font();
    if (font.pixelSize() != m.fontSize)
    {
        font.setPixelSize(m.fontSize);
        lineEdit()->setFont(font);
        changed = true;
    }
    if (lineEdit()->hasFrame())
    {
        lineEdit()->setFrame(false);
        changed = true;
    }
    if (lineEdit()->alignment() != alignment())
    {
        lineEdit()->setAlignment(alignment());
        changed = true;
    }

    const auto setControlColor = [](QPalette& target, QPalette::ColorRole role, const QColor& active, const QColor& disabled) {
        target.setColor(QPalette::Active, role, active);
        target.setColor(QPalette::Inactive, role, active);
        target.setColor(QPalette::Disabled, role, disabled);
    };

    QPalette controlPalette = palette();
    setControlColor(controlPalette, QPalette::Base, Qt::transparent, Qt::transparent);
    setControlColor(controlPalette, QPalette::Text, token.colorText, token.colorTextDisabled);
    setControlColor(controlPalette, QPalette::WindowText, token.colorText, token.colorTextDisabled);
    setControlColor(controlPalette, QPalette::ButtonText, token.colorText, token.colorTextDisabled);
    setControlColor(controlPalette, QPalette::Highlight, token.colorPrimary, token.colorPrimary);
    setControlColor(controlPalette, QPalette::HighlightedText, token.colorTextLightSolid, token.colorTextLightSolid);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    setControlColor(controlPalette, QPalette::PlaceholderText, token.colorTextPlaceholder, token.colorTextDisabled);
#endif
    if (palette() != controlPalette)
    {
        setPalette(controlPalette);
        changed = true;
    }

    QPalette lePalette = lineEdit()->palette();
    setControlColor(lePalette, QPalette::Base, Qt::transparent, Qt::transparent);
    setControlColor(lePalette, QPalette::Text, token.colorText, token.colorTextDisabled);
    setControlColor(lePalette, QPalette::WindowText, token.colorText, token.colorTextDisabled);
    setControlColor(lePalette, QPalette::ButtonText, token.colorText, token.colorTextDisabled);
    setControlColor(lePalette, QPalette::Highlight, token.colorPrimary, token.colorPrimary);
    setControlColor(lePalette, QPalette::HighlightedText, token.colorTextLightSolid, token.colorTextLightSolid);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    setControlColor(lePalette, QPalette::PlaceholderText, token.colorTextPlaceholder, token.colorTextDisabled);
#endif
    if (!m_lineEditTransparent || !lineEdit()->testAttribute(Qt::WA_TranslucentBackground))
    {
        lineEdit()->setAttribute(Qt::WA_TranslucentBackground, true);
        lineEdit()->setAutoFillBackground(false);
        m_lineEditTransparent = true;
        changed = true;
    }
    if (!lineEdit()->testAttribute(Qt::WA_NoSystemBackground))
    {
        lineEdit()->setAttribute(Qt::WA_NoSystemBackground, true);
        changed = true;
    }
    if (lineEdit()->autoFillBackground())
    {
        lineEdit()->setAutoFillBackground(false);
        changed = true;
    }
    if (lineEdit()->palette() != lePalette)
    {
        lineEdit()->setPalette(lePalette);
        changed = true;
    }
    updateLineEditControlsInset();

    const int contentHeight = qMax(1, m.height - 2 * token.lineWidth);
    if (lineEdit()->minimumHeight() != contentHeight)
    {
        lineEdit()->setMinimumHeight(contentHeight);
        changed = true;
    }
    if (lineEdit()->maximumHeight() != contentHeight)
    {
        lineEdit()->setMaximumHeight(contentHeight);
        changed = true;
    }

    if (minimumHeight() != m.height)
    {
        setMinimumHeight(m.height);
        changed = true;
    }
    if (maximumHeight() != m.height)
    {
        setMaximumHeight(m.height);
        changed = true;
    }

    if (changed)
    {
        ++m_editStyleApplyCount;
        syncInputNumberPerfCounters();
    }
}

void AntInputNumber::updateLineEditControlsInset()
{
    if (!lineEdit())
    {
        return;
    }
    const int inset = controlsInsetWidth();
    if (m_lastControlsInset == inset)
    {
        return;
    }
    m_lastControlsInset = inset;
    lineEdit()->setTextMargins(0, 0, inset, 0);
    ++m_controlsInsetUpdateCount;
    syncInputNumberPerfCounters();
}

void AntInputNumber::updateFrameChrome()
{
    const int inset = antTheme->tokens().controlOutlineWidth + antTheme->tokens().lineWidth + 2;
    const QRect r = rect();
    if (r.isEmpty() || m_variant == Ant::Variant::Filled || m_variant == Ant::Variant::Borderless)
    {
        ++m_frameUpdateCount;
        syncInputNumberPerfCounters();
        update();
        return;
    }

    QRegion region;
    if (m_variant == Ant::Variant::Underlined)
    {
        region += QRect(r.left(), qMax(r.top(), r.bottom() - inset), r.width(), inset + 1);
    }
    else
    {
        region += QRect(r.left(), r.top(), r.width(), inset);
        region += QRect(r.left(), qMax(r.top(), r.bottom() - inset + 1), r.width(), inset);
        region += QRect(r.left(), r.top(), inset, r.height());
        region += QRect(qMax(r.left(), r.right() - inset + 1), r.top(), inset, r.height());
    }
    ++m_frameUpdateCount;
    syncInputNumberPerfCounters();
    update(region);
}

void AntInputNumber::updateControlsRegion(const QRect& oldRect)
{
    QRect dirty = oldRect.united(controlsDirtyRect());
    if (!dirty.isValid() || dirty.isEmpty())
    {
        dirty = controlsDirtyRect();
    }
    if (!dirty.isValid() || dirty.isEmpty())
    {
        dirty = rect();
    }
    dirty = dirty.adjusted(-2, -2, 2, 2).intersected(rect());
    ++m_controlsRegionUpdateCount;
    syncInputNumberPerfCounters();
    update(dirty);
}

QRect AntInputNumber::controlsDirtyRect() const
{
    if (!m_controlsGeometryDirty)
    {
        return m_cachedControlsDirtyRect;
    }
    const int handlerWidth = handlerWidthForInput(this);
    const QRect control = rect().adjusted(1, 1, -1, -1);
    if (control.isEmpty() || handlerWidth <= 0)
    {
        m_cachedControlsDirtyRect = QRect();
    }
    else
    {
        m_cachedControlsDirtyRect = QRect(control.right() - handlerWidth + 1,
                                          control.top(),
                                          handlerWidth,
                                          control.height());
    }
    m_controlsGeometryDirty = false;
    syncInputNumberPerfCounters();
    return m_cachedControlsDirtyRect;
}

QStyle::SubControl AntInputNumber::hitSubControl(const QPoint& pos) const
{
    if (!m_controlsVisible || !isEnabled() || m_controlsProgress <= 0.01)
    {
        return QStyle::SC_None;
    }

    const QRect panelRect = controlsDirtyRect();
    if (!panelRect.contains(pos))
    {
        return QStyle::SC_None;
    }

    const QRect upRect(panelRect.left(), panelRect.top(), panelRect.width(), panelRect.height() / 2);
    if (upRect.contains(pos))
    {
        return QStyle::SC_SpinBoxUp;
    }
    const QRect downRect(panelRect.left(), panelRect.center().y(), panelRect.width(), panelRect.height() / 2);
    if (downRect.contains(pos))
    {
        return QStyle::SC_SpinBoxDown;
    }
    return QStyle::SC_None;
}

bool AntInputNumber::shouldShowControls() const
{
    return m_controlsVisible && isEnabled() && (m_hovered || hasFocus() || (lineEdit() && lineEdit()->hasFocus()));
}

void AntInputNumber::animateControls(bool visible)
{
    visible = visible && m_controlsVisible && isEnabled();
    const qreal end = visible ? 1.0 : 0.0;
    if (qFuzzyCompare(m_controlsProgress, end) && m_controlsAnimation->state() != QAbstractAnimation::Running)
    {
        return;
    }

    m_controlsAnimation->stop();
    m_controlsAnimation->setStartValue(m_controlsProgress);
    m_controlsAnimation->setEndValue(end);
    m_controlsAnimation->start();
}

int AntInputNumber::controlsInsetWidth() const
{
    if (!m_controlsVisible || !isEnabled())
    {
        return 0;
    }

    return qRound(handlerWidthForInput(this) * m_controlsProgress);
}

void AntInputNumber::updateControlsOverlayGeometry()
{
    if (!m_controlsOverlay)
    {
        return;
    }

    const int handlerWidth = handlerWidthForInput(this);
    const QRect control = rect().adjusted(1, 1, -1, -1);
    const QRect nextGeometry(control.right() - handlerWidth + 1,
                             control.top(),
                             handlerWidth,
                             control.height());
    const bool nextVisible = m_controlsVisible && isEnabled() && m_controlsProgress > 0.01;
    bool changed = false;
    if (m_controlsOverlay->geometry() != nextGeometry)
    {
        m_controlsOverlay->setGeometry(nextGeometry);
        changed = true;
    }
    if ((!m_controlsOverlay->isHidden()) != nextVisible)
    {
        m_controlsOverlay->setVisible(nextVisible);
        changed = true;
    }
    if (nextVisible)
    {
        m_controlsOverlay->raise();
    }
    if (changed)
    {
        ++m_controlsGeometryApplyCount;
        syncInputNumberPerfCounters();
    }
}

void AntInputNumber::syncInputNumberPerfCounters() const
{
    auto* self = const_cast<AntInputNumber*>(this);
    self->setProperty("antInputNumberMetricsResolveCount", m_metricsResolveCount);
    self->setProperty("antInputNumberSizeHintResolveCount", m_sizeHintResolveCount);
    self->setProperty("antInputNumberEditStyleApplyCount", m_editStyleApplyCount);
    self->setProperty("antInputNumberControlsInsetUpdateCount", m_controlsInsetUpdateCount);
    self->setProperty("antInputNumberControlsGeometryApplyCount", m_controlsGeometryApplyCount);
    self->setProperty("antInputNumberControlsRegionUpdateCount", m_controlsRegionUpdateCount);
    self->setProperty("antInputNumberFrameUpdateCount", m_frameUpdateCount);
}
