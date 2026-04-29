#pragma once

#include <QVector>
#include <QWidget>

class QPlainTextEdit;

class AntLog : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int maxEntries READ maxEntries WRITE setMaxEntries NOTIFY maxEntriesChanged)
    Q_PROPERTY(bool autoScroll READ autoScroll WRITE setAutoScroll NOTIFY autoScrollChanged)

public:
    enum Level { Debug, Info, Success, Warning, Error };

    explicit AntLog(QWidget* parent = nullptr);

    int maxEntries() const;
    void setMaxEntries(int n);

    bool autoScroll() const;
    void setAutoScroll(bool enabled);

    void log(Level level, const QString& message);
    void debug(const QString& message);
    void info(const QString& message);
    void success(const QString& message);
    void warning(const QString& message);
    void error(const QString& message);

    void clear();

Q_SIGNALS:
    void maxEntriesChanged(int n);
    void autoScrollChanged(bool enabled);

private:
    struct Entry
    {
        QString timestamp;
        Level level = Info;
        QString message;
    };

    void appendEntry(Level level, const QString& message);
    void trimEntries();
    void updateTheme();
    void rebuildDocument();
    void insertEntry(const Entry& entry);

    QPlainTextEdit* m_view = nullptr;
    QVector<Entry> m_entries;
    int m_maxEntries = 5000;
    bool m_autoScroll = true;
};
