#pragma once

#include "core/QtAntDesignExport.h"

#include <QAbstractNativeEventFilter>
#include <QMainWindow>
#include <QPointer>
#include <QRect>

#include <array>

#include "core/AntTypes.h"

class AntModal;
class QEvent;
class QCloseEvent;
class QHideEvent;
class QMouseEvent;
class QMoveEvent;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QWidget;
class AntRibbon;

class QT_ANT_DESIGN_EXPORT AntWindow : public QMainWindow, public QAbstractNativeEventFilter
{
    Q_OBJECT
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool alwaysOnTop READ isAlwaysOnTop WRITE setAlwaysOnTop NOTIFY alwaysOnTopChanged)
    Q_PROPERTY(bool pinButtonVisible READ isPinButtonVisible WRITE setPinButtonVisible NOTIFY pinButtonVisibleChanged)
    Q_PROPERTY(bool themeButtonVisible READ isThemeButtonVisible WRITE setThemeButtonVisible NOTIFY themeButtonVisibleChanged)
    Q_PROPERTY(bool minimizeButtonVisible READ isMinimizeButtonVisible WRITE setMinimizeButtonVisible NOTIFY minimizeButtonVisibleChanged)
    Q_PROPERTY(bool maximizeButtonVisible READ isMaximizeButtonVisible WRITE setMaximizeButtonVisible NOTIFY maximizeButtonVisibleChanged)
    Q_PROPERTY(bool closeButtonVisible READ isCloseButtonVisible WRITE setCloseButtonVisible NOTIFY closeButtonVisibleChanged)
    Q_PROPERTY(bool closeConfirmationEnabled READ isCloseConfirmationEnabled WRITE setCloseConfirmationEnabled NOTIFY closeConfirmationEnabledChanged)
    Q_PROPERTY(QString closeConfirmationTitle READ closeConfirmationTitle WRITE setCloseConfirmationTitle NOTIFY closeConfirmationTitleChanged)
    Q_PROPERTY(QString closeConfirmationContent READ closeConfirmationContent WRITE setCloseConfirmationContent NOTIFY closeConfirmationContentChanged)
    Q_PROPERTY(QString closeConfirmationOkText READ closeConfirmationOkText WRITE setCloseConfirmationOkText NOTIFY closeConfirmationOkTextChanged)
    Q_PROPERTY(QString closeConfirmationCancelText READ closeConfirmationCancelText WRITE setCloseConfirmationCancelText NOTIFY closeConfirmationCancelTextChanged)
    Q_PROPERTY(int cornerRadius READ cornerRadius WRITE setCornerRadius NOTIFY cornerRadiusChanged)

public:
    enum class TitleBarButton
    {
        None,
        Pin,
        Theme,
        Minimize,
        Maximize,
        Close,
    };
    Q_ENUM(TitleBarButton)

    explicit AntWindow(QWidget* parent = nullptr);
    ~AntWindow() override;

    void setWindowTitle(const QString& title);
    void setCentralWidget(QWidget* widget);
    void setRibbon(AntRibbon* ribbon);
    AntRibbon* ribbon() const;
    void setRibbonVisible(bool visible);
    bool isRibbonVisible() const;
    void moveToCenter();

    static constexpr int TitleBarHeight = 40;
    static constexpr int TitleBarButtonWidth = 46;

    bool isMaximized() const;
    bool isAlwaysOnTop() const;
    void setAlwaysOnTop(bool on);
    void toggleAlwaysOnTop();
    int cornerRadius() const;
    void setCornerRadius(int radius);
    // True when this AntWindow is taking the Win10 opaque path (no
    // WA_TranslucentBackground, no AA corner smoother, no DWM glass
    // extension, square corners). On Win11 / non-Windows this is false
    // and the original alpha-corner path is used.
    bool usesLegacyOpaquePath() const;

    bool isTitleBarButtonVisible(TitleBarButton button) const;
    void setTitleBarButtonVisible(TitleBarButton button, bool visible);
    QRect titleBarButtonRect(TitleBarButton button) const;
    TitleBarButton hoveredTitleBarButton() const;

    bool isPinButtonVisible() const;
    bool isThemeButtonVisible() const;
    bool isMinimizeButtonVisible() const;
    bool isMaximizeButtonVisible() const;
    bool isCloseButtonVisible() const;
    bool isCloseConfirmationEnabled() const;
    QString closeConfirmationTitle() const;
    QString closeConfirmationContent() const;
    QString closeConfirmationOkText() const;
    QString closeConfirmationCancelText() const;

    void setPinButtonVisible(bool visible);
    void setThemeButtonVisible(bool visible);
    void setMinimizeButtonVisible(bool visible);
    void setMaximizeButtonVisible(bool visible);
    void setCloseButtonVisible(bool visible);
    void setCloseConfirmationEnabled(bool enabled);
    void setCloseConfirmationTitle(const QString& title);
    void setCloseConfirmationContent(const QString& content);
    void setCloseConfirmationOkText(const QString& text);
    void setCloseConfirmationCancelText(const QString& text);
    void forceClose();

Q_SIGNALS:
    void windowTitleChanged(const QString& title);
    void alwaysOnTopChanged(bool on);
    void titleBarButtonVisibilityChanged(AntWindow::TitleBarButton button, bool visible);
    void pinButtonVisibleChanged(bool visible);
    void themeButtonVisibleChanged(bool visible);
    void minimizeButtonVisibleChanged(bool visible);
    void maximizeButtonVisibleChanged(bool visible);
    void closeButtonVisibleChanged(bool visible);
    void closeConfirmationEnabledChanged(bool enabled);
    void closeConfirmationTitleChanged(const QString& title);
    void closeConfirmationContentChanged(const QString& content);
    void closeConfirmationOkTextChanged(const QString& text);
    void closeConfirmationCancelTextChanged(const QString& text);
    void cornerRadiusChanged(int radius);
    void minimizeRequested();
    void maximizeRequested();
    void restoreRequested();
    void closeRequested();

protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, AntNativeEventResult* result) override;
    bool nativeEventFilter(const QByteArray& eventType, void* message, AntNativeEventResult* result) override;

private:
    bool isTitleBarArea(const QPoint& pos) const;
    bool isButtonArea(const QPoint& pos) const;
    TitleBarButton buttonAtPosition(const QPoint& pos) const;
    QRect titleBarRect() const;
    void updateTitleBarHover(const QPoint& pos);
    void setHoveredTitleBarButton(TitleBarButton button);
    void clearTitleBarHover();
    bool handleTitleBarMousePress(const QPoint& pos, const QPoint& globalPos, Qt::MouseButton button);
    bool handleTitleBarMouseMove(const QPoint& pos, const QPoint& globalPos, Qt::MouseButtons buttons);
    bool handleTitleBarMouseRelease(const QPoint& pos, const QPoint& globalPos, Qt::MouseButton button);
    bool handleTitleBarMouseDoubleClick(const QPoint& pos, Qt::MouseButton button);
    void handleButtonClicked(TitleBarButton button);
    void ensureTitleBarButtonRectCache() const;
    void invalidateTitleBarButtonRectCache() const;
    int titleBarButtonVisibilityMask() const;
    QRect titleBarButtonStripRect() const;
    void updateTitleBarRegion(const QRect& rect);
    void syncTitleBarPerfCounters() const;
    void showCloseConfirmationModal();
    void syncCloseConfirmationModal();
    void startThemeModeTransition();
    void emitTitleBarButtonVisibleChanged(TitleBarButton button, bool visible);
    void applyManualSnap(const QPoint& globalPos);
    void applyNativeWindowFrame();
    void updateLegacySoftwareShadow();
    void hideLegacySoftwareShadow();
    void updateCornerSmoother();
    void syncTheme();
    void applyContentPalette(QWidget* widget);

    bool m_dragging = false;
    QPoint m_dragStartPosition;
    QPoint m_dragStartWindowPos;
    qreal m_dragStartTitleXRatio = 0.5;
    int m_dragStartTitleY = TitleBarHeight / 2;
    bool m_windowMaximized = false;
    bool m_alwaysOnTop = false;
    bool m_pinButtonVisible = true;
    bool m_themeButtonVisible = true;
    bool m_minimizeButtonVisible = true;
    bool m_maximizeButtonVisible = true;
    bool m_closeButtonVisible = true;
    bool m_closeConfirmationEnabled = false;
    bool m_closingWithoutConfirmation = false;
    QString m_closeConfirmationTitle = QStringLiteral("Exit application?");
    QString m_closeConfirmationContent = QStringLiteral("The window will close. Do you want to exit?");
    QString m_closeConfirmationOkText = QStringLiteral("Exit");
    QString m_closeConfirmationCancelText = QStringLiteral("Cancel");
    int m_cornerRadius = 8;
    TitleBarButton m_hoveredButton = TitleBarButton::None;
    TitleBarButton m_pressedButton = TitleBarButton::None;
    mutable std::array<QRect, 6> m_titleBarButtonRectCache;
    mutable int m_titleBarButtonCacheWidth = -1;
    mutable int m_titleBarButtonCacheMask = -1;
    mutable int m_titleBarButtonRectCacheRebuildCount = 0;
    int m_titleBarDirtyUpdateCount = 0;
    QWidget* m_contentWidget = nullptr;
    QWidget* m_centralContentWidget = nullptr;
    AntRibbon* m_ribbon = nullptr;
    bool m_ribbonVisible = true;
    QWidget* m_themeTransitionOverlay = nullptr;
    QPointer<AntModal> m_closeConfirmationModal;
    QWidget* m_legacySoftwareShadow = nullptr;
    QWidget* m_cornerSmoother = nullptr;
    bool m_legacyLiveResize = false;
    bool m_useTranslucentBackground = true;
};
