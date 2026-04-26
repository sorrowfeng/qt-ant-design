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
    QCOMPARE(input->inputSize(), Ant::Size::Middle);
    QCOMPARE(input->status(), Ant::Status::Normal);
    QCOMPARE(input->variant(), Ant::Variant::Outlined);
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
    input->setInputSize(Ant::Size::Large);
    QCOMPARE(input->inputSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy statusSpy(input, &AntInput::statusChanged);
    input->setStatus(Ant::Status::Error);
    QCOMPARE(input->status(), Ant::Status::Error);
    QCOMPARE(statusSpy.count(), 1);

    input->setVariant(Ant::Variant::Filled);
    QCOMPARE(input->variant(), Ant::Variant::Filled);

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
