#pragma once

#include "core/QtAntDesignExport.h"

#include <QFont>
#include <QPainterPath>
#include <QWidget>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class AntCheckBoxStyle;

class QT_ANT_DESIGN_EXPORT AntCheckBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    Q_PROPERTY(Qt::CheckState checkState READ checkState WRITE setCheckState NOTIFY checkStateChanged)
    Q_PROPERTY(bool indeterminate READ isIndeterminate WRITE setIndeterminate NOTIFY indeterminateChanged)
    Q_PROPERTY(bool tristate READ isTristate WRITE setTristate NOTIFY tristateChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit AntCheckBox(QWidget* parent = nullptr);
    explicit AntCheckBox(const QString& text, QWidget* parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked);
    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);

    bool isIndeterminate() const;
    void setIndeterminate(bool indeterminate);
    bool isTristate() const;
    void setTristate(bool tristate);
    void toggle();
    void click();

    QString text() const;
    void setText(const QString& text);
    bool isHoveredState() const;
    bool isPressedState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void checkedChanged(bool checked);
    void checkStateChanged(Qt::CheckState state);
    void indeterminateChanged(bool indeterminate);
    void tristateChanged(bool tristate);
    void textChanged(const QString& text);
    void stateChanged(int state);
    void toggled(bool checked);
    void clicked(bool checked);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    friend class AntCheckBoxStyle;

    struct LayoutData
    {
        QRectF indicatorRect;
        QRectF textRect;
        QPainterPath checkPath;
        QRectF indeterminateRect;
        QSize sizeHint;
    };

    const LayoutData& layoutData() const;
    QRect indicatorDirtyRect() const;
    void invalidateLayoutCache();
    void updateIndicatorRegion();
    void syncCheckBoxPerfCounters() const;
    QRectF indicatorRect() const;
    QColor indicatorBorderColor() const;
    QColor indicatorBackgroundColor() const;

    bool m_checked = false;
    bool m_indeterminate = false;
    bool m_tristate = false;
    bool m_hovered = false;
    bool m_pressed = false;
    QString m_text;
    quint64 m_layoutRevision = 1;
    mutable bool m_layoutCacheValid = false;
    mutable quint64 m_layoutCacheRevision = 0;
    mutable QSize m_layoutCacheWidgetSize;
    mutable QFont m_layoutCacheFont;
    mutable LayoutData m_cachedLayout;
    mutable int m_layoutCacheHitCount = 0;
    mutable int m_layoutBuildCount = 0;
    int m_indicatorScopedUpdateCount = 0;
};
