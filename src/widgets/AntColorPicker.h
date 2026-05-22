#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QFrame;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class QT_ANT_DESIGN_EXPORT AntColorPicker : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor currentColor READ currentColor WRITE setCurrentColor NOTIFY currentColorChanged)
    Q_PROPERTY(bool showText READ showText WRITE setShowText NOTIFY showTextChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntColorPicker(QWidget* parent = nullptr);
    explicit AntColorPicker(const QColor& initial, QWidget* parent = nullptr);

    QColor currentColor() const;
    void setCurrentColor(const QColor& color);

    bool showText() const;
    void setShowText(bool showText);

    bool isOpen() const;
    void setOpen(bool open);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void colorSelected(const QColor& color);
    void currentColorChanged(const QColor& color);
    void showTextChanged(bool showText);
    void openChanged(bool open);

private:
    struct TriggerSizeHintCache
    {
        bool valid = false;
        bool showText = false;
        int fontSize = 0;
        QString fontKey;
        QString displayText;
        QSize sizeHint;
        QSize minimumSizeHint;
    };

    struct TriggerLayoutCache
    {
        bool valid = false;
        bool showText = false;
        int fontSize = 0;
        QString fontKey;
        QString displayText;
        QSize widgetSize;
        QRectF frameRect;
        QRect colorBlockRect;
        QRect textRect;
    };

    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void openEditor();
    void updatePopupGeometry();
    QRect colorBlockRect() const;
    QString displayColorText() const;
    const TriggerSizeHintCache& cachedTriggerSizeHints() const;
    const TriggerLayoutCache& cachedTriggerLayout() const;
    void invalidateTriggerCaches() const;
    void requestTriggerUpdate(const QString& mode, const QRect& dirty = QRect());
    void syncTriggerPerfCounters() const;

    bool m_showText = false;
    bool m_open = false;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_appEventFilterInstalled = false;
    bool m_popupOpensAbove = false;
    int m_popupMotionSerial = 0;
    QColor m_currentColor = Qt::white;
    QFrame* m_popup = nullptr;
    mutable TriggerSizeHintCache m_triggerSizeHintCache;
    mutable TriggerLayoutCache m_triggerLayoutCache;
    mutable int m_triggerSizeHintBuildCount = 0;
    mutable int m_triggerSizeHintCacheHitCount = 0;
    mutable int m_triggerLayoutBuildCount = 0;
    mutable int m_triggerLayoutCacheHitCount = 0;
    int m_triggerRegionUpdateCount = 0;
    QString m_lastTriggerUpdateMode;
};
