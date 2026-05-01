#include <QFrame>
#include <QLineEdit>
#include <QPointer>
#include <QSignalSpy>
#include <QTest>
#include <QVariant>

#include "core/AntTheme.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntMessage.h"
#include "widgets/AntNotification.h"
#include "widgets/AntTable.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTree.h"

class TestAntAdvancedInteractions : public QObject
{
    Q_OBJECT

private slots:
    void colorPickerHexEditorCommitsSelection();
    void treeExpandCheckAndSelectFlow();
    void tableSortSelectionAndPaginationFlow();
    void tabsKeyboardAddAndCloseFlow();
    void messageAndNotificationManualDismissFlow();
};

namespace
{
QFrame* visiblePopupFrame(QWidget* owner)
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

AntTreeNode treeNode(const QString& key, const QString& title, bool isLeaf = true, QVector<AntTreeNode> children = {})
{
    AntTreeNode node;
    node.key = key;
    node.title = title;
    node.isLeaf = isLeaf;
    node.children = std::move(children);
    return node;
}

AntTableRow tableRow(const QString& id, const QString& name)
{
    AntTableRow row;
    row.data.insert(QStringLiteral("id"), id);
    row.data.insert(QStringLiteral("name"), name);
    return row;
}

QRect editableTabRect(const AntTabs& tabs, const QStringList& labels, int index)
{
    const auto& token = antTheme->tokens();
    QFont font = tabs.font();
    font.setPixelSize(token.fontSize);
    const QFontMetrics metrics(font);

    int x = 0;
    for (int i = 0; i <= index; ++i)
    {
        const int width = qMax(72, metrics.horizontalAdvance(labels.at(i)) + 26 + token.padding * 2);
        if (i == index)
        {
            return QRect(x, 0, width, token.controlHeightLG);
        }
        x += width + 2;
    }
    return {};
}

QRect editableAddRect(const AntTabs& tabs, const QStringList& labels)
{
    const auto& token = antTheme->tokens();
    const QRect last = editableTabRect(tabs, labels, labels.size() - 1);
    const int side = token.controlHeightLG - token.paddingXS;
    return QRect(last.right() + token.marginXS, token.paddingXS / 2, side, side);
}

QRect editableCloseRect(const QRect& tabRect)
{
    constexpr int side = 18;
    return QRect(tabRect.right() - side - 8, tabRect.center().y() - side / 2, side, side);
}
} // namespace

void TestAntAdvancedInteractions::colorPickerHexEditorCommitsSelection()
{
    AntColorPicker picker(QColor(QStringLiteral("#1677FF")));
    picker.resize(picker.sizeHint());
    picker.show();
    QVERIFY(QTest::qWaitForWindowExposed(&picker));

    QSignalSpy colorSpy(&picker, &AntColorPicker::currentColorChanged);
    QSignalSpy selectedSpy(&picker, &AntColorPicker::colorSelected);

    picker.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(picker.isOpen(), 300);

    QFrame* popup = nullptr;
    QTRY_VERIFY_WITH_TIMEOUT((popup = visiblePopupFrame(&picker)) != nullptr, 300);

    QLineEdit* hexEdit = nullptr;
    const auto edits = popup->findChildren<QLineEdit*>();
    for (QLineEdit* edit : edits)
    {
        if (edit && !edit->isReadOnly())
        {
            hexEdit = edit;
            break;
        }
    }
    QVERIFY(hexEdit);

    hexEdit->setFocus();
    hexEdit->clear();
    QTest::keyClicks(hexEdit, QStringLiteral("#00FF00"));
    QTest::keyClick(hexEdit, Qt::Key_Return);

    QTRY_COMPARE_WITH_TIMEOUT(picker.currentColor().name(QColor::HexRgb).toUpper(), QStringLiteral("#00FF00"), 300);
    QCOMPARE(colorSpy.count(), 1);
    QCOMPARE(selectedSpy.count(), 1);
    QCOMPARE(selectedSpy.takeFirst().at(0).value<QColor>().name(QColor::HexRgb).toUpper(), QStringLiteral("#00FF00"));
}

void TestAntAdvancedInteractions::treeExpandCheckAndSelectFlow()
{
    AntTree tree;
    const AntTreeNode child = treeNode(QStringLiteral("child"), QStringLiteral("Child"));
    tree.setTreeData({treeNode(QStringLiteral("root"), QStringLiteral("Root"), false, {child})});
    tree.resize(240, 120);
    tree.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tree));

    QSignalSpy expandedSpy(&tree, &AntTree::nodeExpanded);
    QSignalSpy checkedSpy(&tree, &AntTree::nodeChecked);
    QSignalSpy selectedSpy(&tree, &AntTree::nodeSelected);

    QTest::mouseClick(&tree, Qt::LeftButton, Qt::NoModifier, QPoint(12, 14));
    QCOMPARE(tree.expandedKeys(), QStringList({QStringLiteral("root")}));
    QCOMPARE(expandedSpy.count(), 1);
    QCOMPARE(expandedSpy.takeFirst().at(0).toString(), QStringLiteral("root"));

    QTest::mouseClick(&tree, Qt::LeftButton, Qt::NoModifier, QPoint(60, 42));
    QCOMPARE(tree.checkedKeys(), QStringList({QStringLiteral("root"), QStringLiteral("child")}));
    QCOMPARE(checkedSpy.count(), 1);
    QCOMPARE(checkedSpy.takeFirst().at(0).toStringList(), QStringList({QStringLiteral("root"), QStringLiteral("child")}));

    QTest::mouseClick(&tree, Qt::LeftButton, Qt::NoModifier, QPoint(92, 42));
    QCOMPARE(tree.selectedKeys(), QStringList({QStringLiteral("child")}));
    QCOMPARE(selectedSpy.count(), 1);
    QCOMPARE(selectedSpy.takeFirst().at(0).toString(), QStringLiteral("child"));
}

void TestAntAdvancedInteractions::tableSortSelectionAndPaginationFlow()
{
    AntTableColumn idColumn;
    idColumn.title = QStringLiteral("ID");
    idColumn.dataIndex = QStringLiteral("id");
    idColumn.key = QStringLiteral("id");
    idColumn.width = 120;

    AntTableColumn nameColumn;
    nameColumn.title = QStringLiteral("Name");
    nameColumn.dataIndex = QStringLiteral("name");
    nameColumn.key = QStringLiteral("name");
    nameColumn.width = 160;
    nameColumn.sorter = true;

    AntTable table;
    table.setRowSelection(Ant::TableSelectionMode::Checkbox);
    table.setPageSize(2);
    table.setColumns({idColumn, nameColumn});
    table.setRows({tableRow(QStringLiteral("2"), QStringLiteral("Beta")),
                   tableRow(QStringLiteral("1"), QStringLiteral("Alpha")),
                   tableRow(QStringLiteral("4"), QStringLiteral("Delta")),
                   tableRow(QStringLiteral("3"), QStringLiteral("Gamma"))});
    table.resize(table.sizeHint());
    table.show();
    QVERIFY(QTest::qWaitForWindowExposed(&table));

    QSignalSpy sortSpy(&table, &AntTable::sortChanged);
    QSignalSpy selectionSpy(&table, &AntTable::selectionChanged);
    QSignalSpy rowSpy(&table, &AntTable::rowClicked);
    QSignalSpy pageSpy(&table, &AntTable::pageChanged);

    QTest::mouseClick(&table, Qt::LeftButton, Qt::NoModifier, QPoint(188, 28));
    QCOMPARE(table.currentSortColumn(), QStringLiteral("name"));
    QCOMPARE(table.sortOrder(), Ant::TableSortOrder::Ascending);
    QCOMPARE(sortSpy.count(), 1);

    QTest::mouseClick(&table, Qt::LeftButton, Qt::NoModifier, QPoint(24, 28));
    QCOMPARE(table.selectedRowKeys(), QStringList({QStringLiteral("1"), QStringLiteral("2")}));
    QCOMPARE(selectionSpy.count(), 1);

    QTest::mouseClick(&table, Qt::LeftButton, Qt::NoModifier, QPoint(24, 84));
    QCOMPARE(table.selectedRowKeys(), QStringList({QStringLiteral("2")}));
    QCOMPARE(selectionSpy.count(), 2);

    QTest::mouseClick(&table, Qt::LeftButton, Qt::NoModifier, QPoint(220, 140));
    QCOMPARE(rowSpy.count(), 1);
    QCOMPARE(rowSpy.takeFirst().at(0).toInt(), 1);

    QTest::mouseClick(&table, Qt::LeftButton, Qt::NoModifier, QPoint(table.width() - 32, table.height() - 24));
    QCOMPARE(table.currentPage(), 2);
    QCOMPARE(pageSpy.count(), 1);
    QCOMPARE(pageSpy.takeFirst().at(0).toInt(), 2);
}

void TestAntAdvancedInteractions::tabsKeyboardAddAndCloseFlow()
{
    AntTabs tabs;
    tabs.setTabsType(Ant::TabsType::EditableCard);
    const QStringList labels{QStringLiteral("One"), QStringLiteral("Two"), QStringLiteral("Three")};
    tabs.addTab(new QWidget, QStringLiteral("one"), labels.at(0));
    tabs.addTab(new QWidget, QStringLiteral("two"), labels.at(1), QString(), true);
    tabs.addTab(new QWidget, QStringLiteral("three"), labels.at(2));
    tabs.resize(360, 160);
    tabs.show();
    QVERIFY(QTest::qWaitForWindowExposed(&tabs));

    QSignalSpy currentSpy(&tabs, &AntTabs::currentChanged);
    QSignalSpy addSpy(&tabs, &AntTabs::tabAddRequested);
    QSignalSpy closeSpy(&tabs, &AntTabs::tabCloseRequested);

    tabs.setFocus();
    QTest::keyClick(&tabs, Qt::Key_Right);
    QCOMPARE(tabs.activeKey(), QStringLiteral("three"));
    QCOMPARE(currentSpy.count(), 1);
    QCOMPARE(currentSpy.takeFirst().at(0).toInt(), 2);

    QTest::mouseClick(&tabs, Qt::LeftButton, Qt::NoModifier, editableAddRect(tabs, labels).center());
    QCOMPARE(addSpy.count(), 1);

    QTest::mouseClick(&tabs, Qt::LeftButton, Qt::NoModifier, editableCloseRect(editableTabRect(tabs, labels, 0)).center());
    QCOMPARE(closeSpy.count(), 1);
    QCOMPARE(closeSpy.takeFirst().at(0).toString(), QStringLiteral("one"));
    QCOMPARE(tabs.activeKey(), QStringLiteral("three"));
}

void TestAntAdvancedInteractions::messageAndNotificationManualDismissFlow()
{
    QWidget anchor;
    anchor.resize(480, 320);
    anchor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&anchor));

    {
        QPointer<AntMessage> message = AntMessage::success(QStringLiteral("Saved"), &anchor, 0, Ant::Placement::Top);
        QVERIFY(message);
        QSignalSpy messageClosedSpy(message.data(), &AntMessage::closed);
        QTRY_VERIFY_WITH_TIMEOUT(message->isVisible(), 300);

        QTest::mouseClick(message.data(), Qt::LeftButton, Qt::NoModifier, message->rect().center());
        QTRY_VERIFY_WITH_TIMEOUT(message.isNull(), 600);
        QVERIFY(messageClosedSpy.count() >= 1);
    }

    {
        QPointer<AntNotification> notification = AntNotification::success(QStringLiteral("Saved"),
                                                                          QStringLiteral("Configuration updated"),
                                                                          &anchor,
                                                                          0,
                                                                          Ant::Placement::TopRight);
        QVERIFY(notification);
        QSignalSpy clickedSpy(notification.data(), &AntNotification::clicked);
        QSignalSpy notificationClosedSpy(notification.data(), &AntNotification::closed);
        QTRY_VERIFY_WITH_TIMEOUT(notification->isVisible(), 300);

        QTest::mouseClick(notification.data(), Qt::LeftButton, Qt::NoModifier, notification->rect().center());
        QCOMPARE(clickedSpy.count(), 1);

        QTest::mouseClick(notification.data(), Qt::LeftButton, Qt::NoModifier, QPoint(notification->width() - 53, 42));
        QTRY_VERIFY_WITH_TIMEOUT(notification.isNull(), 600);
        QVERIFY(notificationClosedSpy.count() >= 1);
    }
}

QTEST_MAIN(TestAntAdvancedInteractions)
#include "TestAntAdvancedInteractions.moc"
