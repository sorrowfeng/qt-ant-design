#include "AntPlainTextEdit.h"

#include <QContextMenuEvent>
#include <QFocusEvent>
#include <QMenu>
#include <QMouseEvent>

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
    viewport()->installEventFilter(this);

    auto applyVisualState = [this]() {
        const auto& token = antTheme->tokens();
        QFont f = font();
        f.setPixelSize(token.fontSize);
        setFont(f);

        QPalette pal = palette();
        pal.setColor(QPalette::Base, Qt::transparent);
        pal.setColor(QPalette::Text, token.colorText);
        pal.setColor(QPalette::PlaceholderText, token.colorTextPlaceholder);
        pal.setColor(QPalette::Disabled, QPalette::Text, token.colorTextDisabled);
        pal.setColor(QPalette::Disabled, QPalette::PlaceholderText, token.colorTextDisabled);
        setPalette(pal);
        viewport()->setAutoFillBackground(false);
        viewport()->setPalette(pal);
    };
    applyVisualState();

    connect(antTheme, &AntTheme::themeChanged, this, [this, applyVisualState]() {
        applyVisualState();
        update();
    });
}

AntPlainTextEdit::AntPlainTextEdit(const QString& text, QWidget* parent)
    : AntPlainTextEdit(parent)
{
    setPlainText(text);
}

Ant::Variant AntPlainTextEdit::variant() const { return m_variant; }

void AntPlainTextEdit::setVariant(Ant::Variant variant)
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

bool AntPlainTextEdit::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == viewport())
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseMove:
        case QEvent::MouseButtonRelease:
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            const QPoint widgetPos = viewport()->mapTo(this, mouseEvent->position().toPoint());
            if (handleResizeGripMouseEvent(mouseEvent, widgetPos))
            {
                return true;
            }
            break;
        }
        case QEvent::Leave:
            if (!m_resizing)
            {
                viewport()->unsetCursor();
            }
            break;
        default:
            break;
        }
    }
    return QPlainTextEdit::eventFilter(watched, event);
}

QRect AntPlainTextEdit::resizeGripRect() const
{
    constexpr int gripSize = 18;
    return QRect(width() - gripSize, height() - gripSize, gripSize, gripSize);
}

bool AntPlainTextEdit::handleResizeGripMouseEvent(QMouseEvent* event, const QPoint& widgetPos)
{
    if (!event || !isEnabled() || m_variant != Ant::Variant::Outlined)
    {
        return false;
    }

    const bool overGrip = resizeGripRect().contains(widgetPos);
    if (event->type() == QEvent::MouseMove && !m_resizing)
    {
        overGrip ? viewport()->setCursor(Qt::SizeFDiagCursor) : viewport()->unsetCursor();
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress && event->button() == Qt::LeftButton && overGrip)
    {
        m_resizing = true;
        m_resizeStartGlobal = event->globalPosition().toPoint();
        m_resizeStartSize = size();
        viewport()->setCursor(Qt::SizeFDiagCursor);
        event->accept();
        return true;
    }

    if (event->type() == QEvent::MouseMove && m_resizing)
    {
        const QPoint delta = event->globalPosition().toPoint() - m_resizeStartGlobal;
        const QSize minSize(120, 56);
        const QSize nextSize(qMax(minSize.width(), m_resizeStartSize.width() + delta.x()),
                             qMax(minSize.height(), m_resizeStartSize.height() + delta.y()));
        setFixedSize(nextSize);
        updateGeometry();
        event->accept();
        return true;
    }

    if (event->type() == QEvent::MouseButtonRelease && m_resizing)
    {
        m_resizing = false;
        overGrip ? viewport()->setCursor(Qt::SizeFDiagCursor) : viewport()->unsetCursor();
        event->accept();
        return true;
    }

    return false;
}
