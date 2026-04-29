#pragma once

#include <QWidget>
#include <QVariant>

class QLineEdit;
class QFrame;
class QVBoxLayout;

class AntAutoComplete : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)
    Q_PROPERTY(Qt::CaseSensitivity caseSensitivity READ caseSensitivity WRITE setCaseSensitivity NOTIFY caseSensitivityChanged)
    Q_PROPERTY(int maxVisibleItems READ maxVisibleItems WRITE setMaxVisibleItems NOTIFY maxVisibleItemsChanged)

public:
    explicit AntAutoComplete(QWidget* parent = nullptr);
    ~AntAutoComplete() override;

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

    Qt::CaseSensitivity caseSensitivity() const;
    void setCaseSensitivity(Qt::CaseSensitivity cs);

    int maxVisibleItems() const;
    void setMaxVisibleItems(int n);

    QString text() const;
    void setText(const QString& text);

    QString addSuggestion(const QString& text, const QVariant& data = {});
    void removeSuggestion(int index);
    void clearSuggestions();
    int suggestionCount() const;

Q_SIGNALS:
    void textChanged(const QString& text);
    void suggestionClicked(const QString& text, const QVariant& data);
    void placeholderTextChanged(const QString& text);
    void caseSensitivityChanged(Qt::CaseSensitivity cs);
    void maxVisibleItemsChanged(int n);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void filterSuggestions();
    void showPopup();
    void hidePopup();
    void updatePopupGeometry();
    void selectHighlighted();

    struct Suggestion {
        QString text;
        QVariant data;
    };

    QLineEdit* m_lineEdit = nullptr;
    QFrame* m_popup = nullptr;
    QVBoxLayout* m_popupLayout = nullptr;
    QList<Suggestion> m_suggestions;
    QList<Suggestion> m_filtered;
    int m_highlightedIndex = -1;
    int m_maxVisibleItems = 8;
    Qt::CaseSensitivity m_caseSensitivity = Qt::CaseInsensitive;
    bool m_open = false;
    bool m_focused = false;
};
