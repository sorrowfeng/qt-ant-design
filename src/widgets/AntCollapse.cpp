#include "AntCollapse.h"

#include <QMouseEvent>
#include <QPainter>
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
        m_contentHeight = v.toInt();
        updateGeometry();
        update();
    });

    setExpanded(false);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

QString AntCollapsePanel::title() const { return m_title; }
void AntCollapsePanel::setTitle(const QString& title)
{
    if (m_title == title) return;
    m_title = title;
    update();
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
        m_layout->removeWidget(m_content);
        m_content->deleteLater();
    }
    m_content = widget;
    if (m_content)
    {
        m_content->setVisible(m_expanded);
        m_layout->addWidget(m_content);
        m_content->adjustSize();
        applyExpanded(m_expanded, false);
    }
    updateGeometry();
}

QSize AntCollapsePanel::sizeHint() const
{
    int contentH = (m_content && m_expanded && m_animation->state() != QAbstractAnimation::Running)
                       ? m_content->sizeHint().height()
                       : m_contentHeight;
    if (contentH > 0)
        contentH += kCollapseContentPadding * 2;
    return QSize(300, kCollapseHeaderHeight + contentH);
}

void AntCollapsePanel::applyExpanded(bool expanded, bool animate)
{
    if (m_expanded == expanded && m_animation->state() == QAbstractAnimation::Stopped)
        return;
    m_expanded = expanded;
    m_layout->setContentsMargins(16,
        kCollapseHeaderHeight + (expanded ? kCollapseContentPadding : 0),
        16,
        expanded ? kCollapseContentPadding : 0);

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

    updateGeometry();
    update();
    Q_EMIT expandedChanged(m_expanded);
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

void AntCollapsePanel::enterEvent(QEnterEvent*)
{
    m_hovered = true;
    update();
}

void AntCollapsePanel::leaveEvent(QEvent*)
{
    m_hovered = false;
    update();
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
    });
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
    int h = 0;
    for (auto* p : m_panels)
        h += p->sizeHint().height();
    return QSize(300, h + (m_bordered ? 2 : 0));
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
