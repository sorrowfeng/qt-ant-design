#pragma once

#include "core/QtAntDesignExport.h"

#include <QFont>
#include <QPair>
#include <QPainterPath>
#include <QRectF>
#include <QSize>
#include <QVariant>
#include <QWidget>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QMoveEvent;
class QPaintEvent;
class QResizeEvent;

class AntRadioStyle;

class QT_ANT_DESIGN_EXPORT AntRadio : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool autoExclusive READ autoExclusive WRITE setAutoExclusive NOTIFY autoExclusiveChanged)
    Q_PROPERTY(bool buttonStyle READ isButtonStyle WRITE setButtonStyle NOTIFY buttonStyleChanged)

public:
    explicit AntRadio(QWidget* parent = nullptr);
    explicit AntRadio(const QString& text, QWidget* parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);

    QString text() const;
    void setText(const QString& text);

    QVariant value() const;
    void setValue(const QVariant& value);

    bool autoExclusive() const;
    void setAutoExclusive(bool autoExclusive);
    bool isButtonStyle() const;
    void setButtonStyle(bool buttonStyle);
    bool isHoveredState() const;
    bool isPressedState() const;
    void toggle();
    void click();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void checkedChanged(bool checked);
    void textChanged(const QString& text);
    void valueChanged(const QVariant& value);
    void autoExclusiveChanged(bool autoExclusive);
    void buttonStyleChanged(bool buttonStyle);
    void toggled(bool checked);
    void clicked(bool checked);

protected:
    void enterEvent(AntEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    friend class AntRadioStyle;

    struct LayoutCache
    {
        QSize widgetSize;
        QString text;
        bool buttonStyle = false;
        int fontSize = 14;
        int controlHeight = 32;
        int borderRadius = 6;
        QSize sizeHint;
        QSize minimumSizeHint;
        QRectF indicatorRect;
        QRectF textRect;
        QRectF buttonFrame;
        QPainterPath buttonPath;
        QPair<bool, bool> buttonEdges = {true, true};
        bool valid = false;
    };

    const LayoutCache& layoutCache() const;
    QPair<bool, bool> buttonSegmentEdges() const;
    QRectF indicatorRect() const;
    QRectF textRect() const;
    QRectF buttonFrameRect() const;
    const QPainterPath& buttonSegmentPath() const;
    QFont radioFont() const;
    QRect visualStateRect() const;
    void invalidateLayoutCache() const;
    void updateVisualStateRegion(const QRect& oldRect = QRect());
    void syncRadioPerfCounters() const;
    void toggleFromUser();
    void uncheckSiblings();
    QColor indicatorBorderColor() const;
    QColor indicatorBackgroundColor() const;

    bool m_checked = false;
    bool m_autoExclusive = true;
    bool m_buttonStyle = false;
    bool m_hovered = false;
    bool m_pressed = false;
    QString m_text;
    QVariant m_value;
    mutable LayoutCache m_layoutCache;
    mutable int m_layoutBuildCount = 0;
    mutable int m_sizeHintResolveCount = 0;
    mutable int m_buttonEdgeResolveCount = 0;
    int m_regionUpdateCount = 0;
};
