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
    QCOMPARE(input->placeholderText(), QString());
    QCOMPARE(input->isReadOnly(), false);
    QCOMPARE(input->maxLength(), 32767);
    QCOMPARE(input->echoMode(), QLineEdit::Normal);
    QCOMPARE(input->alignment(), Qt::AlignLeft | Qt::AlignVCenter);

    QSignalSpy textSpy(input, &AntInput::textChanged);
    input->setText("hello");
    QCOMPARE(input->text(), "hello");
    QCOMPARE(textSpy.count(), 1);

    input->setPlaceholderText("Enter text");
    QCOMPARE(input->placeholderText(), "Enter text");
    QCOMPARE(input->lineEdit()->placeholderText(), "Enter text");

    input->setReadOnly(true);
    QCOMPARE(input->isReadOnly(), true);
    QCOMPARE(input->lineEdit()->isReadOnly(), true);
    input->setReadOnly(false);

    input->setMaxLength(4);
    QCOMPARE(input->maxLength(), 4);
    input->setText("abcdef");
    QCOMPARE(input->text(), "abcd");

    input->setEchoMode(QLineEdit::Password);
    QCOMPARE(input->echoMode(), QLineEdit::Password);
    QCOMPARE(input->lineEdit()->echoMode(), QLineEdit::Password);

    input->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QCOMPARE(input->alignment(), Qt::AlignRight | Qt::AlignVCenter);
    QCOMPARE(input->lineEdit()->alignment(), Qt::AlignRight | Qt::AlignVCenter);

    input->setCursorPosition(2);
    QCOMPARE(input->cursorPosition(), 2);
    input->selectAll();
    QVERIFY(input->hasSelectedText());
    QCOMPARE(input->selectedText(), QStringLiteral("abcd"));
    input->deselect();
    QVERIFY(!input->hasSelectedText());
    input->clear();
    QCOMPARE(input->text(), QString());

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

    QSignalSpy editedSpy(input, &AntInput::textEdited);
    QSignalSpy returnSpy(input, &AntInput::returnPressed);
    QTest::keyClicks(input->lineEdit(), "xy");
    QCOMPARE(input->text(), QStringLiteral("xy"));
    QCOMPARE(editedSpy.count(), 2);
    QTest::keyClick(input->lineEdit(), Qt::Key_Return);
    QCOMPARE(returnSpy.count(), 1);

    QSize hint = input->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);
}

QTEST_MAIN(TestAntInput)
#include "TestAntInput.moc"
