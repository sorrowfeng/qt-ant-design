#pragma once

#include "core/QtAntDesignExport.h"

#include <QObject>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QWidget>

class QAbstractScrollArea;

class QT_ANT_DESIGN_EXPORT AntAffix : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int offsetTop READ offsetTop WRITE setOffsetTop NOTIFY offsetTopChanged)
    Q_PROPERTY(int offsetBottom READ offsetBottom WRITE setOffsetBottom NOTIFY offsetBottomChanged)

public:
    explicit AntAffix(QObject* parent = nullptr);
    ~AntAffix() override;

    int offsetTop() const;
    void setOffsetTop(int offset);
    int offsetBottom() const;
    void setOffsetBottom(int offset);

    QWidget* affixedWidget() const;
    void setAffixedWidget(QWidget* widget);
    QWidget* scrollTarget() const;
    void setScrollTarget(QWidget* target);

    bool isAffixed() const;

Q_SIGNALS:
    void offsetTopChanged(int);
    void offsetBottomChanged(int);
    void affixStateChanged(bool affixed);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void findScrollContainer();
    void attachScrollMonitor();
    void detachScrollMonitor();
    void scheduleAffixCheck();
    void checkAffixState();
    void applyAffixed();
    void removeAffixed(bool notify = true);
    void updateAffixedGeometry();
    void resetCheckCache();
    void trackScrollTarget(QWidget* target);
    void trackScrollViewport(QWidget* viewport);

    int m_offsetTop = 0;
    int m_offsetBottom = 0;
    bool m_hasOffsetTop = false;
    bool m_hasOffsetBottom = false;
    QPointer<QWidget> m_scrollTarget;
    QPointer<QWidget> m_affixedWidget;
    QPointer<QWidget> m_scrollViewport;
    QPointer<QWidget> m_placeholder;
    bool m_isAffixed = false;
    QPoint m_originalPos;
    QPointer<QWidget> m_originalParent;
    QSize m_originalSize;
    bool m_checkQueued = false;
    quint64 m_checkGeneration = 0;
    quint64 m_scrollTargetGeneration = 0;
    quint64 m_scrollViewportGeneration = 0;
    QRect m_lastWidgetViewportRect;
    QSize m_lastViewportSize;
    bool m_lastShouldAffix = false;
    int m_affixCheckCount = 0;
    int m_effectiveAffixCheckCount = 0;
    QMetaObject::Connection m_verticalScrollConnection;
    QMetaObject::Connection m_horizontalScrollConnection;
    QMetaObject::Connection m_affixedWidgetDestroyedConnection;
    QMetaObject::Connection m_scrollTargetDestroyedConnection;
    QMetaObject::Connection m_scrollViewportDestroyedConnection;
};
