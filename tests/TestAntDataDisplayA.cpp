#include <QSignalSpy>
#include <QTest>
#include <QDateTime>
#include <QLabel>
#include <QRegularExpression>
#include "core/AntTheme.h"
#include "widgets/AntAvatar.h"
#include "widgets/AntCard.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntCalendar.h"
#include "widgets/AntImage.h"
#include "widgets/AntEmpty.h"

class TestAntDataDisplayA : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void avatarCachesScaledImagePixmap();
    void calendarCachesSelectionAndViewMetrics();
    void cardCachesPaintAndSpinnerUpdates();
    void statisticFormattedValueCache();
    void cardTitlePaletteTracksTheme();
};

void TestAntDataDisplayA::propertiesAndSignals()
{
    // AntAvatar
    auto* a = new AntAvatar;
    QCOMPARE(a->text(), QString());
    QCOMPARE(a->iconText(), QString());
    QCOMPARE(a->imagePath(), QString());
    QCOMPARE(a->backgroundColor(), QColor());
    QCOMPARE(a->gap(), 4);
    QCOMPARE(a->customSize(), 0);
    QCOMPARE(a->shape(), Ant::AvatarShape::Circle);
    QCOMPARE(a->avatarSize(), Ant::Size::Middle);

    QSignalSpy textSpy(a, &AntAvatar::textChanged);
    a->setText("AB");
    QCOMPARE(a->text(), "AB");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy iconSpy(a, &AntAvatar::iconTextChanged);
    a->setIconText("user");
    QCOMPARE(a->iconText(), "user");
    QCOMPARE(iconSpy.count(), 1);

    QSignalSpy shapeSpy(a, &AntAvatar::shapeChanged);
    a->setShape(Ant::AvatarShape::Square);
    QCOMPARE(a->shape(), Ant::AvatarShape::Square);
    QCOMPARE(shapeSpy.count(), 1);

    QSignalSpy bgSpy(a, &AntAvatar::backgroundColorChanged);
    a->setBackgroundColor(QColor("#1677ff"));
    QCOMPARE(a->backgroundColor(), QColor("#1677ff"));
    QCOMPARE(bgSpy.count(), 1);

    QSignalSpy sizeSpy(a, &AntAvatar::avatarSizeChanged);
    a->setAvatarSize(Ant::Size::Large);
    QCOMPARE(a->avatarSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy gapSpy(a, &AntAvatar::gapChanged);
    a->setGap(8);
    QCOMPARE(a->gap(), 8);
    QCOMPARE(gapSpy.count(), 1);

    QSignalSpy imageSpy(a, &AntAvatar::imagePathChanged);
    a->setImagePath(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
    QCOMPARE(a->imagePath(), QStringLiteral(":/qt-ant-design/images/image-basic.png"));
    QCOMPARE(imageSpy.count(), 1);

    auto* a2 = new AntAvatar("CD");
    QCOMPARE(a2->text(), "CD");

    // AntAvatarGroup
    auto* g = new AntAvatarGroup;
    QCOMPARE(g->maxCount(), 0);
    QCOMPARE(g->avatarSize(), Ant::Size::Middle);

    QSignalSpy gMaxSpy(g, &AntAvatarGroup::maxCountChanged);
    g->setMaxCount(3);
    QCOMPARE(g->maxCount(), 3);
    QCOMPARE(gMaxSpy.count(), 1);

    QSignalSpy gSizeSpy(g, &AntAvatarGroup::avatarSizeChanged);
    g->setAvatarSize(Ant::Size::Small);
    QCOMPARE(g->avatarSize(), Ant::Size::Small);
    QCOMPARE(gSizeSpy.count(), 1);

    auto* av1 = new AntAvatar("A");
    auto* av2 = new AntAvatar("B");
    g->addAvatar(av1);
    g->addAvatar(av2);
    QCOMPARE(g->avatars().size(), 2);
    g->removeAvatar(av1);
    QCOMPARE(g->avatars().size(), 1);

    // AntCard
    auto* c = new AntCard;
    QCOMPARE(c->title(), QString());
    QCOMPARE(c->extra(), QString());
    QCOMPARE(c->isBordered(), true);
    QCOMPARE(c->isHoverable(), false);
    QCOMPARE(c->isLoading(), false);
    QCOMPARE(c->cardSize(), Ant::CardSize::Default);

    QSignalSpy titleSpy(c, &AntCard::titleChanged);
    c->setTitle("Card Title");
    QCOMPARE(c->title(), "Card Title");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy extraSpy(c, &AntCard::extraChanged);
    c->setExtra("More");
    QCOMPARE(c->extra(), "More");
    QCOMPARE(extraSpy.count(), 1);

    QSignalSpy borderSpy(c, &AntCard::borderedChanged);
    c->setBordered(false);
    QCOMPARE(c->isBordered(), false);
    QCOMPARE(borderSpy.count(), 1);

    QSignalSpy hoverSpy(c, &AntCard::hoverableChanged);
    c->setHoverable(true);
    QCOMPARE(c->isHoverable(), true);
    QCOMPARE(hoverSpy.count(), 1);

    QSignalSpy loadSpy(c, &AntCard::loadingChanged);
    c->setLoading(true);
    QCOMPARE(c->isLoading(), true);
    QCOMPARE(loadSpy.count(), 1);

    QSignalSpy cardSizeSpy(c, &AntCard::cardSizeChanged);
    c->setCardSize(Ant::CardSize::Small);
    QCOMPARE(c->cardSize(), Ant::CardSize::Small);
    QCOMPARE(cardSizeSpy.count(), 1);

    auto* c2 = new AntCard("Title");
    QCOMPARE(c2->title(), "Title");

    // AntStatistic
    auto* s = new AntStatistic;
    QCOMPARE(s->title(), QString());
    QCOMPARE(s->value(), 0.0);
    QCOMPARE(s->precision(), 0);
    QCOMPARE(s->prefix(), QString());
    QCOMPARE(s->suffix(), QString());
    QCOMPARE(s->isCountdownMode(), false);

    QSignalSpy stSpy(s, &AntStatistic::titleChanged);
    s->setTitle("Revenue");
    QCOMPARE(s->title(), "Revenue");
    QCOMPARE(stSpy.count(), 1);

    QSignalSpy valSpy(s, &AntStatistic::valueChanged);
    s->setValue(1234.56);
    QCOMPARE(s->value(), 1234.56);
    QCOMPARE(s->formattedValue(), QStringLiteral("1,234"));
    QCOMPARE(valSpy.count(), 1);

    QSignalSpy precSpy(s, &AntStatistic::precisionChanged);
    s->setPrecision(2);
    QCOMPARE(s->precision(), 2);
    QCOMPARE(s->formattedValue(), QStringLiteral("1,234.56"));
    QCOMPARE(precSpy.count(), 1);

    QSignalSpy prefixSpy(s, &AntStatistic::prefixChanged);
    s->setPrefix("$");
    QCOMPARE(s->prefix(), "$");
    QCOMPARE(prefixSpy.count(), 1);

    QSignalSpy suffixSpy(s, &AntStatistic::suffixChanged);
    s->setSuffix("USD");
    QCOMPARE(s->suffix(), "USD");
    QCOMPARE(suffixSpy.count(), 1);

    QSignalSpy cdSpy(s, &AntStatistic::countdownModeChanged);
    s->setCountdownMode(true);
    QCOMPARE(s->isCountdownMode(), true);
    QCOMPARE(cdSpy.count(), 1);

    QSignalSpy fmtSpy(s, &AntStatistic::countdownFormatChanged);
    s->setCountdownFormat("mm:ss");
    QCOMPARE(s->countdownFormat(), "mm:ss");
    QCOMPARE(fmtSpy.count(), 1);

    auto* s2 = new AntStatistic("Sales");
    QCOMPARE(s2->title(), "Sales");

    // AntCalendar
    auto* cal = new AntCalendar;
    QCOMPARE(cal->calendarMode(), Ant::CalendarMode::Day);
    QCOMPARE(cal->minimumDate(), QDate(1924, 1, 1));
    QCOMPARE(cal->maximumDate(), QDate(2124, 12, 31));

    QSignalSpy dateSpy(cal, &AntCalendar::selectedDateChanged);
    QDate testDate = QDate::currentDate().addDays(10);
    cal->setSelectedDate(testDate);
    QCOMPARE(cal->selectedDate(), testDate);
    QCOMPARE(dateSpy.count(), 1);

    QSignalSpy modeSpy(cal, &AntCalendar::calendarModeChanged);
    cal->setCalendarMode(Ant::CalendarMode::Month);
    QCOMPARE(cal->calendarMode(), Ant::CalendarMode::Month);
    QCOMPARE(modeSpy.count(), 1);

    QSignalSpy minSpy(cal, &AntCalendar::minimumDateChanged);
    cal->setMinimumDate(QDate(2020, 1, 1));
    QCOMPARE(cal->minimumDate(), QDate(2020, 1, 1));
    QCOMPARE(minSpy.count(), 1);

    QSignalSpy calMaxSpy(cal, &AntCalendar::maximumDateChanged);
    cal->setMaximumDate(QDate(2030, 12, 31));
    QCOMPARE(cal->maximumDate(), QDate(2030, 12, 31));
    QCOMPARE(calMaxSpy.count(), 1);

    // AntImage
    auto* img = new AntImage;
    QCOMPARE(img->src(), QString());
    QCOMPARE(img->alt(), QStringLiteral("Image"));
    QCOMPARE(img->preview(), true);
    QCOMPARE(img->imgWidth(), 0);
    QCOMPARE(img->imgHeight(), 0);

    QSignalSpy srcSpy(img, &AntImage::srcChanged);
    img->setSrc("/path/to/image.png");
    QCOMPARE(img->src(), "/path/to/image.png");
    QCOMPARE(srcSpy.count(), 1);

    QSignalSpy altSpy(img, &AntImage::altChanged);
    img->setAlt("Photo");
    QCOMPARE(img->alt(), "Photo");
    QCOMPARE(altSpy.count(), 1);

    QSignalSpy previewSpy(img, &AntImage::previewChanged);
    img->setPreview(false);
    QCOMPARE(img->preview(), false);
    QCOMPARE(previewSpy.count(), 1);

    QSignalSpy imgWSpy(img, &AntImage::imgWidthChanged);
    img->setImgWidth(200);
    QCOMPARE(img->imgWidth(), 200);
    QCOMPARE(imgWSpy.count(), 1);

    QSignalSpy imgHSpy(img, &AntImage::imgHeightChanged);
    img->setImgHeight(150);
    QCOMPARE(img->imgHeight(), 150);
    QCOMPARE(imgHSpy.count(), 1);

    auto* loadedImg = new AntImage;
    loadedImg->setSrc(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
    loadedImg->setImgWidth(100);
    QCOMPARE(loadedImg->sizeHint(), QSize(100, 100));

    // AntEmpty
    auto* e = new AntEmpty;
    QCOMPARE(e->description(), QStringLiteral("No data"));
    QCOMPARE(e->imageVisible(), true);
    QCOMPARE(e->isSimple(), false);

    QSignalSpy descSpy(e, &AntEmpty::descriptionChanged);
    e->setDescription("No results");
    QCOMPARE(e->description(), "No results");
    QCOMPARE(descSpy.count(), 1);

    QSignalSpy visSpy(e, &AntEmpty::imageVisibleChanged);
    e->setImageVisible(false);
    QCOMPARE(e->imageVisible(), false);
    QCOMPARE(visSpy.count(), 1);

    QSignalSpy simpleSpy(e, &AntEmpty::simpleChanged);
    e->setSimple(true);
    QCOMPARE(e->isSimple(), true);
    QCOMPARE(simpleSpy.count(), 1);

    QSignalSpy eSizeSpy(e, &AntEmpty::imageSizeChanged);
    e->setImageSize(QSize(100, 100));
    QCOMPARE(e->imageSize(), QSize(100, 100));
    QCOMPARE(eSizeSpy.count(), 1);
}

void TestAntDataDisplayA::avatarCachesScaledImagePixmap()
{
    const Ant::ThemeMode previousMode = antTheme->themeMode();

    AntAvatar avatar;
    avatar.setCustomSize(64);
    avatar.resize(64, 64);
    avatar.setImagePath(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
    QCOMPARE(avatar.property("antAvatarLastUpdateMode").toString(), QStringLiteral("image"));
    avatar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&avatar));

    avatar.grab();
    const int imageBuilds = avatar.property("antAvatarImagePixmapBuildCount").toInt();
    const int imageHits = avatar.property("antAvatarImagePixmapCacheHitCount").toInt();
    QVERIFY(imageBuilds > 0);

    avatar.grab();
    QCOMPARE(avatar.property("antAvatarImagePixmapBuildCount").toInt(), imageBuilds);
    QVERIFY(avatar.property("antAvatarImagePixmapCacheHitCount").toInt() > imageHits);

    const int buildsBeforeShape = avatar.property("antAvatarImagePixmapBuildCount").toInt();
    avatar.setShape(Ant::AvatarShape::Square);
    QCOMPARE(avatar.property("antAvatarLastUpdateMode").toString(), QStringLiteral("shape"));
    avatar.grab();
    QVERIFY(avatar.property("antAvatarImagePixmapBuildCount").toInt() > buildsBeforeShape);

    const int buildsBeforeSize = avatar.property("antAvatarImagePixmapBuildCount").toInt();
    avatar.setCustomSize(72);
    avatar.resize(72, 72);
    QCOMPARE(avatar.property("antAvatarLastUpdateMode").toString(), QStringLiteral("size"));
    avatar.grab();
    QVERIFY(avatar.property("antAvatarImagePixmapBuildCount").toInt() > buildsBeforeSize);

    const int buildsBeforeTheme = avatar.property("antAvatarImagePixmapBuildCount").toInt();
    antTheme->setThemeMode(previousMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);
    QCOMPARE(avatar.property("antAvatarLastUpdateMode").toString(), QStringLiteral("theme"));
    avatar.grab();
    QVERIFY(avatar.property("antAvatarImagePixmapBuildCount").toInt() > buildsBeforeTheme);

    antTheme->setThemeMode(previousMode);
}

void TestAntDataDisplayA::calendarCachesSelectionAndViewMetrics()
{
    const Ant::ThemeMode previousMode = antTheme->themeMode();
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntCalendar calendar;
    calendar.resize(360, 360);
    calendar.show();
    QVERIFY(QTest::qWaitForWindowExposed(&calendar));

    const int initialModelBuilds = calendar.property("antCalendarModelBuildCount").toInt();
    const int initialMetricBuilds = calendar.property("antCalendarViewMetricsBuildCount").toInt();
    QVERIFY(initialModelBuilds > 0);
    QVERIFY(initialMetricBuilds > 0);

    const QDate current = QDate::currentDate();
    const int firstDay = current.day() == 10 ? 11 : 10;
    const int secondDay = firstDay == 10 ? 11 : 10;
    const QDate firstSelection(current.year(), current.month(), firstDay);
    const QDate secondSelection(current.year(), current.month(), secondDay);

    const int selectionUpdatesBefore = calendar.property("antCalendarSelectionRegionUpdateCount").toInt();
    calendar.setSelectedDate(firstSelection);
    QCOMPARE(calendar.property("antCalendarModelBuildCount").toInt(), initialModelBuilds);
    QVERIFY(calendar.property("antCalendarSelectionRegionUpdateCount").toInt() > selectionUpdatesBefore);
    QCOMPARE(calendar.property("antCalendarLastUpdateMode").toString(), QStringLiteral("selection"));

    const int buildsBeforeSameMonth = calendar.property("antCalendarModelBuildCount").toInt();
    calendar.setSelectedDate(secondSelection);
    QCOMPARE(calendar.property("antCalendarModelBuildCount").toInt(), buildsBeforeSameMonth);

    const int metricHitsBeforeResize = calendar.property("antCalendarViewMetricsCacheHitCount").toInt();
    calendar.resize(calendar.width() + 12, calendar.height());
    QCoreApplication::processEvents();
    QVERIFY(calendar.property("antCalendarViewMetricsCacheHitCount").toInt() > metricHitsBeforeResize);

    const int buildsBeforeNextMonth = calendar.property("antCalendarModelBuildCount").toInt();
    const QDate nextMonth = secondSelection.addMonths(1);
    calendar.setSelectedDate(QDate(nextMonth.year(), nextMonth.month(), 1));
    QVERIFY(calendar.property("antCalendarModelBuildCount").toInt() > buildsBeforeNextMonth);
    QCOMPARE(calendar.property("antCalendarLastUpdateMode").toString(), QStringLiteral("model"));

    const int buildsBeforeMode = calendar.property("antCalendarModelBuildCount").toInt();
    calendar.setCalendarMode(Ant::CalendarMode::Month);
    QVERIFY(calendar.property("antCalendarModelBuildCount").toInt() > buildsBeforeMode);
    QCOMPARE(calendar.calendarMode(), Ant::CalendarMode::Month);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(calendar.property("antCalendarLastUpdateMode").toString(), QStringLiteral("theme"));
    antTheme->setThemeMode(previousMode);
}

void TestAntDataDisplayA::cardCachesPaintAndSpinnerUpdates()
{
    AntCard card(QStringLiteral("Cache Card"));
    card.setExtra(QStringLiteral("More"));
    card.addActionWidget(new QLabel(QStringLiteral("Edit")));
    card.addActionWidget(new QLabel(QStringLiteral("Share")));
    card.resize(320, 180);
    card.show();
    QVERIFY(QTest::qWaitForWindowExposed(&card));

    card.grab();
    const int paintBuilds = card.property("antCardPaintCacheBuildCount").toInt();
    const int paintHits = card.property("antCardPaintCacheHitCount").toInt();
    QVERIFY(paintBuilds > 0);

    card.grab();
    QCOMPARE(card.property("antCardPaintCacheBuildCount").toInt(), paintBuilds);
    QVERIFY(card.property("antCardPaintCacheHitCount").toInt() > paintHits);

    card.setLoading(true);
    QVERIFY(card.property("antCardSpinnerTimerActive").toBool());
    const int spinnerUpdates = card.property("antCardSpinnerRegionUpdateCount").toInt();
    QTRY_VERIFY(card.property("antCardSpinnerRegionUpdateCount").toInt() > spinnerUpdates);
    QCOMPARE(card.property("antCardLastUpdateMode").toString(), QStringLiteral("spinner"));

    card.hide();
    QCoreApplication::processEvents();
    QVERIFY(!card.property("antCardSpinnerTimerActive").toBool());
    const int stoppedUpdates = card.property("antCardSpinnerRegionUpdateCount").toInt();
    QTest::qWait(120);
    QCOMPARE(card.property("antCardSpinnerRegionUpdateCount").toInt(), stoppedUpdates);

    card.show();
    QVERIFY(QTest::qWaitForWindowExposed(&card));
    QTRY_VERIFY(card.property("antCardSpinnerTimerActive").toBool());
    card.setLoading(false);
    QVERIFY(!card.property("antCardSpinnerTimerActive").toBool());
    QCOMPARE(card.property("antCardLastUpdateMode").toString(), QStringLiteral("loading"));

    const int buildsBeforeSize = card.property("antCardPaintCacheBuildCount").toInt();
    card.setCardSize(Ant::CardSize::Small);
    card.grab();
    QVERIFY(card.property("antCardPaintCacheBuildCount").toInt() > buildsBeforeSize);
}

void TestAntDataDisplayA::statisticFormattedValueCache()
{
    AntStatistic statistic;
    QCOMPARE(statistic.formattedValue(), QStringLiteral("0"));

    statistic.setValue(1234567.89);
    QCOMPARE(statistic.formattedValue(), QStringLiteral("1,234,567"));
    const QString roundedValue = statistic.formattedValue();
    QCOMPARE(statistic.formattedValue(), roundedValue);

    statistic.setPrecision(2);
    QCOMPARE(statistic.formattedValue(), QStringLiteral("1,234,567.89"));

    statistic.setGroupSeparator(QString());
    QCOMPARE(statistic.formattedValue(), QStringLiteral("1234567.89"));

    statistic.setGroupSeparator(QStringLiteral(" "));
    QCOMPARE(statistic.formattedValue(), QStringLiteral("1 234 567.89"));

    statistic.setValue(QDateTime::currentMSecsSinceEpoch() / 1000.0 + 65.0);
    statistic.setCountdownMode(true);
    statistic.setCountdownFormat(QStringLiteral("mm:ss"));
    const QRegularExpression countdownPattern(QStringLiteral("^01:0[0-5]$"));
    QVERIFY2(countdownPattern.match(statistic.formattedValue()).hasMatch(),
             qPrintable(QStringLiteral("unexpected countdown value: %1").arg(statistic.formattedValue())));

    statistic.setCountdownMode(false);
    QVERIFY(!statistic.formattedValue().contains(QLatin1Char(':')));
}

void TestAntDataDisplayA::cardTitlePaletteTracksTheme()
{
    const Ant::ThemeMode previousMode = antTheme->themeMode();
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntCard card(QStringLiteral("Card Title"));
    card.setMetaTitle(QStringLiteral("Meta Title"));

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCoreApplication::processEvents();

    const auto labels = card.findChildren<QLabel*>();
    QLabel* titleLabel = nullptr;
    QLabel* metaTitleLabel = nullptr;
    for (QLabel* label : labels)
    {
        if (label->text() == QStringLiteral("Card Title"))
        {
            titleLabel = label;
        }
        else if (label->text() == QStringLiteral("Meta Title"))
        {
            metaTitleLabel = label;
        }
    }

    QVERIFY(titleLabel != nullptr);
    QVERIFY(metaTitleLabel != nullptr);
    QCOMPARE(titleLabel->palette().color(QPalette::WindowText), antTheme->tokens().colorText);
    QCOMPARE(metaTitleLabel->palette().color(QPalette::WindowText), antTheme->tokens().colorText);

    antTheme->setThemeMode(previousMode);
}

QTEST_MAIN(TestAntDataDisplayA)
#include "TestAntDataDisplayA.moc"
