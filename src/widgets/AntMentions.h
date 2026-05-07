#pragma once

#include "core/QtAntDesignExport.h"

#include <QWidget>
#include <QVariant>

class QLineEdit;
class QFrame;
class QPaintEvent;

class QT_ANT_DESIGN_EXPORT AntMentions : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY prefixChanged)
    Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged)

public:
    explicit AntMentions(QWidget* parent = nullptr);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    QString prefix() const;
    void setPrefix(const QString& prefix);

    int rows() const;
    void setRows(int rows);

    QString text() const;

    void setSuggestions(const QStringList& items);
    void addSuggestion(const QString& text);

    QSize sizeHint() const override;

Q_SIGNALS:
    void textChanged(const QString& text);
    void mentionSelected(const QString& text);
    void placeholderTextChanged(const QString& text);
    void prefixChanged(const QString& prefix);
    void rowsChanged(int rows);

private:
    void paintEvent(QPaintEvent* event) override;

    void checkForPrefix();
    void showPopup();
    void hidePopup();
    void selectSuggestion(int index);

    QLineEdit* m_lineEdit = nullptr;
    QFrame* m_popup = nullptr;
    QStringList m_suggestions;
    QString m_placeholderText;
    QString m_prefix = QStringLiteral("@");
    int m_rows = 1;
    int m_prefixPos = -1;
    bool m_open = false;
};
