#include "AntCarousel.h"

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QEvent>
#include <QHideEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QResizeEvent>
#include <QShowEvent>
#include <QTimer>

#include <algorithm>

#include "core/AntCompat.h"
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
        if (!m_carousel || m_carousel->count() <= 1)
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
        if (m_carousel->showDots())
        {
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

        if (m_carousel->manualNavigationEnabled() && m_carousel->showArrows() && count > 1)
        {
            auto drawArrow = [&painter, &token](const QRect& rect, bool left) {
                QColor bg = token.colorBgContainer;
                bg.setAlphaF(0.72);
                QColor border = token.colorBorderSecondary;
                border.setAlphaF(0.78);
                painter.setPen(QPen(border, token.lineWidth));
                painter.setBrush(bg);
                painter.drawEllipse(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5));

                QPen pen(token.colorText, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
                painter.setPen(pen);
                painter.setBrush(Qt::NoBrush);
                const QPointF center(rect.left() + rect.width() / 2.0, rect.top() + rect.height() / 2.0);
                const qreal halfWidth = 3.4;
                const qreal halfHeight = 5.2;
                const qreal tipOffset = left ? -halfWidth : halfWidth;
                QPainterPath path;
                path.moveTo(center.x() - tipOffset, center.y() - halfHeight);
                path.lineTo(center.x() + tipOffset, center.y());
                path.lineTo(center.x() - tipOffset, center.y() + halfHeight);
                painter.drawPath(path);
            };
            const QSize arrowSize(28, 28);
            const int margin = 12;
            drawArrow(QRect(margin, (height() - arrowSize.height()) / 2, arrowSize.width(), arrowSize.height()), true);
            drawArrow(QRect(width() - margin - arrowSize.width(),
                            (height() - arrowSize.height()) / 2,
                            arrowSize.width(),
                            arrowSize.height()),
                      false);
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
        if (!shouldAutoPlayTimerRun())
        {
            updateAutoPlayTimer();
            return;
        }
        if (isTransitionRunning())
        {
            resetAutoPlayTimer();
            return;
        }
        setCurrentIndex(m_currentIndex + 1);
    });

    m_dotsOverlay = new CarouselDotsOverlay(this);
    m_dotsOverlay->hide();

    m_transitionAnimation = new QPropertyAnimation(this, "transitionProgress", this);
    m_transitionAnimation->setDuration(500);
    m_transitionAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_transitionAnimation, &QPropertyAnimation::finished, this, &AntCarousel::finishTransition);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        update();
        updateDotsOverlay();
    });
    updateAutoPlayTimer();
    syncCarouselPerfCounters();
}

bool AntCarousel::autoPlay() const { return m_autoPlay; }
void AntCarousel::setAutoPlay(bool autoPlay)
{
    if (m_autoPlay == autoPlay) return;
    m_autoPlay = autoPlay;
    updateAutoPlayTimer();
    Q_EMIT autoPlayChanged(m_autoPlay);
}

int AntCarousel::interval() const { return m_interval; }
void AntCarousel::setInterval(int ms)
{
    if (m_interval == ms) return;
    m_interval = ms;
    m_timer->setInterval(ms);
    updateAutoPlayTimer();
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

bool AntCarousel::manualNavigationEnabled() const { return m_manualNavigationEnabled; }

void AntCarousel::setManualNavigationEnabled(bool enabled)
{
    if (m_manualNavigationEnabled == enabled)
        return;
    m_manualNavigationEnabled = enabled;
    updateDotsOverlay();
    Q_EMIT manualNavigationEnabledChanged(m_manualNavigationEnabled);
}

bool AntCarousel::showArrows() const { return m_showArrows; }

void AntCarousel::setShowArrows(bool show)
{
    if (m_showArrows == show)
        return;
    m_showArrows = show;
    updateDotsOverlay();
    Q_EMIT showArrowsChanged(m_showArrows);
}

int AntCarousel::currentIndex() const { return m_currentIndex; }
void AntCarousel::setCurrentIndex(int index)
{
    if (m_slides.isEmpty()) return;
    const int requestedIndex = index;
    index = (index % m_slides.size() + m_slides.size()) % m_slides.size();
    if (m_currentIndex == index) return;
    const int previous = m_currentIndex;
    m_currentIndex = index;
    startTransition(previous, m_currentIndex, requestedIndex);
    Q_EMIT currentIndexChanged(m_currentIndex);
}

qreal AntCarousel::transitionProgress() const { return m_transitionProgress; }

void AntCarousel::setTransitionProgress(qreal progress)
{
    const qreal nextProgress = std::clamp(progress, 0.0, 1.0);
    if (qFuzzyCompare(m_transitionProgress + 1.0, nextProgress + 1.0))
        return;
    m_transitionProgress = nextProgress;
    layoutTransitionSlides();
}

int AntCarousel::count() const { return m_slides.size(); }

void AntCarousel::addSlide(QWidget* widget)
{
    if (!widget)
        return;
    widget->setParent(this);
    widget->installEventFilter(this);
    widget->hide();
    m_slides.append(widget);
    updateSlideVisibility();
    updateAutoPlayTimer();
}

void AntCarousel::removeSlide(int index)
{
    if (index < 0 || index >= m_slides.size()) return;
    m_transitionAnimation->stop();
    m_previousIndex = -1;
    m_slides[index]->removeEventFilter(this);
    m_slides[index]->deleteLater();
    m_slides.removeAt(index);
    if (m_currentIndex >= m_slides.size()) m_currentIndex = 0;
    updateSlideVisibility();
    updateAutoPlayTimer();
}

void AntCarousel::clearSlides()
{
    m_transitionAnimation->stop();
    for (auto* w : m_slides)
    {
        w->removeEventFilter(this);
        w->deleteLater();
    }
    m_slides.clear();
    m_currentIndex = 0;
    m_previousIndex = -1;
    m_transitionProgress = 1.0;
    updateDotsOverlay();
    updateAutoPlayTimer();
    update();
}

void AntCarousel::previous()
{
    const int previousIndex = m_currentIndex;
    setCurrentIndex(m_currentIndex - 1);
    if (previousIndex == m_currentIndex)
        resetAutoPlayTimer();
}

void AntCarousel::next()
{
    const int previousIndex = m_currentIndex;
    setCurrentIndex(m_currentIndex + 1);
    if (previousIndex == m_currentIndex)
        resetAutoPlayTimer();
}

bool AntCarousel::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
    {
        auto* slide = qobject_cast<QWidget*>(watched);
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (slide && mouseEvent->button() == Qt::LeftButton && m_slides.contains(slide))
        {
            const QPoint pos = slide->mapTo(this, antEventPositionPoint(mouseEvent));
            handleManualClick(pos, true);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntCarousel::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        updateAutoPlayTimer();
}

void AntCarousel::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    updateSlideVisibility();
    updateAutoPlayTimer();
}

void AntCarousel::hideEvent(QHideEvent* event)
{
    if (m_timer->isActive())
        m_timer->stop();
    m_restartAutoPlayAfterTransition = false;
    syncCarouselPerfCounters();
    QWidget::hideEvent(event);
}

bool AntCarousel::shouldAutoPlayTimerRun() const
{
    return m_autoPlay && isVisible() && isEnabled() && m_slides.size() > 1;
}

bool AntCarousel::isTransitionRunning() const
{
    return m_previousIndex >= 0;
}

void AntCarousel::updateAutoPlayTimer()
{
    const bool shouldRun = shouldAutoPlayTimerRun();
    if (shouldRun && isTransitionRunning())
    {
        if (m_timer->isActive())
            m_timer->stop();
        m_restartAutoPlayAfterTransition = true;
        ++m_autoPlayTimerRefreshCount;
        syncCarouselPerfCounters();
        return;
    }

    if (shouldRun)
    {
        if (!m_timer->isActive())
            m_timer->start(m_interval);
        else if (m_timer->interval() != m_interval)
            m_timer->setInterval(m_interval);
        m_restartAutoPlayAfterTransition = false;
    }
    else if (m_timer->isActive())
    {
        m_timer->stop();
        m_restartAutoPlayAfterTransition = false;
    }
    else
    {
        m_restartAutoPlayAfterTransition = false;
    }

    ++m_autoPlayTimerRefreshCount;
    syncCarouselPerfCounters();
}

void AntCarousel::updateSlideVisibility()
{
    if (isTransitionRunning())
    {
        layoutTransitionSlides();
        return;
    }

    for (int i = 0; i < m_slides.size(); ++i)
    {
        if (i == m_currentIndex)
        {
            if (m_slides[i]->geometry() != rect())
            {
                m_slides[i]->setGeometry(rect());
                ++m_slideGeometryUpdateCount;
            }
            if (m_slides[i]->isHidden())
                m_slides[i]->show();
            m_slides[i]->raise();
        }
        else
        {
            if (!m_slides[i]->isHidden())
                m_slides[i]->hide();
        }
    }
    updateDotsOverlay(false);
    syncCarouselPerfCounters();
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
    if (e->button() == Qt::LeftButton && handleManualClick(antEventPositionPoint(e), false))
    {
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}

void AntCarousel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (isTransitionRunning())
        layoutTransitionSlides();
    else
        updateSlideVisibility();
}

void AntCarousel::updateDotsOverlay(bool repaint)
{
    if (!m_dotsOverlay)
        return;

    const bool shouldShow = m_slides.size() > 1 &&
                            (m_showDots || (m_manualNavigationEnabled && m_showArrows));
    bool needsPaint = repaint;
    if (m_dotsOverlay->geometry() != rect())
    {
        m_dotsOverlay->setGeometry(rect());
        ++m_dotsGeometryUpdateCount;
        needsPaint = true;
    }
    if (m_lastDotsVisible != shouldShow)
    {
        m_dotsOverlay->setVisible(shouldShow);
        m_lastDotsVisible = shouldShow;
        needsPaint = true;
    }
    const int slideCount = m_slides.size();
    if (m_lastDotsCount != slideCount || m_lastDotsActive != m_currentIndex)
    {
        m_lastDotsCount = slideCount;
        m_lastDotsActive = m_currentIndex;
        needsPaint = true;
    }

    if (shouldShow)
    {
        raiseDotsOverlay();
        if (needsPaint)
        {
            ++m_dotsPaintUpdateCount;
            m_dotsOverlay->update();
        }
    }
    syncCarouselPerfCounters();
}

void AntCarousel::raiseDotsOverlay()
{
    if (!m_dotsOverlay || !m_dotsOverlay->isVisible())
        return;
    m_dotsOverlay->raise();
    ++m_dotsRaiseCount;
}

QRect AntCarousel::previousArrowRect() const
{
    const QSize arrowSize(28, 28);
    const int margin = 12;
    return QRect(margin, (height() - arrowSize.height()) / 2, arrowSize.width(), arrowSize.height());
}

QRect AntCarousel::nextArrowRect() const
{
    const QSize arrowSize(28, 28);
    const int margin = 12;
    return QRect(width() - margin - arrowSize.width(),
                 (height() - arrowSize.height()) / 2,
                 arrowSize.width(),
                 arrowSize.height());
}

int AntCarousel::dotIndexAt(const QPoint& pos) const
{
    if (!m_showDots || m_slides.size() <= 1)
        return -1;

    constexpr qreal dotWidth = 16.0;
    constexpr qreal dotActiveWidth = 24.0;
    constexpr qreal dotHeight = 3.0;
    constexpr qreal dotGap = 8.0;
    constexpr qreal dotOffset = 12.0;

    const int slideCount = m_slides.size();
    const qreal totalWidth = dotActiveWidth + (slideCount - 1) * dotWidth + (slideCount - 1) * dotGap;
    qreal x = (width() - totalWidth) / 2.0;
    const qreal y = height() - dotOffset - dotHeight;
    const QRectF hitBand((width() - totalWidth) / 2.0 - dotGap / 2.0,
                         y - 8.0,
                         totalWidth + dotGap,
                         dotHeight + 16.0);
    if (!hitBand.contains(pos))
        return -1;

    for (int i = 0; i < slideCount; ++i)
    {
        const bool active = i == m_currentIndex;
        const qreal w = active ? dotActiveWidth : dotWidth;
        const QRectF hitRect(x - dotGap / 2.0, y - 8.0, w + dotGap, dotHeight + 16.0);
        if (hitRect.contains(pos))
            return i;
        x += w + dotGap;
    }
    return -1;
}

bool AntCarousel::handleManualClick(const QPoint& pos, bool emitSlideClick)
{
    if (m_slides.isEmpty())
        return false;

    if (m_manualNavigationEnabled && m_slides.size() > 1)
    {
        const int dotIndex = dotIndexAt(pos);
        if (dotIndex >= 0)
        {
            const int previousIndex = m_currentIndex;
            setCurrentIndex(dotIndex);
            if (previousIndex == m_currentIndex)
                resetAutoPlayTimer();
            return true;
        }
        if (m_showArrows && previousArrowRect().contains(pos))
        {
            previous();
            return true;
        }
        if (m_showArrows && nextArrowRect().contains(pos))
        {
            next();
            return true;
        }
    }

    if (emitSlideClick)
    {
        emitSlideClicked();
        return true;
    }
    return false;
}

void AntCarousel::emitSlideClicked()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_slides.size())
        return;
    Q_EMIT slideClicked(m_currentIndex, m_slides.at(m_currentIndex));
}

void AntCarousel::resetAutoPlayTimer()
{
    if (!shouldAutoPlayTimerRun())
    {
        if (m_timer->isActive())
            m_timer->stop();
        m_restartAutoPlayAfterTransition = false;
        syncCarouselPerfCounters();
        return;
    }
    if (isTransitionRunning())
    {
        if (m_timer->isActive())
            m_timer->stop();
        m_restartAutoPlayAfterTransition = true;
        ++m_autoPlayTimerRefreshCount;
        syncCarouselPerfCounters();
        return;
    }
    m_timer->start(m_interval);
    m_restartAutoPlayAfterTransition = false;
    ++m_autoPlayTimerRefreshCount;
    syncCarouselPerfCounters();
}

void AntCarousel::pauseAutoPlayTimerForTransition()
{
    if (m_timer->isActive())
    {
        m_timer->stop();
    }
    m_restartAutoPlayAfterTransition = shouldAutoPlayTimerRun();
    if (m_restartAutoPlayAfterTransition)
    {
        ++m_autoPlayTimerRefreshCount;
    }
    syncCarouselPerfCounters();
}

void AntCarousel::startTransition(int from, int to, int requestedIndex)
{
    if (from < 0 || from >= m_slides.size() || to < 0 || to >= m_slides.size() || rect().isEmpty())
    {
        m_previousIndex = -1;
        m_transitionProgress = 1.0;
        updateSlideVisibility();
        resetAutoPlayTimer();
        return;
    }

    pauseAutoPlayTimerForTransition();
    m_transitionAnimation->stop();
    m_previousIndex = from;
    m_transitionDirection = transitionDirection(from, to, requestedIndex);
    m_transitionProgress = 0.0;
    m_lastUpdateMode = QStringLiteral("transition");

    for (int i = 0; i < m_slides.size(); ++i)
    {
        if (i == from || i == to)
            m_slides[i]->show();
        else
            m_slides[i]->hide();
    }

    layoutTransitionSlides();
    updateDotsOverlay(true);
    m_transitionAnimation->setStartValue(0.0);
    m_transitionAnimation->setEndValue(1.0);
    m_transitionAnimation->start();
    syncCarouselPerfCounters();
}

void AntCarousel::layoutTransitionSlides()
{
    if (m_previousIndex < 0 || m_previousIndex >= m_slides.size() || m_currentIndex < 0 || m_currentIndex >= m_slides.size())
        return;

    const QRect base = rect();
    const int dx = qRound(base.width() * m_transitionProgress);
    auto* previousSlide = m_slides[m_previousIndex];
    auto* currentSlide = m_slides[m_currentIndex];
    const QRect previousRect = base.translated(-m_transitionDirection * dx, 0);
    const QRect currentRect = base.translated(m_transitionDirection * (base.width() - dx), 0);

    if (previousSlide->geometry() != previousRect)
    {
        previousSlide->setGeometry(previousRect);
        ++m_slideGeometryUpdateCount;
    }
    if (currentSlide->geometry() != currentRect)
    {
        currentSlide->setGeometry(currentRect);
        ++m_slideGeometryUpdateCount;
    }
    if (previousSlide->isHidden())
        previousSlide->show();
    if (currentSlide->isHidden())
        currentSlide->show();
    currentSlide->raise();
    raiseDotsOverlay();
    syncCarouselPerfCounters();
}

void AntCarousel::finishTransition()
{
    const bool restartAutoPlay = m_restartAutoPlayAfterTransition;
    m_restartAutoPlayAfterTransition = false;
    m_previousIndex = -1;
    m_transitionProgress = 1.0;
    updateSlideVisibility();
    if (restartAutoPlay && shouldAutoPlayTimerRun())
    {
        m_timer->start(m_interval);
        ++m_autoPlayTimerRefreshCount;
        syncCarouselPerfCounters();
    }
}

int AntCarousel::transitionDirection(int from, int to, int requestedIndex) const
{
    const int slideCount = m_slides.size();
    if (requestedIndex < 0)
        return -1;
    if (requestedIndex >= slideCount)
        return 1;
    return to > from ? 1 : -1;
}

void AntCarousel::syncCarouselPerfCounters() const
{
    auto* self = const_cast<AntCarousel*>(this);
    self->setProperty("antCarouselAutoPlayTimerActive", m_timer && m_timer->isActive());
    self->setProperty("antCarouselAutoPlayTimerRefreshCount", m_autoPlayTimerRefreshCount);
    self->setProperty("antCarouselSlideGeometryUpdateCount", m_slideGeometryUpdateCount);
    self->setProperty("antCarouselDotsGeometryUpdateCount", m_dotsGeometryUpdateCount);
    self->setProperty("antCarouselDotsPaintUpdateCount", m_dotsPaintUpdateCount);
    self->setProperty("antCarouselDotsRaiseCount", m_dotsRaiseCount);
    self->setProperty("antCarouselLastUpdateMode", m_lastUpdateMode);
    self->setProperty("antCarouselManualNavigationEnabled", m_manualNavigationEnabled);
    self->setProperty("antCarouselShowArrows", m_showArrows);
    self->setProperty("antCarouselTransitionRunning", isTransitionRunning());
    self->setProperty("antCarouselDeferredAutoPlayRestart", m_restartAutoPlayAfterTransition);
    self->setProperty("antCarouselAutoPlayTimerRemainingMs", m_timer ? m_timer->remainingTime() : -1);
}
