#include "AntDialog.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QByteArray>
#include <QDialogButtonBox>
#include <QDynamicPropertyChangeEvent>
#include <QEvent>
#include <QFrame>
#include <QHeaderView>
#include <QHideEvent>
#include <QLineEdit>
#include <QMoveEvent>
#include <QPalette>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include "../styles/AntDialogStyle.h"
#include "AntScrollBar.h"
#include "AntWindowFrame.h"
#include "core/AntTheme.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#endif

namespace
{
constexpr auto kAntDialogForceLegacyFramePolicyProperty = "antDialogForceLegacyFramePolicy";
constexpr auto kAntDialogLegacySoftwareShadowObjectName = "AntDialogLegacySoftwareShadow";
constexpr auto kAntDialogUsesNativeCaptionFrameProperty = "antDialogUsesNativeCaptionFrame";
constexpr auto kAntDialogDwmFrameMarginsProperty = "antDialogDwmFrameMargins";
constexpr auto kAntDialogDwmFrameApplyCountProperty = "antDialogDwmFrameApplyCount";
constexpr auto kAntDialogDwmFrameLastReasonProperty = "antDialogDwmFrameLastReason";
constexpr auto kAntDialogLegacyRoundedMaskAppliedProperty = "antDialogLegacyRoundedMaskApplied";
constexpr auto kAntDialogLegacyRoundedMaskFrameInsetProperty = "antDialogLegacyRoundedMaskFrameInset";
constexpr auto kAntDialogLegacyClassDropShadowEnabledProperty = "antDialogLegacyClassDropShadowEnabled";

#ifdef Q_OS_WIN
QPoint antDialogNativeMessageLocalPoint(HWND hwnd, LPARAM messagePos, qreal devicePixelRatio)
{
    POINT nativePoint{GET_X_LPARAM(messagePos), GET_Y_LPARAM(messagePos)};
    ::ScreenToClient(hwnd, &nativePoint);
    const qreal dpr = qMax<qreal>(1.0, devicePixelRatio);
    return QPoint(qRound(static_cast<qreal>(nativePoint.x) / dpr),
                  qRound(static_cast<qreal>(nativePoint.y) / dpr));
}
#endif

bool antDialogShouldUseRoundedCorners(const QWidget* widget)
{
    return AntWindowFrame::usesNativeCaptionFrameForWidget(widget, kAntDialogForceLegacyFramePolicyProperty);
}

QPoint antDialogGlobalPos(QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return antEventGlobalPosition(event);
#else
    return event->globalPos();
#endif
}

void setAntDialogScrollBars(QAbstractScrollArea* area)
{
    if (!area)
    {
        return;
    }
    if (!qobject_cast<AntScrollBar*>(area->verticalScrollBar()))
    {
        area->setVerticalScrollBar(new AntScrollBar(Qt::Vertical, area));
    }
    if (!qobject_cast<AntScrollBar*>(area->horizontalScrollBar()))
    {
        area->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal, area));
    }
}
} // namespace

AntDialog::AntDialog(QWidget* parent, Qt::WindowFlags flags)
    : QDialog(parent, flags | Qt::FramelessWindowHint)
{
    qRegisterMetaType<AntDialog::TitleBarButton>("AntDialog::TitleBarButton");
    initializeAntStyle();

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setSpacing(0);

    m_contentWidget = new QWidget(this);
    m_contentWidget->setObjectName(QStringLiteral("AntDialogContentWidget"));
    m_rootLayout->addWidget(m_contentWidget);
    updateChromeMargins();

    refreshThemeCache();
    connect(antTheme, &AntTheme::themeModeChanged, this, &AntDialog::handleThemeChanged);
    syncChildControls();
}

const AntThemeTokens& AntDialog::tokens() const
{
    return antTheme->tokens();
}

Ant::ThemeMode AntDialog::currentTheme() const
{
    return antTheme->themeMode();
}

QWidget* AntDialog::contentWidget() const
{
    return m_contentWidget;
}

void AntDialog::setContentWidget(QWidget* widget)
{
    if (!widget || widget == m_contentWidget)
    {
        return;
    }

    if (m_contentWidget)
    {
        m_rootLayout->removeWidget(m_contentWidget);
        m_contentWidget->deleteLater();
    }

    m_contentWidget = widget;
    m_contentWidget->setParent(this);
    m_rootLayout->addWidget(m_contentWidget);
    updateChromeMargins();
    syncChildControls();
    updateGeometry();
    update();
}

bool AntDialog::isTitleBarVisible() const
{
    return m_titleBarVisible;
}

void AntDialog::setTitleBarVisible(bool visible)
{
    if (m_titleBarVisible == visible)
    {
        return;
    }

    m_titleBarVisible = visible;
    if (!m_titleBarVisible)
    {
        clearTitleBarHover();
        clearTitleBarPress();
    }
    updateChromeMargins();
    updateGeometry();
    update();
    syncDialogPerfCounters();
    Q_EMIT titleBarVisibleChanged(m_titleBarVisible);
}

bool AntDialog::isCloseButtonVisible() const
{
    return m_closeButtonVisible;
}

void AntDialog::setCloseButtonVisible(bool visible)
{
    if (m_closeButtonVisible == visible)
    {
        return;
    }

    const QRect oldRect = titleBarCloseButtonRect();
    m_closeButtonVisible = visible;
    if (!m_closeButtonVisible)
    {
        clearTitleBarHover();
        clearTitleBarPress();
    }
    update(oldRect.united(titleBarCloseButtonRect()));
    syncDialogPerfCounters();
    Q_EMIT closeButtonVisibleChanged(m_closeButtonVisible);
}

int AntDialog::titleBarHeight() const
{
    return m_titleBarHeight;
}

void AntDialog::setTitleBarHeight(int height)
{
    const int normalized = qMax(32, height);
    if (m_titleBarHeight == normalized)
    {
        return;
    }

    m_titleBarHeight = normalized;
    updateChromeMargins();
    updateGeometry();
    update();
    syncDialogPerfCounters();
    Q_EMIT titleBarHeightChanged(m_titleBarHeight);
}

int AntDialog::cornerRadius() const
{
    return m_cornerRadius;
}

void AntDialog::setCornerRadius(int radius)
{
    const int normalized = qMax(0, radius);
    if (m_cornerRadius == normalized)
    {
        return;
    }

    m_cornerRadius = normalized;
    updateRoundedCornerPolicy();
    updateChromeMargins();
    updateGeometry();
    update();
    syncDialogPerfCounters();
    Q_EMIT cornerRadiusChanged(m_cornerRadius);
}

bool AntDialog::usesRoundedCorners() const
{
    return m_useRoundedCorners && m_cornerRadius > 0;
}

bool AntDialog::usesLegacyOpaquePath() const
{
    return !m_useRoundedCorners;
}

QRect AntDialog::titleBarRect() const
{
    if (!m_titleBarVisible)
    {
        return QRect();
    }
    const QRect surface = surfaceRect();
    if (!surface.isValid())
    {
        return QRect();
    }
    return QRect(surface.left(), surface.top(), surface.width(), m_titleBarHeight);
}

QRect AntDialog::titleBarTextRect() const
{
    const int iconOffset = windowIcon().isNull() ? 0 : 16 + 8;
    QRect textRect = titleBarRect().adjusted(12 + iconOffset, 0, -12, 0);
    if (m_closeButtonVisible)
    {
        textRect.setRight(qMin(textRect.right(), titleBarCloseButtonRect().left() - 12));
    }
    return textRect;
}

QRect AntDialog::titleBarCloseButtonRect() const
{
    if (!m_titleBarVisible || !m_closeButtonVisible)
    {
        return QRect();
    }
    const QRect bar = titleBarRect();
    const int width = TitleBarButtonWidth;
    return QRect(bar.right() - width + 1, bar.top(), width, bar.height());
}

AntDialog::TitleBarButton AntDialog::hoveredTitleBarButton() const
{
    return m_hoveredButton;
}

AntDialog::TitleBarButton AntDialog::pressedTitleBarButton() const
{
    return m_pressedButton;
}

void AntDialog::refreshAntStyle()
{
    syncChildControls();
    updateChromeMargins();
    applyNativeWindowFrame();
    updateLegacySoftwareShadow();
    update();
}

void AntDialog::onThemeChanged(Ant::ThemeMode mode)
{
    Q_UNUSED(mode)
}

bool AntDialog::event(QEvent* event)
{
    const bool handled = QDialog::event(event);
    switch (event->type())
    {
    case QEvent::ChildAdded:
    case QEvent::LayoutRequest:
    case QEvent::PaletteChange:
    case QEvent::Polish:
    case QEvent::StyleChange:
        scheduleChildSync();
        break;
    case QEvent::WindowIconChange:
    case QEvent::WindowTitleChange:
        update(titleBarRect());
        break;
    case QEvent::DynamicPropertyChange:
    {
        auto* propertyEvent = static_cast<QDynamicPropertyChangeEvent*>(event);
        if (propertyEvent->propertyName() == kAntDialogForceLegacyFramePolicyProperty)
        {
            updateRoundedCornerPolicy();
            updateChromeMargins();
            applyNativeWindowFrame();
            updateGeometry();
            update();
        }
        break;
    }
    default:
        break;
    }
    return handled;
}

void AntDialog::leaveEvent(QEvent* event)
{
    if (!m_dragging)
    {
        clearTitleBarHover();
    }
    QDialog::leaveEvent(event);
}

void AntDialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const QPoint pos = event->pos();
        if (titleBarCloseButtonRect().contains(pos))
        {
            m_pressedButton = TitleBarButton::Close;
            update(titleBarCloseButtonRect());
            event->accept();
            return;
        }

        if (titleBarRect().contains(pos))
        {
            m_dragging = true;
            m_dragStartGlobalPos = antDialogGlobalPos(event);
            m_dragStartDialogPos = frameGeometry().topLeft();
            event->accept();
            return;
        }
    }
    QDialog::mousePressEvent(event);
}

void AntDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging && event->buttons().testFlag(Qt::LeftButton))
    {
        move(m_dragStartDialogPos + antDialogGlobalPos(event) - m_dragStartGlobalPos);
        event->accept();
        return;
    }

    updateTitleBarHover(event->pos());
    QDialog::mouseMoveEvent(event);
}

void AntDialog::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        const TitleBarButton pressed = m_pressedButton;
        const QRect closeRect = titleBarCloseButtonRect();
        m_dragging = false;
        clearTitleBarPress();
        updateTitleBarHover(event->pos());

        if (pressed == TitleBarButton::Close && closeRect.contains(event->pos()))
        {
            reject();
            event->accept();
            return;
        }
    }
    QDialog::mouseReleaseEvent(event);
}

void AntDialog::moveEvent(QMoveEvent* event)
{
    QDialog::moveEvent(event);
    updateLegacySoftwareShadow();
}

void AntDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    applyNativeWindowFrame();
    updateLegacySoftwareShadow();
}

void AntDialog::hideEvent(QHideEvent* event)
{
    hideLegacySoftwareShadow();
    QDialog::hideEvent(event);
}

void AntDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    scheduleChildSync();
    applyNativeWindowFrame();
    updateLegacySoftwareShadow();
}

bool AntDialog::nativeEvent(const QByteArray& eventType, void* message, AntNativeEventResult* result)
{
#ifdef Q_OS_WIN
    if ((eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") && message)
    {
        auto* msg = static_cast<MSG*>(message);
        const HWND hwnd = msg->hwnd;
        switch (msg->message)
        {
        case WM_NCCALCSIZE:
            *result = 0;
            return true;
        case WM_NCACTIVATE:
            if (usesLegacyOpaquePath())
            {
                *result = ::DefWindowProcW(hwnd, WM_NCACTIVATE, msg->wParam, -1);
                return true;
            }
            return false;
        case WM_NCPAINT:
            if (usesLegacyOpaquePath())
            {
                *result = 0;
                return true;
            }
            return false;
        case WM_NCHITTEST:
        {
            const QPoint widgetPos = antDialogNativeMessageLocalPoint(hwnd, msg->lParam, devicePixelRatioF());
            if (titleBarCloseButtonRect().contains(widgetPos))
            {
                *result = HTCLIENT;
                return true;
            }
            if (titleBarRect().contains(widgetPos))
            {
                *result = HTCAPTION;
                return true;
            }
            *result = HTCLIENT;
            return true;
        }
        default:
            break;
        }
    }
#endif
    return QDialog::nativeEvent(eventType, message, result);
}

void AntDialog::initializeAntStyle()
{
    updateRoundedCornerPolicy();
    installAntStyle<AntDialogStyle>(this);
    setAttribute(Qt::WA_Hover, true);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(320, 180);
}

void AntDialog::updateRoundedCornerPolicy()
{
    // Keep the top-level corner policy aligned with AntWindow: Win11 and
    // non-Windows may use alpha-painted rounded corners; Win10 takes the
    // stable opaque path and therefore paints square corners.
    m_useRoundedCorners = antDialogShouldUseRoundedCorners(this);
    const bool rounded = usesRoundedCorners();
    setAttribute(Qt::WA_TranslucentBackground, rounded);
    if (!rounded)
    {
        clearMask();
    }
    setProperty("antDialogUsesRoundedCorners", rounded);
    setProperty("antDialogUsesLegacyOpaquePath", usesLegacyOpaquePath());
    setProperty("antDialogCornerRadius", m_cornerRadius);
    setProperty("antDialogEffectiveCornerRadius", effectiveCornerRadius());
    setProperty("antDialogShadowMargin", shadowMargin());
    setProperty("antDialogShadowInnerClearance", AntWindowFrame::LegacySoftwareShadowInnerClearance);
    applyNativeWindowFrame();
    updateLegacySoftwareShadow();
}

int AntDialog::effectiveCornerRadius() const
{
    return usesRoundedCorners() ? m_cornerRadius : 0;
}

int AntDialog::shadowMargin() const
{
    return 0;
}

QRect AntDialog::surfaceRect() const
{
    const int margin = shadowMargin();
    return rect().adjusted(margin, margin, -margin, -margin);
}

void AntDialog::updateChromeMargins()
{
    if (!m_rootLayout)
    {
        return;
    }

    const int lineWidth = qMax(1, antTheme->tokens().lineWidth);
    const int top = m_titleBarVisible ? m_titleBarHeight + lineWidth : lineWidth;
    m_rootLayout->setContentsMargins(lineWidth, top, lineWidth, lineWidth);
}

void AntDialog::applyNativeWindowFrame()
{
#ifdef Q_OS_WIN
    if (!windowHandle())
    {
        return;
    }

    AntWindowFrame::NativeFrameOptions frameOptions;
    frameOptions.forceLegacyFramePolicyProperty = kAntDialogForceLegacyFramePolicyProperty;
    frameOptions.usesNativeCaptionFrameProperty = kAntDialogUsesNativeCaptionFrameProperty;
    frameOptions.dwmFrameMarginsProperty = kAntDialogDwmFrameMarginsProperty;
    frameOptions.dwmFrameApplyCountProperty = kAntDialogDwmFrameApplyCountProperty;
    frameOptions.dwmFrameLastReasonProperty = kAntDialogDwmFrameLastReasonProperty;
    frameOptions.legacyRoundedMaskAppliedProperty = kAntDialogLegacyRoundedMaskAppliedProperty;
    frameOptions.legacyRoundedMaskFrameInsetProperty = kAntDialogLegacyRoundedMaskFrameInsetProperty;
    frameOptions.legacyClassDropShadowEnabledProperty = kAntDialogLegacyClassDropShadowEnabledProperty;
    frameOptions.cornerRadius = m_cornerRadius;
    frameOptions.translucentBackground = usesRoundedCorners();
    frameOptions.maximized = isMaximized();
    frameOptions.fullScreen = isFullScreen();
    frameOptions.enableNativeCaption = true;
    frameOptions.enableMinimizeBox = false;
    frameOptions.enableMaximizeBox = false;
    AntWindowFrame::applyNativeFrame(this, frameOptions);
#else
    update();
#endif
}

void AntDialog::updateLegacySoftwareShadow()
{
#ifdef Q_OS_WIN
    const bool enabled = isVisible()
        && windowHandle()
        && usesLegacyOpaquePath()
        && !isMinimized();

    AntWindowFrame::updateLegacySoftwareShadow(this,
                                               m_legacySoftwareShadow,
                                               QString::fromLatin1(kAntDialogLegacySoftwareShadowObjectName),
                                               "antDialogLegacySoftwareShadowEnabled",
                                               "antDialogLegacySoftwareShadowMargin",
                                               "antDialogLegacySoftwareShadowInnerClearance",
                                               "antDialogLegacySoftwareShadowGeometry",
                                               "antDialogLegacySoftwareShadowGeometryMode",
                                               "antDialogLegacySoftwareShadowDevicePixelRatio",
                                               "antDialogLegacySoftwareShadowClickThrough",
                                               enabled,
                                               0);
#else
    hideLegacySoftwareShadow();
#endif
}

void AntDialog::hideLegacySoftwareShadow()
{
    AntWindowFrame::hideLegacySoftwareShadow(this,
                                             m_legacySoftwareShadow,
                                             "antDialogLegacySoftwareShadowEnabled",
                                             "antDialogLegacySoftwareShadowClickThrough");
}

void AntDialog::handleThemeChanged(Ant::ThemeMode mode)
{
    ++m_themeChangeCount;

    const bool styleOrPaletteChanged = style() != m_cachedStyle || palette().cacheKey() != m_cachedPaletteKey;
    if (styleOrPaletteChanged)
    {
        style()->unpolish(this);
        style()->polish(this);
        ++m_repolishCount;
    }

    refreshAntStyle();
    if (refreshCachedHints())
    {
        updateGeometry();
        ++m_updateGeometryCount;
    }

    ++m_surfaceUpdateCount;
    refreshThemeCache();
    applyNativeWindowFrame();
    updateLegacySoftwareShadow();
    syncDialogPerfCounters();
    onThemeChanged(mode);
}

bool AntDialog::refreshCachedHints()
{
    const QSize currentSizeHint = sizeHint();
    const QSize currentMinimumSizeHint = minimumSizeHint();
    const bool changed = currentSizeHint != m_cachedSizeHint || currentMinimumSizeHint != m_cachedMinimumSizeHint;
    m_cachedSizeHint = currentSizeHint;
    m_cachedMinimumSizeHint = currentMinimumSizeHint;
    return changed;
}

void AntDialog::refreshThemeCache()
{
    m_cachedStyle = style();
    m_cachedPaletteKey = palette().cacheKey();
    refreshCachedHints();
    syncDialogPerfCounters();
}

void AntDialog::scheduleChildSync()
{
    if (m_childSyncQueued)
    {
        return;
    }
    m_childSyncQueued = true;
    QTimer::singleShot(0, this, [this]() {
        m_childSyncQueued = false;
        syncChildControls();
    });
}

void AntDialog::syncChildControls()
{
    applyDialogPalette(this);
    applyDialogPalette(m_contentWidget);

    const auto scrollAreas = findChildren<QAbstractScrollArea*>();
    for (QAbstractScrollArea* area : scrollAreas)
    {
        area->setFrameShape(QFrame::NoFrame);
        applyDialogPalette(area);
        if (area->viewport())
        {
            applyDialogPalette(area->viewport());
            area->viewport()->setAutoFillBackground(true);
        }
        setAntDialogScrollBars(area);
    }

    const auto itemViews = findChildren<QAbstractItemView*>();
    for (QAbstractItemView* view : itemViews)
    {
        view->setAlternatingRowColors(false);
        view->setMouseTracking(true);
        applyDialogPalette(view);
        if (view->viewport())
        {
            applyDialogPalette(view->viewport());
        }
    }

    const auto headers = findChildren<QHeaderView*>();
    for (QHeaderView* header : headers)
    {
        header->setHighlightSections(false);
        applyDialogPalette(header);
    }

    const auto lineEdits = findChildren<QLineEdit*>();
    for (QLineEdit* lineEdit : lineEdits)
    {
        lineEdit->setFrame(false);
        lineEdit->setAttribute(Qt::WA_TranslucentBackground, true);
        lineEdit->setAutoFillBackground(false);
        applyDialogPalette(lineEdit);
    }

    const auto toolButtons = findChildren<QToolButton*>();
    for (QToolButton* button : toolButtons)
    {
        button->setAutoRaise(true);
        button->setMouseTracking(true);
        applyDialogPalette(button);
    }

    const auto buttons = findChildren<QAbstractButton*>();
    for (QAbstractButton* button : buttons)
    {
        button->setMouseTracking(true);
        applyDialogPalette(button);
    }

    const auto buttonBoxes = findChildren<QDialogButtonBox*>();
    for (QDialogButtonBox* box : buttonBoxes)
    {
        applyDialogPalette(box);
    }

    ++m_childSyncCount;
    syncDialogPerfCounters();
}

void AntDialog::applyDialogPalette(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::Base, token.colorBgContainer);
    pal.setColor(QPalette::AlternateBase, token.colorFillQuaternary);
    pal.setColor(QPalette::Button, token.colorBgContainer);
    pal.setColor(QPalette::Text, token.colorText);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::ButtonText, token.colorText);
    pal.setColor(QPalette::PlaceholderText, token.colorTextPlaceholder);
    pal.setColor(QPalette::Highlight, token.colorPrimaryBg);
    pal.setColor(QPalette::HighlightedText, token.colorText);
    pal.setColor(QPalette::Disabled, QPalette::Text, token.colorTextDisabled);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, token.colorTextDisabled);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, token.colorTextDisabled);
    pal.setColor(QPalette::Disabled, QPalette::Base, token.colorBgContainerDisabled);
    pal.setColor(QPalette::Disabled, QPalette::Button, token.colorBgContainerDisabled);
    widget->setPalette(pal);
}

void AntDialog::updateTitleBarHover(const QPoint& pos)
{
    setHoveredTitleBarButton(titleBarCloseButtonRect().contains(pos)
                                 ? TitleBarButton::Close
                                 : TitleBarButton::None);
}

void AntDialog::setHoveredTitleBarButton(TitleBarButton button)
{
    if (m_hoveredButton == button)
    {
        return;
    }

    const QRect oldRect = titleBarCloseButtonRect();
    m_hoveredButton = button;
    update(oldRect.united(titleBarCloseButtonRect()));
    syncDialogPerfCounters();
}

void AntDialog::clearTitleBarHover()
{
    setHoveredTitleBarButton(TitleBarButton::None);
}

void AntDialog::clearTitleBarPress()
{
    if (m_pressedButton == TitleBarButton::None)
    {
        return;
    }

    const QRect oldRect = titleBarCloseButtonRect();
    m_pressedButton = TitleBarButton::None;
    update(oldRect.united(titleBarCloseButtonRect()));
    syncDialogPerfCounters();
}

void AntDialog::syncDialogPerfCounters() const
{
    auto* self = const_cast<AntDialog*>(this);
    self->setProperty("antDialogChildSyncCount", m_childSyncCount);
    self->setProperty("antDialogThemeChangeCount", m_themeChangeCount);
    self->setProperty("antDialogThemeRepolishCount", m_repolishCount);
    self->setProperty("antDialogUpdateGeometryCount", m_updateGeometryCount);
    self->setProperty("antDialogSurfaceUpdateCount", m_surfaceUpdateCount);
    self->setProperty("antDialogTitleBarVisible", m_titleBarVisible);
    self->setProperty("antDialogCloseButtonVisible", m_closeButtonVisible);
    self->setProperty("antDialogUsesRoundedCorners", usesRoundedCorners());
    self->setProperty("antDialogUsesLegacyOpaquePath", usesLegacyOpaquePath());
    self->setProperty("antDialogCornerRadius", m_cornerRadius);
    self->setProperty("antDialogEffectiveCornerRadius", effectiveCornerRadius());
    self->setProperty("antDialogHoveredTitleBarButton", static_cast<int>(m_hoveredButton));
    self->setProperty("antDialogPressedTitleBarButton", static_cast<int>(m_pressedButton));
    self->setProperty("antDialogCachedSizeHint", m_cachedSizeHint);
    self->setProperty("antDialogCachedMinimumSizeHint", m_cachedMinimumSizeHint);
}
