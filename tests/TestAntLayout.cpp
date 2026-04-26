#include <QSignalSpy>
#include <QTest>
#include "widgets/AntDivider.h"
#include "widgets/AntFlex.h"
#include "widgets/AntGrid.h"
#include "widgets/AntSpace.h"
#include "widgets/AntLayout.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntAffix.h"

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
    QCOMPARE(w->isPlain(), true);
    QCOMPARE(w->orientation(), Ant::Orientation::Horizontal);
    QCOMPARE(w->titlePlacement(), Ant::DividerTitlePlacement::Center);
    QCOMPARE(w->variant(), Ant::DividerVariant::Solid);
    QCOMPARE(w->dividerSize(), Ant::Size::Large);

    QSignalSpy textSpy(w, &AntDivider::textChanged);
    w->setText("OR");
    QCOMPARE(w->text(), "OR");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy plainSpy(w, &AntDivider::plainChanged);
    w->setPlain(false);
    QCOMPARE(w->isPlain(), false);
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

    auto* c1 = new QWidget;
    row->addWidget(c1, 12, 0);
    auto* c2 = new QWidget;
    row->addWidget(c2, 12, 0);
}

void TestAntLayout::space()
{
    auto* w = new AntSpace;
    QCOMPARE(w->orientation(), Ant::Orientation::Horizontal);
    QCOMPARE(w->size(), Ant::Size::Small);
    QCOMPARE(w->isWrap(), false);
    QCOMPARE(w->itemCount(), 0);

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

    auto* i1 = new QWidget;
    auto* i2 = new QWidget;
    w->addItem(i1);
    w->addItem(i2);
    QCOMPARE(w->itemCount(), 2);
    QCOMPARE(w->itemAt(0), i1);

    w->clearItems();
    QCOMPARE(w->itemCount(), 0);
}

void TestAntLayout::layout()
{
    auto* w = new AntLayout;
    QCOMPARE(w->hasSider(), false);

    auto* header = new AntLayoutHeader;
    w->setHeader(header);
    QCOMPARE(w->header(), header);

    auto* footer = new AntLayoutFooter;
    w->setFooter(footer);
    QCOMPARE(w->footer(), footer);

    auto* content = new AntLayoutContent;
    w->setContent(content);
    QCOMPARE(w->content(), content);
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
    auto* i2 = new QWidget;
    w->addWidget(i1);
    w->addWidget(i2);
    w->clear();
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
}

QTEST_MAIN(TestAntLayout)
#include "TestAntLayout.moc"
