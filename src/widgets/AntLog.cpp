#include "AntLog.h"

#include <QFont>
#include <QPalette>
#include <QMutexLocker>
#include <QPointer>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

#include "AntPlainTextEdit.h"
#include "AntScrollBar.h"
#include "core/AntTheme.h"

namespace
{
constexpr int kLevelCount = 5;
constexpr int kAppendFlushThreshold = 128;
constexpr int kMaxPendingCrossThreadCommands = 1024;
constexpr int kPendingCommandDrainBatchSize = 256;

QColor levelColor(AntLog::Level level)
{
    const auto& token = antTheme->tokens();
    switch (level)
    {
    case AntLog::Debug:
        return token.colorTextPlaceholder;
    case AntLog::Info:
        return token.colorText;
    case AntLog::Success:
        return token.colorSuccess;
    case AntLog::Warning:
        return token.colorWarning;
    case AntLog::Error:
        return token.colorError;
    }
    return token.colorText;
}

QString levelTag(AntLog::Level level)
{
    switch (level)
    {
    case AntLog::Debug:   return QStringLiteral("DEBUG");
    case AntLog::Info:    return QStringLiteral("INFO");
    case AntLog::Success: return QStringLiteral(" OK ");
    case AntLog::Warning: return QStringLiteral("WARN");
    case AntLog::Error:   return QStringLiteral("ERROR");
    }
    return QStringLiteral("INFO");
}

int levelIndex(AntLog::Level level)
{
    const int index = static_cast<int>(level);
    return index >= 0 && index < kLevelCount ? index : static_cast<int>(AntLog::Info);
}

} // namespace

AntLog::AntLog(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_view = new AntPlainTextEdit(this);
    m_view->setReadOnly(true);
    m_view->setUndoRedoEnabled(false);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setVerticalScrollBar(new AntScrollBar(Qt::Vertical, m_view));
    m_view->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal, m_view));

    QFont mono(QStringLiteral("Consolas"), 12);
    mono.setStyleHint(QFont::Monospace);
    m_view->setFont(mono);
    m_view->document()->setMaximumBlockCount(m_maxEntries);

    layout->addWidget(m_view);

    m_appendFlushTimer = new QTimer(this);
    m_appendFlushTimer->setSingleShot(true);
    m_appendFlushTimer->setInterval(0);
    connect(m_appendFlushTimer, &QTimer::timeout, this, &AntLog::flushPendingAppendViewUpdates);

    m_levelFormats.resize(kLevelCount);
    updateTheme();
    updateDiagnostics();
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        flushPendingAppendViewUpdates();
        updateTheme();
        rebuildDocument();
    });
}

int AntLog::maxEntries() const { return m_maxEntries; }
void AntLog::setMaxEntries(int n)
{
    if (!isOwnerThread())
    {
        PendingCommand command;
        command.type = PendingCommandType::SetMaxEntries;
        command.integerValue = n;
        enqueuePendingCommand(command);
        return;
    }

    n = qMax(1, n);
    if (m_maxEntries == n) return;
    flushPendingAppendViewUpdates();
    m_maxEntries = n;
    m_view->document()->setMaximumBlockCount(m_maxEntries);
    const int sizeBefore = m_entries.size();
    trimEntries();
    rebuildDocument();
    updateDiagnostics(sizeBefore - m_entries.size());
    Q_EMIT maxEntriesChanged(m_maxEntries);
}

bool AntLog::autoScroll() const { return m_autoScroll; }
void AntLog::setAutoScroll(bool enabled)
{
    if (!isOwnerThread())
    {
        PendingCommand command;
        command.type = PendingCommandType::SetAutoScroll;
        command.boolValue = enabled;
        enqueuePendingCommand(command);
        return;
    }

    if (m_autoScroll == enabled) return;
    m_autoScroll = enabled;
    Q_EMIT autoScrollChanged(m_autoScroll);
}

void AntLog::debug(const QString& message)   { log(Debug, message); }
void AntLog::info(const QString& message)    { log(Info, message); }
void AntLog::success(const QString& message) { log(Success, message); }
void AntLog::warning(const QString& message) { log(Warning, message); }
void AntLog::error(const QString& message)   { log(Error, message); }

void AntLog::log(Level level, const QString& message)
{
    if (!isOwnerThread())
    {
        PendingCommand command;
        command.type = PendingCommandType::Append;
        command.level = level;
        command.message = message;
        enqueuePendingCommand(command);
        return;
    }

    appendEntry(level, message);
}

void AntLog::clear()
{
    if (!isOwnerThread())
    {
        PendingCommand command;
        command.type = PendingCommandType::Clear;
        enqueuePendingCommand(command);
        return;
    }

    QPointer<AntLog> guard(this);
    flushPendingAppendViewUpdates();
    if (!guard)
    {
        return;
    }
    m_entries.clear();
    {
        QSignalBlocker viewBlocker(m_view);
        QSignalBlocker documentBlocker(m_view->document());
        m_view->clear();
    }
    if (!guard)
    {
        return;
    }
    updateDiagnostics();
}

bool AntLog::isOwnerThread() const
{
    return QThread::currentThread() == thread();
}

void AntLog::enqueuePendingCommand(const PendingCommand& command)
{
    bool scheduleDrain = false;
    {
        QMutexLocker locker(&m_pendingCommandMutex);
        if (m_pendingCommands.size() >= kMaxPendingCrossThreadCommands)
        {
            if (command.type == PendingCommandType::Append)
            {
                ++m_droppedCrossThreadCommandCount;
                return;
            }

            auto removable = std::find_if(m_pendingCommands.begin(), m_pendingCommands.end(),
                                          [](const PendingCommand& pending) {
                return pending.type == PendingCommandType::Append;
            });
            if (removable != m_pendingCommands.end())
            {
                m_pendingCommands.erase(removable);
            }
            else
            {
                m_pendingCommands.removeFirst();
            }
            ++m_droppedCrossThreadCommandCount;
        }

        m_pendingCommands.append(command);
        if (!m_pendingCommandDrainScheduled)
        {
            m_pendingCommandDrainScheduled = true;
            scheduleDrain = true;
        }
    }

    if (scheduleDrain &&
        !QMetaObject::invokeMethod(this, [this]() { drainPendingCommands(); }, Qt::QueuedConnection))
    {
        QMutexLocker locker(&m_pendingCommandMutex);
        m_pendingCommandDrainScheduled = false;
    }
}

void AntLog::drainPendingCommands()
{
    QVector<PendingCommand> commands;
    bool scheduleNextDrain = false;
    {
        QMutexLocker locker(&m_pendingCommandMutex);
        const int commandCount = qMin(kPendingCommandDrainBatchSize, m_pendingCommands.size());
        commands.reserve(commandCount);
        for (int index = 0; index < commandCount; ++index)
        {
            commands.append(std::move(m_pendingCommands[index]));
        }
        m_pendingCommands.erase(m_pendingCommands.begin(),
                                m_pendingCommands.begin() + commandCount);
        scheduleNextDrain = !m_pendingCommands.isEmpty();
        m_pendingCommandDrainScheduled = scheduleNextDrain;
    }

    ++m_crossThreadDrainCount;
    QPointer<AntLog> guard(this);
    for (const PendingCommand& command : std::as_const(commands))
    {
        switch (command.type)
        {
        case PendingCommandType::Append:
            appendEntry(command.level, command.message);
            break;
        case PendingCommandType::Clear:
            clear();
            break;
        case PendingCommandType::SetMaxEntries:
            setMaxEntries(command.integerValue);
            break;
        case PendingCommandType::SetAutoScroll:
            setAutoScroll(command.boolValue);
            break;
        }
        if (!guard)
        {
            return;
        }
    }

    updateDiagnostics();
    if (scheduleNextDrain &&
        !QMetaObject::invokeMethod(this, [this]() { drainPendingCommands(); }, Qt::QueuedConnection))
    {
        QMutexLocker locker(&m_pendingCommandMutex);
        m_pendingCommandDrainScheduled = false;
    }
}

void AntLog::appendEntry(Level level, const QString& message)
{
    Entry entry;
    entry.timestamp = QTime::currentTime().toString(QStringLiteral("hh:mm:ss.zzz"));
    entry.level = level;
    entry.message = message;
    m_entries.append(entry);
    const int sizeBeforeTrim = m_entries.size();
    trimEntries();
    if (!insertEntry(entry))
    {
        return;
    }
    const int trimmedCount = sizeBeforeTrim - m_entries.size();
    scheduleAppendFlush(trimmedCount);
    updateDiagnostics(trimmedCount);
}

void AntLog::trimEntries()
{
    const int maxEntries = qMax(1, m_maxEntries);
    const int extra = m_entries.size() - maxEntries;
    if (extra > 0)
    {
        m_entries.erase(m_entries.begin(), m_entries.begin() + extra);
    }
}

void AntLog::updateTheme()
{
    const auto& token = antTheme->tokens();
    const QColor background = token.colorFillQuaternary;
    bool appliedThemeWork = false;
    QPalette pal = m_view->palette();
    pal.setColor(QPalette::Base, background);
    pal.setColor(QPalette::Window, background);
    pal.setColor(QPalette::Text, token.colorText);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::PlaceholderText, token.colorTextPlaceholder);
    pal.setColor(QPalette::Inactive, QPalette::Base, background);
    pal.setColor(QPalette::Inactive, QPalette::Window, background);
    pal.setColor(QPalette::Disabled, QPalette::Base, background);
    pal.setColor(QPalette::Disabled, QPalette::Window, background);
    pal.setColor(QPalette::Disabled, QPalette::Text, token.colorTextDisabled);
    if (m_cachedViewPalette != pal || m_view->palette() != pal)
    {
        m_view->setPalette(pal);
        m_cachedViewPalette = pal;
        appliedThemeWork = true;
    }
    if (!m_view->viewport()->autoFillBackground())
    {
        m_view->viewport()->setAutoFillBackground(true);
        appliedThemeWork = true;
    }
    if (m_cachedViewportPalette != pal || m_view->viewport()->palette() != pal)
    {
        m_view->viewport()->setPalette(pal);
        m_cachedViewportPalette = pal;
        appliedThemeWork = true;
    }
    if (m_view->frameShape() != QFrame::NoFrame)
    {
        m_view->setFrameShape(QFrame::NoFrame);
        appliedThemeWork = true;
    }
    if (!m_view->testAttribute(Qt::WA_TranslucentBackground))
    {
        m_view->setAttribute(Qt::WA_TranslucentBackground, true);
        appliedThemeWork = true;
    }
    if (m_view->autoFillBackground())
    {
        m_view->setAutoFillBackground(false);
        appliedThemeWork = true;
    }

    for (int i = 0; i < m_levelFormats.size(); ++i)
    {
        QTextCharFormat fmt;
        fmt.setForeground(levelColor(static_cast<Level>(i)));
        if (m_levelFormats.at(i) != fmt)
        {
            m_levelFormats[i] = fmt;
            appliedThemeWork = true;
        }
    }

    if (appliedThemeWork)
    {
        ++m_themeApplyCount;
    }
    else
    {
        ++m_themeSkipCount;
    }
    setProperty("antLogThemeApplyCount", m_themeApplyCount);
    setProperty("antLogThemeSkipCount", m_themeSkipCount);
    setProperty("antLogThemeUsesPalette", true);
}

void AntLog::rebuildDocument()
{
    flushPendingAppendViewUpdates();
    const bool shouldScroll = m_autoScroll;
    const bool updatesEnabled = m_view->updatesEnabled();
    QSignalBlocker blocker(m_view->document());
    m_view->setUpdatesEnabled(false);
    {
        m_view->clear();
        QTextCursor cursor(m_view->document());
        cursor.movePosition(QTextCursor::End);
        cursor.beginEditBlock();
        for (const Entry& entry : std::as_const(m_entries))
        {
            const QString line = QStringLiteral("[%1] [%2] %3").arg(entry.timestamp, levelTag(entry.level), entry.message);
            cursor.insertText(line + QStringLiteral("\n"), formatForLevel(entry.level));
        }
        cursor.endEditBlock();
    }
    m_view->setUpdatesEnabled(updatesEnabled);
    m_view->viewport()->update();
    if (shouldScroll)
    {
        auto* sb = m_view->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
    updateDiagnostics();
}

bool AntLog::insertEntry(const Entry& entry)
{
    const QString line = QStringLiteral("[%1] [%2] %3").arg(entry.timestamp, levelTag(entry.level), entry.message);

    QPointer<AntLog> guard(this);
    QSignalBlocker viewBlocker(m_view);
    QSignalBlocker documentBlocker(m_view->document());
    QTextCursor cursor(m_view->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(line + QStringLiteral("\n"), formatForLevel(entry.level));
    if (!guard)
    {
        return false;
    }
    ++m_appendInsertCount;
    setProperty("antLogUsesDocumentCursor", true);
    return guard;
}

void AntLog::scheduleAppendFlush(int trimmedCount)
{
    if (!m_view)
    {
        return;
    }

    if (!m_appendBatchActive)
    {
        m_appendBatchActive = true;
        m_viewUpdatesEnabledBeforeAppendBatch = m_view->updatesEnabled();
        if (m_viewUpdatesEnabledBeforeAppendBatch)
        {
            m_view->setUpdatesEnabled(false);
        }
        ++m_appendBatchStartCount;
    }

    ++m_pendingAppendCount;
    m_pendingTrimmedCount += qMax(0, trimmedCount);

    if (m_pendingAppendCount >= kAppendFlushThreshold)
    {
        flushPendingAppendViewUpdates();
        return;
    }

    if (m_appendFlushTimer && !m_appendFlushTimer->isActive())
    {
        m_appendFlushTimer->start();
    }
}

void AntLog::flushPendingAppendViewUpdates()
{
    if (!m_appendBatchActive)
    {
        updateDiagnostics();
        return;
    }

    if (m_appendFlushTimer && m_appendFlushTimer->isActive())
    {
        m_appendFlushTimer->stop();
    }

    if (m_view)
    {
        if (m_viewUpdatesEnabledBeforeAppendBatch && !m_view->updatesEnabled())
        {
            m_view->setUpdatesEnabled(true);
        }
        else if (!m_viewUpdatesEnabledBeforeAppendBatch && m_view->updatesEnabled())
        {
            m_view->setUpdatesEnabled(false);
        }

        m_view->viewport()->update();
        if (m_autoScroll)
        {
            auto* sb = m_view->verticalScrollBar();
            sb->setValue(sb->maximum());
        }
    }

    ++m_appendFlushCount;
    const int trimmedCount = m_pendingTrimmedCount;
    m_pendingAppendCount = 0;
    m_pendingTrimmedCount = 0;
    m_appendBatchActive = false;
    updateDiagnostics(trimmedCount);
}

QTextCharFormat AntLog::formatForLevel(Level level) const
{
    const int index = levelIndex(level);
    if (index >= 0 && index < m_levelFormats.size())
    {
        return m_levelFormats.at(index);
    }

    QTextCharFormat fmt;
    fmt.setForeground(levelColor(level));
    return fmt;
}

void AntLog::updateDiagnostics(int trimmedCount)
{
    int pendingCommandCount = 0;
    quint64 droppedCommandCount = 0;
    {
        QMutexLocker locker(&m_pendingCommandMutex);
        pendingCommandCount = m_pendingCommands.size();
        droppedCommandCount = m_droppedCrossThreadCommandCount;
    }
    setProperty("antLogEntryCount", m_entries.size());
    setProperty("antLogDocumentBlockCount", m_view ? m_view->document()->blockCount() : 0);
    setProperty("antLogLastTrimmedCount", qMax(0, trimmedCount));
    setProperty("antLogUndoRedoEnabled", m_view ? m_view->isUndoRedoEnabled() : false);
    setProperty("antLogAppendInsertCount", m_appendInsertCount);
    setProperty("antLogPendingAppendCount", m_pendingAppendCount);
    setProperty("antLogAppendBatchStartCount", m_appendBatchStartCount);
    setProperty("antLogAppendFlushCount", m_appendFlushCount);
    setProperty("antLogPendingCrossThreadCommandCount", pendingCommandCount);
    setProperty("antLogDroppedCrossThreadCommandCount",
                QVariant::fromValue<qulonglong>(droppedCommandCount));
    setProperty("antLogCrossThreadDrainCount", m_crossThreadDrainCount);
}
