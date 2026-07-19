#include <QSignalSpy>
#include <QTest>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QImage>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPointer>
#include <QWheelEvent>
#include <QWidget>
#include <QTemporaryDir>
#include <QtMath>
#include "core/AntTheme.h"
#include "core/AntUrlPolicy.h"
#include "widgets/AntCascader.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntMentions.h"
#include "widgets/AntTransfer.h"
#include "widgets/AntTreeSelect.h"
#include "widgets/AntUpload.h"

class TestAntDataEntryB : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void mentionsReusesPopupRowsAndScopesHighlight();
    void cascaderPopupClosesOnOutsideClick();
    void cascaderPopupCachesColumnsAndScopesHover();
    void datePickerPopupCachesCellsAndScopesHover();
    void timePickerPopupCachesRowsAndScopesHover();
    void transferCachesVisibleRowsAndScopesUpdates();
    void treeSelectReusesPopupTreeAndCachesRows();
    void uploadCachesLayoutThumbsAndScopesUpdates();
    void uploadBoundsThumbnailDecodeAndEvictsLru();
    void uploadPreviewUsesExplicitUrlPolicy();
};

namespace
{
class ThemeModeRestorerForDataEntryBTest
{
public:
    ThemeModeRestorerForDataEntryBTest()
        : m_originalMode(antTheme->themeMode())
    {
    }

    ~ThemeModeRestorerForDataEntryBTest()
    {
        antTheme->setThemeMode(m_originalMode);
        QCoreApplication::processEvents();
    }

    Ant::ThemeMode alternateMode() const
    {
        return m_originalMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark;
    }

private:
    Ant::ThemeMode m_originalMode;
};

QPoint cascaderPopupCellCenter(int column, int row, int rowHeight = 32)
{
    constexpr int shadow = 32;
    constexpr int padding = 4;
    constexpr int columnWidth = 112;
    return QPoint(shadow + padding + column * columnWidth + columnWidth / 2,
                  shadow + padding + row * rowHeight + rowHeight / 2);
}

QFrame* directVisibleFrameChild(QWidget* owner)
{
    const auto frames = owner->findChildren<QFrame*>(QString(), Qt::FindDirectChildrenOnly);
    for (QFrame* frame : frames)
    {
        if (frame && frame->isVisible())
        {
            return frame;
        }
    }
    return nullptr;
}

QPoint datePickerPopupCellCenter(int column, int row)
{
    constexpr int shadowMargin = 32;
    constexpr int topMargin = 12;
    constexpr int panelWidth = 288;
    constexpr int gridLeftInset = 14;
    constexpr int gridTopInset = 88;
    constexpr qreal cellWidth = (panelWidth - gridLeftInset * 2) / 7.0;
    constexpr qreal cellHeight = 34.0;
    return QPoint(qRound(shadowMargin + gridLeftInset + (column + 0.5) * cellWidth),
                  qRound(topMargin + gridTopInset + (row + 0.5) * cellHeight));
}

QPoint timePickerPopupCellCenter(int column, int row)
{
    constexpr int shadowMargin = 32;
    constexpr int topMargin = 12;
    constexpr int panelWidth = 168;
    constexpr qreal columnWidth = panelWidth / 3.0;
    constexpr qreal rowHeight = 28.0;
    return QPoint(qRound(shadowMargin + (column + 0.5) * columnWidth),
                  qRound(topMargin + 8 + row * rowHeight + 12));
}

AntTreeNode treeSelectNode(const QString& key, const QString& title, QVector<AntTreeNode> children = {})
{
    AntTreeNode node;
    node.key = key;
    node.title = title;
    node.children = children;
    node.isLeaf = children.isEmpty();
    return node;
}

void sendMouseMove(QWidget* target, const QPoint& point)
{
    QMouseEvent event(QEvent::MouseMove,
                      QPointF(point),
                      QPointF(target->mapToGlobal(point)),
                      Qt::NoButton,
                      Qt::NoButton,
                      Qt::NoModifier);
    QCoreApplication::sendEvent(target, &event);
}
} // namespace

void TestAntDataEntryB::propertiesAndSignals()
{
    // AntCascader
    auto* w1 = new AntCascader;
    QCOMPARE(w1->cascaderSize(), Ant::Size::Middle);
    QCOMPARE(w1->status(), Ant::Status::Normal);
    QCOMPARE(w1->variant(), Ant::Variant::Outlined);
    QCOMPARE(w1->expandTrigger(), Ant::Trigger::Click);
    QCOMPARE(w1->isOpen(), false);
    QCOMPARE(w1->allowClear(), false);

    QSignalSpy sizeSpy1(w1, &AntCascader::cascaderSizeChanged);
    w1->setCascaderSize(Ant::Size::Large);
    QCOMPARE(w1->cascaderSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy1.count(), 1);

    QSignalSpy statusSpy1(w1, &AntCascader::statusChanged);
    w1->setStatus(Ant::Status::Error);
    QCOMPARE(w1->status(), Ant::Status::Error);
    QCOMPARE(statusSpy1.count(), 1);

    QSignalSpy triggerSpy(w1, &AntCascader::expandTriggerChanged);
    w1->setExpandTrigger(Ant::Trigger::Hover);
    QCOMPARE(w1->expandTrigger(), Ant::Trigger::Hover);
    QCOMPARE(triggerSpy.count(), 1);

    QSignalSpy clearSpy1(w1, &AntCascader::allowClearChanged);
    w1->setAllowClear(true);
    QCOMPARE(w1->allowClear(), true);
    QCOMPARE(clearSpy1.count(), 1);

    w1->setPlaceholder("Select...");
    QCOMPARE(w1->placeholder(), "Select...");

    // AntDatePicker
    auto* w2 = new AntDatePicker;
    QCOMPARE(w2->pickerSize(), Ant::Size::Middle);
    QCOMPARE(w2->status(), Ant::Status::Normal);
    QCOMPARE(w2->variant(), Ant::Variant::Outlined);
    QCOMPARE(w2->allowClear(), true);
    QCOMPARE(w2->isOpen(), false);
    QCOMPARE(w2->hasSelectedDate(), false);
    QCOMPARE(w2->isHoveredState(), false);

    QSignalSpy sizeSpy2(w2, &AntDatePicker::pickerSizeChanged);
    w2->setPickerSize(Ant::Size::Large);
    QCOMPARE(w2->pickerSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy2.count(), 1);

    QSignalSpy statusSpy2(w2, &AntDatePicker::statusChanged);
    w2->setStatus(Ant::Status::Error);
    QCOMPARE(w2->status(), Ant::Status::Error);
    QCOMPARE(statusSpy2.count(), 1);

    QSignalSpy dateSpy(w2, &AntDatePicker::selectedDateChanged);
    w2->setSelectedDate(QDate(2026, 4, 26));
    QCOMPARE(w2->selectedDate(), QDate(2026, 4, 26));
    QCOMPARE(w2->hasSelectedDate(), true);
    QCOMPARE(dateSpy.count(), 1);

    QSignalSpy dateRangeSpy(w2, &AntDatePicker::dateRangeChanged);
    w2->setDateRange(QDate(2026, 4, 1), QDate(2026, 4, 30));
    QCOMPARE(w2->minimumDate(), QDate(2026, 4, 1));
    QCOMPARE(w2->maximumDate(), QDate(2026, 4, 30));
    QCOMPARE(dateRangeSpy.count(), 1);

    QSignalSpy dateAliasSpy(w2, &AntDatePicker::dateChanged);
    w2->setDate(QDate(2026, 5, 9));
    QCOMPARE(w2->date(), QDate(2026, 4, 30));
    QCOMPARE(w2->selectedDate(), QDate(2026, 4, 30));
    QCOMPARE(dateAliasSpy.count(), 1);

    w2->clearMaximumDate();
    QCOMPARE(w2->maximumDate(), QDate(9999, 12, 31));

    QSignalSpy clearSpy2(w2, &AntDatePicker::allowClearChanged);
    w2->setAllowClear(false);
    QCOMPARE(w2->allowClear(), false);
    QCOMPARE(clearSpy2.count(), 1);

    w2->setDisplayFormat("yyyy-MM-dd");
    QCOMPARE(w2->displayFormat(), "yyyy-MM-dd");

    auto* dateRange = new AntDatePicker;
    dateRange->setRangeMode(true);
    dateRange->setStartDate(QDate(2026, 4, 1));
    dateRange->setEndDate(QDate(2026, 4, 8));
    QCOMPARE(dateRange->hasSelectedDate(), true);
    QCOMPARE(dateRange->dateString(), QStringLiteral("2026-04-01 - 2026-04-08"));
    dateRange->clear();
    QCOMPARE(dateRange->hasSelectedDate(), false);

    // AntTimePicker
    auto* w3 = new AntTimePicker;
    QCOMPARE(w3->pickerSize(), Ant::Size::Middle);
    QCOMPARE(w3->status(), Ant::Status::Normal);
    QCOMPARE(w3->variant(), Ant::Variant::Outlined);
    QCOMPARE(w3->allowClear(), true);
    QCOMPARE(w3->isOpen(), false);
    QCOMPARE(w3->hasSelectedTime(), false);
    QCOMPARE(w3->isHoveredState(), false);

    QSignalSpy sizeSpy3(w3, &AntTimePicker::pickerSizeChanged);
    w3->setPickerSize(Ant::Size::Small);
    QCOMPARE(w3->pickerSize(), Ant::Size::Small);
    QCOMPARE(sizeSpy3.count(), 1);

    QSignalSpy timeSpy(w3, &AntTimePicker::selectedTimeChanged);
    w3->setSelectedTime(QTime(14, 30, 0));
    QCOMPARE(w3->selectedTime(), QTime(14, 30, 0));
    QCOMPARE(w3->hasSelectedTime(), true);
    QCOMPARE(timeSpy.count(), 1);

    QSignalSpy timeRangeSpy(w3, &AntTimePicker::timeRangeChanged);
    w3->setTimeRange(QTime(9, 0), QTime(18, 0));
    QCOMPARE(w3->minimumTime(), QTime(9, 0));
    QCOMPARE(w3->maximumTime(), QTime(18, 0));
    QCOMPARE(timeRangeSpy.count(), 1);

    QSignalSpy timeAliasSpy(w3, &AntTimePicker::timeChanged);
    w3->setTime(QTime(20, 15, 0));
    QCOMPARE(w3->time(), QTime(18, 0));
    QCOMPARE(w3->selectedTime(), QTime(18, 0));
    QCOMPARE(timeAliasSpy.count(), 1);

    w3->clearMinimumTime();
    QCOMPARE(w3->minimumTime(), QTime(0, 0));

    QSignalSpy hourSpy(w3, &AntTimePicker::hourStepChanged);
    w3->setHourStep(2);
    QCOMPARE(w3->hourStep(), 2);
    QCOMPARE(hourSpy.count(), 1);

    QSignalSpy minSpy(w3, &AntTimePicker::minuteStepChanged);
    w3->setMinuteStep(15);
    QCOMPARE(w3->minuteStep(), 15);
    QCOMPARE(minSpy.count(), 1);

    auto* timeRange = new AntTimePicker;
    timeRange->setRangeMode(true);
    timeRange->setDisplayFormat(QStringLiteral("HH:mm"));
    timeRange->setStartTime(QTime(9, 0));
    timeRange->setEndTime(QTime(18, 30));
    QCOMPARE(timeRange->hasSelectedTime(), true);
    QCOMPARE(timeRange->timeString(), QStringLiteral("09:00 - 18:30"));
    timeRange->clear();
    QCOMPARE(timeRange->hasSelectedTime(), false);

    // AntMentions
    auto* w4 = new AntMentions;
    QCOMPARE(w4->text(), QString());
    QCOMPARE(w4->prefix(), "@");

    QSignalSpy prefixSpy(w4, &AntMentions::prefixChanged);
    w4->setPrefix("#");
    QCOMPARE(w4->prefix(), "#");
    QCOMPARE(prefixSpy.count(), 1);

    w4->addSuggestion("alice");
    w4->addSuggestion("bob");
    w4->setSuggestions({"charlie", "dave"});

    // AntTransfer
    auto* w5 = new AntTransfer;
    for (QWidget* child : w5->findChildren<QWidget*>())
    {
        QVERIFY(QString::fromLatin1(child->metaObject()->className()) != QStringLiteral("QListWidget"));
    }
    QCOMPARE(w5->sourceItems().isEmpty(), true);
    QCOMPARE(w5->targetItems().isEmpty(), true);

    w5->setSourceItems({"A", "B", "C"});
    QCOMPARE(w5->sourceItems().size(), 3);

    QSignalSpy itemsSpy(w5, &AntTransfer::itemsChanged);
    w5->setTargetItems({"A"});
    QCOMPARE(w5->targetItems().size(), 1);
    QVERIFY(itemsSpy.count() >= 1);

    auto* allTransfer = new AntTransfer;
    allTransfer->setSourceItems({"A", "B", "C"});
    allTransfer->resize(allTransfer->sizeHint());
    QTest::mouseClick(allTransfer, Qt::LeftButton, Qt::NoModifier, QPoint(20, 20));
    QTest::mouseClick(allTransfer, Qt::LeftButton, Qt::NoModifier, QPoint(200, 86));
    QCOMPARE(allTransfer->sourceItems().size(), 0);
    QCOMPARE(allTransfer->targetItems(), QStringList({"A", "B", "C"}));

    auto* scrollTransfer = new AntTransfer;
    scrollTransfer->setSourceItems({"A", "B", "C", "D", "E", "F", "G"});
    scrollTransfer->resize(scrollTransfer->sizeHint());
    const QPointF wheelPos(90, 72);
    QWheelEvent wheelEvent(wheelPos, wheelPos, QPoint(), QPoint(0, -120),
                           Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(scrollTransfer, &wheelEvent);
    QTest::mouseClick(scrollTransfer, Qt::LeftButton, Qt::NoModifier, QPoint(20, 56));
    QTest::mouseClick(scrollTransfer, Qt::LeftButton, Qt::NoModifier, QPoint(200, 86));
    QCOMPARE(scrollTransfer->targetItems(), QStringList({"B"}));

    // AntTreeSelect
    auto* w6 = new AntTreeSelect;
    QCOMPARE(w6->selectSize(), Ant::Size::Middle);
    QCOMPARE(w6->status(), Ant::Status::Normal);
    QCOMPARE(w6->variant(), Ant::Variant::Outlined);
    QCOMPARE(w6->isMultiple(), false);
    QCOMPARE(w6->isTreeCheckable(), false);
    QCOMPARE(w6->isOpen(), false);

    QSignalSpy sizeSpy6(w6, &AntTreeSelect::selectSizeChanged);
    w6->setSelectSize(Ant::Size::Large);
    QCOMPARE(w6->selectSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy6.count(), 1);

    QSignalSpy multiSpy6(w6, &AntTreeSelect::multipleChanged);
    w6->setMultiple(true);
    QCOMPARE(w6->isMultiple(), true);
    QCOMPARE(multiSpy6.count(), 1);

    QSignalSpy checkSpy6(w6, &AntTreeSelect::treeCheckableChanged);
    w6->setTreeCheckable(true);
    QCOMPARE(w6->isTreeCheckable(), true);
    QCOMPARE(checkSpy6.count(), 1);

    w6->setPlaceholder("Select tree...");
    QCOMPARE(w6->placeholder(), "Select tree...");

    // AntUpload
    auto* w7 = new AntUpload;
    QCOMPARE(w7->isMultiple(), false);
    QCOMPARE(w7->maxCount(), 0);
    QCOMPARE(w7->isDisabled(), false);
    QCOMPARE(w7->listType(), Ant::UploadListType::Text);
    QCOMPARE(w7->isDraggerMode(), false);

    QSignalSpy multiSpy7(w7, &AntUpload::multipleChanged);
    w7->setMultiple(true);
    QCOMPARE(w7->isMultiple(), true);
    QCOMPARE(multiSpy7.count(), 1);

    QSignalSpy maxSpy7(w7, &AntUpload::maxCountChanged);
    w7->setMaxCount(5);
    QCOMPARE(w7->maxCount(), 5);
    QCOMPARE(maxSpy7.count(), 1);

    QSignalSpy disabledSpy7(w7, &AntUpload::disabledChanged);
    w7->setDisabled(true);
    QCOMPARE(w7->isDisabled(), true);
    QCOMPARE(disabledSpy7.count(), 1);

    QSignalSpy typeSpy7(w7, &AntUpload::listTypeChanged);
    w7->setListType(Ant::UploadListType::PictureCard);
    QCOMPARE(w7->listType(), Ant::UploadListType::PictureCard);
    QCOMPARE(typeSpy7.count(), 1);

    QSignalSpy draggerSpy(w7, &AntUpload::draggerModeChanged);
    w7->setDraggerMode(true);
    QCOMPARE(w7->isDraggerMode(), true);
    QCOMPARE(draggerSpy.count(), 1);

    w7->setAccept(".jpg,.png");
    QCOMPARE(w7->accept(), ".jpg,.png");
}

void TestAntDataEntryB::mentionsReusesPopupRowsAndScopesHighlight()
{
    AntMentions mentions;
    mentions.setSuggestions({QStringLiteral("alice"),
                             QStringLiteral("alex"),
                             QStringLiteral("mallory"),
                             QStringLiteral("bob"),
                             QStringLiteral("zara"),
                             QStringLiteral("aaron"),
                             QStringLiteral("albert")});
    mentions.resize(260, mentions.sizeHint().height());
    mentions.show();
    QVERIFY(QTest::qWaitForWindowExposed(&mentions));

    auto* editor = mentions.findChild<QLineEdit*>();
    QVERIFY(editor);
    editor->setFocus();
    QTest::keyClicks(editor, QStringLiteral("@a"));

    auto* popup = mentions.findChild<QFrame*>(QStringLiteral("AntMentionsPopup"));
    QVERIFY(popup);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);
    QTRY_COMPARE(mentions.property("antMentionsVisibleSuggestionCount").toInt(), 6);
    QCOMPARE(mentions.property("antMentionsPopupRowBuildCount").toInt(), 6);
    QVERIFY(mentions.property("antMentionsFilterResolveCount").toInt() > 0);

    const int rowsBuiltBeforeNarrow = mentions.property("antMentionsPopupRowBuildCount").toInt();
    QTest::keyClicks(editor, QStringLiteral("l"));
    QTRY_COMPARE(mentions.property("antMentionsVisibleSuggestionCount").toInt(), 4);
    QCOMPARE(mentions.property("antMentionsPopupRowBuildCount").toInt(), rowsBuiltBeforeNarrow);

    const int highlightedUpdatesBefore = mentions.property("antMentionsHighlightedRowUpdateCount").toInt();
    QTest::keyClick(editor, Qt::Key_Down);
    QTRY_VERIFY(mentions.property("antMentionsHighlightedRowUpdateCount").toInt() > highlightedUpdatesBefore);
    const int highlightedUpdatesAfterFirstMove = mentions.property("antMentionsHighlightedRowUpdateCount").toInt();
    QTest::keyClick(editor, Qt::Key_Down);
    QTRY_VERIFY(mentions.property("antMentionsHighlightedRowUpdateCount").toInt() > highlightedUpdatesAfterFirstMove);

    ThemeModeRestorerForDataEntryBTest restoreMentionsTheme;
    const int mentionsThemeGeometryUpdates = mentions.property("antThemeRefreshUpdateGeometryCount").toInt();
    antTheme->setThemeMode(restoreMentionsTheme.alternateMode());
    QCoreApplication::processEvents();
    QCOMPARE(mentions.property("antThemeRefreshSizeHintChanged").toBool(), false);
    QCOMPARE(mentions.property("antThemeRefreshUpdateGeometryCount").toInt(), mentionsThemeGeometryUpdates);
    QCOMPARE(mentions.property("antMentionsThemeFixedHeightChanged").toBool(), false);
    QVERIFY(popup->isVisible());

    QSignalSpy selectedSpy(&mentions, &AntMentions::mentionSelected);
    QTest::keyClick(editor, Qt::Key_Return);
    QCOMPARE(selectedSpy.count(), 1);
    QCOMPARE(mentions.text(), QStringLiteral("@mallory "));
    QTRY_VERIFY_WITH_TIMEOUT(!popup->isVisible(), 500);
}

void TestAntDataEntryB::cascaderPopupClosesOnOutsideClick()
{
    AntCascaderOption hangzhou;
    hangzhou.value = QStringLiteral("hangzhou");
    hangzhou.label = QStringLiteral("Hangzhou");
    hangzhou.isLeaf = true;

    AntCascaderOption zhejiang;
    zhejiang.value = QStringLiteral("zhejiang");
    zhejiang.label = QStringLiteral("Zhejiang");
    zhejiang.children = {hangzhou};
    zhejiang.isLeaf = false;

    QWidget host;
    host.resize(420, 260);

    AntCascader cascader(&host);
    cascader.setOptions({zhejiang});
    cascader.resize(180, cascader.sizeHint().height());
    cascader.move(16, 16);
    cascader.show();

    host.show();
    QVERIFY(QTest::qWaitForWindowExposed(&host));

    QSignalSpy openSpy(&cascader, &AntCascader::openChanged);

    QTest::mouseClick(&cascader, Qt::LeftButton, Qt::NoModifier, cascader.rect().center());
    QTRY_VERIFY_WITH_TIMEOUT(cascader.isOpen(), 300);

    auto* popup = cascader.findChild<QFrame*>(QStringLiteral("CascaderPopup"));
    QVERIFY(popup);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);

    QTest::mouseClick(&host, Qt::LeftButton, Qt::NoModifier, QPoint(360, 220));
    QTRY_VERIFY_WITH_TIMEOUT(!cascader.isOpen(), 300);
    QTRY_VERIFY_WITH_TIMEOUT(!popup->isVisible(), 500);
    QVERIFY(openSpy.count() >= 2);
}

void TestAntDataEntryB::cascaderPopupCachesColumnsAndScopesHover()
{
    AntCascaderOption hangzhou{QStringLiteral("hangzhou"), QStringLiteral("Hangzhou"), {}, false, true};
    AntCascaderOption ningbo{QStringLiteral("ningbo"), QStringLiteral("Ningbo"), {}, false, true};
    AntCascaderOption zhejiang{QStringLiteral("zhejiang"), QStringLiteral("Zhejiang"), {hangzhou, ningbo}, false, false};
    AntCascaderOption nanjing{QStringLiteral("nanjing"), QStringLiteral("Nanjing"), {}, false, true};
    AntCascaderOption jiangsu{QStringLiteral("jiangsu"), QStringLiteral("Jiangsu"), {nanjing}, false, false};

    AntCascader cascader;
    cascader.setOptions({zhejiang, jiangsu});
    cascader.resize(180, cascader.sizeHint().height());
    cascader.show();
    QVERIFY(QTest::qWaitForWindowExposed(&cascader));

    cascader.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(cascader.isOpen(), 300);
    auto* popup = cascader.findChild<QFrame*>(QStringLiteral("CascaderPopup"));
    QVERIFY(popup);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);
    QVERIFY(cascader.property("antCascaderPopupColumnBuildCount").toInt() > 0);
    QVERIFY(cascader.property("antCascaderPopupGeometryApplyCount").toInt() > 0);

    cascader.setOpen(false);
    QTRY_VERIFY_WITH_TIMEOUT(!popup->isVisible(), 500);
    const int columnHitsBeforeReopen = cascader.property("antCascaderPopupColumnCacheHitCount").toInt();
    cascader.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);
    QVERIFY(cascader.property("antCascaderPopupColumnCacheHitCount").toInt() > columnHitsBeforeReopen);

    QCoreApplication::processEvents();
    const int scopedRowsBeforeHover = cascader.property("antCascaderPopupScopedRowUpdateCount").toInt();
    const QPoint firstHoverPoint = cascaderPopupCellCenter(0, 0);
    QMouseEvent firstHoverEvent(QEvent::MouseMove,
                                QPointF(firstHoverPoint),
                                QPointF(popup->mapToGlobal(firstHoverPoint)),
                                Qt::NoButton,
                                Qt::NoButton,
                                Qt::NoModifier);
    QCoreApplication::sendEvent(popup, &firstHoverEvent);
    QVERIFY(cascader.property("antCascaderPopupScopedRowUpdateCount").toInt() > scopedRowsBeforeHover);

    const int scopedRowsBeforeSameHover = cascader.property("antCascaderPopupScopedRowUpdateCount").toInt();
    QMouseEvent sameHoverEvent(QEvent::MouseMove,
                               QPointF(firstHoverPoint),
                               QPointF(popup->mapToGlobal(firstHoverPoint)),
                               Qt::NoButton,
                               Qt::NoButton,
                               Qt::NoModifier);
    QCoreApplication::sendEvent(popup, &sameHoverEvent);
    QCOMPARE(cascader.property("antCascaderPopupScopedRowUpdateCount").toInt(), scopedRowsBeforeSameHover);

    const int columnBuildsBeforeExpand = cascader.property("antCascaderPopupColumnBuildCount").toInt();
    const int scopedColumnsBeforeExpand = cascader.property("antCascaderPopupScopedColumnUpdateCount").toInt();
    QTest::mouseClick(popup, Qt::LeftButton, Qt::NoModifier, cascaderPopupCellCenter(0, 0));
    QCOMPARE(cascader.property("antCascaderPopupColumnBuildCount").toInt(), columnBuildsBeforeExpand);
    QVERIFY(cascader.property("antCascaderPopupScopedColumnUpdateCount").toInt() > scopedColumnsBeforeExpand);
    QCOMPARE(cascader.value(), QStringList({QStringLiteral("zhejiang")}));

    cascader.setOpen(false);
    QTRY_VERIFY_WITH_TIMEOUT(!popup->isVisible(), 500);
    const QSize expandedPopupSize = popup->size();
    const int geometryDecisionsBefore = cascader.property("antCascaderPopupGeometryApplyCount").toInt()
        + cascader.property("antCascaderPopupGeometrySkipCount").toInt();
    cascader.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);
    QCOMPARE(popup->size(), expandedPopupSize);
    const int geometryDecisionsAfter = cascader.property("antCascaderPopupGeometryApplyCount").toInt()
        + cascader.property("antCascaderPopupGeometrySkipCount").toInt();
    QVERIFY(geometryDecisionsAfter > geometryDecisionsBefore);
}

void TestAntDataEntryB::datePickerPopupCachesCellsAndScopesHover()
{
    AntDatePicker picker;
    picker.setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    picker.setSelectedDate(QDate(2026, 4, 1));
    picker.resize(picker.sizeHint());
    picker.show();
    QVERIFY(QTest::qWaitForWindowExposed(&picker));

    picker.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(picker.isOpen(), 300);
    auto* popup = directVisibleFrameChild(&picker);
    QVERIFY(popup);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);

    const QPoint firstCell = datePickerPopupCellCenter(3, 2);
    const int scopedBeforeHover = picker.property("antDatePickerPopupScopedCellUpdateCount").toInt();
    sendMouseMove(popup, firstCell);
    QVERIFY(picker.property("antDatePickerPopupDayCacheBuildCount").toInt() > 0);
    QVERIFY(picker.property("antDatePickerPopupScopedCellUpdateCount").toInt() > scopedBeforeHover);

    const int scopedBeforeSameHover = picker.property("antDatePickerPopupScopedCellUpdateCount").toInt();
    const int cacheHitsBeforeSameHover = picker.property("antDatePickerPopupDayCacheHitCount").toInt();
    sendMouseMove(popup, firstCell);
    QCOMPARE(picker.property("antDatePickerPopupScopedCellUpdateCount").toInt(), scopedBeforeSameHover);
    QVERIFY(picker.property("antDatePickerPopupDayCacheHitCount").toInt() > cacheHitsBeforeSameHover);

    const int scopedBeforeSecondHover = picker.property("antDatePickerPopupScopedCellUpdateCount").toInt();
    sendMouseMove(popup, datePickerPopupCellCenter(4, 2));
    QVERIFY(picker.property("antDatePickerPopupScopedCellUpdateCount").toInt() > scopedBeforeSecondHover);

    const int buildsBeforeMonthChange = picker.property("antDatePickerPopupDayCacheBuildCount").toInt();
    picker.setFocus();
    QTest::keyClick(&picker, Qt::Key_Right);
    sendMouseMove(popup, datePickerPopupCellCenter(0, 0));
    QVERIFY(picker.property("antDatePickerPopupDayCacheBuildCount").toInt() > buildsBeforeMonthChange);
}

void TestAntDataEntryB::timePickerPopupCachesRowsAndScopesHover()
{
    AntTimePicker picker;
    picker.setSelectedTime(QTime(10, 20, 30));
    picker.resize(picker.sizeHint());
    picker.show();
    QVERIFY(QTest::qWaitForWindowExposed(&picker));

    picker.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(picker.isOpen(), 300);
    auto* popup = picker.findChild<QFrame*>(QStringLiteral("AntTimePickerPopup"));
    QVERIFY(popup);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);
    QVERIFY(picker.property("antTimePickerPopupGeometryApplyCount").toInt() > 0);

    const QPoint firstRow = timePickerPopupCellCenter(0, 1);
    const int scopedRowsBeforeHover = picker.property("antTimePickerPopupScopedRowUpdateCount").toInt();
    sendMouseMove(popup, firstRow);
    QVERIFY(picker.property("antTimePickerPopupLayoutBuildCount").toInt() > 0);
    QVERIFY(picker.property("antTimePickerPopupScopedRowUpdateCount").toInt() > scopedRowsBeforeHover);

    const int scopedRowsBeforeSameHover = picker.property("antTimePickerPopupScopedRowUpdateCount").toInt();
    const int layoutHitsBeforeSameHover = picker.property("antTimePickerPopupLayoutCacheHitCount").toInt();
    sendMouseMove(popup, firstRow);
    QCOMPARE(picker.property("antTimePickerPopupScopedRowUpdateCount").toInt(), scopedRowsBeforeSameHover);
    QVERIFY(picker.property("antTimePickerPopupLayoutCacheHitCount").toInt() > layoutHitsBeforeSameHover);

    const int scopedRowsBeforeSecondHover = picker.property("antTimePickerPopupScopedRowUpdateCount").toInt();
    sendMouseMove(popup, timePickerPopupCellCenter(1, 1));
    QVERIFY(picker.property("antTimePickerPopupScopedRowUpdateCount").toInt() > scopedRowsBeforeSecondHover);

    const int scopedColumnsBeforeTimeChange = picker.property("antTimePickerPopupScopedColumnUpdateCount").toInt();
    picker.setSelectedTime(QTime(11, 20, 30));
    QVERIFY(picker.property("antTimePickerPopupScopedColumnUpdateCount").toInt() > scopedColumnsBeforeTimeChange);
    QCOMPARE(picker.property("antTimePickerPopupLastUpdateMode").toString(), QStringLiteral("time"));

    const int scopedColumnsBeforeStepChange = picker.property("antTimePickerPopupScopedColumnUpdateCount").toInt();
    picker.setMinuteStep(15);
    QVERIFY(picker.property("antTimePickerPopupScopedColumnUpdateCount").toInt() > scopedColumnsBeforeStepChange);
    QCOMPARE(picker.property("antTimePickerPopupLastUpdateMode").toString(), QStringLiteral("step"));

    picker.setOpen(false);
    QTRY_VERIFY_WITH_TIMEOUT(!popup->isVisible(), 500);
    const int geometrySkipsBeforeReopen = picker.property("antTimePickerPopupGeometrySkipCount").toInt();
    picker.setOpen(true);
    QTRY_VERIFY_WITH_TIMEOUT(popup->isVisible(), 300);
    QVERIFY(picker.property("antTimePickerPopupGeometrySkipCount").toInt() > geometrySkipsBeforeReopen);
}

void TestAntDataEntryB::transferCachesVisibleRowsAndScopesUpdates()
{
    AntTransfer transfer;
    transfer.setSourceItems({QStringLiteral("A"),
                             QStringLiteral("B"),
                             QStringLiteral("C"),
                             QStringLiteral("D"),
                             QStringLiteral("E"),
                             QStringLiteral("F"),
                             QStringLiteral("G")});
    transfer.setTargetItems({QStringLiteral("One")});
    transfer.resize(transfer.sizeHint());
    transfer.show();
    QVERIFY(QTest::qWaitForWindowExposed(&transfer));

    transfer.grab();
    const int layoutBuildsBefore = transfer.property("antTransferLayoutBuildCount").toInt();
    const int layoutHitsBefore = transfer.property("antTransferLayoutCacheHitCount").toInt();
    transfer.grab();
    QCOMPARE(transfer.property("antTransferLayoutBuildCount").toInt(), layoutBuildsBefore);
    QVERIFY(transfer.property("antTransferLayoutCacheHitCount").toInt() > layoutHitsBefore);

    const int rowUpdatesBeforeSelection = transfer.property("antTransferRowRegionUpdateCount").toInt();
    QTest::mouseClick(&transfer, Qt::LeftButton, Qt::NoModifier, QPoint(20, 56));
    QVERIFY(transfer.property("antTransferRowRegionUpdateCount").toInt() > rowUpdatesBeforeSelection);
    QCOMPARE(transfer.property("antTransferLastUpdateMode").toString(), QStringLiteral("selection"));

    const int panelUpdatesBeforeScroll = transfer.property("antTransferPanelRegionUpdateCount").toInt();
    const QPointF wheelPos(90, 72);
    QWheelEvent wheelEvent(wheelPos, wheelPos, QPoint(), QPoint(0, -120),
                           Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(&transfer, &wheelEvent);
    QVERIFY(transfer.property("antTransferPanelRegionUpdateCount").toInt() > panelUpdatesBeforeScroll);
    QCOMPARE(transfer.property("antTransferLastUpdateMode").toString(), QStringLiteral("scroll"));

    const int panelUpdatesBeforeTransfer = transfer.property("antTransferPanelRegionUpdateCount").toInt();
    QTest::mouseClick(&transfer, Qt::LeftButton, Qt::NoModifier, QPoint(200, 86));
    QVERIFY(transfer.property("antTransferPanelRegionUpdateCount").toInt() > panelUpdatesBeforeTransfer);
    QCOMPARE(transfer.property("antTransferLastUpdateMode").toString(), QStringLiteral("transfer"));
    QVERIFY(!transfer.sourceItems().contains(QStringLiteral("A")));
    QVERIFY(transfer.targetItems().contains(QStringLiteral("A")));
}

void TestAntDataEntryB::treeSelectReusesPopupTreeAndCachesRows()
{
    AntTreeSelect treeSelect;
    const QVector<AntTreeNode> children = {
        treeSelectNode(QStringLiteral("child-a"), QStringLiteral("Child A")),
        treeSelectNode(QStringLiteral("child-b"), QStringLiteral("Child B"))
    };
    treeSelect.setTreeData({treeSelectNode(QStringLiteral("root"), QStringLiteral("Root"), children)});
    treeSelect.resize(treeSelect.sizeHint());
    treeSelect.show();
    QVERIFY(QTest::qWaitForWindowExposed(&treeSelect));

    treeSelect.grab();
    const int triggerBuildsBefore = treeSelect.property("antTreeSelectTriggerLayoutBuildCount").toInt();
    const int triggerHitsBefore = treeSelect.property("antTreeSelectTriggerLayoutCacheHitCount").toInt();
    treeSelect.grab();
    QCOMPARE(treeSelect.property("antTreeSelectTriggerLayoutBuildCount").toInt(), triggerBuildsBefore);
    QVERIFY(treeSelect.property("antTreeSelectTriggerLayoutCacheHitCount").toInt() > triggerHitsBefore);

    treeSelect.setValue({QStringLiteral("root")});
    QCOMPARE(treeSelect.displayText(), QStringLiteral("Root"));
    QCOMPARE(treeSelect.property("antTreeSelectTitleCacheBuildCount").toInt(), 1);
    treeSelect.setValue({QStringLiteral("child-a")});
    QCOMPARE(treeSelect.displayText(), QStringLiteral("Child A"));
    QCOMPARE(treeSelect.property("antTreeSelectTitleCacheBuildCount").toInt(), 1);

    treeSelect.setOpen(true);
    QTRY_VERIFY(treeSelect.isOpen());
    QFrame* popup = nullptr;
    QTRY_VERIFY((popup = directVisibleFrameChild(&treeSelect)) != nullptr);
    auto* popupTree = popup->findChild<AntTree*>();
    QVERIFY(popupTree != nullptr);
    QCOMPARE(treeSelect.property("antTreeSelectPopupTreeRebuildCount").toInt(), 1);
    QCOMPARE(treeSelect.property("antTreeSelectVisibleRowCount").toInt(), 1);

    const int visibleRowHitsBeforeReopen = treeSelect.property("antTreeSelectVisibleRowsCacheHitCount").toInt();
    const int geometrySkipsBeforeReopen = treeSelect.property("antTreeSelectPopupGeometrySkipCount").toInt();
    treeSelect.setOpen(false);
    QTRY_VERIFY(!treeSelect.isOpen());
    QTRY_VERIFY(!popup->isVisible());

    treeSelect.setOpen(true);
    QTRY_VERIFY(treeSelect.isOpen());
    QCOMPARE(treeSelect.property("antTreeSelectPopupTreeRebuildCount").toInt(), 1);
    QVERIFY(treeSelect.property("antTreeSelectVisibleRowsCacheHitCount").toInt() > visibleRowHitsBeforeReopen);
    QVERIFY(treeSelect.property("antTreeSelectPopupGeometrySkipCount").toInt() > geometrySkipsBeforeReopen);

    ThemeModeRestorerForDataEntryBTest restoreTreeTheme;
    const int treeThemeGeometryUpdates = treeSelect.property("antThemeRefreshUpdateGeometryCount").toInt();
    antTheme->setThemeMode(restoreTreeTheme.alternateMode());
    QCoreApplication::processEvents();
    QCOMPARE(treeSelect.property("antThemeRefreshSizeHintChanged").toBool(), false);
    QCOMPARE(treeSelect.property("antThemeRefreshUpdateGeometryCount").toInt(), treeThemeGeometryUpdates);
    QCOMPARE(treeSelect.property("antTreeSelectLastUpdateMode").toString(), QStringLiteral("theme"));

    const int rebuildsBeforeCheckable = treeSelect.property("antTreeSelectPopupTreeRebuildCount").toInt();
    treeSelect.setTreeCheckable(true);
    QCOMPARE(treeSelect.property("antTreeSelectPopupTreeRebuildCount").toInt(), rebuildsBeforeCheckable);

    popup = directVisibleFrameChild(&treeSelect);
    QVERIFY(popup != nullptr);
    popupTree = popup->findChild<AntTree*>();
    QVERIFY(popupTree != nullptr);
    QTest::mouseClick(popupTree, Qt::LeftButton, Qt::NoModifier, QPoint(12, 14));
    QTRY_COMPARE(treeSelect.property("antTreeSelectLastPopupUpdateMode").toString(), QStringLiteral("expand"));
    QCOMPARE(treeSelect.property("antTreeSelectVisibleRowCount").toInt(), 3);
}

void TestAntDataEntryB::uploadCachesLayoutThumbsAndScopesUpdates()
{
    AntUpload upload;
    AntUploadFile file;
    file.uid = QStringLiteral("uploading");
    file.name = QStringLiteral("report.txt");
    file.status = Ant::UploadFileStatus::Uploading;
    file.percent = 10;
    upload.addFile(file);
    upload.resize(260, upload.sizeHint().height());
    upload.show();
    QVERIFY(QTest::qWaitForWindowExposed(&upload));

    upload.grab();
    const int layoutBuildsBefore = upload.property("antUploadLayoutBuildCount").toInt();
    const int layoutHitsBefore = upload.property("antUploadLayoutCacheHitCount").toInt();
    upload.grab();
    QCOMPARE(upload.property("antUploadLayoutBuildCount").toInt(), layoutBuildsBefore);
    QVERIFY(upload.property("antUploadLayoutCacheHitCount").toInt() > layoutHitsBefore);

    const int itemUpdatesBeforeHover = upload.property("antUploadItemRegionUpdateCount").toInt();
    sendMouseMove(&upload, QPoint(24, 54));
    QVERIFY(upload.property("antUploadItemRegionUpdateCount").toInt() > itemUpdatesBeforeHover);
    QCOMPARE(upload.property("antUploadLastUpdateMode").toString(), QStringLiteral("hover"));

    const int progressUpdatesBefore = upload.property("antUploadProgressRegionUpdateCount").toInt();
    upload.updateFileStatus(QStringLiteral("uploading"), Ant::UploadFileStatus::Uploading, 65);
    QVERIFY(upload.property("antUploadProgressRegionUpdateCount").toInt() > progressUpdatesBefore);
    QCOMPARE(upload.property("antUploadLastUpdateMode").toString(), QStringLiteral("status"));

    QTemporaryDir thumbnailDirectory;
    QVERIFY(thumbnailDirectory.isValid());
    const QString thumbnailPath = thumbnailDirectory.filePath(QStringLiteral("thumbnail.png"));
    QImage thumbnail(QSize(96, 72), QImage::Format_ARGB32_Premultiplied);
    thumbnail.fill(QColor(QStringLiteral("#1677ff")));
    QVERIFY(thumbnail.save(thumbnailPath));

    AntUpload pictureUpload;
    pictureUpload.setListType(Ant::UploadListType::Picture);
    AntUploadFile imageFile;
    imageFile.uid = QStringLiteral("image");
    imageFile.name = QStringLiteral("thumbnail.png");
    imageFile.status = Ant::UploadFileStatus::Done;
    imageFile.percent = 100;
    imageFile.thumbUrl = thumbnailPath;
    pictureUpload.addFile(imageFile);
    pictureUpload.resize(280, pictureUpload.sizeHint().height());
    pictureUpload.show();
    QVERIFY(QTest::qWaitForWindowExposed(&pictureUpload));

    pictureUpload.grab();
    QVERIFY(pictureUpload.property("antUploadThumbPixmapBuildCount").toInt() > 0);
    const int thumbHitsBefore = pictureUpload.property("antUploadThumbPixmapCacheHitCount").toInt();
    pictureUpload.grab();
    QVERIFY(pictureUpload.property("antUploadThumbPixmapCacheHitCount").toInt() > thumbHitsBefore);
}

void TestAntDataEntryB::uploadBoundsThumbnailDecodeAndEvictsLru()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    auto writeImage = [&directory](const QString& name, const QSize& size, const QColor& color) {
        const QString path = directory.filePath(name);
        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        image.fill(color);
        return image.save(path) ? path : QString();
    };
    auto uploadFile = [](const QString& uid, const QString& path) {
        AntUploadFile file;
        file.uid = uid;
        file.name = QFileInfo(path).fileName();
        file.status = Ant::UploadFileStatus::Done;
        file.percent = 100;
        file.url = path;
        file.thumbUrl = path;
        file.size = QFileInfo(path).size();
        return file;
    };

    const QString firstPath = writeImage(QStringLiteral("first.png"), QSize(512, 512), QColor(QStringLiteral("#1677ff")));
    const QString secondPath = writeImage(QStringLiteral("second.png"), QSize(512, 512), QColor(QStringLiteral("#52c41a")));
    const QString thirdPath = writeImage(QStringLiteral("third.png"), QSize(512, 512), QColor(QStringLiteral("#faad14")));
    QVERIFY(!firstPath.isEmpty());
    QVERIFY(!secondPath.isEmpty());
    QVERIFY(!thirdPath.isEmpty());

    AntUpload boundedCache;
    boundedCache.setListType(Ant::UploadListType::Picture);
    QSignalSpy budgetSpy(&boundedCache, &AntUpload::thumbnailCacheBudgetBytesChanged);
    const qreal boundedDpr = qBound<qreal>(1.0, boundedCache.devicePixelRatioF(), 4.0);
    const qint64 physicalThumbEdge = qCeil(40.0 * boundedDpr);
    const int twoThumbnailBudget = static_cast<int>(physicalThumbEdge * physicalThumbEdge * 4 * 2);
    boundedCache.setThumbnailCacheBudgetBytes(twoThumbnailBudget);
    QCOMPARE(boundedCache.thumbnailCacheBudgetBytes(), twoThumbnailBudget);
    QCOMPARE(budgetSpy.count(), 1);
    boundedCache.addFile(uploadFile(QStringLiteral("first"), firstPath));
    boundedCache.addFile(uploadFile(QStringLiteral("second"), secondPath));
    boundedCache.resize(280, boundedCache.sizeHint().height());
    boundedCache.grab();

    QVERIFY(boundedCache.property("antUploadThumbPixmapBuildCount").toInt() >= 2);
    QVERIFY(boundedCache.thumbnailCacheBytes() > 0);
    QVERIFY(boundedCache.thumbnailCacheBytes() <= boundedCache.thumbnailCacheBudgetBytes());
    QVERIFY(boundedCache.property("antUploadThumbPixmapCacheEntryCount").toInt() <= 2);

    const int hitsBefore = boundedCache.property("antUploadThumbPixmapCacheHitCount").toInt();
    boundedCache.grab();
    QVERIFY(boundedCache.property("antUploadThumbPixmapCacheHitCount").toInt() > hitsBefore);

    boundedCache.addFile(uploadFile(QStringLiteral("third"), thirdPath));
    boundedCache.resize(280, boundedCache.sizeHint().height());
    boundedCache.grab();
    QVERIFY(boundedCache.property("antUploadThumbPixmapBuildCount").toInt() >= 3);
    QVERIFY(boundedCache.property("antUploadThumbPixmapEvictionCount").toInt() > 0);
    QVERIFY(boundedCache.property("antUploadThumbPixmapCacheEntryCount").toInt() <= 2);
    QVERIFY(boundedCache.thumbnailCacheBytes() <= boundedCache.thumbnailCacheBudgetBytes());

    AntUpload lruProof;
    constexpr int directThumbEdge = 40;
    lruProof.setThumbnailCacheBudgetBytes(directThumbEdge * directThumbEdge * 4 * 2);
    const QSize directThumbSize(directThumbEdge, directThumbEdge);
    QVERIFY(!lruProof.cachedThumbPixmap(firstPath, directThumbSize, 1.0).isNull());
    QVERIFY(!lruProof.cachedThumbPixmap(secondPath, directThumbSize, 1.0).isNull());
    QCOMPARE(lruProof.m_thumbPixmapCache.size(), 2);

    const auto containsCachedSource = [&lruProof](const QString& path) {
        QString normalized = QFileInfo(path).canonicalFilePath();
        if (normalized.isEmpty())
            normalized = QFileInfo(path).absoluteFilePath();
        normalized = QDir::cleanPath(normalized);
        for (auto it = lruProof.m_thumbPixmapCache.cbegin();
             it != lruProof.m_thumbPixmapCache.cend(); ++it)
        {
            if (it->normalizedPath == normalized)
                return true;
        }
        return false;
    };
    QVERIFY(containsCachedSource(firstPath));
    QVERIFY(containsCachedSource(secondPath));

    const int directHitsBeforeTouch = lruProof.m_thumbPixmapCacheHitCount;
    QVERIFY(!lruProof.cachedThumbPixmap(firstPath, directThumbSize, 1.0).isNull());
    QCOMPARE(lruProof.m_thumbPixmapCacheHitCount, directHitsBeforeTouch + 1);
    QVERIFY(!lruProof.cachedThumbPixmap(thirdPath, directThumbSize, 1.0).isNull());
    QCOMPARE(lruProof.m_thumbPixmapCache.size(), 2);
    QVERIFY(containsCachedSource(firstPath));
    QVERIFY(!containsCachedSource(secondPath));
    QVERIFY(containsCachedSource(thirdPath));

    AntUpload cacheInvalidation;
    cacheInvalidation.setListType(Ant::UploadListType::Picture);
    cacheInvalidation.addFile(uploadFile(QStringLiteral("removable"), firstPath));
    cacheInvalidation.resize(280, cacheInvalidation.sizeHint().height());
    cacheInvalidation.grab();
    QCOMPARE(cacheInvalidation.property("antUploadThumbPixmapCacheEntryCount").toInt(), 1);
    cacheInvalidation.removeFile(QStringLiteral("removable"));
    QCOMPARE(cacheInvalidation.thumbnailCacheBytes(), qint64(0));
    QCOMPARE(cacheInvalidation.property("antUploadThumbPixmapCacheEntryCount").toInt(), 0);

    cacheInvalidation.addFile(uploadFile(QStringLiteral("replaceable"), firstPath));
    cacheInvalidation.grab();
    QCOMPARE(cacheInvalidation.property("antUploadThumbPixmapCacheEntryCount").toInt(), 1);
    cacheInvalidation.setFileList(QVector<AntUploadFile>{uploadFile(QStringLiteral("replacement"), secondPath)});
    QCOMPARE(cacheInvalidation.thumbnailCacheBytes(), qint64(0));
    QCOMPARE(cacheInvalidation.property("antUploadThumbPixmapCacheEntryCount").toInt(), 0);

    const QString mutablePath = writeImage(QStringLiteral("mutable.png"), QSize(96, 72), QColor(QStringLiteral("#722ed1")));
    QVERIFY(!mutablePath.isEmpty());
    AntUpload sourceChange;
    sourceChange.setListType(Ant::UploadListType::Picture);
    sourceChange.addFile(uploadFile(QStringLiteral("mutable"), mutablePath));
    sourceChange.resize(280, sourceChange.sizeHint().height());
    sourceChange.grab();
    const int buildsBeforeSourceChange = sourceChange.property("antUploadThumbPixmapBuildCount").toInt();
    QCOMPARE(sourceChange.property("antUploadThumbPixmapCacheEntryCount").toInt(), 1);

    QTest::qWait(20);
    QImage replacement(QSize(31, 27), QImage::Format_ARGB32_Premultiplied);
    replacement.fill(QColor(QStringLiteral("#eb2f96")));
    QVERIFY(replacement.save(mutablePath));
    sourceChange.grab();
    QVERIFY(sourceChange.property("antUploadThumbPixmapBuildCount").toInt() > buildsBeforeSourceChange);
    QVERIFY(sourceChange.property("antUploadThumbPixmapSourceChangeCount").toInt() > 0);
    QVERIFY(sourceChange.thumbnailCacheBytes() <= sourceChange.thumbnailCacheBudgetBytes());

    const QString sameStampPath = writeImage(QStringLiteral("same-stamp.bmp"),
                                             QSize(48, 48),
                                             QColor(QStringLiteral("#1677ff")));
    QVERIFY(!sameStampPath.isEmpty());
    const QDateTime originalModified = QFileInfo(sameStampPath).lastModified();
    const qint64 originalBytes = QFileInfo(sameStampPath).size();
    AntUpload explicitInvalidation;
    explicitInvalidation.setListType(Ant::UploadListType::Picture);
    explicitInvalidation.addFile(uploadFile(QStringLiteral("same-stamp"), sameStampPath));
    explicitInvalidation.resize(280, explicitInvalidation.sizeHint().height());
    explicitInvalidation.grab();
    const int buildsBeforeSameStampRewrite =
        explicitInvalidation.property("antUploadThumbPixmapBuildCount").toInt();

    QImage sameSizeReplacement(QSize(48, 48), QImage::Format_ARGB32_Premultiplied);
    sameSizeReplacement.fill(QColor(QStringLiteral("#52c41a")));
    QVERIFY(sameSizeReplacement.save(sameStampPath, "BMP"));
    QCOMPARE(QFileInfo(sameStampPath).size(), originalBytes);
    QFile sameStampFile(sameStampPath);
    QVERIFY(sameStampFile.open(QIODevice::ReadWrite));
    QVERIFY(sameStampFile.setFileTime(originalModified, QFileDevice::FileModificationTime));
    sameStampFile.close();
    QCOMPARE(QFileInfo(sameStampPath).lastModified().toMSecsSinceEpoch(),
             originalModified.toMSecsSinceEpoch());

    explicitInvalidation.grab();
    QCOMPARE(explicitInvalidation.property("antUploadThumbPixmapBuildCount").toInt(),
             buildsBeforeSameStampRewrite);
    explicitInvalidation.invalidateThumbnail(sameStampPath);
    QCOMPARE(explicitInvalidation.thumbnailCacheBytes(), qint64(0));
    explicitInvalidation.grab();
    QVERIFY(explicitInvalidation.property("antUploadThumbPixmapBuildCount").toInt()
            > buildsBeforeSameStampRewrite);

    const QString oversizedPath = directory.filePath(QStringLiteral("oversized.ppm"));
    QFile oversized(oversizedPath);
    QVERIFY(oversized.open(QIODevice::WriteOnly));
    QCOMPARE(oversized.write("P6\n40000 40000\n255\n"), qint64(19));
    oversized.close();

    AntUpload rejected;
    rejected.setListType(Ant::UploadListType::Picture);
    rejected.addFile(uploadFile(QStringLiteral("oversized"), oversizedPath));
    rejected.resize(280, rejected.sizeHint().height());
    QSignalSpy failureSpy(&rejected, &AntUpload::thumbnailLoadFailed);
    QElapsedTimer timer;
    timer.start();
    rejected.grab();
    QVERIFY2(timer.elapsed() < 1000, "oversized thumbnail must be rejected from metadata before decoding");
    QTRY_COMPARE(failureSpy.count(), 1);
    QVERIFY2(rejected.thumbnailError(oversizedPath).contains(QStringLiteral("dimensions")),
             qPrintable(rejected.thumbnailError(oversizedPath)));
    QCOMPARE(rejected.thumbnailCacheBytes(), qint64(0));
    QCOMPARE(rejected.property("antUploadThumbPixmapCacheEntryCount").toInt(), 0);
    QCOMPARE(rejected.property("antUploadThumbPixmapFailureCount").toInt(), 1);

    rejected.grab();
    QCoreApplication::processEvents();
    QCOMPARE(failureSpy.count(), 1);
    QCOMPARE(rejected.property("antUploadThumbPixmapFailureCount").toInt(), 1);

    const QString rapidFailurePath = directory.filePath(QStringLiteral("rapid-failure.ppm"));
    QFile rapidFailureFile(rapidFailurePath);
    QVERIFY(rapidFailureFile.open(QIODevice::WriteOnly));
    QCOMPARE(rapidFailureFile.write("P6\n40000 40000\n255\n"), qint64(19));
    rapidFailureFile.close();

    AntUpload rapidFailure;
    rapidFailure.setListType(Ant::UploadListType::Picture);
    rapidFailure.addFile(uploadFile(QStringLiteral("rapid-failure"), rapidFailurePath));
    rapidFailure.resize(280, rapidFailure.sizeHint().height());
    QSignalSpy rapidFailureSpy(&rapidFailure, &AntUpload::thumbnailLoadFailed);
    rapidFailure.grab();
    QCOMPARE(rapidFailureSpy.count(), 0);

    QVERIFY(rapidFailureFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(rapidFailureFile.write("not-an-image"), qint64(12));
    rapidFailureFile.close();
    rapidFailure.invalidateThumbnail(rapidFailurePath);
    rapidFailure.grab();
    QCOMPARE(rapidFailureSpy.count(), 0);
    QTRY_COMPARE(rapidFailureSpy.count(), 1);
    QCOMPARE(rapidFailureSpy.first().at(1).toString(),
             rapidFailure.thumbnailError(rapidFailurePath));
    QCoreApplication::processEvents();
    QCOMPARE(rapidFailureSpy.count(), 1);
}

void TestAntDataEntryB::uploadPreviewUsesExplicitUrlPolicy()
{
    AntUrlPolicy::reset();
    struct PolicyReset
    {
        ~PolicyReset() { AntUrlPolicy::reset(); }
    } reset;

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString localPath = directory.filePath(QStringLiteral("preview.txt"));
    QFile localFile(localPath);
    QVERIFY(localFile.open(QIODevice::WriteOnly));
    QCOMPARE(localFile.write("preview"), qint64(7));
    localFile.close();

    AntUpload upload;
    AntUploadFile local;
    local.uid = QStringLiteral("local");
    local.url = localPath;
    upload.addFile(local);

    QSignalSpy localSpy(&upload, &AntUpload::localFilePreviewRequested);
    QSignalSpy blockedSpy(&upload, &AntUpload::externalPreviewBlocked);
    QVERIFY(upload.requestFilePreview(local.uid));
    QCOMPARE(localSpy.count(), 1);
    QCOMPARE(QDir::cleanPath(localSpy.first().at(0).toString()), QDir::cleanPath(localPath));
    QCOMPARE(blockedSpy.count(), 0);

    AntUploadFile fileUrl;
    fileUrl.uid = QStringLiteral("file-url");
    fileUrl.url = QUrl::fromLocalFile(localPath).toString();
    upload.addFile(fileUrl);
    QVERIFY(upload.requestFilePreview(fileUrl.uid));
    QCOMPARE(localSpy.count(), 2);
    QCOMPARE(blockedSpy.count(), 0);

    AntUploadFile qrc;
    qrc.uid = QStringLiteral("qrc");
    qrc.url = QStringLiteral("qrc:/qt-ant-design/images/image-basic.png");
    upload.addFile(qrc);
    QVERIFY(upload.requestFilePreview(qrc.uid));
    QCOMPARE(localSpy.count(), 3);
    QCOMPARE(localSpy.last().at(0).toString(), QStringLiteral(":/qt-ant-design/images/image-basic.png"));

    AntUploadFile script;
    script.uid = QStringLiteral("script");
    script.url = QStringLiteral("javascript:alert(1)");
    upload.addFile(script);
    QVERIFY(!upload.requestFilePreview(script.uid));
    QCOMPARE(blockedSpy.count(), 1);
    QCOMPARE(blockedSpy.first().at(0).toUrl().scheme(), QStringLiteral("javascript"));

    bool approvalCalled = false;
    AntUrlPolicy::setApprovalCallback([&approvalCalled](const QUrl&) {
        approvalCalled = true;
        return false;
    });
    AntUploadFile custom;
    custom.uid = QStringLiteral("custom");
    custom.url = QStringLiteral("custom:preview");
    upload.addFile(custom);
    QVERIFY(!upload.requestFilePreview(custom.uid));
    QVERIFY(approvalCalled);
    QCOMPARE(blockedSpy.count(), 2);

    QVERIFY(!upload.requestFilePreview(QStringLiteral("missing")));

    QPointer<AntUpload> deletedByPolicy = new AntUpload;
    AntUploadFile deletingFile;
    deletingFile.uid = QStringLiteral("delete-in-policy");
    deletingFile.url = QStringLiteral("custom:delete-upload");
    deletedByPolicy->addFile(deletingFile);
    AntUrlPolicy::setApprovalCallback([&deletedByPolicy](const QUrl&) {
        delete deletedByPolicy.data();
        return false;
    });
    AntUpload* deletingRaw = deletedByPolicy.data();
    QVERIFY(!deletingRaw->requestFilePreview(deletingFile.uid));
    QVERIFY(deletedByPolicy.isNull());
}

QTEST_MAIN(TestAntDataEntryB)
#include "TestAntDataEntryB.moc"
