#include <QAccessible>
#include <QApplication>
#include <QSet>
#include <QTest>
#include <QWidget>

#include "WidgetInventory.h"

// 无障碍最小覆盖：验证每个公开控件都暴露可用的 Qt 无障碍接口
// （accessibleName/accessibleDescription 读写 + QAccessible 查询），
// 保证屏幕阅读器等辅助技术能识别它们。
//
// 注意：这里不要求控件预设 accessibleName（那是产品文案策略），而是
// 验证无障碍 API 本身对这些控件是"活的"——即 setAccessibleName 之后
// 能正确回读、QAccessible 能查询到非空接口。

class TestAntAccessibility : public QObject
{
    Q_OBJECT

private slots:
    void everyWidgetExposesAccessibleInterface();
};

namespace
{
QSet<QString> nonWidgetObjects()
{
    return {
        QStringLiteral("AntAffix"),
        QStringLiteral("AntApp"),
        QStringLiteral("AntConfigProvider"),
        QStringLiteral("AntTour"),
    };
}
} // namespace

void TestAntAccessibility::everyWidgetExposesAccessibleInterface()
{
    auto* root = new QWidget;
    int widgetCount = 0;

    for (const AntTestUtils::WidgetFactoryCase& objectCase : AntTestUtils::allWidgetFactoryCases())
    {
        if (nonWidgetObjects().contains(QString::fromLatin1(objectCase.name)))
        {
            continue;
        }

        QObject* object = objectCase.create(root);
        auto* widget = qobject_cast<QWidget*>(object);
        QVERIFY2(widget != nullptr, objectCase.name);
        ++widgetCount;

        // accessibleName 读写往返。
        const QString name = QStringLiteral("Accessible ") + QString::fromLatin1(objectCase.name);
        widget->setAccessibleName(name);
        QCOMPARE(widget->accessibleName(), name);

        // accessibleDescription 读写往返。
        const QString description = QStringLiteral("Description for ") + QString::fromLatin1(objectCase.name);
        widget->setAccessibleDescription(description);
        QCOMPARE(widget->accessibleDescription(), description);

        // QAccessible 能查询到非空接口（辅助技术的入口）。该接口由 Qt 的
        // 无障碍缓存管理，调用者不负责释放。角色必须有效（非 NoRole）。
        QAccessibleInterface* interface = QAccessible::queryAccessibleInterface(widget);
        QVERIFY2(interface != nullptr, objectCase.name);
        QVERIFY2(interface->role() != QAccessible::NoRole, objectCase.name);
    }

    QVERIFY(widgetCount > 0);
    delete root;
}

QTEST_MAIN(TestAntAccessibility)
#include "TestAntAccessibility.moc"
