#include "AntCarousel.h"

#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>

#include "core/AntTheme.h"

namespace
{
class CarouselDotsOverlay : public QWidget
{
public:
    explicit CarouselDotsOverlay(AntCarousel* carousel)
        : QWidget(carousel)
        , m_carousel(carousel)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        if (!m_carousel || !m_carousel->showDots() || m_carousel->count() <= 1)
            return;

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        constexpr qreal dotWidth = 16.0;
        constexpr qreal dotActiveWidth = 24.0;
        constexpr qreal dotHeight = 3.0;
        constexpr qreal dotGap = 8.0;
        constexpr qreal dotOffset = 12.0;

        const int count = m_carousel->count();
        const int active = m_carousel->currentIndex();
        const qreal totalWidth = dotActiveWidth + (count - 1) * dotWidth + (count - 1) * dotGap;
        qreal x = (width() - totalWidth) / 2.0;
        const qreal y = height() - dotOffset - dotHeight;

        for (int i = 0; i < count; ++i)
        {
            const bool isActive = i == active;
            const qreal w = isActive ? dotActiveWidth : dotWidth;
            QColor color = token.colorBgContainer;
            color.setAlphaF(isActive ? 1.0 : 0.2);
            painter.setBrush(color);
            painter.drawRoundedRect(QRectF(x, y, w, dotHeight), dotHeight / 2.0, dotHeight / 2.0);
            x += w + dotGap;
        }
    }

private:
    AntCarousel* m_carousel = nullptr;
};
}

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

    m_dotsOverlay = new CarouselDotsOverlay(this);
    m_dotsOverlay->hide();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        update();
        updateDotsOverlay();
    });
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
    updateDotsOverlay();
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
    updateDotsOverlay();
    update();
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
    updateDotsOverlay();
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
}

void AntCarousel::mousePressEvent(QMouseEvent* e)
{
    if (e->x() < width() / 3)
        setCurrentIndex(m_currentIndex - 1);
    else if (e->x() > width() * 2 / 3)
        setCurrentIndex(m_currentIndex + 1);
    QWidget::mousePressEvent(e);
}

void AntCarousel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateSlideVisibility();
}

void AntCarousel::updateDotsOverlay()
{
    if (!m_dotsOverlay)
        return;

    m_dotsOverlay->setGeometry(rect());
    m_dotsOverlay->setVisible(m_showDots && m_slides.size() > 1);
    m_dotsOverlay->raise();
    m_dotsOverlay->update();
}
