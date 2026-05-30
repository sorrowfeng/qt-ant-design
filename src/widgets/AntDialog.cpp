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
#include <QLineEdit>
#include <QPalette>
#include <QShowEvent>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include "../styles/AntDialogStyle.h"
#include "AntScrollBar.h"
#include "core/AntTheme.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace
{
constexpr auto kAntDialogForceLegacyFramePolicyProperty = "antDialogForceLegacyFramePolicy";

bool antDialogLegacyPolicyEnabled(const QWidget* widget)
{
    if (widget && widget->property(kAntDialogForceLegacyFramePolicyProperty).toBool())
    {
        return true;
    }

    const QByteArray policy = qgetenv("QT_ANT_DESIGN_FORCE_LEGACY_FRAME").trimmed().toLower();
    return policy == "1" || policy == "true" || policy == "yes" || policy == "on";
}

#ifdef Q_OS_WIN
using AntDialogRtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOW*);

int antDialogWindowsBuildNumber()
{
    static const int buildNumber = []() {
        if (HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll"))
        {
            auto* rtlGetVersion =
                reinterpret_cast<AntDialogRtlGetVersionFn>(::GetProcAddress(ntdll, "RtlGetVersion"));
            if (rtlGetVersion)
            {
                OSVERSIONINFOW version{};
                version.dwOSVersionInfoSize = sizeof(version);
                if (rtlGetVersion(&version) == 0)
                {
                    return static_cast<int>(version.dwBuildNumber);
                }
            }
        }
        return 0;
    }();
    return buildNumber;
}

bool antDialogShouldUseRoundedCorners(const QWidget* widget)
{
    constexpr int kWindows11Build = 22000;
    return antDialogWindowsBuildNumber() >= kWindows11Build && !antDialogLegacyPolicyEnabled(widget);
}
#else
bool antDialogShouldUseRoundedCorners(const QWidget* widget)
{
    return !antDialogLegacyPolicyEnabled(widget);
}
#endif

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
    const int inset = qMax(1, antTheme->tokens().lineWidth);
    return QRect(inset, inset, qMax(0, width() - inset * 2), m_titleBarHeight);
}

QRect AntDialog::titleBarTextRect() const
{
    QRect textRect = titleBarRect().adjusted(16, 0, -16, 0);
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
    const int width = 46;
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
    case QEvent::WindowTitleChange:
        update(titleBarRect());
        break;
    case QEvent::DynamicPropertyChange:
    {
        auto* propertyEvent = static_cast<QDynamicPropertyChangeEvent*>(event);
        if (propertyEvent->propertyName() == kAntDialogForceLegacyFramePolicyProperty)
        {
            updateRoundedCornerPolicy();
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

void AntDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    scheduleChildSync();
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
}

int AntDialog::effectiveCornerRadius() const
{
    return usesRoundedCorners() ? m_cornerRadius : 0;
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
