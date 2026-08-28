// M12 deep behavioral coverage.
//
// Adds signal / property / interaction assertions for the public widgets that
// historically only had smoke-level (construct + render) coverage:
//   AntToolBar, AntFileDialog, AntAutoComplete, AntBreadcrumb,
//   AntMenuBar, AntMentions, AntConfigProvider, AntRibbon.
//
// The intent is behavior regression detection (signals, set/get round-trips,
// user interaction), not pixel or performance checks.

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFontMetrics>
#include <QIcon>
#include <QLineEdit>
#include <QMenu>
#include <QPixmap>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QToolButton>

#include "core/AntDesign.h"
#include "core/AntTheme.h"
#include "widgets/AntAutoComplete.h"
#include "widgets/AntBreadcrumb.h"
#include "widgets/AntConfigProvider.h"
#include "widgets/AntFileDialog.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntMentions.h"
#include "widgets/AntRibbon.h"
#include "widgets/AntToolBar.h"

class TestAntDeepCoverage : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void toolBarActionLifecycle();
    void fileDialogValueApi();
    void autoCompleteInteractionCommit();
    void breadcrumbClickRouting();
    void menuBarMenuLifecycle();
    void mentionsSelectionCommit();
    void configProviderApply();
    void ribbonPageAndGroupManagement();
};

namespace
{
// Restores the global theme configuration that configProviderApply() touches.
class ThemeGuard
{
public:
    ThemeGuard()
        : m_mode(antTheme->themeMode()),
          m_fontSize(antTheme->fontSize()),
          m_borderRadius(antTheme->borderRadius())
    {
    }

    ~ThemeGuard()
    {
        antTheme->applyConfiguration(m_mode, QColor(), m_fontSize, m_borderRadius);
        QCoreApplication::processEvents();
    }

private:
    Ant::ThemeMode m_mode;
    int m_fontSize;
    int m_borderRadius;
};

// The breadcrumb layout cache measures text with the widget font forced to the
// theme pixel size; mirror that here so click positions are exact.
QFontMetrics breadcrumbMetrics(const AntBreadcrumb& breadcrumb)
{
    QFont f = breadcrumb.font();
    f.setPixelSize(antTheme->tokens().fontSize);
    return QFontMetrics(f);
}
} // namespace

void TestAntDeepCoverage::initTestCase()
{
    AntDesign::initialize(qobject_cast<QApplication*>(QCoreApplication::instance()));
}

void TestAntDeepCoverage::toolBarActionLifecycle()
{
    AntToolBar bar;
    QCOMPARE(bar.actions().size(), 0);

    auto* newAction = bar.addAction(QStringLiteral("New"));
    auto* openAction = bar.addAction(QStringLiteral("Open"));
    QCOMPARE(bar.actions().size(), 2);

    auto* newButton = qobject_cast<QToolButton*>(bar.widgetForAction(newAction));
    auto* openButton = qobject_cast<QToolButton*>(bar.widgetForAction(openAction));
    QVERIFY(newButton != nullptr);
    QVERIFY(openButton != nullptr);
    QVERIFY(newButton->property("antToolBarButton").toBool());
    QVERIFY(openButton->property("antToolBarButton").toBool());

    // Removing an action tears down its themed button.
    bar.removeAction(newAction);
    QCOMPARE(bar.actions().size(), 1);
    QVERIFY(bar.widgetForAction(newAction) == nullptr);
    QVERIFY(bar.widgetForAction(openAction) != nullptr);

    // Title constructor and QToolBar style knobs.
    AntToolBar titled(QStringLiteral("My Toolbar"));
    QCOMPARE(titled.windowTitle(), QStringLiteral("My Toolbar"));
    titled.setMovable(false);
    QCOMPARE(titled.isMovable(), false);
    titled.setToolButtonStyle(Qt::ToolButtonTextOnly);
    QCOMPARE(titled.toolButtonStyle(), Qt::ToolButtonTextOnly);

    bar.clear();
    QCOMPARE(bar.actions().size(), 0);
}

void TestAntDeepCoverage::fileDialogValueApi()
{
    AntFileDialog dialog;
    QCOMPARE(dialog.acceptMode(), QFileDialog::AcceptOpen);
    QCOMPARE(dialog.fileMode(), QFileDialog::ExistingFile);
    QVERIFY(dialog.testOption(QFileDialog::DontUseNativeDialog));
    QCOMPARE(dialog.nameFilters(), QStringList{QStringLiteral("All Files (*)")});

    dialog.setAcceptMode(QFileDialog::AcceptSave);
    QCOMPARE(dialog.acceptMode(), QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::Directory);
    QCOMPARE(dialog.fileMode(), QFileDialog::Directory);

    dialog.setNameFilter(QStringLiteral("Images (*.png *.jpg)"));
    QCOMPARE(dialog.nameFilters(), QStringList{QStringLiteral("Images (*.png *.jpg)")});
    dialog.setNameFilters({QStringLiteral("Text (*.txt)"), QStringLiteral("All (*.*)")});
    QCOMPARE(dialog.nameFilters().size(), 2);
    dialog.selectNameFilter(QStringLiteral("Text (*.txt)"));
    QCOMPARE(dialog.selectedNameFilter(), QStringLiteral("Text (*.txt)"));

    dialog.setDefaultSuffix(QStringLiteral("txt"));
    QCOMPARE(dialog.defaultSuffix(), QStringLiteral("txt"));

    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    QVERIFY(dialog.testOption(QFileDialog::ShowDirsOnly));
    QVERIFY(dialog.testOption(QFileDialog::DontUseNativeDialog));

    // The caption lands on the window title.
    AntFileDialog captioned(nullptr, QStringLiteral("Save File"));
    QCOMPARE(captioned.windowTitle(), QStringLiteral("Save File"));

    // Directory / selection round-trips against a real temp file.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QFile file(tempDir.filePath(QStringLiteral("hello.txt")));
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("hi", 2);
    file.close();

    AntFileDialog picker(nullptr, QString(), tempDir.path());
    QCOMPARE(picker.directory(), QDir(tempDir.path()));
    picker.selectFile(QStringLiteral("hello.txt"));
    QCOMPARE(picker.selectedFiles(),
             QStringList{QDir(tempDir.path()).absoluteFilePath(QStringLiteral("hello.txt"))});
    picker.setDirectory(tempDir.path());
    QCOMPARE(picker.directory(), QDir(tempDir.path()));
}

void TestAntDeepCoverage::autoCompleteInteractionCommit()
{
    AntAutoComplete autoComplete;
    autoComplete.addSuggestion(QStringLiteral("Apple"), QStringLiteral("fruit"));
    autoComplete.addSuggestion(QStringLiteral("Avocado"), QStringLiteral("vegetable"));
    QCOMPARE(autoComplete.suggestionCount(), 2);

    // Property set/get round-trips with signals.
    QSignalSpy placeholderSpy(&autoComplete, &AntAutoComplete::placeholderTextChanged);
    autoComplete.setPlaceholderText(QStringLiteral("Search fruit"));
    QCOMPARE(autoComplete.placeholderText(), QStringLiteral("Search fruit"));
    QCOMPARE(placeholderSpy.count(), 1);

    QSignalSpy caseSpy(&autoComplete, &AntAutoComplete::caseSensitivityChanged);
    autoComplete.setCaseSensitivity(Qt::CaseSensitive);
    QCOMPARE(autoComplete.caseSensitivity(), Qt::CaseSensitive);
    QCOMPARE(caseSpy.count(), 1);

    QSignalSpy textSpy(&autoComplete, &AntAutoComplete::textChanged);
    autoComplete.setText(QStringLiteral("programmatic"));
    QCOMPARE(autoComplete.text(), QStringLiteral("programmatic"));
    QCOMPARE(textSpy.count(), 0); // textChanged only fires on user input

    // removeSuggestion round-trip.
    autoComplete.removeSuggestion(1);
    QCOMPARE(autoComplete.suggestionCount(), 1);
    autoComplete.clearSuggestions();
    QCOMPARE(autoComplete.suggestionCount(), 0);

    // User commit flow: type a filter, highlight with Down, accept with Return.
    autoComplete.addSuggestion(QStringLiteral("Apple"), QStringLiteral("fruit"));
    autoComplete.addSuggestion(QStringLiteral("Avocado"), QStringLiteral("vegetable"));
    // The property round-trip above left CaseSensitive active, which would make
    // the lowercase "av" filter match nothing; restore the default first.
    autoComplete.setCaseSensitivity(Qt::CaseInsensitive);
    autoComplete.setText(QString()); // reset the editor before typing
    autoComplete.resize(260, autoComplete.sizeHint().height());
    autoComplete.show();
    QVERIFY(QTest::qWaitForWindowExposed(&autoComplete));

    auto* editor = autoComplete.findChild<QLineEdit*>();
    QVERIFY(editor != nullptr);
    editor->setFocus();
    QCoreApplication::processEvents();
    QSignalSpy commitSpy(&autoComplete, &AntAutoComplete::suggestionClicked);
    QTest::keyClicks(editor, QStringLiteral("av"));
    QTRY_VERIFY(autoComplete.property("antAutoCompletePopupItemCreateCount").toInt() > 0);
    QVERIFY(textSpy.count() >= 1);
    QCOMPARE(textSpy.at(textSpy.count() - 1).at(0).toString(), QStringLiteral("av"));

    QTest::keyClick(editor, Qt::Key_Down);
    // Invoke returnPressed directly: native Return routing is unreliable while
    // the ToolTip popup window is showing on Windows. This mirrors the suite's
    // existing textEdited invocation for the same reason.
    QVERIFY(QMetaObject::invokeMethod(editor, "returnPressed", Qt::DirectConnection));
    QCOMPARE(commitSpy.count(), 1);
    QCOMPARE(commitSpy.at(0).at(0).toString(), QStringLiteral("Avocado"));
    QCOMPARE(commitSpy.at(0).at(1).toString(), QStringLiteral("vegetable"));
    QCOMPARE(editor->text(), QStringLiteral("Avocado"));
}

void TestAntDeepCoverage::breadcrumbClickRouting()
{
    AntBreadcrumb breadcrumb;
    breadcrumb.addItem(QStringLiteral("Home"), QStringLiteral("/"));
    breadcrumb.addItem(QStringLiteral("Docs"), QStringLiteral("/docs"));
    breadcrumb.addItem(QStringLiteral("Secret"), QString(), QString(), true); // disabled
    breadcrumb.addItem(QStringLiteral("Guide"));                              // last route item
    QCOMPARE(breadcrumb.count(), 4);
    QCOMPARE(breadcrumb.itemAt(0).title, QStringLiteral("Home"));
    QCOMPARE(breadcrumb.itemAt(0).href, QStringLiteral("/"));
    QCOMPARE(breadcrumb.itemAt(2).disabled, true);
    QCOMPARE(breadcrumb.itemAt(2).separatorOnly, false);

    QSignalSpy separatorSpy(&breadcrumb, &AntBreadcrumb::separatorChanged);
    breadcrumb.setSeparator(QStringLiteral(">"));
    QCOMPARE(breadcrumb.separator(), QStringLiteral(">"));
    QCOMPARE(separatorSpy.count(), 1);

    breadcrumb.resize(500, 32);
    breadcrumb.show();
    QVERIFY(QTest::qWaitForWindowExposed(&breadcrumb));

    const QFontMetrics fm(breadcrumbMetrics(breadcrumb));
    const int sepWidth = fm.horizontalAdvance(QStringLiteral("/")) + antTheme->tokens().marginXS * 2;
    const int homeWidth = fm.horizontalAdvance(QStringLiteral("Home"));
    const int docsWidth = fm.horizontalAdvance(QStringLiteral("Docs"));
    const int secretWidth = fm.horizontalAdvance(QStringLiteral("Secret"));
    const int x0 = antTheme->tokens().paddingXS;

    QSignalSpy clickSpy(&breadcrumb, &AntBreadcrumb::itemClicked);

    // Clicking a normal route item emits itemClicked(index, title, href).
    QTest::mouseClick(&breadcrumb, Qt::LeftButton, Qt::NoModifier, QPoint(x0 + homeWidth / 2, 16));
    QCOMPARE(clickSpy.count(), 1);
    QCOMPARE(clickSpy.at(0).at(0).toInt(), 0);
    QCOMPARE(clickSpy.at(0).at(1).toString(), QStringLiteral("Home"));
    QCOMPARE(clickSpy.at(0).at(2).toString(), QStringLiteral("/"));

    // Disabled items swallow clicks.
    const int secretX = x0 + homeWidth + sepWidth + docsWidth + sepWidth;
    QTest::mouseClick(&breadcrumb, Qt::LeftButton, Qt::NoModifier,
                      QPoint(secretX + secretWidth / 2, 16));
    QCOMPARE(clickSpy.count(), 1);

    // The last route item is a "current page" label, not a link.
    const int guideX = secretX + secretWidth + sepWidth;
    QTest::mouseClick(&breadcrumb, Qt::LeftButton, Qt::NoModifier, QPoint(guideX + 20, 16));
    QCOMPARE(clickSpy.count(), 1);

    breadcrumb.clearItems();
    QCOMPARE(breadcrumb.count(), 0);
}

void TestAntDeepCoverage::menuBarMenuLifecycle()
{
    AntMenuBar bar;
    QCOMPARE(bar.actions().size(), 0);

    auto* fileMenu = bar.addMenu(QStringLiteral("&File"));
    QVERIFY(fileMenu != nullptr);
    QCOMPARE(fileMenu->title(), QStringLiteral("&File"));

    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::red);
    QIcon icon(pixmap);
    auto* editMenu = bar.addMenu(icon, QStringLiteral("Edit"));
    QVERIFY(editMenu != nullptr);
    QCOMPARE(editMenu->title(), QStringLiteral("Edit"));
    QVERIFY(!editMenu->icon().isNull());
    QCOMPARE(bar.actions().size(), 2);

    bar.removeAction(fileMenu->menuAction());
    QCOMPARE(bar.actions().size(), 1);
    QCOMPARE(bar.actions().constFirst(), editMenu->menuAction());

    bar.clear();
    QCOMPARE(bar.actions().size(), 0);

    bar.addMenu(QStringLiteral("View"));
    bar.setEnabled(false);
    QVERIFY(!bar.isEnabled());
}

void TestAntDeepCoverage::mentionsSelectionCommit()
{
    AntMentions mentions;
    QCOMPARE(mentions.prefix(), QStringLiteral("@"));
    QCOMPARE(mentions.rows(), 1);

    QSignalSpy rowsSpy(&mentions, &AntMentions::rowsChanged);
    mentions.setRows(3);
    QCOMPARE(mentions.rows(), 3);
    QCOMPARE(rowsSpy.count(), 1);

    QSignalSpy prefixSpy(&mentions, &AntMentions::prefixChanged);
    mentions.setPrefix(QStringLiteral("#"));
    QCOMPARE(mentions.prefix(), QStringLiteral("#"));
    QCOMPARE(prefixSpy.count(), 1);
    mentions.setPrefix(QStringLiteral("@"));
    QCOMPARE(prefixSpy.count(), 2);

    QSignalSpy placeholderSpy(&mentions, &AntMentions::placeholderTextChanged);
    mentions.setPlaceholderText(QStringLiteral("Mention someone"));
    QCOMPARE(mentions.placeholderText(), QStringLiteral("Mention someone"));
    QCOMPARE(placeholderSpy.count(), 1);

    mentions.setSuggestions({QStringLiteral("alice"), QStringLiteral("bob"), QStringLiteral("carol")});
    mentions.addSuggestion(QStringLiteral("dave"));

    mentions.resize(260, mentions.sizeHint().height());
    mentions.show();
    QVERIFY(QTest::qWaitForWindowExposed(&mentions));

    auto* editor = mentions.findChild<QLineEdit*>();
    QVERIFY(editor != nullptr);
    QSignalSpy selectSpy(&mentions, &AntMentions::mentionSelected);
    QTest::mouseClick(editor, Qt::LeftButton);
    QTest::keyClicks(editor, QStringLiteral("@bo"));
    QTRY_COMPARE(mentions.property("antMentionsVisibleSuggestionCount").toInt(), 1);

    QTest::keyClick(editor, Qt::Key_Return);
    QCOMPARE(selectSpy.count(), 1);
    QCOMPARE(selectSpy.at(0).at(0).toString(), QStringLiteral("bob"));
    QCOMPARE(editor->text(), QStringLiteral("@bob "));
}

void TestAntDeepCoverage::configProviderApply()
{
    ThemeGuard guard;
    AntConfigProvider provider;
    QCOMPARE(provider.revision(), 0);

    provider.setThemeMode(Ant::ThemeMode::Dark);
    provider.setPrimaryColor(QColor(Qt::red));
    provider.setFontSize(16);
    provider.setBorderRadius(8);
    QCOMPARE(provider.revision(), 0); // debounced, not yet delivered

    provider.apply();
    QCOMPARE(antTheme->themeMode(), Ant::ThemeMode::Dark);
    QCOMPARE(antTheme->fontSize(), 16);
    QCOMPARE(antTheme->borderRadius(), 8);
    QCOMPARE(provider.fontSize(), 16);
    QCOMPARE(provider.borderRadius(), 8);

    // The deferred configChanged still fires exactly once after apply().
    QTRY_COMPARE(provider.revision(), 1);

    // Re-applying the same configuration does not bump the revision.
    const int revisionBefore = provider.revision();
    provider.apply();
    QCOMPARE(antTheme->fontSize(), 16);
    QCOMPARE(provider.revision(), revisionBefore);
}

void TestAntDeepCoverage::ribbonPageAndGroupManagement()
{
    AntRibbon ribbon;
    auto* home = ribbon.addPage(QStringLiteral("Home"), QStringLiteral("home"));
    auto* insert = ribbon.addPage(QStringLiteral("Insert"), QStringLiteral("insert"));
    QVERIFY(home != nullptr);
    QVERIFY(insert != nullptr);
    QCOMPARE(ribbon.pageCount(), 2);
    QCOMPARE(ribbon.pageAt(0), home);
    QCOMPARE(ribbon.pageByKey(QStringLiteral("insert")), insert);
    QCOMPARE(ribbon.pageByKey(QStringLiteral("missing")), nullptr);

    auto* design = ribbon.insertPage(1, QStringLiteral("Design"), QStringLiteral("design"));
    QVERIFY(design != nullptr);
    QCOMPARE(ribbon.pageCount(), 3);
    QCOMPARE(ribbon.pageAt(1), design);

    // Group APIs on a page.
    auto* group = design->addGroup(QStringLiteral("Clipboard"));
    QVERIFY(group != nullptr);
    QCOMPARE(design->groupCount(), 1);
    QCOMPARE(design->groupAt(0), group);
    QCOMPARE(group->title(), QStringLiteral("Clipboard"));

    QSignalSpy groupTitleSpy(group, &AntRibbonGroup::titleChanged);
    group->setTitle(QStringLiteral("Clip"));
    QCOMPARE(group->title(), QStringLiteral("Clip"));
    QCOMPARE(groupTitleSpy.count(), 1);

    auto* pasteAction = group->addAction(QStringLiteral("Paste"), QIcon(), Ant::RibbonItemSize::Large);
    QVERIFY(pasteAction != nullptr);
    QCOMPARE(group->itemCount(), 1);

    auto* spacer = new QWidget(group);
    group->addWidget(spacer, Ant::RibbonItemSize::Large);
    QCOMPARE(group->itemCount(), 2);

    auto* insertGroup = design->insertGroup(0, QStringLiteral("Font"));
    QVERIFY(insertGroup != nullptr);
    QCOMPARE(design->groupCount(), 2);
    QCOMPARE(design->groupAt(0), insertGroup);
    design->removeGroup(0);
    QCOMPARE(design->groupCount(), 1);
    QCOMPARE(design->groupAt(0), group);
    design->clearGroups();
    QCOMPARE(design->groupCount(), 0);

    // actionTriggered chains group -> page -> ribbon.
    auto* chainGroup = design->addGroup(QStringLiteral("Chain"));
    auto* chainAction = chainGroup->addAction(QStringLiteral("Go"), QIcon(), Ant::RibbonItemSize::Large);
    QSignalSpy pageActionSpy(design, &AntRibbonPage::actionTriggered);
    QSignalSpy ribbonActionSpy(&ribbon, &AntRibbon::actionTriggered);
    chainAction->trigger();
    QCOMPARE(pageActionSpy.count(), 1);
    QCOMPARE(pageActionSpy.at(0).at(0).value<QAction*>(), chainAction);
    QCOMPARE(ribbonActionSpy.count(), 1);
    QCOMPARE(ribbonActionSpy.at(0).at(0).value<QAction*>(), chainAction);

    // Page key/title round-trips.
    QSignalSpy pageKeySpy(design, &AntRibbonPage::keyChanged);
    design->setKey(QStringLiteral("design2"));
    QCOMPARE(design->key(), QStringLiteral("design2"));
    QCOMPARE(pageKeySpy.count(), 1);
    QSignalSpy pageTitleSpy(design, &AntRibbonPage::titleChanged);
    design->setTitle(QStringLiteral("Design 2"));
    QCOMPARE(design->title(), QStringLiteral("Design 2"));
    QCOMPARE(pageTitleSpy.count(), 1);
    QCOMPARE(ribbon.pageByKey(QStringLiteral("design2")), design);

    // Current page navigation via key.
    QSignalSpy pageIndexSpy(&ribbon, &AntRibbon::currentPageChanged);
    QSignalSpy pageKeySpy2(&ribbon, &AntRibbon::currentPageKeyChanged);
    ribbon.setCurrentPageKey(QStringLiteral("insert"));
    QCOMPARE(ribbon.currentPageIndex(), 2);
    QCOMPARE(ribbon.currentPageKey(), QStringLiteral("insert"));
    QCOMPARE(pageIndexSpy.count(), 1);
    QCOMPARE(pageKeySpy2.count(), 1);
    ribbon.setCurrentPageKey(QStringLiteral("insert"));
    QCOMPARE(pageIndexSpy.count(), 1); // idempotent

    // Tab clicks route to pageClicked(key). Tab geometry mirrors tabRects():
    // x starts at 8, each tab is max(72, text + 32) wide with a 4px gap.
    ribbon.resize(400, 200);
    ribbon.show();
    QVERIFY(QTest::qWaitForWindowExposed(&ribbon));
    const QFontMetrics tabFm(ribbon.font());
    const int tabWidth0 = qMax(72, tabFm.horizontalAdvance(QStringLiteral("Home")) + 32);
    const int tabWidth1 = qMax(72, tabFm.horizontalAdvance(QStringLiteral("Design 2")) + 32);
    const int tabWidth2 = qMax(72, tabFm.horizontalAdvance(QStringLiteral("Insert")) + 32);
    const int tabX1 = 8 + tabWidth0 + 4;
    const int tabX2 = tabX1 + tabWidth1 + 4;
    QSignalSpy pageClickSpy(&ribbon, &AntRibbon::pageClicked);
    QTest::mouseClick(&ribbon, Qt::LeftButton, Qt::NoModifier, QPoint(8 + tabWidth0 / 2, 21));
    QCOMPARE(pageClickSpy.count(), 1);
    QCOMPARE(pageClickSpy.at(0).at(0).toString(), QStringLiteral("home"));
    QTest::mouseClick(&ribbon, Qt::LeftButton, Qt::NoModifier, QPoint(tabX1 + tabWidth1 / 2, 21));
    QCOMPARE(pageClickSpy.count(), 2);
    QCOMPARE(pageClickSpy.at(1).at(0).toString(), QStringLiteral("design2"));
    QTest::mouseClick(&ribbon, Qt::LeftButton, Qt::NoModifier, QPoint(tabX2 + tabWidth2 / 2, 21));
    QCOMPARE(pageClickSpy.count(), 3);
    QCOMPARE(pageClickSpy.at(2).at(0).toString(), QStringLiteral("insert"));
    QCOMPARE(ribbon.currentPageKey(), QStringLiteral("insert"));

    // Removal and teardown.
    ribbon.removePage(QStringLiteral("design2"));
    QCOMPARE(ribbon.pageCount(), 2);
    ribbon.clearPages();
    QCOMPARE(ribbon.pageCount(), 0);
}

QTEST_MAIN(TestAntDeepCoverage)

#include "TestAntDeepCoverage.moc"
