#pragma once

#include <QMainWindow>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QShowEvent;
class QWidget;

class AntWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool alwaysOnTop READ isAlwaysOnTop WRITE setAlwaysOnTop NOTIFY alwaysOnTopChanged)
    Q_PROPERTY(bool pinButtonVisible READ isPinButtonVisible WRITE setPinButtonVisible NOTIFY pinButtonVisibleChanged)
    Q_PROPERTY(bool themeButtonVisible READ isThemeButtonVisible WRITE setThemeButtonVisible NOTIFY themeButtonVisibleChanged)
    Q_PROPERTY(bool minimizeButtonVisible READ isMinimizeButtonVisible WRITE setMinimizeButtonVisible NOTIFY minimizeButtonVisibleChanged)
    Q_PROPERTY(bool maximizeButtonVisible READ isMaximizeButtonVisible WRITE setMaximizeButtonVisible NOTIFY maximizeButtonVisibleChanged)
    Q_PROPERTY(bool closeButtonVisible READ isCloseButtonVisible WRITE setCloseButtonVisible NOTIFY closeButtonVisibleChanged)
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
    ~AntWindow() override = default;

    void setWindowTitle(const QString& title);
    void setCentralWidget(QWidget* widget);
    void moveToCenter();

    static constexpr int TitleBarHeight = 40;
    static constexpr int TitleBarButtonWidth = 46;

    bool isMaximized() const;
    bool isAlwaysOnTop() const;
    void setAlwaysOnTop(bool on);
    void toggleAlwaysOnTop();
    int cornerRadius() const;
    void setCornerRadius(int radius);

    bool isTitleBarButtonVisible(TitleBarButton button) const;
    void setTitleBarButtonVisible(TitleBarButton button, bool visible);
    QRect titleBarButtonRect(TitleBarButton button) const;
    TitleBarButton hoveredTitleBarButton() const;

    bool isPinButtonVisible() const;
    bool isThemeButtonVisible() const;
    bool isMinimizeButtonVisible() const;
    bool isMaximizeButtonVisible() const;
    bool isCloseButtonVisible() const;

    void setPinButtonVisible(bool visible);
    void setThemeButtonVisible(bool visible);
    void setMinimizeButtonVisible(bool visible);
    void setMaximizeButtonVisible(bool visible);
    void setCloseButtonVisible(bool visible);

Q_SIGNALS:
    void windowTitleChanged(const QString& title);
    void alwaysOnTopChanged(bool on);
    void titleBarButtonVisibilityChanged(AntWindow::TitleBarButton button, bool visible);
    void pinButtonVisibleChanged(bool visible);
    void themeButtonVisibleChanged(bool visible);
    void minimizeButtonVisibleChanged(bool visible);
    void maximizeButtonVisibleChanged(bool visible);
    void closeButtonVisibleChanged(bool visible);
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
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

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
    void emitTitleBarButtonVisibleChanged(TitleBarButton button, bool visible);
    void applyManualSnap(const QPoint& globalPos);
    void applyNativeWindowFrame();
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
    int m_cornerRadius = 8;
    TitleBarButton m_hoveredButton = TitleBarButton::None;
    TitleBarButton m_pressedButton = TitleBarButton::None;
    QWidget* m_contentWidget = nullptr;
};
