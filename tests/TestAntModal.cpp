#include <QCoreApplication>
#include <QMoveEvent>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>

#include "widgets/AntModal.h"

class TestAntModal : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void modalCachesGeometryThemeAndShadow();
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
    QCOMPARE(modal->commandIconType(), Ant::IconType::None);

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

    modal->setCommandIconType(Ant::IconType::InfoCircle);
    QCOMPARE(modal->commandIconType(), Ant::IconType::InfoCircle);

    auto* parent = new QWidget;
    auto* infoModal = AntModal::info("Info", "Info content", parent);
    QVERIFY(infoModal != nullptr);
    QCOMPARE(infoModal->title(), "Info");
    QCOMPARE(infoModal->content(), "Info content");
    QCOMPARE(infoModal->commandIconType(), Ant::IconType::InfoCircle);

    auto* successModal = AntModal::success("Success", "Done", parent);
    QVERIFY(successModal != nullptr);
    QCOMPARE(successModal->commandIconType(), Ant::IconType::CheckCircle);

    auto* warningModal = AntModal::warning("Warning", "Be careful", parent);
    QVERIFY(warningModal != nullptr);
    QCOMPARE(warningModal->commandIconType(), Ant::IconType::ExclamationCircle);

    auto* errorModal = AntModal::error("Error", "Failed", parent);
    QVERIFY(errorModal != nullptr);
    QCOMPARE(errorModal->commandIconType(), Ant::IconType::CloseCircle);

    auto* confirmModal = AntModal::confirm("Confirm", "Are you sure?", parent);
    QVERIFY(confirmModal != nullptr);
    QCOMPARE(confirmModal->commandIconType(), Ant::IconType::ExclamationCircle);
}

void TestAntModal::modalCachesGeometryThemeAndShadow()
{
    QWidget host;
    host.resize(640, 420);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    AntModal modal(&host);
    modal.setTitle(QStringLiteral("Cache modal"));
    modal.setContent(QStringLiteral("Dialog layout should stay stable while the mask animates."));
    modal.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(modal.isVisible(), 300);
    QTest::qWait(280);

    QVERIFY(modal.property("antModalMaskRegionUpdateCount").toInt() > 0);
    QVERIFY(modal.property("antModalDialogGeometryBuildCount").toInt() > 0);
    QVERIFY(modal.property("antModalOverlayGeometryApplyCount").toInt() > 0);
    QVERIFY(modal.property("antModalThemeApplyCount").toInt() > 0);
    QVERIFY(modal.property("antModalBodySyncApplyCount").toInt() > 0);
    QVERIFY(modal.property("antModalFooterSyncApplyCount").toInt() > 0);

    QWidget* panel = nullptr;
    const auto children = modal.findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* child : children)
    {
        if (child->isVisible())
        {
            panel = child;
            break;
        }
    }
    QVERIFY(panel != nullptr);
    panel->grab();
    const int shadowBuildCount = panel->property("antModalShadowBuildCount").toInt();
    const int shadowHitCount = panel->property("antModalShadowCacheHitCount").toInt();
    QVERIFY(shadowBuildCount > 0);
    panel->repaint();
    QCoreApplication::processEvents();
    QCOMPARE(panel->property("antModalShadowBuildCount").toInt(), shadowBuildCount);
    QVERIFY(panel->property("antModalShadowCacheHitCount").toInt() > shadowHitCount);

    const int geometryHitCount = modal.property("antModalDialogGeometryCacheHitCount").toInt();
    const int themeSkipCount = modal.property("antModalThemeSkipCount").toInt();
    QMoveEvent hostMoveEvent(host.pos(), host.pos());
    QCoreApplication::sendEvent(&host, &hostMoveEvent);
    QVERIFY(modal.property("antModalDialogGeometryCacheHitCount").toInt() > geometryHitCount);
    QVERIFY(modal.property("antModalThemeSkipCount").toInt() > themeSkipCount);

    const int bodyApplyCount = modal.property("antModalBodySyncApplyCount").toInt();
    modal.setContent(QStringLiteral("Updated modal content."));
    QVERIFY(modal.property("antModalBodySyncApplyCount").toInt() > bodyApplyCount);

    const int footerApplyCount = modal.property("antModalFooterSyncApplyCount").toInt();
    modal.setOkText(QStringLiteral("Proceed"));
    QVERIFY(modal.property("antModalFooterSyncApplyCount").toInt() > footerApplyCount);

    modal.setOpen(false);
    QTRY_VERIFY_WITH_TIMEOUT(!modal.isVisible(), 500);
}

QTEST_MAIN(TestAntModal)
#include "TestAntModal.moc"
