#pragma once

#include "core/QtAntDesignExport.h"

#include <QHash>
#include <QMetaType>
#include <QPixmap>
#include <QRect>
#include <QSet>
#include <QSize>
#include <QUrl>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class TestAntDataEntryB;

struct AntUploadFile
{
    QString uid;
    QString name;
    Ant::UploadFileStatus status = Ant::UploadFileStatus::Uploading;
    int percent = 0;
    QString thumbUrl;
    QString url;
    qint64 size = 0;
};

Q_DECLARE_METATYPE(AntUploadFile)

class QT_ANT_DESIGN_EXPORT AntUpload : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString accept READ accept WRITE setAccept NOTIFY acceptChanged)
    Q_PROPERTY(bool multiple READ isMultiple WRITE setMultiple NOTIFY multipleChanged)
    Q_PROPERTY(int maxCount READ maxCount WRITE setMaxCount NOTIFY maxCountChanged)
    Q_PROPERTY(bool disabled READ isDisabled WRITE setDisabled NOTIFY disabledChanged)
    Q_PROPERTY(Ant::UploadListType listType READ listType WRITE setListType NOTIFY listTypeChanged)
    Q_PROPERTY(bool draggerMode READ isDraggerMode WRITE setDraggerMode NOTIFY draggerModeChanged)
    Q_PROPERTY(int thumbnailCacheBudgetBytes READ thumbnailCacheBudgetBytes WRITE setThumbnailCacheBudgetBytes NOTIFY thumbnailCacheBudgetBytesChanged)

public:
    explicit AntUpload(QWidget* parent = nullptr);

    QString accept() const;
    void setAccept(const QString& accept);

    bool isMultiple() const;
    void setMultiple(bool multiple);

    int maxCount() const;
    void setMaxCount(int maxCount);

    bool isDisabled() const;
    void setDisabled(bool disabled);

    Ant::UploadListType listType() const;
    void setListType(Ant::UploadListType type);

    bool isDraggerMode() const;
    void setDraggerMode(bool dragger);

    void addFile(const AntUploadFile& file);
    void removeFile(const QString& uid);
    void updateFileStatus(const QString& uid, Ant::UploadFileStatus status, int percent = -1);
    void setFileList(const QVector<AntUploadFile>& files);
    QVector<AntUploadFile> fileList() const;

    int thumbnailCacheBudgetBytes() const;
    void setThumbnailCacheBudgetBytes(int bytes);
    qint64 thumbnailCacheBytes() const;
    QString thumbnailError(const QString& path) const;
    // QFile metadata cannot detect an in-place rewrite that deliberately keeps
    // both size and timestamp. Call this after such a rewrite to force reload.
    void invalidateThumbnail(const QString& path) const;
    void clearThumbnailCache() const;
    bool requestFilePreview(const QString& uid);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void acceptChanged(const QString& accept);
    void multipleChanged(bool multiple);
    void maxCountChanged(int maxCount);
    void disabledChanged(bool disabled);
    void listTypeChanged(Ant::UploadListType type);
    void draggerModeChanged(bool dragger);
    void fileAdded(const AntUploadFile& file);
    void fileRemoved(const QString& uid);
    void fileStatusChanged(const QString& uid, Ant::UploadFileStatus status);
    void uploadRequested();
    void thumbnailCacheBudgetBytesChanged(int bytes);
    void thumbnailLoadFailed(const QString& path, const QString& error);
    void localFilePreviewRequested(const QString& path);
    void externalPreviewBlocked(const QUrl& url);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    friend class AntUploadStyle;
    friend class TestAntDataEntryB;

    struct UploadLayout
    {
        QSize widgetSize;
        Ant::UploadListType listType = Ant::UploadListType::Text;
        bool draggerMode = false;
        int maxCount = 0;
        int fileCount = 0;
        QRect triggerRect;
        QVector<QRect> fileItemRects;
        QVector<QRect> fileCardRects;
        QVector<QRect> fileCardPreviewRects;
        QVector<QRect> fileCardRemoveRects;
        QSize sizeHint;
        QSize minimumSizeHint;
        bool valid = false;
    };

    struct ThumbnailCacheEntry
    {
        QString normalizedPath;
        qint64 sourceBytes = -1;
        qint64 sourceModifiedMs = 0;
        QSize physicalSize;
        qreal devicePixelRatio = 1.0;
        QPixmap pixmap;
        qint64 bytes = 0;
        quint64 lastAccess = 0;
    };

    struct ThumbnailFailureEntry
    {
        QString normalizedPath;
        qint64 sourceBytes = -1;
        qint64 sourceModifiedMs = 0;
        QString error;
    };

    const UploadLayout& uploadLayout() const;
    void invalidateUploadLayout() const;
    void syncDisabledState(bool disabled);
    QRect triggerRect() const;
    QRect fileItemRect(int index) const;
    QRect fileItemRemoveButtonRect(int index) const;
    QRect fileCardRect(int index) const;
    QRect fileCardPreviewButtonRect(int index) const;
    QRect fileCardRemoveButtonRect(int index) const;
    QRect dirtyRectForIndex(int index) const;
    QRect dirtyRectFromIndex(int index) const;
    bool isOverRemoveButton(const QPoint& pos) const;
    bool canAcceptMoreFiles() const;
    bool fileMatchesAccept(const QString& path) const;
    QString dialogNameFilter() const;
    void requestUploadFiles();
    void addLocalFiles(const QStringList& paths);
    bool openFilePreview(const AntUploadFile& file);
    QPixmap cachedThumbPixmap(const QString& path, const QSize& logicalSize, qreal devicePixelRatio) const;
    void evictThumbnailSource(const QString& path) const;
    void enforceThumbnailCacheBudget() const;
    void recordThumbnailFailure(const QString& path,
                                const QString& normalizedPath,
                                qint64 sourceBytes,
                                qint64 sourceModifiedMs,
                                const QString& error) const;
    void updateUploadRegion(const QRect& dirty,
                            const QString& mode,
                            bool itemScoped = false,
                            bool triggerScoped = false,
                            bool progressScoped = false);
    void syncUploadPerfCounters() const;

    QString m_accept;
    bool m_multiple = false;
    int m_maxCount = 0;
    bool m_disabled = false;
    Ant::UploadListType m_listType = Ant::UploadListType::Text;
    bool m_draggerMode = false;
    bool m_dragOver = false;
    QVector<AntUploadFile> m_files;

    QPoint m_mousePos;
    int m_hoveredItemIndex = -1;
    bool m_triggerHovered = false;
    mutable UploadLayout m_layoutCache;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    int m_thumbnailCacheBudgetBytes = 16 * 1024 * 1024;
    mutable QHash<QString, ThumbnailCacheEntry> m_thumbPixmapCache;
    mutable QHash<QString, ThumbnailFailureEntry> m_thumbLoadErrors;
    mutable QSet<QString> m_pendingThumbnailFailureSignals;
    mutable qint64 m_thumbnailCacheBytes = 0;
    mutable quint64 m_thumbnailAccessClock = 0;
    mutable int m_thumbPixmapBuildCount = 0;
    mutable int m_thumbPixmapCacheHitCount = 0;
    mutable int m_thumbPixmapEvictionCount = 0;
    mutable int m_thumbPixmapFailureCount = 0;
    mutable int m_thumbPixmapSourceChangeCount = 0;
    int m_regionUpdateCount = 0;
    int m_itemRegionUpdateCount = 0;
    int m_triggerRegionUpdateCount = 0;
    int m_progressRegionUpdateCount = 0;
};
