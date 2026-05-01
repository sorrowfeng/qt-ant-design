#include <QColor>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPalette>
#include <QTest>
#include <QVBoxLayout>
#include <QWidget>

#include <cmath>

#include "core/AntTheme.h"
#include "widgets/AntAlert.h"
#include "widgets/AntBadge.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntCheckbox.h"
#include "widgets/AntDescriptions.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntLayout.h"
#include "widgets/AntList.h"
#include "widgets/AntMessage.h"
#include "widgets/AntMenu.h"
#include "widgets/AntModal.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPagination.h"
#include "widgets/AntPopover.h"
#include "widgets/AntProgress.h"
#include "widgets/AntRadio.h"
#include "widgets/AntSteps.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTable.h"
#include "widgets/AntTag.h"

class TestAntVisualRegression : public QObject
{
    Q_OBJECT

private slots:
    void primaryButtonKeepsTokenFill();
    void alertSemanticBackgroundsStayVisible();
    void progressStatusColorsStayVisible();
    void navigationActiveIndicatorsStayPrimary();
    void inputNumberHandlersStayVisible();
    void selectionControlsKeepPrimaryStateFills();
    void tagAndBadgeStatusColorsStayVisible();
    void feedbackSurfacesKeepElevatedTokenFill();
    void dataDisplayBordersAndSeparatorsStayVisible();
    void navigationAndLayoutStructureStayVisible();
    void complexPopupSurfacesStayElevated();
    void lightAndDarkThemesRenderDifferentSurfaces();
};

namespace
{
class ThemeModeGuard
{
public:
    ThemeModeGuard()
        : m_original(antTheme->themeMode())
    {
    }

    ~ThemeModeGuard()
    {
        antTheme->setThemeMode(m_original);
        QCoreApplication::processEvents();
    }

private:
    Ant::ThemeMode m_original;
};

QImage renderWidget(QWidget* widget, const QSize& size)
{
    widget->resize(size);
    widget->ensurePolished();
    QCoreApplication::sendPostedEvents(widget, QEvent::Polish);
    widget->show();
    QCoreApplication::processEvents();

    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        widget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    }
    widget->hide();
    return image;
}

QImage renderCurrentWidget(QWidget* widget)
{
    widget->ensurePolished();
    QCoreApplication::sendPostedEvents(widget, QEvent::Polish);
    QCoreApplication::processEvents();

    QImage image(widget->size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        widget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    }
    return image;
}

void prepareHost(QWidget& host, const QSize& size, const QColor& background)
{
    QPalette palette = host.palette();
    palette.setColor(QPalette::Window, background);
    host.setPalette(palette);
    host.setAutoFillBackground(true);
    host.resize(size);
    host.show();
    QCoreApplication::processEvents();
}

bool nearColor(const QColor& actual, const QColor& expected, int tolerance)
{
    if (actual.alpha() < 24)
    {
        return false;
    }
    return std::abs(actual.red() - expected.red()) <= tolerance &&
           std::abs(actual.green() - expected.green()) <= tolerance &&
           std::abs(actual.blue() - expected.blue()) <= tolerance;
}

int countNearColor(const QImage& image, const QColor& target, int tolerance = 28)
{
    int count = 0;
    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            if (nearColor(image.pixelColor(x, y), target, tolerance))
            {
                ++count;
            }
        }
    }
    return count;
}

qreal averageLuminance(const QImage& image)
{
    qreal total = 0;
    int count = 0;
    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            const QColor color = image.pixelColor(x, y);
            if (color.alpha() < 24)
            {
                continue;
            }
            total += 0.2126 * color.red() + 0.7152 * color.green() + 0.0722 * color.blue();
            ++count;
        }
    }
    return count > 0 ? total / count : 0;
}

QString countMessage(const char* name, int count, int expected)
{
    return QStringLiteral("%1 near-color pixels: %2, expected > %3")
        .arg(QString::fromLatin1(name))
        .arg(count)
        .arg(expected);
}

void assertNearColorPixels(const QImage& image, const QColor& color, int expected, const char* name, int tolerance = 28)
{
    const int pixels = countNearColor(image, color, tolerance);
    QVERIFY2(pixels > expected, qPrintable(countMessage(name, pixels, expected)));
}
} // namespace

void TestAntVisualRegression::primaryButtonKeepsTokenFill()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntButton primary(QStringLiteral("Submit"));
    primary.setButtonType(Ant::ButtonType::Primary);
    const QImage primaryImage = renderWidget(&primary, QSize(140, 40));
    const int primaryPixels = countNearColor(primaryImage, antTheme->tokens().colorPrimary);
    QVERIFY2(primaryPixels > 900, qPrintable(countMessage("primary button", primaryPixels, 900)));

    AntButton defaultButton(QStringLiteral("Submit"));
    const QImage defaultImage = renderWidget(&defaultButton, QSize(140, 40));
    const int defaultPrimaryPixels = countNearColor(defaultImage, antTheme->tokens().colorPrimary);
    QVERIFY2(defaultPrimaryPixels < primaryPixels / 4,
             qPrintable(QStringLiteral("default button primary pixels: %1, primary button pixels: %2")
                            .arg(defaultPrimaryPixels)
                            .arg(primaryPixels)));
}

void TestAntVisualRegression::alertSemanticBackgroundsStayVisible()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const auto& token = antTheme->tokens();

    auto assertAlertBackground = [](Ant::AlertType type, const QColor& color, const char* name) {
        AntAlert alert;
        alert.setTitle(QStringLiteral("Status"));
        alert.setDescription(QStringLiteral("Semantic background"));
        alert.setShowIcon(true);
        alert.setAlertType(type);
        const int pixels = countNearColor(renderWidget(&alert, QSize(280, 88)), color, 18);
        QVERIFY2(pixels > 3500, qPrintable(countMessage(name, pixels, 3500)));
    };

    assertAlertBackground(Ant::AlertType::Success, token.colorSuccessBg, "success alert");
    assertAlertBackground(Ant::AlertType::Warning, token.colorWarningBg, "warning alert");
    assertAlertBackground(Ant::AlertType::Error, token.colorErrorBg, "error alert");
}

void TestAntVisualRegression::progressStatusColorsStayVisible()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const auto& token = antTheme->tokens();

    AntProgress successProgress;
    successProgress.setShowInfo(false);
    successProgress.setPercent(72);
    successProgress.setStatus(Ant::ProgressStatus::Success);
    const int successPixels = countNearColor(renderWidget(&successProgress, QSize(240, 32)), token.colorSuccess);
    QVERIFY2(successPixels > 500, qPrintable(countMessage("success progress", successPixels, 500)));

    AntProgress exceptionProgress;
    exceptionProgress.setShowInfo(false);
    exceptionProgress.setPercent(72);
    exceptionProgress.setStatus(Ant::ProgressStatus::Exception);
    const int exceptionPixels = countNearColor(renderWidget(&exceptionProgress, QSize(240, 32)), token.colorError);
    QVERIFY2(exceptionPixels > 500, qPrintable(countMessage("exception progress", exceptionPixels, 500)));
}

void TestAntVisualRegression::navigationActiveIndicatorsStayPrimary()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const QColor primary = antTheme->tokens().colorPrimary;

    AntTabs tabs;
    tabs.addTab(new QWidget, QStringLiteral("one"), QStringLiteral("One"));
    tabs.addTab(new QWidget, QStringLiteral("two"), QStringLiteral("Two"));
    tabs.setActiveKey(QStringLiteral("two"));
    const int tabsPrimaryPixels = countNearColor(renderWidget(&tabs, QSize(320, 120)), primary);
    QVERIFY2(tabsPrimaryPixels > 80, qPrintable(countMessage("tabs active", tabsPrimaryPixels, 80)));

    AntNavItem navItem(QStringLiteral("Dashboard"));
    navItem.setActive(true);
    const int navPrimaryPixels = countNearColor(renderWidget(&navItem, QSize(220, 36)), primary);
    QVERIFY2(navPrimaryPixels > 80, qPrintable(countMessage("nav item active", navPrimaryPixels, 80)));
}

void TestAntVisualRegression::inputNumberHandlersStayVisible()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const auto& token = antTheme->tokens();

    AntInputNumber inputNumber;
    inputNumber.setValue(24);
    inputNumber.setControlsProgress(1.0);
    assertNearColorPixels(renderWidget(&inputNumber, QSize(220, 40)), token.colorTextTertiary, 20, "input number handler arrows", 36);
}

void TestAntVisualRegression::selectionControlsKeepPrimaryStateFills()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const QColor primary = antTheme->tokens().colorPrimary;

    AntCheckbox checkbox(QStringLiteral("Checked"));
    checkbox.setChecked(true);
    assertNearColorPixels(renderWidget(&checkbox, QSize(120, 32)), primary, 90, "checked checkbox");

    AntRadio radio(QStringLiteral("Selected"));
    radio.setChecked(true);
    assertNearColorPixels(renderWidget(&radio, QSize(120, 32)), primary, 60, "checked radio");

    AntSwitch switcher;
    switcher.setChecked(true);
    switcher.setHandleProgress(1.0);
    assertNearColorPixels(renderWidget(&switcher, QSize(72, 36)), primary, 420, "checked switch");

    AntTag checkableTag(QStringLiteral("Active"));
    checkableTag.setCheckable(true);
    checkableTag.setChecked(true);
    assertNearColorPixels(renderWidget(&checkableTag, QSize(92, 28)), primary, 1000, "checked tag");
}

void TestAntVisualRegression::tagAndBadgeStatusColorsStayVisible()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const auto& token = antTheme->tokens();

    AntTag successTag(QStringLiteral("Success"));
    successTag.setColor(QStringLiteral("success"));
    const QImage successTagImage = renderWidget(&successTag, QSize(96, 28));
    assertNearColorPixels(successTagImage, token.colorSuccessBg, 1000, "success tag bg", 18);
    assertNearColorPixels(successTagImage, token.colorSuccess, 60, "success tag text/border");

    AntTag errorTag(QStringLiteral("Error"));
    errorTag.setColor(QStringLiteral("error"));
    const QImage errorTagImage = renderWidget(&errorTag, QSize(80, 28));
    assertNearColorPixels(errorTagImage, token.colorErrorBg, 800, "error tag bg", 18);
    assertNearColorPixels(errorTagImage, token.colorError, 40, "error tag text/border");

    AntBadge countBadge(12);
    assertNearColorPixels(renderWidget(&countBadge, QSize(56, 32)), token.colorError, 120, "count badge fill");

    AntBadge successStatus;
    successStatus.setStatus(Ant::BadgeStatus::Success);
    successStatus.setText(QStringLiteral("Ready"));
    assertNearColorPixels(renderWidget(&successStatus, QSize(120, 32)), token.colorSuccess, 24, "success badge status");
}

void TestAntVisualRegression::feedbackSurfacesKeepElevatedTokenFill()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const auto& token = antTheme->tokens();

    AntMessage message;
    message.setText(QStringLiteral("Saved"));
    message.setMessageType(Ant::MessageType::Success);
    const QImage messageImage = renderWidget(&message, QSize(180, 52));
    assertNearColorPixels(messageImage, token.colorBgElevated, 3400, "message elevated surface", 12);
    assertNearColorPixels(messageImage, token.colorSuccess, 50, "message success icon");

    AntNotification notification;
    notification.setTitle(QStringLiteral("Notice"));
    notification.setDescription(QStringLiteral("Visual surface"));
    notification.setNotificationType(Ant::MessageType::Info);
    notification.setShowProgress(true);
    const QImage notificationImage = renderWidget(&notification, QSize(420, 140));
    assertNearColorPixels(notificationImage, token.colorBgElevated, 18000, "notification elevated surface", 12);
    assertNearColorPixels(notificationImage, token.colorPrimary, 80, "notification primary accent");
}

void TestAntVisualRegression::dataDisplayBordersAndSeparatorsStayVisible()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const auto& token = antTheme->tokens();

    AntCard card(QStringLiteral("Card Title"));
    card.setExtra(QStringLiteral("More"));
    card.bodyLayout()->addWidget(new AntButton(QStringLiteral("Action"), &card));
    const QImage cardImage = renderWidget(&card, QSize(320, 160));
    assertNearColorPixels(cardImage, token.colorBgContainer, 22000, "card container surface", 12);
    assertNearColorPixels(cardImage, token.colorBorderSecondary, 600, "card border and header separator", 18);

    AntList list;
    list.setBordered(true);
    list.setSplit(true);
    list.setHeaderWidget(new QLabel(QStringLiteral("Header")));
    list.setFooterWidget(new QLabel(QStringLiteral("Footer")));
    for (const QString& text : {QStringLiteral("First item"), QStringLiteral("Second item")})
    {
        auto* item = new AntListItem;
        item->setContentWidget(new QLabel(text));
        list.addItem(item);
    }
    const QImage listImage = renderWidget(&list, QSize(360, 190));
    assertNearColorPixels(listImage, token.colorBorder, 700, "list outer/header/footer border", 18);
    assertNearColorPixels(listImage, token.colorSplit, 280, "list item split lines", 18);

    AntTableColumn nameColumn;
    nameColumn.title = QStringLiteral("Name");
    nameColumn.dataIndex = QStringLiteral("name");
    nameColumn.key = QStringLiteral("name");
    nameColumn.width = 160;

    AntTableColumn roleColumn;
    roleColumn.title = QStringLiteral("Role");
    roleColumn.dataIndex = QStringLiteral("role");
    roleColumn.key = QStringLiteral("role");
    roleColumn.width = 160;

    AntTableRow selectedRow;
    selectedRow.data.insert(QStringLiteral("name"), QStringLiteral("Alice"));
    selectedRow.data.insert(QStringLiteral("role"), QStringLiteral("Designer"));
    selectedRow.selected = true;

    AntTableRow normalRow;
    normalRow.data.insert(QStringLiteral("name"), QStringLiteral("Bob"));
    normalRow.data.insert(QStringLiteral("role"), QStringLiteral("Engineer"));

    AntTable table;
    table.setRowSelection(Ant::TableSelectionMode::Checkbox);
    table.setPageSize(4);
    table.setColumns({nameColumn, roleColumn});
    table.setRows({selectedRow, normalRow});
    const QImage tableImage = renderWidget(&table, QSize(420, 180));
    assertNearColorPixels(tableImage, token.colorFillQuaternary, 9000, "table header and alternating fill", 18);
    assertNearColorPixels(tableImage, token.colorBorderSecondary, 1200, "table grid separators", 18);
    assertNearColorPixels(tableImage, token.colorPrimaryBg, 7000, "table selected row fill", 18);
    assertNearColorPixels(tableImage, token.colorPrimary, 120, "table selected checkbox fill");

    AntDescriptions descriptions;
    descriptions.setTitle(QStringLiteral("User Info"));
    descriptions.setBordered(true);
    descriptions.setColumnCount(2);
    descriptions.addItem(QStringLiteral("Name"), QStringLiteral("Alice"));
    descriptions.addItem(QStringLiteral("Role"), QStringLiteral("Designer"));
    const QImage descriptionsImage = renderWidget(&descriptions, QSize(420, 150));
    assertNearColorPixels(descriptionsImage, token.colorFillQuaternary, 7000, "descriptions label cells", 18);
    assertNearColorPixels(descriptionsImage, token.colorBgContainer, 12000, "descriptions content cells", 12);
    assertNearColorPixels(descriptionsImage, token.colorSplit, 1000, "descriptions cell borders", 18);
}

void TestAntVisualRegression::navigationAndLayoutStructureStayVisible()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const auto& token = antTheme->tokens();

    AntMenu menu;
    menu.setMode(Ant::MenuMode::Vertical);
    menu.addItem(QStringLiteral("home"), QStringLiteral("Home"), Ant::IconType::Home);
    menu.addItem(QStringLiteral("reports"), QStringLiteral("Reports"), Ant::IconType::Setting);
    menu.addDivider();
    menu.addItem(QStringLiteral("alerts"), QStringLiteral("Alerts"), Ant::IconType::Bell, QStringLiteral("8"));
    menu.setSelectedKey(QStringLiteral("reports"));
    const QImage menuImage = renderWidget(&menu, QSize(260, 160));
    assertNearColorPixels(menuImage, token.colorBgContainer, 26000, "menu container surface", 12);
    assertNearColorPixels(menuImage, token.colorPrimaryBg, 3500, "menu selected item fill", 18);
    assertNearColorPixels(menuImage, token.colorPrimary, 120, "menu selected item text and icon");
    assertNearColorPixels(menuImage, token.colorSplit, 120, "menu divider", 18);

    AntPagination pagination;
    pagination.setTotal(180);
    pagination.setCurrent(4);
    pagination.setShowTotal(true);
    pagination.setShowSizeChanger(true);
    pagination.setShowQuickJumper(true);
    const QImage paginationImage = renderWidget(&pagination, QSize(640, 48));
    assertNearColorPixels(paginationImage, token.colorBgContainer, 7000, "pagination item surfaces", 12);
    assertNearColorPixels(paginationImage, token.colorBorder, 1200, "pagination item borders", 18);
    assertNearColorPixels(paginationImage, token.colorPrimary, 80, "pagination active page");
    assertNearColorPixels(paginationImage, token.colorTextSecondary, 120, "pagination total text", 36);

    AntSteps steps;
    steps.addStep(QStringLiteral("Login"), QStringLiteral("Done"), QString(), Ant::StepStatus::Finish);
    steps.addStep(QStringLiteral("Profile"), QStringLiteral("In progress"));
    steps.addStep(QStringLiteral("Review"), QStringLiteral("Needs attention"), QString(), Ant::StepStatus::Error);
    steps.addStep(QStringLiteral("Done"), QStringLiteral("Waiting"));
    steps.setCurrentIndex(1);
    const QImage stepsImage = renderWidget(&steps, QSize(720, 104));
    assertNearColorPixels(stepsImage, token.colorPrimary, 700, "steps process and finish color", 28);
    assertNearColorPixels(stepsImage, token.colorPrimaryBg, 500, "steps finished icon fill", 18);
    assertNearColorPixels(stepsImage, token.colorError, 500, "steps error state", 28);
    assertNearColorPixels(stepsImage, token.colorSplit, 300, "steps wait connector", 28);

    AntLayout layout;
    layout.setBorderRadius(8);
    auto* sider = new AntLayoutSider;
    sider->setWidth(120);
    sider->setCollapsible(true);
    layout.addSider(sider);
    layout.setHeader(new AntLayoutHeader);
    layout.setContent(new AntLayoutContent);
    layout.setFooter(new AntLayoutFooter);
    const QImage layoutImage = renderWidget(&layout, QSize(480, 260));
    assertNearColorPixels(layoutImage, token.colorBgLayout, 56000, "layout background and footer", 12);
    assertNearColorPixels(layoutImage, token.colorBgContainer, 45000, "layout content surface", 12);
    assertNearColorPixels(layoutImage, QColor(0, 21, 41), 34000, "layout header and sider dark surface", 12);
}

void TestAntVisualRegression::complexPopupSurfacesStayElevated()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const auto& token = antTheme->tokens();

    AntMenu popupSource;
    popupSource.setMode(Ant::MenuMode::Vertical);
    popupSource.addSubMenu(QStringLiteral("products"), QStringLiteral("Products"), Ant::IconType::Setting);
    popupSource.addSubItem(QStringLiteral("products"), QStringLiteral("analytics"), QStringLiteral("Analytics"));
    popupSource.addSubItem(QStringLiteral("products"), QStringLiteral("billing"), QStringLiteral("Billing"));
    popupSource.resize(popupSource.sizeHint());
    popupSource.show();
    QVERIFY(QTest::qWaitForWindowExposed(&popupSource));
    QTest::mouseClick(&popupSource, Qt::LeftButton, Qt::NoModifier, QPoint(24, 20));

    AntMenu* popupMenu = nullptr;
    QTRY_VERIFY_WITH_TIMEOUT(([&]() {
        const auto menus = popupSource.findChildren<AntMenu*>();
        for (AntMenu* candidate : menus)
        {
            if (candidate && candidate->isVisible())
            {
                popupMenu = candidate;
                return true;
            }
        }
        return false;
    })(), 300);
    QWidget* popupPanel = popupMenu->parentWidget();
    QVERIFY(popupPanel != nullptr);
    QTest::qWait(180);
    const QImage submenuImage = renderCurrentWidget(popupPanel);
    assertNearColorPixels(submenuImage, token.colorBgElevated, 8500, "submenu popup elevated surface", 12);
    assertNearColorPixels(submenuImage, token.colorBorder, 50, "submenu popup border", 18);

    AntPopover popover;
    popover.setTitle(QStringLiteral("Popover Title"));
    popover.setTitleIconType(Ant::IconType::InfoCircle);
    popover.setContent(QStringLiteral("Elevated popover body"));
    popover.setPlacement(Ant::TooltipPlacement::Bottom);
    popover.setArrowVisible(true);
    const QImage popoverImage = renderWidget(&popover, QSize(280, 132));
    assertNearColorPixels(popoverImage, token.colorBgElevated, 19000, "popover elevated surface", 12);
    assertNearColorPixels(popoverImage, token.colorPrimary, 70, "popover title icon");

    QWidget modalHost;
    prepareHost(modalHost, QSize(640, 420), token.colorBgLayout);
    QVERIFY(QTest::qWaitForWindowExposed(&modalHost));
    AntModal modal(&modalHost);
    modal.setTitle(QStringLiteral("Modal Title"));
    modal.setContent(QStringLiteral("Dialog content"));
    modal.setCommandIconType(Ant::IconType::InfoCircle);
    modal.setOpen(true);
    QTest::qWait(280);
    const QImage modalImage = renderCurrentWidget(&modalHost);
    assertNearColorPixels(modalImage, token.colorBgElevated, 54000, "modal elevated dialog", 12);
    assertNearColorPixels(modalImage, token.colorPrimary, 950, "modal primary action and icon", 28);
    modal.setOpen(false);

    QWidget drawerHost;
    prepareHost(drawerHost, QSize(640, 420), token.colorBgLayout);
    QVERIFY(QTest::qWaitForWindowExposed(&drawerHost));
    AntDrawer drawer(&drawerHost);
    drawer.setTitle(QStringLiteral("Drawer Title"));
    drawer.setDrawerWidth(260);
    drawer.setPlacement(Ant::DrawerPlacement::Right);
    drawer.setBodyWidget(new QLabel(QStringLiteral("Drawer body")));
    drawer.setOpen(true);
    QTest::qWait(240);
    const QImage drawerImage = renderCurrentWidget(&drawerHost);
    assertNearColorPixels(drawerImage, token.colorBgContainer, 94000, "drawer panel surface", 12);
    assertNearColorPixels(drawerImage, token.colorBorderSecondary, 220, "drawer header divider", 18);
    drawer.setOpen(false);
}

void TestAntVisualRegression::lightAndDarkThemesRenderDifferentSurfaces()
{
    ThemeModeGuard guard;

    AntCard lightCard(QStringLiteral("Theme Surface"));
    lightCard.bodyLayout()->addWidget(new AntButton(QStringLiteral("Action"), &lightCard));
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    const QImage lightImage = renderWidget(&lightCard, QSize(280, 140));
    const qreal lightLuminance = averageLuminance(lightImage);

    AntCard darkCard(QStringLiteral("Theme Surface"));
    darkCard.bodyLayout()->addWidget(new AntButton(QStringLiteral("Action"), &darkCard));
    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    const QImage darkImage = renderWidget(&darkCard, QSize(280, 140));
    const qreal darkLuminance = averageLuminance(darkImage);

    QVERIFY2(std::abs(lightLuminance - darkLuminance) > 40.0,
             qPrintable(QStringLiteral("light luminance: %1, dark luminance: %2")
                            .arg(lightLuminance)
                            .arg(darkLuminance)));
}

QTEST_MAIN(TestAntVisualRegression)
#include "TestAntVisualRegression.moc"
