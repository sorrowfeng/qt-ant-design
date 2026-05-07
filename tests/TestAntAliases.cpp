#include <QTest>
#include <type_traits>

#include "widgets/AntCalendarWidget.h"
#include "widgets/AntComboBox.h"
#include "widgets/AntDateEdit.h"
#include "widgets/AntDialog.h"
#include "widgets/AntDoubleSpinBox.h"
#include "widgets/AntLabel.h"
#include "widgets/AntLineEdit.h"
#include "widgets/AntListView.h"
#include "widgets/AntListWidget.h"
#include "widgets/AntMainWindow.h"
#include "widgets/AntProgressBar.h"
#include "widgets/AntPushButton.h"
#include "widgets/AntRadioButton.h"
#include "widgets/AntSpinBox.h"
#include "widgets/AntTabWidget.h"
#include "widgets/AntTableView.h"
#include "widgets/AntTableWidget.h"
#include "widgets/AntTimeEdit.h"
#include "widgets/AntTreeView.h"
#include "widgets/AntTreeWidget.h"

#include "widgets/AntButton.h"
#include "widgets/AntCalendar.h"
#include "widgets/AntCheckBox.h"
#include "widgets/AntDatePicker.h"
#include "widgets/AntInput.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntList.h"
#include "widgets/AntModal.h"
#include "widgets/AntProgress.h"
#include "widgets/AntRadio.h"
#include "widgets/AntSelect.h"
#include "widgets/AntTable.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTimePicker.h"
#include "widgets/AntToolTip.h"
#include "widgets/AntTree.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWindow.h"

class TestAntAliases : public QObject
{
    Q_OBJECT

private slots:
    void qtStyleAliasesMapToCanonicalWidgets();
};

void TestAntAliases::qtStyleAliasesMapToCanonicalWidgets()
{
    QVERIFY((std::is_same_v<AntCalendarWidget, AntCalendar>));
    QVERIFY((std::is_same_v<AntComboBox, AntSelect>));
    QVERIFY((std::is_same_v<AntDateEdit, AntDatePicker>));
    QVERIFY((std::is_same_v<AntDialog, AntModal>));
    QVERIFY((std::is_same_v<AntDoubleSpinBox, AntInputNumber>));
    QVERIFY((std::is_same_v<AntLabel, AntTypography>));
    QVERIFY((std::is_same_v<AntLineEdit, AntInput>));
    QVERIFY((std::is_same_v<AntListView, AntList>));
    QVERIFY((std::is_same_v<AntListWidget, AntList>));
    QVERIFY((std::is_same_v<AntMainWindow, AntWindow>));
    QVERIFY((std::is_same_v<AntProgressBar, AntProgress>));
    QVERIFY((std::is_same_v<AntPushButton, AntButton>));
    QVERIFY((std::is_same_v<AntRadioButton, AntRadio>));
    QVERIFY((std::is_same_v<AntSpinBox, AntInputNumber>));
    QVERIFY((std::is_same_v<AntTabWidget, AntTabs>));
    QVERIFY((std::is_same_v<AntTableView, AntTable>));
    QVERIFY((std::is_same_v<AntTableWidget, AntTable>));
    QVERIFY((std::is_same_v<AntTimeEdit, AntTimePicker>));
    QVERIFY((std::is_same_v<AntTreeView, AntTree>));
    QVERIFY((std::is_same_v<AntTreeWidget, AntTree>));

    AntLabel label(QStringLiteral("Alias"));
    QCOMPARE(label.text(), QStringLiteral("Alias"));

    AntPushButton button(QStringLiteral("Alias"));
    QCOMPARE(button.text(), QStringLiteral("Alias"));
}

QTEST_MAIN(TestAntAliases)
#include "TestAntAliases.moc"
