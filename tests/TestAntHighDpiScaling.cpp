#include <QApplication>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QTest>
#include <QVBoxLayout>

#include <cmath>

#include "core/AntDesign.h"
#include "core/AntTheme.h"
#include "widgets/AntButton.h"
#include "widgets/AntIcon.h"
#include "widgets/AntList.h"
#include "widgets/AntNav.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntWindow.h"

class TestAntHighDpiScaling : public QObject
{
    Q_OBJECT

private slots:
    void startupPolicyIsAppliedBeforeQApplication();
    void widgetGrabKeepsLogicalGeometryAtScale();
    void antWindowKeepsLogicalFrameMetricsAtScale();
};

namespace
{
qreal envScaleFactor()
{
    bool ok = false;
    const qreal scale = QString::fromLocal8Bit(qgetenv("QT_SCALE_FACTOR")).toDouble(&ok);
    return ok && scale > 0.0 ? scale : 1.0;
}

void verifyLogicalGrab(const QWidget& widget, const QPixmap& grab)
{
    QVERIFY2(!grab.isNull(), "grab should produce a pixmap");

    const qreal grabDpr = qMax<qreal>(1.0, grab.devicePixelRatio());
    const qreal widgetDpr = qMax<qreal>(1.0, widget.devicePixelRatioF());
    const QSizeF logicalSize(static_cast<qreal>(grab.width()) / grabDpr,
                             static_cast<qreal>(grab.height()) / grabDpr);

    QVERIFY2(std::abs(logicalSize.width() - widget.width()) <= 1.0,
             qPrintable(QStringLiteral("grab logical width %1 should match widget width %2 at DPR %3")
                            .arg(logicalSize.width())
                            .arg(widget.width())
                            .arg(grabDpr)));
    QVERIFY2(std::abs(logicalSize.height() - widget.height()) <= 1.0,
             qPrintable(QStringLiteral("grab logical height %1 should match widget height %2 at DPR %3")
                            .arg(logicalSize.height())
                            .arg(widget.height())
                            .arg(grabDpr)));

    const qreal requestedScale = envScaleFactor();
    if (requestedScale > 1.01)
    {
        QVERIFY2(widgetDpr > 1.01,
                 qPrintable(QStringLiteral("widget DPR %1 should be high-DPI when QT_SCALE_FACTOR is %2")
                                .arg(widgetDpr)
                                .arg(requestedScale)));
        QVERIFY2(std::abs(grabDpr - widgetDpr) <= 0.05,
                 qPrintable(QStringLiteral("grab DPR %1 should track widget DPR %2")
                                .arg(grabDpr)
                                .arg(widgetDpr)));
    }
}

QPixmap makePixmapIcon()
{
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(22, 119, 255));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QRectF(4, 4, 56, 56));
    painter.setBrush(QColor(255, 255, 255));
    painter.drawEllipse(QRectF(21, 15, 10, 10));
    painter.drawEllipse(QRectF(35, 15, 10, 10));
    painter.drawRoundedRect(QRectF(20, 38, 25, 6), 3, 3);
    return pixmap;
}
} // namespace

void TestAntHighDpiScaling::startupPolicyIsAppliedBeforeQApplication()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QVERIFY(QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling));
    QVERIFY(QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps));
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QCOMPARE(QGuiApplication::highDpiScaleFactorRoundingPolicy(),
             Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

void TestAntHighDpiScaling::widgetGrabKeepsLogicalGeometryAtScale()
{
    QWidget host;
    host.setWindowTitle(QStringLiteral("Ant High DPI Scaling Probe"));
    host.resize(420, 260);

    auto* layout = new QVBoxLayout(&host);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    auto* row = new QWidget(&host);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);

    auto* primary = new AntButton(QStringLiteral("Primary"), row);
    primary->setButtonType(Ant::ButtonType::Primary);
    rowLayout->addWidget(primary);

    auto* icon = new AntIcon(row);
    icon->setIconType(Ant::IconType::Search);
    icon->setIconSize(24);
    rowLayout->addWidget(icon);
    rowLayout->addStretch();
    layout->addWidget(row);

    auto* list = new AntList(&host);
    list->setFixedHeight(96);
    auto* item = new AntListItem(list);
    item->setText(QStringLiteral("Pixmap item"));
    item->setIconPixmap(makePixmapIcon());
    item->setIconSize(QSize(24, 24));
    list->addItem(item);
    layout->addWidget(list);

    auto* nav = new AntNav(&host);
    nav->setFixedHeight(64);
    const int navIndex = nav->addItem(QStringLiteral("Navigation"));
    nav->setItemIcon(navIndex, Ant::IconType::Home);
    layout->addWidget(nav);

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));
    QTRY_VERIFY(host.windowHandle() != nullptr);

    const qreal requestedScale = envScaleFactor();
    if (requestedScale > 1.01)
    {
        QVERIFY2(host.devicePixelRatioF() > 1.01,
                 qPrintable(QStringLiteral("widget DPR %1 should be high-DPI when QT_SCALE_FACTOR is %2")
                                .arg(host.devicePixelRatioF())
                                .arg(requestedScale)));
    }

    verifyLogicalGrab(host, host.grab());
}

void TestAntHighDpiScaling::antWindowKeepsLogicalFrameMetricsAtScale()
{
    AntWindow window;
    window.setWindowTitle(QStringLiteral("AntWindow High DPI Probe"));
    window.resize(520, 340);

    auto* content = new QWidget(&window);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(20, 20, 20, 20);
    auto* button = new AntButton(QStringLiteral("Window content"), content);
    button->setButtonType(Ant::ButtonType::Primary);
    layout->addWidget(button);
    layout->addStretch();
    window.setCentralWidget(content);

    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QTRY_VERIFY(window.windowHandle() != nullptr);

    const QRect closeRect = window.titleBarButtonRect(AntWindow::TitleBarButton::Close);
    QVERIFY(closeRect.isValid());
    QCOMPARE(closeRect.height(), AntWindow::TitleBarHeight);
    QVERIFY(closeRect.right() <= window.width());

    verifyLogicalGrab(window, window.grab());
    window.forceClose();
}

int main(int argc, char** argv)
{
    AntDesign::configureHighDpi();

    QApplication app(argc, argv);
    AntDesign::initialize(&app);
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    TestAntHighDpiScaling test;
    return QTest::qExec(&test, argc, argv);
}

#include "TestAntHighDpiScaling.moc"
