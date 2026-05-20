#include "AntSplitter.h"

#include <QPainter>

#include "core/AntTheme.h"

// ---- AntSplitter ----

AntSplitter::AntSplitter(QWidget* parent)
    : QSplitter(parent)
{
    initialize();
}

AntSplitter::AntSplitter(Qt::Orientation orientation, QWidget* parent)
    : QSplitter(orientation, parent)
{
    initialize();
}

QColor AntSplitter::handleColor(bool hovered) const
{
    return hovered ? m_hoverHandleColor : m_handleColor;
}

int AntSplitter::handleThemeRevision() const
{
    return m_handleThemeRevision;
}

QSplitterHandle* AntSplitter::createHandle()
{
    return new AntSplitterHandle(orientation(), this);
}

void AntSplitter::initialize()
{
    setChildrenCollapsible(false);
    setHandleWidth(4);
    refreshHandlePalette();
    connect(antTheme, &AntTheme::themeChanged, this, [this]() { refreshHandlePalette(); });
}

void AntSplitter::refreshHandlePalette()
{
    const auto& token = antTheme->tokens();
    const QColor handleColor = token.colorBorder;
    const QColor hoverHandleColor = token.colorPrimary;
    if (m_handleThemeRevision > 0 && m_handleColor == handleColor && m_hoverHandleColor == hoverHandleColor)
    {
        syncPerfCounters();
        return;
    }

    m_handleColor = handleColor;
    m_hoverHandleColor = hoverHandleColor;
    ++m_handleThemeRevision;
    ++m_handlePaletteRefreshCount;
    syncPerfCounters();
    updateHandles();
}

void AntSplitter::updateHandles()
{
    for (int i = 1; i < count(); ++i)
    {
        if (auto* splitterHandle = handle(i))
        {
            splitterHandle->update();
        }
    }
}

void AntSplitter::syncPerfCounters() const
{
    const_cast<AntSplitter*>(this)->setProperty("antSplitterHandleThemeRevision", m_handleThemeRevision);
    const_cast<AntSplitter*>(this)->setProperty("antSplitterHandlePaletteRefreshCount", m_handlePaletteRefreshCount);
}

// ---- AntSplitterHandle ----

AntSplitterHandle::AntSplitterHandle(Qt::Orientation orientation, AntSplitter* parent)
    : QSplitterHandle(orientation, parent)
{
    setMouseTracking(true);
    syncPerfCounters();
}

void AntSplitterHandle::paintEvent(QPaintEvent*)
{
    ++m_paintCount;
    QPainter p(this);
    p.fillRect(rect(), resolvedColor());
    syncPerfCounters();
}

void AntSplitterHandle::enterEvent(QEnterEvent*)
{
    if (m_hovered)
    {
        return;
    }
    m_hovered = true;
    ++m_hoverUpdateCount;
    invalidateColorCache();
    update();
    if (orientation() == Qt::Horizontal)
        setCursor(Qt::SplitHCursor);
    else
        setCursor(Qt::SplitVCursor);
}

void AntSplitterHandle::leaveEvent(QEvent*)
{
    if (!m_hovered)
    {
        return;
    }
    m_hovered = false;
    ++m_hoverUpdateCount;
    invalidateColorCache();
    update();
    setCursor(Qt::ArrowCursor);
}

QColor AntSplitterHandle::resolvedColor()
{
    auto* parentSplitter = qobject_cast<AntSplitter*>(splitter());
    const int themeRevision = parentSplitter ? parentSplitter->handleThemeRevision() : 0;
    if (m_cachedColor.isValid() && m_cachedThemeRevision == themeRevision && m_cachedHovered == m_hovered)
    {
        return m_cachedColor;
    }

    m_cachedColor = parentSplitter ? parentSplitter->handleColor(m_hovered)
                                   : (m_hovered ? antTheme->tokens().colorPrimary : antTheme->tokens().colorBorder);
    m_cachedThemeRevision = themeRevision;
    m_cachedHovered = m_hovered;
    ++m_colorResolveCount;
    syncPerfCounters();
    return m_cachedColor;
}

void AntSplitterHandle::invalidateColorCache()
{
    m_cachedThemeRevision = -1;
    m_cachedColor = QColor();
    syncPerfCounters();
}

void AntSplitterHandle::syncPerfCounters() const
{
    const_cast<AntSplitterHandle*>(this)->setProperty("antSplitterHandlePaintCount", m_paintCount);
    const_cast<AntSplitterHandle*>(this)->setProperty("antSplitterHandleColorResolveCount", m_colorResolveCount);
    const_cast<AntSplitterHandle*>(this)->setProperty("antSplitterHandleHoverUpdateCount", m_hoverUpdateCount);
    const_cast<AntSplitterHandle*>(this)->setProperty("antSplitterHandleCachedThemeRevision", m_cachedThemeRevision);
    const_cast<AntSplitterHandle*>(this)->setProperty("antSplitterHandleHovered", m_hovered);
}
