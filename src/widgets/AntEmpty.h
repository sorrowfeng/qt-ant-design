#pragma once

#include "core/QtAntDesignExport.h"

#include <QPointer>
#include <QRect>
#include <QSize>
#include <QWidget>

class AntEmptyStyle;
class QEvent;

class QT_ANT_DESIGN_EXPORT AntEmpty : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(bool imageVisible READ imageVisible WRITE setImageVisible NOTIFY imageVisibleChanged)
    Q_PROPERTY(bool simple READ isSimple WRITE setSimple NOTIFY simpleChanged)
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)

public:
    explicit AntEmpty(QWidget* parent = nullptr);

    QString description() const;
    void setDescription(const QString& description);

    bool imageVisible() const;
    void setImageVisible(bool visible);

    bool isSimple() const;
    void setSimple(bool simple);

    QSize imageSize() const;
    void setImageSize(const QSize& size);

    QWidget* extraWidget() const;
    void setExtraWidget(QWidget* widget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void descriptionChanged(const QString& description);
    void imageVisibleChanged(bool visible);
    void simpleChanged(bool simple);
    void imageSizeChanged(const QSize& size);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    friend class AntEmptyStyle;

    struct EmptyLayoutCache
    {
        bool valid = false;
        QSize widgetSize;
        bool imageVisible = true;
        bool simple = false;
        QSize imageSize;
        QString description;
        QSize extraSize;
        QSize effectiveImageSize;
        QRect imageRect;
        QRect descriptionRect;
        QRect extraRect;
    };

    QRect imageRect() const;
    QRect descriptionRect() const;
    QRect extraRect() const;
    void syncExtraGeometry();
    QSize effectiveImageSize() const;
    const EmptyLayoutCache& emptyLayoutCache(const QSize& widgetSize) const;
    QSize cachedSizeHint() const;
    void invalidateEmptyCaches() const;
    void requestEmptyUpdate(const QRect& region, const QString& mode);
    void syncEmptyPerfCounters() const;

    QString m_description = QStringLiteral("No data");
    bool m_imageVisible = true;
    bool m_simple = false;
    QSize m_imageSize;
    QPointer<QWidget> m_extraWidget;
    mutable EmptyLayoutCache m_layoutCache;
    mutable bool m_sizeHintDirty = true;
    mutable QSize m_cachedSizeHint;
    mutable int m_layoutCacheBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    mutable int m_sizeHintBuildCount = 0;
    mutable int m_sizeHintHitCount = 0;
    mutable int m_illustrationPixmapBuildCount = 0;
    mutable int m_illustrationPixmapHitCount = 0;
    int m_regionUpdateCount = 0;
    int m_extraGeometryUpdateCount = 0;
    QString m_lastUpdateMode;
};
