#include "AntImage.h"

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWindow>
#include <QtMath>

#include "AntButton.h"
#include "AntIcon.h"
#include "core/AntTheme.h"
#include "private/AntImageDecodeUtils.h"

namespace
{

QSize imageTargetSize(const QPixmap& pixmap, int requestedWidth, int requestedHeight)
{
    const QSize natural = pixmap.isNull() ? QSize(200, 200) : pixmap.size();

    if (requestedWidth > 0 && requestedHeight > 0)
    {
        return QSize(requestedWidth, requestedHeight);
    }
    if (requestedWidth > 0)
    {
        const int height = natural.width() > 0 ? qRound(static_cast<qreal>(requestedWidth) * natural.height() / natural.width())
                                               : requestedWidth;
        return QSize(requestedWidth, qMax(1, height));
    }
    if (requestedHeight > 0)
    {
        const int width = natural.height() > 0 ? qRound(static_cast<qreal>(requestedHeight) * natural.width() / natural.height())
                                               : requestedHeight;
        return QSize(qMax(1, width), requestedHeight);
    }
    return natural;
}

class ImagePreviewDialog : public QDialog
{
public:
    explicit ImagePreviewDialog(const QList<QPixmap>& pixmaps, int startIndex, QWidget* parent = nullptr)
        : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
        , m_pixmaps(pixmaps)
        , m_currentIndex(qBound(0, startIndex, qMax(0, pixmaps.size() - 1)))
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_DeleteOnClose);
        setMouseTracking(true);

        const QScreen* screen = QApplication::screenAt(QCursor::pos());
        if (!screen)
        {
            screen = QApplication::primaryScreen();
        }
        const QRect geometry = screen ? screen->availableGeometry() : QRect(0, 0, 1280, 800);
        setGeometry(geometry);

        resetView();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

        // antd 全屏遮罩（colorBgMask），无面板、无边框
        p.fillRect(rect(), token.colorBgMask);

        // 图片本体（居中 + 缩放 + 平移 + 旋转）
        const QPixmap& pix = currentPixmap();
        if (!pix.isNull())
        {
            p.save();
            const QRectF imageRect = currentImageRect();
            const QPointF center = imageRect.center();
            p.translate(center);
            p.rotate(m_rotation);
            p.translate(-center);
            // 按缩放后的 imageRect 绘制源图（旋转围绕中心进行）
            p.drawPixmap(imageRect, pix, QRectF(QPointF(0, 0), QSizeF(pix.size())));
            p.restore();
        }

        // 控制按钮
        drawRoundControl(p, closeRect(), QLatin1String("Close"), Control::Close, token);
        const bool hasGroup = m_pixmaps.size() > 1;
        if (hasGroup)
        {
            drawRoundControl(p, prevRect(), QLatin1String("Left"), Control::Prev, token, m_currentIndex <= 0);
            drawRoundControl(p, nextRect(), QLatin1String("Right"), Control::Next, token,
                             m_currentIndex >= m_pixmaps.size() - 1);
        }
        drawFooter(p, token);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (m_dragging)
        {
            const QPointF delta = event->pos() - m_dragStartPos;
            m_offset = clampOffset(m_dragStartOffset + delta);
            update();
            return;
        }
        const Control control = controlAt(event->pos());
        if (control != m_hoverControl)
        {
            m_hoverControl = control;
            update();
        }
        QDialog::mouseMoveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            const Control control = controlAt(event->pos());
            switch (control)
            {
            case Control::Close:
                close();
                return;
            case Control::Prev:
                navigate(-1);
                return;
            case Control::Next:
                navigate(1);
                return;
            case Control::RotateLeft:
                setRotation(m_rotation - 90);
                return;
            case Control::RotateRight:
                setRotation(m_rotation + 90);
                return;
            case Control::ZoomIn:
                applyScaleAt(event->position(), m_scale * kZoomStep);
                return;
            case Control::ZoomOut:
                applyScaleAt(event->position(), m_scale / kZoomStep);
                return;
            case Control::None:
                break;
            }

            if (m_scale > m_fitScale + 0.01)
            {
                m_dragging = true;
                m_dragStartPos = event->pos();
                m_dragStartOffset = m_offset;
                setCursor(Qt::ClosedHandCursor);
                return;
            }
        }
        QDialog::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (m_dragging)
        {
            m_dragging = false;
            updateCursor();
            return;
        }
        QDialog::mouseReleaseEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            // 双击在「适应窗口」与「100%」之间切换（antd 无此快捷键，属附加便利操作）
            if (qFuzzyCompare(m_scale, 1.0))
            {
                applyScaleAt(event->position(), m_fitScale);
            }
            else
            {
                applyScaleAt(event->position(), 1.0);
            }
            return;
        }
        QDialog::mouseDoubleClickEvent(event);
    }

    void wheelEvent(QWheelEvent* event) override
    {
        const qreal factor = event->angleDelta().y() > 0 ? kZoomStep : 1.0 / kZoomStep;
        applyScaleAt(event->position(), m_scale * factor);
        event->accept();
    }

    void keyPressEvent(QKeyEvent* event) override
    {
        switch (event->key())
        {
        case Qt::Key_Escape:
            close();
            return;
        case Qt::Key_Left:
            navigate(-1);
            return;
        case Qt::Key_Right:
            navigate(1);
            return;
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            applyScaleAt(rect().center(), m_scale * kZoomStep);
            return;
        case Qt::Key_Minus:
            applyScaleAt(rect().center(), m_scale / kZoomStep);
            return;
        case Qt::Key_Home:
        case Qt::Key_0:
            resetView();
            update();
            return;
        default:
            break;
        }
        QDialog::keyPressEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        if (m_hoverControl != Control::None)
        {
            m_hoverControl = Control::None;
            update();
        }
        QDialog::leaveEvent(event);
    }

private:
    enum class Control
    {
        None,
        Close,
        Prev,
        Next,
        RotateLeft,
        RotateRight,
        ZoomIn,
        ZoomOut,
    };

    static constexpr qreal kZoomStep = 1.15;
    static constexpr qreal kMaxScale = 50.0;
    static constexpr int kControlSize = 40;
    static constexpr int kControlMargin = 12;
    static constexpr qreal kEdgeKeep = 80.0;
    static constexpr int kFooterPillGap = 12;
    static constexpr int kIconSize = 18;

    const QPixmap& currentPixmap() const { return m_pixmaps.at(m_currentIndex); }

    QSizeF rotatedSourceSize() const
    {
        const QSizeF source = currentPixmap().isNull() ? QSizeF(1, 1) : QSizeF(currentPixmap().size());
        return (m_rotation % 180 != 0) ? QSizeF(source.height(), source.width()) : source;
    }

    QSizeF scaledImageSize() const
    {
        return rotatedSourceSize() * m_scale;
    }

    QPointF centeredTopLeft() const
    {
        const QSizeF image = scaledImageSize();
        return QPointF((width() - image.width()) / 2.0, (height() - image.height()) / 2.0);
    }

    QRectF currentImageRect() const
    {
        return QRectF(centeredTopLeft() + m_offset, scaledImageSize());
    }

    void resetView()
    {
        const QSizeF source = rotatedSourceSize();
        if (source.isEmpty())
        {
            return;
        }
        // antd：maxWidth 100%、maxHeight 约 70%（留出工具栏空间），小图按 100% 显示
        const qreal availableWidth = width() - (kControlMargin + 16) * 2.0;
        const qreal availableHeight = height() * 0.70;
        m_fitScale = qMin(1.0, qMin(availableWidth / source.width(), availableHeight / source.height()));
        m_scale = m_fitScale;
        m_offset = QPointF();
        updateCursor();
    }

    void setRotation(int rotation)
    {
        const int step = ((rotation % 360) + 360) % 360 / 90 * 90;
        if (m_rotation == step)
        {
            return;
        }
        // 旋转后源图宽高互换，需重新按适应尺寸摆放
        m_rotation = step;
        resetView();
        update();
    }

    void applyScaleAt(const QPointF& anchor, qreal newScale)
    {
        newScale = qBound(m_fitScale, newScale, kMaxScale);
        if (qFuzzyCompare(newScale, m_scale))
        {
            return;
        }
        const QPointF oldTopLeft = centeredTopLeft() + m_offset;
        const QPointF imagePoint = (anchor - oldTopLeft) / m_scale;
        m_scale = newScale;
        m_offset = clampOffset(anchor - imagePoint * newScale - centeredTopLeft());
        updateCursor();
        update();
    }

    QPointF clampOffset(const QPointF& offset) const
    {
        const QSizeF image = scaledImageSize();
        const qreal areaWidth = width();
        const qreal areaHeight = height();
        qreal dx = offset.x();
        if (image.width() <= areaWidth)
        {
            dx = 0.0;
        }
        else
        {
            dx = qBound(areaWidth - image.width() - kEdgeKeep, dx, kEdgeKeep);
        }
        qreal dy = offset.y();
        if (image.height() <= areaHeight)
        {
            dy = 0.0;
        }
        else
        {
            dy = qBound(areaHeight - image.height() - kEdgeKeep, dy, kEdgeKeep);
        }
        return QPointF(dx, dy);
    }

    void updateCursor()
    {
        setCursor(m_scale > m_fitScale + 0.01 ? Qt::OpenHandCursor : Qt::ArrowCursor);
    }

    void navigate(int delta)
    {
        const int count = m_pixmaps.size();
        if (count <= 1)
        {
            return;
        }
        const int target = m_currentIndex + delta;
        if (target < 0 || target >= count)
        {
            return;
        }
        m_currentIndex = target;
        resetView();
        update();
    }

    Control controlAt(const QPoint& pos) const
    {
        if (closeRect().contains(pos))
        {
            return Control::Close;
        }
        if (m_pixmaps.size() > 1)
        {
            if (prevRect().contains(pos))
            {
                return Control::Prev;
            }
            if (nextRect().contains(pos))
            {
                return Control::Next;
            }
        }
        if (footerPillRect().contains(pos))
        {
            const QPoint pillTopLeft = footerPillRect().topLeft();
            const int index = (pos.x() - pillTopLeft.x()) / (kControlSize + kFooterPillGap);
            switch (index)
            {
            case 0:
                return Control::RotateLeft;
            case 1:
                return Control::RotateRight;
            case 2:
                return Control::ZoomOut;
            case 3:
                return Control::ZoomIn;
            default:
                return Control::None;
            }
        }
        return Control::None;
    }

    QRect closeRect() const
    {
        return QRect(width() - kControlSize - kControlMargin, kControlMargin, kControlSize, kControlSize);
    }

    QRect prevRect() const
    {
        return QRect(kControlMargin, (height() - kControlSize) / 2, kControlSize, kControlSize);
    }

    QRect nextRect() const
    {
        return QRect(width() - kControlSize - kControlMargin, (height() - kControlSize) / 2, kControlSize, kControlSize);
    }

    QRect footerPillRect() const
    {
        // 4 个操作（rotateLeft/rotateRight/zoomOut/zoomIn），下方留出页码文本的空间
        const int actionCount = 4;
        const int widthSum = actionCount * kControlSize + (actionCount - 1) * kFooterPillGap;
        const int x = (width() - widthSum) / 2;
        const int y = height() - kControlMargin - kControlSize - 32;
        return QRect(x, y, widthSum, kControlSize);
    }

    QRect footerProgressRect() const
    {
        const QRect pill = footerPillRect();
        return QRect(0, pill.bottom() + 10, width(), 22);
    }

    QColor operationColor(const QColor& base, Control control, bool disabled) const
    {
        if (disabled)
        {
            QColor c = base;
            c.setAlphaF(0.28);
            return c;
        }
        if (m_hoverControl == control)
        {
            QColor c = base;
            c.setAlphaF(0.85);
            return c;
        }
        QColor c = base;
        c.setAlphaF(0.65);
        return c;
    }

    void drawRoundControl(QPainter& p,
                          const QRect& rect,
                          const QString& iconName,
                          Control control,
                          const AntThemeTokens& token,
                          bool disabled = false) const
    {
        const bool hovered = !disabled && m_hoverControl == control;
        QColor circle = token.colorBgMask;
        circle.setAlphaF(hovered ? 0.25 : 0.12);
        p.setPen(Qt::NoPen);
        p.setBrush(circle);
        p.drawEllipse(QRectF(rect));

        const QColor iconColor = operationColor(token.colorTextLightSolid, control, disabled);
        const QPixmap icon = AntIcon::renderPixmap(iconName, kIconSize, iconColor);
        if (!icon.isNull())
        {
            const QRectF iconRect(rect.center().x() - kIconSize / 2.0,
                                  rect.center().y() - kIconSize / 2.0,
                                  kIconSize,
                                  kIconSize);
            p.drawPixmap(iconRect, icon, QRectF(icon.rect()));
        }
    }

    void drawFooter(QPainter& p, const AntThemeTokens& token)
    {
        const QRect pill = footerPillRect();
        QColor pillBg = token.colorBgMask;
        pillBg.setAlphaF(0.12);
        p.setPen(Qt::NoPen);
        p.setBrush(pillBg);
        p.drawRoundedRect(QRectF(pill), pill.height() / 2.0, pill.height() / 2.0);

        const struct
        {
            QString icon;
            Control control;
        } actions[4] = {
            {QStringLiteral("RotateLeft"), Control::RotateLeft},
            {QStringLiteral("RotateRight"), Control::RotateRight},
            {QStringLiteral("ZoomOut"), Control::ZoomOut},
            {QStringLiteral("ZoomIn"), Control::ZoomIn},
        };
        for (int i = 0; i < 4; ++i)
        {
            const QRect buttonRect(pill.left() + i * (kControlSize + kFooterPillGap), pill.top(), kControlSize, kControlSize);
            QColor circle = token.colorBgMask;
            const bool hovered = m_hoverControl == actions[i].control;
            circle.setAlphaF(hovered ? 0.25 : 0.08);
            p.setPen(Qt::NoPen);
            p.setBrush(circle);
            p.drawEllipse(QRectF(buttonRect));

            const QColor iconColor = operationColor(token.colorTextLightSolid, actions[i].control, false);
            const QPixmap icon = AntIcon::renderPixmap(actions[i].icon, kIconSize, iconColor);
            if (!icon.isNull())
            {
                const QRectF iconRect(buttonRect.center().x() - kIconSize / 2.0,
                                      buttonRect.center().y() - kIconSize / 2.0,
                                      kIconSize,
                                      kIconSize);
                p.drawPixmap(iconRect, icon, QRectF(icon.rect()));
            }
        }

        // 页码（antd preview footer 的 progress 文案）
        QRect progressRect = footerProgressRect();
        QFont f = font();
        f.setPixelSize(token.fontSize);
        p.setFont(f);
        QColor progressColor = token.colorTextLightSolid;
        progressColor.setAlphaF(0.65);
        p.setPen(progressColor);
        p.drawText(progressRect, Qt::AlignCenter,
                   QStringLiteral("%1 / %2").arg(m_currentIndex + 1).arg(m_pixmaps.size()));
    }

    QList<QPixmap> m_pixmaps;
    int m_currentIndex = 0;
    qreal m_scale = 1.0;
    qreal m_fitScale = 1.0;
    int m_rotation = 0;
    QPointF m_offset;
    bool m_dragging = false;
    QPointF m_dragStartPos;
    QPointF m_dragStartOffset;
    Control m_hoverControl = Control::None;
};

} // namespace

static void initAntImageResources()
{
    Q_INIT_RESOURCE(qt_ant_design);
}

AntImage::AntImage(QWidget* parent)
    : QWidget(parent)
{
    initAntImageResources();
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidatePreviewOverlayCache();
        requestImageUpdate(QStringLiteral("theme"));
    });
    syncImagePerfCounters();
}

QString AntImage::src() const { return m_src; }

void AntImage::setSrc(const QString& path)
{
    if (m_src == path) return;
    const bool previousLoaded = m_loaded;
    const QString previousError = m_loadError;
    m_src = path;
    decodeCurrentSource();
    const quint64 generation = m_loadGeneration;

    QPointer<AntImage> self(this);
    Q_EMIT srcChanged(m_src);
    if (!self)
        return;
    self->emitLoadStateChanges(previousLoaded, previousError, generation);
}

QString AntImage::alt() const { return m_alt; }
void AntImage::setAlt(const QString& text)
{
    if (m_alt == text) return;
    m_alt = text;
    requestImageUpdate(QStringLiteral("alt"));
    Q_EMIT altChanged(m_alt);
}

bool AntImage::preview() const { return m_preview; }
void AntImage::setPreview(bool enable)
{
    if (m_preview == enable) return;
    m_preview = enable;
    invalidatePreviewOverlayCache();
    requestImageUpdate(QStringLiteral("preview"));
    Q_EMIT previewChanged(m_preview);
}

int AntImage::imgWidth() const { return m_imgWidth; }
void AntImage::setImgWidth(int w)
{
    if (m_imgWidth == w) return;
    m_imgWidth = w;
    invalidateScaledPixmapCache();
    updateGeometry();
    requestImageUpdate(QStringLiteral("width"));
    Q_EMIT imgWidthChanged(m_imgWidth);
}

int AntImage::imgHeight() const { return m_imgHeight; }
void AntImage::setImgHeight(int h)
{
    if (m_imgHeight == h) return;
    m_imgHeight = h;
    invalidateScaledPixmapCache();
    updateGeometry();
    requestImageUpdate(QStringLiteral("height"));
    Q_EMIT imgHeightChanged(m_imgHeight);
}

bool AntImage::isLoaded() const { return m_loaded; }

QString AntImage::loadError() const { return m_loadError; }

bool AntImage::reload()
{
    const bool previousLoaded = m_loaded;
    const QString previousError = m_loadError;
    decodeCurrentSource();
    const bool loaded = m_loaded;
    const quint64 generation = m_loadGeneration;
    emitLoadStateChanges(previousLoaded, previousError, generation);
    return loaded;
}

QSize AntImage::sizeHint() const
{
    return imageTargetSize(m_pixmap, m_imgWidth, m_imgHeight);
}

void AntImage::paintEvent(QPaintEvent*)
{
    const auto& token = antTheme->tokens();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QRectF r = rect();

    if (m_loaded)
    {
        const bool hasExplicitBox = m_imgWidth > 0 && m_imgHeight > 0;
        const QPixmap scaled = cachedScaledPixmap(devicePixelRatioF(),
                                                  r.size().toSize(),
                                                  hasExplicitBox ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio);
        p.drawPixmap(r.topLeft(), scaled);
    }
    else
    {
        p.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        p.setBrush(token.colorFillQuaternary);
        p.drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), token.borderRadius, token.borderRadius);

        QFont f = p.font();
        f.setPixelSize(token.fontSizeSM);
        p.setFont(f);
        p.setPen(token.colorTextPlaceholder);
        p.drawText(r, Qt::AlignCenter, m_alt);
    }

    // Preview overlay on hover
    if (m_hovered && m_preview && m_loaded)
    {
        const QPixmap overlay = cachedPreviewOverlayPixmap(devicePixelRatioF(), r.size().toSize());
        p.drawPixmap(r.topLeft(), overlay);
    }
}

void AntImage::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        QPointer<AntImage> self(this);
        Q_EMIT clicked();
        if (!self)
        {
            return;
        }
        if (m_preview && m_loaded)
        {
            showPreviewDialog();
            if (!self)
            {
                return;
            }
        }
    }
    QWidget::mousePressEvent(e);
}

void AntImage::enterEvent(AntEnterEvent*)
{
    m_hovered = true;
    if (m_preview && m_loaded)
        requestImageUpdate(QStringLiteral("hover"), rect());
}

void AntImage::leaveEvent(QEvent*)
{
    m_hovered = false;
    if (m_preview && m_loaded)
        requestImageUpdate(QStringLiteral("hover"), rect());
}

void AntImage::setPreviewGroup(const QList<AntImage*>& group)
{
    m_previewGroup.clear();
    m_previewGroup.reserve(group.size());
    for (AntImage* image : group)
    {
        if (image)
        {
            m_previewGroup.append(QPointer<AntImage>(image));
        }
    }
}

void AntImage::showPreviewDialog()
{
    showPreviewDialogAt(-1);
}

void AntImage::showPreviewDialogAt(int index)
{
    QList<QPixmap> pixmaps;
    int startIndex = 0;

    if (m_previewGroup.isEmpty())
    {
        pixmaps.append(m_pixmap);
        startIndex = 0;
    }
    else
    {
        for (int i = 0; i < m_previewGroup.size(); ++i)
        {
            AntImage* img = m_previewGroup[i].data();
            if (img && img->m_loaded)
            {
                pixmaps.append(img->m_pixmap);
                if (img == this)
                    startIndex = pixmaps.size() - 1;
            }
        }
        if (index >= 0 && index < pixmaps.size())
            startIndex = index;
    }

    if (pixmaps.isEmpty())
        return;

    auto* dlg = new ImagePreviewDialog(pixmaps, startIndex, window());
    dlg->exec();
}

QPixmap AntImage::cachedScaledPixmap(qreal devicePixelRatio,
                                     const QSize& targetSize,
                                     Qt::AspectRatioMode aspectMode) const
{
    if (!m_loaded || m_pixmap.isNull() || targetSize.isEmpty())
    {
        return {};
    }

    const qreal dpr = qMax<qreal>(1.0, devicePixelRatio);
    const QSize logicalSize(qMax(1, targetSize.width()), qMax(1, targetSize.height()));
    const qint64 sourceCacheKey = m_pixmap.cacheKey();
    const QSize sourceSize = m_pixmap.size();
    const qreal sourceDpr = m_pixmap.devicePixelRatio();

    if (m_scaledPixmapCache.valid &&
        qFuzzyCompare(m_scaledPixmapCache.devicePixelRatio, dpr) &&
        qFuzzyCompare(m_scaledPixmapCache.sourceDevicePixelRatio, sourceDpr) &&
        m_scaledPixmapCache.logicalSize == logicalSize &&
        m_scaledPixmapCache.aspectMode == aspectMode &&
        m_scaledPixmapCache.sourceCacheKey == sourceCacheKey &&
        m_scaledPixmapCache.sourceSize == sourceSize)
    {
        ++m_scaledPixmapCacheHitCount;
        syncImagePerfCounters();
        return m_scaledPixmapCache.pixmap;
    }

    ++m_scaledPixmapBuildCount;

    ScaledPixmapCache cache;
    cache.valid = true;
    cache.devicePixelRatio = dpr;
    cache.sourceDevicePixelRatio = sourceDpr;
    cache.logicalSize = logicalSize;
    cache.aspectMode = aspectMode;
    cache.sourceCacheKey = sourceCacheKey;
    cache.sourceSize = sourceSize;
    cache.pixmap = QPixmap(QSize(qCeil(logicalSize.width() * dpr), qCeil(logicalSize.height() * dpr)));
    cache.pixmap.setDevicePixelRatio(dpr);
    cache.pixmap.fill(Qt::transparent);

    QRectF imageRect(QPointF(0, 0), QSizeF(logicalSize));
    if (aspectMode == Qt::KeepAspectRatio)
    {
        const QSize scaledSize = sourceSize.scaled(logicalSize, Qt::KeepAspectRatio);
        imageRect = QRectF((logicalSize.width() - scaledSize.width()) / 2.0,
                           (logicalSize.height() - scaledSize.height()) / 2.0,
                           scaledSize.width(),
                           scaledSize.height());
    }

    QPainter painter(&cache.pixmap);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.drawPixmap(imageRect, m_pixmap, QRectF(QPointF(0, 0), QSizeF(sourceSize)));

    m_scaledPixmapCache = cache;
    syncImagePerfCounters();
    return m_scaledPixmapCache.pixmap;
}

QPixmap AntImage::cachedPreviewOverlayPixmap(qreal devicePixelRatio, const QSize& targetSize) const
{
    if (targetSize.isEmpty())
    {
        return {};
    }

    const auto& token = antTheme->tokens();
    const qreal dpr = qMax<qreal>(1.0, devicePixelRatio);
    const QSize logicalSize(qMax(1, targetSize.width()), qMax(1, targetSize.height()));
    // antd cover：rgba(0,0,0,0.3)
    const QColor overlayColor = QColor(0, 0, 0, 77);
    const int iconSize = 40;

    if (m_previewOverlayPixmapCache.valid &&
        qFuzzyCompare(m_previewOverlayPixmapCache.devicePixelRatio, dpr) &&
        m_previewOverlayPixmapCache.logicalSize == logicalSize &&
        m_previewOverlayPixmapCache.overlayColor == overlayColor)
    {
        ++m_previewOverlayPixmapCacheHitCount;
        syncImagePerfCounters();
        return m_previewOverlayPixmapCache.pixmap;
    }

    ++m_previewOverlayPixmapBuildCount;

    PreviewOverlayPixmapCache cache;
    cache.valid = true;
    cache.devicePixelRatio = dpr;
    cache.logicalSize = logicalSize;
    cache.overlayColor = overlayColor;
    cache.pixmap = QPixmap(QSize(qCeil(logicalSize.width() * dpr), qCeil(logicalSize.height() * dpr)));
    cache.pixmap.setDevicePixelRatio(dpr);
    cache.pixmap.fill(Qt::transparent);

    const QRectF r(QPointF(0, 0), QSizeF(logicalSize));
    QPainter painter(&cache.pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.fillRect(r, overlayColor);

    // antd cover 图标（EyeOutlined，无文字）
    const QPixmap icon = AntIcon::renderPixmap(QStringLiteral("Eye"), iconSize, token.colorTextLightSolid);
    if (!icon.isNull())
    {
        const QRectF iconRect(r.center().x() - iconSize / 2.0,
                              r.center().y() - iconSize / 2.0,
                              iconSize,
                              iconSize);
        painter.drawPixmap(iconRect, icon, QRectF(icon.rect()));
    }

    m_previewOverlayPixmapCache = cache;
    syncImagePerfCounters();
    return m_previewOverlayPixmapCache.pixmap;
}

void AntImage::invalidateScaledPixmapCache() const
{
    m_scaledPixmapCache.valid = false;
}

void AntImage::invalidatePreviewOverlayCache() const
{
    m_previewOverlayPixmapCache.valid = false;
}

void AntImage::requestImageUpdate(const QString& mode, const QRect& dirty)
{
    m_lastUpdateMode = mode;
    ++m_regionUpdateCount;
    syncImagePerfCounters();
    if (dirty.isValid())
        update(dirty);
    else
        update();
}

void AntImage::syncImagePerfCounters() const
{
    auto* self = const_cast<AntImage*>(this);
    self->setProperty("antImageScaledPixmapBuildCount", m_scaledPixmapBuildCount);
    self->setProperty("antImageScaledPixmapCacheHitCount", m_scaledPixmapCacheHitCount);
    self->setProperty("antImagePreviewOverlayPixmapBuildCount", m_previewOverlayPixmapBuildCount);
    self->setProperty("antImagePreviewOverlayPixmapCacheHitCount", m_previewOverlayPixmapCacheHitCount);
    self->setProperty("antImageRegionUpdateCount", m_regionUpdateCount);
    self->setProperty("antImageLastUpdateMode", m_lastUpdateMode);
}

bool AntImage::decodeCurrentSource()
{
    ++m_loadGeneration;
    m_pixmap = {};
    m_loaded = false;
    m_loadError.clear();

    AntImageDecode::DecodeMetadata metadata;
    QImage image;
    if (!m_src.isEmpty())
    {
        m_loaded = AntImageDecode::read(m_src, QSize(), &image, &metadata, &m_loadError);
        if (m_loaded)
            m_pixmap = QPixmap::fromImage(image);
    }

    setProperty("antImageLoadError", m_loadError);
    setProperty("antImageSourceFormat", QString::fromLatin1(metadata.format));
    setProperty("antImageSourceWidth", metadata.sourceSize.width());
    setProperty("antImageSourceHeight", metadata.sourceSize.height());
    setProperty("antImageEncodedBytes", metadata.stamp.encodedBytes);
    setProperty("antImageEstimatedDecodedBytes", metadata.estimatedDecodedBytes);

    invalidateScaledPixmapCache();
    updateGeometry();
    requestImageUpdate(QStringLiteral("src"));
    return m_loaded;
}

void AntImage::emitLoadStateChanges(bool previousLoaded, const QString& previousError, quint64 generation)
{
    if (generation != m_loadGeneration)
        return;

    QPointer<AntImage> self(this);
    if (previousLoaded != m_loaded)
    {
        Q_EMIT loadedChanged(m_loaded);
        if (!self || self->m_loadGeneration != generation)
            return;
    }
    if (previousError != m_loadError)
    {
        Q_EMIT loadErrorChanged(m_loadError);
        if (!self || self->m_loadGeneration != generation)
            return;
    }
    if (!m_src.isEmpty() && !m_loaded)
        Q_EMIT loadFailed(m_src, m_loadError);
}
