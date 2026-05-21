#include "AntDrawer.h"

#include <QAbstractButton>
#include <QApplication>
#include <QEnterEvent>
#include <QEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QScreen>
#include <QShowEvent>
#include <QVariant>
#include <QVBoxLayout>

#include "core/AntTheme.h"
#include "../styles/AntDrawerStyle.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int DrawerHeaderHeight = 56;
constexpr int DrawerOverlayShadowSize = 32;

class DrawerCloseButton : public QAbstractButton
{
public:
    explicit DrawerCloseButton(QWidget* parent = nullptr)
        : QAbstractButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setFixedSize(28, 28);
    }

    QSize sizeHint() const override
    {
        return QSize(28, 28);
    }

protected:
    void enterEvent(QEnterEvent* event) override
    {
        m_hovered = true;
        update();
        QAbstractButton::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        m_hovered = false;
        update();
        QAbstractButton::leaveEvent(event);
    }

    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        const auto& token = antTheme->tokens();

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_hovered ? token.colorFillTertiary : Qt::transparent);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), token.borderRadiusSM, token.borderRadiusSM);

        painter.setPen(QPen(m_hovered ? token.colorTextSecondary : token.colorTextTertiary,
                            1.8,
                            Qt::SolidLine,
                            Qt::RoundCap));
        const QPointF c = rect().center();
        painter.drawLine(QPointF(c.x() - 4.5, c.y() - 4.5), QPointF(c.x() + 4.5, c.y() + 4.5));
        painter.drawLine(QPointF(c.x() + 4.5, c.y() - 4.5), QPointF(c.x() - 4.5, c.y() + 4.5));
    }

private:
    bool m_hovered = false;
};

QRect fallbackGeometry()
{
    if (QScreen* screen = QGuiApplication::primaryScreen())
    {
        return screen->availableGeometry();
    }
    return QRect(0, 0, 1280, 720);
}
} // namespace

class AntDrawer::DrawerPanel : public QWidget
{
public:
    explicit DrawerPanel(AntDrawer* owner, QWidget* parent = nullptr)
        : QWidget(parent)
        , m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, false);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        if (!m_owner)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

        const QRect panelRect = rect();

        // Draw shadow on the edge facing the screen center
        drawEdgeShadow(painter, panelRect, token);

        // Draw panel background
        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorBgContainer);
        painter.drawRect(panelRect);

        // Draw title border bottom
        if (!m_owner->m_title.isEmpty() || m_owner->m_closable)
        {
            painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
            painter.drawLine(QPoint(0, DrawerHeaderHeight), QPoint(panelRect.width(), DrawerHeaderHeight));
        }
    }

private:
    void drawEdgeShadow(QPainter& painter, const QRect& rect, const AntThemeTokens& token) const
    {
        if (!m_owner)
        {
            return;
        }

        const int shadowWidth = 3;
        QColor shadowColor = token.colorShadow.isValid() ? token.colorShadow : QColor(0, 0, 0, 40);
        painter.setPen(Qt::NoPen);

        switch (m_owner->m_placement)
        {
        case Ant::DrawerPlacement::Right:
        {
            QLinearGradient gradient(rect.topLeft(), rect.topLeft() + QPoint(shadowWidth, 0));
            gradient.setColorAt(0.0, shadowColor);
            gradient.setColorAt(1.0, Qt::transparent);
            painter.setBrush(gradient);
            painter.drawRect(QRect(rect.topLeft(), QSize(shadowWidth, rect.height())));
            break;
        }
        case Ant::DrawerPlacement::Left:
        {
            QLinearGradient gradient(rect.topRight() - QPoint(shadowWidth - 1, 0), rect.topRight());
            gradient.setColorAt(0.0, Qt::transparent);
            gradient.setColorAt(1.0, shadowColor);
            painter.setBrush(gradient);
            painter.drawRect(QRect(rect.topRight() - QPoint(shadowWidth - 1, 0), QSize(shadowWidth, rect.height())));
            break;
        }
        case Ant::DrawerPlacement::Bottom:
        {
            QLinearGradient gradient(rect.topLeft(), rect.topLeft() + QPoint(0, shadowWidth));
            gradient.setColorAt(0.0, shadowColor);
            gradient.setColorAt(1.0, Qt::transparent);
            painter.setBrush(gradient);
            painter.drawRect(QRect(rect.topLeft(), QSize(rect.width(), shadowWidth)));
            break;
        }
        case Ant::DrawerPlacement::Top:
        {
            QLinearGradient gradient(rect.bottomLeft() - QPoint(0, shadowWidth - 1), rect.bottomLeft());
            gradient.setColorAt(0.0, Qt::transparent);
            gradient.setColorAt(1.0, shadowColor);
            painter.setBrush(gradient);
            painter.drawRect(QRect(rect.bottomLeft() - QPoint(0, shadowWidth - 1), QSize(rect.width(), shadowWidth)));
            break;
        }
        }
    }

    AntDrawer* m_owner = nullptr;
};

AntDrawer::AntDrawer(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntDrawerStyle>(this);
    setAttribute(Qt::WA_StyledBackground, false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFocusPolicy(Qt::StrongFocus);
    hide();

    m_panel = new DrawerPanel(this, this);
    m_headerWidget = new QWidget(m_panel);
    m_bodyWidget = new QWidget(m_panel);

    auto* panelLayout = new QVBoxLayout(m_panel);
    panelLayout->setContentsMargins(0, 0, 0, 0);
    panelLayout->setSpacing(0);

    auto* headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(24, 0, 24, 0);
    headerLayout->setSpacing(4);

    m_closeButton = new DrawerCloseButton(m_headerWidget);
    connect(m_closeButton, &QAbstractButton::clicked, this, [this]() {
        close();
    });
    headerLayout->addWidget(m_closeButton, 0, Qt::AlignLeft | Qt::AlignVCenter);

    m_titleLabel = new QLabel(m_headerWidget);
    m_titleLabel->setWordWrap(false);
    headerLayout->addWidget(m_titleLabel, 1);

    m_headerWidget->setFixedHeight(DrawerHeaderHeight);

    auto* bodyLayout = new QVBoxLayout(m_bodyWidget);
    bodyLayout->setContentsMargins(24, 24, 24, 24);
    bodyLayout->setSpacing(0);

    panelLayout->addWidget(m_headerWidget);
    panelLayout->addWidget(m_bodyWidget, 1);

    m_animation = new QPropertyAnimation(m_panel, "geometry", this);
    m_animation->setDuration(200);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_animation, &QPropertyAnimation::finished, this, &AntDrawer::onAnimationFinished);
    connect(m_animation, &QPropertyAnimation::valueChanged, this, [this](const QVariant& value) {
        updateAnimationFrame(value.toRect());
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        m_themeKey.clear();
        syncTheme();
        if (m_panel)
        {
            m_panel->update();
        }
        requestDrawerUpdate(rect(), QStringLiteral("theme"));
    });
    syncTheme();
    syncDrawerPerfCounters();
}

QString AntDrawer::title() const { return m_title; }

void AntDrawer::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    m_themeKey.clear();
    syncTheme();
    if (m_open)
    {
        updatePanelGeometry();
    }
    Q_EMIT titleChanged(m_title);
}

Ant::DrawerPlacement AntDrawer::placement() const { return m_placement; }

void AntDrawer::setPlacement(Ant::DrawerPlacement placement)
{
    if (m_placement == placement)
    {
        return;
    }
    m_placement = placement;
    invalidateDrawerGeometry();
    m_lastPanelAnimationGeometry = QRect();
    m_lastMaskProgress = -1.0;
    m_themeKey.clear();
    syncTheme();
    if (m_open)
    {
        updatePanelGeometry();
    }
    Q_EMIT placementChanged(m_placement);
}

int AntDrawer::drawerWidth() const { return m_drawerWidth; }

void AntDrawer::setDrawerWidth(int width)
{
    width = qMax(100, width);
    if (m_drawerWidth == width)
    {
        return;
    }
    m_drawerWidth = width;
    invalidateDrawerGeometry();
    if (m_open)
    {
        updatePanelGeometry();
    }
    Q_EMIT drawerWidthChanged(m_drawerWidth);
}

int AntDrawer::drawerHeight() const { return m_drawerHeight; }

void AntDrawer::setDrawerHeight(int height)
{
    height = qMax(100, height);
    if (m_drawerHeight == height)
    {
        return;
    }
    m_drawerHeight = height;
    invalidateDrawerGeometry();
    if (m_open)
    {
        updatePanelGeometry();
    }
    Q_EMIT drawerHeightChanged(m_drawerHeight);
}

bool AntDrawer::isClosable() const { return m_closable; }

void AntDrawer::setClosable(bool closable)
{
    if (m_closable == closable)
    {
        return;
    }
    m_closable = closable;
    m_themeKey.clear();
    syncTheme();
    Q_EMIT closableChanged(m_closable);
}

bool AntDrawer::isMaskClosable() const { return m_maskClosable; }

void AntDrawer::setMaskClosable(bool closable)
{
    if (m_maskClosable == closable)
    {
        return;
    }
    m_maskClosable = closable;
    Q_EMIT maskClosableChanged(m_maskClosable);
}

bool AntDrawer::isOpen() const { return m_open; }

void AntDrawer::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }

    if (open)
    {
        AntDrawer::open();
    }
    else
    {
        AntDrawer::close();
    }
}

QWidget* AntDrawer::bodyWidget() const
{
    return m_customBodyWidget;
}

void AntDrawer::setBodyWidget(QWidget* widget)
{
    if (m_customBodyWidget == widget)
    {
        return;
    }

    if (m_customBodyWidget)
    {
        m_customBodyWidget->setParent(nullptr);
    }

    m_customBodyWidget = widget;
    if (m_customBodyWidget)
    {
        m_customBodyWidget->setParent(m_bodyWidget);
        if (auto* layout = m_bodyWidget->layout())
        {
            layout->addWidget(m_customBodyWidget);
        }
        m_customBodyWidget->show();
    }
}

void AntDrawer::open()
{
    if (m_open || m_animating)
    {
        return;
    }

    ensureHostWidget();
    updateOverlayGeometry();
    m_open = true;
    show();
    raise();
    activateWindow();
    setFocus(Qt::OtherFocusReason);

    const QRect startGeometry = panelStartGeometry();
    const QRect endGeometry = panelEndGeometry();
    m_panel->setGeometry(startGeometry);
    m_panel->show();
    m_panel->raise();

    startAnimation(startGeometry, endGeometry);
    Q_EMIT openChanged(true);
}

void AntDrawer::close()
{
    if (!m_open || m_animating)
    {
        return;
    }

    Q_EMIT aboutToClose();

    m_open = false;
    startAnimation(m_panel->geometry(), panelStartGeometry());
    Q_EMIT openChanged(false);
}

void AntDrawer::toggle()
{
    if (m_open)
    {
        close();
    }
    else
    {
        open();
    }
}

bool AntDrawer::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_hostWidget)
    {
        switch (event->type())
        {
        case QEvent::Resize:
        case QEvent::Move:
        case QEvent::Show:
            if (m_open)
            {
                updateOverlayGeometry();
            }
            break;
        case QEvent::Hide:
        case QEvent::Close:
            if (m_open)
            {
                setOpen(false);
            }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntDrawer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntDrawer::mousePressEvent(QMouseEvent* event)
{
    if (m_panel && !m_panel->geometry().contains(event->pos()) && m_maskClosable)
    {
        close();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntDrawer::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && m_closable)
    {
        close();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void AntDrawer::resizeEvent(QResizeEvent* event)
{
    invalidateDrawerGeometry();
    QWidget::resizeEvent(event);
}

void AntDrawer::showEvent(QShowEvent* event)
{
    updateOverlayGeometry();
    QWidget::showEvent(event);
}

void AntDrawer::ensureHostWidget()
{
    QWidget* host = parentWidget() ? parentWidget()->window() : nullptr;
    if (!host)
    {
        return;
    }
    if (m_hostWidget == host)
    {
        if (parentWidget() != host)
        {
            setParent(host);
        }
        return;
    }

    releaseHostWidget();
    m_hostWidget = host;
    setParent(m_hostWidget);
    m_hostWidget->installEventFilter(this);
}

void AntDrawer::releaseHostWidget()
{
    if (m_hostWidget)
    {
        m_hostWidget->removeEventFilter(this);
        m_hostWidget = nullptr;
    }
}

void AntDrawer::syncTheme()
{
    const auto& token = antTheme->tokens();
    const QString key = QStringLiteral("%1:%2:%3:%4")
        .arg(static_cast<int>(antTheme->themeMode()))
        .arg(m_closable ? 1 : 0)
        .arg(m_title)
        .arg(font().toString());
    if (m_themeKey == key)
    {
        ++m_themeSkipCount;
        syncDrawerPerfCounters();
        return;
    }
    m_themeKey = key;
    ++m_themeApplyCount;

    QFont titleFont = font();
    titleFont.setPixelSize(token.fontSizeLG);
    titleFont.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(titleFont);
    QPalette titlePalette = m_titleLabel->palette();
    titlePalette.setColor(QPalette::WindowText, token.colorText);
    m_titleLabel->setPalette(titlePalette);
    m_titleLabel->setText(m_title);
    m_titleLabel->setVisible(!m_title.isEmpty());

    m_closeButton->setVisible(m_closable);
    m_headerWidget->setVisible(!m_title.isEmpty() || m_closable);
    syncDrawerPerfCounters();
}

void AntDrawer::updateOverlayGeometry()
{
    const QRect target = m_hostWidget ? m_hostWidget->rect() : fallbackGeometry();
    const bool geometryChanged = geometry() != target;
    if (geometryChanged)
    {
        setGeometry(target);
        invalidateDrawerGeometry();
        ++m_overlayGeometryApplyCount;
    }
    else
    {
        ++m_overlayGeometrySkipCount;
    }
    syncDrawerPerfCounters();

    if (m_hostWidget)
    {
        raise();
    }
    if (m_open)
    {
        updatePanelGeometry();
    }
}

void AntDrawer::updatePanelGeometry()
{
    if (!m_panel || m_animating)
    {
        return;
    }
    syncTheme();
    const QRect target = panelEndGeometry();
    if (m_panel->geometry() == target)
    {
        ++m_panelGeometrySkipCount;
        syncDrawerPerfCounters();
        return;
    }
    m_panel->setGeometry(target);
    ++m_panelGeometryApplyCount;
    syncDrawerPerfCounters();
}

QRect AntDrawer::panelEndGeometry() const
{
    return drawerGeometryCache().endGeometry;
}

QRect AntDrawer::panelStartGeometry() const
{
    return drawerGeometryCache().startGeometry;
}

void AntDrawer::startAnimation(const QRect& start, const QRect& end)
{
    m_animating = true;
    m_lastPanelAnimationGeometry = start;
    m_lastMaskProgress = maskProgressForPanelGeometry(start);
    m_animation->stop();
    m_animation->setStartValue(start);
    m_animation->setEndValue(end);
    m_animation->start();
    requestDrawerUpdate(rect(), QStringLiteral("animationStart"));
}

void AntDrawer::updateAnimationFrame(const QRect& panelGeometry)
{
    requestDrawerUpdate(animationDirtyRegion(panelGeometry), QStringLiteral("animation"));
}

void AntDrawer::onAnimationFinished()
{
    m_animating = false;
    m_lastPanelAnimationGeometry = QRect();
    m_lastMaskProgress = -1.0;

    if (m_open)
    {
        Q_EMIT opened();
    }
    else
    {
        m_panel->hide();
        hide();
        Q_EMIT closed();
    }
}

qreal AntDrawer::maskProgress() const
{
    return maskProgressForPanelGeometry(m_panel ? m_panel->geometry() : QRect());
}

QRect AntDrawer::currentPanelGeometry() const
{
    return m_panel ? m_panel->geometry() : QRect();
}

const AntDrawer::GeometryCache& AntDrawer::drawerGeometryCache() const
{
    const bool cacheMatches = m_geometryCache.valid
        && m_geometryCache.overlaySize == size()
        && m_geometryCache.placement == m_placement
        && m_geometryCache.drawerWidth == m_drawerWidth
        && m_geometryCache.drawerHeight == m_drawerHeight;
    if (cacheMatches)
    {
        ++m_geometryCacheHitCount;
        syncDrawerPerfCounters();
        return m_geometryCache;
    }

    ++m_geometryBuildCount;
    GeometryCache cache;
    cache.valid = true;
    cache.overlaySize = size();
    cache.placement = m_placement;
    cache.drawerWidth = m_drawerWidth;
    cache.drawerHeight = m_drawerHeight;

    const int w = width();
    const int h = height();
    switch (m_placement)
    {
    case Ant::DrawerPlacement::Right:
    {
        const int dw = qMin(m_drawerWidth, w);
        cache.endGeometry = QRect(w - dw, 0, dw, h);
        cache.startGeometry = QRect(w, 0, dw, h);
        break;
    }
    case Ant::DrawerPlacement::Left:
    {
        const int dw = qMin(m_drawerWidth, w);
        cache.endGeometry = QRect(0, 0, dw, h);
        cache.startGeometry = QRect(-dw, 0, dw, h);
        break;
    }
    case Ant::DrawerPlacement::Bottom:
    {
        const int dh = qMin(m_drawerHeight, h);
        cache.endGeometry = QRect(0, h - dh, w, dh);
        cache.startGeometry = QRect(0, h, w, dh);
        break;
    }
    case Ant::DrawerPlacement::Top:
    {
        const int dh = qMin(m_drawerHeight, h);
        cache.endGeometry = QRect(0, 0, w, dh);
        cache.startGeometry = QRect(0, -dh, w, dh);
        break;
    }
    }

    m_geometryCache = cache;
    syncDrawerPerfCounters();
    return m_geometryCache;
}

void AntDrawer::invalidateDrawerGeometry() const
{
    m_geometryCache.valid = false;
}

qreal AntDrawer::maskProgressForPanelGeometry(const QRect& panelGeometry) const
{
    if (!m_panel)
    {
        return 0.0;
    }
    const QRect end = panelEndGeometry();
    const QRect start = panelStartGeometry();
    qreal denom = 0.0;
    qreal travelled = 0.0;
    switch (m_placement)
    {
    case Ant::DrawerPlacement::Right:
    case Ant::DrawerPlacement::Left:
        denom = qAbs(static_cast<qreal>(end.x() - start.x()));
        travelled = qAbs(static_cast<qreal>(panelGeometry.x() - start.x()));
        break;
    case Ant::DrawerPlacement::Top:
    case Ant::DrawerPlacement::Bottom:
        denom = qAbs(static_cast<qreal>(end.y() - start.y()));
        travelled = qAbs(static_cast<qreal>(panelGeometry.y() - start.y()));
        break;
    }
    if (denom <= 0.0)
    {
        return m_open ? 1.0 : 0.0;
    }
    return qBound(0.0, travelled / denom, 1.0);
}

QRect AntDrawer::panelShadowRegion(const QRect& panelGeometry) const
{
    if (panelGeometry.isEmpty())
    {
        return QRect();
    }

    switch (m_placement)
    {
    case Ant::DrawerPlacement::Right:
        return QRect(panelGeometry.left() - DrawerOverlayShadowSize,
                     panelGeometry.top(),
                     DrawerOverlayShadowSize,
                     panelGeometry.height()).intersected(rect());
    case Ant::DrawerPlacement::Left:
        return QRect(panelGeometry.right() + 1,
                     panelGeometry.top(),
                     DrawerOverlayShadowSize,
                     panelGeometry.height()).intersected(rect());
    case Ant::DrawerPlacement::Bottom:
        return QRect(panelGeometry.left(),
                     panelGeometry.top() - DrawerOverlayShadowSize,
                     panelGeometry.width(),
                     DrawerOverlayShadowSize).intersected(rect());
    case Ant::DrawerPlacement::Top:
        return QRect(panelGeometry.left(),
                     panelGeometry.bottom() + 1,
                     panelGeometry.width(),
                     DrawerOverlayShadowSize).intersected(rect());
    }
    return QRect();
}

QRegion AntDrawer::animationDirtyRegion(const QRect& panelGeometry)
{
    QRegion dirty;
    const qreal nextProgress = maskProgressForPanelGeometry(panelGeometry);
    if (m_lastMaskProgress < 0.0 || !qFuzzyCompare(m_lastMaskProgress + 1.0, nextProgress + 1.0))
    {
        dirty += rect();
        ++m_maskRegionUpdateCount;
    }
    else
    {
        const QRect lastPanelDirty = m_lastPanelAnimationGeometry.united(panelShadowRegion(m_lastPanelAnimationGeometry));
        const QRect nextPanelDirty = panelGeometry.united(panelShadowRegion(panelGeometry));
        dirty += lastPanelDirty;
        dirty += nextPanelDirty;
    }

    ++m_panelRegionUpdateCount;
    m_lastPanelAnimationGeometry = panelGeometry;
    m_lastMaskProgress = nextProgress;
    return dirty;
}

void AntDrawer::requestDrawerUpdate(const QRegion& region, const QString& mode)
{
    m_lastUpdateMode = mode;
    ++m_animationRegionUpdateCount;
    syncDrawerPerfCounters();
    if (region.isEmpty())
    {
        update();
        return;
    }
    update(region);
}

void AntDrawer::syncDrawerPerfCounters() const
{
    auto* that = const_cast<AntDrawer*>(this);
    that->setProperty("antDrawerGeometryBuildCount", m_geometryBuildCount);
    that->setProperty("antDrawerGeometryCacheHitCount", m_geometryCacheHitCount);
    that->setProperty("antDrawerOverlayGeometryApplyCount", m_overlayGeometryApplyCount);
    that->setProperty("antDrawerOverlayGeometrySkipCount", m_overlayGeometrySkipCount);
    that->setProperty("antDrawerPanelGeometryApplyCount", m_panelGeometryApplyCount);
    that->setProperty("antDrawerPanelGeometrySkipCount", m_panelGeometrySkipCount);
    that->setProperty("antDrawerAnimationRegionUpdateCount", m_animationRegionUpdateCount);
    that->setProperty("antDrawerMaskRegionUpdateCount", m_maskRegionUpdateCount);
    that->setProperty("antDrawerPanelRegionUpdateCount", m_panelRegionUpdateCount);
    that->setProperty("antDrawerThemeApplyCount", m_themeApplyCount);
    that->setProperty("antDrawerThemeSkipCount", m_themeSkipCount);
    that->setProperty("antDrawerLastUpdateMode", m_lastUpdateMode);
}
