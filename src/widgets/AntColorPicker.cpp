#include "AntColorPicker.h"

#include <QEvent>
#include <QFontMetrics>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QVBoxLayout>

#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

// ---- HueSatField ----

class HueSatField : public QWidget
{
public:
    explicit HueSatField(QWidget* parent = nullptr) : QWidget(parent)
    {
        setFixedSize(234, 160);
        setMouseTracking(true);
    }

    void setHsv(int h, int s, int v)
    {
        m_hue = qBound(0, h < 0 ? 0 : h, 359);
        const int x = qBound(0, s * width() / 255, width() - 1);
        const int y = qBound(0, height() - 1 - v * height() / 255, height() - 1);
        const QPoint next(x, y);
        if (m_point != next)
        {
            m_point = next;
        }
        update();
    }

    void setCurrentHsv(int& h, int& s, int& v)
    {
        h = m_hue;
        s = qBound(0, m_point.x() * 255 / qMax(1, width() - 1), 255);
        v = qBound(0, 255 - m_point.y() * 255 / qMax(1, height() - 1), 255);
    }

    void setHue(int hue)
    {
        m_hue = qBound(0, hue < 0 ? 0 : hue, 359);
        update();
    }

std::function<void()> onChanged;

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const QRectF field = rect().adjusted(0.5, 0.5, -0.5, -0.5);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor::fromHsv(m_hue, 255, 255));
        p.drawRoundedRect(field, 3, 3);

        QLinearGradient white(0, 0, width(), 0);
        white.setColorAt(0, Qt::white);
        white.setColorAt(1, QColor(255, 255, 255, 0));
        p.setBrush(white);
        p.drawRoundedRect(field, 3, 3);

        QLinearGradient black(0, 0, 0, height());
        black.setColorAt(0, QColor(0, 0, 0, 0));
        black.setColorAt(1, Qt::black);
        p.setBrush(black);
        p.drawRoundedRect(field, 3, 3);

        p.setPen(QPen(Qt::white, 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(m_point), 8, 8);
        p.setPen(QPen(QColor(0, 0, 0, 70), 1));
        p.drawEllipse(QPointF(m_point), 9, 9);
    }

    void mousePressEvent(QMouseEvent* e) override { updatePoint(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (e->buttons() & Qt::LeftButton) updatePoint(e->pos());
    }

private:
    void updatePoint(const QPoint& pos)
    {
        const int x = qBound(0, pos.x(), width() - 1);
        const int y = qBound(0, pos.y(), height() - 1);
        m_point = QPoint(x, y);
        update();
        if (onChanged) onChanged();
    }

    QPoint m_point = QPoint(233, 0);
    int m_hue = 215;
};

// ---- HueSlider ----

class HueSlider : public QWidget
{
public:
    explicit HueSlider(QWidget* parent = nullptr) : QWidget(parent)
    {
        setFixedSize(234, 16);
        setMouseTracking(true);
    }

    void setHue(int hue)
    {
        m_hue = qBound(0, hue < 0 ? 0 : hue, 359);
        update();
    }

    int hue() const { return m_hue; }

std::function<void(int)> onHueChanged;

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const QRectF r = rect().adjusted(0.5, 4.5, -0.5, -4.5);
        QLinearGradient grad(r.left(), 0, r.right(), 0);
        for (int i = 0; i <= 6; ++i)
        {
            grad.setColorAt(i / 6.0, QColor::fromHsv(i * 60 % 360, 255, 255));
        }
        p.setPen(Qt::NoPen);
        p.setBrush(grad);
        p.drawRoundedRect(r, 4, 4);

        const qreal cx = r.left() + m_hue * r.width() / 359.0;
        p.setPen(QPen(antTheme->tokens().colorBgContainer, 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(cx, r.center().y()), 6, 6);
        p.setPen(QPen(AntPalette::alpha(antTheme->tokens().colorText, 0.35), 1));
        p.drawEllipse(QPointF(cx, r.center().y()), 7, 7);
    }

    void mousePressEvent(QMouseEvent* e) override { updateFromPos(e->pos()); }
    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (e->buttons() & Qt::LeftButton) updateFromPos(e->pos());
    }

private:
    void updateFromPos(const QPoint& pos)
    {
        const QRectF r = rect().adjusted(0.5, 4.5, -0.5, -4.5);
        const int nextHue = qBound(0, static_cast<int>((pos.x() - r.left()) * 359.0 / r.width()), 359);
        if (nextHue != m_hue)
        {
            m_hue = nextHue;
            update();
            if (onHueChanged) onHueChanged(m_hue);
        }
    }

    int m_hue = 215;
};

// ---- AlphaSlider ----

class AlphaSlider : public QWidget
{
public:
    explicit AlphaSlider(QWidget* parent = nullptr) : QWidget(parent)
    {
        setFixedSize(234, 16);
    }

    void setColor(const QColor& color)
    {
        m_color = color;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const QRectF r = rect().adjusted(0.5, 4.5, -0.5, -4.5);
        const int cell = 4;
        for (int y = static_cast<int>(r.top()); y < r.bottom(); y += cell)
        {
            for (int x = static_cast<int>(r.left()); x < r.right(); x += cell)
            {
                const bool dark = ((x / cell) + (y / cell)) % 2;
                p.fillRect(QRect(x, y, cell, cell), dark ? QColor(QStringLiteral("#d9d9d9")) : Qt::white);
            }
        }

        QLinearGradient grad(r.left(), 0, r.right(), 0);
        QColor transparent = m_color;
        transparent.setAlpha(0);
        QColor opaque = m_color;
        opaque.setAlpha(255);
        grad.setColorAt(0.0, transparent);
        grad.setColorAt(1.0, opaque);
        p.setPen(Qt::NoPen);
        p.setBrush(grad);
        p.drawRoundedRect(r, 3, 3);

        const qreal cx = r.right();
        p.setPen(QPen(antTheme->tokens().colorBgContainer, 2));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(cx, r.center().y()), 6, 6);
    }

private:
    QColor m_color = QColor(QStringLiteral("#1677ff"));
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

class ColorPickerPopup : public QFrame
{
public:
    explicit ColorPickerPopup(AntColorPicker* owner)
        : QFrame(owner, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setFocusPolicy(Qt::StrongFocus);
        setupUi();
    }

    void setCurrentColor(const QColor& color)
    {
        if (!color.isValid() || m_currentColor == color)
        {
            return;
        }

        m_currentColor = color;
        updateFromColor(color);
    }

private:
    void setupUi();
    void updateFromColor(const QColor& color);
    void updateSlidersFromColor();
    void updateHexField();
    void syncColor();
    void emitColor();

    void hideEvent(QHideEvent* event) override
    {
        if (m_owner && m_owner->isOpen())
        {
            m_owner->setOpen(false);
        }
        QFrame::hideEvent(event);
    }

    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        const auto& token = antTheme->tokens();
        const QRectF panel = QRectF(rect()).adjusted(10.5, 10.5, -10.5, -10.5);
        const QPointF arrowCenter(panel.left() + 22.0, panel.top());

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        antTheme->drawEffectShadow(&painter, panel.toRect(), 16, token.borderRadiusLG, 0.42);

        QPainterPath arrow;
        arrow.moveTo(arrowCenter.x() - 7.0, arrowCenter.y());
        arrow.lineTo(arrowCenter.x(), arrowCenter.y() - 7.0);
        arrow.lineTo(arrowCenter.x() + 7.0, arrowCenter.y());
        arrow.closeSubpath();

        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorBgElevated);
        painter.drawPath(arrow);
        painter.drawRoundedRect(panel, token.borderRadiusLG, token.borderRadiusLG);

        QPainterPath border;
        border.addRoundedRect(panel, token.borderRadiusLG, token.borderRadiusLG);
        border.addPath(arrow);
        painter.setPen(QPen(token.colorBorder, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(border);

        painter.setPen(QPen(token.colorBgElevated, 2));
        painter.drawLine(QPointF(arrowCenter.x() - 6.0, panel.top() + 0.5),
                         QPointF(arrowCenter.x() + 6.0, panel.top() + 0.5));
    }

    AntColorPicker* m_owner = nullptr;
    QColor m_currentColor = Qt::white;
    bool m_updating = false;

    QWidget* m_hsField = nullptr;
    QWidget* m_hueSlider = nullptr;
    QWidget* m_alphaSlider = nullptr;
    QLineEdit* m_hexEdit = nullptr;
    QWidget* m_previewNew = nullptr;
};

// ---- ColorPickerPopup ----

void ColorPickerPopup::setupUi()
{
    setFixedSize(278, 292);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(22, 22, 22, 22);
    mainLayout->setSpacing(8);

    m_hsField = new HueSatField(this);
    static_cast<HueSatField*>(m_hsField)->onChanged = [this]() {
        syncColor();
        emitColor();
    };
    mainLayout->addWidget(m_hsField, 0, Qt::AlignHCenter);

    m_hueSlider = new HueSlider(this);
    static_cast<HueSlider*>(m_hueSlider)->onHueChanged = [this](int hue) {
        static_cast<HueSatField*>(m_hsField)->setHue(hue);
        syncColor();
        emitColor();
    };
    mainLayout->addWidget(m_hueSlider, 0, Qt::AlignHCenter);

    m_alphaSlider = new AlphaSlider(this);
    mainLayout->addWidget(m_alphaSlider, 0, Qt::AlignHCenter);

    auto* valueRow = new QHBoxLayout();
    valueRow->setSpacing(6);
    m_previewNew = new ColorPreview(this);
    static_cast<ColorPreview*>(m_previewNew)->setFixedSize(28, 28);
    valueRow->addWidget(m_previewNew);

    auto* modeLabel = new QLabel(QStringLiteral("HEX v"), this);
    modeLabel->setFixedWidth(40);
    modeLabel->setAlignment(Qt::AlignCenter);
    valueRow->addWidget(modeLabel);

    m_hexEdit = new QLineEdit(this);
    m_hexEdit->setFixedHeight(32);
    m_hexEdit->setText(QStringLiteral("#FFFFFF"));
    m_hexEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(QStringLiteral("#[0-9A-Fa-f]{0,6}")), this));
    connect(m_hexEdit, &QLineEdit::editingFinished, this, [this]() {
        if (m_updating) return;
        QString t = m_hexEdit->text().trimmed();
        if (!t.startsWith(QLatin1Char('#')))
        {
            t.prepend(QLatin1Char('#'));
        }
        if (t.size() < 7)
        {
            t = t.left(1) + QString(7 - t.size(), QLatin1Char('0')) + t.mid(1);
        }
        QColor c(t);
        if (c.isValid())
        {
            m_currentColor = c;
            updateFromColor(c);
            emitColor();
        }
    });
    valueRow->addWidget(m_hexEdit, 1);

    auto* alphaEdit = new QLineEdit(QStringLiteral("100%"), this);
    alphaEdit->setFixedSize(44, 32);
    alphaEdit->setReadOnly(true);
    valueRow->addWidget(alphaEdit);
    mainLayout->addLayout(valueRow);

    updateFromColor(Qt::white);
}

void ColorPickerPopup::updateFromColor(const QColor& color)
{
    m_updating = true;
    m_currentColor = color;
    updateSlidersFromColor();
    updateHexField();
    static_cast<ColorPreview*>(m_previewNew)->setColor(m_currentColor);
    m_updating = false;
}

void ColorPickerPopup::updateSlidersFromColor()
{
    const int hue = m_currentColor.hue() < 0 ? 0 : m_currentColor.hue();
    static_cast<HueSlider*>(m_hueSlider)->setHue(hue);
    static_cast<AlphaSlider*>(m_alphaSlider)->setColor(m_currentColor);
    static_cast<HueSatField*>(m_hsField)->setHsv(hue, m_currentColor.saturation(), m_currentColor.value());
}

void ColorPickerPopup::updateHexField()
{
    m_hexEdit->setText(m_currentColor.name().toUpper());
}

void ColorPickerPopup::syncColor()
{
    if (m_updating) return;
    m_updating = true;
    int h, s, v;
    static_cast<HueSatField*>(m_hsField)->setCurrentHsv(h, s, v);
    m_currentColor.setHsv(h, s, v);
    static_cast<AlphaSlider*>(m_alphaSlider)->setColor(m_currentColor);
    updateHexField();
    static_cast<ColorPreview*>(m_previewNew)->setColor(m_currentColor);
    m_updating = false;
}

void ColorPickerPopup::emitColor()
{
    if (!m_owner)
    {
        return;
    }
    m_owner->setCurrentColor(m_currentColor);
    Q_EMIT m_owner->colorSelected(m_currentColor);
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
        updatePopupGeometry();
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
    if (m_popup)
    {
        static_cast<ColorPickerPopup*>(m_popup)->setCurrentColor(m_currentColor);
    }
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

bool AntColorPicker::isOpen() const { return m_open; }

void AntColorPicker::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }

    m_open = open;
    if (m_open)
    {
        if (!m_popup)
        {
            m_popup = new ColorPickerPopup(this);
        }
        static_cast<ColorPickerPopup*>(m_popup)->setCurrentColor(m_currentColor);
        updatePopupGeometry();
        AntPopupMotion::show(m_popup);
    }
    else if (m_popup)
    {
        AntPopupMotion::hide(m_popup);
    }

    update();
    Q_EMIT openChanged(m_open);
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
    setOpen(!m_open);
}

void AntColorPicker::updatePopupGeometry()
{
    if (!m_popup)
    {
        return;
    }

    const QSize popupSize = m_popup->sizeHint().isValid() ? m_popup->sizeHint() : m_popup->size();
    m_popup->resize(popupSize);

    QPoint globalPos = mapToGlobal(QPoint(0, height() + 4));
    QRect available = QGuiApplication::primaryScreen() ? QGuiApplication::primaryScreen()->availableGeometry()
                                                       : QRect(globalPos, QSize(1280, 720));
    if (QScreen* screen = QGuiApplication::screenAt(mapToGlobal(rect().center())))
    {
        available = screen->availableGeometry();
    }

    if (globalPos.x() + m_popup->width() > available.right())
    {
        globalPos.setX(qMax(available.left() + 8, available.right() - m_popup->width() - 8));
    }
    if (globalPos.y() + m_popup->height() > available.bottom())
    {
        globalPos.setY(mapToGlobal(QPoint(0, -m_popup->height() - 4)).y());
    }
    m_popup->move(globalPos);
}

QRect AntColorPicker::colorBlockRect() const
{
    return QRect(TriggerPadding, (height() - SwatchSize) / 2, SwatchSize, SwatchSize);
}
