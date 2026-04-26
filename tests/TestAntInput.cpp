#include <QSignalSpy>
#include <QTest>

#include "widgets/AntInput.h"

class TestAntInput : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
};

void TestAntInput::propertiesAndSignals()
{
    auto* input = new AntInput;
    QCOMPARE(input->text(), QString());
    QCOMPARE(input->inputSize(), Ant::InputSize::Middle);
    QCOMPARE(input->status(), Ant::InputStatus::Normal);
    QCOMPARE(input->variant(), Ant::InputVariant::Outlined);
    QCOMPARE(input->allowClear(), false);
    QCOMPARE(input->isPasswordMode(), false);
    QCOMPARE(input->isSearchMode(), false);

    QSignalSpy textSpy(input, &AntInput::textChanged);
    input->setText("hello");
    QCOMPARE(input->text(), "hello");
    QCOMPARE(textSpy.count(), 1);

    input->setPlaceholderText("Enter text");
    QCOMPARE(input->lineEdit()->placeholderText(), "Enter text");

    QSignalSpy sizeSpy(input, &AntInput::inputSizeChanged);
    input->setInputSize(Ant::InputSize::Large);
    QCOMPARE(input->inputSize(), Ant::InputSize::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy statusSpy(input, &AntInput::statusChanged);
    input->setStatus(Ant::InputStatus::Error);
    QCOMPARE(input->status(), Ant::InputStatus::Error);
    QCOMPARE(statusSpy.count(), 1);

    input->setVariant(Ant::InputVariant::Filled);
    QCOMPARE(input->variant(), Ant::InputVariant::Filled);

    QSignalSpy clearSpy(input, &AntInput::allowClearChanged);
    input->setAllowClear(true);
    QCOMPARE(input->allowClear(), true);
    QCOMPARE(clearSpy.count(), 1);

    QSignalSpy passwordSpy(input, &AntInput::passwordModeChanged);
    input->setPasswordMode(true);
    QCOMPARE(input->isPasswordMode(), true);
    QCOMPARE(passwordSpy.count(), 1);

    QSize hint = input->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);
}

QTEST_MAIN(TestAntInput)
#include "TestAntInput.moc"
