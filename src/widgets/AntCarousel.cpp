#include "AntCarousel.h"

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include "core/AntTheme.h"

AntCarousel::AntCarousel(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setMinimumHeight(160);

    m_timer = new QTimer(this);
    m_timer->setInterval(m_interval);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        if (m_slides.isEmpty()) return;
        setCurrentIndex((m_currentIndex + 1) % m_slides.size());
    });
    if (m_autoPlay) m_timer->start();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

bool AntCarousel::autoPlay() const { return m_autoPlay; }
void AntCarousel::setAutoPlay(bool autoPlay)
{
    if (m_autoPlay == autoPlay) return;
    m_autoPlay = autoPlay;
    if (autoPlay) m_timer->start(); else m_timer->stop();
    Q_EMIT autoPlayChanged(m_autoPlay);
}

int AntCarousel::interval() const { return m_interval; }
void AntCarousel::setInterval(int ms)
{
    if (m_interval == ms) return;
    m_interval = ms;
    m_timer->setInterval(ms);
    Q_EMIT intervalChanged(m_interval);
}

bool AntCarousel::showDots() const { return m_showDots; }
void AntCarousel::setShowDots(bool show)
{
    if (m_showDots == show) return;
    m_showDots = show;
    update();
    Q_EMIT showDotsChanged(m_showDots);
}

int AntCarousel::currentIndex() const { return m_currentIndex; }
void AntCarousel::setCurrentIndex(int index)
{
    if (m_slides.isEmpty()) return;
    index = (index % m_slides.size() + m_slides.size()) % m_slides.size();
    if (m_currentIndex == index) return;
    m_currentIndex = index;
    updateSlideVisibility();
    update();
    Q_EMIT currentIndexChanged(m_currentIndex);
}

int AntCarousel::count() const { return m_slides.size(); }

void AntCarousel::addSlide(QWidget* widget)
{
    widget->setParent(this);
    widget->hide();
    m_slides.append(widget);
    updateSlideVisibility();
}

void AntCarousel::removeSlide(int index)
{
    if (index < 0 || index >= m_slides.size()) return;
    m_slides[index]->deleteLater();
    m_slides.removeAt(index);
    if (m_currentIndex >= m_slides.size()) m_currentIndex = 0;
    updateSlideVisibility();
}

void AntCarousel::clearSlides()
{
    for (auto* w : m_slides) w->deleteLater();
    m_slides.clear();
    m_currentIndex = 0;
}

void AntCarousel::updateSlideVisibility()
{
    for (int i = 0; i < m_slides.size(); ++i)
    {
        if (i == m_currentIndex)
        {
            m_slides[i]->setGeometry(rect());
            m_slides[i]->show();
            m_slides[i]->raise();
        }
        else
        {
            m_slides[i]->hide();
        }
    }
}

void AntCarousel::paintEvent(QPaintEvent*)
{
    const auto& token = antTheme->tokens();
    QPainter p(this);

    if (m_slides.isEmpty())
    {
        p.setPen(token.colorTextPlaceholder);
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("No slides"));
        return;
    }

    if (m_showDots && m_slides.size() > 1)
    {
        const qreal dotSz = 8;
        const qreal spacing = 12;
        const qreal totalW = m_slides.size() * (dotSz + spacing) - spacing;
        const qreal startX = (width() - totalW) / 2.0;
        const qreal y = height() - 24;

        for (int i = 0; i < m_slides.size(); ++i)
        {
            QRectF dotR(startX + i * (dotSz + spacing), y, dotSz, dotSz);
            p.setPen(Qt::NoPen);
            p.setBrush(i == m_currentIndex ? token.colorPrimary : token.colorBorderSecondary);
            p.drawEllipse(dotR);
        }
    }
}

void AntCarousel::mousePressEvent(QMouseEvent* e)
{
    if (e->x() < width() / 3)
        setCurrentIndex(m_currentIndex - 1);
    else if (e->x() > width() * 2 / 3)
        setCurrentIndex(m_currentIndex + 1);
    QWidget::mousePressEvent(e);
}
