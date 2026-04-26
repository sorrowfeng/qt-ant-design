#include "AntDockWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

#include "AntButton.h"
#include "core/AntTheme.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace
{

class DockTitleBar : public QWidget
{
public:
    explicit DockTitleBar(AntDockWidget* dock)
        : QWidget(dock), m_dock(dock)
    {
        setMouseTracking(true);

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(8, 0, 4, 0);
        layout->setSpacing(4);

        m_iconLabel = new QLabel(this);
        m_iconLabel->setFixedSize(16, 16);
        layout->addWidget(m_iconLabel);

        m_titleLabel = new QLabel(this);
        QFont f = m_titleLabel->font();
        f.setBold(true);
        m_titleLabel->setFont(f);
        m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        layout->addWidget(m_titleLabel, 1);

        auto* floatBtn = new AntButton(this);
        floatBtn->setButtonType(Ant::ButtonType::Text);
        floatBtn->setButtonSize(Ant::Size::Small);
        floatBtn->setText(QStringLiteral("⯈")); // float icon
        floatBtn->setFixedSize(26, 26);
        connect(floatBtn, &QPushButton::clicked, dock, [dock]() {
            dock->setFloating(!dock->isFloating());
        });
        layout->addWidget(floatBtn);

        auto* closeBtn = new AntButton(this);
        closeBtn->setButtonType(Ant::ButtonType::Text);
        closeBtn->setButtonSize(Ant::Size::Small);
        closeBtn->setText(QStringLiteral("✕")); // X mark
        closeBtn->setFixedSize(26, 26);
        connect(closeBtn, &QPushButton::clicked, dock, &QDockWidget::close);
        layout->addWidget(closeBtn);

        connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });

        updateFromDock();
    }

    void updateFromDock()
    {
        m_titleLabel->setText(m_dock->windowTitle());
        m_iconLabel->setPixmap(m_dock->windowIcon().pixmap(16, 16));
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.fillRect(rect(), m_dock->isFloating() ? token.colorBgElevated : token.colorBgContainer);
        p.setPen(QPen(token.colorSplit, 1));
        p.drawLine(rect().bottomLeft(), rect().bottomRight());
    }

private:
    AntDockWidget* m_dock;
    QLabel* m_iconLabel = nullptr;
    QLabel* m_titleLabel = nullptr;
};

} // namespace

AntDockWidget::AntDockWidget(QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
{
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable |
                QDockWidget::DockWidgetFloatable);
    setupTitleBar();

    connect(this, &QDockWidget::topLevelChanged, this, [this](bool) {
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
}

#if defined(Q_OS_WIN)
bool AntDockWidget::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    if (eventType != "windows_generic_MSG") return QDockWidget::nativeEvent(eventType, message, result);

    auto* msg = static_cast<MSG*>(message);
    if (msg->message == WM_NCHITTEST && isFloating())
    {
        const int border = 6;
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
