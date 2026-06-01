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
    Q_PROPERTY(bool manualNavigationEnabled READ manualNavigationEnabled WRITE setManualNavigationEnabled NOTIFY manualNavigationEnabledChanged)
    Q_PROPERTY(bool showArrows READ showArrows WRITE setShowArrows NOTIFY showArrowsChanged)
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

    bool manualNavigationEnabled() const;
    void setManualNavigationEnabled(bool enabled);

    bool showArrows() const;
    void setShowArrows(bool show);

    int currentIndex() const;
    void setCurrentIndex(int index);
    qreal transitionProgress() const;
    void setTransitionProgress(qreal progress);

    int count() const;
    void addSlide(QWidget* widget);
    void removeSlide(int index);
    void clearSlides();

public Q_SLOTS:
    void previous();
    void next();

Q_SIGNALS:
    void currentIndexChanged(int index);
    void autoPlayChanged(bool autoPlay);
    void intervalChanged(int ms);
    void showDotsChanged(bool show);
    void manualNavigationEnabledChanged(bool enabled);
    void showArrowsChanged(bool show);
    void slideClicked(int index, QWidget* slide);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    bool shouldAutoPlayTimerRun() const;
    bool isTransitionRunning() const;
    void updateAutoPlayTimer();
    void updateSlideVisibility();
    void updateDotsOverlay(bool repaint = true);
    void raiseDotsOverlay();
    QRect previousArrowRect() const;
    QRect nextArrowRect() const;
    int dotIndexAt(const QPoint& pos) const;
    bool handleManualClick(const QPoint& pos, bool emitSlideClick);
    void emitSlideClicked();
    void resetAutoPlayTimer();
    void pauseAutoPlayTimerForTransition();
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
    bool m_manualNavigationEnabled = true;
    bool m_showArrows = true;
    bool m_restartAutoPlayAfterTransition = false;
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
