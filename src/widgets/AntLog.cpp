#include "AntLog.h"

#include <QFont>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QTime>
#include <QVBoxLayout>

#include "AntScrollBar.h"
#include "core/AntTheme.h"

namespace
{

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

} // namespace

AntLog::AntLog(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_view = new QPlainTextEdit(this);
    m_view->setReadOnly(true);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setVerticalScrollBar(new AntScrollBar(Qt::Vertical, m_view));
    m_view->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal, m_view));

    QFont mono(QStringLiteral("Consolas"), 12);
    mono.setStyleHint(QFont::Monospace);
    m_view->setFont(mono);
    m_view->document()->setMaximumBlockCount(m_maxEntries);

    layout->addWidget(m_view);
}

int AntLog::maxEntries() const { return m_maxEntries; }
void AntLog::setMaxEntries(int n)
{
    if (m_maxEntries == n) return;
    m_maxEntries = n;
    m_view->document()->setMaximumBlockCount(m_maxEntries);
    Q_EMIT maxEntriesChanged(m_maxEntries);
}

bool AntLog::autoScroll() const { return m_autoScroll; }
void AntLog::setAutoScroll(bool enabled)
{
    if (m_autoScroll == enabled) return;
    m_autoScroll = enabled;
    Q_EMIT autoScrollChanged(m_autoScroll);
}

void AntLog::debug(const QString& message)   { appendEntry(Debug, message); }
void AntLog::info(const QString& message)    { appendEntry(Info, message); }
void AntLog::success(const QString& message) { appendEntry(Success, message); }
void AntLog::warning(const QString& message) { appendEntry(Warning, message); }
void AntLog::error(const QString& message)   { appendEntry(Error, message); }

void AntLog::log(Level level, const QString& message)
{
    appendEntry(level, message);
}

void AntLog::clear()
{
    m_view->clear();
}

void AntLog::appendEntry(Level level, const QString& message)
{
    const QString timestamp = QTime::currentTime().toString(QStringLiteral("hh:mm:ss.zzz"));
    const QString line = QStringLiteral("[%1] [%2] %3").arg(timestamp, levelTag(level), message);
    const QColor color = levelColor(level);

    m_view->moveCursor(QTextCursor::End);
    QTextCharFormat fmt;
    fmt.setForeground(color);
    m_view->textCursor().insertText(line + QStringLiteral("\n"), fmt);

    if (m_autoScroll)
    {
        auto* sb = m_view->verticalScrollBar();
        sb->setValue(sb->maximum());
    }
}
