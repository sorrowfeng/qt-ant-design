#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "core/AntTypes.h"
#include "widgets/AntAlert.h"
#include "widgets/AntButton.h"
#include "widgets/AntCheckbox.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntModal.h"
#include "widgets/AntProgress.h"
#include "widgets/AntRadio.h"
#include "widgets/AntSegmented.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSlider.h"
#include "widgets/AntSteps.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTag.h"

namespace example::pages
{
QWidget* createShowcasePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    page->setObjectName(QStringLiteral("showcasePage"));
    page->setStyleSheet(QStringLiteral(
        "#showcasePage {"
        "  background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #f7fbff, stop:0.55 #ffffff, stop:1 #eef7f3);"
        "}"));

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 24, 24, 24);

    auto* surface = new QFrame(page);
    surface->setObjectName(QStringLiteral("showcaseSurface"));
    surface->setStyleSheet(QStringLiteral(
        "#showcaseSurface {"
        "  background:white;"
        "  border:1px solid rgba(22,119,255,0.08);"
        "  border-radius:16px;"
        "}"));
    surface->setMaximumWidth(760);

    auto* surfaceLayout = new QVBoxLayout(surface);
    surfaceLayout->setContentsMargins(160, 180, 160, 110);
    surfaceLayout->setSpacing(14);

    auto* info = new AntAlert(surface);
    info->setTitle(QStringLiteral("信息内容展示"));
    info->setAlertType(Ant::AlertType::Info);
    info->setShowIcon(false);
    surfaceLayout->addWidget(info);

    auto* row1 = new QHBoxLayout();
    row1->setSpacing(12);
    auto* dropdown = new AntSelect(surface);
    dropdown->setPlaceholderText(QStringLiteral("下拉菜单"));
    dropdown->addOptions({QStringLiteral("苹果"), QStringLiteral("香蕉"), QStringLiteral("橘子")});
    dropdown->setFixedWidth(140);
    row1->addWidget(dropdown);

    auto* colorBox = new QFrame(surface);
    colorBox->setStyleSheet(QStringLiteral("background:white; border:1px solid #d9d9d9; border-radius:8px;"));
    auto* colorLayout = new QHBoxLayout(colorBox);
    colorLayout->setContentsMargins(10, 6, 10, 6);
    colorLayout->setSpacing(8);
    auto* swatch = new QLabel(colorBox);
    swatch->setFixedSize(20, 20);
    swatch->setStyleSheet(QStringLiteral("background:#1677ff; border-radius:4px;"));
    colorLayout->addWidget(swatch);
    colorLayout->addWidget(new QLabel(QStringLiteral("#1677FF"), colorBox));
    row1->addWidget(colorBox);

    auto* missingTags = new QFrame(surface);
    missingTags->setStyleSheet(QStringLiteral(
        "background:white; border:1px solid #d9d9d9; border-radius:8px;"));
    auto* missingTagsLayout = new QHBoxLayout(missingTags);
    missingTagsLayout->setContentsMargins(10, 6, 10, 6);
    missingTagsLayout->setSpacing(6);
    auto* appleTag = new AntTag(QStringLiteral("苹果"), missingTags);
    appleTag->setClosable(true);
    auto* bananaTag = new AntTag(QStringLiteral("香蕉"), missingTags);
    bananaTag->setClosable(true);
    missingTagsLayout->addWidget(appleTag);
    missingTagsLayout->addWidget(bananaTag);
    auto* missingSelectNote = new QLabel(QStringLiteral("缺 Select multiple/tags"), missingTags);
    missingSelectNote->setStyleSheet(QStringLiteral("color:#bfbfbf; font-size:12px;"));
    missingTagsLayout->addWidget(missingSelectNote);
    missingTagsLayout->addStretch();
    row1->addWidget(missingTags, 1);
    surfaceLayout->addLayout(row1);

    auto* row2 = new QHBoxLayout();
    row2->setSpacing(12);
    auto* datePicker = new AntDatePicker(surface);
    datePicker->setPlaceholderText(QStringLiteral("请选择日期"));
    datePicker->setFixedWidth(140);
    row2->addWidget(datePicker);

    auto* missingDate = new QFrame(surface);
    missingDate->setStyleSheet(QStringLiteral(
        "background:white; border:1px dashed #d9d9d9; border-radius:8px;"));
    auto* missingDateLayout = new QHBoxLayout(missingDate);
    missingDateLayout->setContentsMargins(10, 6, 10, 6);
    missingDateLayout->setSpacing(6);
    auto* springTag = new AntTag(QStringLiteral("苹果"), missingDate);
    springTag->setClosable(true);
    auto* summerTag = new AntTag(QStringLiteral("香蕉"), missingDate);
    summerTag->setClosable(true);
    missingDateLayout->addWidget(springTag);
    missingDateLayout->addWidget(summerTag);
    auto* missingDateNote = new QLabel(QStringLiteral("缺标签化多选日期/复合输入"), missingDate);
    missingDateNote->setStyleSheet(QStringLiteral("color:#bfbfbf; font-size:12px;"));
    missingDateLayout->addWidget(missingDateNote);
    missingDateLayout->addStretch();
    row2->addWidget(missingDate, 1);
    surfaceLayout->addLayout(row2);

    auto* progress = new AntProgress(surface);
    progress->setPercent(60);
    surfaceLayout->addWidget(progress);

    auto* steps = new AntSteps(surface);
    steps->addStep(QStringLiteral("已完成"), QString(), QString(), Ant::StepStatus::Finish);
    steps->addStep(QStringLiteral("进行中"), QString(), QString(), Ant::StepStatus::Process);
    steps->addStep(QStringLiteral("等待中"), QString(), QString(), Ant::StepStatus::Wait);
    steps->setCurrentIndex(1);
    surfaceLayout->addWidget(steps);

    auto* slider = new AntSlider(surface);
    slider->setValue(60);
    QObject::connect(slider, &AntSlider::valueChanged, progress, &AntProgress::setPercent);
    surfaceLayout->addWidget(slider);

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(12);
    auto* primary = new AntButton(QStringLiteral("主要按钮"));
    primary->setButtonType(Ant::ButtonType::Primary);
    auto* danger = new AntButton(QStringLiteral("危险按钮"));
    danger->setButtonType(Ant::ButtonType::Primary);
    danger->setDanger(true);
    auto* normal = new AntButton(QStringLiteral("默认按钮"));
    auto* dashed = new AntButton(QStringLiteral("虚线按钮"));
    dashed->setButtonType(Ant::ButtonType::Dashed);
    buttonRow->addWidget(primary);
    buttonRow->addWidget(danger);
    buttonRow->addWidget(normal);
    buttonRow->addWidget(dashed);
    buttonRow->addStretch();
    surfaceLayout->addLayout(buttonRow);

    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(14);
    auto* enabledSwitch = new AntSwitch(surface);
    enabledSwitch->setChecked(true);
    stateRow->addWidget(enabledSwitch);

    auto* cbApple = new AntCheckbox(QStringLiteral("苹果"), surface);
    cbApple->setChecked(true);
    auto* cbBanana = new AntCheckbox(QStringLiteral("香蕉"), surface);
    auto* cbOrange = new AntCheckbox(QStringLiteral("橘子"), surface);
    stateRow->addWidget(cbApple);
    stateRow->addWidget(cbBanana);
    stateRow->addWidget(cbOrange);

    auto* radioApple = new AntRadio(QStringLiteral("苹果"), surface);
    radioApple->setChecked(true);
    auto* radioBanana = new AntRadio(QStringLiteral("香蕉"), surface);
    stateRow->addWidget(radioApple);
    stateRow->addWidget(radioBanana);
    stateRow->addStretch();
    surfaceLayout->addLayout(stateRow);

    auto* segmentedRow = new QHBoxLayout();
    segmentedRow->setSpacing(14);
    auto* abc = new AntSegmented(surface);
    abc->setOptions({
        {QStringLiteral("a"), QStringLiteral("A")},
        {QStringLiteral("b"), QStringLiteral("B")},
        {QStringLiteral("c"), QStringLiteral("C")},
    });
    abc->setValue(QStringLiteral("a"));
    segmentedRow->addWidget(abc);

    auto* cadence = new AntSegmented(surface);
    cadence->setOptions({
        {QStringLiteral("daily"), QStringLiteral("每日")},
        {QStringLiteral("weekly"), QStringLiteral("每周")},
        {QStringLiteral("monthly"), QStringLiteral("每月")},
    });
    cadence->setValue(QStringLiteral("daily"));
    segmentedRow->addWidget(cadence);
    segmentedRow->addStretch();
    surfaceLayout->addLayout(segmentedRow);

    root->addWidget(surface, 0, Qt::AlignCenter);
    root->addStretch();

    auto* modal = new AntModal(page);
    modal->setTitle(QStringLiteral("Ant Design"));
    modal->setContent(QStringLiteral("Ant Design 使用 CSS-in-JS 技术以提供动态与混合主题的能力。与此同时，我们使用组件级别的 CSS-in-JS 解决方案，让你的应用获得更好的性能。"));
    modal->setOkText(QStringLiteral("确定"));
    modal->setCancelText(QStringLiteral("取消"));
    modal->setDialogWidth(460);
    QTimer::singleShot(0, page, [modal]() { modal->setOpen(true); });

    return page;
}
}
