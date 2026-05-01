#include <QColor>
#include <QImage>
#include <QPainter>
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
#include "widgets/AntInputNumber.h"
#include "widgets/AntMessage.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntNotification.h"
#include "widgets/AntProgress.h"
#include "widgets/AntRadio.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTabs.h"
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
