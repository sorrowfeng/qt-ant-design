#include "AntDivider.h"

#include <QFontMetrics>
#include <QPainter>

#include <algorithm>

#include "../styles/AntDividerStyle.h"
#include "core/AntTheme.h"

AntDivider::AntDivider(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntDividerStyle>(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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
    invalidatePaintCache();
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
    invalidatePaintCache();
    updateGeometry();
    update();
    Q_EMIT plainChanged(m_plain);
}

Ant::Orientation AntDivider::orientation() const { return m_orientation; }

void AntDivider::setOrientation(Ant::Orientation orientation)
{
    if (m_orientation == orientation)
    {
        return;
    }
    m_orientation = orientation;
    invalidatePaintCache();
    setSizePolicy(m_orientation == Ant::Orientation::Horizontal ? QSizePolicy::Expanding : QSizePolicy::Fixed,
                  m_orientation == Ant::Orientation::Horizontal ? QSizePolicy::Fixed : QSizePolicy::Preferred);
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
    invalidatePaintCache();
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
    invalidatePaintCache();
    update();
    Q_EMIT variantChanged(m_variant);
}

Ant::Size AntDivider::dividerSize() const { return m_dividerSize; }

void AntDivider::setDividerSize(Ant::Size size)
{
    if (m_dividerSize == size)
    {
        return;
    }
    m_dividerSize = size;
    invalidatePaintCache();
    updateGeometry();
    update();
    Q_EMIT dividerSizeChanged(m_dividerSize);
}

QSize AntDivider::sizeHint() const
{
    const auto& token = antTheme->tokens();
    if (m_orientation == Ant::Orientation::Vertical)
    {
        return QSize(token.marginXS * 2 + token.lineWidth, qRound(token.fontSize * 0.9));
    }
    const int margin = horizontalMargin();
    const int lineHeight = qRound(textFontSize() * token.lineHeight);
    const int textHeight = m_text.isEmpty() ? 0 : lineHeight;
    const int height = m_text.isEmpty()
        ? margin * 2 + token.lineWidth
        : margin + std::max(textHeight, token.lineWidth);
    return QSize(160, std::max(1, height));
}

QSize AntDivider::minimumSizeHint() const
{
    return m_orientation == Ant::Orientation::Vertical ? QSize(antTheme->tokens().marginSM, 16) : QSize(40, 1);
}

void AntDivider::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

const AntDivider::PaintCache& AntDivider::paintCache(const QRect& rect) const
{
    const auto& token = antTheme->tokens();
    const QFont currentFont = font();
    const QColor lineColor = token.colorSplit;
    const QColor textColor = token.colorText;
    const Ant::ThemeMode mode = antTheme->themeMode();

    if (m_paintCache.valid &&
        m_paintCache.rect == rect &&
        m_paintCache.text == m_text &&
        m_paintCache.baseFont == currentFont &&
        m_paintCache.lineColor == lineColor &&
        m_paintCache.textColor == textColor &&
        m_paintCache.themeMode == mode &&
        m_paintCache.orientation == m_orientation &&
        m_paintCache.titlePlacement == m_titlePlacement &&
        m_paintCache.variant == m_variant &&
        m_paintCache.plain == m_plain &&
        m_paintCache.tokenFontSize == token.fontSize &&
        m_paintCache.tokenFontSizeLG == token.fontSizeLG &&
        m_paintCache.lineWidth == token.lineWidth)
    {
        return m_paintCache;
    }

    const int previousBuildCount = m_paintCache.buildCount;
    m_paintCache = PaintCache{};
    m_paintCache.valid = true;
    m_paintCache.rect = rect;
    m_paintCache.text = m_text;
    m_paintCache.baseFont = currentFont;
    m_paintCache.lineColor = lineColor;
    m_paintCache.textColor = textColor;
    m_paintCache.themeMode = mode;
    m_paintCache.orientation = m_orientation;
    m_paintCache.titlePlacement = m_titlePlacement;
    m_paintCache.variant = m_variant;
    m_paintCache.plain = m_plain;
    m_paintCache.horizontal = m_orientation == Ant::Orientation::Horizontal;
    m_paintCache.hasTitle = !m_text.isEmpty();
    m_paintCache.tokenFontSize = token.fontSize;
    m_paintCache.tokenFontSizeLG = token.fontSizeLG;
    m_paintCache.lineWidth = token.lineWidth;
    m_paintCache.buildCount = previousBuildCount + 1;

    Qt::PenStyle penStyle = Qt::SolidLine;
    if (m_variant == Ant::DividerVariant::Dashed)
    {
        penStyle = Qt::DashLine;
    }
    else if (m_variant == Ant::DividerVariant::Dotted)
    {
        penStyle = Qt::DotLine;
    }
    m_paintCache.linePen = QPen(lineColor, token.lineWidth, penStyle);

    if (m_paintCache.horizontal)
    {
        const int y = rect.height() / 2;
        if (!m_paintCache.hasTitle)
        {
            m_paintCache.firstLine = QLineF(rect.left(), y, rect.right(), y);
        }
        else
        {
            QFont titleFont = currentFont;
            titleFont.setPixelSize(m_plain ? token.fontSize : token.fontSizeLG);
            titleFont.setWeight(m_plain ? QFont::Normal : QFont::Medium);
            const QFontMetrics fm(titleFont);
            m_paintCache.titleFont = titleFont;
            m_paintCache.textWidth = fm.horizontalAdvance(m_text);
            m_paintCache.textPadding = titleFont.pixelSize();
            const int totalTextWidth = m_paintCache.textWidth + m_paintCache.textPadding * 2;

            int blockX = rect.left() + (rect.width() - totalTextWidth) / 2;
            if (m_titlePlacement == Ant::DividerTitlePlacement::Start)
            {
                blockX = rect.left() + qRound(rect.width() * 0.05);
            }
            else if (m_titlePlacement == Ant::DividerTitlePlacement::End)
            {
                blockX = rect.right() - qRound(rect.width() * 0.05) - totalTextWidth;
            }
            blockX = qBound(rect.left(), blockX, rect.right() - totalTextWidth);

            m_paintCache.firstLine = QLineF(rect.left(), y, blockX, y);
            m_paintCache.secondLine = QLineF(blockX + totalTextWidth, y, rect.right(), y);
            m_paintCache.textRect = QRect(blockX + m_paintCache.textPadding,
                                          y - fm.height() / 2 - 1,
                                          m_paintCache.textWidth,
                                          fm.height());
        }
    }
    else
    {
        const int x = rect.width() / 2;
        const int lineHeight = qRound(token.fontSize * 0.9);
        const int y1 = rect.top() + (rect.height() - lineHeight) / 2;
        const int y2 = y1 + lineHeight;
        m_paintCache.firstLine = QLineF(x, y1, x, y2);
    }

    const_cast<AntDivider*>(this)->setProperty("antDividerPaintCacheBuildCount", m_paintCache.buildCount);
    return m_paintCache;
}

void AntDivider::invalidatePaintCache()
{
    m_paintCache.valid = false;
}

int AntDivider::horizontalMargin() const
{
    const auto& token = antTheme->tokens();
    if (!m_text.isEmpty())
    {
        return token.margin;
    }
    switch (m_dividerSize)
    {
    case Ant::Size::Small:
        return token.marginXS;
    case Ant::Size::Middle:
        return token.margin;
    case Ant::Size::Large:
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
    f.setWeight(m_plain ? QFont::Normal : QFont::Medium);
    painter.setFont(f);

    const int padding = textFontSize();
    const int textWidth = QFontMetrics(f).horizontalAdvance(m_text);
    const int blockWidth = textWidth + padding * 2;
    const int edge = qRound(width() * 0.05);
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

    painter.drawLine(QPointF(0, y), QPointF(std::max(0, textX), y));
    painter.drawLine(QPointF(std::min(width(), textX + blockWidth), y), QPointF(width(), y));

    painter.setPen(token.colorText);
    painter.drawText(QRectF(textX + padding, 0, textWidth, height()), Qt::AlignCenter, m_text);
}

void AntDivider::drawVertical(QPainter& painter)
{
    painter.setPen(dividerPen());
    const qreal x = width() / 2.0;
    const qreal lineHeight = antTheme->tokens().fontSize * 0.9;
    const qreal top = (height() - lineHeight) / 2.0;
    const qreal bottom = top + lineHeight;
    painter.drawLine(QPointF(x, top), QPointF(x, bottom));
}
