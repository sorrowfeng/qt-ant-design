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

public:
    explicit AntWindow(QWidget* parent = nullptr);
    ~AntWindow() override = default;

    void setCentralWidget(QWidget* widget);

    static constexpr int TitleBarHeight = 40;
    static constexpr int TitleBarButtonWidth = 46;

    bool isMaximized() const;

Q_SIGNALS:
    void windowTitleChanged(const QString& title);
    void minimizeRequested();
    void maximizeRequested();
    void restoreRequested();
    void closeRequested();

protected:
    bool event(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    enum class TitleBarButton
    {
        None,
        Minimize,
        Maximize,
        Close,
    };

    bool isTitleBarArea(const QPoint& pos) const;
    bool isButtonArea(const QPoint& pos) const;
    TitleBarButton buttonAtPosition(const QPoint& pos) const;
    QRect titleBarRect() const;
    QRect minimizeButtonRect() const;
    QRect maximizeButtonRect() const;
    QRect closeButtonRect() const;
    void handleButtonClicked(TitleBarButton button);
    void syncTheme();
    void applyContentPalette(QWidget* widget);

    bool m_dragging = false;
    QPoint m_dragStartPosition;
    QPoint m_dragStartWindowPos;
    bool m_windowMaximized = false;
    TitleBarButton m_hoveredButton = TitleBarButton::None;
    QWidget* m_contentWidget = nullptr;
};
