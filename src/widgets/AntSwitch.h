#pragma once

#include <QPropertyAnimation>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QEnterEvent;
class QKeyEvent;
class QMouseEvent;
class QPainter;
class QTimer;

class AntSwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(Ant::SwitchSize switchSize READ switchSize WRITE setSwitchSize NOTIFY switchSizeChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(QString checkedText READ checkedText WRITE setCheckedText NOTIFY checkedTextChanged)
    Q_PROPERTY(QString uncheckedText READ uncheckedText WRITE setUncheckedText NOTIFY uncheckedTextChanged)
    Q_PROPERTY(qreal handleProgress READ handleProgress WRITE setHandleProgress)
    Q_PROPERTY(qreal handleStretch READ handleStretch WRITE setHandleStretch)

public:
    explicit AntSwitch(QWidget* parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

    Ant::SwitchSize switchSize() const;
    void setSwitchSize(Ant::SwitchSize size);

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

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void checkedChanged(bool checked);
    void toggled(bool checked);
    void clicked(bool checked);
    void switchSizeChanged(Ant::SwitchSize size);
    void loadingChanged(bool loading);
    void checkedTextChanged(const QString& text);
    void uncheckedTextChanged(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

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

    Metrics metrics() const;
    QRectF trackRect() const;
    QRectF handleRect(const Metrics& metrics) const;
    QColor trackColor() const;
    void animateToChecked(bool checked);
    void animateStretch(qreal endValue);
    void updateGeometryFromState();
    void drawLoading(QPainter& painter, const QRectF& rect) const;

    bool m_checked = false;
    bool m_loading = false;
    bool m_hovered = false;
    bool m_pressed = false;
    Ant::SwitchSize m_switchSize = Ant::SwitchSize::Middle;
    QString m_checkedText;
    QString m_uncheckedText;
    qreal m_handleProgress = 0.0;
    qreal m_handleStretch = 0.0;
    int m_loadingAngle = 0;
    QPropertyAnimation* m_progressAnimation = nullptr;
    QPropertyAnimation* m_stretchAnimation = nullptr;
    QTimer* m_loadingTimer = nullptr;
};
