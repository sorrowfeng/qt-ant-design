#include "Pages.h"

#include <QColor>
#include <QHBoxLayout>
#include <QPalette>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntAlert.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntCheckBox.h"
#include "widgets/AntColorPicker.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntProgress.h"
#include "widgets/AntRadio.h"
#include "widgets/AntSegmented.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSlider.h"
#include "widgets/AntSteps.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWidget.h"

namespace
{
class ShowcaseRoot : public AntWidget
{
public:
    explicit ShowcaseRoot(QWidget* parent = nullptr)
        : AntWidget(parent)
    {
        setAutoFillBackground(true);
        syncPalette();
    }

protected:
    void onThemeChanged(Ant::ThemeMode mode) override
    {
        Q_UNUSED(mode)
        syncPalette();
    }

private:
    void syncPalette()
    {
        QPalette palette = this->palette();
        palette.setColor(QPalette::Window, tokens().colorBgContainer);
        setPalette(palette);
    }
};

AntTypography* makeModalText(const QString& text, QWidget* parent, bool paragraph = false, bool strong = false)
{
    auto* typography = new AntTypography(text, parent);
    typography->setParagraph(paragraph);
    typography->setStrong(strong);
    if (paragraph)
    {
        typography->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    }
    return typography;
}

AntCard* makeModalPreview(QWidget* parent)
{
    auto* modal = new AntCard(parent);
    modal->setFixedSize(484, 196);
    modal->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    modal->setBordered(true);
    modal->bodyLayout()->setSpacing(12);

    auto* header = new QWidget(modal->bodyWidget());
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    headerLayout->addWidget(makeModalText(QStringLiteral("Ant Design"), header, false, true));
    headerLayout->addStretch();

    auto* close = new AntButton(header);
    close->setButtonType(Ant::ButtonType::Text);
    close->setButtonIconType(Ant::IconType::Close);
    close->setFixedSize(28, 28);
    headerLayout->addWidget(close);
    modal->bodyLayout()->addWidget(header);

    auto* body = makeModalText(
        QStringLiteral("Ant Design 使用 CSS-in-JS 技术以提供动态与混合主题的能力。与此同时，我们使用组件级别的 CSS-in-JS 解决方案，让你的应用获得更好的性能。"),
        modal,
        true);
    modal->bodyLayout()->addWidget(body);

    auto* footer = new QWidget(modal->bodyWidget());
    auto* footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->setSpacing(8);
    footerLayout->addStretch();

    auto* cancel = new AntButton(QStringLiteral("取消"), footer);
    cancel->setButtonSize(Ant::Size::Middle);
    cancel->setFixedWidth(64);
    footerLayout->addWidget(cancel);

    auto* ok = new AntButton(QStringLiteral("确定"), footer);
    ok->setButtonType(Ant::ButtonType::Primary);
    ok->setFixedWidth(64);
    footerLayout->addWidget(ok);
    modal->bodyLayout()->addWidget(footer);

    return modal;
}

AntSelect* makeTagSelect(QWidget* parent)
{
    auto* select = new AntSelect(parent);
    select->setSelectMode(Ant::SelectMode::Tags);
    select->addOptions({QStringLiteral("苹果"), QStringLiteral("香蕉"), QStringLiteral("橘子")});
    select->addTag(QStringLiteral("苹果"));
    select->addTag(QStringLiteral("香蕉"));
    return select;
}

AntButton* makeButton(const QString& text, QWidget* parent, Ant::ButtonType type = Ant::ButtonType::Default)
{
    auto* button = new AntButton(text, parent);
    button->setButtonType(type);
    button->setFixedWidth(96);
    return button;
}
} // namespace

namespace example::pages
{
QWidget* createShowcasePage(QWidget* /*owner*/)
{
    auto* page = new ShowcaseRoot();
    page->setObjectName(QStringLiteral("showcasePage"));
    page->setMinimumSize(560, 620);

    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(40, 40, 40, 40);
    root->setSpacing(0);
    root->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    auto* board = new QWidget(page);
    board->setFixedWidth(484);
    auto* boardLayout = new QVBoxLayout(board);
    boardLayout->setContentsMargins(0, 0, 0, 0);
    boardLayout->setSpacing(12);

    boardLayout->addWidget(makeModalPreview(board));

    auto* alert = new AntAlert(board);
    alert->setTitle(QStringLiteral("信息内容展示"));
    alert->setAlertType(Ant::AlertType::Info);
    alert->setShowIcon(false);
    boardLayout->addWidget(alert);

    auto* row1 = new QHBoxLayout();
    row1->setSpacing(12);
    auto* dropdown = new AntSelect(board);
    dropdown->addOption(QStringLiteral("下拉菜单"));
    dropdown->setCurrentIndex(0);
    dropdown->setFixedWidth(120);
    row1->addWidget(dropdown);
    auto* colorPicker = new AntColorPicker(QColor(QStringLiteral("#1677ff")), board);
    colorPicker->setShowText(true);
    row1->addWidget(colorPicker);
    row1->addWidget(makeTagSelect(board), 1);
    boardLayout->addLayout(row1);

    auto* row2 = new QHBoxLayout();
    row2->setSpacing(12);
    auto* datePicker = new AntDatePicker(board);
    datePicker->setPlaceholderText(QStringLiteral("请选择日期"));
    datePicker->setFixedWidth(148);
    row2->addWidget(datePicker);
    row2->addWidget(makeTagSelect(board), 1);
    boardLayout->addLayout(row2);

    auto* progress = new AntProgress(board);
    progress->setPercent(60);
    boardLayout->addWidget(progress);

    auto* steps = new AntSteps(board);
    steps->addStep(QStringLiteral("已完成"), QString(), QString(), Ant::StepStatus::Finish);
    steps->addStep(QStringLiteral("进行中"), QString(), QString(), Ant::StepStatus::Process);
    steps->addStep(QStringLiteral("等待中"), QString(), QString(), Ant::StepStatus::Wait);
    steps->setCurrentIndex(1);
    steps->setFixedHeight(56);
    boardLayout->addWidget(steps);

    auto* slider = new AntSlider(board);
    slider->setValue(55);
    slider->setFixedHeight(36);
    QObject::connect(slider, &AntSlider::valueChanged, progress, &AntProgress::setPercent);
    boardLayout->addWidget(slider);

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(16);
    auto* primary = makeButton(QStringLiteral("主要按钮"), board, Ant::ButtonType::Primary);
    auto* danger = makeButton(QStringLiteral("危险按钮"), board, Ant::ButtonType::Primary);
    danger->setDanger(true);
    buttonRow->addWidget(primary);
    buttonRow->addWidget(danger);
    buttonRow->addWidget(makeButton(QStringLiteral("默认按钮"), board));
    buttonRow->addWidget(makeButton(QStringLiteral("虚线按钮"), board, Ant::ButtonType::Dashed));
    boardLayout->addLayout(buttonRow);

    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(12);
    auto* enabledSwitch = new AntSwitch(board);
    enabledSwitch->setChecked(true);
    stateRow->addWidget(enabledSwitch);

    auto* cbApple = new AntCheckBox(QStringLiteral("苹果"), board);
    cbApple->setChecked(true);
    stateRow->addWidget(cbApple);
    stateRow->addWidget(new AntCheckBox(QStringLiteral("香蕉"), board));
    stateRow->addWidget(new AntCheckBox(QStringLiteral("橘子"), board));

    auto* radioApple = new AntRadio(QStringLiteral("苹果"), board);
    radioApple->setChecked(true);
    stateRow->addWidget(radioApple);
    stateRow->addWidget(new AntRadio(QStringLiteral("香蕉"), board));
    stateRow->addStretch();
    boardLayout->addLayout(stateRow);

    auto* segmentedRow = new QHBoxLayout();
    segmentedRow->setSpacing(18);
    auto* abc = new AntSegmented(board);
    abc->setOptions({
        {QStringLiteral("a"), QStringLiteral("A")},
        {QStringLiteral("b"), QStringLiteral("B")},
        {QStringLiteral("c"), QStringLiteral("C")},
    });
    abc->setValue(QStringLiteral("a"));
    segmentedRow->addWidget(abc);

    auto* cadence = new AntSegmented(board);
    cadence->setOptions({
        {QStringLiteral("daily"), QStringLiteral("每日")},
        {QStringLiteral("weekly"), QStringLiteral("每周")},
        {QStringLiteral("monthly"), QStringLiteral("每月")},
    });
    cadence->setValue(QStringLiteral("daily"));
    segmentedRow->addWidget(cadence);
    segmentedRow->addStretch();
    boardLayout->addLayout(segmentedRow);

    root->addWidget(board, 0, Qt::AlignHCenter | Qt::AlignTop);
    root->addStretch();

    return page;
}
} // namespace example::pages
