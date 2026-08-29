#include <QtTest>

#include <QFileInfo>

#include "private/AntWindowsSystemLibrary.h"

class TestAntWindowsSystemLibrary : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void loadsOnlyFromSystem32()
    {
#ifdef Q_OS_WIN
        const auto module = AntWindowsSystemLibrary::loadSystem32Library(L"dwmapi.dll");
        QVERIFY(module != nullptr);
        QVERIFY(AntWindowsSystemLibrary::isLoadedFromSystem32(module));

        const QString modulePath = AntWindowsSystemLibrary::moduleFilePath(module);
        const QString systemPath = AntWindowsSystemLibrary::system32Directory();
        QVERIFY(!modulePath.isEmpty());
        QVERIFY(!systemPath.isEmpty());
        QCOMPARE(QFileInfo(modulePath).absolutePath().compare(systemPath, Qt::CaseInsensitive), 0);

        QVERIFY(AntWindowsSystemLibrary::loadSystem32Library(L"..\\dwmapi.dll") == nullptr);
        QVERIFY(AntWindowsSystemLibrary::loadSystem32Library(L"C:\\Windows\\System32\\dwmapi.dll") == nullptr);
#else
        QVERIFY(AntWindowsSystemLibrary::loadSystem32Library(L"dwmapi.dll") == nullptr);
#endif
    }
};

QTEST_MAIN(TestAntWindowsSystemLibrary)
#include "TestAntWindowsSystemLibrary.moc"
