#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QRect>
#include <QSize>
#include <QWidget>

class QEnterEvent;
class QEvent;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

class QT_ANT_DESIGN_EXPORT AntNavItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit AntNavItem(const QString& text, QWidget* parent = nullptr);

    bool isActive() const;
    void setActive(bool active);

    QString text() const;
    void setText(const QString& text);

Q_SIGNALS:
    void activeChanged(bool active);
    void textChanged(const QString& text);
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void syncVisualState();
    void invalidatePaintCache() const;
    void ensurePaintCache() const;
    void syncNavPerfCounters() const;

    QLabel* m_label = nullptr;
    bool m_active = false;
    bool m_hovered = false;
    QColor m_cachedLabelColor;
    int m_cachedLabelWeight = -1;
    int m_visualStateApplyCount = 0;
    mutable bool m_paintCacheValid = false;
    mutable QSize m_cachedPaintSize;
    mutable bool m_cachedPaintActive = false;
    mutable bool m_cachedPaintHovered = false;
    mutable QColor m_cachedBackground;
    mutable QColor m_cachedIndicatorColor;
    mutable QRect m_cachedIndicatorRect;
    mutable int m_paintCacheBuildCount = 0;
};
