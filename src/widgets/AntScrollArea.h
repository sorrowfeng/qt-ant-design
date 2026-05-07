#pragma once

#include "core/QtAntDesignExport.h"

#include <QScrollArea>

class AntScrollBar;

class QT_ANT_DESIGN_EXPORT AntScrollArea : public QScrollArea
{
    Q_OBJECT
    Q_PROPERTY(bool autoHideScrollBar READ autoHideScrollBar WRITE setAutoHideScrollBar NOTIFY autoHideScrollBarChanged)
    Q_PROPERTY(bool enableGesture READ isGestureEnabled WRITE setEnableGesture NOTIFY enableGestureChanged)

public:
    explicit AntScrollArea(QWidget* parent = nullptr);

    void setWidget(QWidget* widget);

    bool autoHideScrollBar() const;
    void setAutoHideScrollBar(bool autoHide);

    bool isGestureEnabled() const;
    void setEnableGesture(bool enable);

Q_SIGNALS:
    void autoHideScrollBarChanged(bool autoHide);
    void enableGestureChanged(bool enable);

private:
    void updateTheme();

    AntScrollBar* m_vScrollBar = nullptr;
    AntScrollBar* m_hScrollBar = nullptr;
    bool m_autoHideScrollBar = true;
    bool m_enableGesture = true;
};
