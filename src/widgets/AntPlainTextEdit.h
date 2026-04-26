#pragma once

#include <QPlainTextEdit>

#include "core/AntTypes.h"

class AntPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)

public:
    explicit AntPlainTextEdit(QWidget* parent = nullptr);
    explicit AntPlainTextEdit(const QString& text, QWidget* parent = nullptr);

    Ant::Variant variant() const;
    void setVariant(Ant::Variant variant);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

Q_SIGNALS:
    void variantChanged(Ant::Variant variant);
    void placeholderTextChanged(const QString& text);

protected:
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    Ant::Variant m_variant = Ant::Variant::Outlined;
    QString m_placeholderText;
    bool m_focused = false;
};
