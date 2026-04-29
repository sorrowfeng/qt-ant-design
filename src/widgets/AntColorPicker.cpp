#include "AntColorPicker.h"

#include <QComboBox>
#include <QDialog>
#include <QEvent>
#include <QFontMetrics>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QSpinBox>
#include <QVBoxLayout>

#include "AntButton.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

// ---- HueSatField ----

class HueSatField : public QWidget
{
public:
    explicit HueSatField(QWidget* parent = nullptr) : QWidget(parent)
    {
        setFixedSize(200, 200);
        setMouseTracking(true);
        m_image = QImage(256, 256, QImage::Format_RGB32);
        for (int y = 0; y < 256; ++y)
        {
            for (int x = 0; x < 256; ++x)
            {
                m_image.setPixelColor(x, y, QColor::fromHsvF(x / 255.0, 1.0 - y / 255.0, 1.0));
            }
        }
        // Default position: red (hue=0, sat=1)
        m_point = QPoint(0, 0);
    }

    void setHsv(int h, int s)
    {
        int x = qBound(0, h * 256 / 360, 255);
        int y = qBound(0, 255 - s * 256 / 255, 255);
        if (m_point != QPoint(x, y))
        {
            m_point = QPoint(x, y);
            update();
        }
    }

    void setCurrentHsv(int& h, int& s, int& v)
    {
        h = m_point.x() * 360 / 256;
        s = 255 - m_point.y() * 255 / 256;
        v = m_value;
    }

std::function<void()> onChanged;

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawImage(rect().adjusted(3, 3, -3, -3), m_image);

        const auto& token = antTheme->tokens();
        p.setPen(QPen(token.colorBorder, 3));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect(), 5, 5);

        // Cursor
        p.setPen(QPen(token.colorTextLightSolid, 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(m_point.x() * width() / 256.0, m_point.y() * height() / 256.0), 8, 8);
        p.setPen(QPen(QColor(0, 0, 0, 80), 1));
        p.drawEllipse(QPointF(m_point.x() * width() / 256.0, m_point.y() * height() / 256.0), 9, 9);
    }

    void mousePressEvent(QMouseEvent* e) override { updatePoint(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (e->buttons() & Qt::LeftButton) updatePoint(e->pos());
    }

private:
    void updatePoint(const QPoint& pos)
    {
        int x = qBound(3, pos.x(), width() - 4);
        int y = qBound(3, pos.y(), height() - 4);
        m_point = QPoint(x * 256 / width(), y * 256 / height());
        update();
        if (onChanged) onChanged();
    }

    QImage m_image;
    QPoint m_point;
    int m_value = 255;
};

// ---- ColorSlider ----

class ColorSlider : public QWidget
{
public:
    explicit ColorSlider(QWidget* parent = nullptr) : QWidget(parent)
    {
        setFixedSize(200, 20);
        setMouseTracking(true);
    }

    void setBaseColor(const QColor& c)
    {
        m_baseColor = c;
        update();
    }

    void setValue(int v)
    {
        m_value = qBound(0, v, 255);
        update();
    }

    int value() const { return m_value; }

std::function<void(int)> onValueChanged;

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const QRectF r = rect().adjusted(4, 8, -4, -8);
        QLinearGradient grad(r.left(), 0, r.right(), 0);
        grad.setColorAt(0.0, Qt::black);
        QColor mid = m_baseColor;
        mid.setHsv(m_baseColor.hue(), m_baseColor.saturation(), 128);
        grad.setColorAt(0.5, mid);
        grad.setColorAt(1.0, QColor::fromHsv(m_baseColor.hue(), m_baseColor.saturation(), 255));
        p.setPen(QPen(antTheme->tokens().colorBorder, 1));
        p.setBrush(grad);
        p.drawRoundedRect(r, 3, 3);

        qreal cx = r.left() + m_value * r.width() / 255.0;
        p.setPen(QPen(antTheme->tokens().colorBorder, 1));
        p.setBrush(antTheme->tokens().colorBgContainer);
        p.drawEllipse(QPointF(cx, r.center().y()), 6, 6);
    }

    void mousePressEvent(QMouseEvent* e) override { updateFromPos(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (e->buttons() & Qt::LeftButton) updateFromPos(e->pos());
    }

private:
    void updateFromPos(const QPoint& pos)
    {
        const QRectF r = rect().adjusted(4, 8, -4, -8);
        int v = qBound(0, static_cast<int>((pos.x() - r.left()) * 255.0 / r.width()), 255);
        if (v != m_value) { m_value = v; update(); if (onValueChanged) onValueChanged(v); }
    }

    QColor m_baseColor = Qt::red;
    int m_value = 255;
};

// ---- ColorPreview ----

class ColorPreview : public QWidget
{
public:
    explicit ColorPreview(QWidget* parent = nullptr) : QWidget(parent) { setFixedSize(40, 40); }
    void setColor(const QColor& c) { m_color = c; update(); }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(antTheme->tokens().colorBorder, 1));
        p.setBrush(m_color);
        p.drawRoundedRect(rect(), 4, 4);
    }

private:
    QColor m_color = Qt::white;
};

// ---- ColorGrid ----

class ColorGrid : public QWidget
{
public:
    explicit ColorGrid(int cols, QWidget* parent = nullptr) : QWidget(parent), m_cols(cols) {}

    void setColors(const QList<QColor>& colors)
    {
        m_colors = colors;
        setFixedHeight(((colors.size() + m_cols - 1) / m_cols) * 28 + 4);
        update();
    }

    void setColor(int idx, const QColor& c)
    {
        while (m_colors.size() <= idx) m_colors.append(QColor());
        m_colors[idx] = c;
        update();
    }

    int indexAt(const QPoint& pos) const
    {
        int row = (pos.y() - 2) / 28;
        int col = (pos.x() - 2) / 28;
        int i = row * m_cols + col;
        return i < m_colors.size() ? i : -1;
    }

std::function<void(int)> onColorClicked;

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const auto& token = antTheme->tokens();
        for (int i = 0; i < m_colors.size(); ++i)
        {
            int row = i / m_cols;
            int col = i % m_cols;
            QRectF r(2 + col * 28, 2 + row * 28, 24, 24);
            if (m_colors[i].isValid())
            {
                p.setPen(QPen(token.colorBorderSecondary, 1));
                p.setBrush(m_colors[i]);
                p.drawEllipse(r);
                if (i == m_selected)
                {
                    p.setPen(QPen(token.colorPrimary, 2));
                    p.setBrush(Qt::NoBrush);
                    p.drawEllipse(r.adjusted(-2, -2, 2, 2));
                }
            }
            else
            {
                p.setPen(QPen(token.colorBorderSecondary, 1, Qt::DashLine));
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(r);
            }
        }
    }

    void mousePressEvent(QMouseEvent* e) override
    {
        int idx = indexAt(e->pos());
        if (idx >= 0) { m_selected = idx; update(); if (onColorClicked) onColorClicked(idx); }
    }

private:
    QList<QColor> m_colors;
    int m_cols = 8;
    int m_selected = -1;
};

class ColorPickerDialog : public QDialog
{
public:
    explicit ColorPickerDialog(QWidget* parent = nullptr);
    explicit ColorPickerDialog(const QColor& initial, QWidget* parent = nullptr);

    QColor currentColor() const;
    void setCurrentColor(const QColor& color);

private:
    void setupUi();
    void updateFromColor(const QColor& color);
    void updateSlidersFromColor();
    void updateEditFieldsFromColor();
    void updatePreviewFromColor();
    void syncColor();

    QColor m_currentColor = Qt::white;
    QColor m_previousColor = Qt::white;
    bool m_updating = false;

    QWidget* m_hsField = nullptr;
    QWidget* m_valueSlider = nullptr;
    QLineEdit* m_hexEdit = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QSpinBox* m_rEdit = nullptr;
    QSpinBox* m_gEdit = nullptr;
    QSpinBox* m_bEdit = nullptr;
    QLabel* m_rLabel = nullptr;
    QLabel* m_gLabel = nullptr;
    QLabel* m_bLabel = nullptr;
    QWidget* m_previewOld = nullptr;
    QWidget* m_previewNew = nullptr;
    QWidget* m_presetGrid = nullptr;
    QWidget* m_customGrid = nullptr;
    QPushButton* m_addCustomBtn = nullptr;
    QPushButton* m_removeCustomBtn = nullptr;

    QList<QColor> m_customColors;
};

// ---- ColorPickerDialog ----

ColorPickerDialog::ColorPickerDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi();
}

ColorPickerDialog::ColorPickerDialog(const QColor& initial, QWidget* parent)
    : QDialog(parent)
{
    setupUi();
    setCurrentColor(initial);
}

QColor ColorPickerDialog::currentColor() const { return m_currentColor; }

void ColorPickerDialog::setCurrentColor(const QColor& color)
{
    if (m_currentColor == color) return;
    m_currentColor = color;
    m_previousColor = color;
    updateFromColor(color);
}

void ColorPickerDialog::setupUi()
{
    setWindowTitle(QStringLiteral("Color Picker"));
    setFixedSize(480, 440);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Color area: HS field + sliders + inputs
    auto* colorArea = new QHBoxLayout();

    m_hsField = new HueSatField(this);
    static_cast<HueSatField*>(m_hsField)->onChanged = [this]() { syncColor(); };
    colorArea->addWidget(m_hsField);

    auto* rightCol = new QVBoxLayout();

    // Value slider
    auto* valLabel = new QLabel(QStringLiteral("Value"), this);
    rightCol->addWidget(valLabel);
    m_valueSlider = new ColorSlider(this);
    static_cast<ColorSlider*>(m_valueSlider)->onValueChanged = [this](int) { syncColor(); };
    rightCol->addWidget(m_valueSlider);
    rightCol->addSpacing(8);

    // Hex input
    auto* hexLayout = new QHBoxLayout();
    hexLayout->addWidget(new QLabel(QStringLiteral("Hex"), this));
    m_hexEdit = new QLineEdit(this);
    m_hexEdit->setFixedWidth(100);
    m_hexEdit->setText(QStringLiteral("#FFFFFF"));
    m_hexEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("#[0-9A-Fa-f]{0,6}")), this));
    connect(m_hexEdit, &QLineEdit::editingFinished, this, [this]() {
        if (m_updating) return;
        QString t = m_hexEdit->text().trimmed();
        if (t.size() < 7) t = t.left(1) + QString(7 - t.size(), QLatin1Char('0')) + t.mid(1);
        QColor c(t);
        if (c.isValid()) { m_updating = true; m_currentColor = c; updateFromColor(c); m_updating = false; }
    });
    hexLayout->addWidget(m_hexEdit);
    rightCol->addLayout(hexLayout);
    rightCol->addSpacing(4);

    // Mode combo
    auto* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel(QStringLiteral("Mode"), this));
    m_modeCombo = new QComboBox(this);
    m_modeCombo->addItems({QStringLiteral("RGB"), QStringLiteral("HSV")});
    modeLayout->addWidget(m_modeCombo);
    rightCol->addLayout(modeLayout);
    rightCol->addSpacing(4);

    // RGB/HSV edits
    auto* editGrid = new QGridLayout();
    m_rLabel = new QLabel(QStringLiteral("R"), this);
    m_gLabel = new QLabel(QStringLiteral("G"), this);
    m_bLabel = new QLabel(QStringLiteral("B"), this);
    m_rEdit = new QSpinBox(this); m_rEdit->setRange(0, 255);
    m_gEdit = new QSpinBox(this); m_gEdit->setRange(0, 255);
    m_bEdit = new QSpinBox(this); m_bEdit->setRange(0, 255);
    editGrid->addWidget(m_rLabel, 0, 0); editGrid->addWidget(m_rEdit, 0, 1);
    editGrid->addWidget(m_gLabel, 1, 0); editGrid->addWidget(m_gEdit, 1, 1);
    editGrid->addWidget(m_bLabel, 2, 0); editGrid->addWidget(m_bEdit, 2, 1);
    rightCol->addLayout(editGrid);

    auto rgbChanged = [this]() {
        if (m_updating) return;
        m_updating = true;
        QColor c;
        if (m_modeCombo->currentIndex() == 0)
            c.setRgb(m_rEdit->value(), m_gEdit->value(), m_bEdit->value());
        else
            c.setHsv(m_rEdit->value(), m_gEdit->value(), m_bEdit->value());
        m_currentColor = c;
        updateSlidersFromColor();
        updatePreviewFromColor();
        m_updating = false;
    };
    connect(m_rEdit, &QSpinBox::valueChanged, this, rgbChanged);
    connect(m_gEdit, &QSpinBox::valueChanged, this, rgbChanged);
    connect(m_bEdit, &QSpinBox::valueChanged, this, rgbChanged);

    connect(m_modeCombo, &QComboBox::currentIndexChanged, this, [this](int idx) {
        m_updating = true;
        if (idx == 0) { m_rLabel->setText(QStringLiteral("R")); m_gLabel->setText(QStringLiteral("G")); m_bLabel->setText(QStringLiteral("B")); m_rEdit->setRange(0, 255); m_gEdit->setRange(0, 255); m_bEdit->setRange(0, 255); }
        else { m_rLabel->setText(QStringLiteral("H")); m_gLabel->setText(QStringLiteral("S")); m_bLabel->setText(QStringLiteral("V")); m_rEdit->setRange(0, 359); m_gEdit->setRange(0, 100); m_bEdit->setRange(0, 100); }
        m_updating = false;
        updateEditFieldsFromColor();
    });

    rightCol->addStretch();
    colorArea->addLayout(rightCol);

    // Preview
    auto* previewLayout = new QHBoxLayout();
    previewLayout->addWidget(new QLabel(QStringLiteral("Old"), this));
    m_previewOld = new ColorPreview(this);
    previewLayout->addWidget(m_previewOld);
    previewLayout->addSpacing(8);
    m_previewNew = new ColorPreview(this);
    previewLayout->addWidget(m_previewNew);
    previewLayout->addWidget(new QLabel(QStringLiteral("New"), this));
    previewLayout->addStretch();
    colorArea->addLayout(previewLayout);

    mainLayout->addLayout(colorArea);
    mainLayout->addSpacing(12);

    // Preset colors
    auto* presetLabel = new QLabel(QStringLiteral("Preset Colors"), this);
    mainLayout->addWidget(presetLabel);
    auto* presetGrid = new ColorGrid(13, this);
    m_presetGrid = presetGrid;
    QList<QColor> presets;
    const QStringList presetNames = {QStringLiteral("blue"), QStringLiteral("purple"), QStringLiteral("cyan"),
                                     QStringLiteral("green"), QStringLiteral("magenta"), QStringLiteral("pink"),
                                     QStringLiteral("red"), QStringLiteral("orange"), QStringLiteral("yellow"),
                                     QStringLiteral("volcano"), QStringLiteral("geekblue"), QStringLiteral("gold"),
                                     QStringLiteral("lime")};
    for (const auto& name : presetNames)
        presets.append(AntPalette::presetColor(name));
    presetGrid->setColors(presets);
    presetGrid->onColorClicked = [this](int idx) {
        const auto& colors = QList<QColor>{
            AntPalette::presetColor(QStringLiteral("blue")), AntPalette::presetColor(QStringLiteral("purple")),
            AntPalette::presetColor(QStringLiteral("cyan")), AntPalette::presetColor(QStringLiteral("green")),
            AntPalette::presetColor(QStringLiteral("magenta")), AntPalette::presetColor(QStringLiteral("pink")),
            AntPalette::presetColor(QStringLiteral("red")), AntPalette::presetColor(QStringLiteral("orange")),
            AntPalette::presetColor(QStringLiteral("yellow")), AntPalette::presetColor(QStringLiteral("volcano")),
            AntPalette::presetColor(QStringLiteral("geekblue")), AntPalette::presetColor(QStringLiteral("gold")),
            AntPalette::presetColor(QStringLiteral("lime"))};
        if (idx < colors.size()) setCurrentColor(colors[idx]);
    };
    mainLayout->addWidget(m_presetGrid);

    // Custom colors
    auto* customHeader = new QHBoxLayout();
    customHeader->addWidget(new QLabel(QStringLiteral("Custom Colors"), this));
    customHeader->addStretch();
    m_addCustomBtn = new QPushButton(QStringLiteral("+"), this);
    m_addCustomBtn->setFixedSize(24, 24);
    m_removeCustomBtn = new QPushButton(QStringLiteral("-"), this);
    m_removeCustomBtn->setFixedSize(24, 24);
    customHeader->addWidget(m_addCustomBtn);
    customHeader->addWidget(m_removeCustomBtn);
    mainLayout->addLayout(customHeader);

    auto* customGrid = new ColorGrid(9, this);
    m_customGrid = customGrid;
    m_customColors = QList<QColor>(18, QColor());
    customGrid->setColors(m_customColors);
    customGrid->onColorClicked = [this](int idx) {
        if (idx < m_customColors.size() && m_customColors[idx].isValid())
            setCurrentColor(m_customColors[idx]);
    };
    mainLayout->addWidget(m_customGrid);

    connect(m_addCustomBtn, &QPushButton::clicked, this, [this, customGrid]() {
        for (auto& c : m_customColors) { if (!c.isValid()) { c = m_currentColor; break; } }
        customGrid->setColors(m_customColors);
    });
    connect(m_removeCustomBtn, &QPushButton::clicked, this, [this, customGrid]() {
        for (int i = m_customColors.size() - 1; i >= 0; --i)
        { if (m_customColors[i].isValid()) { m_customColors[i] = QColor(); break; } }
        customGrid->setColors(m_customColors);
    });

    // Buttons
    mainLayout->addSpacing(12);
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    auto* cancelBtn = new AntButton(QStringLiteral("Cancel"), this);
    cancelBtn->setButtonType(Ant::ButtonType::Default);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);
    auto* okBtn = new AntButton(QStringLiteral("OK"), this);
    okBtn->setButtonType(Ant::ButtonType::Primary);
    connect(okBtn, &QPushButton::clicked, this, [this]() {
        accept();
    });
    btnLayout->addWidget(okBtn);
    mainLayout->addLayout(btnLayout);

    updateFromColor(Qt::white);
}

void ColorPickerDialog::updateFromColor(const QColor& color)
{
    m_updating = true;
    m_currentColor = color;
    updateSlidersFromColor();
    updateEditFieldsFromColor();
    updatePreviewFromColor();
    m_updating = false;
}

void ColorPickerDialog::updateSlidersFromColor()
{
    static_cast<ColorSlider*>(m_valueSlider)->setBaseColor(m_currentColor);
    static_cast<ColorSlider*>(m_valueSlider)->setValue(m_currentColor.value());
    static_cast<HueSatField*>(m_hsField)->setHsv(m_currentColor.hue(), m_currentColor.saturation());
}

void ColorPickerDialog::updateEditFieldsFromColor()
{
    if (m_modeCombo->currentIndex() == 0)
    {
        m_rEdit->setValue(m_currentColor.red());
        m_gEdit->setValue(m_currentColor.green());
        m_bEdit->setValue(m_currentColor.blue());
    }
    else
    {
        m_rEdit->setValue(m_currentColor.hue());
        m_gEdit->setValue(m_currentColor.saturation());
        m_bEdit->setValue(m_currentColor.value());
    }
    m_hexEdit->setText(m_currentColor.name().toUpper());
}

void ColorPickerDialog::updatePreviewFromColor()
{
    static_cast<ColorPreview*>(m_previewOld)->setColor(m_previousColor);
    static_cast<ColorPreview*>(m_previewNew)->setColor(m_currentColor);
}

void ColorPickerDialog::syncColor()
{
    if (m_updating) return;
    m_updating = true;
    int h, s, v;
    static_cast<HueSatField*>(m_hsField)->setCurrentHsv(h, s, v);
    v = static_cast<ColorSlider*>(m_valueSlider)->value();
    m_currentColor.setHsv(h, s, v);
    updateEditFieldsFromColor();
    updatePreviewFromColor();
    m_updating = false;
}

// ---- AntColorPicker ----

namespace
{
constexpr int TriggerHeight = 32;
constexpr int SwatchSize = 22;
constexpr int TriggerPadding = 5;
constexpr int TextGap = 8;
constexpr int TextRightPadding = 11;
}

AntColorPicker::AntColorPicker(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this]() {
        update();
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

AntColorPicker::AntColorPicker(const QColor& initial, QWidget* parent)
    : AntColorPicker(parent)
{
    setCurrentColor(initial);
}

QColor AntColorPicker::currentColor() const { return m_currentColor; }

void AntColorPicker::setCurrentColor(const QColor& color)
{
    if (!color.isValid() || m_currentColor == color)
    {
        return;
    }

    m_currentColor = color;
    updateGeometry();
    update();
    Q_EMIT currentColorChanged(m_currentColor);
}

bool AntColorPicker::showText() const { return m_showText; }

void AntColorPicker::setShowText(bool showText)
{
    if (m_showText == showText)
    {
        return;
    }

    m_showText = showText;
    updateGeometry();
    update();
    Q_EMIT showTextChanged(m_showText);
}

QColor AntColorPicker::getColor(const QColor& initial, QWidget* parent, const QString& title)
{
    ColorPickerDialog dlg(initial, parent);
    if (!title.isEmpty())
    {
        dlg.setWindowTitle(title);
    }
    if (dlg.exec() == QDialog::Accepted)
    {
        return dlg.currentColor();
    }
    return {};
}

QSize AntColorPicker::sizeHint() const
{
    if (!m_showText)
    {
        return QSize(TriggerHeight, TriggerHeight);
    }

    QFont f = font();
    f.setPixelSize(antTheme->tokens().fontSize);
    const QFontMetrics fm(f);
    const int textWidth = fm.horizontalAdvance(m_currentColor.name().toUpper());
    return QSize(TriggerPadding + SwatchSize + TextGap + textWidth + TextRightPadding, TriggerHeight);
}

QSize AntColorPicker::minimumSizeHint() const
{
    return m_showText ? QSize(TriggerHeight + TextGap + 48, TriggerHeight) : QSize(TriggerHeight, TriggerHeight);
}

void AntColorPicker::paintEvent(QPaintEvent* /*event*/)
{
    const auto& token = antTheme->tokens();
    const bool enabled = isEnabled();

    QColor border = enabled ? token.colorBorder : token.colorBorderSecondary;
    if (enabled && (m_hovered || hasFocus()))
    {
        border = m_hovered ? token.colorPrimaryHover : token.colorPrimary;
    }

    QColor background = enabled ? token.colorBgContainer : token.colorBgContainerDisabled;

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    const QRectF frame = rect().adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setPen(QPen(border, token.lineWidth));
    painter.setBrush(background);
    painter.drawRoundedRect(frame, token.borderRadius, token.borderRadius);

    QRect block = colorBlockRect();
    QColor color = m_currentColor;
    if (!enabled)
    {
        color.setAlpha(120);
    }

    painter.setPen(QPen(AntPalette::alpha(token.colorText, 0.06), 1));
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(block).adjusted(0.5, 0.5, -0.5, -0.5), token.borderRadiusSM, token.borderRadiusSM);

    if (m_showText)
    {
        QFont f = painter.font();
        f.setPixelSize(token.fontSize);
        painter.setFont(f);
        painter.setPen(enabled ? token.colorText : token.colorTextDisabled);
        const QRect textRect(block.right() + TextGap + 1, 0, width() - block.right() - TextGap - TextRightPadding, height());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_currentColor.name().toUpper());
    }
}

void AntColorPicker::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntColorPicker::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void AntColorPicker::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        m_pressed = true;
        update();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntColorPicker::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        update();
        if (rect().contains(event->pos()))
        {
            openEditor();
        }
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntColorPicker::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && isEnabled())
    {
        openEditor();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void AntColorPicker::openEditor()
{
    ColorPickerDialog dlg(m_currentColor, window());
    dlg.setWindowTitle(QStringLiteral("Color Picker"));
    if (dlg.exec() == QDialog::Accepted)
    {
        setCurrentColor(dlg.currentColor());
        Q_EMIT colorSelected(m_currentColor);
    }
}

QRect AntColorPicker::colorBlockRect() const
{
    return QRect(TriggerPadding, (height() - SwatchSize) / 2, SwatchSize, SwatchSize);
}
