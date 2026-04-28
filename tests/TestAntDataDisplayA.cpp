#include <QSignalSpy>
#include <QTest>
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
    QCOMPARE(valSpy.count(), 1);

    QSignalSpy precSpy(s, &AntStatistic::precisionChanged);
    s->setPrecision(2);
    QCOMPARE(s->precision(), 2);
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
    QCOMPARE(loadedImg->sizeHint(), QSize(100, 98));

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

QTEST_MAIN(TestAntDataDisplayA)
#include "TestAntDataDisplayA.moc"
