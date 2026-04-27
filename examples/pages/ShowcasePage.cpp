#include "Pages.h"

#include <QColor>
#include <QDate>
#include <QFrame>
#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
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

namespace
{
class ColorSwatch : public QFrame
{
public:
    explicit ColorSwatch(const QColor& color, QWidget* parent = nullptr)
        : QFrame(parent), m_color(color)
    {
        setFixedSize(20, 20);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_color);
        painter.drawRoundedRect(rect(), 4, 4);
    }

private:
    QColor m_color;
};
}

namespace example::pages
{
QWidget* createShowcasePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    page->setObjectName(QStringLiteral("showcasePage"));

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(24, 24, 24, 24);

    auto* surface = new QWidget(page);
    surface->setObjectName(QStringLiteral("showcaseSurface"));
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
    auto* colorLayout = new QHBoxLayout(colorBox);
    colorLayout->setContentsMargins(10, 6, 10, 6);
    colorLayout->setSpacing(8);
    colorLayout->addWidget(new ColorSwatch(QColor(QStringLiteral("#1677ff")), colorBox));
    colorLayout->addWidget(makeText(QStringLiteral("#1677FF"), colorBox));
    row1->addWidget(colorBox);

    auto* tagSelect = new AntSelect(surface);
    tagSelect->setSelectMode(Ant::SelectMode::Tags);
    tagSelect->addOptions({QStringLiteral("苹果"), QStringLiteral("香蕉"), QStringLiteral("橘子")});
    tagSelect->addTag(QStringLiteral("苹果"));
    tagSelect->addTag(QStringLiteral("香蕉"));
    row1->addWidget(tagSelect, 1);
    surfaceLayout->addLayout(row1);

    auto* row2 = new QHBoxLayout();
    row2->setSpacing(12);
    auto* datePicker = new AntDatePicker(surface);
    datePicker->setPlaceholderText(QStringLiteral("请选择日期"));
    datePicker->setFixedWidth(140);
    row2->addWidget(datePicker);

    auto* dateRange = new AntDatePicker(surface);
    dateRange->setRangeMode(true);
    dateRange->setStartDate(QDate::currentDate());
    dateRange->setEndDate(QDate::currentDate().addDays(7));
    row2->addWidget(dateRange, 1);
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
