#include <QColor>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QSignalSpy>
#include <QTest>
#include <QWidget>

#include <functional>

#include "widgets/AntAffix.h"
#include "widgets/AntAlert.h"
#include "widgets/AntAnchor.h"
#include "widgets/AntApp.h"
#include "widgets/AntAutoComplete.h"
#include "widgets/AntAvatar.h"
#include "widgets/AntBadge.h"
#include "widgets/AntBreadcrumb.h"
#include "widgets/AntButton.h"
#include "widgets/AntCalendar.h"
#include "widgets/AntCard.h"
#include "widgets/AntCarousel.h"
#include "widgets/AntCascader.h"
#include "widgets/AntCheckBox.h"
#include "widgets/AntCollapse.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntConfigProvider.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntDescriptions.h"
#include "widgets/AntDivider.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntEmpty.h"
#include "widgets/AntFlex.h"
#include "widgets/AntFloatButton.h"
#include "widgets/AntForm.h"
#include "widgets/AntGrid.h"
#include "widgets/AntIcon.h"
#include "widgets/AntImage.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntLayout.h"
#include "widgets/AntList.h"
#include "widgets/AntLog.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntMentions.h"
#include "widgets/AntMenu.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntMessage.h"
#include "widgets/AntModal.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPagination.h"
#include "widgets/AntPlainTextEdit.h"
#include "widgets/AntPopconfirm.h"
#include "widgets/AntPopover.h"
#include "widgets/AntProgress.h"
#include "widgets/AntQRCode.h"
#include "widgets/AntRadio.h"
#include "widgets/AntRate.h"
#include "widgets/AntResult.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntSegmented.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSkeleton.h"
#include "widgets/AntSlider.h"
#include "widgets/AntSpace.h"
#include "widgets/AntSpin.h"
#include "widgets/AntSplitter.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntSteps.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTable.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTag.h"
#include "widgets/AntTimeline.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntToolTip.h"
#include "widgets/AntTour.h"
#include "widgets/AntTransfer.h"
#include "widgets/AntTree.h"
#include "widgets/AntTreeSelect.h"
#include "widgets/AntTypography.h"
#include "widgets/AntUpload.h"
#include "widgets/AntWatermark.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"

class TestAntRenderSmoke : public QObject
{
    Q_OBJECT

private slots:
    void everyVisualWidgetRendersNonBlank();
};

namespace
{
constexpr QRgb SentinelPixel = 0xffff00ff;

struct RenderCase
{
    const char* name;
    std::function<QWidget*(QWidget*)> create;
    std::function<void(QWidget*)> configure;
    QSize size = QSize(260, 120);
    bool expectPaint = true;
};

QWidget* coloredBox(QWidget* parent, const QColor& color = QColor(0x16, 0x77, 0xff))
{
    auto* box = new QWidget(parent);
    box->setAutoFillBackground(true);
    QPalette palette = box->palette();
    palette.setColor(QPalette::Window, color);
    box->setPalette(palette);
    box->setMinimumSize(48, 32);
    return box;
}

QSize renderSizeFor(QWidget* widget, QSize preferred)
{
    QSize hint = widget->sizeHint();
    if (!hint.isValid() || hint.isEmpty())
    {
        hint = preferred;
    }
    return QSize(qMax(preferred.width(), hint.width()), qMax(preferred.height(), hint.height()));
}

int changedPixelCount(const QImage& image)
{
    int changed = 0;
    for (int y = 0; y < image.height(); ++y)
    {
        const QRgb* row = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x)
        {
            if (row[x] != SentinelPixel)
            {
                ++changed;
            }
        }
    }
    return changed;
}

QImage renderWidget(QWidget* widget)
{
    widget->ensurePolished();
    QCoreApplication::sendPostedEvents(widget, QEvent::Polish);
    QCoreApplication::processEvents();

    QImage image(widget->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(SentinelPixel);
    {
        QPainter painter(&image);
        widget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    }
    return image;
}

QList<RenderCase> renderCases()
{
    return {
        {"AntAlert", [](QWidget* p) { return new AntAlert(p); }, [](QWidget* w) {
             auto* alert = qobject_cast<AntAlert*>(w);
             alert->setTitle(QStringLiteral("Saved"));
             alert->setDescription(QStringLiteral("The record was updated."));
             alert->setShowIcon(true);
         }},
        {"AntAnchor", [](QWidget* p) { return new AntAnchor(p); }, [](QWidget* w) {
             auto* anchor = qobject_cast<AntAnchor*>(w);
             anchor->addLink(QStringLiteral("Section 1"), 0);
             anchor->addLink(QStringLiteral("Section 2"), 120);
         }},
        {"AntAutoComplete", [](QWidget* p) { return new AntAutoComplete(p); }, [](QWidget* w) {
             auto* autocomplete = qobject_cast<AntAutoComplete*>(w);
             autocomplete->setText(QStringLiteral("app"));
             autocomplete->addSuggestion(QStringLiteral("Apple"), QStringLiteral("apple"));
         }},
        {"AntAvatarGroup", [](QWidget* p) { return new AntAvatarGroup(p); }, [](QWidget* w) {
             auto* group = qobject_cast<AntAvatarGroup*>(w);
             group->addAvatar(new AntAvatar(QStringLiteral("A")));
             group->addAvatar(new AntAvatar(QStringLiteral("B")));
         }, QSize(160, 64)},
        {"AntAvatar", [](QWidget* p) { return new AntAvatar(p); }, [](QWidget* w) {
             qobject_cast<AntAvatar*>(w)->setText(QStringLiteral("AB"));
         }, QSize(64, 64)},
        {"AntBadge", [](QWidget* p) { return new AntBadge(p); }, [](QWidget* w) {
             auto* badge = qobject_cast<AntBadge*>(w);
             badge->setCount(12);
             badge->setShowZero(true);
         }, QSize(72, 40)},
        {"AntBreadcrumb", [](QWidget* p) { return new AntBreadcrumb(p); }, [](QWidget* w) {
             auto* breadcrumb = qobject_cast<AntBreadcrumb*>(w);
             breadcrumb->addItem(QStringLiteral("Home"), QStringLiteral("/"));
             breadcrumb->addItem(QStringLiteral("Detail"), QStringLiteral("/detail"));
         }},
        {"AntButton", [](QWidget* p) { return new AntButton(QStringLiteral("Button"), p); }, nullptr, QSize(120, 48)},
        {"AntCalendar", [](QWidget* p) { return new AntCalendar(p); }, [](QWidget* w) {
             qobject_cast<AntCalendar*>(w)->setSelectedDate(QDate(2026, 4, 30));
         }, QSize(360, 280)},
        {"AntCard", [](QWidget* p) { return new AntCard(QStringLiteral("Card"), p); }, [](QWidget* w) {
             qobject_cast<AntCard*>(w)->setExtra(QStringLiteral("More"));
         }, QSize(260, 160)},
        {"AntCarousel", [](QWidget* p) { return new AntCarousel(p); }, [](QWidget* w) {
             auto* carousel = qobject_cast<AntCarousel*>(w);
             carousel->setAutoPlay(false);
             carousel->addSlide(coloredBox(carousel));
             carousel->addSlide(coloredBox(carousel, QColor(0x52, 0xc4, 0x1a)));
         }, QSize(260, 160)},
        {"AntCascader", [](QWidget* p) { return new AntCascader(p); }, [](QWidget* w) {
             AntCascaderOption option;
             option.value = QStringLiteral("zhejiang");
             option.label = QStringLiteral("Zhejiang");
             qobject_cast<AntCascader*>(w)->setOptions({option});
         }},
        {"AntCheckBox", [](QWidget* p) { return new AntCheckBox(p); }, [](QWidget* w) {
             auto* checkbox = qobject_cast<AntCheckBox*>(w);
             checkbox->setText(QStringLiteral("Remember"));
             checkbox->setChecked(true);
         }},
        {"AntCollapsePanel", [](QWidget* p) { return new AntCollapsePanel(QStringLiteral("Panel"), p); }, [](QWidget* w) {
             qobject_cast<AntCollapsePanel*>(w)->setExpanded(true);
         }},
        {"AntCollapse", [](QWidget* p) { return new AntCollapse(p); }, [](QWidget* w) {
             auto* collapse = qobject_cast<AntCollapse*>(w);
             collapse->addPanel(QStringLiteral("Panel 1"))->setExpanded(true);
         }},
        {"AntColorPicker", [](QWidget* p) { return new AntColorPicker(p); }, [](QWidget* w) {
             qobject_cast<AntColorPicker*>(w)->setCurrentColor(QColor(0x16, 0x77, 0xff));
         }},
        {"AntDatePicker", [](QWidget* p) { return new AntDatePicker(p); }, [](QWidget* w) {
             qobject_cast<AntDatePicker*>(w)->setSelectedDate(QDate(2026, 4, 30));
         }},
        {"AntDescriptionsItem", [](QWidget* p) { return new AntDescriptionsItem(QStringLiteral("Name"), QStringLiteral("Alice"), p); }, nullptr, QSize(160, 40), false},
        {"AntDescriptions", [](QWidget* p) { return new AntDescriptions(p); }, [](QWidget* w) {
             auto* descriptions = qobject_cast<AntDescriptions*>(w);
             descriptions->setTitle(QStringLiteral("User"));
             descriptions->addItem(QStringLiteral("Name"), QStringLiteral("Alice"));
         }, QSize(320, 160)},
        {"AntDivider", [](QWidget* p) { return new AntDivider(QStringLiteral("OR"), p); }, nullptr},
        {"AntDockWidget", [](QWidget* p) { return new AntDockWidget(QStringLiteral("Dock"), p); }, [](QWidget* w) {
             qobject_cast<AntDockWidget*>(w)->setWidget(coloredBox(w));
         }, QSize(260, 160)},
        {"AntDrawer", [](QWidget* p) { return new AntDrawer(p); }, [](QWidget* w) {
             auto* drawer = qobject_cast<AntDrawer*>(w);
             drawer->setTitle(QStringLiteral("Drawer"));
             drawer->setOpen(true);
         }, QSize(320, 180), false},
        {"AntDropdown", [](QWidget* p) { return new AntDropdown(p); }, [](QWidget* w) {
             qobject_cast<AntDropdown*>(w)->addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
         }, QSize(120, 40), false},
        {"AntEmpty", [](QWidget* p) { return new AntEmpty(p); }, nullptr, QSize(240, 180)},
        {"AntFlex", [](QWidget* p) { return new AntFlex(p); }, [](QWidget* w) {
             qobject_cast<AntFlex*>(w)->addWidget(new AntButton(QStringLiteral("A")));
         }, QSize(220, 72), false},
        {"AntFloatButton", [](QWidget* p) { return new AntFloatButton(p); }, nullptr, QSize(72, 72)},
        {"AntFormItem", [](QWidget* p) { return new AntFormItem(p); }, [](QWidget* w) {
             auto* item = qobject_cast<AntFormItem*>(w);
             item->setLabel(QStringLiteral("Email"));
             item->setHelpText(QStringLiteral("Required"));
             item->setRequired(true);
         }},
        {"AntFormProvider", [](QWidget* p) { return new AntFormProvider(p); }, nullptr, QSize(120, 40), false},
        {"AntForm", [](QWidget* p) { return new AntForm(p); }, [](QWidget* w) {
             qobject_cast<AntForm*>(w)->addItem(QStringLiteral("Name"), new AntInput, true);
         }, QSize(340, 120)},
        {"AntFormList", [](QWidget* p) { return new AntFormList(p); }, nullptr, QSize(120, 40), false},
        {"AntCol", [](QWidget* p) { return new AntCol(12, p); }, nullptr, QSize(120, 40), false},
        {"AntRow", [](QWidget* p) { return new AntRow(p); }, [](QWidget* w) {
             qobject_cast<AntRow*>(w)->addWidget(coloredBox(w), 12, 0);
         }, QSize(220, 64), false},
        {"AntIcon", [](QWidget* p) { return new AntIcon(p); }, [](QWidget* w) {
             qobject_cast<AntIcon*>(w)->setIconType(Ant::IconType::Search);
         }, QSize(48, 48)},
        {"AntImage", [](QWidget* p) { return new AntImage(p); }, [](QWidget* w) {
             auto* image = qobject_cast<AntImage*>(w);
             image->setSrc(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
             image->setImgWidth(120);
             image->setImgHeight(90);
         }, QSize(140, 110)},
        {"AntInput", [](QWidget* p) { return new AntInput(p); }, [](QWidget* w) {
             qobject_cast<AntInput*>(w)->setText(QStringLiteral("Input"));
         }},
        {"AntInputNumber", [](QWidget* p) { return new AntInputNumber(p); }, [](QWidget* w) {
             qobject_cast<AntInputNumber*>(w)->setValue(42);
         }},
        {"AntLayoutHeader", [](QWidget* p) { return new AntLayoutHeader(p); }, nullptr, QSize(260, 64)},
        {"AntLayoutFooter", [](QWidget* p) { return new AntLayoutFooter(p); }, nullptr, QSize(260, 64)},
        {"AntLayoutContent", [](QWidget* p) { return new AntLayoutContent(p); }, nullptr, QSize(260, 100)},
        {"AntLayoutSider", [](QWidget* p) { return new AntLayoutSider(p); }, nullptr, QSize(120, 160)},
        {"AntLayout", [](QWidget* p) { return new AntLayout(p); }, [](QWidget* w) {
             auto* layout = qobject_cast<AntLayout*>(w);
             layout->setHeader(new AntLayoutHeader);
             layout->setContent(new AntLayoutContent);
             layout->setFooter(new AntLayoutFooter);
         }, QSize(320, 220)},
        {"AntListItemMeta", [](QWidget* p) { return new AntListItemMeta(p); }, [](QWidget* w) {
             auto* meta = qobject_cast<AntListItemMeta*>(w);
             meta->setTitle(QStringLiteral("Title"));
             meta->setDescription(QStringLiteral("Description"));
         }},
        {"AntListItem", [](QWidget* p) { return new AntListItem(p); }, [](QWidget* w) {
             qobject_cast<AntListItem*>(w)->setContentWidget(new QLabel(QStringLiteral("List item")));
         }},
        {"AntList", [](QWidget* p) { return new AntList(p); }, [](QWidget* w) {
             auto* list = qobject_cast<AntList*>(w);
             auto* item = new AntListItem;
             item->setContentWidget(new QLabel(QStringLiteral("List item")));
             list->addItem(item);
         }, QSize(300, 120)},
        {"AntLog", [](QWidget* p) { return new AntLog(p); }, [](QWidget* w) {
             qobject_cast<AntLog*>(w)->info(QStringLiteral("Render smoke"));
         }, QSize(320, 140)},
        {"AntMasonry", [](QWidget* p) { return new AntMasonry(p); }, [](QWidget* w) {
             qobject_cast<AntMasonry*>(w)->addWidget(coloredBox(w));
         }, QSize(260, 120), false},
        {"AntMentions", [](QWidget* p) { return new AntMentions(p); }, [](QWidget* w) {
             auto* mentions = qobject_cast<AntMentions*>(w);
             mentions->setPlaceholderText(QStringLiteral("@alice"));
             mentions->addSuggestion(QStringLiteral("alice"));
         }},
        {"AntMenu", [](QWidget* p) { return new AntMenu(p); }, [](QWidget* w) {
             auto* menu = qobject_cast<AntMenu*>(w);
             menu->addItem(QStringLiteral("home"), QStringLiteral("Home"));
             menu->addItem(QStringLiteral("docs"), QStringLiteral("Docs"));
             menu->setSelectedKey(QStringLiteral("home"));
         }, QSize(220, 100)},
        {"AntMenuBar", [](QWidget* p) { return new AntMenuBar(p); }, [](QWidget* w) {
             qobject_cast<AntMenuBar*>(w)->addMenu(QStringLiteral("File"));
         }},
        {"AntMessage", [](QWidget* p) { return new AntMessage(p); }, [](QWidget* w) {
             qobject_cast<AntMessage*>(w)->setText(QStringLiteral("Saved"));
         }},
        {"AntModal", [](QWidget* p) { return new AntModal(p); }, [](QWidget* w) {
             auto* modal = qobject_cast<AntModal*>(w);
             modal->setTitle(QStringLiteral("Modal"));
             modal->setContent(QStringLiteral("Content"));
         }, QSize(320, 180), false},
        {"AntNavItem", [](QWidget* p) { return new AntNavItem(QStringLiteral("Nav"), p); }, nullptr, QSize(220, 48)},
        {"AntNotification", [](QWidget* p) { return new AntNotification(p); }, [](QWidget* w) {
             auto* notification = qobject_cast<AntNotification*>(w);
             notification->setTitle(QStringLiteral("Update"));
             notification->setDescription(QStringLiteral("Ready"));
         }, QSize(360, 140)},
        {"AntPagination", [](QWidget* p) { return new AntPagination(p); }, [](QWidget* w) {
             auto* pagination = qobject_cast<AntPagination*>(w);
             pagination->setTotal(120);
             pagination->setCurrent(3);
         }, QSize(360, 64)},
        {"AntPlainTextEdit", [](QWidget* p) { return new AntPlainTextEdit(p); }, [](QWidget* w) {
             qobject_cast<AntPlainTextEdit*>(w)->setPlainText(QStringLiteral("Text area"));
         }, QSize(240, 120)},
        {"AntPopconfirm", [](QWidget* p) { return new AntPopconfirm(p); }, [](QWidget* w) {
             auto* popconfirm = qobject_cast<AntPopconfirm*>(w);
             popconfirm->setTitle(QStringLiteral("Delete?"));
             popconfirm->setDescription(QStringLiteral("This cannot be undone."));
         }, QSize(260, 120), false},
        {"AntPopover", [](QWidget* p) { return new AntPopover(p); }, [](QWidget* w) {
             auto* popover = qobject_cast<AntPopover*>(w);
             popover->setTitle(QStringLiteral("Title"));
             popover->setContent(QStringLiteral("Content"));
         }, QSize(260, 120), false},
        {"AntProgress", [](QWidget* p) { return new AntProgress(p); }, [](QWidget* w) {
             qobject_cast<AntProgress*>(w)->setPercent(68);
         }},
        {"AntQRCode", [](QWidget* p) { return new AntQRCode(p); }, [](QWidget* w) {
             qobject_cast<AntQRCode*>(w)->setValue(QStringLiteral("https://example.com"));
         }, QSize(200, 220)},
        {"AntRadio", [](QWidget* p) { return new AntRadio(p); }, [](QWidget* w) {
             auto* radio = qobject_cast<AntRadio*>(w);
             radio->setText(QStringLiteral("Option"));
             radio->setChecked(true);
         }},
        {"AntRate", [](QWidget* p) { return new AntRate(p); }, [](QWidget* w) {
             qobject_cast<AntRate*>(w)->setValue(3.5);
         }},
        {"AntResult", [](QWidget* p) { return new AntResult(QStringLiteral("Success"), p); }, [](QWidget* w) {
             qobject_cast<AntResult*>(w)->setSubTitle(QStringLiteral("Completed"));
         }, QSize(320, 220)},
        {"AntScrollArea", [](QWidget* p) { return new AntScrollArea(p); }, [](QWidget* w) {
             qobject_cast<AntScrollArea*>(w)->setWidget(coloredBox(w));
         }, QSize(220, 120)},
        {"AntScrollBar", [](QWidget* p) { return new AntScrollBar(p); }, [](QWidget* w) {
             auto* bar = qobject_cast<AntScrollBar*>(w);
             bar->setAutoHide(false);
             bar->setRange(0, 100);
             bar->setValue(40);
         }, QSize(24, 140)},
        {"AntSegmented", [](QWidget* p) { return new AntSegmented(p); }, [](QWidget* w) {
             qobject_cast<AntSegmented*>(w)->setOptions({{QStringLiteral("A"), QStringLiteral("A")},
                                                         {QStringLiteral("B"), QStringLiteral("B")}});
         }},
        {"AntSelect", [](QWidget* p) { return new AntSelect(p); }, [](QWidget* w) {
             auto* select = qobject_cast<AntSelect*>(w);
             select->addOption(QStringLiteral("Apple"), QStringLiteral("apple"));
             select->setCurrentIndex(0);
         }},
        {"AntSkeleton", [](QWidget* p) { return new AntSkeleton(p); }, nullptr, QSize(320, 120)},
        {"AntSlider", [](QWidget* p) { return new AntSlider(p); }, [](QWidget* w) {
             qobject_cast<AntSlider*>(w)->setValue(50);
         }},
        {"AntSpace", [](QWidget* p) { return new AntSpace(p); }, [](QWidget* w) {
             qobject_cast<AntSpace*>(w)->addItem(new AntButton(QStringLiteral("A")));
         }, QSize(220, 72), false},
        {"AntSpin", [](QWidget* p) { return new AntSpin(p); }, [](QWidget* w) {
             qobject_cast<AntSpin*>(w)->setDescription(QStringLiteral("Loading"));
         }, QSize(120, 100)},
        {"AntSplitter", [](QWidget* p) { return new AntSplitter(p); }, [](QWidget* w) {
             auto* splitter = qobject_cast<AntSplitter*>(w);
             splitter->addWidget(coloredBox(splitter));
             splitter->addWidget(coloredBox(splitter, QColor(0x52, 0xc4, 0x1a)));
         }, QSize(260, 120), false},
        {"AntStatistic", [](QWidget* p) { return new AntStatistic(QStringLiteral("Revenue"), p); }, [](QWidget* w) {
             auto* statistic = qobject_cast<AntStatistic*>(w);
             statistic->setPrefix(QStringLiteral("$"));
             statistic->setValue(1234.56);
             statistic->setPrecision(2);
         }},
        {"AntStatusBar", [](QWidget* p) { return new AntStatusBar(p); }, [](QWidget* w) {
             auto* status = qobject_cast<AntStatusBar*>(w);
             status->setMessage(QStringLiteral("Ready"));
             status->addPermanentItem(QStringLiteral("Ln 1"));
         }},
        {"AntSteps", [](QWidget* p) { return new AntSteps(p); }, [](QWidget* w) {
             auto* steps = qobject_cast<AntSteps*>(w);
             steps->addStep(QStringLiteral("One"));
             steps->addStep(QStringLiteral("Two"));
         }, QSize(320, 100)},
        {"AntSwitch", [](QWidget* p) { return new AntSwitch(p); }, [](QWidget* w) {
             qobject_cast<AntSwitch*>(w)->setChecked(true);
         }, QSize(84, 48)},
        {"AntTable", [](QWidget* p) { return new AntTable(p); }, [](QWidget* w) {
             auto* table = qobject_cast<AntTable*>(w);
             table->addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 120});
             table->addColumn({QStringLiteral("Age"), QStringLiteral("age"), QStringLiteral("age"), 80});
             table->addRow({{{QStringLiteral("name"), QStringLiteral("Alice")}, {QStringLiteral("age"), 30}}});
         }, QSize(360, 180)},
        {"AntTabs", [](QWidget* p) { return new AntTabs(p); }, [](QWidget* w) {
             auto* tabs = qobject_cast<AntTabs*>(w);
             tabs->addTab(coloredBox(tabs), QStringLiteral("one"), QStringLiteral("One"));
             tabs->addTab(coloredBox(tabs, QColor(0x52, 0xc4, 0x1a)), QStringLiteral("two"), QStringLiteral("Two"));
         }, QSize(320, 160)},
        {"AntTag", [](QWidget* p) { return new AntTag(QStringLiteral("Tag"), p); }, nullptr},
        {"AntTimeline", [](QWidget* p) { return new AntTimeline(p); }, [](QWidget* w) {
             auto* timeline = qobject_cast<AntTimeline*>(w);
             timeline->addItem(QStringLiteral("Step 1"), QStringLiteral("Content 1"));
             timeline->addItem(QStringLiteral("Step 2"), QStringLiteral("Content 2"));
         }, QSize(320, 160)},
        {"AntTimePicker", [](QWidget* p) { return new AntTimePicker(p); }, [](QWidget* w) {
             qobject_cast<AntTimePicker*>(w)->setSelectedTime(QTime(10, 20, 30));
         }},
        {"AntToolBar", [](QWidget* p) { return new AntToolBar(p); }, [](QWidget* w) {
             qobject_cast<AntToolBar*>(w)->addAction(QStringLiteral("New"));
         }, QSize(260, 64)},
        {"AntToolButton", [](QWidget* p) { return new AntToolButton(QStringLiteral("Tool"), p); }, nullptr},
        {"AntToolTip", [](QWidget* p) { return new AntToolTip(p); }, [](QWidget* w) {
             qobject_cast<AntToolTip*>(w)->setTitle(QStringLiteral("Tooltip"));
         }, QSize(160, 48), false},
        {"AntTransfer", [](QWidget* p) { return new AntTransfer(p); }, [](QWidget* w) {
             qobject_cast<AntTransfer*>(w)->setSourceItems({QStringLiteral("A"), QStringLiteral("B")});
         }, QSize(420, 220)},
        {"AntTree", [](QWidget* p) { return new AntTree(p); }, [](QWidget* w) {
             auto* tree = qobject_cast<AntTree*>(w);
             AntTreeNode node;
             node.key = QStringLiteral("root");
             node.title = QStringLiteral("Root");
             tree->setTreeData({node});
         }, QSize(240, 140)},
        {"AntTreeSelect", [](QWidget* p) { return new AntTreeSelect(p); }, [](QWidget* w) {
             auto* treeSelect = qobject_cast<AntTreeSelect*>(w);
             AntTreeNode node;
             node.key = QStringLiteral("root");
             node.title = QStringLiteral("Root");
             treeSelect->setTreeData({node});
             treeSelect->setValue({QStringLiteral("root")});
         }},
        {"AntTypography", [](QWidget* p) { return new AntTypography(QStringLiteral("Typography"), p); }, nullptr},
        {"AntUpload", [](QWidget* p) { return new AntUpload(p); }, [](QWidget* w) {
             AntUploadFile file;
             file.uid = QStringLiteral("1");
             file.name = QStringLiteral("demo.txt");
             file.status = Ant::UploadFileStatus::Done;
             qobject_cast<AntUpload*>(w)->addFile(file);
         }, QSize(260, 100)},
        {"AntWatermark", [](QWidget* p) { return new AntWatermark(p); }, [](QWidget* w) {
             qobject_cast<AntWatermark*>(w)->setContent(QStringLiteral("Draft"));
         }, QSize(320, 180)},
        {"AntWidget", [](QWidget* p) { return new AntWidget(p); }, nullptr, QSize(160, 80), false},
        {"AntWindow", [](QWidget* p) { return new AntWindow(p); }, [](QWidget* w) {
             auto* window = qobject_cast<AntWindow*>(w);
             window->setWindowTitle(QStringLiteral("Window"));
             window->setCentralWidget(coloredBox(window));
         }, QSize(360, 240)}
    };
}
} // namespace

void TestAntRenderSmoke::everyVisualWidgetRendersNonBlank()
{
    // These public QObject utilities are intentionally included above so the
    // inventory guard can track them, but they have no paint surface to render:
    // AntAffix, AntApp, AntConfigProvider, AntTour.
    QVERIFY(!renderCases().isEmpty());

    for (const RenderCase& renderCase : renderCases())
    {
        QWidget host;
        host.resize(renderCase.size + QSize(20, 20));

        QWidget* widget = renderCase.create(&host);
        QVERIFY2(widget != nullptr, renderCase.name);
        if (renderCase.configure)
        {
            renderCase.configure(widget);
        }

        const QSize size = renderSizeFor(widget, renderCase.size);
        widget->resize(size);
        widget->move(10, 10);
        widget->show();

        const QImage image = renderWidget(widget);
        QVERIFY2(!image.isNull(), renderCase.name);
        if (renderCase.expectPaint)
        {
            QVERIFY2(changedPixelCount(image) > 0, renderCase.name);
        }
    }
}

QTEST_MAIN(TestAntRenderSmoke)
#include "TestAntRenderSmoke.moc"
