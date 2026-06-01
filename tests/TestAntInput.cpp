#include <QCoreApplication>
#include <QImage>
#include <QLayout>
#include <QPainter>
#include <QPixmap>
#include <QSignalSpy>
#include <QTest>

#include "core/AntTheme.h"
#include "widgets/AntInput.h"

class TestAntInput : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void outlinedFrameStaysVisibleAroundLineEdit();
    void performanceCachesAndScopedUpdates();
};

namespace
{
QImage renderInput(AntInput* input)
{
    input->ensurePolished();
    input->layout()->activate();
    input->show();
    input->lineEdit()->setFocus(Qt::OtherFocusReason);
    QCoreApplication::processEvents();

    QImage image(input->size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    input->render(&painter, QPoint(), QRegion(), QWidget::DrawChildren);
    input->hide();
    return image;
}

bool closeToColor(const QColor& color, const QColor& target)
{
    return color.alpha() > 80 &&
           qAbs(color.red() - target.red()) <= 48 &&
           qAbs(color.green() - target.green()) <= 64 &&
           qAbs(color.blue() - target.blue()) <= 64;
}

int countPrimaryPixelsOnRows(const QImage& image, const QList<int>& rows)
{
    const QColor primary = antTheme->tokens().colorPrimary;
    int count = 0;
    for (const int y : rows)
    {
        if (y < 0 || y >= image.height())
        {
            continue;
        }
        for (int x = 28; x < image.width() - 28; ++x)
        {
            if (closeToColor(image.pixelColor(x, y), primary))
            {
                ++count;
            }
        }
    }
    return count;
}
} // namespace

void TestAntInput::propertiesAndSignals()
{
    auto* input = new AntInput;
    QCOMPARE(input->text(), QString());
    QCOMPARE(input->inputSize(), Ant::Size::Middle);
    QCOMPARE(input->status(), Ant::Status::Normal);
    QCOMPARE(input->variant(), Ant::Variant::Outlined);
    QCOMPARE(input->allowClear(), false);
    QCOMPARE(input->isPasswordMode(), false);
    QCOMPARE(input->isSearchMode(), false);
    QCOMPARE(input->placeholderText(), QString());
    QCOMPARE(input->isReadOnly(), false);
    QCOMPARE(input->maxLength(), 32767);
    QCOMPARE(input->echoMode(), QLineEdit::Normal);
    QCOMPARE(input->alignment(), Qt::AlignLeft | Qt::AlignVCenter);

    QSignalSpy textSpy(input, &AntInput::textChanged);
    input->setText("hello");
    QCOMPARE(input->text(), "hello");
    QCOMPARE(textSpy.count(), 1);

    input->setPlaceholderText("Enter text");
    QCOMPARE(input->placeholderText(), "Enter text");
    QCOMPARE(input->lineEdit()->placeholderText(), "Enter text");

    input->setReadOnly(true);
    QCOMPARE(input->isReadOnly(), true);
    QCOMPARE(input->lineEdit()->isReadOnly(), true);
    input->setReadOnly(false);

    input->setMaxLength(4);
    QCOMPARE(input->maxLength(), 4);
    input->setText("abcdef");
    QCOMPARE(input->text(), "abcd");

    input->setEchoMode(QLineEdit::Password);
    QCOMPARE(input->echoMode(), QLineEdit::Password);
    QCOMPARE(input->lineEdit()->echoMode(), QLineEdit::Password);

    input->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QCOMPARE(input->alignment(), Qt::AlignRight | Qt::AlignVCenter);
    QCOMPARE(input->lineEdit()->alignment(), Qt::AlignRight | Qt::AlignVCenter);

    input->setCursorPosition(2);
    QCOMPARE(input->cursorPosition(), 2);
    input->selectAll();
    QVERIFY(input->hasSelectedText());
    QCOMPARE(input->selectedText(), QStringLiteral("abcd"));
    input->deselect();
    QVERIFY(!input->hasSelectedText());
    input->clear();
    QCOMPARE(input->text(), QString());

    QSignalSpy sizeSpy(input, &AntInput::inputSizeChanged);
    input->setInputSize(Ant::Size::Large);
    QCOMPARE(input->inputSize(), Ant::Size::Large);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy statusSpy(input, &AntInput::statusChanged);
    input->setStatus(Ant::Status::Error);
    QCOMPARE(input->status(), Ant::Status::Error);
    QCOMPARE(statusSpy.count(), 1);

    input->setVariant(Ant::Variant::Filled);
    QCOMPARE(input->variant(), Ant::Variant::Filled);

    QSignalSpy clearSpy(input, &AntInput::allowClearChanged);
    input->setAllowClear(true);
    QCOMPARE(input->allowClear(), true);
    QCOMPARE(clearSpy.count(), 1);

    QSignalSpy passwordSpy(input, &AntInput::passwordModeChanged);
    input->setPasswordMode(true);
    QCOMPARE(input->isPasswordMode(), true);
    QCOMPARE(passwordSpy.count(), 1);

    QSignalSpy editedSpy(input, &AntInput::textEdited);
    QSignalSpy returnSpy(input, &AntInput::returnPressed);
    QTest::keyClicks(input->lineEdit(), "xy");
    QCOMPARE(input->text(), QStringLiteral("xy"));
    QCOMPARE(editedSpy.count(), 2);
    QTest::keyClick(input->lineEdit(), Qt::Key_Return);
    QCOMPARE(returnSpy.count(), 1);

    QSize hint = input->sizeHint();
    QVERIFY(hint.width() > 0);
    QVERIFY(hint.height() > 0);
}

void TestAntInput::outlinedFrameStaysVisibleAroundLineEdit()
{
    AntInput input;
    input.setPlaceholderText(QStringLiteral("File name"));
    input.resize(448, input.sizeHint().height());
    input.layout()->activate();

    QVERIFY(input.lineEdit()->geometry().top() > input.rect().top());
    QVERIFY(input.lineEdit()->geometry().bottom() < input.rect().bottom());

    const QImage image = renderInput(&input);
    QVERIFY(!image.isNull());

    const int topPrimary = countPrimaryPixelsOnRows(image, {0, 1, 2, 3});
    const int bottomPrimary = countPrimaryPixelsOnRows(image, {image.height() - 4,
                                                               image.height() - 3,
                                                               image.height() - 2,
                                                               image.height() - 1});
    QVERIFY2(topPrimary > image.width() / 2,
             qPrintable(QStringLiteral("top input border primary pixels too sparse: %1").arg(topPrimary)));
    QVERIFY2(bottomPrimary > image.width() / 2,
             qPrintable(QStringLiteral("bottom input border primary pixels too sparse: %1").arg(bottomPrimary)));
}

void TestAntInput::performanceCachesAndScopedUpdates()
{
    AntInput input;
    input.resize(260, input.sizeHint().height());

    const int sizeHintResolvesBefore = input.property("antInputSizeHintResolveCount").toInt();
    input.sizeHint();
    input.minimumSizeHint();
    input.sizeHint();
    QCOMPARE(input.property("antInputSizeHintResolveCount").toInt(), sizeHintResolvesBefore);

    const int metricsResolvesBefore = input.property("antInputMetricsResolveCount").toInt();
    input.sizeHint();
    input.minimumSizeHint();
    QCOMPARE(input.property("antInputMetricsResolveCount").toInt(), metricsResolvesBefore);

    const int rebuildsBeforeAddon = input.property("antInputLayoutRebuildCount").toInt();
    input.setAddonBefore(QStringLiteral("https://"));
    const int rebuildsAfterAddon = input.property("antInputLayoutRebuildCount").toInt();
    QVERIFY(rebuildsAfterAddon > rebuildsBeforeAddon);
    input.setAddonBefore(QStringLiteral("https://"));
    QCOMPARE(input.property("antInputLayoutRebuildCount").toInt(), rebuildsAfterAddon);

    QPixmap pixmap(16, 16);
    pixmap.fill(Qt::blue);
    const QIcon icon(pixmap);
    input.setPrefixIcon(icon);
    const int rebuildsAfterIcon = input.property("antInputLayoutRebuildCount").toInt();
    input.setPrefixIcon(icon);
    QCOMPARE(input.property("antInputLayoutRebuildCount").toInt(), rebuildsAfterIcon);

    input.setAllowClear(true);
    const int visibilityChangesBefore = input.property("antInputButtonVisibilityChangeCount").toInt();
    input.setText(QStringLiteral("abc"));
    QVERIFY(input.property("antInputButtonVisibilityChangeCount").toInt() > visibilityChangesBefore);
    const int visibilityChangesAfterText = input.property("antInputButtonVisibilityChangeCount").toInt();
    input.setText(QStringLiteral("abcd"));
    QCOMPARE(input.property("antInputButtonVisibilityChangeCount").toInt(), visibilityChangesAfterText);

    input.setSearchMode(true);
    input.layout()->activate();
    QCoreApplication::processEvents();
    QVERIFY(input.clearButtonRect().isValid());
    QVERIFY(input.searchButtonRect().isValid());

    const int scopedUpdatesBefore = input.property("antInputScopedUpdateCount").toInt();
    input.setStatus(Ant::Status::Error);
    QVERIFY(input.property("antInputScopedUpdateCount").toInt() > scopedUpdatesBefore);
}

QTEST_MAIN(TestAntInput)
#include "TestAntInput.moc"
