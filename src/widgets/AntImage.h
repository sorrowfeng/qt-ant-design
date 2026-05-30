#pragma once

#include "core/QtAntDesignExport.h"

#include <QWidget>
#include <QPixmap>

class QT_ANT_DESIGN_EXPORT AntImage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString src READ src WRITE setSrc NOTIFY srcChanged)
    Q_PROPERTY(QString alt READ alt WRITE setAlt NOTIFY altChanged)
    Q_PROPERTY(bool preview READ preview WRITE setPreview NOTIFY previewChanged)
    Q_PROPERTY(int imgWidth READ imgWidth WRITE setImgWidth NOTIFY imgWidthChanged)
    Q_PROPERTY(int imgHeight READ imgHeight WRITE setImgHeight NOTIFY imgHeightChanged)

public:
    explicit AntImage(QWidget* parent = nullptr);

    QString src() const;
    void setSrc(const QString& path);

    QString alt() const;
    void setAlt(const QString& text);

    bool preview() const;
    void setPreview(bool enable);

    int imgWidth() const;
    void setImgWidth(int w);

    int imgHeight() const;
    void setImgHeight(int h);

    void setPreviewGroup(const QList<AntImage*>& group);

    QSize sizeHint() const override;

Q_SIGNALS:
    void srcChanged(const QString& path);
    void altChanged(const QString& text);
    void previewChanged(bool enable);
    void imgWidthChanged(int w);
    void imgHeightChanged(int h);
    void clicked();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void enterEvent(AntEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    struct ScaledPixmapCache
    {
        bool valid = false;
        qreal devicePixelRatio = 1.0;
        qreal sourceDevicePixelRatio = 1.0;
        QSize logicalSize;
        Qt::AspectRatioMode aspectMode = Qt::KeepAspectRatio;
        qint64 sourceCacheKey = 0;
        QSize sourceSize;
        QPixmap pixmap;
    };

    struct PreviewOverlayPixmapCache
    {
        bool valid = false;
        qreal devicePixelRatio = 1.0;
        QSize logicalSize;
        QColor textColor;
        QColor overlayColor;
        int fontSize = 0;
        QString fontKey;
        QPixmap pixmap;
    };

    void showPreviewDialog();
    void showPreviewDialogAt(int index);
    QPixmap cachedScaledPixmap(qreal devicePixelRatio, const QSize& targetSize, Qt::AspectRatioMode aspectMode) const;
    QPixmap cachedPreviewOverlayPixmap(qreal devicePixelRatio, const QSize& targetSize) const;
    void invalidateScaledPixmapCache() const;
    void invalidatePreviewOverlayCache() const;
    void requestImageUpdate(const QString& mode, const QRect& dirty = QRect());
    void syncImagePerfCounters() const;

    QList<AntImage*> m_previewGroup;
    QString m_src;
    QString m_alt = QStringLiteral("Image");
    bool m_preview = true;
    int m_imgWidth = 0;
    int m_imgHeight = 0;
    bool m_hovered = false;
    QPixmap m_pixmap;
    bool m_loaded = false;
    mutable ScaledPixmapCache m_scaledPixmapCache;
    mutable PreviewOverlayPixmapCache m_previewOverlayPixmapCache;
    mutable int m_scaledPixmapBuildCount = 0;
    mutable int m_scaledPixmapCacheHitCount = 0;
    mutable int m_previewOverlayPixmapBuildCount = 0;
    mutable int m_previewOverlayPixmapCacheHitCount = 0;
    int m_regionUpdateCount = 0;
    QString m_lastUpdateMode;
};
