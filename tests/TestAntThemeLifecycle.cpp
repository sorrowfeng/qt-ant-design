#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QFrame>
#include <QPointer>
#include <QPushButton>
#include <QStyle>
#include <QTest>
#include <QVector>
#include <QWidget>

#include <functional>
#include <utility>

#include "core/AntTheme.h"
#include "core/AntStyleBase.h"
#include "styles/AntTagStyle.h"
#include "TestUtils.h"
#include "WidgetInventory.h"

class TestAntThemeLifecycle : public QObject
{
    Q_OBJECT

private slots:
    void allControlsSurviveThemeSwitchAndDestroyCleanly();
    void openPopupsSurviveThemeSwitch();
    void styleThemeUpdateScopesToOwnedWidget();
};

namespace
{
class ThemeModeGuard
{
public:
    ThemeModeGuard()
        : m_originalMode(antTheme->themeMode())
    {
    }

    ~ThemeModeGuard()
    {
        antTheme->setThemeMode(m_originalMode);
        QCoreApplication::processEvents();
    }

    Ant::ThemeMode alternateMode() const
    {
        return m_originalMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark;
    }

private:
    Ant::ThemeMode m_originalMode;
};

class ThemeMetricProbe : public QWidget
{
    Q_OBJECT

public:
    using QWidget::QWidget;

    QSize sizeHint() const override
    {
        return QSize(120, antTheme->themeMode() == Ant::ThemeMode::Dark ? 48 : 24);
    }

    QSize minimumSizeHint() const override
    {
        return sizeHint();
    }
};

class ThemeMetricProbeStyle : public AntStyleBase
{
public:
    explicit ThemeMetricProbeStyle(QStyle* style = nullptr)
        : AntStyleBase(style)
    {
        connectThemeUpdate<ThemeMetricProbe>();
    }
};

class ThemeStableProbe : public QWidget
{
    Q_OBJECT

public:
    using QWidget::QWidget;

    QSize sizeHint() const override
    {
        return QSize(120, 24);
    }

    QSize minimumSizeHint() const override
    {
        return sizeHint();
    }
};

class ThemeStableProbeStyle : public AntStyleBase
{
public:
    explicit ThemeStableProbeStyle(QStyle* style = nullptr)
        : AntStyleBase(style)
    {
        connectThemeUpdate<ThemeStableProbe>();
    }
};

using AntTestUtils::waitUntil;

void drainDeferredDeletes()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

void sizeAndShowIfWidget(QObject* object)
{
    auto* widget = qobject_cast<QWidget*>(object);
    if (!widget)
    {
        return;
    }
    QSize size = widget->sizeHint();
    if (!size.isValid() || size.isEmpty())
    {
        size = QSize(120, 48);
    }
    widget->resize(size.expandedTo(QSize(80, 32)));
    widget->ensurePolished();
    widget->show();
}

QWidget* directVisiblePopupChild(QWidget* owner)
{
    const auto children = owner->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* child : children)
    {
        if (child && child->isVisible())
        {
            const Qt::WindowType type = child->windowType();
            if (type == Qt::Popup || type == Qt::ToolTip)
            {
                return child;
            }
        }
    }
    return nullptr;
}

AntCascaderOption cascaderOption()
{
    AntCascaderOption option;
    option.value = QStringLiteral("root");
    option.label = QStringLiteral("Root");
    option.isLeaf = true;
    return option;
}

AntTreeNode treeNode()
{
    AntTreeNode node;
    node.key = QStringLiteral("root");
    node.title = QStringLiteral("Root");
    return node;
}
} // namespace

void TestAntThemeLifecycle::allControlsSurviveThemeSwitchAndDestroyCleanly()
{
    ThemeModeGuard guard;
    auto* root = new QWidget;
    root->resize(960, 720);
    root->show();
    QVERIFY(QTest::qWaitForWindowExposed(root));

    QList<QPointer<QObject>> tracked;
    for (const AntTestUtils::WidgetFactoryCase& objectCase : AntTestUtils::allWidgetFactoryCases())
    {
        QObject* object = objectCase.create(root);
        QVERIFY2(object != nullptr, objectCase.name);
        tracked.append(QPointer<QObject>(object));
        sizeAndShowIfWidget(object);
    }

    antTheme->setThemeMode(guard.alternateMode());
    QCoreApplication::processEvents();
    for (const QPointer<QObject>& object : std::as_const(tracked))
    {
        QVERIFY(!object.isNull());
    }

    antTheme->setThemeMode(guard.alternateMode() == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);
    QCoreApplication::processEvents();
    for (const QPointer<QObject>& object : std::as_const(tracked))
    {
        QVERIFY(!object.isNull());
    }

    delete root;
    drainDeferredDeletes();
    for (const QPointer<QObject>& object : std::as_const(tracked))
    {
        QVERIFY(object.isNull());
    }

    antTheme->toggleThemeMode();
    QCoreApplication::processEvents();
}

void TestAntThemeLifecycle::openPopupsSurviveThemeSwitch()
{
    ThemeModeGuard guard;
    QWidget host;
    host.resize(640, 360);
    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    QPushButton target(QStringLiteral("Target"), &host);
    target.resize(120, 32);
    target.move(24, 24);
    target.show();

    AntSelect select(&host);
    select.addOption(QStringLiteral("Apple"), QStringLiteral("apple"));
    select.resize(180, select.sizeHint().height());
    select.move(24, 72);
    select.show();
    select.setOpen(true);
    QVERIFY(waitUntil([&]() { return select.isOpen() && directVisiblePopupChild(&select); }));

    AntCascader cascader(&host);
    cascader.setOptions({cascaderOption()});
    cascader.resize(180, cascader.sizeHint().height());
    cascader.move(220, 72);
    cascader.show();
    cascader.setOpen(true);
    QVERIFY(waitUntil([&]() { return cascader.isOpen() && directVisiblePopupChild(&cascader); }));

    AntColorPicker colorPicker(&host);
    colorPicker.resize(colorPicker.sizeHint());
    colorPicker.move(24, 128);
    colorPicker.show();
    colorPicker.setOpen(true);
    QVERIFY(waitUntil([&]() { return colorPicker.isOpen() && directVisiblePopupChild(&colorPicker); }));

    AntTreeSelect treeSelect(&host);
    treeSelect.setTreeData({treeNode()});
    treeSelect.resize(180, treeSelect.sizeHint().height());
    treeSelect.move(220, 128);
    treeSelect.show();
    treeSelect.setOpen(true);
    QVERIFY(waitUntil([&]() { return treeSelect.isOpen() && directVisiblePopupChild(&treeSelect); }));

    AntDropdown dropdown(&host);
    dropdown.setTarget(&target);
    dropdown.addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
    dropdown.setOpen(true);
    QVERIFY(waitUntil([&]() { return dropdown.isOpen() && directVisiblePopupChild(&dropdown); }));

    AntPopover popover(&host);
    popover.setTarget(&target);
    popover.setTitle(QStringLiteral("Title"));
    popover.setContent(QStringLiteral("Content"));
    popover.setOpen(true);
    QVERIFY(waitUntil([&]() { return popover.isOpen() && popover.isVisible(); }));

    AntToolTip tooltip(&host);
    tooltip.setTarget(&target);
    tooltip.setTitle(QStringLiteral("Tooltip"));
    tooltip.showTooltip();
    QVERIFY(waitUntil([&]() { return tooltip.isVisible(); }));

    antTheme->setThemeMode(guard.alternateMode());
    QCoreApplication::processEvents();

    QVERIFY(select.isOpen());
    QVERIFY(cascader.isOpen());
    QVERIFY(colorPicker.isOpen());
    QVERIFY(treeSelect.isOpen());
    QVERIFY(dropdown.isOpen());
    QVERIFY(popover.isOpen());
    QVERIFY(tooltip.isVisible());

    select.setOpen(false);
    cascader.setOpen(false);
    colorPicker.setOpen(false);
    treeSelect.setOpen(false);
    dropdown.setOpen(false);
    popover.setOpen(false);
    tooltip.hideTooltip();
}

void TestAntThemeLifecycle::styleThemeUpdateScopesToOwnedWidget()
{
    ThemeModeGuard guard;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    QCoreApplication::processEvents();

    QWidget host;
    host.resize(900, 640);

    QVector<AntTag*> tags;
    tags.reserve(96);
    for (int i = 0; i < 96; ++i)
    {
        auto* tag = new AntTag(QStringLiteral("Tag %1").arg(i), &host);
        tag->move(8 + (i % 12) * 72, 8 + (i / 12) * 32);
        tags.append(tag);
    }

    for (int i = 0; i < 240; ++i)
    {
        auto* decoy = new QWidget(&host);
        decoy->setObjectName(QStringLiteral("ThemeDecoy_%1").arg(i));
    }

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCoreApplication::processEvents();

    for (AntTag* tag : std::as_const(tags))
    {
        QObject* styleObject = tag->style();
        QVERIFY(styleObject != nullptr);
        QCOMPARE(styleObject->property("antStyleThemeUpdateCount").toInt(), 1);
        QCOMPARE(styleObject->property("antStyleThemeUsesGlobalWidgetScan").toBool(), false);
        QCOMPARE(styleObject->property("antStyleThemeCandidateCount").toInt(), 1);
        QCOMPARE(styleObject->property("antStyleThemeUpdatedWidgetCount").toInt(), 1);
        QVERIFY(tag->property("antStyleThemeSizeHintChanged").isValid());
    }

    auto* sharedStyle = new AntTagStyle(host.style());
    sharedStyle->setParent(&host);
    AntTag sharedA(QStringLiteral("Shared A"), &host);
    AntTag sharedB(QStringLiteral("Shared B"), &host);
    sharedA.setStyle(sharedStyle);
    sharedB.setStyle(sharedStyle);

    antTheme->setThemeMode(Ant::ThemeMode::Default);
    QCoreApplication::processEvents();

    QCOMPARE(sharedStyle->property("antStyleThemeUsesGlobalWidgetScan").toBool(), false);
    QCOMPARE(sharedStyle->property("antStyleThemeCandidateCount").toInt(), 2);
    QCOMPARE(sharedStyle->property("antStyleThemeUpdatedWidgetCount").toInt(), 2);
    QVERIFY(sharedA.property("antStyleThemeSizeHintChanged").isValid());
    QVERIFY(sharedB.property("antStyleThemeSizeHintChanged").isValid());

    ThemeStableProbe stableProbe(&host);
    auto* stableStyle = new ThemeStableProbeStyle(stableProbe.style());
    stableStyle->setParent(&stableProbe);
    stableProbe.setStyle(stableStyle);
    stableProbe.ensurePolished();
    QCOMPARE(stableProbe.sizeHint().height(), 24);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCoreApplication::processEvents();

    QCOMPARE(stableStyle->property("antStyleThemeUsesGlobalWidgetScan").toBool(), false);
    QCOMPARE(stableStyle->property("antStyleThemeCandidateCount").toInt(), 1);
    QCOMPARE(stableStyle->property("antStyleThemeUpdatedWidgetCount").toInt(), 1);
    QCOMPARE(stableProbe.property("antStyleThemeSizeHintChanged").toBool(), false);
    QCOMPARE(stableProbe.property("antStyleThemeUpdateGeometryCount").toInt(), 0);

    antTheme->setThemeMode(Ant::ThemeMode::Default);
    QCoreApplication::processEvents();

    ThemeMetricProbe metricProbe(&host);
    auto* metricStyle = new ThemeMetricProbeStyle(metricProbe.style());
    metricStyle->setParent(&metricProbe);
    metricProbe.setStyle(metricStyle);
    metricProbe.ensurePolished();
    metricProbe.show();
    QCOMPARE(metricProbe.sizeHint().height(), 24);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCoreApplication::processEvents();

    QCOMPARE(metricStyle->property("antStyleThemeUsesGlobalWidgetScan").toBool(), false);
    QCOMPARE(metricStyle->property("antStyleThemeCandidateCount").toInt(), 1);
    QCOMPARE(metricStyle->property("antStyleThemeUpdatedWidgetCount").toInt(), 1);
    QCOMPARE(metricProbe.property("antStyleThemeSizeHintChanged").toBool(), true);
    QCOMPARE(metricProbe.property("antStyleThemeUpdateGeometryCount").toInt(), 1);
}

QTEST_MAIN(TestAntThemeLifecycle)
#include "TestAntThemeLifecycle.moc"
