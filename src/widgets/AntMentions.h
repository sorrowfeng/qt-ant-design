#pragma once

#include "core/QtAntDesignExport.h"

#include <QStringList>
#include <QVector>
#include <QWidget>

class AntMentionItem;
class QLineEdit;
class QFrame;
class QPaintEvent;
class QTimer;
class QVBoxLayout;

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
    bool eventFilter(QObject* watched, QEvent* event) override;

    void applyLineEditTheme();
    void scheduleSuggestionRefresh();
    void refreshSuggestions();
    QString currentFilterText(bool* ok = nullptr) const;
    QStringList filteredSuggestions(const QString& filter);
    void syncPopupRows(const QStringList& matched);
    void ensurePopupRows(int count);
    void updatePopupGeometry(int visibleCount);
    void setHighlightedIndex(int index);
    void updateHighlightedRows(int previous, int current);
    void showPopup();
    void hidePopup();
    void selectSuggestion(int index);
    void invalidateSuggestionCache();
    void syncMentionsPerfCounters() const;

    QLineEdit* m_lineEdit = nullptr;
    QFrame* m_popup = nullptr;
    QVBoxLayout* m_popupLayout = nullptr;
    QTimer* m_filterTimer = nullptr;
    QVector<AntMentionItem*> m_itemWidgets;
    QStringList m_suggestions;
    QStringList m_filteredSuggestions;
    QString m_cachedFilter;
    QString m_placeholderText;
    QString m_prefix = QStringLiteral("@");
    int m_rows = 1;
    int m_prefixPos = -1;
    int m_suggestionsRevision = 0;
    int m_cachedFilterRevision = -1;
    int m_visibleSuggestionCount = 0;
    int m_highlightedIndex = -1;
    bool m_open = false;
    QRect m_lastPopupGeometry;
    mutable int m_filterResolveCount = 0;
    int m_popupRowBuildCount = 0;
    int m_rowTextApplyCount = 0;
    int m_popupGeometryApplyCount = 0;
    int m_highlightedRowUpdateCount = 0;
    int m_refreshCount = 0;
};
