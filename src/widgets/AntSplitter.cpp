#include "AntSplitter.h"

#include <QPainter>

#include "core/AntTheme.h"

// ---- AntSplitter ----

AntSplitter::AntSplitter(QWidget* parent)
    : QSplitter(parent)
{
    setChildrenCollapsible(false);
    setHandleWidth(4);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

AntSplitter::AntSplitter(Qt::Orientation orientation, QWidget* parent)
    : QSplitter(orientation, parent)
{
    setChildrenCollapsible(false);
    setHandleWidth(4);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

QSplitterHandle* AntSplitter::createHandle()
{
    return new AntSplitterHandle(orientation(), this);
}

// ---- AntSplitterHandle ----

AntSplitterHandle::AntSplitterHandle(Qt::Orientation orientation, AntSplitter* parent)
    : QSplitterHandle(orientation, parent)
{
    setMouseTracking(true);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

void AntSplitterHandle::paintEvent(QPaintEvent*)
{
    const auto& token = antTheme->tokens();
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor color = m_hovered ? token.colorPrimary : token.colorBorder;
    p.fillRect(rect(), color);
}

void AntSplitterHandle::enterEvent(QEnterEvent*)
{
    m_hovered = true;
    update();
    if (orientation() == Qt::Horizontal)
        setCursor(Qt::SplitHCursor);
    else
        setCursor(Qt::SplitVCursor);
}

void AntSplitterHandle::leaveEvent(QEvent*)
{
    m_hovered = false;
    update();
    setCursor(Qt::ArrowCursor);
}
