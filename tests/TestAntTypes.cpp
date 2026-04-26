#include <QMetaEnum>
#include <QTest>

#include "core/AntTypes.h"

class TestAntTypes : public QObject
{
    Q_OBJECT
private slots:
    void buttonTypeValues();
    void buttonSizeValues();
    void buttonShapeValues();
    void inputSizeValues();
    void inputStatusValues();
    void inputVariantValues();
    void switchSizeValues();
    void selectModeValues();
    void badgeModeValues();
    void skeletonElementValues();
    void messagePlacementValues();
    void typographyTypeValues();
    void enumRegistration();
};

void TestAntTypes::buttonTypeValues()
{
    QCOMPARE(static_cast<int>(Ant::ButtonType::Default), 0);
    QCOMPARE(static_cast<int>(Ant::ButtonType::Primary), 1);
    QCOMPARE(static_cast<int>(Ant::ButtonType::Dashed), 2);
    QCOMPARE(static_cast<int>(Ant::ButtonType::Text), 3);
    QCOMPARE(static_cast<int>(Ant::ButtonType::Link), 4);
}

void TestAntTypes::buttonSizeValues()
{
    QCOMPARE(static_cast<int>(Ant::ButtonSize::Large), 0);
    QCOMPARE(static_cast<int>(Ant::ButtonSize::Middle), 1);
    QCOMPARE(static_cast<int>(Ant::ButtonSize::Small), 2);
}

void TestAntTypes::buttonShapeValues()
{
    QCOMPARE(static_cast<int>(Ant::ButtonShape::Default), 0);
    QCOMPARE(static_cast<int>(Ant::ButtonShape::Circle), 1);
    QCOMPARE(static_cast<int>(Ant::ButtonShape::Round), 2);
}

void TestAntTypes::inputSizeValues()
{
    QCOMPARE(static_cast<int>(Ant::InputSize::Large), 0);
    QCOMPARE(static_cast<int>(Ant::InputSize::Middle), 1);
    QCOMPARE(static_cast<int>(Ant::InputSize::Small), 2);
}

void TestAntTypes::inputStatusValues()
{
    QCOMPARE(static_cast<int>(Ant::InputStatus::Normal), 0);
    QCOMPARE(static_cast<int>(Ant::InputStatus::Error), 1);
    QCOMPARE(static_cast<int>(Ant::InputStatus::Warning), 2);
}

void TestAntTypes::inputVariantValues()
{
    QCOMPARE(static_cast<int>(Ant::InputVariant::Outlined), 0);
    QCOMPARE(static_cast<int>(Ant::InputVariant::Borderless), 1);
    QCOMPARE(static_cast<int>(Ant::InputVariant::Filled), 2);
    QCOMPARE(static_cast<int>(Ant::InputVariant::Underlined), 3);
}

void TestAntTypes::switchSizeValues()
{
    QCOMPARE(static_cast<int>(Ant::SwitchSize::Middle), 0);
    QCOMPARE(static_cast<int>(Ant::SwitchSize::Small), 1);
}

void TestAntTypes::selectModeValues()
{
    QCOMPARE(static_cast<int>(Ant::SelectMode::Single), 0);
    QCOMPARE(static_cast<int>(Ant::SelectMode::Multiple), 1);
    QCOMPARE(static_cast<int>(Ant::SelectMode::Tags), 2);
}

void TestAntTypes::badgeModeValues()
{
    QCOMPARE(static_cast<int>(Ant::BadgeMode::Default), 0);
    QCOMPARE(static_cast<int>(Ant::BadgeMode::Dot), 1);
    QCOMPARE(static_cast<int>(Ant::BadgeMode::Ribbon), 2);
}

void TestAntTypes::skeletonElementValues()
{
    QCOMPARE(static_cast<int>(Ant::SkeletonElement::Default), 0);
    QCOMPARE(static_cast<int>(Ant::SkeletonElement::Button), 1);
    QCOMPARE(static_cast<int>(Ant::SkeletonElement::Avatar), 2);
    QCOMPARE(static_cast<int>(Ant::SkeletonElement::Input), 3);
    QCOMPARE(static_cast<int>(Ant::SkeletonElement::Image), 4);
    QCOMPARE(static_cast<int>(Ant::SkeletonElement::Node), 5);
}

void TestAntTypes::messagePlacementValues()
{
    QCOMPARE(static_cast<int>(Ant::MessagePlacement::Top), 0);
    QCOMPARE(static_cast<int>(Ant::MessagePlacement::TopLeft), 1);
    QCOMPARE(static_cast<int>(Ant::MessagePlacement::TopRight), 2);
    QCOMPARE(static_cast<int>(Ant::MessagePlacement::Bottom), 3);
    QCOMPARE(static_cast<int>(Ant::MessagePlacement::BottomLeft), 4);
    QCOMPARE(static_cast<int>(Ant::MessagePlacement::BottomRight), 5);
}

void TestAntTypes::typographyTypeValues()
{
    QCOMPARE(static_cast<int>(Ant::TypographyType::Default), 0);
    QCOMPARE(static_cast<int>(Ant::TypographyType::Secondary), 1);
    QCOMPARE(static_cast<int>(Ant::TypographyType::Success), 2);
    QCOMPARE(static_cast<int>(Ant::TypographyType::Warning), 3);
    QCOMPARE(static_cast<int>(Ant::TypographyType::Danger), 4);
    QCOMPARE(static_cast<int>(Ant::TypographyType::LightSolid), 5);
    QCOMPARE(static_cast<int>(Ant::TypographyType::Link), 6);
}

void TestAntTypes::enumRegistration()
{
    QMetaEnum me;

    me = QMetaEnum::fromType<Ant::ButtonType>();
    QVERIFY(me.isValid());
    QCOMPARE(me.keyCount(), 5);

    me = QMetaEnum::fromType<Ant::ButtonSize>();
    QVERIFY(me.isValid());
    QCOMPARE(me.keyCount(), 3);

    me = QMetaEnum::fromType<Ant::SelectMode>();
    QVERIFY(me.isValid());
    QCOMPARE(me.keyCount(), 3);

    me = QMetaEnum::fromType<Ant::BadgeMode>();
    QVERIFY(me.isValid());
    QCOMPARE(me.keyCount(), 3);

    me = QMetaEnum::fromType<Ant::SkeletonElement>();
    QVERIFY(me.isValid());
    QCOMPARE(me.keyCount(), 6);

    me = QMetaEnum::fromType<Ant::MessagePlacement>();
    QVERIFY(me.isValid());
    QCOMPARE(me.keyCount(), 6);
}

QTEST_MAIN(TestAntTypes)
#include "TestAntTypes.moc"
