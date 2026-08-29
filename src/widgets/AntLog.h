#pragma once

#include "core/QtAntDesignExport.h"

#include <QPalette>
#include <QMutex>
#include <QString>
#include <QTextCharFormat>
#include <QVector>
#include <QWidget>

class AntPlainTextEdit;
class QTimer;

class QT_ANT_DESIGN_EXPORT AntLog : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int maxEntries READ maxEntries WRITE setMaxEntries NOTIFY maxEntriesChanged)
    Q_PROPERTY(bool autoScroll READ autoScroll WRITE setAutoScroll NOTIFY autoScrollChanged)

public:
    enum Level { Debug, Info, Success, Warning, Error };

    explicit AntLog(QWidget* parent = nullptr);

    // Threading contract: mutating calls made on this widget's GUI/owner thread
    // are applied synchronously. Calls from any other thread are queued to that
    // thread in each caller's enqueue order. The pending queue is bounded; under
    // overload, excess commands are discarded and exposed through the
    // antLogDroppedCrossThreadCommandCount diagnostic property. All pending work
    // is discarded if the widget is destroyed. Getters are owner-thread only.

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

    enum class PendingCommandType
    {
        Append,
        Clear,
        SetMaxEntries,
        SetAutoScroll,
    };

    struct PendingCommand
    {
        PendingCommandType type = PendingCommandType::Append;
        Level level = Info;
        QString message;
        int integerValue = 0;
        bool boolValue = false;
    };

    void appendEntry(Level level, const QString& message);
    void enqueuePendingCommand(const PendingCommand& command);
    void drainPendingCommands();
    void trimEntries();
    void updateTheme();
    void rebuildDocument();
    bool insertEntry(const Entry& entry);
    void scheduleAppendFlush(int trimmedCount);
    void flushPendingAppendViewUpdates();
    QTextCharFormat formatForLevel(Level level) const;
    void updateDiagnostics(int trimmedCount = 0);
    bool isOwnerThread() const;

    AntPlainTextEdit* m_view = nullptr;
    QTimer* m_appendFlushTimer = nullptr;
    QVector<Entry> m_entries;
    mutable QMutex m_pendingCommandMutex;
    QVector<PendingCommand> m_pendingCommands;
    QVector<QTextCharFormat> m_levelFormats;
    QPalette m_cachedViewPalette;
    QPalette m_cachedViewportPalette;
    int m_maxEntries = 5000;
    int m_themeApplyCount = 0;
    int m_themeSkipCount = 0;
    int m_pendingAppendCount = 0;
    int m_pendingTrimmedCount = 0;
    int m_appendBatchStartCount = 0;
    int m_appendFlushCount = 0;
    int m_appendInsertCount = 0;
    quint64 m_droppedCrossThreadCommandCount = 0;
    int m_crossThreadDrainCount = 0;
    bool m_pendingCommandDrainScheduled = false;
    bool m_appendBatchActive = false;
    bool m_viewUpdatesEnabledBeforeAppendBatch = true;
    bool m_autoScroll = true;
};
