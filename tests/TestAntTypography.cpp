#include <QSignalSpy>
#include <QMouseEvent>
#include <QTest>
#include "widgets/AntTypography.h"

class TestAntTypography : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void copyInteractionState();
};

void TestAntTypography::propertiesAndSignals()
{
    auto* w = new AntTypography;
    QCOMPARE(w->text(), QString());
    QCOMPARE(w->type(), Ant::TypographyType::Default);
    QCOMPARE(w->isTitle(), false);
    QCOMPARE(w->isParagraph(), false);
    QCOMPARE(w->isDisabled(), false);
    QCOMPARE(w->isStrong(), false);
    QCOMPARE(w->isUnderline(), false);
    QCOMPARE(w->isDelete(), false);
    QCOMPARE(w->isCode(), false);
    QCOMPARE(w->isMark(), false);
    QCOMPARE(w->isItalic(), false);
    QCOMPARE(w->isCopyable(), false);
    QCOMPARE(w->isEllipsis(), false);
    QCOMPARE(w->href(), QString());
    QCOMPARE(w->alignment(), Qt::AlignLeft);

    QSignalSpy textSpy(w, &AntTypography::textChanged);
    w->setText("Hello");
    QCOMPARE(w->text(), "Hello");
    QCOMPARE(textSpy.count(), 1);

    QSignalSpy typeSpy(w, &AntTypography::typeChanged);
    w->setType(Ant::TypographyType::Secondary);
    QCOMPARE(w->type(), Ant::TypographyType::Secondary);
    QCOMPARE(typeSpy.count(), 1);

    w->setType(Ant::TypographyType::Link);
    QCOMPARE(w->type(), Ant::TypographyType::Link);
    QCOMPARE(w->cursor().shape(), Qt::PointingHandCursor);

    w->setType(Ant::TypographyType::Default);
    QCOMPARE(w->cursor().shape(), Qt::ArrowCursor);

    QSignalSpy titleSpy(w, &AntTypography::titleChanged);
    w->setTitle(true);
    QCOMPARE(w->isTitle(), true);
    QCOMPARE(titleSpy.count(), 1);

    QSignalSpy levelSpy(w, &AntTypography::titleLevelChanged);
    w->setTitleLevel(Ant::TypographyTitleLevel::H2);
    QCOMPARE(w->titleLevel(), Ant::TypographyTitleLevel::H2);
    QCOMPARE(levelSpy.count(), 1);

    QSignalSpy strongSpy(w, &AntTypography::strongChanged);
    w->setStrong(true);
    QCOMPARE(w->isStrong(), true);
    QCOMPARE(strongSpy.count(), 1);

    QSignalSpy underlineSpy(w, &AntTypography::underlineChanged);
    w->setUnderline(true);
    QCOMPARE(w->isUnderline(), true);
    QCOMPARE(underlineSpy.count(), 1);

    QSignalSpy deleteSpy(w, &AntTypography::deleteChanged);
    w->setDelete(true);
    QCOMPARE(w->isDelete(), true);
    QCOMPARE(deleteSpy.count(), 1);

    QSignalSpy codeSpy(w, &AntTypography::codeChanged);
    w->setCode(true);
    QCOMPARE(w->isCode(), true);
    QCOMPARE(codeSpy.count(), 1);

    QSignalSpy markSpy(w, &AntTypography::markChanged);
    w->setMark(true);
    QCOMPARE(w->isMark(), true);
    QCOMPARE(markSpy.count(), 1);

    QSignalSpy italicSpy(w, &AntTypography::italicChanged);
    w->setItalic(true);
    QCOMPARE(w->isItalic(), true);
    QCOMPARE(italicSpy.count(), 1);

    QSignalSpy copyableSpy(w, &AntTypography::copyableChanged);
    w->setCopyable(true);
    QCOMPARE(w->isCopyable(), true);
    QCOMPARE(copyableSpy.count(), 1);

    QSignalSpy ellipsisSpy(w, &AntTypography::ellipsisChanged);
    w->setEllipsis(true);
    QCOMPARE(w->isEllipsis(), true);
    QCOMPARE(ellipsisSpy.count(), 1);

    QSignalSpy hrefSpy(w, &AntTypography::hrefChanged);
    w->setHref("https://example.com");
    QCOMPARE(w->href(), "https://example.com");
    QCOMPARE(hrefSpy.count(), 1);

    QSignalSpy alignmentSpy(w, &AntTypography::alignmentChanged);
    w->setAlignment(Qt::AlignHCenter);
    QCOMPARE(w->alignment(), Qt::AlignHCenter);
    QCOMPARE(alignmentSpy.count(), 1);
}

void TestAntTypography::copyInteractionState()
{
    AntTypography w("Copyable Text");
    w.setCopyable(true);
    w.resize(w.sizeHint());

    QSignalSpy copiedSpy(&w, &AntTypography::copied);
    const QPoint copyPoint(qMax(1, w.width() - 8), qMax(1, w.height() / 2));

    QMouseEvent move(QEvent::MouseMove, QPointF(copyPoint), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &move);
    QVERIFY(w.isCopyHovered());

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(copyPoint), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &press);
    QVERIFY(w.isCopyPressed());

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(copyPoint), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&w, &release);
    QCOMPARE(copiedSpy.count(), 1);
    QVERIFY(!w.isCopyPressed());
    QVERIFY(w.isCopied());
}

QTEST_MAIN(TestAntTypography)
#include "TestAntTypography.moc"
