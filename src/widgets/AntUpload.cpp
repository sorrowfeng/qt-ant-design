#include "AntUpload.h"

#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>

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
constexpr int SectionGap = 12;
} // namespace

AntUpload::AntUpload(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntUploadStyle(style()));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
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
    m_maxCount = maxCount;
    updateGeometry();
    update();
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
    update();
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
    updateGeometry();
    update();
    Q_EMIT listTypeChanged(m_listType);
}

bool AntUpload::isDraggerMode() const { return m_draggerMode; }

void AntUpload::setDraggerMode(bool dragger)
{
    if (m_draggerMode == dragger)
        return;
    m_draggerMode = dragger;
    setAcceptDrops(dragger);
    updateGeometry();
    update();
    Q_EMIT draggerModeChanged(m_draggerMode);
}

void AntUpload::dragEnterEvent(QDragEnterEvent* event)
{
    if (m_draggerMode && event->mimeData()->hasUrls())
    {
        m_dragOver = true;
        update();
        event->acceptProposedAction();
    }
    else
    {
        QWidget::dragEnterEvent(event);
    }
}

void AntUpload::dragLeaveEvent(QDragLeaveEvent* event)
{
    m_dragOver = false;
    update();
    QWidget::dragLeaveEvent(event);
}

void AntUpload::dropEvent(QDropEvent* event)
{
    m_dragOver = false;
    if (m_draggerMode && event->mimeData()->hasUrls())
    {
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl& url : urls)
        {
            if (url.isLocalFile())
            {
                AntUploadFile file;
                file.uid = QString::number(QDateTime::currentMSecsSinceEpoch());
                file.name = url.fileName();
                file.url = url.toLocalFile();
                file.status = Ant::UploadFileStatus::Done;
                addFile(file);
            }
        }
        event->acceptProposedAction();
        update();
    }
    else
    {
        QWidget::dropEvent(event);
    }
}

void AntUpload::addFile(const AntUploadFile& file)
{
    m_files.append(file);
    updateGeometry();
    update();
    Q_EMIT fileAdded(file);
}

void AntUpload::removeFile(const QString& uid)
{
    for (int i = 0; i < m_files.size(); ++i)
    {
        if (m_files[i].uid == uid)
        {
            m_files.remove(i);
            if (m_hoveredItemIndex >= m_files.size())
            {
                m_hoveredItemIndex = -1;
            }
            updateGeometry();
            update();
            Q_EMIT fileRemoved(uid);
            return;
        }
    }
}

void AntUpload::updateFileStatus(const QString& uid, Ant::UploadFileStatus status, int percent)
{
    for (auto& file : m_files)
    {
        if (file.uid == uid)
        {
            file.status = status;
            if (percent >= 0)
            {
                file.percent = qBound(0, percent, 100);
            }
            update();
            Q_EMIT fileStatusChanged(uid, status);
            return;
        }
    }
}

void AntUpload::setFileList(const QVector<AntUploadFile>& files)
{
    m_files = files;
    m_hoveredItemIndex = -1;
    updateGeometry();
    update();
}

QVector<AntUploadFile> AntUpload::fileList() const
{
    return m_files;
}

QSize AntUpload::sizeHint() const
{
    const int contentWidth = qMax(320, width());

    if (m_draggerMode)
    {
        return QSize(contentWidth, 150);
    }

    if (m_listType == Ant::UploadListType::PictureCard)
    {
        const int cols = GridColumns;
        const int cardAreaWidth = cols * CardSize + (cols - 1) * CardGap;
        int totalSlots = m_files.size();
        if (m_maxCount <= 0 || totalSlots < m_maxCount)
        {
            totalSlots += 1;
        }
        const int rows = (totalSlots + cols - 1) / cols;
        const int h = rows * CardSize + (rows - 1) * CardGap;
        return QSize(qMax(contentWidth, cardAreaWidth), h);
    }

    const int triggerH = TriggerHeight + 16;
    int fileAreaH = 0;
    for (int i = 0; i < m_files.size(); ++i)
    {
        const int itemH = m_listType == Ant::UploadListType::Picture ? PictureItemHeight : FileItemHeight;
        fileAreaH += itemH;
    }
    return QSize(contentWidth, triggerH + fileAreaH);
}

QSize AntUpload::minimumSizeHint() const
{
    if (m_listType == Ant::UploadListType::PictureCard)
    {
        return QSize(CardSize, CardSize);
    }
    return QSize(TriggerMinWidth, TriggerHeight);
}

void AntUpload::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntUpload::mouseMoveEvent(QMouseEvent* event)
{
    const QPoint pos = event->pos();
    m_mousePos = pos;

    if (m_listType == Ant::UploadListType::PictureCard)
    {
        m_triggerHovered = triggerRect().contains(pos);
    }
    else
    {
        m_triggerHovered = triggerRect().contains(pos);
        m_hoveredItemIndex = -1;
        for (int i = 0; i < m_files.size(); ++i)
        {
            if (fileItemRect(i).contains(pos))
            {
                m_hoveredItemIndex = i;
                break;
            }
        }
    }

    update();
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
                    if (m_maxCount <= 0 || m_files.size() < m_maxCount)
                    {
                        Q_EMIT uploadRequested();
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

        const int removeBtnSize = 14;
        const QRect removeBtnRect(
            itemRect.right() - 8 - removeBtnSize,
            itemRect.top() + (itemRect.height() - removeBtnSize) / 2,
            removeBtnSize,
            removeBtnSize);

        if (removeBtnRect.contains(pos))
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
        Q_EMIT uploadRequested();
    }

    event->accept();
}

void AntUpload::leaveEvent(QEvent* event)
{
    m_triggerHovered = false;
    m_hoveredItemIndex = -1;
    update();
    QWidget::leaveEvent(event);
}

QRect AntUpload::triggerRect() const
{
    if (m_draggerMode)
    {
        return rect();
    }

    if (m_listType == Ant::UploadListType::PictureCard)
    {
        const int cols = GridColumns;
        int triggerIndex = m_files.size();
        if (m_maxCount > 0 && m_files.size() >= m_maxCount)
        {
            return QRect();
        }
        const int col = triggerIndex % cols;
        const int row = triggerIndex / cols;
        return QRect(col * (CardSize + CardGap), row * (CardSize + CardGap), CardSize, CardSize);
    }

    return QRect(0, 0, qMin(width(), TriggerMinWidth), TriggerHeight);
}

QRect AntUpload::fileItemRect(int index) const
{
    if (index < 0 || index >= m_files.size())
    {
        return QRect();
    }

    int y = TriggerHeight + 8;
    for (int i = 0; i < index; ++i)
    {
        y += (m_listType == Ant::UploadListType::Picture ? PictureItemHeight : FileItemHeight);
    }
    const int h = m_listType == Ant::UploadListType::Picture ? PictureItemHeight : FileItemHeight;
    return QRect(0, y, width(), h);
}

QRect AntUpload::fileCardRect(int index) const
{
    if (index < 0)
    {
        return QRect();
    }

    const int totalSlots = m_files.size() + (m_maxCount <= 0 || m_files.size() < m_maxCount ? 1 : 0);
    if (index >= totalSlots)
    {
        return QRect();
    }

    const int cols = GridColumns;
    const int col = index % cols;
    const int row = index / cols;
    return QRect(col * (CardSize + CardGap), row * (CardSize + CardGap), CardSize, CardSize);
}

bool AntUpload::isOverRemoveButton(const QPoint& pos) const
{
    for (int i = 0; i < m_files.size(); ++i)
    {
        const QRect itemRect = fileItemRect(i);
        if (!itemRect.contains(pos))
        {
            continue;
        }
        const int removeBtnSize = 14;
        const QRect removeBtnRect(
            itemRect.right() - 8 - removeBtnSize,
            itemRect.top() + (itemRect.height() - removeBtnSize) / 2,
            removeBtnSize,
            removeBtnSize);
        return removeBtnRect.contains(pos);
    }
    return false;
}
