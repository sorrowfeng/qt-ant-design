#include <QSignalSpy>
#include <QTest>
#include "widgets/AntApp.h"
#include "widgets/AntConfigProvider.h"
#include "widgets/AntForm.h"
#include "widgets/AntLog.h"
#include "widgets/AntPlainTextEdit.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntSplitter.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"
#include "widgets/AntColorPicker.h"

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
    void plainTextEdit();
    void scrollArea();
    void scrollBar();
    void splitter();
    void statusBar();
    void toolButton();
    void toolBar();
    void menuBar();
    void dockWidget();
    void widget();
    void window();
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
    auto* w = new AntLog;
    QCOMPARE(w->maxEntries(), 5000);
    QCOMPARE(w->autoScroll(), true);

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
    w->clear();
}

void TestAntQtExtensions::plainTextEdit()
{
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

    auto* w2 = new AntPlainTextEdit("Initial text");
    QCOMPARE(w2->toPlainText(), "Initial text");
}

void TestAntQtExtensions::scrollArea()
{
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
    QCOMPARE(msgSpy.count(), 1);

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
}

void TestAntQtExtensions::toolBar()
{
    auto* w = new AntToolBar;
    QVERIFY(w != nullptr);

    auto* w2 = new AntToolBar("My Toolbar");
    QVERIFY(w2 != nullptr);
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
    auto* w = new AntDockWidget;
    QCOMPARE(w->windowTitle(), QString());

    auto* w2 = new AntDockWidget("Properties");
    QCOMPARE(w2->windowTitle(), "Properties");
}

void TestAntQtExtensions::widget()
{
    auto* w = new AntWidget;
    QVERIFY(w->currentTheme() == Ant::ThemeMode::Default ||
            w->currentTheme() == Ant::ThemeMode::Dark);
}

void TestAntQtExtensions::window()
{
    auto* w = new AntWindow;
    QCOMPARE(w->isMaximized(), false);
    QCOMPARE(w->TitleBarHeight, 40);
    QCOMPARE(w->TitleBarButtonWidth, 46);
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

    auto* w2 = new AntColorPicker(Qt::blue);
    QCOMPARE(w2->currentColor(), QColor(Qt::blue));
}

QTEST_MAIN(TestAntQtExtensions)
#include "TestAntQtExtensions.moc"
