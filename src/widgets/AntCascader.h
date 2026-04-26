#pragma once

#include <QList>
#include <QStringList>
#include <QVariant>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QFocusEvent;
class QFrame;
class QMouseEvent;
class QPaintEvent;
class QPropertyAnimation;

struct AntCascaderOption
{
    QVariant value;
    QString label;
    QVector<AntCascaderOption> children;
    bool disabled = false;
    bool isLeaf = true;
};

class AntCascader : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVector<AntCascaderOption> options READ options WRITE setOptions NOTIFY optionsChanged)
    Q_PROPERTY(QStringList value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(QString placeholder READ placeholder WRITE setPlaceholder NOTIFY placeholderChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(Ant::Size cascaderSize READ cascaderSize WRITE setCascaderSize NOTIFY cascaderSizeChanged)
    Q_PROPERTY(Ant::Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(Ant::Trigger expandTrigger READ expandTrigger WRITE setExpandTrigger NOTIFY expandTriggerChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
    Q_PROPERTY(qreal arrowRotation READ arrowRotation WRITE setArrowRotation)

public:
    explicit AntCascader(QWidget* parent = nullptr);
    ~AntCascader() override;

    QVector<AntCascaderOption> options() const;
    void setOptions(const QVector<AntCascaderOption>& options);

    QStringList value() const;
    void setValue(const QStringList& path);

    QString placeholder() const;
    void setPlaceholder(const QString& placeholder);

    bool allowClear() const;
    void setAllowClear(bool allowClear);

    Ant::Size cascaderSize() const;
    void setCascaderSize(Ant::Size size);

    Ant::Status status() const;
    void setStatus(Ant::Status status);

    Ant::Variant variant() const;
    void setVariant(Ant::Variant variant);

    Ant::Trigger expandTrigger() const;
    void setExpandTrigger(Ant::Trigger trigger);

    bool isOpen() const;
    void setOpen(bool open);

    qreal arrowRotation() const;
    void setArrowRotation(qreal rotation);

    QString displayText() const;

    bool isHoveredState() const;
    bool isPressedState() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void optionsChanged();
    void valueChanged(const QStringList& path);
    void placeholderChanged(const QString& placeholder);
    void allowClearChanged(bool allowClear);
    void cascaderSizeChanged(Ant::Size size);
    void statusChanged(Ant::Status status);
    void variantChanged(Ant::Variant variant);
    void expandTriggerChanged(Ant::Trigger trigger);
    void openChanged(bool open);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    friend class CascaderPopup;

    struct Metrics
    {
        int height = 32;
        int fontSize = 14;
        int radius = 6;
        int paddingX = 11;
        int arrowWidth = 28;
    };

    Metrics metrics() const;
    QRectF controlRect() const;
    QRectF clearButtonRect(const Metrics& m) const;
    QColor borderColor() const;
    QColor backgroundColor() const;
    bool canClear() const;
    void updateCursor();
    void animateArrow(bool open);
    void updatePopupPosition();
    void resolveLeafFlags(QVector<AntCascaderOption>& options);

    static const AntCascaderOption* findOptionByValue(const QVector<AntCascaderOption>& options, const QVariant& value);
    static bool buildLabelPath(const QVector<AntCascaderOption>& options, const QStringList& valuePath, QStringList& labelPath, int depth);
    static bool buildValuePathFromLabels(const QVector<AntCascaderOption>& options, const QStringList& labelPath, QStringList& valuePath, int depth);

    QVector<AntCascaderOption> m_options;
    QStringList m_value;
    QString m_placeholder;
    bool m_allowClear = false;
    Ant::Size m_cascaderSize = Ant::Size::Middle;
    Ant::Status m_status = Ant::Status::Normal;
    Ant::Variant m_variant = Ant::Variant::Outlined;
    Ant::Trigger m_expandTrigger = Ant::Trigger::Click;
    bool m_open = false;
    bool m_hovered = false;
    bool m_pressed = false;
    qreal m_arrowRotation = 0.0;

    QFrame* m_popup = nullptr;
    QPropertyAnimation* m_arrowAnimation = nullptr;
};
