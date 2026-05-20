#include "AntScrollArea.h"

#include <QPalette>
#include <QScroller>

#include "AntScrollBar.h"
#include "core/AntTheme.h"

namespace
{

void applySurfaceColors(QPalette& palette, const QColor& bg, const QColor& text)
{
    palette.setColor(QPalette::Window, bg);
    palette.setColor(QPalette::Base, bg);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::WindowText, text);
}

bool surfaceColorsMatch(const QPalette& palette, const QColor& bg, const QColor& text)
{
    return palette.color(QPalette::Window) == bg &&
           palette.color(QPalette::Base) == bg &&
           palette.color(QPalette::Text) == text &&
           palette.color(QPalette::WindowText) == text;
}

} // namespace

AntScrollArea::AntScrollArea(QWidget* parent)
    : QScrollArea(parent)
{
    setFrameShape(QFrame::NoFrame);
    setWidgetResizable(true);

    m_vScrollBar = new AntScrollBar(Qt::Vertical, this);
    m_vScrollBar->setAutoHide(m_autoHideScrollBar);
    setVerticalScrollBar(m_vScrollBar);

    m_hScrollBar = new AntScrollBar(Qt::Horizontal, this);
    m_hScrollBar->setAutoHide(m_autoHideScrollBar);
    setHorizontalScrollBar(m_hScrollBar);

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    updateTheme();
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        if (updateTheme())
        {
            update();
            if (viewport())
            {
                viewport()->update();
                ++m_viewportUpdateCount;
                syncScrollAreaPerfCounters();
            }
        }
    });

    QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    QScrollerProperties sp = QScroller::scroller(this)->scrollerProperties();
    sp.setScrollMetric(QScrollerProperties::MousePressEventDelay, 0.5);
    sp.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.35);
    sp.setScrollMetric(QScrollerProperties::OvershootScrollTime, 0.5);
    sp.setScrollMetric(QScrollerProperties::FrameRate, QScrollerProperties::Fps60);
    QScroller::scroller(this)->setScrollerProperties(sp);
}

void AntScrollArea::setWidget(QWidget* widget)
{
    QScrollArea::setWidget(widget);
    updateTheme();
}

bool AntScrollArea::autoHideScrollBar() const { return m_autoHideScrollBar; }

void AntScrollArea::setAutoHideScrollBar(bool autoHide)
{
    if (m_autoHideScrollBar == autoHide) return;
    m_autoHideScrollBar = autoHide;
    if (m_vScrollBar) m_vScrollBar->setAutoHide(autoHide);
    if (m_hScrollBar) m_hScrollBar->setAutoHide(autoHide);
    Q_EMIT autoHideScrollBarChanged(m_autoHideScrollBar);
}

bool AntScrollArea::isGestureEnabled() const { return m_enableGesture; }

void AntScrollArea::setEnableGesture(bool enable)
{
    if (m_enableGesture == enable) return;
    m_enableGesture = enable;
    if (enable)
        QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    else
        QScroller::ungrabGesture(this);
    Q_EMIT enableGestureChanged(m_enableGesture);
}

bool AntScrollArea::updateTheme()
{
    const auto& token = antTheme->tokens();
    const QColor bg = token.colorBgContainer;
    const QColor text = token.colorText;
    const bool themeChanged = !m_themeCacheValid || m_cachedBgColor != bg || m_cachedTextColor != text;
    bool surfaceChanged = false;

    if (themeChanged || !surfaceColorsMatch(palette(), bg, text))
    {
        QPalette pal = palette();
        applySurfaceColors(pal, bg, text);
        setPalette(pal);
        ++m_surfacePaletteApplyCount;
        surfaceChanged = true;
    }

    if (autoFillBackground())
    {
        setAutoFillBackground(false);
    }

    if (viewport())
    {
        if (!viewport()->autoFillBackground())
        {
            viewport()->setAutoFillBackground(true);
        }
        if (themeChanged || !surfaceColorsMatch(viewport()->palette(), bg, text))
        {
            QPalette viewportPalette = viewport()->palette();
            applySurfaceColors(viewportPalette, bg, text);
            viewport()->setPalette(viewportPalette);
            ++m_viewportPaletteApplyCount;
            surfaceChanged = true;
        }
    }

    QWidget* content = widget();
    if (content)
    {
        const bool contentChanged = m_cachedContentWidget != content;
        if (themeChanged || contentChanged || !surfaceColorsMatch(content->palette(), bg, text))
        {
            QPalette contentPalette = content->palette();
            applySurfaceColors(contentPalette, bg, text);
            content->setPalette(contentPalette);
            content->update();
            ++m_contentPaletteApplyCount;
        }
    }

    m_cachedBgColor = bg;
    m_cachedTextColor = text;
    m_cachedContentWidget = content;
    m_themeCacheValid = true;
    syncScrollAreaPerfCounters();
    return surfaceChanged;
}

void AntScrollArea::syncScrollAreaPerfCounters() const
{
    auto* self = const_cast<AntScrollArea*>(this);
    self->setProperty("antScrollAreaSurfacePaletteApplyCount", m_surfacePaletteApplyCount);
    self->setProperty("antScrollAreaViewportPaletteApplyCount", m_viewportPaletteApplyCount);
    self->setProperty("antScrollAreaContentPaletteApplyCount", m_contentPaletteApplyCount);
    self->setProperty("antScrollAreaViewportUpdateCount", m_viewportUpdateCount);
}
