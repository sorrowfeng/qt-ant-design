#pragma once

#include "core/QtAntDesignExport.h"

#include <QHash>
#include <QMetaType>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

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

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    friend class AntUploadStyle;

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

    const UploadLayout& uploadLayout() const;
    void invalidateUploadLayout() const;
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
    void openFilePreview(const AntUploadFile& file) const;
    QPixmap cachedThumbPixmap(const QString& path) const;
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
    mutable QHash<QString, QPixmap> m_thumbPixmapCache;
    mutable int m_thumbPixmapBuildCount = 0;
    mutable int m_thumbPixmapCacheHitCount = 0;
    int m_regionUpdateCount = 0;
    int m_itemRegionUpdateCount = 0;
    int m_triggerRegionUpdateCount = 0;
    int m_progressRegionUpdateCount = 0;
};
