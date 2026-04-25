#pragma once

#include <QScrollArea>

class AntScrollBar;

class AntScrollArea : public QScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool autoHideScrollBar READ autoHideScrollBar WRITE setAutoHideScrollBar NOTIFY autoHideScrollBarChanged)
    Q_PROPERTY(bool enableGesture READ isGestureEnabled WRITE setEnableGesture NOTIFY enableGestureChanged)

public:
    explicit AntScrollArea(QWidget* parent = nullptr);

    bool autoHideScrollBar() const;
    void setAutoHideScrollBar(bool autoHide);

    bool isGestureEnabled() const;
    void setEnableGesture(bool enable);

Q_SIGNALS:
    void autoHideScrollBarChanged(bool autoHide);
    void enableGestureChanged(bool enable);

private:
    AntScrollBar* m_vScrollBar = nullptr;
    AntScrollBar* m_hScrollBar = nullptr;
    bool m_autoHideScrollBar = true;
    bool m_enableGesture = true;
};
