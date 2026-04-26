#include <QSignalSpy>
#include <QTest>

#include "widgets/AntTag.h"

class TestAntTag : public QObject
{
    Q_OBJECT
private slots:
    void defaultValues();
    void setText();
    void setColor();
    void setIconText();
    void setClosable();
    void setCheckable();
    void setChecked();
    void setVariant();
    void signalsEmitted();
    void sizeHint();
    void textConstructor();
};

void TestAntTag::defaultValues()
{
    AntTag tag;
    QCOMPARE(tag.text(), QString());
    QCOMPARE(tag.color(), QString());
    QCOMPARE(tag.iconText(), QString());
    QCOMPARE(tag.isClosable(), false);
    QCOMPARE(tag.isCheckable(), false);
    QCOMPARE(tag.isChecked(), false);
    QCOMPARE(tag.variant(), Ant::TagVariant::Filled);
}

void TestAntTag::setText()
{
    AntTag tag;
    tag.setText("Hello");
    QCOMPARE(tag.text(), "Hello");
}

void TestAntTag::setColor()
{
    AntTag tag;
    tag.setColor("red");
    QCOMPARE(tag.color(), "red");
    tag.setColor("#1890ff");
    QCOMPARE(tag.color(), "#1890ff");
}

void TestAntTag::setIconText()
{
    AntTag tag;
    tag.setIconText("icon");
    QCOMPARE(tag.iconText(), "icon");
}

void TestAntTag::setClosable()
{
    AntTag tag;
    tag.setClosable(true);
    QCOMPARE(tag.isClosable(), true);
    tag.setClosable(false);
    QCOMPARE(tag.isClosable(), false);
}

void TestAntTag::setCheckable()
{
    AntTag tag;
    tag.setCheckable(true);
    QCOMPARE(tag.isCheckable(), true);
    tag.setCheckable(false);
    QCOMPARE(tag.isCheckable(), false);
}

void TestAntTag::setChecked()
{
    AntTag tag;
    tag.setChecked(true);
    QCOMPARE(tag.isChecked(), true);
    tag.setChecked(false);
    QCOMPARE(tag.isChecked(), false);
}

void TestAntTag::setVariant()
{
    AntTag tag;
    tag.setVariant(Ant::TagVariant::Outlined);
    QCOMPARE(tag.variant(), Ant::TagVariant::Outlined);
    tag.setVariant(Ant::TagVariant::Solid);
    QCOMPARE(tag.variant(), Ant::TagVariant::Solid);
}

void TestAntTag::signalsEmitted()
{
    AntTag tag;

    QSignalSpy textSpy(&tag, &AntTag::textChanged);
    tag.setText("Test");
    QCOMPARE(textSpy.count(), 1);
    QCOMPARE(textSpy.at(0).at(0).toString(), "Test");

    QSignalSpy colorSpy(&tag, &AntTag::colorChanged);
    tag.setColor("blue");
    QCOMPARE(colorSpy.count(), 1);
    QCOMPARE(colorSpy.at(0).at(0).toString(), "blue");

    QSignalSpy iconSpy(&tag, &AntTag::iconTextChanged);
    tag.setIconText("i");
    QCOMPARE(iconSpy.count(), 1);

    QSignalSpy closableSpy(&tag, &AntTag::closableChanged);
    tag.setClosable(true);
    QCOMPARE(closableSpy.count(), 1);

    QSignalSpy checkableSpy(&tag, &AntTag::checkableChanged);
    tag.setCheckable(true);
    QCOMPARE(checkableSpy.count(), 1);

    QSignalSpy checkedSpy(&tag, &AntTag::checkedChanged);
    tag.setChecked(true);
    QCOMPARE(checkedSpy.count(), 1);
    QCOMPARE(checkedSpy.at(0).at(0).toBool(), true);

    QSignalSpy variantSpy(&tag, &AntTag::variantChanged);
    tag.setVariant(Ant::TagVariant::Outlined);
    QCOMPARE(variantSpy.count(), 1);
}

void TestAntTag::sizeHint()
{
    AntTag tag;
    QSize hint = tag.sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);
}

void TestAntTag::textConstructor()
{
    AntTag tag("My Tag");
    QCOMPARE(tag.text(), "My Tag");
}

QTEST_MAIN(TestAntTag)
#include "TestAntTag.moc"
