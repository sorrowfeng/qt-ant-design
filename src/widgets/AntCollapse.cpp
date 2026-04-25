#include "AntCollapse.h"

#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

// ---- AntCollapsePanel ----

AntCollapsePanel::AntCollapsePanel(const QString& title, QWidget* parent)
    : QWidget(parent), m_title(title)
{
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(16, 44, 16, 0);
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
    return QSize(300, 44 + contentH);
}

void AntCollapsePanel::applyExpanded(bool expanded, bool animate)
{
    if (m_expanded == expanded && m_animation->state() == QAbstractAnimation::Stopped)
        return;
    m_expanded = expanded;

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

    const QRectF headerR = QRectF(0, 0, width(), 44);

    // Header background
    if (m_hovered)
    {
        p.fillRect(headerR, token.colorFillQuaternary);
    }

    // Bottom border
    p.setPen(QPen(token.colorSplit, token.lineWidth));
    p.drawLine(QPointF(0, headerR.bottom()), QPointF(width(), headerR.bottom()));

    // Expand arrow
    const qreal arrowX = 16;
    const qreal arrowY = headerR.center().y();
    const qreal sz = 4;
    p.setPen(QPen(token.colorTextSecondary, 2));
    if (m_expanded)
    {
        p.drawLine(QPointF(arrowX - sz, arrowY + sz * 0.5), QPointF(arrowX, arrowY - sz * 0.8));
        p.drawLine(QPointF(arrowX, arrowY - sz * 0.8), QPointF(arrowX + sz, arrowY + sz * 0.5));
    }
    else
    {
        p.drawLine(QPointF(arrowX - sz * 0.5, arrowY - sz), QPointF(arrowX + sz * 0.8, arrowY));
        p.drawLine(QPointF(arrowX + sz * 0.8, arrowY), QPointF(arrowX - sz * 0.5, arrowY + sz));
    }

    // Title text
    QFont f = p.font();
    f.setPixelSize(token.fontSize);
    p.setFont(f);
    p.setPen(isEnabled() ? token.colorText : token.colorTextDisabled);
    p.drawText(QRectF(36, 0, width() - 52, 44), Qt::AlignLeft | Qt::AlignVCenter, m_title);

    // Content area background
    if (m_contentHeight > 0 && m_content)
    {
        const QRectF contentR(0, 44, width(), m_contentHeight);
        p.fillRect(contentR, token.colorBgContainer);
    }
}

void AntCollapsePanel::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && e->pos().y() < 44 && isEnabled())
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
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

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
