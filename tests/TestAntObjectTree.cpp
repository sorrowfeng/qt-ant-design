#include <QCoreApplication>
#include <QEvent>
#include <QPointer>
#include <QSet>
#include <QTest>
#include <QWidget>

#include "WidgetInventory.h"

class TestAntObjectTree : public QObject
{
    Q_OBJECT

private slots:
    void parentOwnsWidgetsAndStyles();
};

namespace
{
// 无独立 AntStyle 的控件（组合/容器类或纯布局辅助），其 style 不以自身为
// parent。其余控件都会安装一个以自身为 parent 的 AntStyle。
QSet<QString> widgetsWithoutOwnStyle()
{
    return {
        QStringLiteral("AntAnchor"),
        QStringLiteral("AntAvatarGroup"),
        QStringLiteral("AntCarousel"),
        QStringLiteral("AntCol"),
        QStringLiteral("AntCollapse"),
        QStringLiteral("AntCollapsePanel"),
        QStringLiteral("AntColorPicker"),
        QStringLiteral("AntDescriptionsItem"),
        QStringLiteral("AntDockManager"),
        QStringLiteral("AntDockWidget"),
        QStringLiteral("AntFlex"),
        QStringLiteral("AntFormItem"),
        QStringLiteral("AntFormList"),
        QStringLiteral("AntFormProvider"),
        QStringLiteral("AntImage"),
        QStringLiteral("AntLayoutContent"),
        QStringLiteral("AntLayoutFooter"),
        QStringLiteral("AntLayoutHeader"),
        QStringLiteral("AntLayoutSider"),
        QStringLiteral("AntListItem"),
        QStringLiteral("AntListItemMeta"),
        QStringLiteral("AntListy"),
        QStringLiteral("AntLog"),
        QStringLiteral("AntMasonry"),
        QStringLiteral("AntNav"),
        QStringLiteral("AntNavItem"),
        QStringLiteral("AntRibbon"),
        QStringLiteral("AntRibbonGroup"),
        QStringLiteral("AntRibbonPage"),
        QStringLiteral("AntRow"),
        QStringLiteral("AntScrollArea"),
        QStringLiteral("AntSplitter"),
        QStringLiteral("AntTransfer"),
        QStringLiteral("AntWidget"),
    };
}

// 纯 QObject（非 QWidget）控件，没有可绘制的 widget 表面，也没有 style。
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

void TestAntObjectTree::parentOwnsWidgetsAndStyles()
{
    auto* root = new QWidget;
    QList<QPointer<QObject>> tracked;

    for (const AntTestUtils::WidgetFactoryCase& objectCase : AntTestUtils::allWidgetFactoryCases())
    {
        QObject* object = objectCase.create(root);
        QVERIFY2(object != nullptr, objectCase.name);
        QCOMPARE(object->parent(), root);
        tracked.append(QPointer<QObject>(object));

        auto* widget = qobject_cast<QWidget*>(object);
        if (!widget || nonWidgetObjects().contains(QString::fromLatin1(objectCase.name)))
        {
            continue;
        }
        if (widgetsWithoutOwnStyle().contains(QString::fromLatin1(objectCase.name)))
        {
            continue;
        }
        QVERIFY(widget->style() != nullptr);
        QCOMPARE(widget->style()->parent(), widget);
    }

    delete root;
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    for (const auto& object : std::as_const(tracked))
    {
        QVERIFY(object.isNull());
    }
}

QTEST_MAIN(TestAntObjectTree)
#include "TestAntObjectTree.moc"
