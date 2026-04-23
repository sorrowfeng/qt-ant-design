#include "AntDivider.h"

#include <QPainter>

#include <algorithm>

#include "core/AntTheme.h"

AntDivider::AntDivider(QWidget* parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

AntDivider::AntDivider(const QString& text, QWidget* parent)
    : AntDivider(parent)
{
    m_text = text;
}

QString AntDivider::text() const { return m_text; }

void AntDivider::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    updateGeometry();
    update();
    Q_EMIT textChanged(m_text);
}

bool AntDivider::isPlain() const { return m_plain; }

void AntDivider::setPlain(bool plain)
{
    if (m_plain == plain)
    {
        return;
    }
    m_plain = plain;
    updateGeometry();
    update();
    Q_EMIT plainChanged(m_plain);
}

Ant::DividerOrientation AntDivider::orientation() const { return m_orientation; }

void AntDivider::setOrientation(Ant::DividerOrientation orientation)
{
    if (m_orientation == orientation)
    {
        return;
    }
    m_orientation = orientation;
    setSizePolicy(m_orientation == Ant::DividerOrientation::Horizontal ? QSizePolicy::Expanding : QSizePolicy::Fixed,
                  m_orientation == Ant::DividerOrientation::Horizontal ? QSizePolicy::Fixed : QSizePolicy::Preferred);
    updateGeometry();
    update();
    Q_EMIT orientationChanged(m_orientation);
}

Ant::DividerTitlePlacement AntDivider::titlePlacement() const { return m_titlePlacement; }

void AntDivider::setTitlePlacement(Ant::DividerTitlePlacement placement)
{
    if (m_titlePlacement == placement)
    {
        return;
    }
    m_titlePlacement = placement;
    update();
    Q_EMIT titlePlacementChanged(m_titlePlacement);
}

Ant::DividerVariant AntDivider::variant() const { return m_variant; }

void AntDivider::setVariant(Ant::DividerVariant variant)
{
    if (m_variant == variant)
    {
        return;
    }
    m_variant = variant;
    update();
    Q_EMIT variantChanged(m_variant);
}

Ant::DividerSize AntDivider::dividerSize() const { return m_dividerSize; }

void AntDivider::setDividerSize(Ant::DividerSize size)
{
    if (m_dividerSize == size)
    {
        return;
    }
    m_dividerSize = size;
    updateGeometry();
    update();
    Q_EMIT dividerSizeChanged(m_dividerSize);
}

QSize AntDivider::sizeHint() const
{
    if (m_orientation == Ant::DividerOrientation::Vertical)
    {
        return QSize(antTheme->tokens().marginLG, antTheme->tokens().fontSizeLG + 4);
    }
    const int margin = horizontalMargin();
    const int textHeight = m_text.isEmpty() ? 0 : textFontSize() + 8;
    return QSize(160, std::max(1, margin * 2 + std::max(textHeight, antTheme->tokens().lineWidth)));
}

QSize AntDivider::minimumSizeHint() const
{
    return m_orientation == Ant::DividerOrientation::Vertical ? QSize(antTheme->tokens().marginSM, 16) : QSize(40, 1);
}

void AntDivider::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    if (m_orientation == Ant::DividerOrientation::Vertical)
    {
        drawVertical(painter);
    }
    else
    {
        drawHorizontal(painter);
    }
}

int AntDivider::horizontalMargin() const
{
    const auto& token = antTheme->tokens();
    switch (m_dividerSize)
    {
    case Ant::DividerSize::Small:
        return token.marginXS;
    case Ant::DividerSize::Middle:
        return token.margin;
    case Ant::DividerSize::Large:
        return token.marginLG;
    }
    return token.marginLG;
}

int AntDivider::textFontSize() const
{
    return m_plain ? antTheme->tokens().fontSize : antTheme->tokens().fontSizeLG;
}

QPen AntDivider::dividerPen() const
{
    QPen pen(antTheme->tokens().colorSplit, antTheme->tokens().lineWidth);
    if (m_variant == Ant::DividerVariant::Dashed)
    {
        pen.setStyle(Qt::DashLine);
    }
    else if (m_variant == Ant::DividerVariant::Dotted)
    {
        pen.setStyle(Qt::DotLine);
        pen.setCapStyle(Qt::RoundCap);
    }
    return pen;
}

void AntDivider::drawHorizontal(QPainter& painter)
{
    const auto& token = antTheme->tokens();
    const qreal y = height() / 2.0;
    painter.setPen(dividerPen());

    if (m_text.isEmpty())
    {
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
        return;
    }

    QFont f = painter.font();
    f.setPixelSize(textFontSize());
    f.setWeight(m_plain ? QFont::Normal : QFont::DemiBold);
    painter.setFont(f);

    const int padding = textFontSize();
    const int textWidth = QFontMetrics(f).horizontalAdvance(m_text);
    const int blockWidth = textWidth + padding * 2;
    const int edge = static_cast<int>(width() * 0.05);
    int textX = (width() - blockWidth) / 2;
    if (m_titlePlacement == Ant::DividerTitlePlacement::Start)
    {
        textX = edge;
    }
    else if (m_titlePlacement == Ant::DividerTitlePlacement::End)
    {
        textX = width() - edge - blockWidth;
    }
    textX = std::clamp(textX, 0, std::max(0, width() - blockWidth));

    const int lineGap = token.paddingXXS;
    painter.drawLine(QPointF(0, y), QPointF(std::max(0, textX - lineGap), y));
    painter.drawLine(QPointF(std::min(width(), textX + blockWidth + lineGap), y), QPointF(width(), y));

    painter.setPen(m_plain ? token.colorText : token.colorText);
    painter.drawText(QRectF(textX + padding, 0, textWidth, height()), Qt::AlignCenter, m_text);
}

void AntDivider::drawVertical(QPainter& painter)
{
    painter.setPen(dividerPen());
    const qreal x = width() / 2.0;
    const qreal top = height() * 0.05;
    const qreal bottom = height() * 0.95;
    painter.drawLine(QPointF(x, top), QPointF(x, bottom));
}
