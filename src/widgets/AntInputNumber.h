#pragma once

#include <QDoubleSpinBox>
#include <QStyle>

#include "core/AntTypes.h"

class AntInputNumber : public QDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY(Ant::InputSize inputSize READ inputSize WRITE setInputSize NOTIFY inputSizeChanged)
    Q_PROPERTY(Ant::InputStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::InputNumberVariant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(bool controlsVisible READ controlsVisible WRITE setControlsVisible NOTIFY controlsVisibleChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)

public:
    explicit AntInputNumber(QWidget* parent = nullptr);

    Ant::InputSize inputSize() const;
    void setInputSize(Ant::InputSize size);

    Ant::InputStatus status() const;
    void setStatus(Ant::InputStatus status);

    Ant::InputNumberVariant variant() const;
    void setVariant(Ant::InputNumberVariant variant);

    bool controlsVisible() const;
    void setControlsVisible(bool visible);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    void setPrefixText(const QString& text);
    void setSuffixText(const QString& text);
    void setPrecision(int decimals);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    bool isHoveredState() const;
    QStyle::SubControl activeSubControl() const;
    bool isStepPressed() const;
    QAbstractSpinBox::StepEnabled stepEnabledFlags() const;

Q_SIGNALS:
    void inputSizeChanged(Ant::InputSize size);
    void statusChanged(Ant::InputStatus status);
    void variantChanged(Ant::InputNumberVariant variant);
    void controlsVisibleChanged(bool visible);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    struct Metrics
    {
        int height = 32;
        int fontSize = 14;
        int radius = 6;
        int paddingX = 11;
    };

    Metrics metrics() const;
    void updateEditStyle();
    QStyle::SubControl hitSubControl(const QPoint& pos) const;

    Ant::InputSize m_inputSize = Ant::InputSize::Middle;
    Ant::InputStatus m_status = Ant::InputStatus::Normal;
    Ant::InputNumberVariant m_variant = Ant::InputNumberVariant::Outlined;
    bool m_controlsVisible = true;
    bool m_hovered = false;
    bool m_stepPressed = false;
    QStyle::SubControl m_activeSubControl = QStyle::SC_None;
};
