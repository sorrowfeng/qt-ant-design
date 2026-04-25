#include "AntPlainTextEdit.h"

#include <QContextMenuEvent>
#include <QFocusEvent>
#include <QMenu>

#include "../styles/AntPlainTextEditStyle.h"
#include "AntScrollBar.h"
#include "core/AntTheme.h"

AntPlainTextEdit::AntPlainTextEdit(QWidget* parent)
    : QPlainTextEdit(parent)
{
    auto* s = new AntPlainTextEditStyle(style());
    s->setParent(this);
    setStyle(s);

    setVerticalScrollBar(new AntScrollBar(Qt::Vertical, this));
    setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal, this));
    setFrameShape(QFrame::NoFrame);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        update();
    });
}

AntPlainTextEdit::AntPlainTextEdit(const QString& text, QWidget* parent)
    : AntPlainTextEdit(parent)
{
    setPlainText(text);
}

Ant::TextEditVariant AntPlainTextEdit::variant() const { return m_variant; }

void AntPlainTextEdit::setVariant(Ant::TextEditVariant variant)
{
    if (m_variant == variant) return;
    m_variant = variant;
    update();
    Q_EMIT variantChanged(m_variant);
}

QString AntPlainTextEdit::placeholderText() const { return m_placeholderText; }

void AntPlainTextEdit::setPlaceholderText(const QString& text)
{
    if (m_placeholderText == text) return;
    m_placeholderText = text;
    update();
    Q_EMIT placeholderTextChanged(m_placeholderText);
}

void AntPlainTextEdit::focusInEvent(QFocusEvent* event)
{
    m_focused = true;
    update();
    QPlainTextEdit::focusInEvent(event);
}

void AntPlainTextEdit::focusOutEvent(QFocusEvent* event)
{
    m_focused = false;
    update();
    QPlainTextEdit::focusOutEvent(event);
}

void AntPlainTextEdit::contextMenuEvent(QContextMenuEvent* event)
{
    auto* menu = new QMenu(this);

    auto* undoAction = menu->addAction(QStringLiteral("Undo"));
    undoAction->setEnabled(isUndoRedoEnabled() && !isReadOnly());
    connect(undoAction, &QAction::triggered, this, &QPlainTextEdit::undo);

    auto* redoAction = menu->addAction(QStringLiteral("Redo"));
    redoAction->setEnabled(isUndoRedoEnabled() && !isReadOnly());
    connect(redoAction, &QAction::triggered, this, &QPlainTextEdit::redo);

    menu->addSeparator();

    auto* cutAction = menu->addAction(QStringLiteral("Cut"));
    cutAction->setEnabled(textCursor().hasSelection() && !isReadOnly());
    connect(cutAction, &QAction::triggered, this, &QPlainTextEdit::cut);

    auto* copyAction = menu->addAction(QStringLiteral("Copy"));
    copyAction->setEnabled(textCursor().hasSelection());
    connect(copyAction, &QAction::triggered, this, &QPlainTextEdit::copy);

    auto* pasteAction = menu->addAction(QStringLiteral("Paste"));
    pasteAction->setEnabled(canPaste() && !isReadOnly());
    connect(pasteAction, &QAction::triggered, this, &QPlainTextEdit::paste);

    auto* deleteAction = menu->addAction(QStringLiteral("Delete"));
    deleteAction->setEnabled(textCursor().hasSelection() && !isReadOnly());
    connect(deleteAction, &QAction::triggered, this, [this]() { textCursor().removeSelectedText(); });

    menu->addSeparator();

    auto* selectAllAction = menu->addAction(QStringLiteral("Select All"));
    connect(selectAllAction, &QAction::triggered, this, &QPlainTextEdit::selectAll);

    menu->exec(event->globalPos());
    menu->deleteLater();
}
