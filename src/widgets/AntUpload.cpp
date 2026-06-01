#include "AntUpload.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMimeType>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QUrl>

#include "AntFileDialog.h"
#include "core/AntTheme.h"
#include "styles/AntUploadStyle.h"

namespace
{
constexpr int TriggerHeight = 32;
constexpr int TriggerMinWidth = 160;
constexpr int FileItemHeight = 28;
constexpr int PictureItemHeight = 48;
constexpr int CardSize = 100;
constexpr int CardGap = 8;
constexpr int GridColumns = 4;
} // namespace

AntUpload::AntUpload(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntUploadStyle>(this);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    syncUploadPerfCounters();
}

QString AntUpload::accept() const { return m_accept; }

void AntUpload::setAccept(const QString& accept)
{
    if (m_accept == accept)
    {
        return;
    }
    m_accept = accept;
    Q_EMIT acceptChanged(m_accept);
}

bool AntUpload::isMultiple() const { return m_multiple; }

void AntUpload::setMultiple(bool multiple)
{
    if (m_multiple == multiple)
    {
        return;
    }
    m_multiple = multiple;
    Q_EMIT multipleChanged(m_multiple);
}

int AntUpload::maxCount() const { return m_maxCount; }

void AntUpload::setMaxCount(int maxCount)
{
    if (m_maxCount == maxCount)
    {
        return;
    }
    const QRect oldTrigger = triggerRect();
    m_maxCount = maxCount;
    invalidateUploadLayout();
    updateGeometry();
    updateUploadRegion(oldTrigger.united(triggerRect()), QStringLiteral("maxCount"), false, true);
    Q_EMIT maxCountChanged(m_maxCount);
}

bool AntUpload::isDisabled() const { return m_disabled; }

void AntUpload::setDisabled(bool disabled)
{
    if (m_disabled == disabled)
    {
        return;
    }
    m_disabled = disabled;
    setAcceptDrops(m_draggerMode && !m_disabled);
    updateUploadRegion(rect(), QStringLiteral("disabled"));
    Q_EMIT disabledChanged(m_disabled);
}

Ant::UploadListType AntUpload::listType() const { return m_listType; }

void AntUpload::setListType(Ant::UploadListType type)
{
    if (m_listType == type)
    {
        return;
    }
    m_listType = type;
    invalidateUploadLayout();
    updateGeometry();
    updateUploadRegion(rect(), QStringLiteral("listType"));
    Q_EMIT listTypeChanged(m_listType);
}

bool AntUpload::isDraggerMode() const { return m_draggerMode; }

void AntUpload::setDraggerMode(bool dragger)
{
    if (m_draggerMode == dragger)
        return;
    m_draggerMode = dragger;
    setAcceptDrops(dragger && !m_disabled);
    invalidateUploadLayout();
    updateGeometry();
    updateUploadRegion(rect(), QStringLiteral("dragger"));
    Q_EMIT draggerModeChanged(m_draggerMode);
}

void AntUpload::dragEnterEvent(QDragEnterEvent* event)
{
    if (m_draggerMode && !m_disabled && canAcceptMoreFiles() && event->mimeData()->hasUrls())
    {
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl& url : urls)
        {
            if (url.isLocalFile() && fileMatchesAccept(url.toLocalFile()))
            {
                m_dragOver = true;
                updateUploadRegion(triggerRect(), QStringLiteral("dragEnter"), false, true);
                event->acceptProposedAction();
                return;
            }
        }
    }
    QWidget::dragEnterEvent(event);
}

void AntUpload::dragLeaveEvent(QDragLeaveEvent* event)
{
    if (m_dragOver)
    {
        m_dragOver = false;
        updateUploadRegion(triggerRect(), QStringLiteral("dragLeave"), false, true);
    }
    QWidget::dragLeaveEvent(event);
}

void AntUpload::dropEvent(QDropEvent* event)
{
    const bool hadDragOver = m_dragOver;
    m_dragOver = false;
    if (m_draggerMode && !m_disabled && canAcceptMoreFiles() && event->mimeData()->hasUrls())
    {
        QStringList paths;
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl& url : urls)
        {
            if (url.isLocalFile())
            {
                paths.append(url.toLocalFile());
            }
        }
        addLocalFiles(paths);
        event->acceptProposedAction();
        if (hadDragOver)
        {
            updateUploadRegion(triggerRect(), QStringLiteral("drop"), false, true);
        }
    }
    else
    {
        QWidget::dropEvent(event);
    }
}

void AntUpload::addFile(const AntUploadFile& file)
{
    const QRect oldTrigger = triggerRect();
    const int newIndex = m_files.size();
    m_files.append(file);
    invalidateUploadLayout();
    updateGeometry();
    updateUploadRegion(oldTrigger.united(dirtyRectForIndex(newIndex)).united(triggerRect()),
                       QStringLiteral("addFile"),
                       true,
                       true);
    Q_EMIT fileAdded(file);
}

void AntUpload::removeFile(const QString& uid)
{
    for (int i = 0; i < m_files.size(); ++i)
    {
        if (m_files[i].uid == uid)
        {
            const QRect dirty = dirtyRectFromIndex(i).united(triggerRect());
            m_files.remove(i);
            if (m_hoveredItemIndex >= m_files.size())
            {
                m_hoveredItemIndex = -1;
            }
            invalidateUploadLayout();
            updateGeometry();
            updateUploadRegion(dirty.united(triggerRect()), QStringLiteral("removeFile"), true, true);
            Q_EMIT fileRemoved(uid);
            return;
        }
    }
}

void AntUpload::updateFileStatus(const QString& uid, Ant::UploadFileStatus status, int percent)
{
    for (int i = 0; i < m_files.size(); ++i)
    {
        auto& file = m_files[i];
        if (file.uid == uid)
        {
            const int nextPercent = percent >= 0 ? qBound(0, percent, 100) : file.percent;
            if (file.status == status && file.percent == nextPercent)
            {
                return;
            }
            file.status = status;
            file.percent = nextPercent;
            updateUploadRegion(dirtyRectForIndex(i), QStringLiteral("status"), true, false, true);
            Q_EMIT fileStatusChanged(uid, status);
            return;
        }
    }
}

void AntUpload::setFileList(const QVector<AntUploadFile>& files)
{
    m_files = files;
    m_hoveredItemIndex = -1;
    invalidateUploadLayout();
    updateGeometry();
    updateUploadRegion(rect(), QStringLiteral("setFileList"));
}

QVector<AntUploadFile> AntUpload::fileList() const
{
    return m_files;
}

QSize AntUpload::sizeHint() const
{
    return uploadLayout().sizeHint;
}

QSize AntUpload::minimumSizeHint() const
{
    return uploadLayout().minimumSizeHint;
}

void AntUpload::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntUpload::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = event->pos();
    const QPoint oldMousePos = m_mousePos;
    const int oldHoveredItem = m_hoveredItemIndex;
    const bool oldTriggerHovered = m_triggerHovered;
    const bool oldRemoveHovered = oldHoveredItem >= 0 && fileItemRemoveButtonRect(oldHoveredItem).contains(oldMousePos);

    int nextHoveredItem = -1;
    bool nextTriggerHovered = false;
    if (m_listType == Ant::UploadListType::PictureCard)
    {
        nextTriggerHovered = triggerRect().contains(pos);
        for (int i = 0; i < m_files.size(); ++i)
        {
            if (fileCardRect(i).contains(pos))
            {
                nextHoveredItem = i;
                break;
            }
        }
    }
    else
    {
        nextTriggerHovered = triggerRect().contains(pos);
        for (int i = 0; i < m_files.size(); ++i)
        {
            if (fileItemRect(i).contains(pos))
            {
                nextHoveredItem = i;
                break;
            }
        }
    }

    const bool nextRemoveHovered = nextHoveredItem >= 0 && fileItemRemoveButtonRect(nextHoveredItem).contains(pos);
    m_mousePos = pos;
    m_triggerHovered = nextTriggerHovered;
    m_hoveredItemIndex = nextHoveredItem;

    QRect dirty;
    bool itemScoped = false;
    bool triggerScoped = false;
    if (oldTriggerHovered != nextTriggerHovered)
    {
        dirty = dirty.united(triggerRect());
        triggerScoped = true;
    }
    if (oldHoveredItem != nextHoveredItem || oldRemoveHovered != nextRemoveHovered)
    {
        dirty = dirty.united(dirtyRectForIndex(oldHoveredItem)).united(dirtyRectForIndex(nextHoveredItem));
        itemScoped = oldHoveredItem >= 0 || nextHoveredItem >= 0;
    }
    if (dirty.isValid() && !dirty.isEmpty())
    {
        updateUploadRegion(dirty, QStringLiteral("hover"), itemScoped, triggerScoped);
    }
    QWidget::mouseMoveEvent(event);
}

void AntUpload::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || m_disabled)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPoint pos = event->pos();

    if (m_listType == Ant::UploadListType::PictureCard)
    {
        const int totalSlots = m_files.size() + 1;
        for (int i = 0; i < totalSlots; ++i)
        {
            if (fileCardRect(i).contains(pos))
            {
                if (i == m_files.size())
                {
                    if (canAcceptMoreFiles())
                    {
                        requestUploadFiles();
                    }
                }
                else if (fileCardRemoveButtonRect(i).contains(pos))
                {
                    removeFile(m_files[i].uid);
                }
                else if (fileCardPreviewButtonRect(i).contains(pos))
                {
                    openFilePreview(m_files[i]);
                }
                else if (!m_files[i].url.isEmpty() || !m_files[i].thumbUrl.isEmpty())
                {
                    openFilePreview(m_files[i]);
                }
                else
                {
                    if (canAcceptMoreFiles())
                    {
                        requestUploadFiles();
                    }
                }
                break;
            }
        }
        event->accept();
        return;
    }

    for (int i = 0; i < m_files.size(); ++i)
    {
        const QRect itemRect = fileItemRect(i);
        if (!itemRect.contains(pos))
        {
            continue;
        }

        if (fileItemRemoveButtonRect(i).contains(pos))
        {
            const QString uid = m_files[i].uid;
            removeFile(uid);
            event->accept();
            return;
        }
        break;
    }

    if (triggerRect().contains(pos))
    {
        requestUploadFiles();
    }

    event->accept();
}

void AntUpload::leaveEvent(QEvent* event)
{
    QRect dirty;
    bool itemScoped = false;
    bool triggerScoped = false;
    if (m_triggerHovered)
    {
        dirty = dirty.united(triggerRect());
        triggerScoped = true;
    }
    if (m_hoveredItemIndex >= 0)
    {
        dirty = dirty.united(dirtyRectForIndex(m_hoveredItemIndex));
        itemScoped = true;
    }
    m_triggerHovered = false;
    m_hoveredItemIndex = -1;
    if (dirty.isValid() && !dirty.isEmpty())
    {
        updateUploadRegion(dirty, QStringLiteral("leave"), itemScoped, triggerScoped);
    }
    QWidget::leaveEvent(event);
}

void AntUpload::resizeEvent(QResizeEvent* event)
{
    invalidateUploadLayout();
    QWidget::resizeEvent(event);
}

const AntUpload::UploadLayout& AntUpload::uploadLayout() const
{
    if (m_layoutCache.valid
        && m_layoutCache.widgetSize == size()
        && m_layoutCache.listType == m_listType
        && m_layoutCache.draggerMode == m_draggerMode
        && m_layoutCache.maxCount == m_maxCount
        && m_layoutCache.fileCount == m_files.size())
    {
        ++m_layoutCacheHitCount;
        syncUploadPerfCounters();
        return m_layoutCache;
    }

    UploadLayout layout;
    layout.widgetSize = size();
    layout.listType = m_listType;
    layout.draggerMode = m_draggerMode;
    layout.maxCount = m_maxCount;
    layout.fileCount = m_files.size();

    const int contentWidth = qMax(320, width());
    if (m_draggerMode)
    {
        layout.triggerRect = rect();
        layout.sizeHint = QSize(contentWidth, 150);
        layout.minimumSizeHint = QSize(TriggerMinWidth, TriggerHeight);
    }
    else if (m_listType == Ant::UploadListType::PictureCard)
    {
        const int cardAreaWidth = GridColumns * CardSize + (GridColumns - 1) * CardGap;
        const bool canAdd = m_maxCount <= 0 || m_files.size() < m_maxCount;
        const int totalSlots = m_files.size() + (canAdd ? 1 : 0);
        const int rows = totalSlots > 0 ? (totalSlots + GridColumns - 1) / GridColumns : 1;
        const int h = rows * CardSize + (rows - 1) * CardGap;
        layout.sizeHint = QSize(qMax(contentWidth, cardAreaWidth), h);
        layout.minimumSizeHint = QSize(CardSize, CardSize);
        layout.fileCardRects.reserve(m_files.size());
        layout.fileCardPreviewRects.reserve(m_files.size());
        layout.fileCardRemoveRects.reserve(m_files.size());
        for (int i = 0; i < m_files.size(); ++i)
        {
            const int col = i % GridColumns;
            const int row = i / GridColumns;
            const QRect cardRect(col * (CardSize + CardGap), row * (CardSize + CardGap), CardSize, CardSize);
            layout.fileCardRects.append(cardRect);
            const int size = 24;
            const int gap = 8;
            const int y = cardRect.center().y() - size / 2;
            layout.fileCardPreviewRects.append(QRect(cardRect.center().x() - size - gap / 2, y, size, size));
            layout.fileCardRemoveRects.append(QRect(cardRect.center().x() + gap / 2, y, size, size));
        }
        if (canAdd)
        {
            const int triggerIndex = m_files.size();
            const int col = triggerIndex % GridColumns;
            const int row = triggerIndex / GridColumns;
            layout.triggerRect = QRect(col * (CardSize + CardGap), row * (CardSize + CardGap), CardSize, CardSize);
        }
    }
    else
    {
        layout.triggerRect = QRect(0, 0, qMin(width(), TriggerMinWidth), TriggerHeight);
        const int itemH = m_listType == Ant::UploadListType::Picture ? PictureItemHeight : FileItemHeight;
        int y = layout.triggerRect.bottom() + 9;
        layout.fileItemRects.reserve(m_files.size());
        for (int i = 0; i < m_files.size(); ++i)
        {
            layout.fileItemRects.append(QRect(0, y, width(), itemH));
            y += itemH;
        }
        layout.sizeHint = QSize(contentWidth, TriggerHeight + 16 + itemH * m_files.size());
        layout.minimumSizeHint = QSize(TriggerMinWidth, TriggerHeight);
    }

    layout.valid = true;
    m_layoutCache = layout;
    ++m_layoutBuildCount;
    syncUploadPerfCounters();
    return m_layoutCache;
}

void AntUpload::invalidateUploadLayout() const
{
    m_layoutCache.valid = false;
    syncUploadPerfCounters();
}

QRect AntUpload::triggerRect() const
{
    return uploadLayout().triggerRect;
}

QRect AntUpload::fileItemRect(int index) const
{
    const auto& layout = uploadLayout();
    if (index < 0 || index >= layout.fileItemRects.size())
    {
        return QRect();
    }
    return layout.fileItemRects.at(index);
}

QRect AntUpload::fileItemRemoveButtonRect(int index) const
{
    const QRect itemRect = fileItemRect(index);
    if (itemRect.isEmpty())
    {
        return QRect();
    }
    const int removeBtnSize = 14;
    return QRect(itemRect.right() - 8 - removeBtnSize,
                 itemRect.top() + (itemRect.height() - removeBtnSize) / 2,
                 removeBtnSize,
                 removeBtnSize);
}

QRect AntUpload::fileCardRect(int index) const
{
    const auto& layout = uploadLayout();
    if (index == m_files.size())
    {
        return layout.triggerRect;
    }
    if (index < 0 || index >= layout.fileCardRects.size())
    {
        return QRect();
    }
    return layout.fileCardRects.at(index);
}

bool AntUpload::isOverRemoveButton(const QPoint& pos) const
{
    for (int i = 0; i < m_files.size(); ++i)
    {
        if (!fileItemRect(i).contains(pos))
        {
            continue;
        }
        return fileItemRemoveButtonRect(i).contains(pos);
    }
    return false;
}

QRect AntUpload::fileCardPreviewButtonRect(int index) const
{
    const auto& layout = uploadLayout();
    if (index < 0 || index >= layout.fileCardPreviewRects.size())
    {
        return {};
    }
    return layout.fileCardPreviewRects.at(index);
}

QRect AntUpload::fileCardRemoveButtonRect(int index) const
{
    const auto& layout = uploadLayout();
    if (index < 0 || index >= layout.fileCardRemoveRects.size())
    {
        return {};
    }
    return layout.fileCardRemoveRects.at(index);
}

QRect AntUpload::dirtyRectForIndex(int index) const
{
    if (m_listType == Ant::UploadListType::PictureCard)
    {
        return fileCardRect(index);
    }
    return fileItemRect(index);
}

QRect AntUpload::dirtyRectFromIndex(int index) const
{
    const QRect first = dirtyRectForIndex(index);
    if (!first.isValid() || first.isEmpty())
    {
        return QRect();
    }
    if (m_listType == Ant::UploadListType::PictureCard)
    {
        return QRect(0, first.top(), width(), qMax(0, height() - first.top()));
    }
    return QRect(0, first.top(), width(), qMax(0, height() - first.top()));
}

bool AntUpload::canAcceptMoreFiles() const
{
    return !m_disabled && (m_maxCount <= 0 || m_files.size() < m_maxCount);
}

bool AntUpload::fileMatchesAccept(const QString& path) const
{
    const QString accept = m_accept.trimmed();
    if (accept.isEmpty())
    {
        return true;
    }

    const QFileInfo info(path);
    const QString suffix = QStringLiteral(".") + info.suffix().toLower();
    const QMimeType mime = QMimeDatabase().mimeTypeForFile(path);
    const QStringList rules = accept.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (QString rule : rules)
    {
        rule = rule.trimmed().toLower();
        if (rule.isEmpty())
        {
            continue;
        }
        if (rule.startsWith(QLatin1Char('.')) && suffix == rule)
        {
            return true;
        }
        if (rule.endsWith(QStringLiteral("/*")))
        {
            const QString prefix = rule.left(rule.size() - 1);
            if (mime.name().startsWith(prefix))
            {
                return true;
            }
        }
        if (rule.contains(QLatin1Char('/')) && (mime.name() == rule || mime.inherits(rule)))
        {
            return true;
        }
    }
    return false;
}

QString AntUpload::dialogNameFilter() const
{
    QStringList patterns;
    const QStringList rules = m_accept.split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (QString rule : rules)
    {
        rule = rule.trimmed();
        if (rule.startsWith(QLatin1Char('.')) && rule.size() > 1)
        {
            patterns.append(QStringLiteral("*") + rule);
        }
    }
    if (patterns.isEmpty())
    {
        return QStringLiteral("All files (*)");
    }
    return QStringLiteral("Accepted files (%1)").arg(patterns.join(QLatin1Char(' ')));
}

void AntUpload::requestUploadFiles()
{
    if (!canAcceptMoreFiles())
    {
        return;
    }

    const bool hasExternalHandler = receivers(SIGNAL(uploadRequested())) > 0;
    Q_EMIT uploadRequested();
    if (hasExternalHandler)
    {
        return;
    }

    QStringList paths;
    const QString filter = dialogNameFilter();
    if (m_multiple)
    {
        paths = AntFileDialog::getOpenFileNames(this, QStringLiteral("Select files"), QString(), filter);
    }
    else
    {
        const QString path = AntFileDialog::getOpenFileName(this, QStringLiteral("Select file"), QString(), filter);
        if (!path.isEmpty())
        {
            paths.append(path);
        }
    }
    addLocalFiles(paths);
}

void AntUpload::addLocalFiles(const QStringList& paths)
{
    if (m_disabled || paths.isEmpty())
    {
        return;
    }

    int remaining = m_maxCount > 0 ? qMax(0, m_maxCount - m_files.size()) : paths.size();
    if (!m_multiple)
    {
        remaining = qMin(remaining, 1);
    }
    if (remaining <= 0)
    {
        return;
    }

    int accepted = 0;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (const QString& path : paths)
    {
        if (accepted >= remaining || path.isEmpty() || !fileMatchesAccept(path))
        {
            continue;
        }

        const QFileInfo info(path);
        AntUploadFile file;
        file.uid = QString::number(now) + QStringLiteral("-") + QString::number(accepted);
        file.name = info.fileName();
        file.url = info.absoluteFilePath();
        file.thumbUrl = m_listType == Ant::UploadListType::Picture || m_listType == Ant::UploadListType::PictureCard
                            ? info.absoluteFilePath()
                            : QString();
        file.size = info.size();
        file.percent = 100;
        file.status = Ant::UploadFileStatus::Done;
        addFile(file);
        ++accepted;
    }
}

void AntUpload::openFilePreview(const AntUploadFile& file) const
{
    const QString target = !file.url.isEmpty() ? file.url : file.thumbUrl;
    if (target.isEmpty())
    {
        return;
    }
    const QUrl url = QUrl::fromUserInput(target);
    QDesktopServices::openUrl(url.isLocalFile() ? QUrl::fromLocalFile(url.toLocalFile()) : url);
}

QPixmap AntUpload::cachedThumbPixmap(const QString& path) const
{
    if (path.isEmpty())
    {
        return {};
    }
    auto it = m_thumbPixmapCache.constFind(path);
    if (it != m_thumbPixmapCache.constEnd())
    {
        ++m_thumbPixmapCacheHitCount;
        syncUploadPerfCounters();
        return it.value();
    }
    const QPixmap pixmap(path);
    m_thumbPixmapCache.insert(path, pixmap);
    ++m_thumbPixmapBuildCount;
    syncUploadPerfCounters();
    return pixmap;
}

void AntUpload::updateUploadRegion(const QRect& dirty,
                                   const QString& mode,
                                   bool itemScoped,
                                   bool triggerScoped,
                                   bool progressScoped)
{
    QRect updateRect = dirty;
    if (!updateRect.isValid() || updateRect.isEmpty())
    {
        updateRect = rect();
    }
    ++m_regionUpdateCount;
    if (itemScoped)
    {
        ++m_itemRegionUpdateCount;
    }
    if (triggerScoped)
    {
        ++m_triggerRegionUpdateCount;
    }
    if (progressScoped)
    {
        ++m_progressRegionUpdateCount;
    }
    setProperty("antUploadLastUpdateMode", mode);
    syncUploadPerfCounters();
    update(updateRect);
}

void AntUpload::syncUploadPerfCounters() const
{
    auto* self = const_cast<AntUpload*>(this);
    self->setProperty("antUploadLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antUploadLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antUploadThumbPixmapBuildCount", m_thumbPixmapBuildCount);
    self->setProperty("antUploadThumbPixmapCacheHitCount", m_thumbPixmapCacheHitCount);
    self->setProperty("antUploadRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antUploadItemRegionUpdateCount", m_itemRegionUpdateCount);
    self->setProperty("antUploadTriggerRegionUpdateCount", m_triggerRegionUpdateCount);
    self->setProperty("antUploadProgressRegionUpdateCount", m_progressRegionUpdateCount);
}
