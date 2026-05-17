#include "AntDockWidget.h"

#include <QEvent>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSvgRenderer>

#include "core/AntTheme.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace
{
constexpr int kFloatingShadowMargin = 14;
constexpr int kFloatingCornerRadius = 8;
constexpr int kFloatingBorderWidth = 1;
constexpr int kFloatingTitleBarHeight = 40;
constexpr int kEmbeddedTitleBarHeight = 32;
constexpr int kFloatingTitleButtonWidth = 46;

QRectF centeredIconRect(const QRect& buttonRect, qreal iconSize = 14.0)
{
    if (buttonRect.isNull())
    {
        return {};
    }

    return QRectF(buttonRect.center().x() - iconSize / 2.0,
                  buttonRect.center().y() - iconSize / 2.0,
                  iconSize,
                  iconSize);
}

bool drawAntdIcon(const QString& iconName, const QRectF& iconRect, const QColor& color, QPainter* painter)
{
    if (iconName.isEmpty() || iconRect.isEmpty() || !painter)
    {
        return false;
    }

    QFile file(QStringLiteral(":/qt-ant-design/icons/antd/%1.svg").arg(iconName));
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QString svg = QString::fromUtf8(file.readAll());
    svg.replace(QStringLiteral("__PRIMARY__"), color.name(QColor::HexRgb));
    svg.replace(QStringLiteral("__SECONDARY__"), color.name(QColor::HexRgb));

    QSvgRenderer renderer(svg.toUtf8());
    if (!renderer.isValid())
    {
        return false;
    }

    renderer.render(painter, iconRect);
    return true;
}

class DockTitleButton : public QWidget
{
public:
    enum class Role
    {
        Minimize,
        Maximize,
        Close,
    };

    DockTitleButton(Role role, AntDockWidget* dock, QWidget* parent = nullptr)
        : QWidget(parent), m_role(role), m_dock(dock)
    {
        setFixedSize(kFloatingTitleButtonWidth, kFloatingTitleBarHeight);
        setCursor(Qt::ArrowCursor);
        setAttribute(Qt::WA_Hover, true);
        setMouseTracking(true);
    }

protected:
    bool event(QEvent* event) override
    {
        if (event->type() == QEvent::Enter)
        {
            m_hovered = true;
            update();
        }
        else if (event->type() == QEvent::Leave)
        {
            m_hovered = false;
            m_pressed = false;
            update();
        }
        return QWidget::event(event);
    }

    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            m_pressed = true;
            update();
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        const bool trigger = m_pressed && event->button() == Qt::LeftButton && rect().contains(event->pos());
        m_pressed = false;
        update();
        if (trigger && m_dock)
        {
            switch (m_role)
            {
            case Role::Minimize:
                m_dock->showMinimized();
                break;
            case Role::Maximize:
                m_dock->isMaximized() ? m_dock->showNormal() : m_dock->showMaximized();
                break;
            case Role::Close:
                m_dock->close();
                break;
            }
            event->accept();
            return;
        }
        QWidget::mouseReleaseEvent(event);
    }

    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

        const bool destructive = m_role == Role::Close;
        if (m_hovered || m_pressed)
        {
            painter.setPen(Qt::NoPen);
            if (destructive && m_hovered)
            {
                painter.setBrush(token.colorError);
            }
            else
            {
                painter.setBrush(m_pressed ? token.colorFillSecondary : token.colorFillTertiary);
            }
            painter.drawRect(rect());
        }

        QString iconName;
        switch (m_role)
        {
        case Role::Minimize:
            iconName = QStringLiteral("MinusOutlined");
            break;
        case Role::Maximize:
            iconName = m_dock && m_dock->isMaximized()
                ? QStringLiteral("FullscreenExitOutlined")
                : QStringLiteral("FullscreenOutlined");
            break;
        case Role::Close:
            iconName = QStringLiteral("CloseOutlined");
            break;
        }

        const QColor iconColor = destructive && m_hovered ? QColor(Qt::white)
                                                          : (m_hovered ? token.colorTextSecondary : token.colorText);
        drawAntdIcon(iconName, centeredIconRect(rect()), iconColor, &painter);
    }

private:
    Role m_role;
    AntDockWidget* m_dock = nullptr;
    bool m_hovered = false;
    bool m_pressed = false;
};

class DockTitleBar : public QWidget
{
public:
    explicit DockTitleBar(AntDockWidget* dock)
        : QWidget(dock), m_dock(dock)
    {
        setMouseTracking(true);
        setAttribute(Qt::WA_Hover, true);

        auto* layout = new QHBoxLayout(this);
        m_layout = layout;
        layout->setContentsMargins(12, 0, 0, 0);
        layout->setSpacing(8);

        m_iconLabel = new QLabel(this);
        m_iconLabel->setFixedSize(16, 16);
        m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        layout->addWidget(m_iconLabel);

        m_titleLabel = new QLabel(this);
        m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        QFont f = m_titleLabel->font();
        f.setBold(true);
        m_titleLabel->setFont(f);
        m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(m_titleLabel, 1);

        m_minimizeButton = new DockTitleButton(DockTitleButton::Role::Minimize, dock, this);
        m_maximizeButton = new DockTitleButton(DockTitleButton::Role::Maximize, dock, this);
        m_closeButton = new DockTitleButton(DockTitleButton::Role::Close, dock, this);
        layout->addWidget(m_minimizeButton);
        layout->addWidget(m_maximizeButton);
        layout->addWidget(m_closeButton);

        connect(antTheme, &AntTheme::themeChanged, this, [this]() {
            updateTheme();
            update();
        });

        updateFromDock();
        updateTheme();
    }

    void updateFromDock()
    {
        m_titleLabel->setText(m_dock->windowTitle());
        m_iconLabel->setPixmap(m_dock->windowIcon().pixmap(16, 16));
        updateChrome();
    }

    void updateTheme()
    {
        const auto& token = antTheme->tokens();
        QPalette pal = palette();
        pal.setColor(QPalette::WindowText, token.colorText);
        pal.setColor(QPalette::ButtonText, token.colorText);
        pal.setColor(QPalette::Text, token.colorText);
        setPalette(pal);

        QPalette titlePalette = m_titleLabel->palette();
        titlePalette.setColor(QPalette::WindowText, token.colorText);
        titlePalette.setColor(QPalette::Text, token.colorText);
        m_titleLabel->setPalette(titlePalette);
        m_iconLabel->setPalette(titlePalette);

        QFont f = m_titleLabel->font();
        f.setPixelSize(m_dock->isFloating() ? token.fontSizeLG : token.fontSize);
        f.setWeight(QFont::DemiBold);
        m_titleLabel->setFont(f);
        if (m_layout)
        {
            m_layout->setContentsMargins(m_dock->isFloating() ? 12 : 8, 0, 0, 0);
            m_layout->setSpacing(m_dock->isFloating() ? 8 : 4);
        }
        update();
    }

    void updateChrome()
    {
        const bool floating = m_dock->isFloating();
        m_minimizeButton->setVisible(floating);
        m_maximizeButton->setVisible(floating);
        m_closeButton->setFixedSize(floating ? QSize(kFloatingTitleButtonWidth, kFloatingTitleBarHeight)
                                             : QSize(26, kEmbeddedTitleBarHeight));
        setProperty("antDockTitleBarHeight", floating ? kFloatingTitleBarHeight : kEmbeddedTitleBarHeight);
        updateTheme();
        updateGeometry();
        update();
    }

    QSize sizeHint() const override
    {
        return QSize(240, m_dock->isFloating() ? kFloatingTitleBarHeight : kEmbeddedTitleBarHeight);
    }

    QSize minimumSizeHint() const override
    {
        return QSize(96, m_dock->isFloating() ? kFloatingTitleBarHeight : kEmbeddedTitleBarHeight);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (event->buttons() & Qt::LeftButton)
        {
            event->accept();
            return;
        }
        QWidget::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            event->accept();
            return;
        }
        QWidget::mouseReleaseEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
        {
            if (m_dock->isFloating())
            {
                m_dock->isMaximized() ? m_dock->showNormal() : m_dock->showMaximized();
            }
            else
            {
                m_dock->setFloating(true);
            }
            event->accept();
            return;
        }
        QWidget::mouseDoubleClickEvent(event);
    }

    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        const bool floating = m_dock->isFloating();
        const bool maximized = m_dock->isMaximized();
        const QColor bg = floating ? token.colorBgElevated : token.colorBgContainer;

        if (floating && !maximized)
        {
            QPainterPath titlePath;
            titlePath.addRoundedRect(QRectF(rect()), kFloatingCornerRadius, kFloatingCornerRadius);
            titlePath.addRect(QRectF(0, kFloatingCornerRadius, width(), qMax(0, height() - kFloatingCornerRadius)));
            p.fillPath(titlePath, bg);
        }
        else
        {
            p.fillRect(rect(), bg);
        }

        p.setPen(QPen(token.colorSplit, 1));
        p.drawLine(rect().bottomLeft(), rect().bottomRight());
    }

private:
    AntDockWidget* m_dock;
    QHBoxLayout* m_layout = nullptr;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
    DockTitleButton* m_minimizeButton = nullptr;
    DockTitleButton* m_maximizeButton = nullptr;
    DockTitleButton* m_closeButton = nullptr;
};

} // namespace

AntDockWidget::AntDockWidget(QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
{
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable |
                QDockWidget::DockWidgetFloatable);
    setupTitleBar();
    updateTheme();

    connect(this, &QDockWidget::topLevelChanged, this, [this](bool) {
        updateFloatingFrame();
        update();
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateTheme();
        update();
    });
}

AntDockWidget::AntDockWidget(const QString& title, QWidget* parent, Qt::WindowFlags flags)
    : AntDockWidget(parent, flags)
{
    setWindowTitle(title);
}

void AntDockWidget::setupTitleBar()
{
    auto* bar = new DockTitleBar(this);
    setTitleBarWidget(bar);

    connect(this, &QDockWidget::windowTitleChanged, bar, &DockTitleBar::updateFromDock);
    connect(this, &QDockWidget::windowIconChanged, bar, [bar](const QIcon&) { bar->updateFromDock(); });
    connect(this, &QDockWidget::topLevelChanged, bar, [bar](bool) { bar->updateChrome(); });
}

void AntDockWidget::setWidget(QWidget* widget)
{
    QDockWidget::setWidget(widget);
    updateTheme();
}

void AntDockWidget::updateTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::Base, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::Text, token.colorText);
    setPalette(pal);

    if (QWidget* content = widget())
    {
        QPalette contentPalette = content->palette();
        contentPalette.setColor(QPalette::Window, token.colorBgContainer);
        contentPalette.setColor(QPalette::Base, token.colorBgContainer);
        contentPalette.setColor(QPalette::WindowText, token.colorText);
        contentPalette.setColor(QPalette::Text, token.colorText);
        content->setPalette(contentPalette);
    }
}

void AntDockWidget::updateFloatingFrame()
{
    const bool floating = isFloating();
    const int shadowMargin = floating ? floatingShadowMargin() : 0;
    const int cornerRadius = floating ? floatingCornerRadius() : 0;
    setProperty("antDockFloatingFrame", floating);
    setProperty("antDockFloatingShadowMargin", shadowMargin);
    setProperty("antDockFloatingCornerRadius", cornerRadius);
    setProperty("antDockFloatingTitleBarHeight", floating ? kFloatingTitleBarHeight : 0);

    if (floating)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAutoFillBackground(false);
        setContentsMargins(shadowMargin, shadowMargin, shadowMargin, shadowMargin);

        Qt::WindowFlags wantedFlags = windowFlags();
        wantedFlags &= ~Qt::WindowType_Mask;
        wantedFlags |= Qt::Window;
        wantedFlags |= Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint;
        wantedFlags &= ~Qt::WindowTitleHint;
        if (windowFlags() != wantedFlags)
        {
            const bool wasVisible = isVisible();
            const QRect oldGeometry = geometry();
            setWindowFlags(wantedFlags);
            setGeometry(oldGeometry);
            if (wasVisible)
            {
                show();
            }
        }
    }
    else
    {
        setAttribute(Qt::WA_TranslucentBackground, false);
        setAttribute(Qt::WA_NoSystemBackground, false);
        setContentsMargins(0, 0, 0, 0);
    }

    if (m_floatingFrameActive != floating)
    {
        m_floatingFrameActive = floating;
        updateGeometry();
    }
    update();
}

QRect AntDockWidget::floatingPanelRect() const
{
    const int margin = floatingShadowMargin();
    return rect().adjusted(margin, margin, -margin, -margin);
}

int AntDockWidget::floatingShadowMargin() const
{
    return isMaximized() ? 0 : kFloatingShadowMargin;
}

int AntDockWidget::floatingCornerRadius() const
{
    return isMaximized() ? 0 : kFloatingCornerRadius;
}

void AntDockWidget::paintEvent(QPaintEvent* event)
{
    if (!isFloating())
    {
        QDockWidget::paintEvent(event);
        return;
    }

    Q_UNUSED(event)
    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRect panel = floatingPanelRect();
    if (panel.isEmpty())
    {
        return;
    }

    const int shadowMargin = floatingShadowMargin();
    const int cornerRadius = floatingCornerRadius();
    if (shadowMargin > 0)
    {
        antTheme->drawEffectShadow(&painter, panel, shadowMargin, cornerRadius, 1.15);
    }

    const auto& token = antTheme->tokens();
    QColor fill = token.colorBgElevated;
    QColor border = token.colorBorderSecondary;
    border.setAlphaF(antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.54 : 0.72);

    const QRectF panelRect = QRectF(panel).adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setPen(QPen(border, kFloatingBorderWidth));
    painter.setBrush(fill);
    if (cornerRadius > 0)
    {
        painter.drawRoundedRect(panelRect, cornerRadius, cornerRadius);
    }
    else
    {
        painter.drawRect(panelRect);
    }
}

void AntDockWidget::resizeEvent(QResizeEvent* event)
{
    QDockWidget::resizeEvent(event);
    if (isFloating())
    {
        update();
    }
}

void AntDockWidget::showEvent(QShowEvent* event)
{
    updateFloatingFrame();
    QDockWidget::showEvent(event);
}

void AntDockWidget::changeEvent(QEvent* event)
{
    QDockWidget::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange && isFloating())
    {
        updateFloatingFrame();
        if (QWidget* bar = titleBarWidget())
        {
            bar->update();
        }
    }
}

#if defined(Q_OS_WIN)
bool AntDockWidget::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    if (eventType != "windows_generic_MSG") return QDockWidget::nativeEvent(eventType, message, result);

    auto* msg = static_cast<MSG*>(message);
    if (msg->message == WM_NCHITTEST && isFloating() && !isMaximized())
    {
        const int border = qMax(8, kFloatingShadowMargin);
        POINT pt = msg->pt;
        ScreenToClient(reinterpret_cast<HWND>(winId()), &pt);
        RECT r;
        GetClientRect(reinterpret_cast<HWND>(winId()), &r);

        bool left = pt.x < border;
        bool right = pt.x > r.right - border;
        bool top = pt.y < border;
        bool bottom = pt.y > r.bottom - border;

        if (top && left) *result = HTTOPLEFT;
        else if (top && right) *result = HTTOPRIGHT;
        else if (bottom && left) *result = HTBOTTOMLEFT;
        else if (bottom && right) *result = HTBOTTOMRIGHT;
        else if (left) *result = HTLEFT;
        else if (right) *result = HTRIGHT;
        else if (top) *result = HTTOP;
        else if (bottom) *result = HTBOTTOM;
        else return false;
        return true;
    }
    return QDockWidget::nativeEvent(eventType, message, result);
}
#endif
