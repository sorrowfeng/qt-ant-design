#include <QPalette>
#include <QPainter>
#include <QImage>
#include <QLabel>
#include <QMargins>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QAction>
#include <QHideEvent>
#include <QSignalSpy>
#include <QHoverEvent>
#include <QTest>
#include <QToolButton>
#include "core/AntTheme.h"
#include "widgets/AntApp.h"
#include "widgets/AntConfigProvider.h"
#include "widgets/AntForm.h"
#include "widgets/AntLog.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntPlainTextEdit.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntSplitter.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntRibbon.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"
#include "widgets/AntColorPicker.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
#endif

namespace
{
bool colorNearForExtensionTest(const QColor& actual, const QColor& expected, int tolerance = 12)
{
    return std::abs(actual.red() - expected.red()) <= tolerance &&
           std::abs(actual.green() - expected.green()) <= tolerance &&
           std::abs(actual.blue() - expected.blue()) <= tolerance;
}

QString colorStringForExtensionTest(const QColor& color)
{
    return QStringLiteral("rgba(%1,%2,%3,%4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alpha());
}

QImage renderForExtensionTest(QWidget* widget)
{
    QImage image(widget->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    widget->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    return image;
}

QLabel* ribbonTitleLabelForExtensionTest(AntRibbonGroup* group, const QString& text)
{
    const auto labels = group->findChildren<QLabel*>();
    for (QLabel* label : labels)
    {
        if (label && label->text() == text)
        {
            return label;
        }
    }
    return nullptr;
}

#ifdef Q_OS_WIN
using RtlGetVersionFn = LONG(WINAPI*)(OSVERSIONINFOW*);

int windowsBuildNumberForTest()
{
    if (HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll"))
    {
        auto* rtlGetVersion = reinterpret_cast<RtlGetVersionFn>(::GetProcAddress(ntdll, "RtlGetVersion"));
        if (rtlGetVersion)
        {
            OSVERSIONINFOW version{};
            version.dwOSVersionInfoSize = sizeof(version);
            if (rtlGetVersion(&version) == 0)
            {
                return static_cast<int>(version.dwBuildNumber);
            }
        }
    }
    return 0;
}

bool supportsNativeCaptionSnapLayoutsForTest()
{
    constexpr int kWindows11Build = 22000;
    return windowsBuildNumberForTest() >= kWindows11Build;
}
#endif
} // namespace

class TestAntQtExtensions : public QObject
{
    Q_OBJECT
private slots:
    void app();
    void configProvider();
    void formItem();
    void form();
    void formList();
    void log();
    void masonry();
    void plainTextEdit();
    void scrollArea();
    void scrollBar();
    void splitter();
    void statusBar();
    void ribbon();
    void toolButton();
    void toolBar();
    void menuBar();
    void dockWidget();
    void widget();
    void window();
    void windowTitleBarButtonsHandleChildDeliveredClicks();
    void windowTitleBarButtonsTriggerOnRelease();
    void windowAlwaysOnTopDoesNotRecreateVisibleWindow();
    void windowTitleBarHoverStateClearsOnLeave();
    void windowThemeButtonShowsTransitionOverlay();
    void windowThemeTransitionOverlayKeepsOldFrameScale();
    void windowThemeTransitionRevealsNewFrameWithoutBlackHole();
    void windowNativeHitTestSupportsSnapZones();
    void windowDwmFrameMarginsPreserveShadow();
    void windowLegacyFramePolicyRestoresShadowAfterResize();
    void windowMaximizedNcCalcKeepsFullWorkArea();
    void colorPicker();
};

void TestAntQtExtensions::app()
{
    auto* root = new QWidget;
    auto* w = new AntApp(root);
    QCOMPARE(w->rootWidget(), root);
}

void TestAntQtExtensions::configProvider()
{
    auto* w = new AntConfigProvider;
    QCOMPARE(w->themeMode(), Ant::ThemeMode::Default);
    QCOMPARE(w->fontSize(), 14);
    QCOMPARE(w->borderRadius(), 6);

    QSignalSpy themeSpy(w, &AntConfigProvider::themeModeChanged);
    w->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(w->themeMode(), Ant::ThemeMode::Dark);
    QCOMPARE(themeSpy.count(), 1);

    QSignalSpy colorSpy(w, &AntConfigProvider::primaryColorChanged);
    w->setPrimaryColor(Qt::blue);
    QCOMPARE(w->primaryColor(), QColor(Qt::blue));
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy fontSpy(w, &AntConfigProvider::fontSizeChanged);
    w->setFontSize(16);
    QCOMPARE(w->fontSize(), 16);
    QCOMPARE(fontSpy.count(), 1);

    QSignalSpy radiusSpy(w, &AntConfigProvider::borderRadiusChanged);
    w->setBorderRadius(8);
    QCOMPARE(w->borderRadius(), 8);
    QCOMPARE(radiusSpy.count(), 1);
}

void TestAntQtExtensions::formItem()
{
    auto* w = new AntFormItem;
    QCOMPARE(w->label(), QString());
    QCOMPARE(w->helpText(), QString());
    QCOMPARE(w->extra(), QString());
    QCOMPARE(w->isRequired(), false);
    QCOMPARE(w->colon(), true);
    QCOMPARE(w->validateStatus(), Ant::Status::Normal);

    QSignalSpy labelSpy(w, &AntFormItem::labelChanged);
    w->setLabel("Username");
    QCOMPARE(w->label(), "Username");
    QCOMPARE(labelSpy.count(), 1);

    QSignalSpy helpSpy(w, &AntFormItem::helpTextChanged);
    w->setHelpText("Enter your username");
    QCOMPARE(w->helpText(), "Enter your username");
    QCOMPARE(helpSpy.count(), 1);

    QSignalSpy extraSpy(w, &AntFormItem::extraChanged);
    w->setExtra("Required");
    QCOMPARE(w->extra(), "Required");
    QCOMPARE(extraSpy.count(), 1);

    QSignalSpy reqSpy(w, &AntFormItem::requiredChanged);
    w->setRequired(true);
    QCOMPARE(w->isRequired(), true);
    QCOMPARE(reqSpy.count(), 1);

    QSignalSpy colonSpy(w, &AntFormItem::colonChanged);
    w->setColon(false);
    QCOMPARE(w->colon(), false);
    QCOMPARE(colonSpy.count(), 1);

    QSignalSpy statusSpy(w, &AntFormItem::validateStatusChanged);
    w->setValidateStatus(Ant::Status::Error);
    QCOMPARE(w->validateStatus(), Ant::Status::Error);
    QCOMPARE(statusSpy.count(), 1);
}

void TestAntQtExtensions::form()
{
    auto* w = new AntForm;
    QCOMPARE(w->formLayout(), Ant::FormLayout::Horizontal);
    QCOMPARE(w->labelAlign(), Ant::FormLabelAlign::Right);
    QCOMPARE(w->colon(), true);
    QCOMPARE(w->requiredMark(), true);
    QCOMPARE(w->labelWidth(), 96);
    QCOMPARE(w->itemSpacing(), 16);

    QSignalSpy layoutSpy(w, &AntForm::formLayoutChanged);
    w->setFormLayout(Ant::FormLayout::Vertical);
    QCOMPARE(w->formLayout(), Ant::FormLayout::Vertical);
    QCOMPARE(layoutSpy.count(), 1);

    QSignalSpy alignSpy(w, &AntForm::labelAlignChanged);
    w->setLabelAlign(Ant::FormLabelAlign::Left);
    QCOMPARE(w->labelAlign(), Ant::FormLabelAlign::Left);
    QCOMPARE(alignSpy.count(), 1);

    QSignalSpy colonSpy(w, &AntForm::colonChanged);
    w->setColon(false);
    QCOMPARE(w->colon(), false);
    QCOMPARE(colonSpy.count(), 1);

    QSignalSpy markSpy(w, &AntForm::requiredMarkChanged);
    w->setRequiredMark(false);
    QCOMPARE(w->requiredMark(), false);
    QCOMPARE(markSpy.count(), 1);

    QSignalSpy widthSpy(w, &AntForm::labelWidthChanged);
    w->setLabelWidth(120);
    QCOMPARE(w->labelWidth(), 120);
    QCOMPARE(widthSpy.count(), 1);

    QSignalSpy spacingSpy(w, &AntForm::itemSpacingChanged);
    w->setItemSpacing(24);
    QCOMPARE(w->itemSpacing(), 24);
    QCOMPARE(spacingSpy.count(), 1);

    auto* item = new AntFormItem;
    item->setLabel("Name");
    w->addItem(item);
    QCOMPARE(w->items().size(), 1);

    w->addItem("Email", new QWidget, true);
    QCOMPARE(w->items().size(), 2);

    w->clearItems();
    QCOMPARE(w->items().size(), 0);
}

void TestAntQtExtensions::formList()
{
    auto* w = new AntFormList;
    QCOMPARE(w->minCount(), 0);
    QCOMPARE(w->maxCount(), 0);
    QCOMPARE(w->count(), 0);

    QSignalSpy minSpy(w, &AntFormList::minCountChanged);
    w->setMinCount(1);
    QCOMPARE(w->minCount(), 1);
    QCOMPARE(minSpy.count(), 1);

    QSignalSpy maxSpy(w, &AntFormList::maxCountChanged);
    w->setMaxCount(5);
    QCOMPARE(w->maxCount(), 5);
    QCOMPARE(maxSpy.count(), 1);
}

void TestAntQtExtensions::log()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntLog;
    QCOMPARE(w->maxEntries(), 5000);
    QCOMPARE(w->autoScroll(), true);
    auto* view = w->findChild<QPlainTextEdit*>();
    QVERIFY(view != nullptr);
    QCOMPARE(view->palette().color(QPalette::Base), antTheme->tokens().colorFillQuaternary);

    QSignalSpy maxSpy(w, &AntLog::maxEntriesChanged);
    w->setMaxEntries(1000);
    QCOMPARE(w->maxEntries(), 1000);
    QCOMPARE(maxSpy.count(), 1);

    QSignalSpy scrollSpy(w, &AntLog::autoScrollChanged);
    w->setAutoScroll(false);
    QCOMPARE(w->autoScroll(), false);
    QCOMPARE(scrollSpy.count(), 1);

    w->info("test message");
    w->warning("warning message");
    w->error("error message");

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(view->palette().color(QPalette::Base), antTheme->tokens().colorFillQuaternary);

    w->clear();
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::masonry()
{
    auto* w = new AntMasonry;
    QCOMPARE(w->columns(), 3);
    QCOMPARE(w->spacing(), 8);

    QSignalSpy columnsSpy(w, &AntMasonry::columnsChanged);
    w->setColumns(2);
    QCOMPARE(w->columns(), 2);
    QCOMPARE(columnsSpy.count(), 1);

    QSignalSpy spacingSpy(w, &AntMasonry::spacingChanged);
    w->setSpacing(12);
    QCOMPARE(w->spacing(), 12);
    QCOMPARE(spacingSpy.count(), 1);

    w->resize(212, 200);
    auto* first = new QWidget;
    first->setMinimumHeight(120);
    auto* second = new QWidget;
    second->setMinimumHeight(60);
    auto* third = new QWidget;
    third->setMinimumHeight(80);

    w->addWidget(first);
    w->addWidget(second);
    w->addWidget(third);

    QCOMPARE(first->geometry(), QRect(0, 0, 100, 120));
    QCOMPARE(second->geometry(), QRect(112, 0, 100, 60));
    QCOMPARE(third->geometry(), QRect(112, 72, 100, 80));
    QCOMPARE(w->minimumHeight(), 152);

    w->clear();
    QCOMPARE(w->minimumHeight(), 0);
}

void TestAntQtExtensions::plainTextEdit()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntPlainTextEdit;
    QCOMPARE(w->variant(), Ant::Variant::Outlined);

    QSignalSpy varSpy(w, &AntPlainTextEdit::variantChanged);
    w->setVariant(Ant::Variant::Filled);
    QCOMPARE(w->variant(), Ant::Variant::Filled);
    QCOMPARE(varSpy.count(), 1);

    QSignalSpy phSpy(w, &AntPlainTextEdit::placeholderTextChanged);
    w->setPlaceholderText("Type here...");
    QCOMPARE(w->placeholderText(), "Type here...");
    QCOMPARE(phSpy.count(), 1);

    w->setEnabled(false);
    QCOMPARE(w->palette().color(QPalette::Disabled, QPalette::Text), antTheme->tokens().colorTextDisabled);

    auto* resizable = new AntPlainTextEdit;
    resizable->setFixedSize(180, 80);
    resizable->show();
    QVERIFY(QTest::qWaitForWindowExposed(resizable));
    const QPoint grip = resizable->viewport()->mapFrom(resizable, QPoint(resizable->width() - 4, resizable->height() - 4));
    QTest::mousePress(resizable->viewport(), Qt::LeftButton, Qt::NoModifier, grip);
    QTest::mouseMove(resizable->viewport(), grip + QPoint(32, 18));
    QTest::mouseRelease(resizable->viewport(), Qt::LeftButton, Qt::NoModifier, grip + QPoint(32, 18));
    QCOMPARE(resizable->size(), QSize(212, 98));

    auto* w2 = new AntPlainTextEdit("Initial text");
    QCOMPARE(w2->toPlainText(), "Initial text");
}

void TestAntQtExtensions::scrollArea()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntScrollArea;
    QCOMPARE(w->autoHideScrollBar(), true);
    QCOMPARE(w->isGestureEnabled(), true);

    QSignalSpy hideSpy(w, &AntScrollArea::autoHideScrollBarChanged);
    w->setAutoHideScrollBar(false);
    QCOMPARE(w->autoHideScrollBar(), false);
    QCOMPARE(hideSpy.count(), 1);

    QSignalSpy gestureSpy(w, &AntScrollArea::enableGestureChanged);
    w->setEnableGesture(false);
    QCOMPARE(w->isGestureEnabled(), false);
    QCOMPARE(gestureSpy.count(), 1);

    auto* content = new QWidget;
    w->setWidget(content);
    QCOMPARE(w->viewport()->palette().color(QPalette::Base), antTheme->tokens().colorBgContainer);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(w->viewport()->palette().color(QPalette::Base), antTheme->tokens().colorBgContainer);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::scrollBar()
{
    auto* w = new AntScrollBar;
    QCOMPARE(w->autoHide(), true);

    QSignalSpy hideSpy(w, &AntScrollBar::autoHideChanged);
    w->setAutoHide(false);
    QCOMPARE(w->autoHide(), false);
    QCOMPARE(hideSpy.count(), 1);

    auto* w2 = new AntScrollBar(Qt::Horizontal);
    QCOMPARE(w2->orientation(), Qt::Horizontal);
}

void TestAntQtExtensions::splitter()
{
    auto* w = new AntSplitter;
    QCOMPARE(w->orientation(), Qt::Horizontal);
    QCOMPARE(w->childrenCollapsible(), false);
    QCOMPARE(w->handleWidth(), 4);

    auto* w2 = new AntSplitter(Qt::Vertical);
    QCOMPARE(w2->orientation(), Qt::Vertical);

    auto* c1 = new QWidget;
    auto* c2 = new QWidget;
    w->addWidget(c1);
    w->addWidget(c2);
    QCOMPARE(w->count(), 2);
}

void TestAntQtExtensions::statusBar()
{
    auto* w = new AntStatusBar;
    QCOMPARE(w->message(), QString());
    QCOMPARE(w->hasSizeGrip(), true);
    QCOMPARE(w->itemCount(), 0);
    QCOMPARE(w->permanentItemCount(), 0);

    QSignalSpy msgSpy(w, &AntStatusBar::messageChanged);
    w->setMessage("Ready");
    QCOMPARE(w->message(), "Ready");
    QCOMPARE(w->currentMessage(), "Ready");
    QCOMPARE(msgSpy.count(), 1);

    w->clearMessage();
    QCOMPARE(w->currentMessage(), QString());
    w->showMessage(QStringLiteral("Saving"), 30);
    QCOMPARE(w->currentMessage(), QStringLiteral("Saving"));
    QTRY_COMPARE(w->currentMessage(), QString());

    QSignalSpy gripSpy(w, &AntStatusBar::sizeGripChanged);
    w->setSizeGrip(false);
    QCOMPARE(w->hasSizeGrip(), false);
    QCOMPARE(gripSpy.count(), 1);

    w->addItem("Item 1");
    QCOMPARE(w->itemCount(), 1);
    w->addPermanentItem("Perm 1");
    QCOMPARE(w->permanentItemCount(), 1);

    auto item = w->itemAt(0);
    QCOMPARE(item.text, "Item 1");
}

void TestAntQtExtensions::toolButton()
{
    auto* w = new AntToolButton;
    QCOMPARE(w->buttonType(), Ant::ButtonType::Default);
    QCOMPARE(w->buttonSize(), Ant::Size::Middle);
    QCOMPARE(w->isDanger(), false);
    QCOMPARE(w->isLoading(), false);
    QCOMPARE(w->arrowRotation(), 0.0);

    QSignalSpy typeSpy(w, &AntToolButton::buttonTypeChanged);
    w->setButtonType(Ant::ButtonType::Primary);
    QCOMPARE(w->buttonType(), Ant::ButtonType::Primary);
    QCOMPARE(typeSpy.count(), 1);

    QSignalSpy sizeSpy(w, &AntToolButton::buttonSizeChanged);
    w->setButtonSize(Ant::Size::Large);
    QCOMPARE(w->buttonSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy dangerSpy(w, &AntToolButton::dangerChanged);
    w->setDanger(true);
    QCOMPARE(w->isDanger(), true);
    QCOMPARE(dangerSpy.count(), 1);

    QSignalSpy loadSpy(w, &AntToolButton::loadingChanged);
    w->setLoading(true);
    QCOMPARE(w->isLoading(), true);
    QCOMPARE(loadSpy.count(), 1);

    QSignalSpy arrowSpy(w, &AntToolButton::arrowRotationChanged);
    w->setArrowRotation(90.0);
    QCOMPARE(w->arrowRotation(), 90.0);
    QCOMPARE(arrowSpy.count(), 1);

    auto* w2 = new AntToolButton("Click");
    QCOMPARE(w2->text(), "Click");

    auto* actionButton = new AntToolButton;
    auto* runAction = new QAction(QStringLiteral("Run"), actionButton);
    actionButton->setDefaultAction(runAction);
    QCOMPARE(actionButton->defaultAction(), runAction);
    QCOMPARE(actionButton->text(), QStringLiteral("Run"));
    QSignalSpy runSpy(runAction, &QAction::triggered);
    actionButton->resize(actionButton->sizeHint());
    QTest::mouseClick(actionButton, Qt::LeftButton, Qt::NoModifier, actionButton->rect().center());
    QCOMPARE(runSpy.count(), 1);
}

void TestAntQtExtensions::toolBar()
{
    auto* w = new AntToolBar;
    QVERIFY(w != nullptr);
    auto* action = w->addAction("New");
    auto* button = qobject_cast<QToolButton*>(w->widgetForAction(action));
    QVERIFY(button != nullptr);
    QVERIFY(button->property("antToolBarButton").toBool());
    QCOMPARE(button->style(), w->style());
    QSignalSpy actionSpy(action, &QAction::triggered);
    QTest::mouseClick(button, Qt::LeftButton, Qt::NoModifier, button->rect().center());
    QCOMPARE(actionSpy.count(), 1);

    auto* w2 = new AntToolBar("My Toolbar");
    QVERIFY(w2 != nullptr);
}

void TestAntQtExtensions::ribbon()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    auto* ribbon = new AntRibbon;
    QCOMPARE(ribbon->pageCount(), 0);
    QCOMPARE(ribbon->currentPageIndex(), -1);
    QCOMPARE(ribbon->currentPageKey(), QString());
    QCOMPARE(ribbon->isCollapsed(), false);
    QCOMPARE(ribbon->isCollapseButtonVisible(), true);

    QSignalSpy currentSpy(ribbon, &AntRibbon::currentPageChanged);
    QSignalSpy currentKeySpy(ribbon, &AntRibbon::currentPageKeyChanged);
    auto* file = ribbon->addPage(QStringLiteral("File"), QStringLiteral("file"));
    QVERIFY(file != nullptr);
    QCOMPARE(ribbon->pageCount(), 1);
    QCOMPARE(ribbon->pageAt(0), file);
    QCOMPARE(ribbon->pageByKey(QStringLiteral("file")), file);
    QCOMPARE(ribbon->currentPageIndex(), 0);
    QCOMPARE(ribbon->currentPageKey(), QStringLiteral("file"));
    QCOMPARE(currentSpy.count(), 1);
    QCOMPARE(currentKeySpy.count(), 1);

    auto* edit = ribbon->addPage(QStringLiteral("Edit"), QStringLiteral("edit"));
    QVERIFY(edit != nullptr);
    const QRectF fileIndicator = ribbon->indicatorRect();
    ribbon->setCurrentPageKey(QStringLiteral("edit"));
    QCOMPARE(ribbon->currentPageIndex(), 1);
    QCOMPARE(ribbon->currentPageKey(), QStringLiteral("edit"));
    QVERIFY(ribbon->indicatorRect().isValid());
    QVERIFY(qAbs(ribbon->indicatorRect().left() - fileIndicator.left()) < 0.5);
    QTRY_VERIFY(qAbs(ribbon->indicatorRect().left() - fileIndicator.left()) > 1.0);
    ribbon->setCurrentPageIndex(0);
    QCOMPARE(ribbon->currentPageKey(), QStringLiteral("file"));

    auto* group = file->addGroup(QStringLiteral("Clipboard"));
    QVERIFY(group != nullptr);
    QCOMPARE(file->groupCount(), 1);
    QCOMPARE(file->groupAt(0), group);

    QSignalSpy groupActionSpy(group, &AntRibbonGroup::actionTriggered);
    QSignalSpy ribbonActionSpy(ribbon, &AntRibbon::actionTriggered);
    auto* pasteAction = new QAction(QIcon(), QStringLiteral("Paste"), ribbon);
    group->addLargeAction(pasteAction);
    group->addSmallAction(new QAction(QStringLiteral("Copy"), ribbon));
    auto* combo = new QComboBox;
    combo->addItem(QStringLiteral("Mode A"));
    group->addWidget(combo, Ant::RibbonItemSize::Small);
    group->addWidget(new QWidget, Ant::RibbonItemSize::Large);
    QCOMPARE(group->itemCount(), 4);

    auto* controls = file->addGroup(QStringLiteral("Ant Controls"));
    QVERIFY(controls != nullptr);
    auto* modeSelect = new QComboBox;
    modeSelect->addItem(QStringLiteral("Advanced"));
    controls->addWidget(modeSelect, Ant::RibbonItemSize::Small);
    controls->addWidget(new QComboBox, Ant::RibbonItemSize::Small);
    controls->addWidget(new QWidget, Ant::RibbonItemSize::Large);
    QCOMPARE(file->groupCount(), 2);
    QVERIFY(group->sizeHint().height() >= 158);
    QVERIFY(ribbon->sizeHint().height() >= 210);
    QVERIFY(ribbon->property("indicatorRect").isValid());
    QVERIFY(ribbon->property("contentHeight").isValid());
    pasteAction->trigger();
    QCOMPARE(groupActionSpy.count(), 1);
    QCOMPARE(ribbonActionSpy.count(), 1);

    ribbon->resize(640, ribbon->sizeHint().height());
    ribbon->show();
    QVERIFY(QTest::qWaitForWindowExposed(ribbon));
    auto* clipboardTitle = ribbonTitleLabelForExtensionTest(group, QStringLiteral("Clipboard"));
    auto* controlsTitle = ribbonTitleLabelForExtensionTest(controls, QStringLiteral("Ant Controls"));
    QVERIFY(clipboardTitle != nullptr);
    QVERIFY(controlsTitle != nullptr);
    QCOMPARE(clipboardTitle->mapTo(ribbon, QPoint(0, 0)).y(), controlsTitle->mapTo(ribbon, QPoint(0, 0)).y());
    ribbon->setCurrentPageIndex(1);
    QTRY_COMPARE(ribbon->currentPageIndex(), 1);
    QTest::mouseMove(ribbon, QPoint(28, 20));
    QCoreApplication::processEvents();
    const QImage hoverImage = renderForExtensionTest(ribbon);
    const QColor hoveredTabBackground = hoverImage.pixelColor(18, 14);
    QVERIFY2(colorNearForExtensionTest(hoveredTabBackground, antTheme->tokens().colorBgElevated),
             qPrintable(QStringLiteral("Hovered tab painted a filled background: sampled %1 expected near %2")
                            .arg(colorStringForExtensionTest(hoveredTabBackground),
                                 colorStringForExtensionTest(antTheme->tokens().colorBgElevated))));
    ribbon->setCurrentPageIndex(0);

    QSignalSpy collapsedSpy(ribbon, &AntRibbon::collapsedChanged);
    ribbon->setCollapsed(true);
    QCOMPARE(ribbon->isCollapsed(), true);
    QCOMPARE(collapsedSpy.count(), 1);
    QVERIFY(ribbon->property("contentHeight").toReal() > 0.0);
    QTRY_VERIFY(ribbon->sizeHint().height() < 80);
    ribbon->setCollapsed(false);
    QCOMPARE(ribbon->isCollapsed(), false);
    QCOMPARE(collapsedSpy.count(), 2);
    QVERIFY(ribbon->property("contentHeight").toReal() < 176.0);
    QTRY_VERIFY(ribbon->sizeHint().height() >= 210);
    ribbon->setCollapsed(true);
    QCOMPARE(collapsedSpy.count(), 3);
    QTRY_VERIFY(ribbon->sizeHint().height() < 80);
    ribbon->resize(640, ribbon->sizeHint().height());
    QTest::mouseClick(ribbon, Qt::LeftButton, Qt::NoModifier, QPoint(28, 20));
    auto* popup = ribbon->findChild<QWidget*>(QStringLiteral("AntRibbonPopup"));
    QVERIFY(popup != nullptr);
    QTRY_VERIFY(popup->isVisible());
    popup->hide();
    ribbon->hide();

    AntWindow window;
    window.resize(700, 420);
    auto* windowRibbon = new AntRibbon;
    window.setRibbon(windowRibbon);
    QCOMPARE(window.ribbon(), windowRibbon);
    QCOMPARE(window.isRibbonVisible(), true);
    window.setRibbonVisible(false);
    QCOMPARE(window.isRibbonVisible(), false);
    QVERIFY(!windowRibbon->isVisible());
    window.setCentralWidget(new QWidget);
    QCOMPARE(window.ribbon(), windowRibbon);
    window.setRibbonVisible(true);
    QCOMPARE(window.isRibbonVisible(), true);
}

void TestAntQtExtensions::menuBar()
{
    auto* w = new AntMenuBar;
    QVERIFY(w->actions().isEmpty());

    auto* menu = w->addMenu("File");
    QVERIFY(menu != nullptr);
    QVERIFY(!w->actions().isEmpty());
}

void TestAntQtExtensions::dockWidget()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntDockWidget;
    QCOMPARE(w->windowTitle(), QString());

    auto* w2 = new AntDockWidget("Properties");
    QCOMPARE(w2->windowTitle(), "Properties");

    auto* content = new QWidget;
    w2->setWidget(content);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(w2->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::widget()
{
    auto* w = new AntWidget;
    QVERIFY(w->currentTheme() == Ant::ThemeMode::Default ||
            w->currentTheme() == Ant::ThemeMode::Dark);
}

void TestAntQtExtensions::window()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);
    auto* w = new AntWindow;
    QCOMPARE(w->isMaximized(), false);
    QCOMPARE(w->TitleBarHeight, 40);
    QCOMPARE(w->TitleBarButtonWidth, 46);
    QCOMPARE(w->isAlwaysOnTop(), false);
    QCOMPARE(w->isPinButtonVisible(), true);
    QCOMPARE(w->isThemeButtonVisible(), true);
    QCOMPARE(w->isMinimizeButtonVisible(), true);
    QCOMPARE(w->isMaximizeButtonVisible(), true);
    QCOMPARE(w->isCloseButtonVisible(), true);
    QCOMPARE(w->cornerRadius(), 8);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Pin), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Theme), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Minimize), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Maximize), true);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Close), true);

    QSignalSpy titleSpy(w, &AntWindow::windowTitleChanged);
    w->setWindowTitle("Example");
    QCOMPARE(w->windowTitle(), "Example");
    QCOMPARE(titleSpy.count(), 1);

    w->resize(640, 400);
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Pin).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Theme).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Minimize).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Maximize).isNull());
    QVERIFY(!w->titleBarButtonRect(AntWindow::TitleBarButton::Close).isNull());

    QSignalSpy visibilitySpy(w, &AntWindow::titleBarButtonVisibilityChanged);
    QSignalSpy pinVisibilitySpy(w, &AntWindow::pinButtonVisibleChanged);
    w->setPinButtonVisible(false);
    QCOMPARE(w->isPinButtonVisible(), false);
    QCOMPARE(w->isTitleBarButtonVisible(AntWindow::TitleBarButton::Pin), false);
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Pin).isNull());
    QCOMPARE(pinVisibilitySpy.count(), 1);
    QCOMPARE(visibilitySpy.count(), 1);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Pin, true);
    QCOMPARE(w->isPinButtonVisible(), true);
    QCOMPARE(pinVisibilitySpy.count(), 2);
    QCOMPARE(visibilitySpy.count(), 2);

    w->setThemeButtonVisible(false);
    w->setMinimizeButtonVisible(false);
    w->setMaximizeButtonVisible(false);
    w->setCloseButtonVisible(false);
    QCOMPARE(w->isThemeButtonVisible(), false);
    QCOMPARE(w->isMinimizeButtonVisible(), false);
    QCOMPARE(w->isMaximizeButtonVisible(), false);
    QCOMPARE(w->isCloseButtonVisible(), false);
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Theme).isNull());
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Minimize).isNull());
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Maximize).isNull());
    QVERIFY(w->titleBarButtonRect(AntWindow::TitleBarButton::Close).isNull());
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Theme, true);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Minimize, true);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Maximize, true);
    w->setTitleBarButtonVisible(AntWindow::TitleBarButton::Close, true);

    QSignalSpy alwaysOnTopSpy(w, &AntWindow::alwaysOnTopChanged);
    w->setAlwaysOnTop(true);
    QCOMPARE(w->isAlwaysOnTop(), true);
    QVERIFY(w->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QCOMPARE(alwaysOnTopSpy.count(), 1);
    w->setAlwaysOnTop(false);
    QCOMPARE(w->isAlwaysOnTop(), false);
    QVERIFY(!w->windowFlags().testFlag(Qt::WindowStaysOnTopHint));
    QCOMPARE(alwaysOnTopSpy.count(), 2);

    QSignalSpy cornerSpy(w, &AntWindow::cornerRadiusChanged);
    w->setCornerRadius(12);
    QCOMPARE(w->cornerRadius(), 12);
    QCOMPARE(cornerSpy.count(), 1);
    w->setCornerRadius(-1);
    QCOMPARE(w->cornerRadius(), 0);
    QCOMPARE(cornerSpy.count(), 2);

    const Ant::ThemeMode beforeClickMode = antTheme->themeMode();
    QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, w->titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    QCOMPARE(antTheme->themeMode(),
             beforeClickMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);
    QTest::mouseClick(w, Qt::LeftButton, Qt::NoModifier, w->titleBarButtonRect(AntWindow::TitleBarButton::Pin).center());
    QCOMPARE(w->isAlwaysOnTop(), true);
    w->setAlwaysOnTop(false);

    auto* content = new QWidget;
    w->setCentralWidget(content);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);

    antTheme->setThemeMode(Ant::ThemeMode::Dark);
    QCOMPARE(content->palette().color(QPalette::Window), antTheme->tokens().colorBgContainer);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowTitleBarButtonsHandleChildDeliveredClicks()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntWindow window;
    window.resize(640, 400);
    window.setCentralWidget(new QWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QPoint themeGlobal = window.mapToGlobal(window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    QWidget* themeTarget = window.centralWidget();
    QVERIFY(themeTarget != nullptr);
    const Ant::ThemeMode beforeClickMode = antTheme->themeMode();
    QTest::mouseClick(themeTarget, Qt::LeftButton, Qt::NoModifier, themeTarget->mapFromGlobal(themeGlobal));
    QTRY_COMPARE(antTheme->themeMode(),
                 beforeClickMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);

    const QPoint pinGlobal = window.mapToGlobal(window.titleBarButtonRect(AntWindow::TitleBarButton::Pin).center());
    QWidget* pinTarget = window.centralWidget();
    QVERIFY(pinTarget != nullptr);
    QTest::mouseClick(pinTarget, Qt::LeftButton, Qt::NoModifier, pinTarget->mapFromGlobal(pinGlobal));
    QTRY_COMPARE(window.isAlwaysOnTop(), true);

    window.setAlwaysOnTop(false);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowTitleBarButtonsTriggerOnRelease()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntWindow window;
    window.resize(640, 400);
    const QPoint themePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center();
    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, themePoint);
    QCOMPARE(antTheme->themeMode(), Ant::ThemeMode::Default);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, themePoint);
    QCOMPARE(antTheme->themeMode(), Ant::ThemeMode::Dark);

    const QPoint pinPoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Pin).center();
    QTest::mousePress(&window, Qt::LeftButton, Qt::NoModifier, pinPoint);
    QCOMPARE(window.isAlwaysOnTop(), false);
    QTest::mouseRelease(&window, Qt::LeftButton, Qt::NoModifier, pinPoint);
    QCOMPARE(window.isAlwaysOnTop(), true);

    window.setAlwaysOnTop(false);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowAlwaysOnTopDoesNotRecreateVisibleWindow()
{
#ifndef Q_OS_WIN
    QSKIP("Windows topmost behavior is verified through native SetWindowPos.");
#else
    class VisibleStateWindow : public AntWindow
    {
    public:
        int showEvents = 0;
        int hideEvents = 0;

    protected:
        void showEvent(QShowEvent* event) override
        {
            ++showEvents;
            AntWindow::showEvent(event);
        }

        void hideEvent(QHideEvent* event) override
        {
            ++hideEvents;
            QMainWindow::hideEvent(event);
        }
    };

    VisibleStateWindow window;
    window.resize(640, 420);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd != nullptr);
    window.showEvents = 0;
    window.hideEvents = 0;

    QSignalSpy alwaysOnTopSpy(&window, &AntWindow::alwaysOnTopChanged);
    window.setAlwaysOnTop(true);
    QCoreApplication::processEvents();
    QCOMPARE(window.isAlwaysOnTop(), true);
    QCOMPARE(alwaysOnTopSpy.count(), 1);
    QCOMPARE(window.showEvents, 0);
    QCOMPARE(window.hideEvents, 0);
    QVERIFY(window.isVisible());
    QVERIFY((::GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0);

    window.setAlwaysOnTop(false);
    QCoreApplication::processEvents();
    QCOMPARE(window.isAlwaysOnTop(), false);
    QCOMPARE(alwaysOnTopSpy.count(), 2);
    QCOMPARE(window.showEvents, 0);
    QCOMPARE(window.hideEvents, 0);
    QVERIFY(window.isVisible());
    QVERIFY((::GetWindowLongPtrW(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == 0);
#endif
}

void TestAntQtExtensions::windowTitleBarHoverStateClearsOnLeave()
{
    AntWindow window;
    window.resize(640, 420);
    window.setCentralWidget(new QWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QPoint closePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Close).center();
    QHoverEvent closeHover(QEvent::HoverMove, QPointF(closePoint), QPointF(closePoint - QPoint(1, 0)));
    QCoreApplication::sendEvent(&window, &closeHover);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::Close);

    QEvent windowLeave(QEvent::Leave);
    QCoreApplication::sendEvent(&window, &windowLeave);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::None);

    QWidget* titleBarEventTarget = window.centralWidget();
    QVERIFY(titleBarEventTarget != nullptr);
    QVERIFY(titleBarEventTarget->testAttribute(Qt::WA_Hover));

    const QPoint themePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center();
    const QPoint contentThemePoint = titleBarEventTarget->mapFrom(&window, themePoint);
    QHoverEvent themeHover(QEvent::HoverMove, QPointF(contentThemePoint), QPointF(contentThemePoint - QPoint(1, 0)));
    QCoreApplication::sendEvent(titleBarEventTarget, &themeHover);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::Theme);

    QEvent contentLeave(QEvent::Leave);
    QCoreApplication::sendEvent(titleBarEventTarget, &contentLeave);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::None);
}

void TestAntQtExtensions::windowThemeButtonShowsTransitionOverlay()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    AntWindow window;
    window.resize(640, 420);
    window.setCentralWidget(new QWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QTest::mouseClick(&window,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    QCOMPARE(antTheme->themeMode(), Ant::ThemeMode::Dark);

    auto* overlay = window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay"));
    QVERIFY(overlay != nullptr);
    QVERIFY(overlay->isVisible());
    QVERIFY(overlay->testAttribute(Qt::WA_TransparentForMouseEvents));
    QCOMPARE(overlay->geometry(), window.rect());
    QCOMPARE(overlay->property("transitionFrameIntervalMs").toInt(), 8);
    QCOMPARE(overlay->property("transitionDurationMs").toInt(), 320);
    QCOMPARE(overlay->property("transitionMotionCurve").toString(), QStringLiteral("smootherstep"));
    QCOMPARE(overlay->property("transitionEdgeFeather").toInt(), 24);
    QCOMPARE(overlay->property("transitionDrawsCapturedNewFrame").toBool(), true);

    QTRY_VERIFY(window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay")) == nullptr);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowThemeTransitionOverlayKeepsOldFrameScale()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    class SplitPaintWidget : public QWidget
    {
    public:
        explicit SplitPaintWidget(QWidget* parent = nullptr)
            : QWidget(parent)
        {
            setAutoFillBackground(false);
        }

    protected:
        void paintEvent(QPaintEvent*) override
        {
            QPainter painter(this);
            painter.fillRect(rect(), QColor(220, 30, 30));
            painter.fillRect(QRect(400, 0, qMax(0, width() - 400), height()), QColor(20, 90, 230));
        }
    };

    AntWindow window;
    window.resize(640, 420);
    window.setCentralWidget(new SplitPaintWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    if (window.devicePixelRatioF() <= 1.01)
    {
        QSKIP("High-DPI frame scale guard only applies when devicePixelRatioF > 1.");
    }

    QTest::mouseClick(&window,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    auto* overlay = window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay"));
    QVERIFY(overlay != nullptr);

    QImage rendered(overlay->size(), QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::transparent);
    QPainter painter(&rendered);
    overlay->render(&painter);
    painter.end();

    const QColor rightSide = rendered.pixelColor(520, 220);
    QVERIFY2(rightSide.blue() > 170 && rightSide.red() < 120,
             qPrintable(QStringLiteral("Overlay old frame was scaled incorrectly: sampled rgba(%1,%2,%3,%4)")
                            .arg(rightSide.red())
                            .arg(rightSide.green())
                            .arg(rightSide.blue())
                            .arg(rightSide.alpha())));

    QTRY_VERIFY(window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay")) == nullptr);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowThemeTransitionRevealsNewFrameWithoutBlackHole()
{
    antTheme->setThemeMode(Ant::ThemeMode::Default);

    class ThemePaintWidget : public QWidget
    {
    public:
        explicit ThemePaintWidget(QWidget* parent = nullptr)
            : QWidget(parent)
        {
            setAutoFillBackground(false);
            connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
        }

    protected:
        void paintEvent(QPaintEvent*) override
        {
            QPainter painter(this);
            painter.fillRect(rect(),
                             antTheme->themeMode() == Ant::ThemeMode::Dark
                                 ? QColor(20, 100, 235)
                                 : QColor(230, 45, 45));
        }
    };

    AntWindow window;
    window.resize(640, 420);
    window.setCentralWidget(new ThemePaintWidget);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QPoint samplePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center() + QPoint(0, 80);
    QTest::mouseClick(&window,
                      Qt::LeftButton,
                      Qt::NoModifier,
                      window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center());
    auto* overlay = window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay"));
    QVERIFY(overlay != nullptr);
    QTest::qWait(140);

    QImage rendered(overlay->size(), QImage::Format_ARGB32_Premultiplied);
    rendered.fill(Qt::black);
    QPainter painter(&rendered);
    overlay->render(&painter);
    painter.end();

    const QColor revealed = rendered.pixelColor(samplePoint);
    QVERIFY2(revealed.blue() > 160 && revealed.red() < 120,
             qPrintable(QStringLiteral("Transition reveal exposed a black/transparent hole instead of the new frame: rgba(%1,%2,%3,%4)")
                            .arg(revealed.red())
                            .arg(revealed.green())
                            .arg(revealed.blue())
                            .arg(revealed.alpha())));

    QTRY_VERIFY(window.findChild<QWidget*>(QStringLiteral("AntWindowThemeTransitionOverlay")) == nullptr);
    antTheme->setThemeMode(Ant::ThemeMode::Default);
}

void TestAntQtExtensions::windowNativeHitTestSupportsSnapZones()
{
#ifndef Q_OS_WIN
    QSKIP("Windows native hit testing is only available on Windows.");
#else
    class NativeHitTestWindow : public AntWindow
    {
    public:
        using AntWindow::nativeEvent;
    };

    NativeHitTestWindow window;
    window.resize(640, 420);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd != nullptr);
    const LONG_PTR nativeStyle = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    QVERIFY((nativeStyle & WS_THICKFRAME) != 0);
    if (supportsNativeCaptionSnapLayoutsForTest())
    {
        QVERIFY((nativeStyle & WS_CAPTION) != 0);
    }
    else
    {
        QVERIFY((nativeStyle & WS_CAPTION) == 0);
    }
    QVERIFY((nativeStyle & WS_MAXIMIZEBOX) != 0);
    QVERIFY((nativeStyle & WS_MINIMIZEBOX) != 0);

    auto nativeGlobalPoint = [&](const QPoint& localPos) {
        POINT nativePoint{qRound(localPos.x() * window.devicePixelRatioF()),
                          qRound(localPos.y() * window.devicePixelRatioF())};
        ::ClientToScreen(hwnd, &nativePoint);
        return QPoint(nativePoint.x, nativePoint.y);
    };

    auto hitTest = [&](const QPoint& localPos) -> qintptr {
        MSG msg{};
        msg.hwnd = hwnd;
        msg.message = WM_NCHITTEST;
        const QPoint globalPos = nativeGlobalPoint(localPos);
        msg.lParam = MAKELPARAM(globalPos.x(), globalPos.y());
        qintptr result = 0;
        if (!window.nativeEvent("windows_generic_MSG", &msg, &result))
        {
            return -1;
        }
        return result;
    };

    QCOMPARE(hitTest(QPoint(2, 2)), static_cast<qintptr>(HTTOPLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, 2)), static_cast<qintptr>(HTTOPRIGHT));
    QCOMPARE(hitTest(QPoint(2, window.height() - 3)), static_cast<qintptr>(HTBOTTOMLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, window.height() - 3)), static_cast<qintptr>(HTBOTTOMRIGHT));
    QCOMPARE(hitTest(QPoint(2, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTLEFT));
    QCOMPARE(hitTest(QPoint(window.width() - 3, AntWindow::TitleBarHeight + 30)), static_cast<qintptr>(HTRIGHT));
    QCOMPARE(hitTest(QPoint(80, 2)), static_cast<qintptr>(HTTOP));
    QCOMPARE(hitTest(QPoint(80, window.height() - 3)), static_cast<qintptr>(HTBOTTOM));
    QCOMPARE(hitTest(QPoint(80, AntWindow::TitleBarHeight / 2)), static_cast<qintptr>(HTCAPTION));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Pin).center()), static_cast<qintptr>(HTCLIENT));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Theme).center()), static_cast<qintptr>(HTCLIENT));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Minimize).center()),
             static_cast<qintptr>(HTREDUCE));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Maximize).center()),
             static_cast<qintptr>(HTZOOM));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Close).center()),
             static_cast<qintptr>(HTCLOSE));

    auto sendNonClientButtonMessage = [&](UINT message, WPARAM hitTestCode, const QPoint& localPos) -> qintptr {
        MSG msg{};
        msg.hwnd = hwnd;
        msg.message = message;
        msg.wParam = hitTestCode;
        const QPoint globalPos = nativeGlobalPoint(localPos);
        msg.lParam = MAKELPARAM(globalPos.x(), globalPos.y());
        qintptr result = 0;
        if (!window.nativeEvent("windows_generic_MSG", &msg, &result))
        {
            return -1;
        }
        return result;
    };

    const QPoint maximizePoint = window.titleBarButtonRect(AntWindow::TitleBarButton::Maximize).center();
    QCOMPARE(hitTest(maximizePoint), static_cast<qintptr>(HTZOOM));
    QVERIFY(sendNonClientButtonMessage(WM_NCMOUSEMOVE, HTCLIENT, maximizePoint) != -1);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::Maximize);
    sendNonClientButtonMessage(WM_NCMOUSELEAVE, HTZOOM, maximizePoint);
    QCOMPARE(window.hoveredTitleBarButton(), AntWindow::TitleBarButton::None);
    QVERIFY(sendNonClientButtonMessage(WM_NCMOUSEMOVE, HTCLIENT, maximizePoint) != -1);
    QVERIFY(sendNonClientButtonMessage(WM_NCMOUSEHOVER, HTCLIENT, maximizePoint) != -1);
    QVERIFY(sendNonClientButtonMessage(WM_NCLBUTTONDOWN, HTZOOM, maximizePoint) != -1);
    QVERIFY(sendNonClientButtonMessage(WM_NCLBUTTONUP, HTZOOM, maximizePoint) != -1);
    QTRY_VERIFY(window.isMaximized());

    window.showMaximized();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    QCoreApplication::processEvents();

    QCOMPARE(hitTest(QPoint(80, AntWindow::TitleBarHeight / 2)), static_cast<qintptr>(HTCAPTION));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Maximize).center()),
             static_cast<qintptr>(HTZOOM));
    QCOMPARE(hitTest(window.titleBarButtonRect(AntWindow::TitleBarButton::Close).center()),
             static_cast<qintptr>(HTCLOSE));
#endif
}

void TestAntQtExtensions::windowDwmFrameMarginsPreserveShadow()
{
#ifndef Q_OS_WIN
    QSKIP("Windows DWM frame margins are only available on Windows.");
#else
    AntWindow window;
    window.resize(640, 420);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    const QVariant usesNativeCaption = window.property("antWindowUsesNativeCaptionFrame");
    QVERIFY2(usesNativeCaption.isValid(), "AntWindow should expose the active native caption frame policy for diagnostics");
    QCOMPARE(usesNativeCaption.toBool(), supportsNativeCaptionSnapLayoutsForTest());

    const QVariant frameMarginsProperty = window.property("antWindowDwmFrameMargins");
    QVERIFY2(frameMarginsProperty.isValid(), "AntWindow should expose the DWM frame margins applied to the native window");
    const QMargins margins = frameMarginsProperty.value<QMargins>();
    QCOMPARE(margins,
             supportsNativeCaptionSnapLayoutsForTest() ? QMargins(1, 1, 1, 1) : QMargins(0, 0, 0, 0));

    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd != nullptr);
    const LONG_PTR nativeStyle = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    if (!supportsNativeCaptionSnapLayoutsForTest())
    {
        QVERIFY2((nativeStyle & WS_CAPTION) == 0, "Windows 10 must keep the no-caption path to avoid native title buttons");
    }
#endif
}

void TestAntQtExtensions::windowLegacyFramePolicyRestoresShadowAfterResize()
{
#ifndef Q_OS_WIN
    QSKIP("Windows legacy frame policy is only available on Windows.");
#else
    AntWindow window;
    window.setProperty("antWindowForceLegacyFramePolicy", true);
    window.resize(640, 420);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QCOMPARE(window.property("antWindowUsesNativeCaptionFrame").toBool(), false);
    QCOMPARE(window.property("antWindowLegacyRoundedMaskApplied").toBool(), true);
    QCOMPARE(window.property("antWindowLegacyRoundedMaskFrameInset").toInt(), 1);
    QCOMPARE(window.property("antWindowDwmFrameMargins").value<QMargins>(), QMargins(0, 0, 0, 0));
    QCOMPARE(window.property("antWindowLegacyClassDropShadowEnabled").toBool(), false);
    QTRY_COMPARE(window.property("antWindowLegacySoftwareShadowEnabled").toBool(), true);
    QCOMPARE(window.property("antWindowLegacySoftwareShadowMargin").toInt(), 14);
    QVERIFY(window.property("antWindowLegacySoftwareShadowInnerClearance").isValid());
    QCOMPARE(window.property("antWindowLegacySoftwareShadowInnerClearance").toInt(), 0);
    const QRect initialShadowGeometry = window.property("antWindowLegacySoftwareShadowGeometry").toRect();
    QVERIFY(initialShadowGeometry.contains(window.geometry()));
    QVERIFY(initialShadowGeometry.left() < window.geometry().left());
    QVERIFY(initialShadowGeometry.top() < window.geometry().top());
    QVERIFY(initialShadowGeometry.right() > window.geometry().right());
    QVERIFY(initialShadowGeometry.bottom() > window.geometry().bottom());
    auto* shadowWidget = window.findChild<QWidget*>(QStringLiteral("AntWindowLegacySoftwareShadow"));
    QVERIFY(shadowWidget);
    QTRY_VERIFY(shadowWidget->isVisible());

    auto maxAlphaIn = [](const QImage& image, const QRect& sampleRect) {
        int maxAlpha = 0;
        const QRect clipped = sampleRect.intersected(image.rect());
        for (int y = clipped.top(); y <= clipped.bottom(); ++y)
        {
            for (int x = clipped.left(); x <= clipped.right(); ++x)
            {
                maxAlpha = qMax(maxAlpha, qAlpha(image.pixel(x, y)));
            }
        }
        return maxAlpha;
    };
    const int shadowMargin = window.property("antWindowLegacySoftwareShadowMargin").toInt();
    const QImage shadowImage = shadowWidget->grab().toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, shadowMargin, shadowMargin, shadowImage.height() - shadowMargin * 2)) > 0);
    QVERIFY(maxAlphaIn(shadowImage, QRect(shadowMargin, 0, shadowImage.width() - shadowMargin * 2, shadowMargin)) > 0);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowImage.width() - shadowMargin,
                             shadowMargin,
                             shadowMargin,
                             shadowImage.height() - shadowMargin * 2)) > 0);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin,
                             shadowImage.height() - shadowMargin,
                             shadowImage.width() - shadowMargin * 2,
                             shadowMargin)) > 0);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin - 2,
                             shadowMargin,
                             2,
                             shadowImage.height() - shadowMargin * 2)) > 4);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin,
                             shadowMargin - 2,
                             shadowImage.width() - shadowMargin * 2,
                             2)) > 4);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, 0, shadowMargin, shadowMargin)) <= 32);
    QVERIFY(maxAlphaIn(shadowImage, QRect(shadowImage.width() - shadowMargin, 0, shadowMargin, shadowMargin)) <= 32);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, shadowImage.height() - shadowMargin, shadowMargin, shadowMargin)) <= 32);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowImage.width() - shadowMargin,
                             shadowImage.height() - shadowMargin,
                             shadowMargin,
                             shadowMargin)) <= 32);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin - 2,
                             shadowMargin,
                             2,
                             shadowImage.height() - shadowMargin * 2)) <= 16);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin,
                             shadowMargin - 2,
                             shadowImage.width() - shadowMargin * 2,
                             2)) <= 16);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin - 2,
                             shadowMargin,
                             2,
                             shadowImage.height() - shadowMargin * 2)) <= 10);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowMargin,
                             shadowMargin - 2,
                             shadowImage.width() - shadowMargin * 2,
                             2)) <= 10);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, 0, shadowMargin, shadowMargin)) <= 20);
    QVERIFY(maxAlphaIn(shadowImage, QRect(shadowImage.width() - shadowMargin, 0, shadowMargin, shadowMargin)) <= 20);
    QVERIFY(maxAlphaIn(shadowImage, QRect(0, shadowImage.height() - shadowMargin, shadowMargin, shadowMargin)) <= 20);
    QVERIFY(maxAlphaIn(shadowImage,
                       QRect(shadowImage.width() - shadowMargin,
                             shadowImage.height() - shadowMargin,
                             shadowMargin,
                             shadowMargin)) <= 20);

    const int frameApplyCount = window.property("antWindowDwmFrameApplyCount").toInt();
    QVERIFY2(frameApplyCount > 0, "legacy frame policy should apply a shadow-preserving DWM frame");

    window.resize(700, 460);

    QVERIFY2(window.property("antWindowDwmFrameApplyCount").toInt() > frameApplyCount,
             "resizing a Win10 legacy-frame AntWindow should reapply the DWM frame after updating the mask");
    QTRY_VERIFY2(window.property("antWindowDwmFrameApplyCount").toInt() >= frameApplyCount + 2,
                 "resizing a Win10 legacy-frame AntWindow should queue a second DWM frame refresh after native resize settles");
    QCOMPARE(window.property("antWindowDwmFrameLastReason").toString(), QStringLiteral("resize-deferred"));

    const QRect resizedShadowGeometry = window.property("antWindowLegacySoftwareShadowGeometry").toRect();
    QVERIFY(resizedShadowGeometry.contains(window.geometry()));
    QCOMPARE(resizedShadowGeometry.marginsRemoved(QMargins(window.property("antWindowLegacySoftwareShadowMargin").toInt(),
                                                          window.property("antWindowLegacySoftwareShadowMargin").toInt(),
                                                          window.property("antWindowLegacySoftwareShadowMargin").toInt(),
                                                          window.property("antWindowLegacySoftwareShadowMargin").toInt())),
             window.geometry());
#endif
}

void TestAntQtExtensions::windowMaximizedNcCalcKeepsFullWorkArea()
{
#ifndef Q_OS_WIN
    QSKIP("Windows native frame sizing is only available on Windows.");
#else
    class NativeFrameWindow : public AntWindow
    {
    public:
        using AntWindow::nativeEvent;
    };

    NativeFrameWindow window;
    window.resize(640, 420);
    window.setWindowState(window.windowState() | Qt::WindowMaximized);
    QVERIFY(window.isMaximized());

    const HWND hwnd = reinterpret_cast<HWND>(window.winId());
    QVERIFY(hwnd != nullptr);

    NCCALCSIZE_PARAMS params{};
    params.rgrc[0] = RECT{100, 120, 900, 760};
    const RECT before = params.rgrc[0];

    MSG msg{};
    msg.hwnd = hwnd;
    msg.message = WM_NCCALCSIZE;
    msg.wParam = TRUE;
    msg.lParam = reinterpret_cast<LPARAM>(&params);

    qintptr result = -1;
    QVERIFY(window.nativeEvent("windows_generic_MSG", &msg, &result));
    QCOMPARE(result, static_cast<qintptr>(0));
    QCOMPARE(params.rgrc[0].left, before.left);
    QCOMPARE(params.rgrc[0].top, before.top);
    QCOMPARE(params.rgrc[0].right, before.right);
    QCOMPARE(params.rgrc[0].bottom, before.bottom);
#endif
}

void TestAntQtExtensions::colorPicker()
{
    auto* w = new AntColorPicker;
    QCOMPARE(w->currentColor(), QColor(Qt::white));
    QCOMPARE(w->showText(), false);

    QSignalSpy colorSpy(w, &AntColorPicker::currentColorChanged);
    w->setCurrentColor(Qt::red);
    QCOMPARE(w->currentColor(), QColor(Qt::red));
    QCOMPARE(colorSpy.count(), 1);

    QSignalSpy textSpy(w, &AntColorPicker::showTextChanged);
    w->setShowText(true);
    QCOMPARE(w->showText(), true);
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy openSpy(w, &AntColorPicker::openChanged);
    w->setOpen(true);
    QCOMPARE(w->isOpen(), true);
    QCOMPARE(openSpy.count(), 1);
    w->setOpen(false);
    QCOMPARE(w->isOpen(), false);
    QCOMPARE(openSpy.count(), 2);

    auto* w2 = new AntColorPicker(Qt::blue);
    QCOMPARE(w2->currentColor(), QColor(Qt::blue));
}

QTEST_MAIN(TestAntQtExtensions)
#include "TestAntQtExtensions.moc"
