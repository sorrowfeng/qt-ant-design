#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFontInfo>
#include <QFontMetrics>
#include <QHeaderView>
#include <QImage>
#include <QLineEdit>
#include <QMap>
#include <QPainter>
#include <QPushButton>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionFrame>
#include <QStyleOptionHeader>
#include <QStyleOptionSlider>
#include <QStyleOptionTab>
#include <QTabBar>
#include <QTableWidget>
#include <QTest>
#include <QTextStream>
#include <QWidget>

#include <cmath>
#include <functional>

#include "core/AntDesign.h"
#include "core/AntTheme.h"
#include "widgets/AntButton.h"
#include "widgets/AntCheckBox.h"
#include "widgets/AntInput.h"
#include "widgets/AntList.h"
#include "widgets/AntBadge.h"
#include "widgets/AntMenu.h"
#include "widgets/AntPagination.h"
#include "widgets/AntProgress.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntSelect.h"
#include "widgets/AntTabs.h"
#include "widgets/AntTable.h"
#include "widgets/AntTag.h"
#include "widgets/AntToolTip.h"
#include "widgets/AntTree.h"

class TestAntQtVersionMetricAudit : public QObject
{
    Q_OBJECT

private slots:
    void stylePaletteFontAndGeometryMetricsAreAuditable();
};

namespace
{
struct MetricRecord
{
    QString group;
    QString item;
    QString metric;
    QString kind;
    QString value;
    qreal tolerance = 0.0;
    QString note;
};

struct ComparisonRecord
{
    MetricRecord actual;
    QString baselineValue;
    QString delta;
    bool passed = true;
};

QString sanitizeTsv(QString text)
{
    text.replace(QLatin1Char('\t'), QLatin1Char(' '));
    text.replace(QLatin1Char('\r'), QLatin1Char(' '));
    text.replace(QLatin1Char('\n'), QLatin1Char(' '));
    return text;
}

QString metricKey(const MetricRecord& record)
{
    return record.group + QLatin1Char('\x1f') + record.item + QLatin1Char('\x1f') + record.metric;
}

QString colorValue(const QColor& color)
{
    return QStringLiteral("#%1%2%3%4")
        .arg(color.alpha(), 2, 16, QLatin1Char('0'))
        .arg(color.red(), 2, 16, QLatin1Char('0'))
        .arg(color.green(), 2, 16, QLatin1Char('0'))
        .arg(color.blue(), 2, 16, QLatin1Char('0'))
        .toUpper();
}

bool parseColorValue(const QString& value, QColor* color)
{
    if (!color || value.size() != 9 || !value.startsWith(QLatin1Char('#')))
    {
        return false;
    }

    bool ok = false;
    const int alpha = value.mid(1, 2).toInt(&ok, 16);
    if (!ok)
    {
        return false;
    }
    const int red = value.mid(3, 2).toInt(&ok, 16);
    if (!ok)
    {
        return false;
    }
    const int green = value.mid(5, 2).toInt(&ok, 16);
    if (!ok)
    {
        return false;
    }
    const int blue = value.mid(7, 2).toInt(&ok, 16);
    if (!ok)
    {
        return false;
    }

    *color = QColor(red, green, blue, alpha);
    return true;
}

int colorMaxChannelDelta(const QColor& a, const QColor& b)
{
    return qMax(qMax(qAbs(a.red() - b.red()), qAbs(a.green() - b.green())),
                qMax(qAbs(a.blue() - b.blue()), qAbs(a.alpha() - b.alpha())));
}

quint64 appendHashValue(quint64 hash, quint32 value)
{
    hash ^= value;
    hash *= 1099511628211ULL;
    return hash;
}

QString imageHashValue(const QImage& image)
{
    const QImage argb = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    quint64 hash = 1469598103934665603ULL;
    hash = appendHashValue(hash, static_cast<quint32>(argb.width()));
    hash = appendHashValue(hash, static_cast<quint32>(argb.height()));
    for (int y = 0; y < argb.height(); ++y)
    {
        const auto* line = reinterpret_cast<const QRgb*>(argb.constScanLine(y));
        for (int x = 0; x < argb.width(); ++x)
        {
            hash = appendHashValue(hash, line[x]);
        }
    }
    return QStringLiteral("0x%1").arg(hash, 16, 16, QLatin1Char('0')).toUpper();
}

void addRecord(QVector<MetricRecord>& records,
               const QString& group,
               const QString& item,
               const QString& metric,
               const QString& kind,
               const QString& value,
               qreal tolerance,
               const QString& note)
{
    records.push_back({group, item, metric, kind, sanitizeTsv(value), tolerance, sanitizeTsv(note)});
}

void addNumber(QVector<MetricRecord>& records,
               const QString& group,
               const QString& item,
               const QString& metric,
               qreal value,
               qreal tolerance,
               const QString& note)
{
    addRecord(records,
              group,
              item,
              metric,
              QStringLiteral("number"),
              QString::number(value, 'f', std::floor(value) == value ? 0 : 3),
              tolerance,
              note);
}

void addText(QVector<MetricRecord>& records,
             const QString& group,
             const QString& item,
             const QString& metric,
             const QString& value,
             qreal tolerance,
             const QString& note)
{
    addRecord(records, group, item, metric, QStringLiteral("text"), value, tolerance, note);
}

void addColor(QVector<MetricRecord>& records,
              const QString& group,
              const QString& item,
              const QString& metric,
              const QColor& color,
              qreal tolerance,
              const QString& note)
{
    addRecord(records, group, item, metric, QStringLiteral("color"), colorValue(color), tolerance, note);
}

void addSize(QVector<MetricRecord>& records,
             const QString& group,
             const QString& item,
             const QString& metric,
             const QSize& size,
             qreal tolerance,
             const QString& note)
{
    addNumber(records, group, item, metric + QStringLiteral(".w"), size.width(), tolerance, note);
    addNumber(records, group, item, metric + QStringLiteral(".h"), size.height(), tolerance, note);
}

void addRect(QVector<MetricRecord>& records,
             const QString& group,
             const QString& item,
             const QString& metric,
             const QRect& rect,
             qreal tolerance,
             const QString& note)
{
    addNumber(records, group, item, metric + QStringLiteral(".x"), rect.x(), tolerance, note);
    addNumber(records, group, item, metric + QStringLiteral(".y"), rect.y(), tolerance, note);
    addNumber(records, group, item, metric + QStringLiteral(".w"), rect.width(), tolerance, note);
    addNumber(records, group, item, metric + QStringLiteral(".h"), rect.height(), tolerance, note);
}

QImage renderStyleProbe(const QSize& size, const std::function<void(QPainter*)>& draw)
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    draw(&painter);
    return image;
}

void addImageFingerprint(QVector<MetricRecord>& records,
                         const QString& group,
                         const QString& item,
                         const QImage& image,
                         const QString& note)
{
    const QImage argb = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    qint64 alphaSum = 0;
    qint64 lumaSum = 0;
    int paintedPixels = 0;
    const int pixelCount = argb.width() * argb.height();
    for (int y = 0; y < argb.height(); ++y)
    {
        for (int x = 0; x < argb.width(); ++x)
        {
            const QColor color = argb.pixelColor(x, y);
            alphaSum += color.alpha();
            lumaSum += qRound(0.2126 * color.red() + 0.7152 * color.green() + 0.0722 * color.blue());
            if (color.alpha() > 0)
            {
                ++paintedPixels;
            }
        }
    }

    addSize(records, group, item, QStringLiteral("renderSize"), argb.size(), -1.0, note);
    addText(records, group, item, QStringLiteral("imageHash"), imageHashValue(argb), -1.0, note);
    addNumber(records, group, item, QStringLiteral("paintedPixels"), paintedPixels, -1.0, note);
    addNumber(records,
              group,
              item,
              QStringLiteral("meanAlpha"),
              pixelCount > 0 ? static_cast<qreal>(alphaSum) / pixelCount : 0.0,
              -1.0,
              note);
    addNumber(records,
              group,
              item,
              QStringLiteral("meanLuma"),
              pixelCount > 0 ? static_cast<qreal>(lumaSum) / pixelCount : 0.0,
              -1.0,
              note);
}

void addWidgetMetrics(QVector<MetricRecord>& records,
                      QWidget& widget,
                      const QString& item,
                      qreal geometryTolerance)
{
    widget.ensurePolished();
    addSize(records, QStringLiteral("ant-widget"), item, QStringLiteral("sizeHint"), widget.sizeHint(), geometryTolerance, QStringLiteral("Ant adapted widget size hint"));
    addSize(records, QStringLiteral("ant-widget"), item, QStringLiteral("minimumSizeHint"), widget.minimumSizeHint(), geometryTolerance, QStringLiteral("Ant adapted widget minimum size hint"));

    const QFontMetrics metrics(widget.font());
    addNumber(records, QStringLiteral("ant-font"), item, QStringLiteral("height"), metrics.height(), 4.0, QStringLiteral("Font engine tolerance for Qt5/Qt6 rasterization"));
    addNumber(records, QStringLiteral("ant-font"), item, QStringLiteral("ascent"), metrics.ascent(), 4.0, QStringLiteral("Font engine tolerance for Qt5/Qt6 rasterization"));
    addNumber(records, QStringLiteral("ant-font"), item, QStringLiteral("sampleAdvance"), metrics.horizontalAdvance(QStringLiteral("Ant Design 123")), 8.0, QStringLiteral("Font engine tolerance for Qt5/Qt6 rasterization"));
}

void addQtStylePixelMetrics(QVector<MetricRecord>& records)
{
    QStyle* style = qApp->style();
    QWidget probe;
    probe.ensurePolished();
    addText(records,
            QStringLiteral("qt-default-style"),
            QStringLiteral("application-style"),
            QStringLiteral("objectName"),
            style->objectName().isEmpty() ? QStringLiteral("<empty>") : style->objectName(),
            -1.0,
            QStringLiteral("Audit-only Qt default style identity"));

    const QList<QPair<QStyle::PixelMetric, QString>> metrics = {
        {QStyle::PM_ButtonMargin, QStringLiteral("PM_ButtonMargin")},
        {QStyle::PM_ButtonDefaultIndicator, QStringLiteral("PM_ButtonDefaultIndicator")},
        {QStyle::PM_DefaultFrameWidth, QStringLiteral("PM_DefaultFrameWidth")},
        {QStyle::PM_FocusFrameHMargin, QStringLiteral("PM_FocusFrameHMargin")},
        {QStyle::PM_LayoutLeftMargin, QStringLiteral("PM_LayoutLeftMargin")},
        {QStyle::PM_MenuHMargin, QStringLiteral("PM_MenuHMargin")},
        {QStyle::PM_MenuPanelWidth, QStringLiteral("PM_MenuPanelWidth")},
        {QStyle::PM_ScrollBarExtent, QStringLiteral("PM_ScrollBarExtent")},
        {QStyle::PM_SliderThickness, QStringLiteral("PM_SliderThickness")},
        {QStyle::PM_TabBarTabHSpace, QStringLiteral("PM_TabBarTabHSpace")},
        {QStyle::PM_TabBarTabVSpace, QStringLiteral("PM_TabBarTabVSpace")},
        {QStyle::PM_HeaderMargin, QStringLiteral("PM_HeaderMargin")},
        {QStyle::PM_IndicatorWidth, QStringLiteral("PM_IndicatorWidth")},
        {QStyle::PM_IndicatorHeight, QStringLiteral("PM_IndicatorHeight")},
        {QStyle::PM_ExclusiveIndicatorWidth, QStringLiteral("PM_ExclusiveIndicatorWidth")},
        {QStyle::PM_ExclusiveIndicatorHeight, QStringLiteral("PM_ExclusiveIndicatorHeight")},
    };

    for (const auto& metric : metrics)
    {
        addNumber(records,
                  QStringLiteral("qt-default-style"),
                  QStringLiteral("application-style"),
                  metric.second,
                  style->pixelMetric(metric.first, nullptr, &probe),
                  -1.0,
                  QStringLiteral("Audit-only Qt default metric; Ant controls should not rely on this drifting value for visual output"));
    }
}

void addQtPaletteAndFontAudit(QVector<MetricRecord>& records)
{
    const QPalette palette = qApp->palette();
    const QList<QPair<QPalette::ColorRole, QString>> roles = {
        {QPalette::Window, QStringLiteral("Window")},
        {QPalette::Base, QStringLiteral("Base")},
        {QPalette::Text, QStringLiteral("Text")},
        {QPalette::Button, QStringLiteral("Button")},
        {QPalette::ButtonText, QStringLiteral("ButtonText")},
        {QPalette::Highlight, QStringLiteral("Highlight")},
        {QPalette::HighlightedText, QStringLiteral("HighlightedText")},
        {QPalette::PlaceholderText, QStringLiteral("PlaceholderText")},
    };

    for (const auto& role : roles)
    {
        addColor(records,
                 QStringLiteral("qt-default-palette"),
                 QStringLiteral("active"),
                 role.second,
                 palette.color(QPalette::Active, role.first),
                 -1.0,
                 QStringLiteral("Audit-only Qt default palette role"));
        addColor(records,
                 QStringLiteral("qt-default-palette"),
                 QStringLiteral("disabled"),
                 role.second,
                 palette.color(QPalette::Disabled, role.first),
                 -1.0,
                 QStringLiteral("Audit-only Qt default palette role"));
    }

    const QFont appFont = qApp->font();
    const QFontInfo info(appFont);
    const QFontMetrics metrics(appFont);
    addText(records, QStringLiteral("qt-font"), QStringLiteral("application"), QStringLiteral("family"), info.family(), -1.0, QStringLiteral("Audit-only resolved font family"));
    addNumber(records, QStringLiteral("qt-font"), QStringLiteral("application"), QStringLiteral("pointSize"), info.pointSize(), -1.0, QStringLiteral("Audit-only resolved font point size"));
    addNumber(records, QStringLiteral("qt-font"), QStringLiteral("application"), QStringLiteral("height"), metrics.height(), -1.0, QStringLiteral("Audit-only application font metric"));
    addNumber(records, QStringLiteral("qt-font"), QStringLiteral("application"), QStringLiteral("sampleAdvance"), metrics.horizontalAdvance(QStringLiteral("Ant Design 123")), -1.0, QStringLiteral("Audit-only application font metric"));
}

void addQtDefaultDrawAudit(QVector<MetricRecord>& records)
{
    QStyle* style = qApp->style();
    QWidget host;
    host.resize(220, 80);
    host.ensurePolished();
    const QString note = QStringLiteral("Audit-only Qt default drawControl/drawPrimitive fingerprint; Ant controls use their own QPainter/QProxyStyle draw paths");

    QStyleOptionButton buttonOption;
    buttonOption.initFrom(&host);
    buttonOption.rect = QRect(0, 0, 118, 34);
    buttonOption.text = QStringLiteral("Button");
    buttonOption.state |= QStyle::State_Enabled | QStyle::State_Raised;
    addImageFingerprint(records,
                        QStringLiteral("qt-default-draw"),
                        QStringLiteral("CE_PushButton"),
                        renderStyleProbe(buttonOption.rect.size(), [&](QPainter* painter) {
                            style->drawControl(QStyle::CE_PushButton, &buttonOption, painter, &host);
                        }),
                        note);

    QStyleOptionButton checkOption;
    checkOption.initFrom(&host);
    checkOption.rect = QRect(0, 0, 140, 28);
    checkOption.text = QStringLiteral("Check");
    checkOption.state |= QStyle::State_Enabled | QStyle::State_On;
    addImageFingerprint(records,
                        QStringLiteral("qt-default-draw"),
                        QStringLiteral("CE_CheckBox"),
                        renderStyleProbe(checkOption.rect.size(), [&](QPainter* painter) {
                            style->drawControl(QStyle::CE_CheckBox, &checkOption, painter, &host);
                        }),
                        note);

    QStyleOptionFrame frameOption;
    frameOption.initFrom(&host);
    frameOption.rect = QRect(0, 0, 180, 34);
    frameOption.lineWidth = style->pixelMetric(QStyle::PM_DefaultFrameWidth, &frameOption, &host);
    frameOption.state |= QStyle::State_Enabled;
    addImageFingerprint(records,
                        QStringLiteral("qt-default-draw"),
                        QStringLiteral("PE_PanelLineEdit"),
                        renderStyleProbe(frameOption.rect.size(), [&](QPainter* painter) {
                            style->drawPrimitive(QStyle::PE_PanelLineEdit, &frameOption, painter, &host);
                        }),
                        note);

    QStyleOptionComboBox comboOption;
    comboOption.initFrom(&host);
    comboOption.rect = QRect(0, 0, 180, 34);
    comboOption.currentText = QStringLiteral("Alpha");
    comboOption.state |= QStyle::State_Enabled;
    addImageFingerprint(records,
                        QStringLiteral("qt-default-draw"),
                        QStringLiteral("CC_ComboBox"),
                        renderStyleProbe(comboOption.rect.size(), [&](QPainter* painter) {
                            style->drawComplexControl(QStyle::CC_ComboBox, &comboOption, painter, &host);
                        }),
                        note);

    QStyleOptionTab tabOption;
    tabOption.initFrom(&host);
    tabOption.rect = QRect(0, 0, 120, 34);
    tabOption.text = QStringLiteral("General");
    tabOption.shape = QTabBar::RoundedNorth;
    tabOption.state |= QStyle::State_Enabled | QStyle::State_Selected;
    addImageFingerprint(records,
                        QStringLiteral("qt-default-draw"),
                        QStringLiteral("CE_TabBarTab"),
                        renderStyleProbe(tabOption.rect.size(), [&](QPainter* painter) {
                            style->drawControl(QStyle::CE_TabBarTab, &tabOption, painter, &host);
                        }),
                        note);

    QStyleOptionHeader headerOption;
    headerOption.initFrom(&host);
    headerOption.rect = QRect(0, 0, 140, 32);
    headerOption.text = QStringLiteral("Column");
    headerOption.section = 0;
    headerOption.orientation = Qt::Horizontal;
    headerOption.position = QStyleOptionHeader::OnlyOneSection;
    headerOption.state |= QStyle::State_Enabled | QStyle::State_Raised;
    addImageFingerprint(records,
                        QStringLiteral("qt-default-draw"),
                        QStringLiteral("CE_Header"),
                        renderStyleProbe(headerOption.rect.size(), [&](QPainter* painter) {
                            style->drawControl(QStyle::CE_Header, &headerOption, painter, &host);
                        }),
                        note);

    QStyleOptionSlider scrollOption;
    scrollOption.initFrom(&host);
    scrollOption.rect = QRect(0, 0, 16, 180);
    scrollOption.orientation = Qt::Vertical;
    scrollOption.minimum = 0;
    scrollOption.maximum = 100;
    scrollOption.pageStep = 20;
    scrollOption.sliderPosition = 40;
    scrollOption.sliderValue = 40;
    scrollOption.state |= QStyle::State_Enabled;
    addImageFingerprint(records,
                        QStringLiteral("qt-default-draw"),
                        QStringLiteral("CC_ScrollBar"),
                        renderStyleProbe(scrollOption.rect.size(), [&](QPainter* painter) {
                            style->drawComplexControl(QStyle::CC_ScrollBar, &scrollOption, painter, &host);
                        }),
                        note);
}

void addQtComplexGeometryAudit(QVector<MetricRecord>& records)
{
    QComboBox combo;
    combo.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta")});
    combo.resize(180, 32);
    combo.ensurePolished();
    QStyleOptionComboBox comboOption;
    comboOption.initFrom(&combo);
    comboOption.rect = combo.rect();
    comboOption.currentText = combo.currentText();
    addRect(records,
            QStringLiteral("qt-default-geometry"),
            QStringLiteral("QComboBox"),
            QStringLiteral("SC_ComboBoxArrow"),
            combo.style()->subControlRect(QStyle::CC_ComboBox, &comboOption, QStyle::SC_ComboBoxArrow, &combo),
            -1.0,
            QStringLiteral("Audit-only default complex-control geometry"));
    addRect(records,
            QStringLiteral("qt-default-geometry"),
            QStringLiteral("QComboBox"),
            QStringLiteral("SC_ComboBoxEditField"),
            combo.style()->subControlRect(QStyle::CC_ComboBox, &comboOption, QStyle::SC_ComboBoxEditField, &combo),
            -1.0,
            QStringLiteral("Audit-only default complex-control geometry"));

    QTabBar tabBar;
    tabBar.addTab(QStringLiteral("General"));
    tabBar.addTab(QStringLiteral("Advanced"));
    tabBar.resize(260, 40);
    tabBar.ensurePolished();
    addSize(records, QStringLiteral("qt-default-geometry"), QStringLiteral("QTabBar"), QStringLiteral("sizeHint"), tabBar.sizeHint(), -1.0, QStringLiteral("Audit-only default tab geometry"));
    addRect(records, QStringLiteral("qt-default-geometry"), QStringLiteral("QTabBar"), QStringLiteral("tabRect0"), tabBar.tabRect(0), -1.0, QStringLiteral("Audit-only default tab geometry"));
    addRect(records, QStringLiteral("qt-default-geometry"), QStringLiteral("QTabBar"), QStringLiteral("tabRect1"), tabBar.tabRect(1), -1.0, QStringLiteral("Audit-only default tab geometry"));

    QTableWidget table(3, 3);
    table.resize(360, 160);
    table.ensurePolished();
    addNumber(records, QStringLiteral("qt-default-geometry"), QStringLiteral("QHeaderView"), QStringLiteral("horizontalDefaultSectionSize"), table.horizontalHeader()->defaultSectionSize(), -1.0, QStringLiteral("Audit-only default header metric"));
    addNumber(records, QStringLiteral("qt-default-geometry"), QStringLiteral("QHeaderView"), QStringLiteral("verticalDefaultSectionSize"), table.verticalHeader()->defaultSectionSize(), -1.0, QStringLiteral("Audit-only default header metric"));
    addNumber(records, QStringLiteral("qt-default-geometry"), QStringLiteral("QHeaderView"), QStringLiteral("horizontalHeight"), table.horizontalHeader()->sizeHint().height(), -1.0, QStringLiteral("Audit-only default header metric"));

    QScrollBar scroll(Qt::Vertical);
    scroll.setRange(0, 100);
    scroll.setPageStep(20);
    scroll.setValue(40);
    scroll.resize(16, 180);
    scroll.ensurePolished();
    QStyleOptionSlider scrollOption;
    scrollOption.initFrom(&scroll);
    scrollOption.orientation = Qt::Vertical;
    scrollOption.minimum = scroll.minimum();
    scrollOption.maximum = scroll.maximum();
    scrollOption.pageStep = scroll.pageStep();
    scrollOption.sliderPosition = scroll.sliderPosition();
    scrollOption.sliderValue = scroll.value();
    scrollOption.rect = scroll.rect();
    addRect(records,
            QStringLiteral("qt-default-geometry"),
            QStringLiteral("QScrollBar"),
            QStringLiteral("SC_ScrollBarSlider"),
            scroll.style()->subControlRect(QStyle::CC_ScrollBar, &scrollOption, QStyle::SC_ScrollBarSlider, &scroll),
            -1.0,
            QStringLiteral("Audit-only default scrollbar geometry"));
}

void addAntWidgetAudit(QVector<MetricRecord>& records)
{
    AntButton button(QStringLiteral("Primary"));
    button.setButtonType(Ant::ButtonType::Primary);
    addWidgetMetrics(records, button, QStringLiteral("AntButton.Primary"), 4.0);
    addNumber(records, QStringLiteral("ant-style"), QStringLiteral("AntButton.Primary"), QStringLiteral("PM_ButtonMargin"), button.style()->pixelMetric(QStyle::PM_ButtonMargin, nullptr, &button), 1.0, QStringLiteral("AntButton QProxyStyle metric"));
    addColor(records, QStringLiteral("ant-palette"), QStringLiteral("AntButton.Primary"), QStringLiteral("ButtonText"), button.palette().color(QPalette::ButtonText), -1.0, QStringLiteral("Audit-only: primary text color is resolved in AntButtonStyle draw path"));

    AntInput input;
    input.setPlaceholderText(QStringLiteral("Search"));
    addWidgetMetrics(records, input, QStringLiteral("AntInput"), 4.0);
    addColor(records, QStringLiteral("ant-palette"), QStringLiteral("AntInput.lineEdit.active"), QStringLiteral("Text"), input.lineEdit()->palette().color(QPalette::Active, QPalette::Text), 0.0, QStringLiteral("Ant token-driven input text palette"));
    addColor(records, QStringLiteral("ant-palette"), QStringLiteral("AntInput.lineEdit.active"), QStringLiteral("PlaceholderText"), input.lineEdit()->palette().color(QPalette::Active, QPalette::PlaceholderText), 0.0, QStringLiteral("Ant token-driven placeholder palette"));
    addColor(records, QStringLiteral("ant-palette"), QStringLiteral("AntInput.lineEdit.disabled"), QStringLiteral("Text"), input.lineEdit()->palette().color(QPalette::Disabled, QPalette::Text), 0.0, QStringLiteral("Ant token-driven disabled text palette"));

    AntSelect select;
    select.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta"), QStringLiteral("Gamma")});
    addWidgetMetrics(records, select, QStringLiteral("AntSelect"), 4.0);

    AntCheckBox checkBox(QStringLiteral("Remember"));
    checkBox.setChecked(true);
    addWidgetMetrics(records, checkBox, QStringLiteral("AntCheckBox.Checked"), 4.0);
    addNumber(records, QStringLiteral("ant-style"), QStringLiteral("AntCheckBox.Checked"), QStringLiteral("PM_IndicatorWidth"), checkBox.style()->pixelMetric(QStyle::PM_IndicatorWidth, nullptr, &checkBox), 1.0, QStringLiteral("AntCheckBox QProxyStyle indicator metric"));
    addNumber(records, QStringLiteral("ant-style"), QStringLiteral("AntCheckBox.Checked"), QStringLiteral("PM_IndicatorHeight"), checkBox.style()->pixelMetric(QStyle::PM_IndicatorHeight, nullptr, &checkBox), 1.0, QStringLiteral("AntCheckBox QProxyStyle indicator metric"));

    AntProgress progress;
    progress.setPercent(64);
    addWidgetMetrics(records, progress, QStringLiteral("AntProgress.Line"), 4.0);

    AntMenu menu;
    menu.setMode(Ant::MenuMode::Inline);
    menu.addItem(QStringLiteral("home"), QStringLiteral("Home"));
    menu.addSubMenu(QStringLiteral("settings"), QStringLiteral("Settings"));
    menu.addSubItem(QStringLiteral("settings"), QStringLiteral("profile"), QStringLiteral("Profile"));
    menu.setOpenKeys({QStringLiteral("settings")});
    menu.setSelectedKey(QStringLiteral("home"));
    addWidgetMetrics(records, menu, QStringLiteral("AntMenu.Inline"), 8.0);

    AntPagination pagination;
    pagination.setTotal(128);
    pagination.setCurrent(3);
    pagination.setShowQuickJumper(true);
    addWidgetMetrics(records, pagination, QStringLiteral("AntPagination"), 8.0);

    AntTag tag(QStringLiteral("Processing"));
    tag.setColor(QStringLiteral("blue"));
    tag.setClosable(true);
    addWidgetMetrics(records, tag, QStringLiteral("AntTag.BlueClosable"), 4.0);

    AntBadge badge(12);
    badge.setShowZero(true);
    addWidgetMetrics(records, badge, QStringLiteral("AntBadge.Count"), 4.0);

    AntTabs tabs;
    tabs.addTab(new QWidget(&tabs), QStringLiteral("general"), QStringLiteral("General"));
    tabs.addTab(new QWidget(&tabs), QStringLiteral("advanced"), QStringLiteral("Advanced"));
    tabs.resize(360, 180);
    addWidgetMetrics(records, tabs, QStringLiteral("AntTabs.Line"), 6.0);

    AntScrollBar scroll(Qt::Vertical);
    scroll.setRange(0, 100);
    scroll.setPageStep(20);
    scroll.setValue(40);
    scroll.resize(12, 180);
    scroll.ensurePolished();
    QStyleOptionSlider scrollOption;
    scrollOption.initFrom(&scroll);
    scrollOption.orientation = Qt::Vertical;
    scrollOption.minimum = scroll.minimum();
    scrollOption.maximum = scroll.maximum();
    scrollOption.pageStep = scroll.pageStep();
    scrollOption.sliderPosition = scroll.sliderPosition();
    scrollOption.sliderValue = scroll.value();
    scrollOption.rect = scroll.rect();
    addSize(records, QStringLiteral("ant-widget"), QStringLiteral("AntScrollBar.Vertical"), QStringLiteral("sizeHint"), scroll.sizeHint(), 2.0, QStringLiteral("Ant adapted scrollbar size hint"));
    addNumber(records, QStringLiteral("ant-style"), QStringLiteral("AntScrollBar.Vertical"), QStringLiteral("PM_ScrollBarExtent"), scroll.style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, &scroll), 1.0, QStringLiteral("AntScrollBar QProxyStyle extent"));
    addRect(records,
            QStringLiteral("ant-geometry"),
            QStringLiteral("AntScrollBar.Vertical"),
            QStringLiteral("SC_ScrollBarSlider"),
            scroll.style()->subControlRect(QStyle::CC_ScrollBar, &scrollOption, QStyle::SC_ScrollBarSlider, &scroll),
            2.0,
            QStringLiteral("Ant adapted scrollbar slider geometry"));

    AntList list;
    list.addItems({QStringLiteral("Alpha"), QStringLiteral("Beta"), QStringLiteral("Gamma")});
    addWidgetMetrics(records, list, QStringLiteral("AntList"), 8.0);

    AntTable table;
    table.addColumn({QStringLiteral("Name"), QStringLiteral("name"), QStringLiteral("name"), 120});
    table.addColumn({QStringLiteral("Status"), QStringLiteral("status"), QStringLiteral("status"), 120});
    AntTableRow firstRow;
    firstRow.data.insert(QStringLiteral("name"), QStringLiteral("Alpha"));
    firstRow.data.insert(QStringLiteral("status"), QStringLiteral("Ready"));
    AntTableRow secondRow;
    secondRow.data.insert(QStringLiteral("name"), QStringLiteral("Beta"));
    secondRow.data.insert(QStringLiteral("status"), QStringLiteral("Running"));
    table.addRow(firstRow);
    table.addRow(secondRow);
    table.setRowSelection(Ant::TableSelectionMode::Checkbox);
    addWidgetMetrics(records, table, QStringLiteral("AntTable.Checkbox"), 8.0);

    AntTreeNode child;
    child.key = QStringLiteral("child");
    child.title = QStringLiteral("Child");
    AntTreeNode root;
    root.key = QStringLiteral("root");
    root.title = QStringLiteral("Root");
    root.expanded = true;
    root.children.append(child);
    AntTree tree;
    tree.setTreeData({root});
    addWidgetMetrics(records, tree, QStringLiteral("AntTree"), 8.0);

    AntToolTip tooltip;
    tooltip.setTitle(QStringLiteral("Token-driven tooltip"));
    tooltip.setPlacement(Ant::TooltipPlacement::Top);
    addWidgetMetrics(records, tooltip, QStringLiteral("AntToolTip.Top"), 4.0);
}

QVector<MetricRecord> collectMetricRecords()
{
    QVector<MetricRecord> records;
    addQtStylePixelMetrics(records);
    addQtPaletteAndFontAudit(records);
    addQtDefaultDrawAudit(records);
    addQtComplexGeometryAudit(records);
    addAntWidgetAudit(records);
    return records;
}

bool writeRecords(const QString& path, const QVector<MetricRecord>& records, QString* error)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (error)
        {
            *error = QStringLiteral("Cannot write %1").arg(path);
        }
        return false;
    }

    QTextStream out(&file);
    out << "group\titem\tmetric\tkind\tvalue\ttolerance\tnote\tqtVersion\n";
    for (const MetricRecord& record : records)
    {
        out << record.group << '\t'
            << record.item << '\t'
            << record.metric << '\t'
            << record.kind << '\t'
            << record.value << '\t'
            << QString::number(record.tolerance, 'f', 3) << '\t'
            << record.note << '\t'
            << QT_VERSION_STR << '\n';
    }
    return true;
}

bool readRecords(const QString& path, QVector<MetricRecord>* records, QString* error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (error)
        {
            *error = QStringLiteral("Cannot read %1").arg(path);
        }
        return false;
    }

    QTextStream in(&file);
    const QString header = in.readLine();
    if (!header.startsWith(QStringLiteral("group\titem\tmetric\tkind\tvalue\t")))
    {
        if (error)
        {
            *error = QStringLiteral("Unexpected metric audit header in %1").arg(path);
        }
        return false;
    }

    while (!in.atEnd())
    {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty())
        {
            continue;
        }
        const QStringList parts = line.split(QLatin1Char('\t'));
        if (parts.size() < 7)
        {
            if (error)
            {
                *error = QStringLiteral("Malformed metric audit row in %1: %2").arg(path, line);
            }
            return false;
        }
        bool ok = false;
        const qreal tolerance = parts.at(5).toDouble(&ok);
        if (!ok)
        {
            if (error)
            {
                *error = QStringLiteral("Invalid tolerance in %1: %2").arg(path, line);
            }
            return false;
        }
        records->push_back({parts.at(0), parts.at(1), parts.at(2), parts.at(3), parts.at(4), tolerance, parts.at(6)});
    }
    return true;
}

bool compareRecordValue(const MetricRecord& actual, const MetricRecord& baseline, QString* delta)
{
    if (actual.tolerance < 0.0 || baseline.tolerance < 0.0)
    {
        if (delta)
        {
            *delta = QStringLiteral("audit-only");
        }
        return true;
    }

    if (actual.kind != baseline.kind)
    {
        if (delta)
        {
            *delta = QStringLiteral("kind-mismatch");
        }
        return false;
    }

    if (actual.kind == QStringLiteral("number"))
    {
        bool actualOk = false;
        bool baselineOk = false;
        const qreal actualValue = actual.value.toDouble(&actualOk);
        const qreal baselineValue = baseline.value.toDouble(&baselineOk);
        if (!actualOk || !baselineOk)
        {
            if (delta)
            {
                *delta = QStringLiteral("parse-error");
            }
            return false;
        }
        const qreal diff = qAbs(actualValue - baselineValue);
        if (delta)
        {
            *delta = QString::number(diff, 'f', 3);
        }
        return diff <= actual.tolerance;
    }

    if (actual.kind == QStringLiteral("color"))
    {
        QColor actualColor;
        QColor baselineColor;
        if (!parseColorValue(actual.value, &actualColor) || !parseColorValue(baseline.value, &baselineColor))
        {
            if (delta)
            {
                *delta = QStringLiteral("parse-error");
            }
            return false;
        }
        const int diff = colorMaxChannelDelta(actualColor, baselineColor);
        if (delta)
        {
            *delta = QString::number(diff);
        }
        return diff <= actual.tolerance;
    }

    const bool matches = actual.value == baseline.value;
    if (delta)
    {
        *delta = matches ? QStringLiteral("0") : QStringLiteral("text-mismatch");
    }
    return matches;
}

bool compareRecords(const QVector<MetricRecord>& actualRecords,
                    const QVector<MetricRecord>& baselineRecords,
                    QVector<ComparisonRecord>* comparisons,
                    QString* error)
{
    QMap<QString, MetricRecord> baselineByKey;
    for (const MetricRecord& record : baselineRecords)
    {
        baselineByKey.insert(metricKey(record), record);
    }

    QSet<QString> actualKeys;
    for (const MetricRecord& actual : actualRecords)
    {
        const QString key = metricKey(actual);
        actualKeys.insert(key);
        if (!baselineByKey.contains(key))
        {
            if (error)
            {
                *error = QStringLiteral("Missing baseline metric row for %1 / %2 / %3")
                             .arg(actual.group, actual.item, actual.metric);
            }
            return false;
        }

        const MetricRecord baseline = baselineByKey.value(key);
        QString delta;
        const bool passed = compareRecordValue(actual, baseline, &delta);
        if (comparisons)
        {
            comparisons->push_back({actual, baseline.value, delta, passed});
        }
        if (!passed)
        {
            if (error)
            {
                *error = QStringLiteral("Metric %1 / %2 / %3 differs: actual %4, baseline %5, delta %6, tolerance %7")
                             .arg(actual.group,
                                  actual.item,
                                  actual.metric,
                                  actual.value,
                                  baseline.value,
                                  delta,
                                  QString::number(actual.tolerance, 'f', 3));
            }
            return false;
        }
    }

    for (auto it = baselineByKey.constBegin(); it != baselineByKey.constEnd(); ++it)
    {
        if (!actualKeys.contains(it.key()))
        {
            const MetricRecord baseline = it.value();
            if (error)
            {
                *error = QStringLiteral("Baseline metric row has no actual counterpart for %1 / %2 / %3")
                             .arg(baseline.group, baseline.item, baseline.metric);
            }
            return false;
        }
    }
    return true;
}

bool writeComparisons(const QString& path, const QVector<ComparisonRecord>& comparisons, QString* error)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (error)
        {
            *error = QStringLiteral("Cannot write %1").arg(path);
        }
        return false;
    }

    QTextStream out(&file);
    out << "group\titem\tmetric\tkind\tactual\tbaseline\tdelta\ttolerance\tpassed\tqtVersion\n";
    for (const ComparisonRecord& record : comparisons)
    {
        out << record.actual.group << '\t'
            << record.actual.item << '\t'
            << record.actual.metric << '\t'
            << record.actual.kind << '\t'
            << record.actual.value << '\t'
            << record.baselineValue << '\t'
            << record.delta << '\t'
            << QString::number(record.actual.tolerance, 'f', 3) << '\t'
            << (record.passed ? QStringLiteral("yes") : QStringLiteral("no")) << '\t'
            << QT_VERSION_STR << '\n';
    }
    return true;
}
} // namespace

void TestAntQtVersionMetricAudit::stylePaletteFontAndGeometryMetricsAreAuditable()
{
    const QVector<MetricRecord> records = collectMetricRecords();
    QVERIFY2(records.size() >= 90, qPrintable(QStringLiteral("Expected a broad metric audit, got %1 rows").arg(records.size())));

    const QString exportDir = QString::fromLocal8Bit(qgetenv("ANT_QT_METRIC_EXPORT_DIR"));
    const QString baselineDir = QString::fromLocal8Bit(qgetenv("ANT_QT_METRIC_BASELINE_DIR"));

    QString error;
    if (!exportDir.isEmpty())
    {
        QDir dir(exportDir);
        QVERIFY2(dir.exists() || QDir().mkpath(exportDir),
                 qPrintable(QStringLiteral("Cannot create metric export dir %1").arg(exportDir)));
        QVERIFY2(writeRecords(dir.filePath(QStringLiteral("metric-audit.tsv")), records, &error),
                 qPrintable(error));
    }

    if (!baselineDir.isEmpty())
    {
        QVector<MetricRecord> baselineRecords;
        QVERIFY2(readRecords(QDir(baselineDir).filePath(QStringLiteral("metric-audit.tsv")), &baselineRecords, &error),
                 qPrintable(error));
        QVector<ComparisonRecord> comparisons;
        QVERIFY2(compareRecords(records, baselineRecords, &comparisons, &error), qPrintable(error));
        if (!exportDir.isEmpty())
        {
            QVERIFY2(writeComparisons(QDir(exportDir).filePath(QStringLiteral("metric-comparison.tsv")), comparisons, &error),
                     qPrintable(error));
        }
    }
}

int main(int argc, char* argv[])
{
    AntDesign::configureHighDpi();
    QApplication app(argc, argv);
    AntDesign::initialize(&app);

    TestAntQtVersionMetricAudit test;
    return QTest::qExec(&test, argc, argv);
}

#include "TestAntQtVersionMetricAudit.moc"
