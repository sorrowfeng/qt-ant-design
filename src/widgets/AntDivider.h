#pragma once

#include <QWidget>

#include "core/AntTypes.h"

class QPaintEvent;
class QPainter;

class AntDivider : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(bool plain READ isPlain WRITE setPlain NOTIFY plainChanged)
    Q_PROPERTY(Ant::Orientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(Ant::DividerTitlePlacement titlePlacement READ titlePlacement WRITE setTitlePlacement NOTIFY titlePlacementChanged)
    Q_PROPERTY(Ant::DividerVariant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(Ant::Size dividerSize READ dividerSize WRITE setDividerSize NOTIFY dividerSizeChanged)

public:
    explicit AntDivider(QWidget* parent = nullptr);
    explicit AntDivider(const QString& text, QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    bool isPlain() const;
    void setPlain(bool plain);

    Ant::Orientation orientation() const;
    void setOrientation(Ant::Orientation orientation);

    Ant::DividerTitlePlacement titlePlacement() const;
    void setTitlePlacement(Ant::DividerTitlePlacement placement);

    Ant::DividerVariant variant() const;
    void setVariant(Ant::DividerVariant variant);

    Ant::Size dividerSize() const;
    void setDividerSize(Ant::Size size);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void textChanged(const QString& text);
    void plainChanged(bool plain);
    void orientationChanged(Ant::Orientation orientation);
    void titlePlacementChanged(Ant::DividerTitlePlacement placement);
    void variantChanged(Ant::DividerVariant variant);
    void dividerSizeChanged(Ant::Size size);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int horizontalMargin() const;
    int textFontSize() const;
    QPen dividerPen() const;
    void drawHorizontal(QPainter& painter);
    void drawVertical(QPainter& painter);

    QString m_text;
    bool m_plain = true;
    Ant::Orientation m_orientation = Ant::Orientation::Horizontal;
    Ant::DividerTitlePlacement m_titlePlacement = Ant::DividerTitlePlacement::Center;
    Ant::DividerVariant m_variant = Ant::DividerVariant::Solid;
    Ant::Size m_dividerSize = Ant::Size::Large;
};
