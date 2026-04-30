#include <QColor>
#include <QDate>
#include <QMetaEnum>
#include <QMetaProperty>
#include <QMetaType>
#include <QPoint>
#include <QRectF>
#include <QSize>
#include <QSignalSpy>
#include <QStringList>
#include <QTest>
#include <QTime>
#include <QVariant>
#include <QWidget>

#include <functional>
#include <memory>

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
#include "widgets/AntCheckbox.h"
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
#include "widgets/AntTooltip.h"
#include "widgets/AntTour.h"
#include "widgets/AntTransfer.h"
#include "widgets/AntTree.h"
#include "widgets/AntTreeSelect.h"
#include "widgets/AntTypography.h"
#include "widgets/AntUpload.h"
#include "widgets/AntWatermark.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"

class TestAntMetaProperties : public QObject
{
    Q_OBJECT

private slots:
    void everyControlHasReadableAndWritableMetaProperties();
};

namespace
{
struct ObjectCase
{
    const char* name;
    std::function<QObject*(QWidget*)> create;
};

QList<ObjectCase> objectCases()
{
    return {
        {"AntAffix", [](QWidget* parent) { return new AntAffix(parent); }},
        {"AntAlert", [](QWidget* parent) { return new AntAlert(parent); }},
        {"AntAnchor", [](QWidget* parent) { return new AntAnchor(parent); }},
        {"AntApp", [](QWidget* parent) { return new AntApp(parent, parent); }},
        {"AntAutoComplete", [](QWidget* parent) { return new AntAutoComplete(parent); }},
        {"AntAvatarGroup", [](QWidget* parent) { return new AntAvatarGroup(parent); }},
        {"AntAvatar", [](QWidget* parent) { return new AntAvatar(parent); }},
        {"AntBadge", [](QWidget* parent) { return new AntBadge(parent); }},
        {"AntBreadcrumb", [](QWidget* parent) { return new AntBreadcrumb(parent); }},
        {"AntButton", [](QWidget* parent) { return new AntButton(parent); }},
        {"AntCalendar", [](QWidget* parent) { return new AntCalendar(parent); }},
        {"AntCard", [](QWidget* parent) { return new AntCard(parent); }},
        {"AntCarousel", [](QWidget* parent) { return new AntCarousel(parent); }},
        {"AntCascader", [](QWidget* parent) { return new AntCascader(parent); }},
        {"AntCheckbox", [](QWidget* parent) { return new AntCheckbox(parent); }},
        {"AntCollapsePanel", [](QWidget* parent) { return new AntCollapsePanel(QStringLiteral("Panel"), parent); }},
        {"AntCollapse", [](QWidget* parent) { return new AntCollapse(parent); }},
        {"AntColorPicker", [](QWidget* parent) { return new AntColorPicker(parent); }},
        {"AntConfigProvider", [](QWidget* parent) { return new AntConfigProvider(parent); }},
        {"AntDatePicker", [](QWidget* parent) { return new AntDatePicker(parent); }},
        {"AntDescriptionsItem", [](QWidget* parent) { return new AntDescriptionsItem(parent); }},
        {"AntDescriptions", [](QWidget* parent) { return new AntDescriptions(parent); }},
        {"AntDivider", [](QWidget* parent) { return new AntDivider(parent); }},
        {"AntDockWidget", [](QWidget* parent) { return new AntDockWidget(parent); }},
        {"AntDrawer", [](QWidget* parent) { return new AntDrawer(parent); }},
        {"AntDropdown", [](QWidget* parent) { return new AntDropdown(parent); }},
        {"AntEmpty", [](QWidget* parent) { return new AntEmpty(parent); }},
        {"AntFlex", [](QWidget* parent) { return new AntFlex(parent); }},
        {"AntFloatButton", [](QWidget* parent) { return new AntFloatButton(parent); }},
        {"AntFormItem", [](QWidget* parent) { return new AntFormItem(parent); }},
        {"AntFormProvider", [](QWidget* parent) { return new AntFormProvider(parent); }},
        {"AntForm", [](QWidget* parent) { return new AntForm(parent); }},
        {"AntFormList", [](QWidget* parent) { return new AntFormList(parent); }},
        {"AntCol", [](QWidget* parent) { return new AntCol(12, parent); }},
        {"AntRow", [](QWidget* parent) { return new AntRow(parent); }},
        {"AntIcon", [](QWidget* parent) { return new AntIcon(parent); }},
        {"AntImage", [](QWidget* parent) { return new AntImage(parent); }},
        {"AntInput", [](QWidget* parent) { return new AntInput(parent); }},
        {"AntInputNumber", [](QWidget* parent) { return new AntInputNumber(parent); }},
        {"AntLayoutHeader", [](QWidget* parent) { return new AntLayoutHeader(parent); }},
        {"AntLayoutFooter", [](QWidget* parent) { return new AntLayoutFooter(parent); }},
        {"AntLayoutContent", [](QWidget* parent) { return new AntLayoutContent(parent); }},
        {"AntLayoutSider", [](QWidget* parent) { return new AntLayoutSider(parent); }},
        {"AntLayout", [](QWidget* parent) { return new AntLayout(parent); }},
        {"AntListItemMeta", [](QWidget* parent) { return new AntListItemMeta(parent); }},
        {"AntListItem", [](QWidget* parent) { return new AntListItem(parent); }},
        {"AntList", [](QWidget* parent) { return new AntList(parent); }},
        {"AntLog", [](QWidget* parent) { return new AntLog(parent); }},
        {"AntMasonry", [](QWidget* parent) { return new AntMasonry(parent); }},
        {"AntMentions", [](QWidget* parent) { return new AntMentions(parent); }},
        {"AntMenu", [](QWidget* parent) { return new AntMenu(parent); }},
        {"AntMenuBar", [](QWidget* parent) { return new AntMenuBar(parent); }},
        {"AntMessage", [](QWidget* parent) { return new AntMessage(parent); }},
        {"AntModal", [](QWidget* parent) { return new AntModal(parent); }},
        {"AntNavItem", [](QWidget* parent) { return new AntNavItem(QStringLiteral("Nav"), parent); }},
        {"AntNotification", [](QWidget* parent) { return new AntNotification(parent); }},
        {"AntPagination", [](QWidget* parent) { return new AntPagination(parent); }},
        {"AntPlainTextEdit", [](QWidget* parent) { return new AntPlainTextEdit(parent); }},
        {"AntPopconfirm", [](QWidget* parent) { return new AntPopconfirm(parent); }},
        {"AntPopover", [](QWidget* parent) { return new AntPopover(parent); }},
        {"AntProgress", [](QWidget* parent) { return new AntProgress(parent); }},
        {"AntQRCode", [](QWidget* parent) { return new AntQRCode(parent); }},
        {"AntRadio", [](QWidget* parent) { return new AntRadio(parent); }},
        {"AntRate", [](QWidget* parent) { return new AntRate(parent); }},
        {"AntResult", [](QWidget* parent) { return new AntResult(parent); }},
        {"AntScrollArea", [](QWidget* parent) { return new AntScrollArea(parent); }},
        {"AntScrollBar", [](QWidget* parent) { return new AntScrollBar(parent); }},
        {"AntSegmented", [](QWidget* parent) { return new AntSegmented(parent); }},
        {"AntSelect", [](QWidget* parent) { return new AntSelect(parent); }},
        {"AntSkeleton", [](QWidget* parent) { return new AntSkeleton(parent); }},
        {"AntSlider", [](QWidget* parent) { return new AntSlider(parent); }},
        {"AntSpace", [](QWidget* parent) { return new AntSpace(parent); }},
        {"AntSpin", [](QWidget* parent) { return new AntSpin(parent); }},
        {"AntSplitter", [](QWidget* parent) { return new AntSplitter(parent); }},
        {"AntStatistic", [](QWidget* parent) { return new AntStatistic(parent); }},
        {"AntStatusBar", [](QWidget* parent) { return new AntStatusBar(parent); }},
        {"AntSteps", [](QWidget* parent) { return new AntSteps(parent); }},
        {"AntSwitch", [](QWidget* parent) { return new AntSwitch(parent); }},
        {"AntTable", [](QWidget* parent) { return new AntTable(parent); }},
        {"AntTabs", [](QWidget* parent) { return new AntTabs(parent); }},
        {"AntTag", [](QWidget* parent) { return new AntTag(parent); }},
        {"AntTimeline", [](QWidget* parent) { return new AntTimeline(parent); }},
        {"AntTimePicker", [](QWidget* parent) { return new AntTimePicker(parent); }},
        {"AntToolBar", [](QWidget* parent) { return new AntToolBar(parent); }},
        {"AntToolButton", [](QWidget* parent) { return new AntToolButton(parent); }},
        {"AntTooltip", [](QWidget* parent) { return new AntTooltip(parent); }},
        {"AntTour", [](QWidget* parent) { return new AntTour(parent); }},
        {"AntTransfer", [](QWidget* parent) { return new AntTransfer(parent); }},
        {"AntTree", [](QWidget* parent) { return new AntTree(parent); }},
        {"AntTreeSelect", [](QWidget* parent) { return new AntTreeSelect(parent); }},
        {"AntTypography", [](QWidget* parent) { return new AntTypography(parent); }},
        {"AntUpload", [](QWidget* parent) { return new AntUpload(parent); }},
        {"AntWatermark", [](QWidget* parent) { return new AntWatermark(parent); }},
        {"AntWidget", [](QWidget* parent) { return new AntWidget(parent); }},
        {"AntWindow", [](QWidget* parent) { return new AntWindow(parent); }},
    };
}

bool shouldSkipWrite(const QByteArray& propertyName)
{
    return propertyName == "open" || propertyName == "isOpen";
}

QVariant enumTestValue(const QMetaProperty& property, const QVariant& current)
{
    const QMetaEnum metaEnum = property.enumerator();
    if (!metaEnum.isValid() || metaEnum.keyCount() == 0)
    {
        return {};
    }

    const int currentValue = current.toInt();
    int chosen = metaEnum.value(0);
    for (int i = 0; i < metaEnum.keyCount(); ++i)
    {
        const int candidate = metaEnum.value(i);
        if (candidate != currentValue)
        {
            chosen = candidate;
            break;
        }
    }
    return chosen;
}

QVariant representativeValue(const QMetaProperty& property, const QVariant& current)
{
    if (property.isEnumType())
    {
        return enumTestValue(property, current);
    }

    const QMetaType metaType = property.metaType();
    if (metaType == QMetaType::fromType<QString>())
        return QString::fromLatin1(property.name()) + QStringLiteral("-value");
    if (metaType == QMetaType::fromType<QStringList>())
        return current.toStringList() == QStringList{QStringLiteral("A"), QStringLiteral("B")}
            ? QStringList{QStringLiteral("C")}
            : QStringList{QStringLiteral("A"), QStringLiteral("B")};
    if (metaType == QMetaType::fromType<bool>())
        return !current.toBool();
    if (metaType == QMetaType::fromType<int>())
        return current.toInt() == 7 ? 8 : 7;
    if (metaType == QMetaType::fromType<double>() || QString::fromLatin1(property.typeName()) == QStringLiteral("qreal"))
        return qFuzzyCompare(current.toDouble(), 0.5) ? 0.75 : 0.5;
    if (metaType == QMetaType::fromType<QColor>())
        return current.value<QColor>() == QColor(Qt::red) ? QColor(Qt::blue) : QColor(Qt::red);
    if (metaType == QMetaType::fromType<QDate>())
        return current.toDate() == QDate(2026, 1, 2) ? QDate(2026, 1, 3) : QDate(2026, 1, 2);
    if (metaType == QMetaType::fromType<QPoint>())
        return current.toPoint() == QPoint(3, 4) ? QPoint(5, 6) : QPoint(3, 4);
    if (metaType == QMetaType::fromType<QRectF>())
        return current.toRectF() == QRectF(1.0, 2.0, 30.0, 4.0)
            ? QRectF(2.0, 3.0, 40.0, 5.0)
            : QRectF(1.0, 2.0, 30.0, 4.0);
    if (metaType == QMetaType::fromType<QSize>())
        return current.toSize() == QSize(48, 32) ? QSize(64, 40) : QSize(48, 32);
    if (metaType == QMetaType::fromType<QTime>())
        return current.toTime() == QTime(9, 30, 15) ? QTime(10, 45, 30) : QTime(9, 30, 15);
    if (metaType == QMetaType::fromType<QVariant>())
        return QVariant(QStringLiteral("meta-value"));

    const QString typeName = QString::fromLatin1(property.typeName());
    if (typeName == QStringLiteral("Qt::Alignment"))
        return current.value<Qt::Alignment>() == (Qt::AlignRight | Qt::AlignVCenter)
            ? QVariant::fromValue(Qt::AlignLeft | Qt::AlignVCenter)
            : QVariant::fromValue(Qt::AlignRight | Qt::AlignVCenter);
    if (typeName == QStringLiteral("Qt::CaseSensitivity"))
        return QVariant::fromValue(current.value<Qt::CaseSensitivity>() == Qt::CaseInsensitive
                                       ? Qt::CaseSensitive
                                       : Qt::CaseInsensitive);
    if (typeName == QStringLiteral("Qt::Orientation"))
        return QVariant::fromValue(current.value<Qt::Orientation>() == Qt::Vertical ? Qt::Horizontal : Qt::Vertical);
    if (typeName == QStringLiteral("QList<AntCascaderOption>") ||
        typeName == QStringLiteral("QVector<AntCascaderOption>"))
    {
        AntCascaderOption option;
        option.value = QStringLiteral("meta-option");
        option.label = QStringLiteral("Meta Option");
        return QVariant::fromValue(QVector<AntCascaderOption>{option});
    }

    return {};
}
} // namespace

void TestAntMetaProperties::everyControlHasReadableAndWritableMetaProperties()
{
    auto* root = new QWidget;
    QStringList unsupportedWritableProperties;
    int ownPropertyCount = 0;
    int writablePropertyCount = 0;

    for (const ObjectCase& objectCase : objectCases())
    {
        QObject* object = objectCase.create(root);
        QVERIFY(object != nullptr);

        const QMetaObject* metaObject = object->metaObject();
        for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
        {
            const QMetaProperty property = metaObject->property(i);
            ++ownPropertyCount;

            const QString qualifiedName = QString::fromLatin1("%1::%2")
                                              .arg(QString::fromLatin1(objectCase.name),
                                                   QString::fromLatin1(property.name()));
            QVERIFY2(property.isReadable(), qPrintable(qualifiedName + QStringLiteral(" is not readable")));

            QVariant current = property.read(object);
            const bool invalidQVariantIsAllowed = property.metaType() == QMetaType::fromType<QVariant>();
            QVERIFY2(current.isValid() || invalidQVariantIsAllowed,
                     qPrintable(qualifiedName + QStringLiteral(" did not return a valid value")));

            if (!property.isWritable() || shouldSkipWrite(property.name()))
            {
                continue;
            }

            ++writablePropertyCount;
            const QVariant value = representativeValue(property, current);
            if (!value.isValid())
            {
                unsupportedWritableProperties.append(qualifiedName + QStringLiteral(" [") +
                                                     QString::fromLatin1(property.typeName()) + QStringLiteral("]"));
                continue;
            }

            const QVariant beforeWrite = property.read(object);
            std::unique_ptr<QSignalSpy> notifySpy;
            if (property.hasNotifySignal())
            {
                notifySpy = std::make_unique<QSignalSpy>(object, property.notifySignal());
            }

            const bool wrote = property.write(object, value);
            QVERIFY2(wrote, qPrintable(qualifiedName + QStringLiteral(" rejected representative meta-property write")));

            const QVariant afterWrite = property.read(object);
            QVERIFY2(afterWrite.isValid(),
                     qPrintable(qualifiedName + QStringLiteral(" returned invalid value after write")));
            if (property.hasNotifySignal() && afterWrite != beforeWrite)
            {
                QVERIFY2(notifySpy->isValid(), qPrintable(qualifiedName + QStringLiteral(" has an invalid NOTIFY signal")));
                QVERIFY2(notifySpy->count() > 0,
                         qPrintable(qualifiedName + QStringLiteral(" changed but did not emit its NOTIFY signal")));
            }
        }
    }

    QCOMPARE(unsupportedWritableProperties, QStringList());
    QVERIFY(ownPropertyCount > 0);
    QVERIFY(writablePropertyCount > 0);

    delete root;
}

QTEST_MAIN(TestAntMetaProperties)
#include "TestAntMetaProperties.moc"
