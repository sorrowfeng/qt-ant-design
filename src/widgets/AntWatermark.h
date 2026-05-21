#pragma once

#include "core/QtAntDesignExport.h"
#include "core/AntTypes.h"

#include <QColor>
#include <QFont>
#include <QPoint>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QWidget>

class AntWatermarkStyle;
class QPaintEvent;
class QPainter;

struct AntWatermarkFont
{
    QColor color;
    int fontSize = 16;
    int fontWeight = 400;
    QString fontFamily;

    bool operator==(const AntWatermarkFont& other) const
    {
        return color == other.color && fontSize == other.fontSize &&
               fontWeight == other.fontWeight && fontFamily == other.fontFamily;
    }
    bool operator!=(const AntWatermarkFont& other) const { return !(*this == other); }
};

class QT_ANT_DESIGN_EXPORT AntWatermark : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(qreal rotate READ rotate WRITE setRotate NOTIFY rotateChanged)

public:
    explicit AntWatermark(QWidget* parent = nullptr);

    QStringList content() const;
    void setContent(const QStringList& lines);
    void setContent(const QString& text);

    qreal rotate() const;
    void setRotate(qreal degrees);

    AntWatermarkFont watermarkFont() const;
    void setWatermarkFont(const AntWatermarkFont& f);

    QSize gap() const;
    void setGap(const QSize& gap);

    QPoint offset() const;
    void setOffset(const QPoint& offset);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void contentChanged(const QStringList&);
    void rotateChanged(qreal);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    friend class AntWatermarkStyle;

    struct WatermarkPixmapCache
    {
        bool valid = false;
        QSize widgetSize;
        qreal devicePixelRatio = 1.0;
        Ant::ThemeMode themeMode = Ant::ThemeMode::Default;
        QStringList content;
        AntWatermarkFont font;
        qreal rotate = -22.0;
        QSize gap;
        QPoint offset;
        QColor resolvedColor;
        QPixmap pixmap;
    };

    QPixmap watermarkPixmap(qreal devicePixelRatio) const;
    QSize watermarkTextSize(QPainter* painter, const QFont& font) const;
    void drawWatermarkPattern(QPainter* painter, const QPointF& center, const QFont& font, const QColor& color) const;
    QColor resolvedTextColor() const;
    void invalidateWatermarkPixmap() const;
    void requestWatermarkUpdate(const QString& mode);
    void syncWatermarkPerfCounters() const;

    QStringList m_content;
    qreal m_rotate = -22.0;
    AntWatermarkFont m_font;
    QSize m_gap = QSize(100, 100);
    QPoint m_offset = QPoint(0, 0);
    mutable WatermarkPixmapCache m_pixmapCache;
    mutable int m_pixmapBuildCount = 0;
    mutable int m_pixmapCacheHitCount = 0;
    int m_regionUpdateCount = 0;
    QString m_lastUpdateMode;
};
