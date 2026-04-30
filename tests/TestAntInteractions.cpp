#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFrame>
#include <QCoreApplication>
#include <QMimeData>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>

#include "widgets/AntCascader.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntMenu.h"
#include "widgets/AntSelect.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntUpload.h"

class TestAntInteractions : public QObject
{
    Q_OBJECT

private slots:
    void dropdownClickTargetAndMenuItem();
    void menuSubmenuPopupItemSelection();
    void selectKeyboardNavigationSkipsDisabledOptions();
    void cascaderPopupColumnSelection();
    void datePickerPopupSelection();
    void timePickerPopupSelection();
    void inputNumberFocusRevealsControlsAndSteps();
    void uploadDraggerDropFlow();
};

namespace
{
QWidget* directVisibleFrameChild(QWidget* owner)
{
    const auto frames = owner->findChildren<QFrame*>(QString(), Qt::FindDirectChildrenOnly);
    for (QFrame* frame : frames)
    {
        if (frame && frame->isVisible())
        {
            return frame;
        }
    }
    return nullptr;
}

QPoint cascaderCellCenter(int column, int row)
{
    constexpr int shadowMargin = 8;
    constexpr int popupPadding = 4;
    constexpr int columnWidth = 112;
    constexpr int optionHeight = 32;
    return QPoint(shadowMargin + popupPadding + column * columnWidth + columnWidth / 2,
                  shadowMargin + popupPadding + row * optionHeight + optionHeight / 2);
}

bool writeTempFile(const QString& path, const QByteArray& data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }
    file.write(data);
    return true;
}

void sendDrop(AntUpload* upload, QMimeData* mimeData, const QPoint& pos = QPoint(12, 12))
{
    QDragEnterEvent enterEvent(pos, Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(upload, &enterEvent);

    QDropEvent dropEvent(QPointF(pos), Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(upload, &dropEvent);
}
} // namespace

void TestAntInteractions::dropdownClickTargetAndMenuItem()
{
    QPushButton target(QStringLiteral("Actions"));
    target.resize(120, 32);
    target.show();
    QVERIFY(QTest::qWaitForWindowExposed(&target));

    AntDropdown dropdown;
    dropdown.setTarget(&target);
    dropdown.setTrigger(Ant::DropdownTrigger::Click);
    dropdown.addItem(QStringLiteral("download"), QStringLiteral("Download"));
    dropdown.addItem(QStringLiteral("archive"), QStringLiteral("Archive"));

    QSignalSpy openSpy(&dropdown, &AntDropdown::openChanged);
    QSignalSpy itemSpy(&dropdown, &AntDropdown::itemTriggered);

    QTest::mouseClick(&target, Qt::LeftButton, Qt::NoModifier, target.rect().center());
    QTRY_VERIFY_WITH_TIMEOUT(dropdown.isOpen(), 300);
    QVERIFY(openSpy.count() >= 1);
    QVERIFY(dropdown.menu()->isVisible());

    QTest::mouseClick(dropdown.menu(), Qt::LeftButton, Qt::NoModifier, QPoint(24, 18));
    QCOMPARE(itemSpy.count(), 1);
    QCOMPARE(itemSpy.takeFirst().at(0).toString(), QStringLiteral("download"));
    QCOMPARE(dropdown.isOpen(), false);
}

void TestAntInteractions::menuSubmenuPopupItemSelection()
{
    AntMenu menu;
    menu.setMode(Ant::MenuMode::Vertical);
    menu.addSubMenu(QStringLiteral("products"), QStringLiteral("Products"));
    menu.addSubItem(QStringLiteral("products"), QStringLiteral("analytics"), QStringLiteral("Analytics"));
    menu.addSubItem(QStringLiteral("products"), QStringLiteral("billing"), QStringLiteral("Billing"));
    menu.resize(menu.sizeHint());
    menu.show();
    QVERIFY(QTest::qWaitForWindowExposed(&menu));

    QSignalSpy clickedSpy(&menu, &AntMenu::itemClicked);
    QSignalSpy selectedSpy(&menu, &AntMenu::itemSelected);

    QTest::mouseClick(&menu, Qt::LeftButton, Qt::NoModifier, QPoint(24, 20));

    AntMenu* popupMenu = nullptr;
    QTRY_VERIFY_WITH_TIMEOUT(([&]() {
        const auto menus = menu.findChildren<AntMenu*>();
        for (AntMenu* candidate : menus)
        {
            if (candidate && candidate->isVisible())
            {
                popupMenu = candidate;
                return true;
            }
        }
        return false;
    })(), 300);

    QTest::mouseClick(popupMenu, Qt::LeftButton, Qt::NoModifier, QPoint(24, 18));
    QTRY_COMPARE_WITH_TIMEOUT(menu.selectedKey(), QStringLiteral("analytics"), 300);
    QCOMPARE(clickedSpy.takeLast().at(0).toString(), QStringLiteral("analytics"));
    QCOMPARE(selectedSpy.takeLast().at(0).toString(), QStringLiteral("analytics"));
}

void TestAntInteractions::selectKeyboardNavigationSkipsDisabledOptions()
{
    AntSelect select;
    select.addOption(QStringLiteral("Apple"), QStringLiteral("apple"));
    select.addOption(QStringLiteral("Banana"), QStringLiteral("banana"), true);
    select.addOption(QStringLiteral("Cherry"), QStringLiteral("cherry"));
    select.resize(180, select.sizeHint().height());
    select.show();
    QVERIFY(QTest::qWaitForWindowExposed(&select));

    QSignalSpy selectedSpy(&select, &AntSelect::optionSelected);

    select.setFocus();
    QTest::keyClick(&select, Qt::Key_Space);
    QTRY_VERIFY_WITH_TIMEOUT(select.isOpen(), 300);

    QTest::keyClick(&select, Qt::Key_Down);
    QTest::keyClick(&select, Qt::Key_Return);

    QCOMPARE(select.currentIndex(), 2);
    QCOMPARE(select.currentText(), QStringLiteral("Cherry"));
    QCOMPARE(selectedSpy.count(), 1);
    QCOMPARE(select.isOpen(), false);
}

void TestAntInteractions::cascaderPopupColumnSelection()
{
    AntCascaderOption hangzhou{QStringLiteral("hangzhou"), QStringLiteral("Hangzhou"), {}, false, true};
    AntCascaderOption ningbo{QStringLiteral("ningbo"), QStringLiteral("Ningbo"), {}, false, true};
    AntCascaderOption zhejiang{QStringLiteral("zhejiang"), QStringLiteral("Zhejiang"), {hangzhou, ningbo}, false, false};
    AntCascaderOption nanjing{QStringLiteral("nanjing"), QStringLiteral("Nanjing"), {}, false, true};
    AntCascaderOption jiangsu{QStringLiteral("jiangsu"), QStringLiteral("Jiangsu"), {nanjing}, false, false};

    AntCascader cascader;
    cascader.setOptions({zhejiang, jiangsu});
    cascader.resize(180, cascader.sizeHint().height());
    cascader.show();
    QVERIFY(QTest::qWaitForWindowExposed(&cascader));

    QSignalSpy valueSpy(&cascader, &AntCascader::valueChanged);

    QTest::mouseClick(&cascader, Qt::LeftButton, Qt::NoModifier, cascader.rect().center());
    QTRY_VERIFY_WITH_TIMEOUT(cascader.isOpen(), 300);

    auto* popup = cascader.findChild<QFrame*>(QStringLiteral("CascaderPopup"));
    QVERIFY(popup);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);

    QTest::mouseClick(popup, Qt::LeftButton, Qt::NoModifier, cascaderCellCenter(0, 0));
    QCOMPARE(cascader.value(), QStringList({QStringLiteral("zhejiang")}));

    QTest::mouseClick(popup, Qt::LeftButton, Qt::NoModifier, cascaderCellCenter(1, 1));
    QCOMPARE(cascader.value(), QStringList({QStringLiteral("zhejiang"), QStringLiteral("ningbo")}));
    QCOMPARE(cascader.isOpen(), false);
    QVERIFY(valueSpy.count() >= 2);
}

void TestAntInteractions::datePickerPopupSelection()
{
    AntDatePicker picker;
    picker.setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    picker.setSelectedDate(QDate(2026, 4, 1));
    picker.resize(picker.sizeHint());
    picker.show();
    QVERIFY(QTest::qWaitForWindowExposed(&picker));

    QSignalSpy dateSpy(&picker, &AntDatePicker::selectedDateChanged);

    QTest::mouseClick(&picker, Qt::LeftButton, Qt::NoModifier, picker.rect().center());
    QTRY_VERIFY_WITH_TIMEOUT(picker.isOpen(), 300);

    auto* popup = directVisibleFrameChild(&picker);
    QVERIFY(popup);

    // 2026-04-15 is row 2, column 3 in the April 2026 panel.
    QTest::mouseClick(popup, Qt::LeftButton, Qt::NoModifier, QPoint(152, 177));
    QCOMPARE(picker.selectedDate(), QDate(2026, 4, 15));
    QCOMPARE(picker.dateString(), QStringLiteral("2026-04-15"));
    QCOMPARE(picker.isOpen(), false);
    QCOMPARE(dateSpy.count(), 1);
}

void TestAntInteractions::timePickerPopupSelection()
{
    AntTimePicker picker;
    picker.setDisplayFormat(QStringLiteral("HH:mm:ss"));
    picker.setSelectedTime(QTime(10, 20, 30));
    picker.resize(picker.sizeHint());
    picker.show();
    QVERIFY(QTest::qWaitForWindowExposed(&picker));

    QSignalSpy acceptedSpy(&picker, &AntTimePicker::accepted);

    QTest::mouseClick(&picker, Qt::LeftButton, Qt::NoModifier, picker.rect().center());
    QTRY_VERIFY_WITH_TIMEOUT(picker.isOpen(), 300);

    auto* popup = directVisibleFrameChild(&picker);
    QVERIFY(popup);

    // Minute column, row 1: current minute plus one step.
    QTest::mouseClick(popup, Qt::LeftButton, Qt::NoModifier, QPoint(92, 54));
    QCOMPARE(picker.selectedTime(), QTime(10, 21, 30));

    QTest::mouseClick(popup, Qt::LeftButton, Qt::NoModifier, QPoint(134, 253));
    QTRY_VERIFY_WITH_TIMEOUT(!picker.isOpen(), 300);
    QCOMPARE(acceptedSpy.count(), 1);
    QCOMPARE(acceptedSpy.takeFirst().at(0).toTime(), QTime(10, 21, 30));
}

void TestAntInteractions::inputNumberFocusRevealsControlsAndSteps()
{
    AntInputNumber input;
    input.setRange(0, 10);
    input.setValue(5);
    input.resize(140, input.sizeHint().height());
    input.show();
    QVERIFY(QTest::qWaitForWindowExposed(&input));

    input.setFocus();
    QTRY_VERIFY_WITH_TIMEOUT(input.controlsProgress() > 0.95, 300);

    QTest::mouseClick(&input, Qt::LeftButton, Qt::NoModifier, QPoint(input.width() - 8, input.height() / 4));
    QCOMPARE(input.value(), 6.0);

    QTest::mouseClick(&input, Qt::LeftButton, Qt::NoModifier, QPoint(input.width() - 8, input.height() * 3 / 4));
    QCOMPARE(input.value(), 5.0);
}

void TestAntInteractions::uploadDraggerDropFlow()
{
    qRegisterMetaType<AntUploadFile>("AntUploadFile");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString textPath = dir.filePath(QStringLiteral("accepted.txt"));
    const QString pngPath = dir.filePath(QStringLiteral("ignored.png"));
    const QString secondTextPath = dir.filePath(QStringLiteral("second.txt"));
    QVERIFY(writeTempFile(textPath, QByteArrayLiteral("ok")));
    QVERIFY(writeTempFile(pngPath, QByteArrayLiteral("png")));
    QVERIFY(writeTempFile(secondTextPath, QByteArrayLiteral("second")));

    AntUpload upload;
    upload.setDraggerMode(true);
    upload.setAccept(QStringLiteral(".txt"));
    upload.setMaxCount(1);
    upload.resize(upload.sizeHint());

    QMimeData mime;
    mime.setUrls({QUrl::fromLocalFile(textPath), QUrl::fromLocalFile(pngPath)});

    QSignalSpy addedSpy(&upload, &AntUpload::fileAdded);
    sendDrop(&upload, &mime);

    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(upload.fileList().size(), 1);
    QCOMPARE(upload.fileList().first().name, QStringLiteral("accepted.txt"));
    QCOMPARE(upload.fileList().first().status, Ant::UploadFileStatus::Done);

    AntUpload singleUpload;
    singleUpload.setDraggerMode(true);
    singleUpload.setAccept(QStringLiteral(".txt"));

    QMimeData multiMime;
    multiMime.setUrls({QUrl::fromLocalFile(textPath), QUrl::fromLocalFile(secondTextPath)});
    sendDrop(&singleUpload, &multiMime);
    QCOMPARE(singleUpload.fileList().size(), 1);
    QCOMPARE(singleUpload.fileList().first().name, QStringLiteral("accepted.txt"));

    AntUpload disabledUpload;
    disabledUpload.setDraggerMode(true);
    disabledUpload.setDisabled(true);

    QMimeData disabledMime;
    disabledMime.setUrls({QUrl::fromLocalFile(textPath)});
    QDragEnterEvent disabledEnter(QPoint(12, 12), Qt::CopyAction, &disabledMime, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&disabledUpload, &disabledEnter);
    QVERIFY(!disabledEnter.isAccepted());

    QDropEvent disabledDrop(QPointF(12, 12), Qt::CopyAction, &disabledMime, Qt::LeftButton, Qt::NoModifier);
    QCoreApplication::sendEvent(&disabledUpload, &disabledDrop);
    QCOMPARE(disabledUpload.fileList().size(), 0);
}

QTEST_MAIN(TestAntInteractions)
#include "TestAntInteractions.moc"
