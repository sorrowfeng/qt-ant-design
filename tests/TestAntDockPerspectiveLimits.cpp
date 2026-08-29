#include <QtTest>

#include <QDataStream>
#include <QRandomGenerator>
#include <QSignalSpy>

#include <utility>

#include "private/AntDockLayoutSerializer.h"
#include "private/AntDockPerspectiveRestorer.h"
#include "widgets/AntDockManager.h"
#include "widgets/AntDockWidget.h"

namespace
{
constexpr const char* kPerspectiveMagic = "AntDockManagerPerspective";
constexpr quint16 kPerspectiveVersion = 1;

void writeNodePrefix(QDataStream& stream,
                     AntDockInternal::DockLayoutNodeType type,
                     qint32 dockCount,
                     qint32 childCount)
{
    stream << static_cast<quint8>(type)
           << static_cast<qint32>(Qt::Horizontal)
           << static_cast<qint32>(0)
           << static_cast<qint32>(0)
           << dockCount;
    for (qint32 i = 0; i < dockCount; ++i)
    {
        stream << QStringLiteral("dock");
    }
    stream << childCount;
}

QByteArray perspectiveWithLeafDepth(int leafDepth)
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << QString::fromLatin1(kPerspectiveMagic) << kPerspectiveVersion;
    for (int depth = 0; depth < leafDepth; ++depth)
    {
        writeNodePrefix(stream, AntDockInternal::DockLayoutNodeType::Splitter, 0, 1);
    }
    writeNodePrefix(stream, AntDockInternal::DockLayoutNodeType::Area, 1, 0);
    stream << static_cast<qint32>(0);
    return state;
}

QByteArray perspectiveWithTooManyNodes()
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << QString::fromLatin1(kPerspectiveMagic) << kPerspectiveVersion;

    // 1 root + 64 branches * (63 splitters + 1 valid area) = 4097 nodes.
    // IDs stay unique and far below the independent global ID budget.
    constexpr int branchCount = 64;
    constexpr int splittersPerBranch = 63;
    writeNodePrefix(stream, AntDockInternal::DockLayoutNodeType::Splitter, 0, branchCount);
    for (int branch = 0; branch < branchCount; ++branch)
    {
        for (int depth = 0; depth < splittersPerBranch; ++depth)
        {
            writeNodePrefix(stream, AntDockInternal::DockLayoutNodeType::Splitter, 0, 1);
        }
        stream << static_cast<quint8>(AntDockInternal::DockLayoutNodeType::Area)
               << static_cast<qint32>(Qt::Horizontal)
               << static_cast<qint32>(0)
               << static_cast<qint32>(0)
               << static_cast<qint32>(1)
               << QStringLiteral("node-budget-%1").arg(branch)
               << static_cast<qint32>(0);
    }
    stream << static_cast<qint32>(0);
    return state;
}

QByteArray perspectiveWithTooManyDockIds()
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << QString::fromLatin1(kPerspectiveMagic) << kPerspectiveVersion;

    // Five structurally valid areas carry 4097 globally unique IDs while using
    // only six nodes, so rejection must come from the global ID budget.
    constexpr int idsPerFullArea = 1024;
    constexpr int areaCount = 5;
    writeNodePrefix(stream, AntDockInternal::DockLayoutNodeType::Splitter, 0, areaCount);
    int nextId = 0;
    for (int area = 0; area < areaCount; ++area)
    {
        const int idCount = area < 4 ? idsPerFullArea : 1;
        stream << static_cast<quint8>(AntDockInternal::DockLayoutNodeType::Area)
               << static_cast<qint32>(Qt::Horizontal)
               << static_cast<qint32>(0)
               << static_cast<qint32>(0)
               << static_cast<qint32>(idCount);
        for (int id = 0; id < idCount; ++id)
        {
            stream << QStringLiteral("dock-budget-%1").arg(nextId++);
        }
        stream << static_cast<qint32>(0);
    }
    stream << static_cast<qint32>(0);
    return state;
}

QByteArray perspectiveWithIdentifier(const QString& identifier)
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << QString::fromLatin1(kPerspectiveMagic) << kPerspectiveVersion;
    stream << static_cast<quint8>(AntDockInternal::DockLayoutNodeType::Area)
           << static_cast<qint32>(Qt::Horizontal)
           << static_cast<qint32>(0)
           << static_cast<qint32>(0)
           << static_cast<qint32>(1)
           << identifier
           << static_cast<qint32>(0)
           << static_cast<qint32>(0);
    return state;
}

QByteArray perspectiveWithDeclaredDockCount(qint32 dockCount)
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << QString::fromLatin1(kPerspectiveMagic) << kPerspectiveVersion
           << static_cast<quint8>(AntDockInternal::DockLayoutNodeType::Area)
           << static_cast<qint32>(Qt::Horizontal)
           << static_cast<qint32>(0)
           << static_cast<qint32>(0)
           << dockCount;
    return state;
}

QByteArray perspectiveWithFloatingSnapshots(qint32 count, const QString& dockId = QString())
{
    QByteArray state;
    QDataStream stream(&state, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_0);
    stream << QString::fromLatin1(kPerspectiveMagic) << kPerspectiveVersion;
    writeNodePrefix(stream, AntDockInternal::DockLayoutNodeType::Empty, 0, 0);
    stream << count;
    for (qint32 i = 0; i < count; ++i)
    {
        stream << dockId << QRect(10, 20, 320, 180) << true;
    }
    return state;
}

bool deserialize(const QByteArray& state)
{
    AntDockInternal::DockLayoutNode root;
    QList<AntDockInternal::FloatingDockSnapshot> floating;
    return AntDockInternal::deserializeDockPerspective(state, &root, &floating);
}

QByteArray perspectiveWithKnownAndUnknownBranches()
{
    AntDockInternal::DockLayoutNode knownArea;
    knownArea.type = AntDockInternal::DockLayoutNodeType::Area;
    knownArea.dockIds = QStringList{QStringLiteral("known")};

    AntDockInternal::DockLayoutNode unknownArea;
    unknownArea.type = AntDockInternal::DockLayoutNodeType::Area;
    unknownArea.dockIds = QStringList{QStringLiteral("missing-embedded")};

    AntDockInternal::DockLayoutNode root;
    root.type = AntDockInternal::DockLayoutNodeType::Splitter;
    root.orientation = Qt::Horizontal;
    root.sizes = QList<int>{300, 200};
    root.children = QList<AntDockInternal::DockLayoutNode>{knownArea, unknownArea};

    AntDockInternal::FloatingDockSnapshot missingFloating;
    missingFloating.dockId = QStringLiteral("missing-floating");
    missingFloating.geometry = QRect(20, 30, 320, 180);
    missingFloating.visible = true;

    return AntDockInternal::serializeDockPerspective(
        root,
        QList<AntDockInternal::FloatingDockSnapshot>{missingFloating});
}

QByteArray perspectiveWithOnlyUnknownDock()
{
    AntDockInternal::DockLayoutNode root;
    root.type = AntDockInternal::DockLayoutNodeType::Area;
    root.dockIds = QStringList{QStringLiteral("not-registered")};
    return AntDockInternal::serializeDockPerspective(root, {});
}
} // namespace

class TestAntDockPerspectiveLimits : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void enforcesDepthBudget()
    {
        QVERIFY(deserialize(perspectiveWithLeafDepth(AntDockInternal::DockPerspectiveMaxDepth)));
        QVERIFY(!deserialize(perspectiveWithLeafDepth(AntDockInternal::DockPerspectiveMaxDepth + 1)));
    }

    void enforcesGlobalNodeBudget()
    {
        QVERIFY(!deserialize(perspectiveWithTooManyNodes()));
    }

    void enforcesGlobalDockIdBudget()
    {
        QVERIFY(!deserialize(perspectiveWithTooManyDockIds()));
    }

    void enforcesStateAndIdentifierBudgets()
    {
        const QString maximumIdentifier(AntDockInternal::DockPerspectiveMaxIdentifierCharacters,
                                        QLatin1Char('x'));
        QVERIFY(deserialize(perspectiveWithIdentifier(maximumIdentifier)));
        QVERIFY(!deserialize(perspectiveWithIdentifier(maximumIdentifier + QLatin1Char('x'))));

        const QByteArray oversizedState(AntDockInternal::DockPerspectiveMaxStateBytes + 1, '\0');
        QVERIFY(!deserialize(oversizedState));
    }

    void rejectsMalformedStreams()
    {
        QByteArray truncated = perspectiveWithLeafDepth(1);
        truncated.chop(1);
        QVERIFY(!deserialize(truncated));

        QVERIFY(!deserialize(perspectiveWithDeclaredDockCount(-1)));
        QVERIFY(!deserialize(perspectiveWithDeclaredDockCount(1025)));
        QVERIFY(!deserialize(perspectiveWithFloatingSnapshots(
            AntDockInternal::DockPerspectiveMaxFloatingSnapshots + 1)));
        QVERIFY(!deserialize(perspectiveWithFloatingSnapshots(
            1,
            QString(AntDockInternal::DockPerspectiveMaxIdentifierCharacters + 1,
                    QLatin1Char('f')))));

        QByteArray trailingBytes = perspectiveWithLeafDepth(1);
        trailingBytes.append('\0');
        QVERIFY(!deserialize(trailingBytes));

        QByteArray duplicateIds;
        QDataStream duplicateStream(&duplicateIds, QIODevice::WriteOnly);
        duplicateStream.setVersion(QDataStream::Qt_5_0);
        duplicateStream << QString::fromLatin1(kPerspectiveMagic) << kPerspectiveVersion;
        writeNodePrefix(duplicateStream, AntDockInternal::DockLayoutNodeType::Area, 2, 0);
        duplicateStream << static_cast<qint32>(0);
        QVERIFY(!deserialize(duplicateIds));
    }

    void rejectsStructurallyInvalidCurrentFormatNodes()
    {
        using AntDockInternal::DockLayoutNode;
        using AntDockInternal::DockLayoutNodeType;

        DockLayoutNode validArea;
        validArea.type = DockLayoutNodeType::Area;
        validArea.dockIds = QStringList{QStringLiteral("known")};

        QList<DockLayoutNode> invalidNodes;

        DockLayoutNode emptyWithDock = DockLayoutNode{};
        emptyWithDock.dockIds = QStringList{QStringLiteral("known")};
        invalidNodes.append(emptyWithDock);

        DockLayoutNode areaWithSizes = validArea;
        areaWithSizes.sizes = QList<int>{100};
        invalidNodes.append(areaWithSizes);

        DockLayoutNode areaWithChild = validArea;
        areaWithChild.children = QList<DockLayoutNode>{validArea};
        invalidNodes.append(areaWithChild);

        DockLayoutNode areaWithInvalidIndex = validArea;
        areaWithInvalidIndex.currentIndex = 1;
        invalidNodes.append(areaWithInvalidIndex);

        DockLayoutNode splitterWithDock;
        splitterWithDock.type = DockLayoutNodeType::Splitter;
        splitterWithDock.dockIds = QStringList{QStringLiteral("known")};
        splitterWithDock.children = QList<DockLayoutNode>{validArea};
        invalidNodes.append(splitterWithDock);

        DockLayoutNode emptySplitter;
        emptySplitter.type = DockLayoutNodeType::Splitter;
        invalidNodes.append(emptySplitter);

        DockLayoutNode splitterWithBadSizes;
        splitterWithBadSizes.type = DockLayoutNodeType::Splitter;
        splitterWithBadSizes.sizes = QList<int>{100, 200};
        splitterWithBadSizes.children = QList<DockLayoutNode>{validArea};
        invalidNodes.append(splitterWithBadSizes);

        DockLayoutNode splitterWithNegativeSize;
        splitterWithNegativeSize.type = DockLayoutNodeType::Splitter;
        splitterWithNegativeSize.sizes = QList<int>{-1};
        splitterWithNegativeSize.children = QList<DockLayoutNode>{validArea};
        invalidNodes.append(splitterWithNegativeSize);

        AntDockManager manager;
        const QByteArray stable = perspectiveWithLeafDepth(1);
        QVERIFY(manager.setPerspectiveState(QStringLiteral("stable-structure"), stable));
        QSignalSpy savedSpy(&manager, &AntDockManager::perspectiveSaved);

        for (const DockLayoutNode& invalidNode : std::as_const(invalidNodes))
        {
            const QByteArray invalidState =
                AntDockInternal::serializeDockPerspective(invalidNode, {});
            QVERIFY(!deserialize(invalidState));
            QVERIFY(!manager.setPerspectiveState(QStringLiteral("stable-structure"), invalidState));
            QCOMPARE(manager.perspectiveState(QStringLiteral("stable-structure")), stable);
        }
        QCOMPARE(savedSpy.count(), 0);
    }

    void invalidReplacementPreservesSavedState()
    {
        AntDockManager manager;
        auto* leftDock = new AntDockWidget(QStringLiteral("Left"));
        leftDock->setObjectName(QStringLiteral("left"));
        leftDock->setWidget(new QWidget);
        auto* rightDock = new AntDockWidget(QStringLiteral("Right"));
        rightDock->setObjectName(QStringLiteral("right"));
        rightDock->setWidget(new QWidget);
        manager.addDockWidget(Qt::LeftDockWidgetArea, leftDock);
        manager.addDockWidget(Qt::RightDockWidgetArea, rightDock);

        const QList<AntDockWidget*> docksBefore = manager.dockWidgets();
        const Qt::DockWidgetArea leftAreaBefore = manager.dockWidgetArea(leftDock);
        const Qt::DockWidgetArea rightAreaBefore = manager.dockWidgetArea(rightDock);
        const int leftTabBefore = manager.dockWidgetTabIndex(leftDock);
        const int rightTabBefore = manager.dockWidgetTabIndex(rightDock);
        QSignalSpy savedSpy(&manager, &AntDockManager::perspectiveSaved);
        const QByteArray valid = perspectiveWithLeafDepth(1);
        const QByteArray invalid = perspectiveWithLeafDepth(AntDockInternal::DockPerspectiveMaxDepth + 1);

        QVERIFY(manager.setPerspectiveState(QStringLiteral("bounded"), valid));
        QCOMPARE(savedSpy.count(), 1);
        const QByteArray savedState = manager.perspectiveState(QStringLiteral("bounded"));

        QVERIFY(!manager.setPerspectiveState(QStringLiteral("bounded"), invalid));
        QCOMPARE(savedSpy.count(), 1);
        QCOMPARE(manager.perspectiveState(QStringLiteral("bounded")), savedState);
        QCOMPARE(manager.dockWidgets(), docksBefore);
        QCOMPARE(manager.dockWidgetArea(leftDock), leftAreaBefore);
        QCOMPARE(manager.dockWidgetArea(rightDock), rightAreaBefore);
        QCOMPARE(manager.dockWidgetTabIndex(leftDock), leftTabBefore);
        QCOMPARE(manager.dockWidgetTabIndex(rightDock), rightTabBefore);
    }

    void legacyRestoreFailsWithoutSuccessSignal()
    {
        AntDockManager manager;
        auto* dock = new AntDockWidget(QStringLiteral("Dock"));
        dock->setObjectName(QStringLiteral("dock"));
        dock->setWidget(new QWidget);
        manager.addDockWidget(Qt::LeftDockWidgetArea, dock);

        const Qt::DockWidgetArea areaBefore = manager.dockWidgetArea(dock);
        const int tabBefore = manager.dockWidgetTabIndex(dock);
        const QByteArray legacyState("AntDockManagerLayout\nlegacy-placeholder");
        QVERIFY(manager.setPerspectiveState(QStringLiteral("legacy"), legacyState));

        QSignalSpy restoredSpy(&manager, &AntDockManager::perspectiveRestored);
        QSignalSpy failedSpy(&manager, &AntDockManager::perspectiveRestoreFailed);
        QVERIFY(!manager.restorePerspective(QStringLiteral("legacy")));
        QCOMPARE(restoredSpy.count(), 0);
        QCOMPARE(failedSpy.count(), 1);
        QCOMPARE(failedSpy.first().at(0).toString(), QStringLiteral("legacy"));
        QCOMPARE(failedSpy.first().at(1).toString(), QStringLiteral("unsupported-legacy-format"));
        QCOMPARE(manager.property("antDockLastPerspectiveRestoreError").toString(),
                 QStringLiteral("unsupported-legacy-format"));
        QCOMPARE(manager.dockWidgetArea(dock), areaBefore);
        QCOMPARE(manager.dockWidgetTabIndex(dock), tabBefore);
    }

    void noMatchingRestoreIsRejectedAtomically()
    {
        AntDockManager manager;
        auto* dock = new AntDockWidget(QStringLiteral("Available"));
        dock->setObjectName(QStringLiteral("available"));
        dock->setWidget(new QWidget);
        manager.addDockWidget(Qt::LeftDockWidgetArea, dock);

        const Qt::DockWidgetArea areaBefore = manager.dockWidgetArea(dock);
        const int tabBefore = manager.dockWidgetTabIndex(dock);
        QVERIFY(manager.setPerspectiveState(QStringLiteral("foreign"), perspectiveWithOnlyUnknownDock()));

        QSignalSpy restoredSpy(&manager, &AntDockManager::perspectiveRestored);
        QSignalSpy failedSpy(&manager, &AntDockManager::perspectiveRestoreFailed);
        QSignalSpy layoutSpy(&manager, &AntDockManager::dockLayoutChanged);
        QVERIFY(!manager.restorePerspective(QStringLiteral("foreign")));
        QCOMPARE(restoredSpy.count(), 0);
        QCOMPARE(failedSpy.count(), 1);
        QCOMPARE(layoutSpy.count(), 0);
        QCOMPARE(failedSpy.first().at(1).toString(), QStringLiteral("no-matching-dock"));
        QCOMPARE(manager.property("antDockLastPerspectiveRestoreError").toString(),
                 QStringLiteral("no-matching-dock"));
        QCOMPARE(manager.dockWidgetArea(dock), areaBefore);
        QCOMPARE(manager.dockWidgetTabIndex(dock), tabBefore);
    }

    void duplicateRuntimeDockIdsAreRejectedAtomically()
    {
        AntDockInternal::DockLayoutNode root;
        root.type = AntDockInternal::DockLayoutNodeType::Area;
        root.dockIds = QStringList{QStringLiteral("duplicate")};
        const QByteArray state = AntDockInternal::serializeDockPerspective(root, {});

        AntDockManager manager;
        auto* first = new AntDockWidget(QStringLiteral("First"));
        first->setObjectName(QStringLiteral("duplicate"));
        first->setWidget(new QWidget);
        auto* second = new AntDockWidget(QStringLiteral("Second"));
        second->setObjectName(QStringLiteral("duplicate"));
        second->setWidget(new QWidget);
        manager.addDockWidget(Qt::LeftDockWidgetArea, first);
        manager.addDockWidget(Qt::RightDockWidgetArea, second);
        QVERIFY(manager.setPerspectiveState(QStringLiteral("duplicate-runtime"), state));

        const Qt::DockWidgetArea firstArea = manager.dockWidgetArea(first);
        const Qt::DockWidgetArea secondArea = manager.dockWidgetArea(second);
        const int firstTab = manager.dockWidgetTabIndex(first);
        const int secondTab = manager.dockWidgetTabIndex(second);
        QSignalSpy restoredSpy(&manager, &AntDockManager::perspectiveRestored);
        QSignalSpy failedSpy(&manager, &AntDockManager::perspectiveRestoreFailed);
        QSignalSpy layoutSpy(&manager, &AntDockManager::dockLayoutChanged);

        QVERIFY(!manager.restorePerspective(QStringLiteral("duplicate-runtime")));
        QCOMPARE(restoredSpy.count(), 0);
        QCOMPARE(failedSpy.count(), 1);
        QCOMPARE(layoutSpy.count(), 0);
        QCOMPARE(failedSpy.first().at(1).toString(), QStringLiteral("duplicate-runtime-dock-id"));
        QCOMPARE(manager.dockWidgetArea(first), firstArea);
        QCOMPARE(manager.dockWidgetArea(second), secondArea);
        QCOMPARE(manager.dockWidgetTabIndex(first), firstTab);
        QCOMPARE(manager.dockWidgetTabIndex(second), secondTab);
    }

    void unknownDockIdsAreSkippedAndRuntimeDocksFallBack()
    {
        AntDockManager manager;
        auto* known = new AntDockWidget(QStringLiteral("Known"));
        known->setObjectName(QStringLiteral("known"));
        known->setWidget(new QWidget);
        auto* runtimeOnly = new AntDockWidget(QStringLiteral("Runtime only"));
        runtimeOnly->setObjectName(QStringLiteral("runtime-only"));
        runtimeOnly->setWidget(new QWidget);
        manager.addDockWidget(Qt::LeftDockWidgetArea, known);
        manager.addDockWidget(Qt::RightDockWidgetArea, runtimeOnly);

        const QByteArray state = perspectiveWithKnownAndUnknownBranches();
        QVERIFY(manager.setPerspectiveState(QStringLiteral("partial"), state));

        const AntDockInternal::DockPerspectiveRestorePlan plan =
            AntDockInternal::DockPerspectiveRestorer::plan(state, manager.dockWidgets());
        QVERIFY(plan.isValid());
        QVERIFY(plan.embeddedDockIds.contains(QStringLiteral("known")));
        QVERIFY(plan.embeddedDockIds.contains(QStringLiteral("missing-embedded")));
        QVERIFY(plan.stateDockIds.contains(QStringLiteral("missing-floating")));
        QCOMPARE(plan.docksById.value(QStringLiteral("known")), known);

        QSignalSpy restoredSpy(&manager, &AntDockManager::perspectiveRestored);
        QSignalSpy failedSpy(&manager, &AntDockManager::perspectiveRestoreFailed);
        QSignalSpy layoutSpy(&manager, &AntDockManager::dockLayoutChanged);
        QVERIFY(manager.restorePerspective(QStringLiteral("partial")));
        QCOMPARE(restoredSpy.count(), 1);
        QCOMPARE(restoredSpy.first().at(0).toString(), QStringLiteral("partial"));
        QCOMPARE(failedSpy.count(), 0);
        QVERIFY(layoutSpy.count() >= 1);
        QCOMPARE(manager.dockWidgetArea(known), Qt::LeftDockWidgetArea);
        QVERIFY(manager.isDockWidgetFloating(runtimeOnly));
        QVERIFY(!manager.property("antDockLastPerspectiveRestoreError").isValid());
        QCOMPARE(manager.property("antDockLastRestoreAreaCount").toInt(), 2);
    }

    void arbitraryBytesAreRejectedAtomically()
    {
        AntDockManager manager;
        const QByteArray valid = perspectiveWithLeafDepth(1);
        QVERIFY(manager.setPerspectiveState(QStringLiteral("stable"), valid));
        const QByteArray saved = manager.perspectiveState(QStringLiteral("stable"));
        QSignalSpy savedSpy(&manager, &AntDockManager::perspectiveSaved);

        QRandomGenerator random(0x51a7c0deu);
        for (int iteration = 0; iteration < 512; ++iteration)
        {
            const int length = iteration % 257;
            QByteArray bytes(length, '\0');
            for (int i = 0; i < bytes.size(); ++i)
            {
                bytes[i] = static_cast<char>(random.generate() & 0xffu);
            }
            QVERIFY(!deserialize(bytes));
            QVERIFY(!manager.setPerspectiveState(QStringLiteral("stable"), bytes));
            QCOMPARE(manager.perspectiveState(QStringLiteral("stable")), saved);
        }
        QCOMPARE(savedSpy.count(), 0);
    }
};

QTEST_MAIN(TestAntDockPerspectiveLimits)
#include "TestAntDockPerspectiveLimits.moc"
