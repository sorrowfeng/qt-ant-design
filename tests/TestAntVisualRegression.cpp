#include <QColor>
#include <QImage>
#include <QPainter>
#include <QTest>
#include <QVBoxLayout>
#include <QWidget>

#include <cmath>

#include "core/AntTheme.h"
#include "widgets/AntAlert.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntProgress.h"
#include "widgets/AntTabs.h"

class TestAntVisualRegression : public QObject
{
    Q_OBJECT

private slots:
    void primaryButtonKeepsTokenFill();
    void alertSemanticBackgroundsStayVisible();
    void progressStatusColorsStayVisible();
    void navigationActiveIndicatorsStayPrimary();
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
