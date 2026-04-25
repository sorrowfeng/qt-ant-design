#pragma once

#include <QWidget>
#include <QVariant>

class QLineEdit;
class QFrame;

class AntMentions : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY prefixChanged)

public:
    explicit AntMentions(QWidget* parent = nullptr);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    QString prefix() const;
    void setPrefix(const QString& prefix);

    QString text() const;

    void setSuggestions(const QStringList& items);
    void addSuggestion(const QString& text);

Q_SIGNALS:
    void textChanged(const QString& text);
    void mentionSelected(const QString& text);
    void placeholderTextChanged(const QString& text);
    void prefixChanged(const QString& prefix);

private:
    void checkForPrefix();
    void showPopup();
    void hidePopup();
    void selectSuggestion(int index);

    QLineEdit* m_lineEdit = nullptr;
    QFrame* m_popup = nullptr;
    QStringList m_suggestions;
    QString m_placeholderText;
    QString m_prefix = QStringLiteral("@");
    int m_prefixPos = -1;
    bool m_open = false;
};
