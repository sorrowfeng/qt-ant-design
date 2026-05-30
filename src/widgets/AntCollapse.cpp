#include "AntCollapse.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QVBoxLayout>

#include "core/AntTheme.h"
#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int kCollapseHeaderHeight = 46;
constexpr int kCollapseContentPadding = 16;
constexpr int kCollapseIconX = 22;
}

// ---- AntCollapsePanel ----

AntCollapsePanel::AntCollapsePanel(const QString& title, QWidget* parent)
    : QWidget(parent), m_title(title)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(16, kCollapseHeaderHeight, 16, 0);
    m_layout->setSpacing(0);

    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(220);
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        const int nextHeight = v.toInt();
        if (m_contentHeight == nextHeight)
            return;
        const QRect dirty = contentPaintRect(m_contentHeight).united(contentPaintRect(nextHeight));
        m_contentHeight = nextHeight;
        invalidateSizeHintCache();
        ++m_layoutUpdateCount;
        updateGeometry();
        requestPanelUpdate(dirty, QStringLiteral("animation"), true);
    });

    setExpanded(false);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        requestPanelUpdate(rect(), QStringLiteral("theme"));
    });
    syncPanelPerfCounters();
}

QString AntCollapsePanel::title() const { return m_title; }
void AntCollapsePanel::setTitle(const QString& title)
{
    if (m_title == title) return;
    m_title = title;
    requestPanelUpdate(headerRect(), QStringLiteral("title"));
    Q_EMIT titleChanged(m_title);
}

bool AntCollapsePanel::isExpanded() const { return m_expanded; }

void AntCollapsePanel::setExpanded(bool expanded)
{
    applyExpanded(expanded, true);
}

QWidget* AntCollapsePanel::contentWidget() const { return m_content; }

void AntCollapsePanel::setContentWidget(QWidget* widget)
{
    if (m_content)
    {
        m_content->removeEventFilter(this);
        m_layout->removeWidget(m_content);
        m_content->deleteLater();
    }
    m_content = widget;
    if (m_content)
    {
        m_content->installEventFilter(this);
        m_content->setVisible(m_expanded);
        m_layout->addWidget(m_content);
        m_content->adjustSize();
        applyExpanded(m_expanded, false);
    }
    invalidateSizeHintCache();
    ++m_layoutUpdateCount;
    updateGeometry();
    requestPanelUpdate(rect(), QStringLiteral("content"));
}

QSize AntCollapsePanel::sizeHint() const
{
    if (!m_sizeHintDirty)
    {
        ++m_sizeHintHitCount;
        syncPanelPerfCounters();
        return m_cachedSizeHint;
    }

    int contentH = (m_content && m_expanded && m_animation->state() != QAbstractAnimation::Running)
                       ? m_content->sizeHint().height()
                       : m_contentHeight;
    if (contentH > 0)
        contentH += kCollapseContentPadding * 2;
    m_cachedSizeHint = QSize(300, kCollapseHeaderHeight + contentH);
    m_sizeHintDirty = false;
    ++m_sizeHintBuildCount;
    syncPanelPerfCounters();
    return m_cachedSizeHint;
}

void AntCollapsePanel::applyExpanded(bool expanded, bool animate)
{
    if (m_expanded == expanded && m_animation->state() == QAbstractAnimation::Stopped)
        return;
    const QRect oldContentRect = contentPaintRect(m_contentHeight);
    m_expanded = expanded;
    const QMargins targetMargins(16,
                                 kCollapseHeaderHeight + (expanded ? kCollapseContentPadding : 0),
                                 16,
                                 expanded ? kCollapseContentPadding : 0);
    if (m_layout->contentsMargins() != targetMargins)
        m_layout->setContentsMargins(targetMargins);

    int targetH = 0;
    if (expanded && m_content)
        targetH = m_content->sizeHint().height();

    m_animation->stop();
    if (animate && isVisible())
    {
        m_animation->setStartValue(m_contentHeight);
        m_animation->setEndValue(targetH);
        m_animation->start();
    }
    else
    {
        m_contentHeight = targetH;
    }

    if (m_content)
        m_content->setVisible(expanded);

    invalidateSizeHintCache();
    ++m_layoutUpdateCount;
    updateGeometry();
    requestPanelUpdate(headerRect().united(oldContentRect).united(contentPaintRect(m_contentHeight)),
                       expanded ? QStringLiteral("expand") : QStringLiteral("collapse"),
                       true);
    Q_EMIT expandedChanged(m_expanded);
}

bool AntCollapsePanel::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_content)
    {
        switch (event->type())
        {
        case QEvent::LayoutRequest:
        case QEvent::Resize:
        case QEvent::FontChange:
        case QEvent::StyleChange:
            refreshStaticContentHeight(QStringLiteral("content"));
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntCollapsePanel::paintEvent(QPaintEvent*)
{
    const auto& token = antTheme->tokens();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRectF headerR = QRectF(0, 0, width(), kCollapseHeaderHeight);

    // Header background
    p.fillRect(headerR, m_hovered ? token.colorFillTertiary : token.colorFillQuaternary);

    // Bottom border
    p.setPen(QPen(token.colorBorder, token.lineWidth));
    p.drawLine(QPointF(0, headerR.bottom()), QPointF(width(), headerR.bottom()));

    // Expand arrow
    const QRectF arrowRect(kCollapseIconX - 7, headerR.center().y() - 7, 14, 14);
    AntIconPainter::drawIcon(p,
                             m_expanded ? Ant::IconType::Down : Ant::IconType::Right,
                             arrowRect,
                             isEnabled() ? token.colorText : token.colorTextDisabled);

    // Title text
    QFont f = p.font();
    f.setPixelSize(token.fontSize);
    p.setFont(f);
    p.setPen(isEnabled() ? token.colorText : token.colorTextDisabled);
    p.drawText(QRectF(40, 0, width() - 56, kCollapseHeaderHeight), Qt::AlignLeft | Qt::AlignVCenter, m_title);

    // Content area background
    if (m_contentHeight > 0 && m_content)
    {
        const QRectF contentR(0, kCollapseHeaderHeight, width(), m_contentHeight + kCollapseContentPadding * 2);
        p.fillRect(contentR, token.colorBgContainer);
        p.setPen(QPen(token.colorBorder, token.lineWidth));
        p.drawLine(QPointF(0, height() - 1), QPointF(width(), height() - 1));
    }
}

void AntCollapsePanel::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && e->pos().y() < kCollapseHeaderHeight && isEnabled())
        applyExpanded(!m_expanded, true);
    QWidget::mousePressEvent(e);
}

void AntCollapsePanel::enterEvent(AntEnterEvent*)
{
    if (!m_hovered)
    {
        m_hovered = true;
        requestPanelUpdate(headerRect(), QStringLiteral("hover"));
    }
}

void AntCollapsePanel::leaveEvent(QEvent*)
{
    if (m_hovered)
    {
        m_hovered = false;
        requestPanelUpdate(headerRect(), QStringLiteral("hover"));
    }
}

void AntCollapsePanel::invalidateSizeHintCache() const
{
    m_sizeHintDirty = true;
    if (auto* collapse = qobject_cast<AntCollapse*>(parentWidget()))
        collapse->invalidateSizeHintCache();
}

QRect AntCollapsePanel::headerRect() const
{
    return QRect(0, 0, width(), kCollapseHeaderHeight).intersected(rect());
}

QRect AntCollapsePanel::contentPaintRect(int contentHeight) const
{
    if (contentHeight <= 0 && !m_content)
        return QRect();
    const int h = qMax(0, contentHeight) + (contentHeight > 0 ? kCollapseContentPadding * 2 : 0);
    return QRect(0, kCollapseHeaderHeight, width(), h).intersected(rect());
}

void AntCollapsePanel::refreshStaticContentHeight(const QString& mode)
{
    invalidateSizeHintCache();
    if (!m_content || !m_expanded || m_animation->state() == QAbstractAnimation::Running)
    {
        syncPanelPerfCounters();
        return;
    }

    const int nextHeight = m_content->sizeHint().height();
    if (m_contentHeight == nextHeight)
    {
        syncPanelPerfCounters();
        return;
    }

    const QRect dirty = contentPaintRect(m_contentHeight).united(contentPaintRect(nextHeight));
    m_contentHeight = nextHeight;
    ++m_layoutUpdateCount;
    updateGeometry();
    requestPanelUpdate(dirty, mode, true);
}

void AntCollapsePanel::requestPanelUpdate(const QRect& region, const QString& mode, bool contentScoped)
{
    QRect dirty = region.isValid() && !region.isEmpty() ? region : rect();
    dirty = dirty.intersected(rect());
    if (dirty.isEmpty())
        dirty = rect();

    ++m_panelRegionUpdateCount;
    if (contentScoped)
        ++m_contentRegionUpdateCount;
    m_lastUpdateMode = mode;
    syncPanelPerfCounters();
    update(dirty);
}

void AntCollapsePanel::syncPanelPerfCounters() const
{
    auto* self = const_cast<AntCollapsePanel*>(this);
    self->setProperty("antCollapsePanelSizeHintBuildCount", m_sizeHintBuildCount);
    self->setProperty("antCollapsePanelSizeHintHitCount", m_sizeHintHitCount);
    self->setProperty("antCollapsePanelLayoutUpdateCount", m_layoutUpdateCount);
    self->setProperty("antCollapsePanelRegionUpdateCount", m_panelRegionUpdateCount);
    self->setProperty("antCollapsePanelContentRegionUpdateCount", m_contentRegionUpdateCount);
    self->setProperty("antCollapsePanelLastUpdateMode", m_lastUpdateMode);
}

// ---- AntCollapse ----

AntCollapse::AntCollapse(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(1, 1, 1, 1);
    m_layout->setSpacing(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setAttribute(Qt::WA_StyledBackground, false);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
    syncCollapsePerfCounters();
}

bool AntCollapse::accordion() const { return m_accordion; }
void AntCollapse::setAccordion(bool accordion)
{
    if (m_accordion == accordion) return;
    m_accordion = accordion;
    if (accordion)
    {
        bool found = false;
        for (auto* p : m_panels) { if (p->isExpanded()) { if (found) p->setExpanded(false); else found = true; } }
    }
    Q_EMIT accordionChanged(m_accordion);
}

bool AntCollapse::bordered() const { return m_bordered; }
void AntCollapse::setBordered(bool bordered)
{
    if (m_bordered == bordered) return;
    m_bordered = bordered;
    invalidateSizeHintCache();
    update();
    Q_EMIT borderedChanged(m_bordered);
}

AntCollapsePanel* AntCollapse::addPanel(const QString& title)
{
    auto* panel = new AntCollapsePanel(title, this);
    m_panels.append(panel);
    m_layout->addWidget(panel);
    connect(panel, &AntCollapsePanel::expandedChanged, this, [this, panel](bool expanded) {
        onPanelExpanded(panel, expanded);
        invalidateSizeHintCache();
        updateGeometry();
    });
    invalidateSizeHintCache();
    updateGeometry();
    return panel;
}

void AntCollapse::onPanelExpanded(AntCollapsePanel* panel, bool expanded)
{
    if (m_accordion && expanded)
    {
        for (auto* p : m_panels)
        {
            if (p != panel && p->isExpanded())
                p->setExpanded(false);
        }
    }
}

QSize AntCollapse::sizeHint() const
{
    if (!m_sizeHintDirty)
    {
        ++m_sizeHintHitCount;
        syncCollapsePerfCounters();
        return m_cachedSizeHint;
    }

    int h = 0;
    for (auto* p : m_panels)
        h += p->sizeHint().height();
    m_cachedSizeHint = QSize(300, h + (m_bordered ? 2 : 0));
    m_sizeHintDirty = false;
    ++m_sizeHintBuildCount;
    syncCollapsePerfCounters();
    return m_cachedSizeHint;
}

void AntCollapse::paintEvent(QPaintEvent*)
{
    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF r = rect().adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath path;
    path.addRoundedRect(r, token.borderRadiusLG, token.borderRadiusLG);
    painter.fillPath(path, token.colorFillQuaternary);

    if (m_bordered)
    {
        painter.setPen(QPen(token.colorBorder, token.lineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }
}

void AntCollapse::invalidateSizeHintCache() const
{
    m_sizeHintDirty = true;
}

void AntCollapse::syncCollapsePerfCounters() const
{
    auto* self = const_cast<AntCollapse*>(this);
    self->setProperty("antCollapseSizeHintBuildCount", m_sizeHintBuildCount);
    self->setProperty("antCollapseSizeHintHitCount", m_sizeHintHitCount);
}
