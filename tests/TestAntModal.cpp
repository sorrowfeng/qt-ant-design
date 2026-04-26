#include <QSignalSpy>
#include <QTest>

#include "widgets/AntModal.h"

class TestAntModal : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void commandApi();
};

void TestAntModal::propertiesAndSignals()
{
    auto* modal = new AntModal;
    QCOMPARE(modal->title(), QString());
    QCOMPARE(modal->content(), QString());
    QCOMPARE(modal->isOpen(), false);
    QCOMPARE(modal->isClosable(), true);
    QCOMPARE(modal->isMaskClosable(), true);
    QCOMPARE(modal->isCentered(), true);
    QCOMPARE(modal->dialogWidth(), 520);
    QCOMPARE(modal->okText(), "OK");
    QCOMPARE(modal->cancelText(), "Cancel");
    QCOMPARE(modal->showCancel(), true);

    QSignalSpy titleSpy(modal, &AntModal::titleChanged);
    modal->setTitle("Confirm");
    QCOMPARE(modal->title(), "Confirm");
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy contentSpy(modal, &AntModal::contentChanged);
    modal->setContent("Are you sure?");
    QCOMPARE(modal->content(), "Are you sure?");
    QCOMPARE(contentSpy.count(), 1);

    QSignalSpy closableSpy(modal, &AntModal::closableChanged);
    modal->setClosable(false);
    QCOMPARE(modal->isClosable(), false);
    QCOMPARE(closableSpy.count(), 1);

    QSignalSpy maskSpy(modal, &AntModal::maskClosableChanged);
    modal->setMaskClosable(false);
    QCOMPARE(modal->isMaskClosable(), false);
    QCOMPARE(maskSpy.count(), 1);

    QSignalSpy centeredSpy(modal, &AntModal::centeredChanged);
    modal->setCentered(false);
    QCOMPARE(modal->isCentered(), false);
    QCOMPARE(centeredSpy.count(), 1);

    QSignalSpy widthSpy(modal, &AntModal::dialogWidthChanged);
    modal->setDialogWidth(800);
    QCOMPARE(modal->dialogWidth(), 800);
    QCOMPARE(widthSpy.count(), 1);

    QSignalSpy okSpy(modal, &AntModal::okTextChanged);
    modal->setOkText("Yes");
    QCOMPARE(modal->okText(), "Yes");
    QCOMPARE(okSpy.count(), 1);

    QSignalSpy cancelSpy(modal, &AntModal::cancelTextChanged);
    modal->setCancelText("No");
    QCOMPARE(modal->cancelText(), "No");
    QCOMPARE(cancelSpy.count(), 1);

    QSignalSpy showCancelSpy(modal, &AntModal::showCancelChanged);
    modal->setShowCancel(false);
    QCOMPARE(modal->showCancel(), false);
    QCOMPARE(showCancelSpy.count(), 1);
}

void TestAntModal::commandApi()
{
    auto* parent = new QWidget;
    auto* infoModal = AntModal::info("Info", "Info content", parent);
    QVERIFY(infoModal != nullptr);
    QCOMPARE(infoModal->title(), "Info");
    QCOMPARE(infoModal->content(), "Info content");

    auto* successModal = AntModal::success("Success", "Done", parent);
    QVERIFY(successModal != nullptr);

    auto* warningModal = AntModal::warning("Warning", "Be careful", parent);
    QVERIFY(warningModal != nullptr);

    auto* errorModal = AntModal::error("Error", "Failed", parent);
    QVERIFY(errorModal != nullptr);

    auto* confirmModal = AntModal::confirm("Confirm", "Are you sure?", parent);
    QVERIFY(confirmModal != nullptr);
}

QTEST_MAIN(TestAntModal)
#include "TestAntModal.moc"
