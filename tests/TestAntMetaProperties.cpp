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

#include "WidgetInventory.h"

#include <functional>
#include <memory>


class TestAntMetaProperties : public QObject
{
    Q_OBJECT

private slots:
    void everyControlHasReadableAndWritableMetaProperties();
};

namespace
{
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

    const int metaTypeId = property.userType();
    if (metaTypeId == qMetaTypeId<QString>())
        return QString::fromLatin1(property.name()) + QStringLiteral("-value");
    if (metaTypeId == qMetaTypeId<QStringList>())
        return current.toStringList() == QStringList{QStringLiteral("A"), QStringLiteral("B")}
            ? QStringList{QStringLiteral("C")}
            : QStringList{QStringLiteral("A"), QStringLiteral("B")};
    if (metaTypeId == qMetaTypeId<bool>())
        return !current.toBool();
    if (metaTypeId == qMetaTypeId<int>())
        return current.toInt() == 7 ? 8 : 7;
    if (metaTypeId == qMetaTypeId<double>() || QString::fromLatin1(property.typeName()) == QStringLiteral("qreal"))
        return qFuzzyCompare(current.toDouble(), 0.5) ? 0.75 : 0.5;
    if (metaTypeId == qMetaTypeId<QColor>())
        return current.value<QColor>() == QColor(Qt::red) ? QColor(Qt::blue) : QColor(Qt::red);
    if (metaTypeId == qMetaTypeId<QDate>())
        return current.toDate() == QDate(2026, 1, 2) ? QDate(2026, 1, 3) : QDate(2026, 1, 2);
    if (metaTypeId == qMetaTypeId<QPoint>())
        return current.toPoint() == QPoint(3, 4) ? QPoint(5, 6) : QPoint(3, 4);
    if (metaTypeId == qMetaTypeId<QRectF>())
        return current.toRectF() == QRectF(1.0, 2.0, 30.0, 4.0)
            ? QRectF(2.0, 3.0, 40.0, 5.0)
            : QRectF(1.0, 2.0, 30.0, 4.0);
    if (metaTypeId == qMetaTypeId<QSize>())
        return current.toSize() == QSize(48, 32) ? QSize(64, 40) : QSize(48, 32);
    if (metaTypeId == qMetaTypeId<QTime>())
        return current.toTime() == QTime(9, 30, 15) ? QTime(10, 45, 30) : QTime(9, 30, 15);
    if (metaTypeId == qMetaTypeId<QVariant>())
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

    for (const AntTestUtils::WidgetFactoryCase& objectCase : AntTestUtils::allWidgetFactoryCases())
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
            const bool invalidQVariantIsAllowed = property.userType() == qMetaTypeId<QVariant>();
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
