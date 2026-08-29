#pragma once

#include "core/QtAntDesignExport.h"

#include <QPropertyAnimation>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QWidget>

#include "core/AntSpinner.h"
#include "core/AntTypes.h"

class QEvent;
class QEnterEvent;
class QHideEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QResizeEvent;
class QShowEvent;
class AntSwitchStyle;

class QT_ANT_DESIGN_EXPORT AntSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(Ant::Size size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(Ant::Size switchSize READ switchSize WRITE setSwitchSize NOTIFY switchSizeChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString checkedText READ checkedText WRITE setCheckedText NOTIFY checkedTextChanged)
    Q_PROPERTY(QString uncheckedText READ uncheckedText WRITE setUncheckedText NOTIFY uncheckedTextChanged)
    Q_PROPERTY(qreal handleProgress READ handleProgress WRITE setHandleProgress)
    Q_PROPERTY(qreal handleStretch READ handleStretch WRITE setHandleStretch)
    friend class AntSwitchStyle;

public:
    explicit AntSwitch(QWidget* parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

    Ant::Size size() const;
    void setSize(Ant::Size size);
    // Legacy alias - prefer size()/setSize().
    Ant::Size switchSize() const { return size(); }
    void setSwitchSize(Ant::Size size) { setSize(size); }

    bool isLoading() const;
    void setLoading(bool loading);

    QString checkedText() const;
    void setCheckedText(const QString& text);

    QString uncheckedText() const;
    void setUncheckedText(const QString& text);

    qreal handleProgress() const;
    void setHandleProgress(qreal progress);

    qreal handleStretch() const;
    void setHandleStretch(qreal stretch);
    bool isHoveredState() const;
    bool isPressedState() const;
    int loadingAngle() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    // Canonical state-change signal - prefer this over the legacy alias below.
    void checkedChanged(bool checked);
    // User-gesture event (fires on click/keyboard activation, not on programmatic set).
    void clicked(bool checked);
    // Deprecated - use checkedChanged() instead.
    void toggled(bool checked);
    void sizeChanged(Ant::Size size);
    void switchSizeChanged(Ant::Size size);
    void loadingChanged(bool loading);
    void checkedTextChanged(const QString& text);
    void uncheckedTextChanged(const QString& text);

protected:
    void enterEvent(AntEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Metrics
    {
        int trackHeight = 22;
        int trackMinWidth = 44;
        int trackPadding = 2;
        int handleSize = 18;
        int fontSize = 12;
        int innerMinMargin = 9;
        int innerMaxMargin = 24;
    };

    struct LayoutCache
    {
        QSize widgetSize;
        Metrics metrics;
        Ant::Size switchSize = Ant::Size::Middle;
        QString checkedText;
        QString uncheckedText;
        qreal handleProgress = 0.0;
        qreal handleStretch = 0.0;
        QSize sizeHint;
        QSize minimumSizeHint;
        QRectF trackRect;
        QRectF handleRect;
        bool valid = false;
    };

    Metrics metrics() const;
    const LayoutCache& layoutCache() const;
    QRectF handleRectForState(const LayoutCache& cache, qreal progress, qreal stretch) const;
    QRect switchTrackDirtyRect() const;
    QRect switchHandleDirtyRect(qreal oldProgress, qreal oldStretch) const;
    QRect switchLoadingDirtyRect() const;
    void updateSwitchRegion(const QRect& dirty, const QString& mode);
    void invalidateLayoutCache() const;
    void updateLoadingTimerState();
    void syncSwitchPerfCounters() const;
    void animateToChecked(bool checked);
    void animateStretch(qreal endValue);
    void updateGeometryFromState(bool notifyGeometry = true);

    bool m_checked = false;
    bool m_loading = false;
    bool m_hovered = false;
    bool m_pressed = false;
    Ant::Size m_size = Ant::Size::Middle;
    QString m_checkedText;
    QString m_uncheckedText;
    qreal m_handleProgress = 0.0;
    qreal m_handleStretch = 0.0;
    AntSpinner m_loadingSpinner;
    QPropertyAnimation* m_progressAnimation = nullptr;
    QPropertyAnimation* m_stretchAnimation = nullptr;
    mutable LayoutCache m_layoutCache;
    mutable int m_layoutBuildCount = 0;
    mutable int m_metricsResolveCount = 0;
    mutable int m_sizeHintResolveCount = 0;
    int m_regionUpdateCount = 0;
    int m_handleRegionUpdateCount = 0;
    int m_loadingRegionUpdateCount = 0;
};
