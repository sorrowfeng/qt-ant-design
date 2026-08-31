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
#include <QVariantAnimation>
#include <QWindow>
#include <QtMath>

#include <functional>

#include "AntButton.h"
#include "AntDialog.h"
#include "AntIcon.h"
#include "core/AntTheme.h"
#include "private/AntImageDecodeUtils.h"

namespace
{

// QMouseEvent::position() 是 Qt6 API；Qt5 使用 localPos()。
QPointF mouseEventPosition(const QMouseEvent* event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->localPos();
#endif
}

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

// 图片预览内容区：缩放/平移/旋转/导航全部在这里绘制与交互，
// 外层由 AntDialog 提供窗口化的标题栏与关闭按钮（类似微信看图）。
class ImagePreviewSurface : public QWidget
{
public:
    explicit ImagePreviewSurface(const QList<QPixmap>& pixmaps, QWidget* parent = nullptr)
        : QWidget(parent)
        , m_pixmaps(pixmaps)
    {
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);

        // 过卷回弹阻尼（微信看图式）：松开后从越界位置平滑弹回边界
        m_bounceAnimation = new QVariantAnimation(this);
        m_bounceAnimation->setDuration(kBounceDuration);
        m_bounceAnimation->setEasingCurve(QEasingCurve::OutCubic);
        connect(m_bounceAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
            m_offset = value.toPointF();
            update();
        });
        connect(m_bounceAnimation, &QVariantAnimation::finished, this, [this]() {
            m_offset = clampOffset(m_offset);
            update();
            updateCursor();
        });
    }

    int currentIndex() const { return m_currentIndex; }

    void setCurrentIndex(int index)
    {
        const int target = qBound(0, index, qMax(0, m_pixmaps.size() - 1));
        m_currentIndex = target;
        resetView();
        update();
    }

    void setIndexChangedHandler(std::function<void(int)> handler)
    {
        m_indexChanged = std::move(handler);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

        p.fillRect(rect(), token.colorBgContainer);

        const QPixmap& pix = currentPixmap();
        if (!pix.isNull())
        {
            p.save();
            const QRectF imageRect = currentImageRect();
            const QPointF center = imageRect.center();
            p.translate(center);
            p.rotate(m_rotation);
            p.translate(-center);
            p.drawPixmap(imageRect, pix, QRectF(QPointF(0, 0), QSizeF(pix.size())));
            p.restore();
        }

        if (m_pixmaps.size() > 1)
        {
            drawRoundControl(p, prevRect(), QLatin1String("Left"), Control::Prev, token);
            drawRoundControl(p, nextRect(), QLatin1String("Right"), Control::Next, token);
        }
        drawFooter(p, token);
    }

    void resizeEvent(QResizeEvent* event) override
    {
        QWidget::resizeEvent(event);
        if (m_bounceAnimation)
        {
            m_bounceAnimation->stop();
        }
        const qreal previousFit = m_fitScale;
        recomputeFitScale();
        if (!qFuzzyCompare(m_fitScale, previousFit))
        {
            // 视图变化后不允许小于新的 fit 比例
            if (m_scale < m_fitScale || qFuzzyCompare(m_scale, previousFit))
            {
                m_scale = m_fitScale;
                m_offset = QPointF();
            }
        }
        m_offset = clampOffset(m_offset);
        update();
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (m_dragging)
        {
            // 拖拽允许有限越界（橡皮筋软化），松手后由回弹动画拉回边界
            m_offset = rubberBandedOffset(m_dragStartOffset + (event->pos() - m_dragStartPos));
            update();
            return;
        }
        const Control control = controlAt(event->pos());
        if (control != m_hoverControl)
        {
            m_hoverControl = control;
            update();
        }
        QWidget::mouseMoveEvent(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            switch (controlAt(event->pos()))
            {
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
                applyScaleAt(mouseEventPosition(event), m_scale * kZoomStep);
                return;
            case Control::ZoomOut:
                applyScaleAt(mouseEventPosition(event), m_scale / kZoomStep);
                return;
            case Control::None:
                break;
            }

            if (m_scale > m_fitScale + 0.01)
            {
                if (m_bounceAnimation)
                {
                    m_bounceAnimation->stop();
                }
                m_dragging = true;
                m_dragStartPos = event->pos();
                m_dragStartOffset = m_offset;
                setCursor(Qt::ClosedHandCursor);
                return;
            }
        }
        QWidget::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (m_dragging)
        {
            m_dragging = false;
            const QPointF target = clampOffset(m_offset);
            if (m_bounceAnimation && (target - m_offset).manhattanLength() > 0.5)
            {
                // 有过卷量：OutCubic 回弹到边界
                m_bounceAnimation->stop();
                m_bounceAnimation->setStartValue(m_offset);
                m_bounceAnimation->setEndValue(target);
                m_bounceAnimation->start();
            }
            else
            {
                m_offset = target;
                updateCursor();
            }
            return;
        }
        QWidget::mouseReleaseEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        // 双击切换只对图片区域生效；工具栏/翻页按钮上的快速连点
        // 会触发 dblclick，不能把缩放重置回 100%。
        if (event->button() == Qt::LeftButton && controlAt(event->pos()) == Control::None)
        {
            // 双击在「适应窗口」与「100%」之间切换
            if (qFuzzyCompare(m_scale, 1.0))
            {
                applyScaleAt(mouseEventPosition(event), m_fitScale);
            }
            else
            {
                applyScaleAt(mouseEventPosition(event), 1.0);
            }
            return;
        }
        QWidget::mouseDoubleClickEvent(event);
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
            if (QWidget* top = window())
            {
                top->close();
            }
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
        QWidget::keyPressEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        if (m_hoverControl != Control::None)
        {
            m_hoverControl = Control::None;
            update();
        }
        QWidget::leaveEvent(event);
    }

private:
    enum class Control
    {
        None,
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
    static constexpr int kFooterPillGap = 12;
    static constexpr int kIconSize = 18;
    // 过卷回弹（微信看图式）
    static constexpr qreal kOverscrollCap = 56.0;
    static constexpr int kBounceDuration = 240;

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

    void recomputeFitScale()
    {
        const QSizeF source = rotatedSourceSize();
        if (source.isEmpty() || width() <= 0 || height() <= 0)
        {
            return;
        }
        const qreal availableWidth = width() - (kControlMargin + 16) * 2.0;
        const qreal availableHeight = height() - 96.0;
        m_fitScale = qMin(1.0, qMin(availableWidth / source.width(), availableHeight / source.height()));
    }

    void resetView()
    {
        if (m_bounceAnimation)
        {
            m_bounceAnimation->stop();
        }
        recomputeFitScale();
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
        if (m_bounceAnimation)
        {
            m_bounceAnimation->stop();
        }
        const QPointF oldTopLeft = centeredTopLeft() + m_offset;
        const QPointF imagePoint = (anchor - oldTopLeft) / m_scale;
        m_scale = newScale;
        m_offset = clampOffset(anchor - imagePoint * newScale - centeredTopLeft());
        updateCursor();
        update();
    }

    QPointF rubberBandedOffset(const QPointF& desired) const
    {
        // 橡皮筋：越界量用渐近曲线软化（越拉越费劲，封顶 kOverscrollCap），
        // 松手后由回弹动画拉回 clampOffset 边界。
        const QPointF clamped = clampOffset(desired);
        const QPointF over = desired - clamped;
        const auto soften = [](qreal delta) {
            const qreal magnitude = qAbs(delta);
            const qreal sign = delta < 0 ? -1.0 : 1.0;
            return kOverscrollCap * (1.0 - 1.0 / (magnitude / kOverscrollCap + 1.0)) * sign;
        };
        return clamped + QPointF(soften(over.x()), soften(over.y()));
    }

    QPointF clampOffset(const QPointF& offset) const
    {
        // 边缘到边缘钳制：图片可滑动到任一边缘贴住视图对侧边缘，
        // 保证放大后图片任意部分（上/下/左/右）都能拖入视野。
        const QSizeF image = scaledImageSize();
        qreal dx = offset.x();
        if (image.width() <= width())
        {
            dx = 0.0;
        }
        else
        {
            const qreal halfOverflow = (image.width() - width()) / 2.0;
            dx = qBound(-halfOverflow, dx, halfOverflow);
        }
        qreal dy = offset.y();
        if (image.height() <= height())
        {
            dy = 0.0;
        }
        else
        {
            const qreal halfOverflow = (image.height() - height()) / 2.0;
            dy = qBound(-halfOverflow, dy, halfOverflow);
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
        if (m_indexChanged)
        {
            m_indexChanged(m_currentIndex);
        }
    }

    Control controlAt(const QPoint& pos) const
    {
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
        const int actionCount = 4;
        const int widthSum = actionCount * kControlSize + (actionCount - 1) * kFooterPillGap;
        const int x = (width() - widthSum) / 2;
        const int y = height() - kControlMargin - kControlSize - 12;
        return QRect(x, y, widthSum, kControlSize);
    }

    void drawRoundControl(QPainter& p, const QRect& rect, const QString& iconName, Control control,
                          const AntThemeTokens& token)
    {
        const bool hovered = m_hoverControl == control;
        if (hovered)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(token.colorFillTertiary);
            p.drawEllipse(QRectF(rect));
        }
        QColor iconColor = hovered ? token.colorPrimaryHover : token.colorTextSecondary;
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
        p.setPen(Qt::NoPen);
        p.setBrush(token.colorFillQuaternary);
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
            const QRect buttonRect(pill.left() + i * (kControlSize + kFooterPillGap),
                                   pill.top(),
                                   kControlSize,
                                   kControlSize);
            drawRoundControl(p, buttonRect, actions[i].icon, actions[i].control, token);
        }
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
    std::function<void(int)> m_indexChanged;
    QVariantAnimation* m_bounceAnimation = nullptr;
};

// Ant 窗口化图片预览（类似微信看图）：窗口带标题栏与关闭按钮，
// 内容区即 ImagePreviewSurface，无全屏遮罩。
class ImagePreviewDialog : public AntDialog
{
public:
    explicit ImagePreviewDialog(const QList<QPixmap>& pixmaps,
                                const QStringList& titles,
                                int startIndex,
                                QWidget* parent = nullptr)
        : AntDialog(parent)
        , m_pixmaps(pixmaps)
        , m_titles(titles)
    {
        setAttribute(Qt::WA_DeleteOnClose);

        m_surface = new ImagePreviewSurface(pixmaps, this);
        setContentWidget(m_surface);
        m_surface->setCurrentIndex(startIndex);
        m_surface->setIndexChangedHandler([this](int) { syncTitle(); });

        // 按首图比例开窗，上限 80% 屏幕，保底 420x320，并居中
        const int firstIndex = qMax(0, qMin(startIndex, m_pixmaps.size() - 1));
        const QPixmap& first = m_pixmaps.at(firstIndex);
        const QScreen* screen = QApplication::screenAt(QCursor::pos());
        if (!screen)
        {
            screen = QApplication::primaryScreen();
        }
        const QSize maxSize = screen ? screen->availableSize() * 0.8 : QSize(1024, 720);
        QSize windowSize = first.size().scaled(maxSize, Qt::KeepAspectRatio);
        windowSize = windowSize.expandedTo(QSize(420, 320)).boundedTo(maxSize);
        resize(windowSize);
        if (screen)
        {
            const QRect available = screen->availableGeometry();
            move(available.center().x() - width() / 2, available.center().y() - height() / 2);
        }
        syncTitle();
    }

protected:
    void showEvent(QShowEvent* event) override
    {
        AntDialog::showEvent(event);
        m_surface->setFocus();
    }

private:
    void syncTitle()
    {
        const int index = m_surface ? m_surface->currentIndex() : 0;
        QString title = (index >= 0 && index < m_titles.size())
            ? (m_titles.at(index).isEmpty() ? QStringLiteral("图片预览") : m_titles.at(index))
            : QStringLiteral("图片预览");
        if (m_pixmaps.size() > 1)
        {
            title += QStringLiteral("（%1 / %2）").arg(index + 1).arg(m_pixmaps.size());
        }
        setWindowTitle(title);
    }

    ImagePreviewSurface* m_surface = nullptr;
    QList<QPixmap> m_pixmaps;
    QStringList m_titles;
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
    QStringList titles;
    int startIndex = 0;

    if (m_previewGroup.isEmpty())
    {
        pixmaps.append(m_pixmap);
        titles.append(m_alt);
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
                titles.append(img->m_alt);
                if (img == this)
                    startIndex = pixmaps.size() - 1;
            }
        }
        if (index >= 0 && index < pixmaps.size())
            startIndex = index;
    }

    if (pixmaps.isEmpty())
        return;

    auto* dlg = new ImagePreviewDialog(pixmaps, titles, startIndex, window());
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
