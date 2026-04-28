#pragma once

#include <QWidget>

class QTimer;
class QResizeEvent;

class AntCarousel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool autoPlay READ autoPlay WRITE setAutoPlay NOTIFY autoPlayChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    Q_PROPERTY(bool showDots READ showDots WRITE setShowDots NOTIFY showDotsChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

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
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void updateSlideVisibility();
    void updateDotsOverlay();

    QList<QWidget*> m_slides;
    QTimer* m_timer = nullptr;
    QWidget* m_dotsOverlay = nullptr;
    bool m_autoPlay = true;
    int m_interval = 3000;
    bool m_showDots = true;
    int m_currentIndex = 0;
};
