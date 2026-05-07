#pragma once

#include "core/QtAntDesignExport.h"

#include <QDoubleSpinBox>
#include <QStyle>

#include "core/AntTypes.h"

class QPropertyAnimation;
class QResizeEvent;
class QWidget;

class QT_ANT_DESIGN_EXPORT AntInputNumber : public QDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY(Ant::Size inputSize READ inputSize WRITE setInputSize NOTIFY inputSizeChanged)
    Q_PROPERTY(Ant::Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(bool controlsVisible READ controlsVisible WRITE setControlsVisible NOTIFY controlsVisibleChanged)
    Q_PROPERTY(qreal controlsProgress READ controlsProgress WRITE setControlsProgress)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)
    Q_PROPERTY(QString prefixText READ prefixText WRITE setPrefixText NOTIFY prefixTextChanged)
    Q_PROPERTY(QString suffixText READ suffixText WRITE setSuffixText NOTIFY suffixTextChanged)
    Q_PROPERTY(int precision READ precision WRITE setPrecision NOTIFY precisionChanged)

public:
    explicit AntInputNumber(QWidget* parent = nullptr);

    Ant::Size inputSize() const;
    void setInputSize(Ant::Size size);

    Ant::Status status() const;
    void setStatus(Ant::Status status);

    Ant::Variant variant() const;
    void setVariant(Ant::Variant variant);

    bool controlsVisible() const;
    void setControlsVisible(bool visible);
    qreal controlsProgress() const;
    void setControlsProgress(qreal progress);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    QString prefixText() const;
    void setPrefixText(const QString& text);
    QString suffixText() const;
    void setSuffixText(const QString& text);
    QString addonAfterText() const;
    void setAddonAfterText(const QString& text);
    int precision() const;
    void setPrecision(int decimals);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    bool isHoveredState() const;
    QStyle::SubControl activeSubControl() const;
    bool isStepPressed() const;
    QAbstractSpinBox::StepEnabled stepEnabledFlags() const;

Q_SIGNALS:
    void inputSizeChanged(Ant::Size size);
    void statusChanged(Ant::Status status);
    void variantChanged(Ant::Variant variant);
    void controlsVisibleChanged(bool visible);
    void prefixTextChanged(const QString& text);
    void suffixTextChanged(const QString& text);
    void precisionChanged(int decimals);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

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
    bool shouldShowControls() const;
    void animateControls(bool visible);
    int controlsInsetWidth() const;
    void updateControlsOverlayGeometry();

    Ant::Size m_inputSize = Ant::Size::Middle;
    Ant::Status m_status = Ant::Status::Normal;
    Ant::Variant m_variant = Ant::Variant::Outlined;
    bool m_controlsVisible = true;
    qreal m_controlsProgress = 0.0;
    bool m_hovered = false;
    bool m_stepPressed = false;
    QStyle::SubControl m_activeSubControl = QStyle::SC_None;
    QString m_addonAfterText;
    QPropertyAnimation* m_controlsAnimation = nullptr;
    QWidget* m_controlsOverlay = nullptr;
};
