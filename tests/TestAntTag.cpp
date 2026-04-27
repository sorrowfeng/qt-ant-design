#include <QSignalSpy>
#include <QTest>

#include "widgets/AntTag.h"

class TestAntTag : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntTag::propertiesAndSignals()
{
    auto* tag = new AntTag;
    QCOMPARE(tag->text(), QString());
    QCOMPARE(tag->color(), QString());
    QCOMPARE(tag->iconText(), QString());
    QCOMPARE(tag->isClosable(), false);
    QCOMPARE(tag->isCheckable(), false);
    QCOMPARE(tag->isChecked(), false);
    QCOMPARE(tag->variant(), Ant::TagVariant::Filled);

    QSignalSpy textSpy(tag, &AntTag::textChanged);
    tag->setText("Test");
    QCOMPARE(tag->text(), "Test");
    QCOMPARE(textSpy.count(), 1);
    QCOMPARE(textSpy.at(0).at(0).toString(), "Test");

    QSignalSpy colorSpy(tag, &AntTag::colorChanged);
    tag->setColor("blue");
    QCOMPARE(tag->color(), "blue");
    QCOMPARE(colorSpy.count(), 1);
    QCOMPARE(colorSpy.at(0).at(0).toString(), "blue");

    QSignalSpy iconSpy(tag, &AntTag::iconTextChanged);
    tag->setIconText("i");
    QCOMPARE(tag->iconText(), "i");
    QCOMPARE(iconSpy.count(), 1);

    QSignalSpy closableSpy(tag, &AntTag::closableChanged);
    tag->setClosable(true);
    QCOMPARE(tag->isClosable(), true);
    QCOMPARE(closableSpy.count(), 1);

    QSignalSpy checkableSpy(tag, &AntTag::checkableChanged);
    tag->setCheckable(true);
    QCOMPARE(tag->isCheckable(), true);
    QCOMPARE(checkableSpy.count(), 1);

    QSignalSpy checkedSpy(tag, &AntTag::checkedChanged);
    tag->setChecked(true);
    QCOMPARE(tag->isChecked(), true);
    QCOMPARE(checkedSpy.count(), 1);
    QCOMPARE(checkedSpy.at(0).at(0).toBool(), true);

    QSignalSpy variantSpy(tag, &AntTag::variantChanged);
    tag->setVariant(Ant::TagVariant::Outlined);
    QCOMPARE(tag->variant(), Ant::TagVariant::Outlined);
    QCOMPARE(variantSpy.count(), 1);

    QSize hint = tag->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);

    auto* textTag = new AntTag("My Tag");
    QCOMPARE(textTag->text(), "My Tag");
}

QTEST_MAIN(TestAntTag)
#include "TestAntTag.moc"
