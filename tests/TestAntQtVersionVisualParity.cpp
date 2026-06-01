#include <QApplication>
#include <QAction>
#include <QCheckBox>
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QLayout>
#include <QLabel>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QScrollBar>
#include <QSet>
#include <QSize>
#include <QStringList>
#include <QTest>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#endif

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>

#include "core/AntDesign.h"
#include "core/AntTheme.h"
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
#include "widgets/AntDialog.h"
#include "widgets/AntDivider.h"
#include "widgets/AntDockManager.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntDropdown.h"
#include "widgets/AntEmpty.h"
#include "widgets/AntFileDialog.h"
#include "widgets/AntFlex.h"
#include "widgets/AntFloatButton.h"
#include "widgets/AntForm.h"
#include "widgets/AntGrid.h"
#include "widgets/AntIcon.h"
#include "widgets/AntImage.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputDialog.h"
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
#include "widgets/AntNav.h"
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
#include "widgets/AntStackedWidget.h"
#include "widgets/AntStatistic.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntRibbon.h"
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

#ifndef ANT_WIDGETS_DIR
#error "ANT_WIDGETS_DIR must be defined by tests/CMakeLists.txt"
#endif

class TestAntQtVersionVisualParity : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void visualParityScenesCoverPublicVisualWidgets();
    void criticalWidgetScenesMatchQtBaseline();
};

namespace
{
struct DiffStats
{
    qreal meanDelta = 0.0;
    qreal changed32Ratio = 0.0;
    qreal changed64Ratio = 0.0;
    int maxDelta = 0;
};

struct Scene
{
    QString name;
    QSize size;
    std::function<QWidget*()> create;
    QStringList controls;
    qreal maxMeanDelta = 14.0;
    qreal maxChanged32Ratio = 0.35;
    std::function<void(QWidget*)> prepare;
};

struct ThemeVariant
{
    QString name;
    Ant::ThemeMode mode = Ant::ThemeMode::Default;
};

struct ComparisonRecord
{
    QString sceneName;
    DiffStats stats;
    qreal maxMeanDelta = 0.0;
    qreal maxChanged32Ratio = 0.0;
};

QSet<QString> aliasWidgetHeaders()
{
    return {
        QStringLiteral("AntCalendarWidget"),
        QStringLiteral("AntComboBox"),
        QStringLiteral("AntDateEdit"),
        QStringLiteral("AntDoubleSpinBox"),
        QStringLiteral("AntLabel"),
        QStringLiteral("AntLineEdit"),
        QStringLiteral("AntListView"),
        QStringLiteral("AntListWidget"),
        QStringLiteral("AntMainWindow"),
        QStringLiteral("AntProgressBar"),
        QStringLiteral("AntPushButton"),
        QStringLiteral("AntRadioButton"),
        QStringLiteral("AntSpinBox"),
        QStringLiteral("AntTabWidget"),
        QStringLiteral("AntTableView"),
        QStringLiteral("AntTableWidget"),
        QStringLiteral("AntTimeEdit"),
        QStringLiteral("AntTreeView"),
        QStringLiteral("AntTreeWidget"),
    };
}

QSet<QString> publicWidgetHeaders()
{
    QSet<QString> headers;
    const QDir widgetsDir(QStringLiteral(ANT_WIDGETS_DIR));
    const QFileInfoList files = widgetsDir.entryInfoList({QStringLiteral("Ant*.h")}, QDir::Files, QDir::Name);
    const QSet<QString> aliases = aliasWidgetHeaders();
    for (const QFileInfo& file : files)
    {
        const QString baseName = file.completeBaseName();
        if (baseName == QStringLiteral("AntSelectPopup") || aliases.contains(baseName))
        {
            continue;
        }
        headers.insert(baseName);
    }
    return headers;
}

QSet<QString> atlasDeferredHeaders()
{
    return {
        QStringLiteral("AntAffix"),
        QStringLiteral("AntApp"),
        QStringLiteral("AntConfigProvider"),
    };
}

QStringList sortedValues(const QSet<QString>& values)
{
    QStringList list = values.values();
    list.sort();
    return list;
}

void applySurfacePalette(QWidget* widget, const QColor& color)
{
    QPalette palette = widget->palette();
    palette.setColor(QPalette::Window, color);
    widget->setPalette(palette);
    widget->setAutoFillBackground(true);
}

QWidget* makeSurface()
{
    auto* surface = new QWidget;
    applySurfacePalette(surface, antTheme->tokens().colorBgLayout);
    return surface;
}

AntTypography* makeLabel(const QString& text, QWidget* parent = nullptr)
{
    auto* label = new AntTypography(text, parent);
    label->setType(Ant::TypographyType::Default);
    label->setStrong(true);
    return label;
}

QWidget* coloredBox(const QColor& color, QWidget* parent = nullptr)
{
    auto* box = new QWidget(parent);
    applySurfacePalette(box, color);
    box->setMinimumSize(92, 56);
    return box;
}

QVBoxLayout* makeVBox(QWidget* surface, int margin = 16, int spacing = 12)
{
    auto* layout = new QVBoxLayout(surface);
    layout->setContentsMargins(margin, margin, margin, margin);
    layout->setSpacing(spacing);
    return layout;
}

QWidget* requiredChild(QWidget* root, const char* objectName)
{
    QWidget* child = root->findChild<QWidget*>(QString::fromLatin1(objectName));
    if (!child)
    {
        qFatal("Missing parity scene child: %s", objectName);
    }
    return child;
}

QPoint defaultInteractionPoint(QWidget* widget)
{
    return widget->rect().center();
}

void sendEnterEvent(QWidget* widget, const QPoint& pos)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QEnterEvent event(QPointF(pos), QPointF(pos), QPointF(widget->mapToGlobal(pos)));
#else
    QEvent event(QEvent::Enter);
#endif
    QCoreApplication::sendEvent(widget, &event);
}

void sendMouseMoveEvent(QWidget* widget, const QPoint& pos)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMouseEvent event(QEvent::MouseMove,
                      QPointF(pos),
                      QPointF(widget->mapToGlobal(pos)),
                      Qt::NoButton,
                      Qt::NoButton,
                      Qt::NoModifier);
#else
    QMouseEvent event(QEvent::MouseMove,
                      pos,
                      widget->mapToGlobal(pos),
                      Qt::NoButton,
                      Qt::NoButton,
                      Qt::NoModifier);
#endif
    QCoreApplication::sendEvent(widget, &event);
}

void sendMousePressEvent(QWidget* widget, const QPoint& pos)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QMouseEvent event(QEvent::MouseButtonPress,
                      QPointF(pos),
                      QPointF(widget->mapToGlobal(pos)),
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);
#else
    QMouseEvent event(QEvent::MouseButtonPress,
                      pos,
                      widget->mapToGlobal(pos),
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);
#endif
    QCoreApplication::sendEvent(widget, &event);
}

void hoverWidget(QWidget* widget, const QPoint& pos = QPoint())
{
    const QPoint target = pos.isNull() ? defaultInteractionPoint(widget) : pos;
    sendEnterEvent(widget, target);
    sendMouseMoveEvent(widget, target);
}

void prepareInteractionStateScene(QWidget* root)
{
    hoverWidget(requiredChild(root, "interactionHoverButton"));

    auto* pressedButton = qobject_cast<AntButton*>(requiredChild(root, "interactionPressedButton"));
    QVERIFY(pressedButton != nullptr);
    pressedButton->setDown(true);

    auto* focusedInput = qobject_cast<AntInput*>(requiredChild(root, "interactionFocusedInput"));
    QVERIFY(focusedInput != nullptr);
    hoverWidget(focusedInput);
    focusedInput->setFocus(Qt::OtherFocusReason);
    focusedInput->lineEdit()->setFocus(Qt::OtherFocusReason);

    hoverWidget(requiredChild(root, "interactionHoverCheckBox"));

    QWidget* pressedRadio = requiredChild(root, "interactionPressedRadio");
    hoverWidget(pressedRadio);
    sendMousePressEvent(pressedRadio, defaultInteractionPoint(pressedRadio));

    auto* slider = qobject_cast<AntSlider*>(requiredChild(root, "interactionFocusedSlider"));
    QVERIFY(slider != nullptr);
    hoverWidget(slider, QPoint(slider->width() / 2, slider->height() / 2));
    slider->setFocus(Qt::OtherFocusReason);
    slider->setFocusProgress(1.0);
    slider->setHandleScale(1.18);

    hoverWidget(requiredChild(root, "interactionMenu"), QPoint(72, 74));
    hoverWidget(requiredChild(root, "interactionTabs"), QPoint(160, 18));
    hoverWidget(requiredChild(root, "interactionPagination"), QPoint(168, 22));
    hoverWidget(requiredChild(root, "interactionTable"), QPoint(170, 88));
}

void prepareAcceptanceStateMatrixScene(QWidget* root)
{
    hoverWidget(requiredChild(root, "acceptanceHoverButton"));

    auto* focusedInput = qobject_cast<AntInput*>(requiredChild(root, "acceptanceFocusedInput"));
    QVERIFY(focusedInput != nullptr);
    focusedInput->setFocus(Qt::OtherFocusReason);
    focusedInput->lineEdit()->setFocus(Qt::OtherFocusReason);

    hoverWidget(requiredChild(root, "acceptanceMenu"), QPoint(72, 76));
    hoverWidget(requiredChild(root, "acceptanceTabs"), QPoint(168, 18));
    hoverWidget(requiredChild(root, "acceptancePagination"), QPoint(164, 22));
    hoverWidget(requiredChild(root, "acceptanceTable"), QPoint(180, 88));
    hoverWidget(requiredChild(root, "acceptanceScrollBar"), QPoint(5, 82));
}

void prepareModalSurfaceScene(QWidget* root)
{
    auto* modal = qobject_cast<AntModal*>(requiredChild(root, "popupModal"));
    QVERIFY(modal != nullptr);
    modal->setOpen(true);

    QTest::qWait(280);
    QCoreApplication::processEvents();
}

void prepareDrawerSurfaceScene(QWidget* root)
{
    auto* drawer = qobject_cast<AntDrawer*>(requiredChild(root, "popupDrawer"));
    QVERIFY(drawer != nullptr);
    drawer->setOpen(true);

    QTest::qWait(240);
    QCoreApplication::processEvents();
}

void prepareDropdownSurfaceScene(QWidget* root)
{
    auto* dropdown = root->findChild<AntDropdown*>(QStringLiteral("dropdownController"));
    if (!dropdown)
    {
        qFatal("Missing parity scene object: dropdownController");
    }
    dropdown->setOpen(true);
    QTest::qWait(200);
    QCoreApplication::processEvents();
}

void preparePopconfirmSurfaceScene(QWidget* root)
{
    auto* popconfirm = root->findChild<AntPopconfirm*>(QStringLiteral("popconfirmController"));
    if (!popconfirm)
    {
        qFatal("Missing parity scene object: popconfirmController");
    }
    popconfirm->setOpen(true);
    QTest::qWait(200);
    QCoreApplication::processEvents();
}

void prepareTourSurfaceScene(QWidget* root)
{
    root->show();
    root->raise();
    root->activateWindow();
    QCoreApplication::processEvents();

    auto* tour = root->findChild<AntTour*>(QStringLiteral("tourController"));
    if (!tour)
    {
        qFatal("Missing parity scene object: tourController");
    }
    tour->start(0);
    QTest::qWait(80);
    QCoreApplication::processEvents();
}

QWidget* makeButtonsScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Buttons"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);

    auto* primary = new AntButton(QStringLiteral("Primary"), row);
    primary->setButtonType(Ant::ButtonType::Primary);
    rowLayout->addWidget(primary);

    rowLayout->addWidget(new AntButton(QStringLiteral("Default"), row));

    auto* dashed = new AntButton(QStringLiteral("Dashed"), row);
    dashed->setButtonType(Ant::ButtonType::Dashed);
    rowLayout->addWidget(dashed);

    auto* icon = new AntButton(row);
    icon->setButtonType(Ant::ButtonType::Text);
    icon->setButtonIconType(Ant::IconType::Search);
    icon->setFixedSize(34, 34);
    rowLayout->addWidget(icon);

    auto* standaloneIcon = new AntIcon(row);
    standaloneIcon->setIconType(Ant::IconType::Setting);
    standaloneIcon->setFixedSize(28, 28);
    rowLayout->addWidget(standaloneIcon);

    auto* disabled = new AntButton(QStringLiteral("Disabled"), row);
    disabled->setEnabled(false);
    rowLayout->addWidget(disabled);
    rowLayout->addStretch();
    layout->addWidget(row);
    layout->addStretch();
    return surface;
}

QWidget* makeIconResourceScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Official resource icons"), surface));

    auto* grid = new QWidget(surface);
    auto* gridLayout = new QGridLayout(grid);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setHorizontalSpacing(28);
    gridLayout->setVerticalSpacing(18);

    const QStringList iconNames = {
        QStringLiteral("HomeOutlined"),
        QStringLiteral("UserOutlined"),
        QStringLiteral("SearchOutlined"),
        QStringLiteral("SettingOutlined"),
        QStringLiteral("StarOutlined"),
        QStringLiteral("HeartOutlined"),
        QStringLiteral("AccountBookOutlined"),
        QStringLiteral("AccountBookTwoTone"),
        QStringLiteral("AimOutlined"),
        QStringLiteral("AlertFilled"),
        QStringLiteral("AlipayCircleFilled"),
        QStringLiteral("AlibabaOutlined"),
        QStringLiteral("AlignCenterOutlined"),
        QStringLiteral("AndroidOutlined"),
        QStringLiteral("AppleFilled"),
        QStringLiteral("DotNetOutlined"),
        QStringLiteral("TwitchFilled"),
        QStringLiteral("XFilled"),
    };

    const QColor twoToneColor = antTheme->tokens().colorPrimary;
    for (int i = 0; i < iconNames.size(); ++i)
    {
        auto* icon = new AntIcon(iconNames.at(i), grid);
        icon->setIconSize(24);
        icon->setFixedSize(28, 28);
        icon->setTwoToneColor(twoToneColor);
        gridLayout->addWidget(icon, i / 9, i % 9, Qt::AlignCenter);
    }

    layout->addWidget(grid);
    layout->addStretch();
    return surface;
}

QWidget* makeSelectionScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Selection controls"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(18);

    auto* checkbox = new AntCheckBox(row);
    checkbox->setText(QStringLiteral("Checked"));
    checkbox->setChecked(true);
    rowLayout->addWidget(checkbox);

    auto* radio = new AntRadio(row);
    radio->setText(QStringLiteral("Selected"));
    radio->setChecked(true);
    rowLayout->addWidget(radio);

    auto* sw = new AntSwitch(row);
    sw->setChecked(true);
    rowLayout->addWidget(sw);

    rowLayout->addStretch();
    layout->addWidget(row);
    layout->addStretch();
    return surface;
}

QWidget* makeInputScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Inputs"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);

    auto* input = new AntInput(row);
    input->setText(QStringLiteral("Search"));
    input->setSearchMode(true);
    input->setFixedWidth(150);
    rowLayout->addWidget(input);

    auto* number = new AntInputNumber(row);
    number->setValue(42);
    number->setControlsProgress(1.0);
    number->setFixedWidth(100);
    rowLayout->addWidget(number);

    auto* select = new AntSelect(row);
    select->addOption(QStringLiteral("Apple"), QStringLiteral("apple"));
    select->addOption(QStringLiteral("Orange"), QStringLiteral("orange"));
    select->setCurrentIndex(0);
    select->setFixedWidth(132);
    rowLayout->addWidget(select);

    auto* date = new AntDatePicker(row);
    date->setSelectedDate(QDate(2026, 6, 1));
    date->setFixedWidth(132);
    rowLayout->addWidget(date);

    auto* time = new AntTimePicker(row);
    time->setSelectedTime(QTime(10, 20, 30));
    time->setFixedWidth(118);
    rowLayout->addWidget(time);
    rowLayout->addStretch();
    layout->addWidget(row);
    layout->addStretch();
    return surface;
}

QWidget* makeNavigationScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Navigation"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(14);

    auto* menu = new AntMenu(row);
    menu->setMode(Ant::MenuMode::Vertical);
    menu->addItem(QStringLiteral("home"), QStringLiteral("Home"), Ant::IconType::Home);
    menu->addItem(QStringLiteral("docs"), QStringLiteral("Docs"), Ant::IconType::Mail);
    menu->addItem(QStringLiteral("settings"), QStringLiteral("Settings"), Ant::IconType::Setting);
    menu->setSelectedKey(QStringLiteral("docs"));
    menu->setFixedSize(180, 132);
    rowLayout->addWidget(menu);

    auto* right = new QWidget(row);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    auto* tabs = new AntTabs(right);
    tabs->addTab(coloredBox(antTheme->tokens().colorPrimaryBg, tabs), QStringLiteral("tab1"), QStringLiteral("Overview"));
    tabs->addTab(coloredBox(antTheme->tokens().colorSuccessBg, tabs), QStringLiteral("tab2"), QStringLiteral("Details"));
    tabs->setActiveKey(QStringLiteral("tab2"));
    tabs->setFixedSize(360, 112);
    rightLayout->addWidget(tabs);

    auto* pagination = new AntPagination(right);
    pagination->setTotal(180);
    pagination->setCurrent(4);
    pagination->setShowTotal(true);
    pagination->setShowSizeChanger(true);
    pagination->setFixedHeight(44);
    rightLayout->addWidget(pagination);

    rowLayout->addWidget(right, 1);
    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeFeedbackScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Feedback"), surface));

    auto* alert = new AntAlert(surface);
    alert->setAlertType(Ant::AlertType::Success);
    alert->setTitle(QStringLiteral("Saved"));
    alert->setDescription(QStringLiteral("Token colors and icon alignment stay visible."));
    alert->setShowIcon(true);
    alert->setFixedHeight(74);
    layout->addWidget(alert);

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);

    rowLayout->addWidget(new AntTag(QStringLiteral("Processing"), row));

    auto* successTag = new AntTag(QStringLiteral("Success"), row);
    successTag->setColor(QStringLiteral("success"));
    rowLayout->addWidget(successTag);

    auto* badge = new AntBadge(row);
    badge->setCount(18);
    badge->setFixedSize(64, 36);
    rowLayout->addWidget(badge);

    auto* progress = new AntProgress(row);
    progress->setPercent(68);
    progress->setFixedSize(220, 34);
    rowLayout->addWidget(progress);

    auto* tooltip = new AntToolTip(row);
    tooltip->setTitle(QStringLiteral("Tooltip"));
    tooltip->setFixedSize(120, 44);
    rowLayout->addWidget(tooltip);
    rowLayout->addStretch();

    layout->addWidget(row);
    layout->addStretch();
    return surface;
}

QWidget* makePopoverSurfaceScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Popover surfaces"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(14);

    auto* popoverStack = new QWidget(row);
    auto* popoverLayout = new QVBoxLayout(popoverStack);
    popoverLayout->setContentsMargins(0, 0, 0, 0);
    popoverLayout->setSpacing(10);

    auto* popover = new AntPopover(popoverStack);
    popover->setWindowFlags(Qt::Widget);
    popover->setTitle(QStringLiteral("Popover"));
    popover->setContent(QStringLiteral("Arrow, text and shadow remain crisp."));
    popover->setTitleIconType(Ant::IconType::InfoCircle);
    popover->setPlacement(Ant::TooltipPlacement::Top);
    popover->setFixedSize(popover->sizeHint());
    popover->show();
    popoverLayout->addWidget(popover, 0, Qt::AlignLeft);

    auto* actionPopover = new AntPopover(popoverStack);
    actionPopover->setWindowFlags(Qt::Widget);
    actionPopover->setTitle(QStringLiteral("Action"));
    actionPopover->setContent(QStringLiteral("Footer widgets keep palette tokens."));
    actionPopover->setPlacement(Ant::TooltipPlacement::Bottom);
    auto* actionButton = new AntButton(QStringLiteral("OK"), actionPopover);
    actionButton->setButtonType(Ant::ButtonType::Primary);
    actionPopover->setActionWidget(actionButton);
    actionPopover->setFixedSize(actionPopover->sizeHint());
    actionPopover->show();
    popoverLayout->addWidget(actionPopover, 0, Qt::AlignLeft);
    popoverLayout->addStretch();
    rowLayout->addWidget(popoverStack);

    auto* right = coloredBox(antTheme->tokens().colorBgContainer, row);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(18, 18, 18, 18);
    rightLayout->setSpacing(12);
    rightLayout->addWidget(new AntTypography(QStringLiteral("Static popovers verify arrow geometry, shadow margin and action layout."), right));
    rightLayout->addStretch();
    rowLayout->addWidget(right, 1);

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeModalSurfaceScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Modal surface"), surface));

    auto* modalTarget = coloredBox(antTheme->tokens().colorPrimaryBg, surface);
    modalTarget->setMinimumHeight(210);
    auto* modalTargetLayout = new QVBoxLayout(modalTarget);
    modalTargetLayout->setContentsMargins(16, 16, 16, 16);
    modalTargetLayout->addWidget(new AntTypography(QStringLiteral("Modal host"), modalTarget));
    modalTargetLayout->addStretch();
    layout->addWidget(modalTarget, 1);

    auto* modal = new AntModal(modalTarget);
    modal->setObjectName(QStringLiteral("popupModal"));
    modal->setTitle(QStringLiteral("Confirm update"));
    modal->setContent(QStringLiteral("Qt5 must keep the same mask, rounded card, shadow and footer spacing."));
    modal->setDialogWidth(360);
    modal->setCommandIconType(Ant::IconType::InfoCircle);
    modal->setMaskClosable(false);
    modal->setOkText(QStringLiteral("Apply"));
    modal->setCancelText(QStringLiteral("Cancel"));

    return surface;
}

QWidget* makeDrawerSurfaceScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Drawer surface"), surface));

    auto* drawerTarget = coloredBox(antTheme->tokens().colorSuccessBg, surface);
    drawerTarget->setMinimumHeight(190);
    auto* drawerTargetLayout = new QVBoxLayout(drawerTarget);
    drawerTargetLayout->setContentsMargins(16, 16, 16, 16);
    drawerTargetLayout->addWidget(new AntTypography(QStringLiteral("Drawer host"), drawerTarget));
    drawerTargetLayout->addStretch();
    layout->addWidget(drawerTarget, 1);

    auto* drawer = new AntDrawer(drawerTarget);
    drawer->setObjectName(QStringLiteral("popupDrawer"));
    drawer->setTitle(QStringLiteral("Drawer"));
    drawer->setPlacement(Ant::DrawerPlacement::Right);
    drawer->setDrawerWidth(220);
    drawer->setMaskClosable(false);
    auto* drawerBody = new QWidget;
    auto* drawerBodyLayout = new QVBoxLayout(drawerBody);
    drawerBodyLayout->setContentsMargins(0, 0, 0, 0);
    drawerBodyLayout->setSpacing(10);
    drawerBodyLayout->addWidget(new AntTypography(QStringLiteral("Panel content"), drawerBody));
    drawerBodyLayout->addWidget(new AntButton(QStringLiteral("Primary action"), drawerBody));
    drawerBodyLayout->addStretch();
    drawer->setBodyWidget(drawerBody);

    return surface;
}

QWidget* makeDropdownSurfaceScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Dropdown surface"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(14);

    auto* trigger = new AntButton(QStringLiteral("Actions"), row);
    trigger->setButtonType(Ant::ButtonType::Primary);
    trigger->setButtonIconType(Ant::IconType::Down);
    trigger->setFixedWidth(126);
    rowLayout->addWidget(trigger, 0, Qt::AlignTop);

    auto* info = coloredBox(antTheme->tokens().colorBgContainer, row);
    auto* infoLayout = new QVBoxLayout(info);
    infoLayout->setContentsMargins(18, 18, 18, 18);
    infoLayout->addWidget(new AntTypography(QStringLiteral("Dropdown popup is rendered as its real top-level tooltip window."), info));
    infoLayout->addStretch();
    rowLayout->addWidget(info, 1);

    auto* dropdown = new AntDropdown(surface);
    dropdown->setObjectName(QStringLiteral("dropdownController"));
    dropdown->setTarget(trigger);
    dropdown->setTrigger(Ant::DropdownTrigger::Click);
    dropdown->setPlacement(Ant::DropdownPlacement::BottomLeft);
    dropdown->setArrowVisible(true);
    dropdown->addItem(QStringLiteral("edit"), QStringLiteral("Edit"));
    dropdown->addItem(QStringLiteral("copy"), QStringLiteral("Copy"));
    dropdown->addDivider();
    dropdown->addItem(QStringLiteral("delete"), QStringLiteral("Delete"));

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makePopconfirmSurfaceScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Popconfirm surface"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(14);

    auto* trigger = new AntButton(QStringLiteral("Delete"), row);
    trigger->setButtonType(Ant::ButtonType::Default);
    trigger->setDanger(true);
    trigger->setFixedWidth(112);
    rowLayout->addWidget(trigger, 0, Qt::AlignTop);

    auto* info = coloredBox(antTheme->tokens().colorBgContainer, row);
    auto* infoLayout = new QVBoxLayout(info);
    infoLayout->setContentsMargins(18, 18, 18, 18);
    infoLayout->addWidget(new AntTypography(QStringLiteral("Popconfirm reuses Popover but adds warning icon and action buttons."), info));
    infoLayout->addStretch();
    rowLayout->addWidget(info, 1);

    auto* popconfirm = new AntPopconfirm(surface);
    popconfirm->setObjectName(QStringLiteral("popconfirmController"));
    popconfirm->setTarget(trigger);
    popconfirm->setTitle(QStringLiteral("Delete item?"));
    popconfirm->setDescription(QStringLiteral("This action cannot be undone."));
    popconfirm->setOkText(QStringLiteral("Delete"));
    popconfirm->setCancelText(QStringLiteral("Cancel"));
    popconfirm->setPlacement(Ant::TooltipPlacement::BottomLeft);

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeTourSurfaceScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Tour overlay"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(16);

    auto* card = coloredBox(antTheme->tokens().colorBgContainer, row);
    card->setFixedSize(240, 122);
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(18, 18, 18, 18);
    auto* target = new AntButton(QStringLiteral("Start here"), card);
    target->setButtonType(Ant::ButtonType::Primary);
    target->setFixedWidth(132);
    cardLayout->addWidget(target, 0, Qt::AlignLeft);
    cardLayout->addWidget(new AntTypography(QStringLiteral("Spotlight target"), card));
    cardLayout->addStretch();
    rowLayout->addWidget(card);

    auto* side = coloredBox(antTheme->tokens().colorSuccessBg, row);
    auto* sideLayout = new QVBoxLayout(side);
    sideLayout->setContentsMargins(18, 18, 18, 18);
    sideLayout->addWidget(new AntTypography(QStringLiteral("Mask and tooltip are captured from the real QDialog."), side));
    sideLayout->addStretch();
    rowLayout->addWidget(side, 1);

    auto* tour = new AntTour(surface);
    tour->setObjectName(QStringLiteral("tourController"));
    AntTourStep step;
    step.target = target;
    step.title = QStringLiteral("Guided step");
    step.description = QStringLiteral("Qt5 mask and spotlight stay aligned.");
    tour->addStep(step);

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeInteractionStateScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Interaction states"), surface));

    auto* topRow = new QWidget(surface);
    auto* topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(10);

    auto* hoverButton = new AntButton(QStringLiteral("Hover"), topRow);
    hoverButton->setObjectName(QStringLiteral("interactionHoverButton"));
    hoverButton->setButtonType(Ant::ButtonType::Primary);
    topLayout->addWidget(hoverButton);

    auto* pressedButton = new AntButton(QStringLiteral("Pressed"), topRow);
    pressedButton->setObjectName(QStringLiteral("interactionPressedButton"));
    pressedButton->setButtonType(Ant::ButtonType::Default);
    topLayout->addWidget(pressedButton);

    auto* focusedInput = new AntInput(topRow);
    focusedInput->setObjectName(QStringLiteral("interactionFocusedInput"));
    focusedInput->setText(QStringLiteral("Focused"));
    focusedInput->setStatus(Ant::Status::Warning);
    focusedInput->setAllowClear(true);
    focusedInput->setFixedWidth(152);
    topLayout->addWidget(focusedInput);

    auto* hoverCheckBox = new AntCheckBox(QStringLiteral("Hover"), topRow);
    hoverCheckBox->setObjectName(QStringLiteral("interactionHoverCheckBox"));
    hoverCheckBox->setChecked(true);
    topLayout->addWidget(hoverCheckBox);

    auto* pressedRadio = new AntRadio(QStringLiteral("Pressed"), topRow);
    pressedRadio->setObjectName(QStringLiteral("interactionPressedRadio"));
    topLayout->addWidget(pressedRadio);

    auto* slider = new AntSlider(topRow);
    slider->setObjectName(QStringLiteral("interactionFocusedSlider"));
    slider->setValue(42);
    slider->setFixedSize(220, 50);
    topLayout->addWidget(slider);
    topLayout->addStretch();
    layout->addWidget(topRow);

    auto* bottomRow = new QWidget(surface);
    auto* bottomLayout = new QHBoxLayout(bottomRow);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(12);

    auto* menu = new AntMenu(bottomRow);
    menu->setObjectName(QStringLiteral("interactionMenu"));
    menu->setMode(Ant::MenuMode::Vertical);
    menu->addItem(QStringLiteral("home"), QStringLiteral("Home"), Ant::IconType::Home);
    menu->addItem(QStringLiteral("docs"), QStringLiteral("Docs"), Ant::IconType::Mail);
    menu->addItem(QStringLiteral("settings"), QStringLiteral("Settings"), Ant::IconType::Setting);
    menu->setSelectedKey(QStringLiteral("home"));
    menu->setFixedSize(180, 112);
    bottomLayout->addWidget(menu);

    auto* centerStack = new QWidget(bottomRow);
    auto* centerLayout = new QVBoxLayout(centerStack);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(10);

    auto* tabs = new AntTabs(centerStack);
    tabs->setObjectName(QStringLiteral("interactionTabs"));
    tabs->addTab(coloredBox(antTheme->tokens().colorPrimaryBg, tabs), QStringLiteral("tab1"), QStringLiteral("Overview"));
    tabs->addTab(coloredBox(antTheme->tokens().colorSuccessBg, tabs), QStringLiteral("tab2"), QStringLiteral("Details"));
    tabs->setFixedSize(300, 82);
    centerLayout->addWidget(tabs);

    auto* pagination = new AntPagination(centerStack);
    pagination->setObjectName(QStringLiteral("interactionPagination"));
    pagination->setTotal(90);
    pagination->setCurrent(3);
    pagination->setShowQuickJumper(true);
    pagination->setFixedHeight(40);
    centerLayout->addWidget(pagination);
    bottomLayout->addWidget(centerStack);

    auto* table = new AntTable(bottomRow);
    table->setObjectName(QStringLiteral("interactionTable"));
    table->setRowSelection(Ant::TableSelectionMode::Checkbox);
    table->setPageSize(3);
    table->setColumns({{QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 110},
                       {QStringLiteral("State"), QStringLiteral("state"), QStringLiteral("state"), 96}});
    AntTableRow selected;
    selected.data.insert(QStringLiteral("name"), QStringLiteral("Alice"));
    selected.data.insert(QStringLiteral("state"), QStringLiteral("Active"));
    selected.selected = true;
    AntTableRow hovered;
    hovered.data.insert(QStringLiteral("name"), QStringLiteral("Bob"));
    hovered.data.insert(QStringLiteral("state"), QStringLiteral("Hover"));
    table->setRows({selected, hovered});
    table->setFixedSize(270, 134);
    bottomLayout->addWidget(table);

    bottomLayout->addStretch();
    layout->addWidget(bottomRow);
    layout->addStretch();
    return surface;
}

QWidget* makeAcceptanceStateMatrixScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface, 14, 10);
    layout->addWidget(makeLabel(QStringLiteral("Acceptance state matrix"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(12);

    auto* controlsPanel = coloredBox(antTheme->tokens().colorBgContainer, row);
    auto* controlsLayout = new QVBoxLayout(controlsPanel);
    controlsLayout->setContentsMargins(14, 12, 14, 12);
    controlsLayout->setSpacing(10);

    auto* buttonRow = new QWidget(controlsPanel);
    auto* buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(8);

    auto* primary = new AntButton(QStringLiteral("Primary"), buttonRow);
    primary->setButtonType(Ant::ButtonType::Primary);
    primary->setObjectName(QStringLiteral("acceptanceHoverButton"));
    buttonLayout->addWidget(primary);

    auto* danger = new AntButton(QStringLiteral("Danger"), buttonRow);
    danger->setDanger(true);
    buttonLayout->addWidget(danger);

    auto* disabled = new AntButton(QStringLiteral("Disabled"), buttonRow);
    disabled->setEnabled(false);
    buttonLayout->addWidget(disabled);
    buttonLayout->addStretch();
    controlsLayout->addWidget(buttonRow);

    auto* selectionRow = new QWidget(controlsPanel);
    auto* selectionLayout = new QHBoxLayout(selectionRow);
    selectionLayout->setContentsMargins(0, 0, 0, 0);
    selectionLayout->setSpacing(12);

    auto* checked = new AntCheckBox(QStringLiteral("Checked"), selectionRow);
    checked->setChecked(true);
    selectionLayout->addWidget(checked);

    auto* disabledCheck = new AntCheckBox(QStringLiteral("Disabled"), selectionRow);
    disabledCheck->setChecked(true);
    disabledCheck->setEnabled(false);
    selectionLayout->addWidget(disabledCheck);

    auto* radio = new AntRadio(QStringLiteral("Selected"), selectionRow);
    radio->setChecked(true);
    selectionLayout->addWidget(radio);

    auto* disabledRadio = new AntRadio(QStringLiteral("Off"), selectionRow);
    disabledRadio->setEnabled(false);
    selectionLayout->addWidget(disabledRadio);
    selectionLayout->addStretch();
    controlsLayout->addWidget(selectionRow);

    auto* inputRow = new QWidget(controlsPanel);
    auto* inputLayout = new QHBoxLayout(inputRow);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(8);

    auto* input = new AntInput(inputRow);
    input->setObjectName(QStringLiteral("acceptanceFocusedInput"));
    input->setText(QStringLiteral("Focused error"));
    input->setStatus(Ant::Status::Error);
    input->setAllowClear(true);
    input->setFixedWidth(156);
    inputLayout->addWidget(input);

    auto* disabledInput = new AntInput(inputRow);
    disabledInput->setText(QStringLiteral("Disabled"));
    disabledInput->setEnabled(false);
    disabledInput->setFixedWidth(132);
    inputLayout->addWidget(disabledInput);
    inputLayout->addStretch();
    controlsLayout->addWidget(inputRow);

    auto* feedbackRow = new QWidget(controlsPanel);
    auto* feedbackLayout = new QHBoxLayout(feedbackRow);
    feedbackLayout->setContentsMargins(0, 0, 0, 0);
    feedbackLayout->setSpacing(10);

    auto* progress = new AntProgress(feedbackRow);
    progress->setPercent(72);
    progress->setStatus(Ant::ProgressStatus::Active);
    progress->setFixedSize(190, 34);
    feedbackLayout->addWidget(progress);

    auto* tag = new AntTag(QStringLiteral("success"), feedbackRow);
    tag->setColor(QStringLiteral("success"));
    feedbackLayout->addWidget(tag);

    auto* badge = new AntBadge(feedbackRow);
    badge->setCount(23);
    badge->setFixedSize(54, 34);
    feedbackLayout->addWidget(badge);
    feedbackLayout->addStretch();
    controlsLayout->addWidget(feedbackRow);

    auto* tooltip = new AntToolTip(controlsPanel);
    tooltip->setTitle(QStringLiteral("Tooltip surface"));
    tooltip->setFixedSize(150, 44);
    controlsLayout->addWidget(tooltip, 0, Qt::AlignLeft);
    controlsLayout->addStretch();
    rowLayout->addWidget(controlsPanel, 0);

    auto* navigationPanel = coloredBox(antTheme->tokens().colorBgContainer, row);
    auto* navigationLayout = new QVBoxLayout(navigationPanel);
    navigationLayout->setContentsMargins(14, 12, 14, 12);
    navigationLayout->setSpacing(10);

    auto* menu = new AntMenu(navigationPanel);
    menu->setObjectName(QStringLiteral("acceptanceMenu"));
    menu->setMode(Ant::MenuMode::Vertical);
    menu->addItem(QStringLiteral("dashboard"), QStringLiteral("Dashboard"), Ant::IconType::Home);
    menu->addItem(QStringLiteral("reports"), QStringLiteral("Reports"), Ant::IconType::Mail);
    menu->addItem(QStringLiteral("settings"), QStringLiteral("Settings"), Ant::IconType::Setting);
    menu->setSelectedKey(QStringLiteral("reports"));
    menu->setFixedSize(190, 126);
    navigationLayout->addWidget(menu);

    auto* tabs = new AntTabs(navigationPanel);
    tabs->setObjectName(QStringLiteral("acceptanceTabs"));
    tabs->addTab(coloredBox(antTheme->tokens().colorPrimaryBg, tabs), QStringLiteral("one"), QStringLiteral("Overview"));
    tabs->addTab(coloredBox(antTheme->tokens().colorSuccessBg, tabs), QStringLiteral("two"), QStringLiteral("Activity"));
    tabs->setActiveKey(QStringLiteral("two"));
    tabs->setFixedSize(330, 88);
    navigationLayout->addWidget(tabs);

    auto* pagination = new AntPagination(navigationPanel);
    pagination->setObjectName(QStringLiteral("acceptancePagination"));
    pagination->setTotal(120);
    pagination->setCurrent(5);
    pagination->setShowQuickJumper(true);
    pagination->setFixedHeight(42);
    navigationLayout->addWidget(pagination);
    navigationLayout->addStretch();
    rowLayout->addWidget(navigationPanel, 1);

    auto* dataPanel = coloredBox(antTheme->tokens().colorBgContainer, row);
    auto* dataLayout = new QVBoxLayout(dataPanel);
    dataLayout->setContentsMargins(14, 12, 14, 12);
    dataLayout->setSpacing(10);

    auto* listAndTreeRow = new QWidget(dataPanel);
    auto* listAndTreeLayout = new QHBoxLayout(listAndTreeRow);
    listAndTreeLayout->setContentsMargins(0, 0, 0, 0);
    listAndTreeLayout->setSpacing(10);

    auto* list = new AntList(listAndTreeRow);
    list->setBordered(true);
    list->setSplit(true);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    auto* firstItem = new AntListItem;
    firstItem->setText(QStringLiteral("Normal row"));
    firstItem->setIcon(Ant::IconType::Edit);
    list->addItem(firstItem);
    auto* selectedItem = new AntListItem;
    selectedItem->setText(QStringLiteral("Selected row"));
    selectedItem->setIcon(Ant::IconType::CheckCircle);
    list->addItem(selectedItem);
    list->setCurrentRow(1);
    list->setFixedSize(190, 116);
    listAndTreeLayout->addWidget(list);

    auto* tree = new AntTree(listAndTreeRow);
    AntTreeNode root;
    root.key = QStringLiteral("root");
    root.title = QStringLiteral("Root");
    root.expanded = true;
    AntTreeNode child;
    child.key = QStringLiteral("child");
    child.title = QStringLiteral("Checked child");
    child.checked = true;
    AntTreeNode leaf;
    leaf.key = QStringLiteral("leaf");
    leaf.title = QStringLiteral("Leaf");
    child.children.append(leaf);
    root.children.append(child);
    tree->setTreeData({root});
    tree->setShowLine(true);
    tree->setCheckedKeys({QStringLiteral("child")});
    tree->setSelectedKeys({QStringLiteral("child")});
    tree->setFixedSize(210, 116);
    listAndTreeLayout->addWidget(tree);
    dataLayout->addWidget(listAndTreeRow);

    auto* bottomDataRow = new QWidget(dataPanel);
    auto* bottomDataLayout = new QHBoxLayout(bottomDataRow);
    bottomDataLayout->setContentsMargins(0, 0, 0, 0);
    bottomDataLayout->setSpacing(10);

    auto* table = new AntTable(bottomDataRow);
    table->setObjectName(QStringLiteral("acceptanceTable"));
    table->setRowSelection(Ant::TableSelectionMode::Checkbox);
    table->setPageSize(3);
    table->setColumns({{QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 116},
                       {QStringLiteral("State"), QStringLiteral("state"), QStringLiteral("state"), 96}});
    AntTableRow selectedRow;
    selectedRow.data.insert(QStringLiteral("name"), QStringLiteral("Ada"));
    selectedRow.data.insert(QStringLiteral("state"), QStringLiteral("Selected"));
    selectedRow.selected = true;
    AntTableRow normalRow;
    normalRow.data.insert(QStringLiteral("name"), QStringLiteral("Grace"));
    normalRow.data.insert(QStringLiteral("state"), QStringLiteral("Normal"));
    table->setRows({selectedRow, normalRow});
    table->setFixedSize(280, 138);
    bottomDataLayout->addWidget(table);

    auto* bar = new AntScrollBar(Qt::Vertical, bottomDataRow);
    bar->setObjectName(QStringLiteral("acceptanceScrollBar"));
    bar->setAutoHide(false);
    bar->setRange(0, 100);
    bar->setValue(48);
    bar->setFixedHeight(138);
    bottomDataLayout->addWidget(bar);
    bottomDataLayout->addStretch();
    dataLayout->addWidget(bottomDataRow);
    dataLayout->addStretch();
    rowLayout->addWidget(dataPanel, 1);

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeDataDisplayScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Data display"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(12);

    auto* list = new AntList(row);
    list->setBordered(true);
    list->setSplit(true);
    for (const QString& text : {QStringLiteral("First item"), QStringLiteral("Second item")})
    {
        auto* item = new AntListItem;
        item->setContentWidget(new AntTypography(text));
        list->addItem(item);
    }
    list->setFixedSize(220, 128);
    rowLayout->addWidget(list);

    auto* table = new AntTable(row);
    table->setRowSelection(Ant::TableSelectionMode::Checkbox);
    table->setPageSize(4);
    table->setColumns({{QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 132},
                       {QStringLiteral("Role"), QStringLiteral("role"), QStringLiteral("role"), 132}});
    AntTableRow selected;
    selected.data.insert(QStringLiteral("name"), QStringLiteral("Alice"));
    selected.data.insert(QStringLiteral("role"), QStringLiteral("Designer"));
    selected.selected = true;
    AntTableRow normal;
    normal.data.insert(QStringLiteral("name"), QStringLiteral("Bob"));
    normal.data.insert(QStringLiteral("role"), QStringLiteral("Engineer"));
    table->setRows({selected, normal});
    table->setFixedSize(320, 152);
    rowLayout->addWidget(table);

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeTreeScene()
{
    auto* tree = new AntTree;
    AntTreeNode root;
    root.key = QStringLiteral("root");
    root.title = QStringLiteral("Root");
    root.expanded = true;
    AntTreeNode child;
    child.key = QStringLiteral("child");
    child.title = QStringLiteral("Child");
    child.checked = true;
    root.children.append(child);
    tree->setTreeData({root});
    tree->setShowLine(true);
    tree->setCheckedKeys({QStringLiteral("child")});
    return tree;
}

QWidget* makeScrollAndCarouselScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Scroll and carousel"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(14);

    auto* scrollArea = new AntScrollArea(row);
    scrollArea->setAutoHideScrollBar(false);
    scrollArea->setWidgetResizable(false);
    auto* content = new QWidget;
    applySurfacePalette(content, antTheme->tokens().colorBgContainer);
    content->setFixedSize(360, 260);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(12, 12, 12, 12);
    contentLayout->setSpacing(8);
    for (int i = 0; i < 7; ++i)
    {
        contentLayout->addWidget(new AntTypography(QStringLiteral("Scrollable row %1").arg(i + 1), content));
    }
    scrollArea->setWidget(content);
    scrollArea->setFixedSize(220, 132);
    scrollArea->verticalScrollBar()->setValue(42);
    rowLayout->addWidget(scrollArea);

    auto* bar = new AntScrollBar(Qt::Vertical, row);
    bar->setAutoHide(false);
    bar->setRange(0, 100);
    bar->setValue(45);
    bar->setFixedHeight(132);
    rowLayout->addWidget(bar);

    auto* carousel = new AntCarousel(row);
    carousel->setAutoPlay(false);
    carousel->setShowArrows(true);
    carousel->addSlide(coloredBox(antTheme->tokens().colorPrimary, carousel));
    carousel->addSlide(coloredBox(antTheme->tokens().colorSuccess, carousel));
    carousel->setFixedSize(260, 132);
    rowLayout->addWidget(carousel);
    rowLayout->addStretch();

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeDesktopExtensionScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface);
    layout->addWidget(makeLabel(QStringLiteral("Desktop extensions"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(14);

    auto* nav = new AntNav(row);
    nav->addCategory(QStringLiteral("Main"));
    const int overviewIndex = nav->addItem(QStringLiteral("Overview"));
    nav->setItemIcon(overviewIndex, Ant::IconType::Home);
    const int filesIndex = nav->addItem(QStringLiteral("Files"));
    nav->setItemIcon(filesIndex, Ant::IconType::Setting);
    nav->setCurrentIndex(1);
    nav->setFixedSize(180, 132);
    rowLayout->addWidget(nav);

    auto* dialog = new AntDialog(row);
    dialog->setWindowTitle(QStringLiteral("Dialog"));
    dialog->setFixedSize(260, 148);
    auto* dialogLayout = new QVBoxLayout(dialog->contentWidget());
    dialogLayout->setContentsMargins(18, 14, 18, 14);
    dialogLayout->addWidget(new AntTypography(QStringLiteral("Dialog body"), dialog->contentWidget()));
    rowLayout->addWidget(dialog);

    auto* inputDialog = new AntInputDialog(row);
    inputDialog->setWindowTitle(QStringLiteral("InputDialog"));
    inputDialog->setLabelText(QStringLiteral("Name"));
    inputDialog->setTextValue(QStringLiteral("qt-ant-design"));
    inputDialog->setFixedSize(300, 148);
    rowLayout->addWidget(inputDialog);

    auto* floatButton = new AntFloatButton(row);
    floatButton->setIcon(QStringLiteral("plus"));
    rowLayout->addWidget(floatButton, 0, Qt::AlignTop);
    rowLayout->addStretch();

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeAdvancedEntryScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface, 14, 10);
    layout->addWidget(makeLabel(QStringLiteral("Advanced entry"), surface));

    auto* firstRow = new QWidget(surface);
    auto* firstLayout = new QHBoxLayout(firstRow);
    firstLayout->setContentsMargins(0, 0, 0, 0);
    firstLayout->setSpacing(10);

    auto* autocomplete = new AntAutoComplete(firstRow);
    autocomplete->setText(QStringLiteral("app"));
    autocomplete->addSuggestion(QStringLiteral("Apple"), QStringLiteral("apple"));
    autocomplete->setFixedWidth(132);
    firstLayout->addWidget(autocomplete);

    auto* mentions = new AntMentions(firstRow);
    mentions->setPlaceholderText(QStringLiteral("@alice"));
    mentions->addSuggestion(QStringLiteral("alice"));
    mentions->setFixedWidth(132);
    firstLayout->addWidget(mentions);

    auto* cascader = new AntCascader(firstRow);
    AntCascaderOption option;
    option.value = QStringLiteral("zhejiang");
    option.label = QStringLiteral("Zhejiang");
    cascader->setOptions({option});
    cascader->setFixedWidth(132);
    firstLayout->addWidget(cascader);

    auto* treeSelect = new AntTreeSelect(firstRow);
    AntTreeNode treeNode;
    treeNode.key = QStringLiteral("root");
    treeNode.title = QStringLiteral("Root");
    treeSelect->setTreeData({treeNode});
    treeSelect->setValue({QStringLiteral("root")});
    treeSelect->setFixedWidth(132);
    firstLayout->addWidget(treeSelect);

    auto* colorPicker = new AntColorPicker(firstRow);
    colorPicker->setCurrentColor(QColor(0x16, 0x77, 0xff));
    firstLayout->addWidget(colorPicker);
    firstLayout->addStretch();
    layout->addWidget(firstRow);

    auto* secondRow = new QWidget(surface);
    auto* secondLayout = new QHBoxLayout(secondRow);
    secondLayout->setContentsMargins(0, 0, 0, 0);
    secondLayout->setSpacing(14);

    auto* slider = new AntSlider(secondRow);
    slider->setRangeMode(true);
    slider->setRangeValues(24, 72);
    slider->setMarks({{0, QStringLiteral("0")}, {50, QStringLiteral("50")}, {100, QStringLiteral("100")}});
    slider->setFixedSize(260, 54);
    secondLayout->addWidget(slider);

    auto* rate = new AntRate(secondRow);
    rate->setAllowHalf(true);
    rate->setValue(3.5);
    secondLayout->addWidget(rate);

    auto* segmented = new AntSegmented(secondRow);
    segmented->setOptions({{QStringLiteral("daily"), QStringLiteral("Daily")},
                           {QStringLiteral("weekly"), QStringLiteral("Weekly")},
                           {QStringLiteral("monthly"), QStringLiteral("Monthly")}});
    segmented->setCurrentIndex(1);
    secondLayout->addWidget(segmented);

    secondLayout->addStretch();
    layout->addWidget(secondRow);

    auto* thirdRow = new QWidget(surface);
    auto* thirdLayout = new QHBoxLayout(thirdRow);
    thirdLayout->setContentsMargins(0, 0, 0, 0);
    thirdLayout->setSpacing(12);

    auto* transfer = new AntTransfer(thirdRow);
    transfer->setSourceItems({QStringLiteral("Alpha"), QStringLiteral("Beta"), QStringLiteral("Gamma")});
    transfer->setFixedSize(420, 174);
    thirdLayout->addWidget(transfer);

    auto* upload = new AntUpload(thirdRow);
    AntUploadFile done;
    done.uid = QStringLiteral("done");
    done.name = QStringLiteral("report.pdf");
    done.status = Ant::UploadFileStatus::Done;
    done.percent = 100;
    AntUploadFile uploading;
    uploading.uid = QStringLiteral("uploading");
    uploading.name = QStringLiteral("image.png");
    uploading.status = Ant::UploadFileStatus::Uploading;
    uploading.percent = 46;
    upload->setFileList({done, uploading});
    upload->setFixedSize(260, 108);
    thirdLayout->addWidget(upload, 0, Qt::AlignTop);
    thirdLayout->addStretch();
    layout->addWidget(thirdRow, 1);

    return surface;
}

QWidget* makeRichDisplayScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface, 14, 10);
    layout->addWidget(makeLabel(QStringLiteral("Rich display"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(12);

    auto* left = new QWidget(row);
    auto* leftLayout = new QVBoxLayout(left);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);

    auto* avatarGroup = new AntAvatarGroup(left);
    avatarGroup->addAvatar(new AntAvatar(QStringLiteral("A")));
    avatarGroup->addAvatar(new AntAvatar(QStringLiteral("B")));
    avatarGroup->addAvatar(new AntAvatar(QStringLiteral("C")));
    leftLayout->addWidget(avatarGroup);

    auto* card = new AntCard(QStringLiteral("Profile"), left);
    card->setExtra(QStringLiteral("More"));
    card->setFixedSize(210, 116);
    auto* cardBody = new QVBoxLayout(card);
    cardBody->setContentsMargins(18, 54, 18, 12);
    cardBody->addWidget(new AntTypography(QStringLiteral("Designer"), card));
    leftLayout->addWidget(card);

    auto* statistic = new AntStatistic(QStringLiteral("Revenue"), left);
    statistic->setPrefix(QStringLiteral("$"));
    statistic->setValue(1234.56);
    statistic->setPrecision(2);
    leftLayout->addWidget(statistic);
    rowLayout->addWidget(left);

    auto* calendar = new AntCalendar(row);
    calendar->setSelectedDate(QDate(2026, 6, 1));
    calendar->setFixedSize(270, 246);
    rowLayout->addWidget(calendar);

    auto* right = new QWidget(row);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    auto* descriptions = new AntDescriptions(right);
    descriptions->setTitle(QStringLiteral("User"));
    descriptions->addItem(QStringLiteral("Name"), QStringLiteral("Alice"));
    descriptions->addItem(QStringLiteral("Role"), QStringLiteral("Designer"));
    descriptions->setFixedSize(290, 108);
    rightLayout->addWidget(descriptions);

    auto* displayRow = new QWidget(right);
    auto* displayRowLayout = new QHBoxLayout(displayRow);
    displayRowLayout->setContentsMargins(0, 0, 0, 0);
    displayRowLayout->setSpacing(10);

    auto* empty = new AntEmpty(displayRow);
    empty->setFixedSize(132, 114);
    displayRowLayout->addWidget(empty);

    auto* qr = new AntQRCode(displayRow);
    qr->setValue(QStringLiteral("https://github.com/sorrowfeng/qt-ant-design"));
    qr->setFixedSize(132, 132);
    displayRowLayout->addWidget(qr);
    rightLayout->addWidget(displayRow);

    auto* timeline = new AntTimeline(right);
    timeline->addItem(QStringLiteral("Created"), QStringLiteral("09:00"));
    timeline->addItem(QStringLiteral("Published"), QStringLiteral("10:30"));
    timeline->setFixedSize(290, 96);
    rightLayout->addWidget(timeline);
    rowLayout->addWidget(right);

    auto* image = new AntImage(row);
    image->setSrc(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
    image->setImgWidth(112);
    image->setImgHeight(84);
    image->setFixedSize(132, 112);
    rowLayout->addWidget(image, 0, Qt::AlignTop);
    rowLayout->addStretch();

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeRichFeedbackScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface, 14, 10);
    layout->addWidget(makeLabel(QStringLiteral("Rich feedback"), surface));

    auto* row = new QWidget(surface);
    auto* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(12);

    auto* result = new AntResult(QStringLiteral("Success"), row);
    result->setSubTitle(QStringLiteral("Completed"));
    result->setFixedSize(250, 196);
    rowLayout->addWidget(result);

    auto* middle = new QWidget(row);
    auto* middleLayout = new QVBoxLayout(middle);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(10);

    auto* notification = new AntNotification(middle);
    notification->setTitle(QStringLiteral("Update"));
    notification->setDescription(QStringLiteral("Ready to install"));
    notification->setFixedSize(300, 108);
    middleLayout->addWidget(notification);

    auto* message = new AntMessage(middle);
    message->setText(QStringLiteral("Saved successfully"));
    message->setFixedSize(300, 56);
    middleLayout->addWidget(message);

    auto* spin = new AntSpin(middle);
    spin->setDescription(QStringLiteral("Loading"));
    spin->setFixedSize(120, 92);
    middleLayout->addWidget(spin, 0, Qt::AlignLeft);
    rowLayout->addWidget(middle);

    auto* right = new QWidget(row);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    auto* skeleton = new AntSkeleton(right);
    skeleton->setActive(true);
    skeleton->setFixedSize(300, 118);
    rightLayout->addWidget(skeleton);

    auto* collapse = new AntCollapse(right);
    auto* panel = collapse->addPanel(QStringLiteral("Panel"));
    panel->setExpanded(true);
    collapse->setFixedSize(300, 116);
    rightLayout->addWidget(collapse);
    rowLayout->addWidget(right);
    rowLayout->addStretch();

    layout->addWidget(row, 1);
    return surface;
}

QWidget* makeLayoutExtensionScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface, 14, 10);
    layout->addWidget(makeLabel(QStringLiteral("Layout and extensions"), surface));

    auto* topRow = new QWidget(surface);
    auto* topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(12);

    auto* breadcrumb = new AntBreadcrumb(topRow);
    breadcrumb->addItem(QStringLiteral("Home"), QStringLiteral("/"));
    breadcrumb->addItem(QStringLiteral("Library"), QStringLiteral("/library"));
    breadcrumb->addItem(QStringLiteral("Item"), QStringLiteral("/library/item"));
    breadcrumb->setFixedSize(260, 38);
    topLayout->addWidget(breadcrumb);

    auto* steps = new AntSteps(topRow);
    steps->addStep(QStringLiteral("Login"));
    steps->addStep(QStringLiteral("Confirm"));
    steps->addStep(QStringLiteral("Done"));
    steps->setCurrentIndex(1);
    steps->setFixedSize(390, 78);
    topLayout->addWidget(steps);

    auto* divider = new AntDivider(QStringLiteral("OR"), topRow);
    divider->setFixedSize(160, 48);
    topLayout->addWidget(divider);
    topLayout->addStretch();
    layout->addWidget(topRow);

    auto* mainRow = new QWidget(surface);
    auto* mainLayout = new QHBoxLayout(mainRow);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    auto* antLayout = new AntLayout(mainRow);
    antLayout->setHeader(new AntLayoutHeader);
    antLayout->setContent(new AntLayoutContent);
    antLayout->setFooter(new AntLayoutFooter);
    antLayout->setFixedSize(260, 176);
    mainLayout->addWidget(antLayout);

    auto* splitter = new AntSplitter(mainRow);
    splitter->addWidget(coloredBox(antTheme->tokens().colorPrimaryBg, splitter));
    splitter->addWidget(coloredBox(antTheme->tokens().colorSuccessBg, splitter));
    splitter->setFixedSize(240, 118);
    mainLayout->addWidget(splitter, 0, Qt::AlignTop);

    auto* stack = new AntStackedWidget(mainRow);
    stack->addWidget(coloredBox(antTheme->tokens().colorPrimaryBg, stack));
    stack->addWidget(coloredBox(antTheme->tokens().colorWarningBg, stack));
    stack->setCurrentIndex(1);
    stack->setFixedSize(220, 118);
    mainLayout->addWidget(stack, 0, Qt::AlignTop);

    auto* right = new QWidget(mainRow);
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    auto* toolbar = new AntToolBar(right);
    toolbar->addAction(QStringLiteral("New"));
    toolbar->addAction(QStringLiteral("Save"));
    toolbar->setFixedSize(220, 42);
    rightLayout->addWidget(toolbar);

    auto* toolButton = new AntToolButton(QStringLiteral("Tool"), right);
    rightLayout->addWidget(toolButton);

    auto* textEdit = new AntPlainTextEdit(right);
    textEdit->setPlainText(QStringLiteral("Plain text"));
    textEdit->setFixedSize(220, 86);
    rightLayout->addWidget(textEdit);
    mainLayout->addWidget(right);
    mainLayout->addStretch();
    layout->addWidget(mainRow, 1);

    auto* bottomRow = new QWidget(surface);
    auto* bottomLayout = new QHBoxLayout(bottomRow);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(12);

    auto* status = new AntStatusBar(bottomRow);
    status->setMessage(QStringLiteral("Ready"));
    status->addPermanentItem(QStringLiteral("Ln 1"));
    status->setFixedSize(320, 34);
    bottomLayout->addWidget(status);

    auto* log = new AntLog(bottomRow);
    log->info(QStringLiteral("Qt5 visual parity"));
    log->setFixedSize(360, 88);
    bottomLayout->addWidget(log);
    bottomLayout->addStretch();
    layout->addWidget(bottomRow);

    return surface;
}

QWidget* makeStructuralLayoutScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface, 14, 10);
    layout->addWidget(makeLabel(QStringLiteral("Structural layout"), surface));

    auto* topRow = new QWidget(surface);
    auto* topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(12);

    auto* anchor = new AntAnchor(topRow);
    anchor->addLink(QStringLiteral("Intro"), 0);
    anchor->addLink(QStringLiteral("API"), 120);
    anchor->addLink(QStringLiteral("Examples"), 240);
    anchor->setFixedSize(150, 102);
    topLayout->addWidget(anchor);

    auto* flex = new AntFlex(topRow);
    flex->setGap(8);
    flex->addWidget(new AntButton(QStringLiteral("Alpha"), flex));
    flex->addWidget(new AntButton(QStringLiteral("Beta"), flex));
    flex->addWidget(coloredBox(antTheme->tokens().colorPrimaryBg, flex));
    flex->setFixedSize(286, 102);
    topLayout->addWidget(flex);

    auto* space = new AntSpace(topRow);
    space->setSize(Ant::Size::Middle);
    space->addItem(new AntTag(QStringLiteral("One"), space));
    auto* checkedTag = new AntTag(QStringLiteral("Checked"), space);
    checkedTag->setChecked(true);
    space->addItem(checkedTag);
    space->addItem(new AntButton(QStringLiteral("Action"), space));
    space->setFixedSize(300, 102);
    topLayout->addWidget(space);
    topLayout->addStretch();
    layout->addWidget(topRow);

    auto* middleRow = new QWidget(surface);
    auto* middleLayout = new QHBoxLayout(middleRow);
    middleLayout->setContentsMargins(0, 0, 0, 0);
    middleLayout->setSpacing(12);

    auto* grid = new AntRow(middleRow);
    grid->setGutter(8);
    grid->addWidget(coloredBox(antTheme->tokens().colorPrimaryBg, grid), 8);
    grid->addWidget(coloredBox(antTheme->tokens().colorSuccessBg, grid), 8);
    grid->addWidget(coloredBox(antTheme->tokens().colorWarningBg, grid), 8);
    grid->setFixedSize(360, 96);
    middleLayout->addWidget(grid);

    auto* masonry = new AntMasonry(middleRow);
    masonry->setColumns(3);
    masonry->setSpacing(8);
    auto* masonryA = coloredBox(antTheme->tokens().colorPrimaryBg, masonry);
    masonryA->setFixedHeight(52);
    auto* masonryB = coloredBox(antTheme->tokens().colorSuccessBg, masonry);
    masonryB->setFixedHeight(72);
    auto* masonryC = coloredBox(antTheme->tokens().colorWarningBg, masonry);
    masonryC->setFixedHeight(60);
    masonry->addWidget(masonryA);
    masonry->addWidget(masonryB);
    masonry->addWidget(masonryC);
    masonry->setFixedSize(300, 112);
    middleLayout->addWidget(masonry);

    auto* antWidget = new AntWidget(middleRow);
    applySurfacePalette(antWidget, antTheme->tokens().colorBgContainer);
    auto* antWidgetLayout = new QVBoxLayout(antWidget);
    antWidgetLayout->setContentsMargins(12, 10, 12, 10);
    antWidgetLayout->addWidget(new AntTypography(QStringLiteral("AntWidget surface"), antWidget));
    antWidget->setFixedSize(220, 96);
    middleLayout->addWidget(antWidget);
    middleLayout->addStretch();
    layout->addWidget(middleRow);

    auto* bottomRow = new QWidget(surface);
    auto* bottomLayout = new QHBoxLayout(bottomRow);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(12);

    auto* form = new AntForm(bottomRow);
    form->addItem(QStringLiteral("Name"), new AntInput(form), true);
    auto* roleSelect = new AntSelect(form);
    roleSelect->addOption(QStringLiteral("Designer"), QStringLiteral("designer"));
    roleSelect->addOption(QStringLiteral("Engineer"), QStringLiteral("engineer"));
    roleSelect->setCurrentIndex(0);
    form->addItem(QStringLiteral("Role"), roleSelect);
    form->setFixedSize(390, 138);
    bottomLayout->addWidget(form);

    auto* watermark = new AntWatermark(bottomRow);
    watermark->setContent({QStringLiteral("qt-ant-design"), QStringLiteral("Qt5 parity")});
    watermark->setFixedSize(360, 138);
    bottomLayout->addWidget(watermark);
    bottomLayout->addStretch();
    layout->addWidget(bottomRow, 1);

    return surface;
}

QWidget* makeDesktopSurfaceScene()
{
    QWidget* surface = makeSurface();
    auto* layout = makeVBox(surface, 14, 10);
    layout->addWidget(makeLabel(QStringLiteral("Desktop surfaces"), surface));

    auto* topRow = new QWidget(surface);
    auto* topLayout = new QHBoxLayout(topRow);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(12);

    auto* menuBar = new AntMenuBar(topRow);
    menuBar->addMenu(QStringLiteral("File"));
    menuBar->addMenu(QStringLiteral("View"));
    menuBar->setFixedSize(220, 34);
    topLayout->addWidget(menuBar, 0, Qt::AlignTop);

    auto* ribbon = new AntRibbon(topRow);
    auto* page = ribbon->addPage(QStringLiteral("Home"), QStringLiteral("home"));
    auto* group = page->addGroup(QStringLiteral("Clipboard"));
    group->addLargeAction(new QAction(QStringLiteral("Paste"), ribbon));
    group->addSmallAction(new QAction(QStringLiteral("Copy"), ribbon));
    group->addSmallAction(new QAction(QStringLiteral("Cut"), ribbon));
    ribbon->setFixedSize(520, 150);
    topLayout->addWidget(ribbon);
    topLayout->addStretch();
    layout->addWidget(topRow);
    layout->addStretch();

    return surface;
}

QWidget* makeDockSurfaceScene()
{
    auto* dockManager = new AntDockManager;
    dockManager->setCentralContent(coloredBox(antTheme->tokens().colorPrimaryBg, dockManager));
    auto* explorerDock = new AntDockWidget(QStringLiteral("Explorer"));
    explorerDock->setWidget(coloredBox(antTheme->tokens().colorBgContainer, explorerDock));
    auto* previewDock = new AntDockWidget(QStringLiteral("Preview"));
    previewDock->setWidget(coloredBox(antTheme->tokens().colorSuccessBg, previewDock));
    dockManager->addDockWidget(Qt::LeftDockWidgetArea, explorerDock);
    dockManager->addDockWidget(previewDock, explorerDock, AntDockManager::DockPlacement::Center);
    return dockManager;
}

QWidget* makeFileDialogScene()
{
    auto* fileDialog = new AntFileDialog(nullptr,
                                         QStringLiteral("Open file"),
                                         QDir::homePath(),
                                         QStringLiteral("Images (*.png *.jpg);;All Files (*)"));
    fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
    fileDialog->setFileMode(QFileDialog::ExistingFile);
    fileDialog->refreshAntStyle();
    return fileDialog;
}

QWidget* makeWindowFrameScene()
{
    auto* window = new AntWindow;
    window->setWindowTitle(QStringLiteral("AntWindow"));
    window->setCentralWidget(coloredBox(antTheme->tokens().colorWarningBg, window));
    return window;
}

QList<Scene> scenes()
{
    return {
        {QStringLiteral("buttons"), QSize(760, 136), makeButtonsScene,
         {QStringLiteral("AntButton"), QStringLiteral("AntIcon"), QStringLiteral("AntTypography")}, 8.0, 0.18},
        {QStringLiteral("icon-resources"), QSize(760, 164), makeIconResourceScene,
         {QStringLiteral("AntIcon"), QStringLiteral("AntTypography")}, 6.0, 0.16},
        {QStringLiteral("selection-controls"), QSize(520, 128), makeSelectionScene,
         {QStringLiteral("AntCheckBox"), QStringLiteral("AntRadio"), QStringLiteral("AntSwitch")}, 8.0, 0.20},
        {QStringLiteral("input-controls"), QSize(760, 132), makeInputScene,
         {QStringLiteral("AntInput"), QStringLiteral("AntInputNumber"), QStringLiteral("AntSelect"),
          QStringLiteral("AntDatePicker"), QStringLiteral("AntTimePicker")}, 12.0, 0.30},
        {QStringLiteral("navigation"), QSize(720, 246), makeNavigationScene,
         {QStringLiteral("AntMenu"), QStringLiteral("AntTabs"), QStringLiteral("AntPagination")}, 14.0, 0.32},
        {QStringLiteral("feedback"), QSize(720, 198), makeFeedbackScene,
         {QStringLiteral("AntAlert"), QStringLiteral("AntTag"), QStringLiteral("AntBadge"),
          QStringLiteral("AntProgress"), QStringLiteral("AntToolTip")}, 12.0, 0.28},
        {QStringLiteral("popover-surfaces"), QSize(760, 340), makePopoverSurfaceScene,
         {QStringLiteral("AntPopover")}, 16.0, 0.36},
        {QStringLiteral("modal-surface"), QSize(760, 300), makeModalSurfaceScene,
         {QStringLiteral("AntModal")}, 22.0, 0.46, prepareModalSurfaceScene},
        {QStringLiteral("drawer-surface"), QSize(760, 260), makeDrawerSurfaceScene,
         {QStringLiteral("AntDrawer")}, 22.0, 0.46, prepareDrawerSurfaceScene},
        {QStringLiteral("dropdown-surface"), QSize(620, 270), makeDropdownSurfaceScene,
         {QStringLiteral("AntDropdown")}, 18.0, 0.42, prepareDropdownSurfaceScene},
        {QStringLiteral("popconfirm-surface"), QSize(680, 280), makePopconfirmSurfaceScene,
         {QStringLiteral("AntPopconfirm")}, 18.0, 0.42, preparePopconfirmSurfaceScene},
        {QStringLiteral("tour-surface"), QSize(700, 340), makeTourSurfaceScene,
         {QStringLiteral("AntTour")}, 22.0, 0.46, prepareTourSurfaceScene},
        {QStringLiteral("interaction-states"), QSize(980, 286), makeInteractionStateScene,
         {QStringLiteral("AntButton"), QStringLiteral("AntInput"), QStringLiteral("AntCheckBox"),
          QStringLiteral("AntRadio"), QStringLiteral("AntSlider"), QStringLiteral("AntMenu"),
          QStringLiteral("AntTabs"), QStringLiteral("AntPagination"), QStringLiteral("AntTable")},
         18.0, 0.42, prepareInteractionStateScene},
        {QStringLiteral("acceptance-state-matrix"), QSize(1120, 354), makeAcceptanceStateMatrixScene,
         {QStringLiteral("AntButton"), QStringLiteral("AntCheckBox"), QStringLiteral("AntRadio"),
          QStringLiteral("AntInput"), QStringLiteral("AntProgress"), QStringLiteral("AntTag"),
          QStringLiteral("AntBadge"), QStringLiteral("AntToolTip"), QStringLiteral("AntMenu"),
          QStringLiteral("AntTabs"), QStringLiteral("AntPagination"), QStringLiteral("AntList"),
          QStringLiteral("AntTable"), QStringLiteral("AntTree"), QStringLiteral("AntScrollBar")},
         18.0, 0.42, prepareAcceptanceStateMatrixScene},
        {QStringLiteral("data-display"), QSize(820, 224), makeDataDisplayScene,
         {QStringLiteral("AntList"), QStringLiteral("AntTable")}, 16.0, 0.36},
        {QStringLiteral("tree"), QSize(260, 150), makeTreeScene,
         {QStringLiteral("AntTree")}, 10.0, 0.28},
        {QStringLiteral("scroll-carousel"), QSize(720, 210), makeScrollAndCarouselScene,
         {QStringLiteral("AntScrollArea"), QStringLiteral("AntScrollBar"), QStringLiteral("AntCarousel")}, 14.0, 0.30},
        {QStringLiteral("desktop-extensions"), QSize(980, 218), makeDesktopExtensionScene,
         {QStringLiteral("AntNav"), QStringLiteral("AntNavItem"), QStringLiteral("AntDialog"),
          QStringLiteral("AntInputDialog"), QStringLiteral("AntFloatButton")}, 18.0, 0.40},
        {QStringLiteral("advanced-entry"), QSize(900, 340), makeAdvancedEntryScene,
         {QStringLiteral("AntAutoComplete"), QStringLiteral("AntMentions"), QStringLiteral("AntCascader"),
          QStringLiteral("AntTreeSelect"), QStringLiteral("AntColorPicker"), QStringLiteral("AntSlider"),
          QStringLiteral("AntRate"), QStringLiteral("AntSegmented"), QStringLiteral("AntTransfer"),
          QStringLiteral("AntUpload")}, 18.0, 0.42},
        {QStringLiteral("rich-display"), QSize(1040, 334), makeRichDisplayScene,
         {QStringLiteral("AntAvatar"), QStringLiteral("AntCard"), QStringLiteral("AntStatistic"),
          QStringLiteral("AntCalendar"), QStringLiteral("AntDescriptions"), QStringLiteral("AntEmpty"),
          QStringLiteral("AntQRCode"), QStringLiteral("AntTimeline"), QStringLiteral("AntImage")}, 22.0, 0.46},
        {QStringLiteral("rich-feedback"), QSize(940, 286), makeRichFeedbackScene,
         {QStringLiteral("AntResult"), QStringLiteral("AntNotification"), QStringLiteral("AntMessage"),
          QStringLiteral("AntSpin"), QStringLiteral("AntSkeleton"), QStringLiteral("AntCollapse")}, 22.0, 0.46},
        {QStringLiteral("layout-extensions"), QSize(1010, 396), makeLayoutExtensionScene,
         {QStringLiteral("AntBreadcrumb"), QStringLiteral("AntSteps"), QStringLiteral("AntDivider"),
          QStringLiteral("AntLayout"), QStringLiteral("AntSplitter"), QStringLiteral("AntStackedWidget"),
          QStringLiteral("AntToolBar"), QStringLiteral("AntToolButton"), QStringLiteral("AntPlainTextEdit"),
          QStringLiteral("AntStatusBar"), QStringLiteral("AntLog")}, 20.0, 0.42},
        {QStringLiteral("structural-layout"), QSize(980, 424), makeStructuralLayoutScene,
         {QStringLiteral("AntAnchor"), QStringLiteral("AntFlex"), QStringLiteral("AntSpace"),
          QStringLiteral("AntGrid"), QStringLiteral("AntMasonry"), QStringLiteral("AntWidget"),
          QStringLiteral("AntForm"), QStringLiteral("AntWatermark")}, 22.0, 0.46},
        {QStringLiteral("desktop-surfaces"), QSize(820, 240), makeDesktopSurfaceScene,
         {QStringLiteral("AntMenuBar"), QStringLiteral("AntRibbon")}, 18.0, 0.40},
        {QStringLiteral("dock-surfaces"), QSize(520, 320), makeDockSurfaceScene,
         {QStringLiteral("AntDockManager"), QStringLiteral("AntDockWidget")}, 20.0, 0.42},
        {QStringLiteral("file-dialog"), QSize(760, 460), makeFileDialogScene,
         {QStringLiteral("AntFileDialog")}, 20.0, 0.42},
        {QStringLiteral("window-frame"), QSize(520, 340), makeWindowFrameScene,
         {QStringLiteral("AntWindow")}, 20.0, 0.42},
    };
}

QList<ThemeVariant> themeVariants()
{
    return {
        {QStringLiteral("light"), Ant::ThemeMode::Default},
        {QStringLiteral("dark"), Ant::ThemeMode::Dark},
    };
}

QString variantSceneName(const ThemeVariant& variant, const Scene& scene)
{
    return variant.name + QLatin1Char('-') + scene.name;
}

void renderIntersectingTopLevelOverlays(QWidget* root, QPainter* painter)
{
    const QWidget* rootWindow = root->window();
    const QRect rootGlobalRect(root->mapToGlobal(QPoint(0, 0)), root->size());
    QWidgetList overlays = QApplication::topLevelWidgets();
    std::sort(overlays.begin(), overlays.end(), [](const QWidget* lhs, const QWidget* rhs) {
        return reinterpret_cast<quintptr>(lhs) < reinterpret_cast<quintptr>(rhs);
    });

    for (QWidget* overlay : overlays)
    {
        if (!overlay || overlay == root || overlay == rootWindow || !overlay->isVisible())
        {
            continue;
        }
        bool belongsToRoot = false;
        for (QObject* owner = overlay; owner; owner = owner->parent())
        {
            if (owner == root)
            {
                belongsToRoot = true;
                break;
            }
        }
        if (!belongsToRoot)
        {
            continue;
        }

        const QRect overlayGlobalRect(overlay->mapToGlobal(QPoint(0, 0)), overlay->size());
        if (!rootGlobalRect.intersects(overlayGlobalRect))
        {
            continue;
        }

        const QPoint offset = overlayGlobalRect.topLeft() - rootGlobalRect.topLeft();
        QImage overlayImage(overlay->size(), QImage::Format_ARGB32_Premultiplied);
        overlayImage.fill(Qt::transparent);
        {
            QPainter overlayPainter(&overlayImage);
            overlay->render(&overlayPainter, QPoint(), QRegion(), QWidget::DrawChildren);
        }

        painter->save();
        painter->setClipRect(QRect(QPoint(0, 0), root->size()));
        painter->drawImage(offset, overlayImage);
        painter->restore();
    }
}

QImage renderScene(QWidget* widget, const QSize& size, const std::function<void(QWidget*)>& prepare = {})
{
    widget->resize(size);
    widget->ensurePolished();
    QCoreApplication::sendPostedEvents(widget, QEvent::Polish);
    if (widget->layout())
    {
        widget->layout()->activate();
    }
    QCoreApplication::processEvents();

    if (prepare)
    {
        prepare(widget);
        if (widget->layout())
        {
            widget->layout()->activate();
        }
        QCoreApplication::processEvents();
    }

    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        widget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
        renderIntersectingTopLevelOverlays(widget, &painter);
    }
    return image;
}

int opaquePixelCount(const QImage& image)
{
    int count = 0;
    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            if (image.pixelColor(x, y).alpha() > 24)
            {
                ++count;
            }
        }
    }
    return count;
}

qreal averageLuminance(const QImage& image)
{
    qreal total = 0.0;
    int count = 0;
    for (int y = 0; y < image.height(); ++y)
    {
        for (int x = 0; x < image.width(); ++x)
        {
            const QColor color = image.pixelColor(x, y);
            if (color.alpha() <= 24)
            {
                continue;
            }
            total += 0.2126 * color.red() + 0.7152 * color.green() + 0.0722 * color.blue();
            ++count;
        }
    }
    return count > 0 ? total / count : 0.0;
}

int pixelDelta(const QColor& a, const QColor& b)
{
    return (std::abs(a.red() - b.red()) +
            std::abs(a.green() - b.green()) +
            std::abs(a.blue() - b.blue()) +
            std::abs(a.alpha() - b.alpha())) /
           4;
}

DiffStats compareImages(const QImage& actual, const QImage& baseline)
{
    DiffStats stats;
    qint64 totalDelta = 0;
    int changed32 = 0;
    int changed64 = 0;
    const int pixelCount = actual.width() * actual.height();
    for (int y = 0; y < actual.height(); ++y)
    {
        for (int x = 0; x < actual.width(); ++x)
        {
            const int delta = pixelDelta(actual.pixelColor(x, y), baseline.pixelColor(x, y));
            totalDelta += delta;
            stats.maxDelta = qMax(stats.maxDelta, delta);
            if (delta > 32)
            {
                ++changed32;
            }
            if (delta > 64)
            {
                ++changed64;
            }
        }
    }
    stats.meanDelta = pixelCount > 0 ? static_cast<qreal>(totalDelta) / pixelCount : 0.0;
    stats.changed32Ratio = pixelCount > 0 ? static_cast<qreal>(changed32) / pixelCount : 0.0;
    stats.changed64Ratio = pixelCount > 0 ? static_cast<qreal>(changed64) / pixelCount : 0.0;
    return stats;
}

QString scenePath(const QString& dir, const QString& name)
{
    return QDir(dir).filePath(name + QStringLiteral(".png"));
}

void writeManifest(const QString& exportDir, const QList<QPair<QString, QImage>>& renderedScenes)
{
    if (exportDir.isEmpty())
    {
        return;
    }

    QDir dir(exportDir);
    QVERIFY2(dir.exists() || QDir().mkpath(exportDir), qPrintable(QStringLiteral("Cannot create %1").arg(exportDir)));

    QFile manifest(dir.filePath(QStringLiteral("manifest.tsv")));
    QVERIFY2(manifest.open(QIODevice::WriteOnly | QIODevice::Text),
             qPrintable(QStringLiteral("Cannot write %1").arg(manifest.fileName())));

    QTextStream out(&manifest);
    out << "scene\tqtVersion\twidth\theight\topaquePixels\taverageLuminance\n";
    for (const auto& item : renderedScenes)
    {
        const QString& sceneName = item.first;
        const QImage& image = item.second;
        out << sceneName << '\t'
            << QT_VERSION_STR << '\t'
            << image.width() << '\t'
            << image.height() << '\t'
            << opaquePixelCount(image) << '\t'
            << QString::number(averageLuminance(image), 'f', 3) << '\n';
    }
}

void writeComparisonManifest(const QString& exportDir,
                             const QString& baselineDir,
                             const QList<ComparisonRecord>& comparisons)
{
    if (exportDir.isEmpty() || baselineDir.isEmpty() || comparisons.isEmpty())
    {
        return;
    }

    QDir dir(exportDir);
    QVERIFY2(dir.exists() || QDir().mkpath(exportDir), qPrintable(QStringLiteral("Cannot create %1").arg(exportDir)));

    QFile manifest(dir.filePath(QStringLiteral("comparison.tsv")));
    QVERIFY2(manifest.open(QIODevice::WriteOnly | QIODevice::Text),
             qPrintable(QStringLiteral("Cannot write %1").arg(manifest.fileName())));

    QTextStream out(&manifest);
    out << "scene\tactualQtVersion\tbaselineDir\tmeanDelta\tmaxMeanDelta\tchanged32Ratio\tmaxChanged32Ratio\tchanged64Ratio\tmaxDelta\n";
    for (const ComparisonRecord& record : comparisons)
    {
        out << record.sceneName << '\t'
            << QT_VERSION_STR << '\t'
            << QDir::toNativeSeparators(baselineDir) << '\t'
            << QString::number(record.stats.meanDelta, 'f', 3) << '\t'
            << QString::number(record.maxMeanDelta, 'f', 3) << '\t'
            << QString::number(record.stats.changed32Ratio, 'f', 4) << '\t'
            << QString::number(record.maxChanged32Ratio, 'f', 4) << '\t'
            << QString::number(record.stats.changed64Ratio, 'f', 4) << '\t'
            << record.stats.maxDelta << '\n';
    }
}

qreal envReal(const char* name, qreal fallback)
{
    bool ok = false;
    const qreal value = QString::fromLocal8Bit(qgetenv(name)).toDouble(&ok);
    return ok ? value : fallback;
}
} // namespace

void TestAntQtVersionVisualParity::initTestCase()
{
    AntDesign::initialize(qobject_cast<QApplication*>(QCoreApplication::instance()));
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtVersionVisualParity::visualParityScenesCoverPublicVisualWidgets()
{
    const QSet<QString> publicHeaders = publicWidgetHeaders();
    QVERIFY(!publicHeaders.isEmpty());

    QSet<QString> coveredHeaders;
    for (const Scene& scene : scenes())
    {
        QVERIFY2(!scene.controls.isEmpty(), qPrintable(scene.name));
        for (const QString& control : scene.controls)
        {
            coveredHeaders.insert(control);
        }
    }

    const QSet<QString> unknownHeaders = coveredHeaders - publicHeaders;
    QCOMPARE(sortedValues(unknownHeaders), QStringList());

    const QSet<QString> missingHeaders = publicHeaders - coveredHeaders - atlasDeferredHeaders();
    QCOMPARE(sortedValues(missingHeaders), QStringList());

    const QSet<QString> invalidDeferredHeaders = atlasDeferredHeaders() - publicHeaders;
    QCOMPARE(sortedValues(invalidDeferredHeaders), QStringList());
}

void TestAntQtVersionVisualParity::criticalWidgetScenesMatchQtBaseline()
{
    const QString exportDir = QString::fromLocal8Bit(qgetenv("ANT_QT_PARITY_EXPORT_DIR"));
    const QString baselineDir = QString::fromLocal8Bit(qgetenv("ANT_QT_PARITY_BASELINE_DIR"));
    const qreal meanScale = envReal("ANT_QT_PARITY_MEAN_SCALE", 1.0);
    const qreal changedScale = envReal("ANT_QT_PARITY_CHANGED_SCALE", 1.0);
    QSet<QString> sceneFilter;
    const QStringList sceneFilterValues =
        QString::fromLocal8Bit(qgetenv("ANT_QT_PARITY_SCENE_FILTER")).split(QLatin1Char(','), Qt::SkipEmptyParts);
    for (const QString& value : sceneFilterValues)
    {
        sceneFilter.insert(value.trimmed());
    }

    QList<QPair<QString, QImage>> renderedScenes;
    QList<ComparisonRecord> comparisons;
    for (const ThemeVariant& theme : themeVariants())
    {
        antTheme->setThemeMode(theme.mode);
        QCoreApplication::processEvents();

        for (const Scene& scene : scenes())
        {
            const QString renderName = variantSceneName(theme, scene);
            if (!sceneFilter.isEmpty() && !sceneFilter.contains(scene.name) && !sceneFilter.contains(renderName))
            {
                continue;
            }

            std::unique_ptr<QWidget> widget(scene.create());
            QVERIFY2(widget != nullptr, qPrintable(renderName));

            const QImage image = renderScene(widget.get(), scene.size, scene.prepare);
            QVERIFY2(!image.isNull(), qPrintable(renderName));
            QVERIFY2(opaquePixelCount(image) > image.width() * image.height() / 2,
                     qPrintable(QStringLiteral("%1 rendered too few opaque pixels").arg(renderName)));

            if (!exportDir.isEmpty())
            {
                QDir dir(exportDir);
                QVERIFY2(dir.exists() || QDir().mkpath(exportDir),
                         qPrintable(QStringLiteral("Cannot create %1").arg(exportDir)));
                QVERIFY2(image.save(scenePath(exportDir, renderName)),
                         qPrintable(QStringLiteral("Cannot save %1").arg(scenePath(exportDir, renderName))));
            }

            if (!baselineDir.isEmpty())
            {
                const QString baselinePath = scenePath(baselineDir, renderName);
                const QImage baseline(baselinePath);
                QVERIFY2(!baseline.isNull(), qPrintable(QStringLiteral("Missing baseline %1").arg(baselinePath)));
                QCOMPARE(image.size(), baseline.size());

                const DiffStats stats = compareImages(image, baseline);
                const qreal maxMean = scene.maxMeanDelta * meanScale;
                const qreal maxChanged32 = scene.maxChanged32Ratio * changedScale;
                comparisons.append({renderName, stats, maxMean, maxChanged32});
                QVERIFY2(stats.meanDelta <= maxMean,
                         qPrintable(QStringLiteral("%1 mean delta %2 exceeds %3; max delta %4, changed32 %5")
                                        .arg(renderName)
                                        .arg(stats.meanDelta, 0, 'f', 3)
                                        .arg(maxMean, 0, 'f', 3)
                                        .arg(stats.maxDelta)
                                        .arg(stats.changed32Ratio, 0, 'f', 4)));
                QVERIFY2(stats.changed32Ratio <= maxChanged32,
                         qPrintable(QStringLiteral("%1 changed32 ratio %2 exceeds %3; mean %4, max %5")
                                        .arg(renderName)
                                        .arg(stats.changed32Ratio, 0, 'f', 4)
                                        .arg(maxChanged32, 0, 'f', 4)
                                        .arg(stats.meanDelta, 0, 'f', 3)
                                        .arg(stats.maxDelta)));
                QVERIFY2(stats.changed64Ratio <= maxChanged32 / 2.0,
                         qPrintable(QStringLiteral("%1 changed64 ratio %2 exceeds %3; mean %4, max %5")
                                        .arg(renderName)
                                        .arg(stats.changed64Ratio, 0, 'f', 4)
                                        .arg(maxChanged32 / 2.0, 0, 'f', 4)
                                        .arg(stats.meanDelta, 0, 'f', 3)
                                        .arg(stats.maxDelta)));
            }

            renderedScenes.append(qMakePair(renderName, image));
        }
    }
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    writeManifest(exportDir, renderedScenes);
    writeComparisonManifest(exportDir, baselineDir, comparisons);
}

QTEST_MAIN(TestAntQtVersionVisualParity)
#include "TestAntQtVersionVisualParity.moc"
