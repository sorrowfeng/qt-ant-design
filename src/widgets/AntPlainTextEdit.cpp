#include "AntPlainTextEdit.h"

#include <QContextMenuEvent>
#include <QCursor>
#include <QFocusEvent>
#include <QFont>
#include <QMenu>
#include <QMouseEvent>
#include <QPalette>
#include <QWidget>

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
    setMouseTracking(true);
    installEventFilter(this);
    const auto resizeGripSources = findChildren<QWidget*>();
    for (auto* source : resizeGripSources)
    {
        source->installEventFilter(this);
        source->setMouseTracking(true);
    }

    applyVisualState();
    syncPlainTextEditPerfCounters();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
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
    if (m_variant != Ant::Variant::Outlined)
    {
        setResizeGripHovered(false);
    }
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
    auto* sourceWidget = qobject_cast<QWidget*>(watched);
    const bool resizeGripEventSource = sourceWidget == this || (sourceWidget && isAncestorOf(sourceWidget));
    if (resizeGripEventSource)
    {
        switch (event->type())
        {
        case QEvent::MouseButtonPress:
        case QEvent::MouseMove:
        case QEvent::MouseButtonRelease:
        {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (sourceWidget)
            {
                const QPoint widgetPos = sourceWidget->mapTo(this, mouseEvent->position().toPoint());
                if (handleResizeGripMouseEvent(mouseEvent, widgetPos))
                {
                    return true;
                }
            }
            break;
        }
        case QEvent::Leave:
            if (!m_resizing && !rect().contains(mapFromGlobal(QCursor::pos())))
            {
                setResizeGripHovered(false);
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
        setResizeGripHovered(overGrip);
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress && event->button() == Qt::LeftButton && overGrip)
    {
        m_resizing = true;
        m_resizeStartGlobal = event->globalPosition().toPoint();
        m_resizeStartSize = size();
        setResizeGripHovered(true);
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
        setResizeGripHovered(overGrip);
        event->accept();
        return true;
    }

    return false;
}

void AntPlainTextEdit::applyVisualState()
{
    const auto& token = antTheme->tokens();
    if (m_cachedVisualFontSize == token.fontSize &&
        m_cachedTextColor == token.colorText &&
        m_cachedPlaceholderColor == token.colorTextPlaceholder &&
        m_cachedDisabledTextColor == token.colorTextDisabled)
    {
        return;
    }

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

    m_cachedVisualFontSize = token.fontSize;
    m_cachedTextColor = token.colorText;
    m_cachedPlaceholderColor = token.colorTextPlaceholder;
    m_cachedDisabledTextColor = token.colorTextDisabled;
    ++m_visualStateApplyCount;
    syncPlainTextEditPerfCounters();
}

void AntPlainTextEdit::setResizeGripHovered(bool hovered)
{
    if (m_resizeGripHovered == hovered)
    {
        return;
    }

    m_resizeGripHovered = hovered;
    if (m_resizeGripHovered || m_resizing)
    {
        setCursor(Qt::SizeFDiagCursor);
        viewport()->setCursor(Qt::SizeFDiagCursor);
    }
    else
    {
        unsetCursor();
        viewport()->unsetCursor();
    }
    ++m_resizeGripCursorUpdateCount;
    updateResizeGripRegion();
    syncPlainTextEditPerfCounters();
}

void AntPlainTextEdit::updateResizeGripRegion()
{
    update(resizeGripRect().adjusted(-2, -2, 2, 2));
    ++m_resizeGripDirtyUpdateCount;
    syncPlainTextEditPerfCounters();
}

void AntPlainTextEdit::syncPlainTextEditPerfCounters() const
{
    auto* self = const_cast<AntPlainTextEdit*>(this);
    self->setProperty("antPlainTextEditVisualStateApplyCount", m_visualStateApplyCount);
    self->setProperty("antPlainTextEditResizeGripHovered", m_resizeGripHovered);
    self->setProperty("antPlainTextEditResizeGripCursorUpdateCount", m_resizeGripCursorUpdateCount);
    self->setProperty("antPlainTextEditResizeGripDirtyUpdateCount", m_resizeGripDirtyUpdateCount);
}
