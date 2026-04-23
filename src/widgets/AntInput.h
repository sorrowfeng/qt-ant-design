#pragma once

#include <QIcon>
#include <QRect>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

#include "core/AntTypes.h"

class QHBoxLayout;
class QLabel;

class AntInput : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::InputSize inputSize READ inputSize WRITE setInputSize NOTIFY inputSizeChanged)
    Q_PROPERTY(Ant::InputStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool passwordMode READ isPasswordMode WRITE setPasswordMode NOTIFY passwordModeChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit AntInput(QWidget* parent = nullptr);

    QLineEdit* lineEdit() const;
    QString text() const;
    void setText(const QString& text);
    void setPlaceholderText(const QString& text);

    Ant::InputSize inputSize() const;
    void setInputSize(Ant::InputSize size);
    Ant::InputStatus status() const;
    void setStatus(Ant::InputStatus status);
    void setError(bool error);

    bool allowClear() const;
    void setAllowClear(bool allowClear);
    bool isPasswordMode() const;
    void setPasswordMode(bool passwordMode);

    void setAddonBefore(const QString& text);
    void setAddonAfter(const QString& text);
    void setPrefixIcon(const QIcon& icon);
    void setSuffixIcon(const QIcon& icon);
    void setPrefixWidget(QWidget* widget);
    void setSuffixWidget(QWidget* widget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    bool isInputFocused() const;
    bool isInputHovered() const;
    QRect addonBeforeRect() const;
    QRect addonAfterRect() const;

Q_SIGNALS:
    void textChanged(const QString& text);
    void inputSizeChanged(Ant::InputSize size);
    void statusChanged(Ant::InputStatus status);
    void allowClearChanged(bool allowClear);
    void passwordModeChanged(bool passwordMode);
    void focusIn();
    void focusOut();

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    struct Metrics
    {
        int height = 32;
        int fontSize = 14;
        int paddingX = 11;
        int radius = 6;
        int iconSize = 16;
    };

    Metrics metrics() const;
    void rebuildLayout();
    void updateButtonVisibility();
    void updateVisualState();
    QColor borderColor() const;

    QHBoxLayout* m_layout = nullptr;
    QLabel* m_addonBefore = nullptr;
    QLabel* m_addonAfter = nullptr;
    QLabel* m_prefixIconLabel = nullptr;
    QLabel* m_suffixIconLabel = nullptr;
    QWidget* m_prefixWidget = nullptr;
    QWidget* m_suffixWidget = nullptr;
    QLineEdit* m_lineEdit = nullptr;
    QToolButton* m_clearButton = nullptr;
    QToolButton* m_passwordButton = nullptr;

    Ant::InputSize m_inputSize = Ant::InputSize::Middle;
    Ant::InputStatus m_status = Ant::InputStatus::Normal;
    bool m_allowClear = false;
    bool m_passwordMode = false;
    bool m_passwordVisible = false;
    bool m_hovered = false;
    bool m_focused = false;
};
