#include <QSignalSpy>
#include <QCoreApplication>
#include <QGridLayout>
#include <QImage>
#include <QPainter>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QScrollArea>
#include <QScrollBar>
#include <QTest>
#include <QVBoxLayout>
#include "core/AntTheme.h"
#include "widgets/AntDivider.h"
#include "widgets/AntFlex.h"
#include "widgets/AntGrid.h"
#include "widgets/AntSpace.h"
#include "widgets/AntLayout.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntAffix.h"

namespace
{
class ThemeModeRestorerForLayoutTest
{
public:
    ThemeModeRestorerForLayoutTest()
        : m_originalMode(antTheme->themeMode())
    {
    }

    ~ThemeModeRestorerForLayoutTest()
    {
        antTheme->setThemeMode(m_originalMode);
        QCoreApplication::processEvents();
    }

private:
    Ant::ThemeMode m_originalMode;
};
} // namespace

class TestAntLayout : public QObject
{
    Q_OBJECT
private slots:
    void divider();
    void flex();
    void grid();
    void space();
    void layout();
    void sider();
    void masonry();
    void affix();
};

void TestAntLayout::divider()
{
    auto* w = new AntDivider;
    QCOMPARE(w->text(), QString());
    QCOMPARE(w->isPlain(), false);
    QCOMPARE(w->orientation(), Ant::Orientation::Horizontal);
    QCOMPARE(w->titlePlacement(), Ant::DividerTitlePlacement::Center);
    QCOMPARE(w->variant(), Ant::DividerVariant::Solid);
    QCOMPARE(w->dividerSize(), Ant::Size::Large);

    QSignalSpy textSpy(w, &AntDivider::textChanged);
    w->setText("OR");
    QCOMPARE(w->text(), "OR");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy plainSpy(w, &AntDivider::plainChanged);
    w->setPlain(true);
    QCOMPARE(w->isPlain(), true);
    QCOMPARE(plainSpy.count(), 1);

    QSignalSpy orientSpy(w, &AntDivider::orientationChanged);
    w->setOrientation(Ant::Orientation::Vertical);
    QCOMPARE(w->orientation(), Ant::Orientation::Vertical);
    QCOMPARE(orientSpy.count(), 1);

    QSignalSpy placeSpy(w, &AntDivider::titlePlacementChanged);
    w->setTitlePlacement(Ant::DividerTitlePlacement::Start);
    QCOMPARE(w->titlePlacement(), Ant::DividerTitlePlacement::Start);
    QCOMPARE(placeSpy.count(), 1);

    QSignalSpy varSpy(w, &AntDivider::variantChanged);
    w->setVariant(Ant::DividerVariant::Dashed);
    QCOMPARE(w->variant(), Ant::DividerVariant::Dashed);
    QCOMPARE(varSpy.count(), 1);

    QSignalSpy sizeSpy(w, &AntDivider::dividerSizeChanged);
    w->setDividerSize(Ant::Size::Small);
    QCOMPARE(w->dividerSize(), Ant::Size::Small);
    QCOMPARE(sizeSpy.count(), 1);

    auto* w2 = new AntDivider("Text");
    QCOMPARE(w2->text(), "Text");

    AntDivider cached(QStringLiteral("Cached"));
    cached.resize(260, 36);
    QImage image(cached.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        cached.render(&painter);
    }
    QCOMPARE(cached.property("antDividerPaintCacheBuildCount").toInt(), 1);

    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        cached.render(&painter);
    }
    QCOMPARE(cached.property("antDividerPaintCacheBuildCount").toInt(), 1);

    cached.setTitlePlacement(Ant::DividerTitlePlacement::End);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        cached.render(&painter);
    }
    QCOMPARE(cached.property("antDividerPaintCacheBuildCount").toInt(), 2);

    cached.setText(QStringLiteral("Cached divider"));
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        cached.render(&painter);
    }
    QCOMPARE(cached.property("antDividerPaintCacheBuildCount").toInt(), 3);
}

void TestAntLayout::flex()
{
    auto* w = new AntFlex;
    QCOMPARE(w->vertical(), false);
    QCOMPARE(w->gap(), 8);
    QCOMPARE(w->wrap(), false);

    QSignalSpy vertSpy(w, &AntFlex::verticalChanged);
    w->setVertical(true);
    QCOMPARE(w->vertical(), true);
    QCOMPARE(vertSpy.count(), 1);

    QSignalSpy gapSpy(w, &AntFlex::gapChanged);
    w->setGap(16);
    QCOMPARE(w->gap(), 16);
    QCOMPARE(gapSpy.count(), 1);

    QSignalSpy wrapSpy(w, &AntFlex::wrapChanged);
    w->setWrap(true);
    QCOMPARE(w->wrap(), true);
    QCOMPARE(wrapSpy.count(), 1);

    auto* child = new QWidget;
    w->addWidget(child);
    w->addStretch();

    AntFlex cachedFlex;
    cachedFlex.setWrap(true);
    cachedFlex.resize(90, 90);

    auto* first = new QWidget;
    first->setFixedSize(40, 20);
    cachedFlex.addWidget(first);

    auto* second = new QWidget;
    second->setFixedSize(40, 20);
    cachedFlex.addWidget(second);

    auto* third = new QWidget;
    third->setFixedSize(40, 20);
    cachedFlex.addWidget(third);

    cachedFlex.layout()->setGeometry(cachedFlex.rect());
    QCOMPARE(cachedFlex.property("antFlexLayoutBuildCount").toInt(), 1);
    QCOMPARE(third->geometry().top(), 28);

    cachedFlex.layout()->setGeometry(cachedFlex.rect());
    QCOMPARE(cachedFlex.property("antFlexLayoutBuildCount").toInt(), 1);

    const QSize firstHint = cachedFlex.sizeHint();
    QCOMPARE(cachedFlex.property("antFlexSizeHintBuildCount").toInt(), 1);
    QCOMPARE(cachedFlex.sizeHint(), firstHint);
    QCOMPARE(cachedFlex.property("antFlexSizeHintBuildCount").toInt(), 1);

    cachedFlex.setGap(12);
    const QSize secondHint = cachedFlex.sizeHint();
    QCOMPARE(cachedFlex.property("antFlexSizeHintBuildCount").toInt(), 2);
    QVERIFY(secondHint.width() > firstHint.width());

    const int layoutBuildsBeforeGapGeometry = cachedFlex.property("antFlexLayoutBuildCount").toInt();
    cachedFlex.layout()->setGeometry(cachedFlex.rect());
    QCOMPARE(cachedFlex.property("antFlexLayoutBuildCount").toInt(), layoutBuildsBeforeGapGeometry + 1);
    QCOMPARE(third->geometry().top(), 64);

    const int layoutBuildsAfterGapGeometry = cachedFlex.property("antFlexLayoutBuildCount").toInt();
    cachedFlex.layout()->setGeometry(cachedFlex.rect());
    QCOMPARE(cachedFlex.property("antFlexLayoutBuildCount").toInt(), layoutBuildsAfterGapGeometry);

    QCOMPARE(cachedFlex.layout()->heightForWidth(90), 84);
    QCOMPARE(cachedFlex.property("antFlexLayoutBuildCount").toInt(), layoutBuildsAfterGapGeometry + 1);
    QCOMPARE(cachedFlex.layout()->heightForWidth(90), 84);
    QCOMPARE(cachedFlex.property("antFlexLayoutBuildCount").toInt(), layoutBuildsAfterGapGeometry + 1);
}

void TestAntLayout::grid()
{
    // AntCol
    auto* col = new AntCol(12);
    QCOMPARE(col->span(), 12);
    QCOMPARE(col->offset(), 0);

    QSignalSpy spanSpy(col, &AntCol::spanChanged);
    col->setSpan(8);
    QCOMPARE(col->span(), 8);
    QCOMPARE(spanSpy.count(), 1);

    QSignalSpy offSpy(col, &AntCol::offsetChanged);
    col->setOffset(4);
    QCOMPARE(col->offset(), 4);
    QCOMPARE(offSpy.count(), 1);

    // AntRow
    auto* row = new AntRow;
    QCOMPARE(row->gutter(), 0);

    QSignalSpy gutSpy(row, &AntRow::gutterChanged);
    row->setGutter(16);
    QCOMPARE(row->gutter(), 16);
    QCOMPARE(gutSpy.count(), 1);
    QCOMPARE(row->property("antGridPlacementBuildCount").toInt(), 0);
    QCOMPARE(row->property("antGridRelayoutCount").toInt(), 0);
    QCOMPARE(row->property("antGridColumnStretchInitialized").toBool(), true);

    auto* c1 = new QWidget;
    row->addWidget(c1, 12, 0);
    QCOMPARE(row->property("antGridPlacementBuildCount").toInt(), 1);
    QCOMPARE(row->property("antGridRelayoutCount").toInt(), 0);
    auto* c2 = new QWidget;
    row->addWidget(c2, 12, 0);
    QCOMPARE(row->property("antGridPlacementBuildCount").toInt(), 2);
    QCOMPARE(row->property("antGridRelayoutCount").toInt(), 0);

    auto* grid = qobject_cast<QGridLayout*>(row->layout());
    QVERIFY(grid);
    auto* col1 = qobject_cast<AntCol*>(c1->parentWidget());
    auto* col2 = qobject_cast<AntCol*>(c2->parentWidget());
    QVERIFY(col1);
    QVERIFY(col2);

    int itemRow = -1;
    int itemColumn = -1;
    int rowSpan = -1;
    int columnSpan = -1;
    grid->getItemPosition(grid->indexOf(col1), &itemRow, &itemColumn, &rowSpan, &columnSpan);
    QCOMPARE(itemRow, 0);
    QCOMPARE(itemColumn, 0);
    QCOMPARE(rowSpan, 1);
    QCOMPARE(columnSpan, 12);

    grid->getItemPosition(grid->indexOf(col2), &itemRow, &itemColumn, &rowSpan, &columnSpan);
    QCOMPARE(itemRow, 0);
    QCOMPARE(itemColumn, 12);
    QCOMPARE(rowSpan, 1);
    QCOMPARE(columnSpan, 12);

    const int placementBuilds = row->property("antGridPlacementBuildCount").toInt();
    row->setGutter(20);
    QCOMPARE(row->gutter(), 20);
    QCOMPARE(gutSpy.count(), 2);
    QCOMPARE(row->property("antGridPlacementBuildCount").toInt(), placementBuilds);
    QCOMPARE(row->property("antGridRelayoutCount").toInt(), 0);
}

void TestAntLayout::space()
{
    auto* w = new AntSpace;
    QCOMPARE(w->orientation(), Ant::Orientation::Horizontal);
    QCOMPARE(w->size(), Ant::Size::Small);
    QCOMPARE(w->isWrap(), false);
    QCOMPARE(w->itemCount(), 0);
    QCOMPARE(w->sizePolicy().horizontalPolicy(), QSizePolicy::Fixed);
    QCOMPARE(w->sizePolicy().verticalPolicy(), QSizePolicy::Fixed);

    QSignalSpy orientSpy(w, &AntSpace::orientationChanged);
    w->setOrientation(Ant::Orientation::Vertical);
    QCOMPARE(w->orientation(), Ant::Orientation::Vertical);
    QCOMPARE(orientSpy.count(), 1);

    QSignalSpy sizeSpy(w, &AntSpace::sizeChanged);
    w->setSize(Ant::Size::Large);
    QCOMPARE(w->size(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy wrapSpy(w, &AntSpace::wrapChanged);
    w->setWrap(true);
    QCOMPARE(w->isWrap(), true);
    QCOMPARE(wrapSpy.count(), 1);
    const int rebuildsAfterWrap = w->property("antSpaceRebuildCount").toInt();

    auto* i1 = new QWidget;
    i1->setMinimumSize(28, 12);
    auto* i2 = new QWidget;
    i2->setMinimumSize(32, 16);
    w->addItem(i1);
    QCOMPARE(w->property("antSpaceRebuildCount").toInt(), rebuildsAfterWrap);
    QCOMPARE(w->property("antSpaceIncrementalAddCount").toInt(), 1);
    QCOMPARE(i1->parentWidget(), w);

    w->addItem(i2);
    QCOMPARE(w->property("antSpaceRebuildCount").toInt(), rebuildsAfterWrap);
    QCOMPARE(w->property("antSpaceIncrementalAddCount").toInt(), 2);
    QCOMPARE(i2->parentWidget(), w);
    QCOMPARE(w->itemCount(), 2);
    QCOMPARE(w->itemAt(0), i1);

    const int sizeHintComputes = w->property("antSpaceSizeHintComputeCount").toInt();
    const QSize firstHint = w->sizeHint();
    const QSize secondHint = w->sizeHint();
    QCOMPARE(firstHint, secondHint);
    QCOMPARE(w->property("antSpaceSizeHintComputeCount").toInt(), sizeHintComputes + 1);
    QCOMPARE(w->property("antSpaceSizeHintDirty").toBool(), true);

    const int minHintComputes = w->property("antSpaceMinimumSizeHintComputeCount").toInt();
    const QSize firstMinHint = w->minimumSizeHint();
    const QSize secondMinHint = w->minimumSizeHint();
    QCOMPARE(firstMinHint, secondMinHint);
    QCOMPARE(w->property("antSpaceMinimumSizeHintComputeCount").toInt(), minHintComputes + 1);
    QCOMPARE(w->property("antSpaceSizeHintDirty").toBool(), false);

    QEvent layoutRequest(QEvent::LayoutRequest);
    QCoreApplication::sendEvent(i2, &layoutRequest);
    QCOMPARE(w->property("antSpaceSizeHintDirty").toBool(), true);
    w->sizeHint();
    QCOMPARE(w->property("antSpaceSizeHintComputeCount").toInt(), sizeHintComputes + 2);

    const int spacingUpdates = w->property("antSpaceSpacingUpdateCount").toInt();
    w->setCustomSpacing(12);
    QCOMPARE(w->customSpacing(), 12);
    QCOMPARE(w->property("antSpaceSpacingUpdateCount").toInt(), spacingUpdates + 1);
    w->setCustomSpacing(12);
    QCOMPARE(w->property("antSpaceSpacingUpdateCount").toInt(), spacingUpdates + 1);

    auto* i0 = new QWidget;
    const int rebuildsBeforeInsert = w->property("antSpaceRebuildCount").toInt();
    w->insertItem(0, i0);
    QCOMPARE(w->itemAt(0), i0);
    QCOMPARE(w->property("antSpaceRebuildCount").toInt(), rebuildsBeforeInsert + 1);

    w->clearItems();
    QCOMPARE(w->itemCount(), 0);
    QCOMPARE(w->property("antSpaceSeparatorCopyCount").toInt(), 0);
}

void TestAntLayout::layout()
{
    auto* w = new AntLayout;
    w->resize(640, 360);
    QCOMPARE(w->hasSider(), false);
    QCOMPARE(w->borderRadius(), 0);

    QSignalSpy radiusSpy(w, &AntLayout::borderRadiusChanged);
    w->setBorderRadius(8);
    QCOMPARE(w->borderRadius(), 8);
    QCOMPARE(radiusSpy.count(), 1);

    auto* header = new AntLayoutHeader;
    w->setHeader(header);
    QCOMPARE(w->header(), header);

    auto* footer = new AntLayoutFooter;
    w->setFooter(footer);
    QCOMPARE(w->footer(), footer);

    auto* content = new AntLayoutContent;
    w->setContent(content);
    QCOMPARE(w->content(), content);

    auto* sider = new AntLayoutSider;
    sider->setWidth(180);
    sider->setCollapsedWidth(72);

    QSignalSpy hasSiderSpy(w, &AntLayout::hasSiderChanged);
    w->addSider(sider);
    QCOMPARE(w->hasSider(), true);
    QCOMPARE(hasSiderSpy.count(), 1);
    QCOMPARE(w->siderCount(), 1);
    QCOMPARE(w->siderAt(0), sider);

    QCOMPARE(header->geometry(), QRect(0, 0, 640, 64));
    QCOMPARE(footer->geometry(), QRect(0, 290, 640, 70));
    QCOMPARE(sider->geometry(), QRect(0, 64, 180, 226));
    QCOMPARE(content->geometry(), QRect(180, 64, 460, 226));

    ThemeModeRestorerForLayoutTest restoreTheme;
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    QCoreApplication::processEvents();
    const int headerThemeGeometryUpdates = header->property("antThemeRefreshUpdateGeometryCount").toInt();
    const int footerThemeGeometryUpdates = footer->property("antThemeRefreshUpdateGeometryCount").toInt();
    const int contentThemeGeometryUpdates = content->property("antThemeRefreshUpdateGeometryCount").toInt();
    const int siderThemeGeometryUpdates = sider->property("antThemeRefreshUpdateGeometryCount").toInt();
    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCoreApplication::processEvents();
    QCOMPARE(header->property("antThemeRefreshSizeHintChanged").toBool(), false);
    QCOMPARE(footer->property("antThemeRefreshSizeHintChanged").toBool(), false);
    QCOMPARE(content->property("antThemeRefreshSizeHintChanged").toBool(), false);
    QCOMPARE(sider->property("antThemeRefreshSizeHintChanged").toBool(), false);
    QCOMPARE(header->property("antThemeRefreshUpdateGeometryCount").toInt(), headerThemeGeometryUpdates);
    QCOMPARE(footer->property("antThemeRefreshUpdateGeometryCount").toInt(), footerThemeGeometryUpdates);
    QCOMPARE(content->property("antThemeRefreshUpdateGeometryCount").toInt(), contentThemeGeometryUpdates);
    QCOMPARE(sider->property("antThemeRefreshUpdateGeometryCount").toInt(), siderThemeGeometryUpdates);

    const int syncCount = w->property("antLayoutSyncCount").toInt();
    const int applyCount = w->property("antLayoutGeometryApplyCount").toInt();
    QResizeEvent sameSizeEvent(QSize(640, 360), QSize(640, 360));
    QCoreApplication::sendEvent(w, &sameSizeEvent);
    QCOMPARE(w->property("antLayoutSyncCount").toInt(), syncCount);
    QCOMPARE(w->property("antLayoutGeometryApplyCount").toInt(), applyCount);

    w->resize(800, 360);
    QResizeEvent widerEvent(QSize(800, 360), QSize(640, 360));
    QCoreApplication::sendEvent(w, &widerEvent);
    QVERIFY(w->property("antLayoutSyncCount").toInt() > syncCount);
    QCOMPARE(header->geometry(), QRect(0, 0, 800, 64));
    QCOMPARE(footer->geometry(), QRect(0, 290, 800, 70));
    QCOMPARE(sider->geometry(), QRect(0, 64, 180, 226));
    QCOMPARE(content->geometry(), QRect(180, 64, 620, 226));

    sider->setCollapsed(true);
    QCOMPARE(sider->geometry(), QRect(0, 64, 72, 226));
    QCOMPARE(content->geometry(), QRect(72, 64, 728, 226));
}

void TestAntLayout::sider()
{
    auto* w = new AntLayoutSider;
    QCOMPARE(w->isCollapsed(), false);
    QCOMPARE(w->isCollapsible(), false);
    QCOMPARE(w->siderWidth(), 200);
    QCOMPARE(w->collapsedWidth(), 80);
    QCOMPARE(w->siderTheme(), Ant::LayoutSiderTheme::Dark);
    QCOMPARE(w->isReverseArrow(), false);

    QSignalSpy colSpy(w, &AntLayoutSider::collapsedChanged);
    w->setCollapsed(true);
    QCOMPARE(w->isCollapsed(), true);
    QCOMPARE(colSpy.count(), 1);

    QSignalSpy collSpy(w, &AntLayoutSider::collapsibleChanged);
    w->setCollapsible(true);
    QCOMPARE(w->isCollapsible(), true);
    QCOMPARE(collSpy.count(), 1);

    QSignalSpy wSpy(w, &AntLayoutSider::widthChanged);
    w->setWidth(250);
    QCOMPARE(w->siderWidth(), 250);
    QCOMPARE(wSpy.count(), 1);

    QSignalSpy cwSpy(w, &AntLayoutSider::collapsedWidthChanged);
    w->setCollapsedWidth(60);
    QCOMPARE(w->collapsedWidth(), 60);
    QCOMPARE(cwSpy.count(), 1);

    QSignalSpy themeSpy(w, &AntLayoutSider::siderThemeChanged);
    w->setSiderTheme(Ant::LayoutSiderTheme::Light);
    QCOMPARE(w->siderTheme(), Ant::LayoutSiderTheme::Light);
    QCOMPARE(themeSpy.count(), 1);

    QSignalSpy revSpy(w, &AntLayoutSider::reverseArrowChanged);
    w->setReverseArrow(true);
    QCOMPARE(w->isReverseArrow(), true);
    QCOMPARE(revSpy.count(), 1);
}

void TestAntLayout::masonry()
{
    auto* w = new AntMasonry;
    w->resize(416, 300);
    QCOMPARE(w->columns(), 3);
    QCOMPARE(w->spacing(), 8);

    QSignalSpy colSpy(w, &AntMasonry::columnsChanged);
    w->setColumns(4);
    QCOMPARE(w->columns(), 4);
    QCOMPARE(colSpy.count(), 1);

    QSignalSpy spSpy(w, &AntMasonry::spacingChanged);
    w->setSpacing(16);
    QCOMPARE(w->spacing(), 16);
    QCOMPARE(spSpy.count(), 1);

    auto* i1 = new QWidget;
    i1->setMinimumHeight(80);
    auto* i2 = new QWidget;
    i2->setMinimumHeight(40);
    auto* i3 = new QWidget;
    i3->setMinimumHeight(60);

    w->addWidget(i1);
    QCOMPARE(w->property("antMasonryFullRelayoutCount").toInt(), 1);
    QCOMPARE(w->property("antMasonryIncrementalLayoutCount").toInt(), 0);
    QCOMPARE(w->property("antMasonryCachedColumnWidth").toInt(), 92);
    QCOMPARE(i1->geometry(), QRect(0, 0, 92, 80));

    w->addWidget(i2);
    QCOMPARE(w->property("antMasonryFullRelayoutCount").toInt(), 1);
    QCOMPARE(w->property("antMasonryIncrementalLayoutCount").toInt(), 1);
    QCOMPARE(i2->geometry(), QRect(108, 0, 92, 40));

    w->addWidget(i3);
    QCOMPARE(w->property("antMasonryFullRelayoutCount").toInt(), 1);
    QCOMPARE(w->property("antMasonryIncrementalLayoutCount").toInt(), 2);
    QCOMPARE(i3->geometry(), QRect(216, 0, 92, 60));

    const int fullRelayouts = w->property("antMasonryFullRelayoutCount").toInt();
    QResizeEvent sameWidthEvent(QSize(416, 260), QSize(416, 300));
    QCoreApplication::sendEvent(w, &sameWidthEvent);
    QCOMPARE(w->property("antMasonryFullRelayoutCount").toInt(), fullRelayouts);

    w->resize(500, 260);
    QResizeEvent widerEvent(QSize(500, 260), QSize(416, 260));
    QCoreApplication::sendEvent(w, &widerEvent);
    QCOMPARE(w->property("antMasonryFullRelayoutCount").toInt(), fullRelayouts + 1);
    QCOMPARE(w->property("antMasonryCachedColumnWidth").toInt(), 113);
    QCOMPARE(i1->geometry(), QRect(0, 0, 113, 80));
    QCOMPARE(i2->geometry(), QRect(129, 0, 113, 40));
    QCOMPARE(i3->geometry(), QRect(258, 0, 113, 60));

    w->clear();
    QCOMPARE(w->minimumHeight(), 0);
}

void TestAntLayout::affix()
{
    auto* w = new AntAffix;
    QCOMPARE(w->offsetTop(), 0);
    QCOMPARE(w->offsetBottom(), 0);
    QCOMPARE(w->isAffixed(), false);

    QSignalSpy topSpy(w, &AntAffix::offsetTopChanged);
    w->setOffsetTop(100);
    QCOMPARE(w->offsetTop(), 100);
    QCOMPARE(topSpy.count(), 1);

    QSignalSpy botSpy(w, &AntAffix::offsetBottomChanged);
    w->setOffsetBottom(50);
    QCOMPARE(w->offsetBottom(), 50);
    QCOMPARE(botSpy.count(), 1);

    QScrollArea area;
    auto* content = new QWidget;
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    auto* header = new QWidget;
    header->setFixedHeight(32);
    auto* filler = new QWidget;
    filler->setMinimumHeight(420);
    layout->addWidget(header);
    layout->addWidget(filler);
    area.setWidget(content);
    area.setWidgetResizable(true);
    area.resize(220, 120);
    area.show();
    QVERIFY(QTest::qWaitForWindowExposed(&area));

    AntAffix affix;
    affix.setAffixedWidget(header);
    affix.setScrollTarget(&area);
    affix.setOffsetTop(0);
    QCoreApplication::processEvents();

    QCOMPARE(affix.property("antAffixUsesQueuedChecks").toBool(), true);
    const int checksBeforeScroll = affix.property("antAffixCheckCount").toInt();
    area.verticalScrollBar()->setValue(24);
    area.verticalScrollBar()->setValue(48);
    area.verticalScrollBar()->setValue(72);
    QCOMPARE(affix.property("antAffixCheckQueued").toBool(), true);
    QCOMPARE(affix.property("antAffixQueuedCheckCoalesced").toBool(), true);
    QCoreApplication::processEvents();
    QCOMPARE(affix.isAffixed(), true);
    QCOMPARE(affix.property("antAffixCheckCount").toInt(), checksBeforeScroll + 1);

    const int effectiveChecks = affix.property("antAffixEffectiveCheckCount").toInt();
    QEvent moveEvent(QEvent::Move);
    QCoreApplication::sendEvent(area.viewport(), &moveEvent);
    QCoreApplication::sendEvent(area.viewport(), &moveEvent);
    QCoreApplication::processEvents();
    QCOMPARE(affix.property("antAffixLastCheckSkipped").toBool(), true);
    QCOMPARE(affix.property("antAffixEffectiveCheckCount").toInt(), effectiveChecks);
}

QTEST_MAIN(TestAntLayout)
#include "TestAntLayout.moc"
