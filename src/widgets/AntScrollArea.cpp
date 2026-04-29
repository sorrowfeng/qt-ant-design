#include "AntScrollArea.h"

#include <QPalette>
#include <QScroller>

#include "AntScrollBar.h"
#include "core/AntTheme.h"

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
        updateTheme();
        update();
        if (viewport())
        {
            viewport()->update();
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

void AntScrollArea::updateTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::Base, token.colorBgContainer);
    pal.setColor(QPalette::Text, token.colorText);
    pal.setColor(QPalette::WindowText, token.colorText);
    setPalette(pal);
    setAutoFillBackground(false);

    if (viewport())
    {
        viewport()->setAutoFillBackground(true);
        viewport()->setPalette(pal);
    }

    if (QWidget* content = widget())
    {
        QPalette contentPalette = content->palette();
        contentPalette.setColor(QPalette::Window, token.colorBgContainer);
        contentPalette.setColor(QPalette::Base, token.colorBgContainer);
        contentPalette.setColor(QPalette::Text, token.colorText);
        contentPalette.setColor(QPalette::WindowText, token.colorText);
        content->setPalette(contentPalette);
    }
}
