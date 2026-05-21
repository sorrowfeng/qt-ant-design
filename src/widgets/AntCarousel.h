#pragma once

#include "core/QtAntDesignExport.h"

#include <QWidget>

class QPropertyAnimation;
class QTimer;
class QEvent;
class QHideEvent;
class QResizeEvent;
class QShowEvent;

class QT_ANT_DESIGN_EXPORT AntCarousel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool autoPlay READ autoPlay WRITE setAutoPlay NOTIFY autoPlayChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    Q_PROPERTY(bool showDots READ showDots WRITE setShowDots NOTIFY showDotsChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(qreal transitionProgress READ transitionProgress WRITE setTransitionProgress)

public:
    explicit AntCarousel(QWidget* parent = nullptr);

    bool autoPlay() const;
    void setAutoPlay(bool autoPlay);

    int interval() const;
    void setInterval(int ms);

    bool showDots() const;
    void setShowDots(bool show);

    int currentIndex() const;
    void setCurrentIndex(int index);
    qreal transitionProgress() const;
    void setTransitionProgress(qreal progress);

    int count() const;
    void addSlide(QWidget* widget);
    void removeSlide(int index);
    void clearSlides();

Q_SIGNALS:
    void currentIndexChanged(int index);
    void autoPlayChanged(bool autoPlay);
    void intervalChanged(int ms);
    void showDotsChanged(bool show);

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateAutoPlayTimer();
    void updateSlideVisibility();
    void updateDotsOverlay(bool repaint = true);
    void raiseDotsOverlay();
    void startTransition(int from, int to, int requestedIndex);
    void layoutTransitionSlides();
    void finishTransition();
    int transitionDirection(int from, int to, int requestedIndex) const;
    void syncCarouselPerfCounters() const;

    QList<QWidget*> m_slides;
    QTimer* m_timer = nullptr;
    QPropertyAnimation* m_transitionAnimation = nullptr;
    QWidget* m_dotsOverlay = nullptr;
    bool m_autoPlay = true;
    int m_interval = 3000;
    bool m_showDots = true;
    int m_currentIndex = 0;
    int m_previousIndex = -1;
    int m_transitionDirection = 1;
    qreal m_transitionProgress = 1.0;
    int m_autoPlayTimerRefreshCount = 0;
    int m_slideGeometryUpdateCount = 0;
    int m_dotsGeometryUpdateCount = 0;
    int m_dotsPaintUpdateCount = 0;
    int m_dotsRaiseCount = 0;
    bool m_lastDotsVisible = false;
    int m_lastDotsCount = -1;
    int m_lastDotsActive = -1;
    QString m_lastUpdateMode;
};
