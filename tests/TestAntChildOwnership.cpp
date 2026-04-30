#include <QCoreApplication>
#include <QEvent>
#include <QPointer>
#include <QTest>
#include <QWidget>

#include "widgets/AntAlert.h"
#include "widgets/AntBadge.h"
#include "widgets/AntCard.h"
#include "widgets/AntCarousel.h"
#include "widgets/AntCollapse.h"
#include "widgets/AntDescriptions.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntEmpty.h"
#include "widgets/AntFlex.h"
#include "widgets/AntForm.h"
#include "widgets/AntGrid.h"
#include "widgets/AntInput.h"
#include "widgets/AntLayout.h"
#include "widgets/AntList.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntModal.h"
#include "widgets/AntPopover.h"
#include "widgets/AntResult.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntSkeleton.h"
#include "widgets/AntSpace.h"
#include "widgets/AntSplitter.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntTabs.h"
#include "widgets/AntWindow.h"

class TestAntChildOwnership : public QObject
{
    Q_OBJECT

private slots:
    void assignedChildWidgetsAreOwnedByHostTree();
};

namespace
{
QWidget* childWidget(const char* name)
{
    auto* widget = new QWidget;
    widget->setObjectName(QString::fromLatin1(name));
    widget->setMinimumSize(24, 16);
    return widget;
}

void drainDeferredDeletes()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QCoreApplication::processEvents();
}

bool isDescendantOf(QObject* child, QObject* ancestor)
{
    for (QObject* parent = child ? child->parent() : nullptr; parent; parent = parent->parent())
    {
        if (parent == ancestor)
        {
            return true;
        }
    }
    return false;
}

void verifyOwnedChildren(QWidget* owner, const QList<QWidget*>& children, const char* name)
{
    QVERIFY2(owner != nullptr, name);
    QList<QPointer<QWidget>> tracked;
    for (QWidget* child : children)
    {
        QVERIFY2(child != nullptr, name);
        QVERIFY2(isDescendantOf(child, owner), name);
        tracked.append(QPointer<QWidget>(child));
    }

    delete owner;
    drainDeferredDeletes();

    for (const QPointer<QWidget>& child : std::as_const(tracked))
    {
        QVERIFY2(child.isNull(), name);
    }
}
} // namespace

void TestAntChildOwnership::assignedChildWidgetsAreOwnedByHostTree()
{
    {
        auto* owner = new AntAlert;
        auto* action = childWidget("alert-action");
        owner->setActionWidget(action);
        verifyOwnedChildren(owner, {action}, "AntAlert::setActionWidget");
    }
    {
        auto* owner = new AntBadge;
        auto* content = childWidget("badge-content");
        owner->setContentWidget(content);
        verifyOwnedChildren(owner, {content}, "AntBadge::setContentWidget");
    }
    {
        auto* owner = new AntCard;
        auto* body = childWidget("card-body");
        owner->setBodyWidget(body);
        verifyOwnedChildren(owner, {body}, "AntCard::setBodyWidget");
    }
    {
        auto* owner = new AntCard;
        auto* cover = childWidget("card-cover");
        auto* action = childWidget("card-action");
        auto* avatar = childWidget("card-avatar");
        auto* grid = childWidget("card-grid");
        owner->setCoverWidget(cover);
        owner->addActionWidget(action);
        owner->setMetaAvatar(avatar);
        owner->addGridItem(grid);
        verifyOwnedChildren(owner, {cover, action, avatar, grid}, "AntCard composite children");
    }
    {
        auto* owner = new AntCarousel;
        auto* slide = childWidget("carousel-slide");
        owner->addSlide(slide);
        verifyOwnedChildren(owner, {slide}, "AntCarousel::addSlide");
    }
    {
        auto* owner = new AntCollapsePanel(QStringLiteral("Panel"));
        auto* content = childWidget("collapse-content");
        owner->setContentWidget(content);
        verifyOwnedChildren(owner, {content}, "AntCollapsePanel::setContentWidget");
    }
    {
        auto* owner = new AntDescriptionsItem;
        auto* content = childWidget("descriptions-content");
        owner->setContentWidget(content);
        verifyOwnedChildren(owner, {content}, "AntDescriptionsItem::setContentWidget");
    }
    {
        auto* owner = new AntDockWidget(QStringLiteral("Dock"));
        auto* content = childWidget("dock-content");
        owner->setWidget(content);
        verifyOwnedChildren(owner, {content}, "AntDockWidget::setWidget");
    }
    {
        auto* owner = new AntDrawer;
        auto* body = childWidget("drawer-body");
        owner->setBodyWidget(body);
        verifyOwnedChildren(owner, {body}, "AntDrawer::setBodyWidget");
    }
    {
        auto* owner = new AntEmpty;
        auto* extra = childWidget("empty-extra");
        owner->setExtraWidget(extra);
        verifyOwnedChildren(owner, {extra}, "AntEmpty::setExtraWidget");
    }
    {
        auto* owner = new AntFlex;
        auto* item = childWidget("flex-item");
        owner->addWidget(item);
        verifyOwnedChildren(owner, {item}, "AntFlex::addWidget");
    }
    {
        auto* owner = new AntFormItem;
        auto* field = childWidget("form-field");
        owner->setFieldWidget(field);
        verifyOwnedChildren(owner, {field}, "AntFormItem::setFieldWidget");
    }
    {
        auto* owner = new AntForm;
        auto* field = childWidget("form-add-field");
        owner->addItem(QStringLiteral("Name"), field);
        verifyOwnedChildren(owner, {field}, "AntForm::addItem");
    }
    {
        auto* owner = new AntRow;
        auto* col = childWidget("grid-col");
        owner->addWidget(col, 12, 0);
        verifyOwnedChildren(owner, {col}, "AntRow::addWidget");
    }
    {
        auto* owner = new AntInput;
        auto* prefix = childWidget("input-prefix");
        auto* suffix = childWidget("input-suffix");
        owner->setPrefixWidget(prefix);
        owner->setSuffixWidget(suffix);
        verifyOwnedChildren(owner, {prefix, suffix}, "AntInput prefix/suffix");
    }
    {
        auto* owner = new AntLayoutSider;
        auto* trigger = childWidget("layout-trigger");
        owner->setTriggerWidget(trigger);
        verifyOwnedChildren(owner, {trigger}, "AntLayoutSider::setTriggerWidget");
    }
    {
        auto* owner = new AntLayout;
        auto* header = new AntLayoutHeader;
        auto* content = new AntLayoutContent;
        auto* footer = new AntLayoutFooter;
        auto* sider = new AntLayoutSider;
        owner->setHeader(header);
        owner->setContent(content);
        owner->setFooter(footer);
        owner->addSider(sider);
        verifyOwnedChildren(owner, {header, content, footer, sider}, "AntLayout regions");
    }
    {
        auto* owner = new AntListItemMeta;
        auto* avatar = childWidget("list-meta-avatar");
        owner->setAvatar(avatar);
        verifyOwnedChildren(owner, {avatar}, "AntListItemMeta::setAvatar");
    }
    {
        auto* owner = new AntListItem;
        auto* content = childWidget("list-item-content");
        auto* extra = childWidget("list-item-extra");
        auto* action = childWidget("list-item-action");
        owner->setContentWidget(content);
        owner->setExtraWidget(extra);
        owner->addActionWidget(action);
        verifyOwnedChildren(owner, {content, extra, action}, "AntListItem widgets");
    }
    {
        auto* owner = new AntList;
        auto* header = childWidget("list-header");
        auto* footer = childWidget("list-footer");
        auto* item = new AntListItem;
        owner->setHeaderWidget(header);
        owner->setFooterWidget(footer);
        owner->addItem(item);
        verifyOwnedChildren(owner, {header, footer, item}, "AntList widgets");
    }
    {
        auto* owner = new AntMasonry;
        owner->resize(120, 120);
        auto* item = childWidget("masonry-item");
        owner->addWidget(item);
        verifyOwnedChildren(owner, {item}, "AntMasonry::addWidget");
    }
    {
        auto* owner = new AntModal;
        auto* content = childWidget("modal-content");
        auto* footer = childWidget("modal-footer");
        owner->setContentWidget(content);
        owner->setFooterWidget(footer);
        verifyOwnedChildren(owner, {content, footer}, "AntModal content/footer");
    }
    {
        auto* owner = new AntPopover;
        auto* action = childWidget("popover-action");
        owner->setActionWidget(action);
        verifyOwnedChildren(owner, {action}, "AntPopover::setActionWidget");
    }
    {
        auto* owner = new AntResult;
        auto* extra = childWidget("result-extra");
        owner->setExtraWidget(extra);
        verifyOwnedChildren(owner, {extra}, "AntResult::setExtraWidget");
    }
    {
        auto* owner = new AntScrollArea;
        auto* content = childWidget("scroll-content");
        owner->setWidget(content);
        verifyOwnedChildren(owner, {content}, "AntScrollArea::setWidget");
    }
    {
        auto* owner = new AntSkeleton;
        auto* content = childWidget("skeleton-content");
        owner->setContentWidget(content);
        verifyOwnedChildren(owner, {content}, "AntSkeleton::setContentWidget");
    }
    {
        auto* owner = new AntSpace;
        auto* item = childWidget("space-item");
        owner->addItem(item);
        verifyOwnedChildren(owner, {item}, "AntSpace::addItem");
    }
    {
        auto* owner = new AntSplitter;
        auto* left = childWidget("splitter-left");
        auto* right = childWidget("splitter-right");
        owner->addWidget(left);
        owner->addWidget(right);
        verifyOwnedChildren(owner, {left, right}, "AntSplitter::addWidget");
    }
    {
        auto* owner = new AntStatistic;
        auto* value = childWidget("statistic-value");
        owner->setValueWidget(value);
        verifyOwnedChildren(owner, {value}, "AntStatistic::setValueWidget");
    }
    {
        auto* owner = new AntTabs;
        auto* page = childWidget("tabs-page");
        owner->addTab(page, QStringLiteral("page"), QStringLiteral("Page"));
        verifyOwnedChildren(owner, {page}, "AntTabs::addTab");
    }
    {
        auto* owner = new AntWindow;
        auto* central = childWidget("window-central");
        owner->setCentralWidget(central);
        verifyOwnedChildren(owner, {central}, "AntWindow::setCentralWidget");
    }
}

QTEST_MAIN(TestAntChildOwnership)
#include "TestAntChildOwnership.moc"
