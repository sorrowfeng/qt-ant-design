#include <QImage>
#include <QPainter>
#include <QSignalSpy>
#include <QMouseEvent>
#include <QTest>
#include "widgets/AntTypography.h"

namespace
{

constexpr QRgb SentinelPixel = 0xffff00ff;

QRect renderedInkBounds(AntTypography& widget)
{
    widget.ensurePolished();
    QCoreApplication::sendPostedEvents(&widget, QEvent::Polish);
    QCoreApplication::processEvents();

    QImage image(widget.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(SentinelPixel);
    {
        QPainter painter(&image);
        widget.render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    }

    QRect bounds;
    for (int y = 0; y < image.height(); ++y)
    {
        const QRgb* row = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x)
        {
            if (row[x] != SentinelPixel)
            {
                bounds |= QRect(x, y, 1, 1);
            }
        }
    }
    return bounds;
}

} // namespace

class TestAntTypography : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void verticalAlignmentRendering();
    void copyInteractionState();
};

void TestAntTypography::propertiesAndSignals()
{
    auto* w = new AntTypography;
    QCOMPARE(w->text(), QString());
    QCOMPARE(w->type(), Ant::TypographyType::Default);
    QCOMPARE(w->isTitle(), false);
    QCOMPARE(w->isParagraph(), false);
    QCOMPARE(w->wordWrap(), false);
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
    QCOMPARE(w->alignment(), Qt::AlignLeft | Qt::AlignVCenter);

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
    w->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    QCOMPARE(w->alignment(), Qt::AlignHCenter | Qt::AlignBottom);
    QCOMPARE(alignmentSpy.count(), 1);

    w->setAlignment(Qt::AlignRight);
    QCOMPARE(w->alignment(), Qt::AlignRight | Qt::AlignVCenter);
    QCOMPARE(alignmentSpy.count(), 2);

    QSignalSpy wordWrapSpy(w, &AntTypography::paragraphChanged);
    w->setWordWrap(true);
    QCOMPARE(w->wordWrap(), true);
    QCOMPARE(w->isParagraph(), true);
    QCOMPARE(wordWrapSpy.count(), 1);

    w->clear();
    QCOMPARE(w->text(), QString());
}

void TestAntTypography::verticalAlignmentRendering()
{
    AntTypography centered(QStringLiteral("Centered"));
    centered.setAttribute(Qt::WA_NoSystemBackground);
    centered.resize(180, 80);
    QRect centeredInk = renderedInkBounds(centered);
    QVERIFY(centeredInk.isValid());
    QVERIFY(centeredInk.height() < centered.height() / 2);
    QVERIFY(centeredInk.center().y() > centered.height() / 3);
    QVERIFY(centeredInk.center().y() < centered.height() * 2 / 3);

    AntTypography bottom(QStringLiteral("Bottom"));
    bottom.setAttribute(Qt::WA_NoSystemBackground);
    bottom.setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    bottom.resize(180, 80);
    QRect bottomInk = renderedInkBounds(bottom);
    QVERIFY(bottomInk.isValid());
    QVERIFY(bottomInk.top() > centeredInk.top());
    QVERIFY(bottomInk.center().y() > bottom.height() * 2 / 3);
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
