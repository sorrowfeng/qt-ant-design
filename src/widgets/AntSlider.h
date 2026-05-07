#pragma once

#include "core/QtAntDesignExport.h"

#include <QPropertyAnimation>
#include <QMap>
#include <QWidget>

class QEnterEvent;
class QEvent;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QPaintEvent;

class QT_ANT_DESIGN_EXPORT AntSlider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum NOTIFY rangeChanged)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY rangeChanged)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int sliderPosition READ sliderPosition WRITE setSliderPosition NOTIFY sliderMoved)
    Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep NOTIFY singleStepChanged)
    Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep NOTIFY pageStepChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(bool reverse READ isReverse WRITE setReverse NOTIFY reverseChanged)
    Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking NOTIFY trackingChanged)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance NOTIFY reverseChanged)
    Q_PROPERTY(bool dots READ dots WRITE setDots NOTIFY dotsChanged)
    Q_PROPERTY(bool included READ included WRITE setIncluded NOTIFY includedChanged)
    Q_PROPERTY(bool keyboard READ keyboard WRITE setKeyboard NOTIFY keyboardChanged)
    Q_PROPERTY(bool range READ isRangeMode WRITE setRangeMode NOTIFY rangeModeChanged)
    Q_PROPERTY(qreal handleScale READ handleScale WRITE setHandleScale)
    Q_PROPERTY(qreal focusProgress READ focusProgress WRITE setFocusProgress)

public:
    explicit AntSlider(QWidget* parent = nullptr);
    explicit AntSlider(Qt::Orientation orientation, QWidget* parent = nullptr);

    int minimum() const;
    void setMinimum(int minimum);

    int maximum() const;
    void setMaximum(int maximum);

    void setRange(int minimum, int maximum);

    int value() const;
    void setValue(int value);
    int sliderPosition() const;
    void setSliderPosition(int position);

    int singleStep() const;
    void setSingleStep(int step);
    int pageStep() const;
    void setPageStep(int step);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orientation);

    bool isReverse() const;
    void setReverse(bool reverse);
    bool invertedAppearance() const;
    void setInvertedAppearance(bool inverted);
    bool hasTracking() const;
    void setTracking(bool tracking);

    bool dots() const;
    void setDots(bool dots);

    bool included() const;
    void setIncluded(bool included);

    bool keyboard() const;
    void setKeyboard(bool enabled);

    bool isRangeMode() const;
    void setRangeMode(bool rangeMode);
    int rangeStart() const;
    int rangeEnd() const;
    void setRangeValues(int start, int end);

    QMap<int, QString> marks() const;
    void setMarks(const QMap<int, QString>& marks);

    qreal handleScale() const;
    void setHandleScale(qreal scale);

    qreal focusProgress() const;
    void setFocusProgress(qreal progress);
    bool isHoveredState() const;
    bool isPressedState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void valueChanged(int value);
    void sliderMoved(int value);
    void sliderPressed();
    void sliderReleased();
    void changeComplete(int value);
    void rangeChanged(int minimum, int maximum);
    void singleStepChanged(int step);
    void pageStepChanged(int step);
    void orientationChanged(Qt::Orientation orientation);
    void reverseChanged(bool reverse);
    void trackingChanged(bool tracking);
    void dotsChanged(bool dots);
    void includedChanged(bool included);
    void keyboardChanged(bool enabled);
    void rangeModeChanged(bool rangeMode);
    void rangeValuesChanged(int start, int end);
    void marksChanged();

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    struct Metrics
    {
        int controlSize = 10;
        int railSize = 4;
        int handleSize = 10;
        int handleSizeHover = 12;
        int dotSize = 8;
        int margin = 8;
    };

    Metrics metrics() const;
    QRectF grooveRect(const Metrics& metrics) const;
    QRectF trackRect(const Metrics& metrics) const;
    QRectF handleRect(const Metrics& metrics) const;
    QRectF handleRectForValue(const Metrics& metrics, int value) const;
    qreal valueRatio() const;
    qreal ratioForValue(int value) const;
    int valueFromPosition(const QPointF& pos) const;
    int normalizeValue(int value) const;
    int steppedValue(int value) const;
    void setValueFromUser(int value, bool finalChange = false);
    void animateHandle(qreal scale);
    void animateFocus(qreal progress);
    int activeDisplayValue() const;
    QPointF activeHandleCenter() const;
    void updateValueBubble();
    void hideValueBubble();
    void updateCursor();

    int m_minimum = 0;
    int m_maximum = 100;
    int m_value = 0;
    int m_rangeStart = 0;
    int m_rangeEnd = 100;
    int m_singleStep = 1;
    int m_pageStep = 10;
    Qt::Orientation m_orientation = Qt::Horizontal;
    bool m_reverse = false;
    bool m_tracking = true;
    bool m_dots = false;
    bool m_included = true;
    bool m_keyboard = true;
    bool m_rangeMode = false;
    bool m_hovered = false;
    bool m_pressed = false;
    int m_activeRangeHandle = 1;
    QMap<int, QString> m_marks;
    qreal m_handleScale = 1.0;
    qreal m_focusProgress = 0.0;
    QPropertyAnimation* m_handleAnimation = nullptr;
    QPropertyAnimation* m_focusAnimation = nullptr;
    QWidget* m_valueBubble = nullptr;
};
