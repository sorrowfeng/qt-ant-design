#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QTest>

#ifndef ANT_WIDGETS_DIR
#error "ANT_WIDGETS_DIR must be defined by tests/CMakeLists.txt"
#endif

#ifndef ANT_TESTS_DIR
#error "ANT_TESTS_DIR must be defined by tests/CMakeLists.txt"
#endif

class TestAntCoverageInventory : public QObject
{
    Q_OBJECT

private slots:
    void publicWidgetHeadersAreInCoverageTests();
};

namespace
{
QString readTextFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

QSet<QString> publicWidgetHeaders()
{
    QSet<QString> headers;
    const QDir widgetsDir(QStringLiteral(ANT_WIDGETS_DIR));
    const QFileInfoList files = widgetsDir.entryInfoList({QStringLiteral("Ant*.h")}, QDir::Files, QDir::Name);
    for (const QFileInfo& file : files)
    {
        const QString baseName = file.completeBaseName();
        if (baseName == QStringLiteral("AntSelectPopup"))
        {
            continue;
        }
        headers.insert(baseName);
    }
    return headers;
}

QSet<QString> includedWidgetHeaders(const QString& fileName)
{
    const QString text = readTextFile(QStringLiteral(ANT_TESTS_DIR) + QLatin1Char('/') + fileName);
    QSet<QString> headers;

    QRegularExpression includePattern(QStringLiteral("[#]include\\s+\"widgets/(Ant[^\"]+)\\.h\""));
    QRegularExpressionMatchIterator it = includePattern.globalMatch(text);
    while (it.hasNext())
    {
        headers.insert(it.next().captured(1));
    }
    return headers;
}
} // namespace

void TestAntCoverageInventory::publicWidgetHeadersAreInCoverageTests()
{
    const QSet<QString> publicHeaders = publicWidgetHeaders();
    QVERIFY(!publicHeaders.isEmpty());

    const QSet<QString> objectTreeHeaders = includedWidgetHeaders(QStringLiteral("TestAntObjectTree.cpp"));
    const QSet<QString> metaPropertyHeaders = includedWidgetHeaders(QStringLiteral("TestAntMetaProperties.cpp"));
    const QSet<QString> themeLifecycleHeaders = includedWidgetHeaders(QStringLiteral("TestAntThemeLifecycle.cpp"));
    const QSet<QString> renderSmokeHeaders = includedWidgetHeaders(QStringLiteral("TestAntRenderSmoke.cpp"));

    const QStringList missingObjectTree = (publicHeaders - objectTreeHeaders).values();
    const QStringList missingMetaProperties = (publicHeaders - metaPropertyHeaders).values();
    const QStringList missingThemeLifecycle = (publicHeaders - themeLifecycleHeaders).values();
    const QStringList missingRenderSmoke = (publicHeaders - renderSmokeHeaders).values();

    QCOMPARE(missingObjectTree, QStringList());
    QCOMPARE(missingMetaProperties, QStringList());
    QCOMPARE(missingThemeLifecycle, QStringList());
    QCOMPARE(missingRenderSmoke, QStringList());
}

QTEST_MAIN(TestAntCoverageInventory)
#include "TestAntCoverageInventory.moc"
