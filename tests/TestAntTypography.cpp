#include <QImage>
#include <QLabel>
#include <QClipboard>
#include <QLineEdit>
#include <QPainter>
#include <QPointer>
#include <QSignalSpy>
#include <QMouseEvent>
#include <QTest>
#include <QVBoxLayout>
#include "core/AntUrlPolicy.h"
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
    void pixelSizeShortcut();
    void enabledStateSync();
    void verticalAlignmentRendering();
    void singleLineMinimumWidthKeepsTextVisible();
    void wordWrapAdaptsToLayoutWidth();
    void sizePolicyCanBeCustomized();
    void copyInteractionState();
    void externalLinkPolicy();
    void synchronousDeletionDuringInteraction();
    void measurementAndCopyRectCaches();
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

    // editable：双击进入行内编辑，提交后发出 edited
    QCOMPARE(w->isEditable(), false);
    QSignalSpy editableSpy(w, &AntTypography::editableChanged);
    w->setEditable(true);
    QCOMPARE(w->isEditable(), true);
    QCOMPARE(editableSpy.count(), 1);
    QCOMPARE(w->isEditing(), false);

    QSignalSpy editedSpy(w, &AntTypography::edited);
    w->setText(QStringLiteral("Editable text"));
    w->startEditing();
    QVERIFY(w->isEditing());
    w->finishEditing(true);
    QCOMPARE(w->isEditing(), false);
    QCOMPARE(editedSpy.count(), 0);

    w->startEditing();
    QVERIFY(w->isEditing());
    if (auto* editor = w->findChild<QLineEdit*>())
    {
        editor->setText(QStringLiteral("Edited text"));
    }
    w->finishEditing(true);
    QCOMPARE(w->text(), QStringLiteral("Edited text"));
    QCOMPARE(editedSpy.count(), 1);

    // 取消不改动文本
    w->startEditing();
    if (auto* editor = w->findChild<QLineEdit*>())
    {
        editor->setText(QStringLiteral("Discarded"));
    }
    w->finishEditing(false);
    QCOMPARE(w->text(), QStringLiteral("Edited text"));
    QCOMPARE(editedSpy.count(), 1);

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

void TestAntTypography::pixelSizeShortcut()
{
    AntTypography label(QStringLiteral("Pixel text"));
    label.setAttribute(Qt::WA_NoSystemBackground);
    label.resize(220, 80);
    const int defaultHeight = renderedInkBounds(label).height();

    label.setPixelSize(28);
    QCOMPARE(label.pixelSize(), 28);
    QCOMPARE(label.font().pixelSize(), 28);
    const int largerHeight = renderedInkBounds(label).height();
    QVERIFY(largerHeight > defaultHeight);
}

void TestAntTypography::enabledStateSync()
{
    AntTypography label(QStringLiteral("Disabled text"));
    QSignalSpy disabledSpy(&label, &AntTypography::disabledChanged);

    label.setEnabled(false);
    QCOMPARE(label.isEnabled(), false);
    QCOMPARE(label.isDisabled(), true);
    QCOMPARE(label.cursor().shape(), Qt::ForbiddenCursor);
    QCOMPARE(disabledSpy.count(), 1);

    label.setEnabled(true);
    QCOMPARE(label.isEnabled(), true);
    QCOMPARE(label.isDisabled(), false);
    QCOMPARE(label.cursor().shape(), Qt::ArrowCursor);
    QCOMPARE(disabledSpy.count(), 2);

    label.setDisabled(true);
    QCOMPARE(label.isEnabled(), false);
    QCOMPARE(label.isDisabled(), true);
    QCOMPARE(disabledSpy.count(), 3);

    label.setDisabled(false);
    QCOMPARE(label.isEnabled(), true);
    QCOMPARE(label.isDisabled(), false);
    QCOMPARE(disabledSpy.count(), 4);
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

void TestAntTypography::singleLineMinimumWidthKeepsTextVisible()
{
    QWidget host;
    auto* layout = new QHBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* text = new AntTypography(QStringLiteral("A status label that should stay readable"), &host);
    auto* filler = new QWidget(&host);
    filler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(text);
    layout->addWidget(filler);

    const int textMinimumWidth = text->minimumSizeHint().width();
    QVERIFY(textMinimumWidth >= text->sizeHint().width());

    host.resize(textMinimumWidth + 80, text->sizeHint().height() + 8);
    host.show();
    QCoreApplication::processEvents();

    QVERIFY(text->width() >= textMinimumWidth);

    const int shortMinimumWidth = AntTypography(QStringLiteral("Short")).minimumSizeHint().width();
    text->setText(QStringLiteral("A much longer status label that asks the layout for more room"));
    QVERIFY(text->minimumSizeHint().width() > shortMinimumWidth);
}

void TestAntTypography::wordWrapAdaptsToLayoutWidth()
{
    QWidget host;
    auto* layout = new QVBoxLayout(&host);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* text = new AntTypography(QStringLiteral("A long typography paragraph should wrap cleanly when a Qt layout gives it a narrow width."));
    text->setWordWrap(true);
    layout->addWidget(text);
    layout->addStretch();

    host.resize(180, 220);
    host.show();
    QCoreApplication::processEvents();

    QVERIFY(text->hasHeightForWidth());
    QLabel nativeWrappedLabel;
    nativeWrappedLabel.setWordWrap(true);
    QCOMPARE(text->sizePolicy().horizontalPolicy(), nativeWrappedLabel.sizePolicy().horizontalPolicy());
    QVERIFY(text->minimumSizeHint().width() < text->sizeHint().width());
    QCOMPARE(text->width(), host.width());
    QVERIFY(text->heightForWidth(120) > text->heightForWidth(260));
    QVERIFY(text->height() >= text->heightForWidth(text->width()));
}

void TestAntTypography::sizePolicyCanBeCustomized()
{
    AntTypography label(QStringLiteral("Custom policy"));
    label.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    label.setWordWrap(true);

    QCOMPARE(label.sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    QCOMPARE(label.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
    QVERIFY(label.sizePolicy().hasHeightForWidth());
    QVERIFY(label.hasHeightForWidth());
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

void TestAntTypography::externalLinkPolicy()
{
    struct PolicyReset
    {
        ~PolicyReset() { AntUrlPolicy::reset(); }
    } reset;
    Q_UNUSED(reset);

    AntUrlPolicy::reset();
    QCOMPARE(AntUrlPolicy::allowedSchemes(),
             QStringList({QStringLiteral("http"), QStringLiteral("https")}));
    QVERIFY(AntUrlPolicy::isExternalUrlAllowed(QUrl(QStringLiteral("https://example.com/path"))));
    QVERIFY(!AntUrlPolicy::isExternalUrlAllowed(QUrl::fromLocalFile(QStringLiteral("C:/tmp/file.txt"))));
    QVERIFY(!AntUrlPolicy::isExternalUrlAllowed(QUrl(QStringLiteral("javascript:alert(1)"))));

    bool approvalCalled = false;
    AntUrlPolicy::setApprovalCallback([&approvalCalled](const QUrl& url) {
        approvalCalled = true;
        return url.scheme() == QStringLiteral("mailto");
    });
    QVERIFY(AntUrlPolicy::isExternalUrlAllowed(QUrl(QStringLiteral("mailto:test@example.com"))));
    QVERIFY(approvalCalled);
    QVERIFY(!AntUrlPolicy::isExternalUrlAllowed(QUrl(QStringLiteral("custom:blocked"))));

    AntUrlPolicy::setApprovalCallback({});
    AntTypography link(QStringLiteral("Local file"));
    link.setType(Ant::TypographyType::Link);
    link.setHref(QUrl::fromLocalFile(QStringLiteral("C:/tmp/private.txt")).toString());
    link.resize(160, 32);
    link.show();
    QSignalSpy activatedSpy(&link, &AntTypography::linkActivated);
    QSignalSpy blockedSpy(&link, &AntTypography::linkOpenBlocked);
    QTest::mouseClick(&link, Qt::LeftButton, Qt::NoModifier, link.rect().center());
    QCOMPARE(activatedSpy.count(), 1);
    QCOMPARE(blockedSpy.count(), 1);
}

void TestAntTypography::synchronousDeletionDuringInteraction()
{
    struct PolicyReset
    {
        ~PolicyReset() { AntUrlPolicy::reset(); }
    } reset;
    Q_UNUSED(reset);

    AntUrlPolicy::reset();
    bool approvalCalled = false;
    QPointer<AntTypography> deletedBySignal = new AntTypography(QStringLiteral("Delete on activation"));
    deletedBySignal->setType(Ant::TypographyType::Link);
    deletedBySignal->setHref(QStringLiteral("custom:signal-delete"));
    deletedBySignal->resize(180, 32);
    QObject::connect(deletedBySignal, &AntTypography::linkActivated, deletedBySignal,
                     [&deletedBySignal](const QString&) { delete deletedBySignal.data(); });
    AntUrlPolicy::setApprovalCallback([&approvalCalled](const QUrl&) {
        approvalCalled = true;
        return false;
    });
    QTest::mouseClick(deletedBySignal.data(), Qt::LeftButton, Qt::NoModifier,
                      deletedBySignal->rect().center());
    QVERIFY(deletedBySignal.isNull());
    QVERIFY(!approvalCalled);

    QPointer<AntTypography> deletedByPolicy = new AntTypography(QStringLiteral("Delete in policy"));
    deletedByPolicy->setType(Ant::TypographyType::Link);
    deletedByPolicy->setHref(QStringLiteral("custom:policy-delete"));
    deletedByPolicy->resize(180, 32);
    AntUrlPolicy::setApprovalCallback([&deletedByPolicy](const QUrl&) {
        delete deletedByPolicy.data();
        return false;
    });
    QTest::mouseClick(deletedByPolicy.data(), Qt::LeftButton, Qt::NoModifier,
                      deletedByPolicy->rect().center());
    QVERIFY(deletedByPolicy.isNull());

    QApplication::clipboard()->setText(QStringLiteral("clipboard-delete-baseline"));
    QPointer<AntTypography> deletedByClipboard =
        new AntTypography(QStringLiteral("clipboard-delete-trigger"));
    deletedByClipboard->setCopyable(true);
    deletedByClipboard->resize(deletedByClipboard->sizeHint());
    QObject::connect(QApplication::clipboard(), &QClipboard::dataChanged,
                     deletedByClipboard, [&deletedByClipboard]() {
        delete deletedByClipboard.data();
    });
    const QPoint clipboardCopyPoint(qMax(1, deletedByClipboard->width() - 8),
                                    qMax(1, deletedByClipboard->height() / 2));
    QTest::mouseClick(deletedByClipboard.data(), Qt::LeftButton, Qt::NoModifier,
                      clipboardCopyPoint);
    QTRY_VERIFY(deletedByClipboard.isNull());

    QPointer<AntTypography> deletedByCopy = new AntTypography(QStringLiteral("Delete on copy"));
    deletedByCopy->setCopyable(true);
    deletedByCopy->resize(deletedByCopy->sizeHint());
    QObject::connect(deletedByCopy, &AntTypography::copied, deletedByCopy,
                     [&deletedByCopy](const QString&) { delete deletedByCopy.data(); });
    const QPoint copyPoint(qMax(1, deletedByCopy->width() - 8),
                           qMax(1, deletedByCopy->height() / 2));
    QTest::mouseClick(deletedByCopy.data(), Qt::LeftButton, Qt::NoModifier, copyPoint);
    QVERIFY(deletedByCopy.isNull());
}

void TestAntTypography::measurementAndCopyRectCaches()
{
    AntTypography paragraph(QStringLiteral("A long typography paragraph should reuse the same measured height when the layout asks repeatedly."));
    paragraph.setWordWrap(true);

    const int initialMeasureHits = paragraph.property("antTypographyMeasuredSizeCacheHitCount").toInt();
    const int initialMeasureMisses = paragraph.property("antTypographyMeasuredSizeCacheMissCount").toInt();
    const int firstHeight = paragraph.heightForWidth(180);
    QVERIFY(firstHeight > 0);
    QVERIFY(paragraph.property("antTypographyMeasuredSizeCacheMissCount").toInt() > initialMeasureMisses);

    QCOMPARE(paragraph.heightForWidth(180), firstHeight);
    QVERIFY(paragraph.property("antTypographyMeasuredSizeCacheHitCount").toInt() > initialMeasureHits);

    const int missesBeforeTextChange = paragraph.property("antTypographyMeasuredSizeCacheMissCount").toInt();
    paragraph.setText(QStringLiteral("Changing the paragraph text should invalidate the cached measurement and calculate a fresh height."));
    QVERIFY(paragraph.heightForWidth(180) > 0);
    QVERIFY(paragraph.property("antTypographyMeasuredSizeCacheMissCount").toInt() > missesBeforeTextChange);

    AntTypography copyable(QStringLiteral("Copy cache"));
    copyable.setCopyable(true);
    copyable.resize(copyable.sizeHint());
    const QPoint copyPoint(qMax(1, copyable.width() - 8), qMax(1, copyable.height() / 2));

    const int initialCopyHits = copyable.property("antTypographyCopyRectCacheHitCount").toInt();
    QMouseEvent firstMove(QEvent::MouseMove, QPointF(copyPoint), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&copyable, &firstMove);
    QMouseEvent secondMove(QEvent::MouseMove, QPointF(copyPoint), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(&copyable, &secondMove);
    QVERIFY(copyable.property("antTypographyCopyRectCacheHitCount").toInt() > initialCopyHits);
}

QTEST_MAIN(TestAntTypography)
#include "TestAntTypography.moc"
