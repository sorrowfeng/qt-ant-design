#include "AntTreeSelectStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "widgets/AntTreeSelect.h"

AntTreeSelectStyle::AntTreeSelectStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntTreeSelect>();
}

void AntTreeSelectStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntTreeSelect*>(widget))
    {
        widget->installEventFilter(this);
    }
}

void AntTreeSelectStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTreeSelect*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTreeSelectStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTreeSelect*>(widget))
    {
        drawTreeSelect(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntTreeSelectStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntTreeSelectStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* treeSelect = qobject_cast<AntTreeSelect*>(watched);
    if (treeSelect && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(treeSelect);
        option.rect = treeSelect->rect();
        QPainter painter(treeSelect);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, treeSelect);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

namespace
{
struct TreeSelectMetrics
{
    int height = 32;
    int fontSize = 14;
    int paddingX = 12;
    int arrowWidth = 24;
    int radius = 6;
};

TreeSelectMetrics metricsFor(const AntTreeSelect* select)
{
    TreeSelectMetrics m;
    const auto& token = antTheme->tokens();
    switch (select->selectSize())
    {
    case Ant::Size::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSizeSM;
        break;
    default:
        m.height = token.controlHeight;
        m.fontSize = token.fontSize;
        break;
    }
    m.radius = token.borderRadius;
    return m;
}

QColor borderColorFor(const AntTreeSelect* select)
{
    const auto& token = antTheme->tokens();
    if (select->status() == Ant::Status::Error)
        return token.colorError;
    if (select->status() == Ant::Status::Warning)
        return token.colorWarning;
    return token.colorBorder;
}

QColor backgroundColorFor(const AntTreeSelect* select)
{
    const auto& token = antTheme->tokens();
    switch (select->variant())
    {
    case Ant::Variant::Filled:
        return token.colorFillQuaternary;
    default:
        return token.colorBgContainer;
    }
}
} // namespace

void AntTreeSelectStyle::drawTreeSelect(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* select = qobject_cast<const AntTreeSelect*>(widget);
    if (!select || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const TreeSelectMetrics m = metricsFor(select);
    const QRectF control = QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5);
    const bool disabled = !option->state.testFlag(QStyle::State_Enabled);
    const bool focused = select->isOpen();
    const QColor borderColor = borderColorFor(select);
    const QColor backgroundColor = backgroundColorFor(select);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (focused && !disabled && select->variant() != Ant::Variant::Borderless
        && select->variant() != Ant::Variant::Underlined)
    {
        const QColor outline = token.colorPrimaryBorder;
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(),
            QPen(outline, token.controlOutlineWidth), Qt::NoBrush,
            m.radius + 1, m.radius + 1);
    }

    if (select->variant() != Ant::Variant::Borderless
        && select->variant() != Ant::Variant::Underlined)
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(),
            QPen(borderColor, token.lineWidth), backgroundColor, m.radius, m.radius);
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(),
            Qt::NoPen, backgroundColor, m.radius, m.radius);
        if (select->variant() == Ant::Variant::Underlined)
        {
            painter->setPen(QPen(borderColor, focused ? 2 : token.lineWidth));
            painter->drawLine(QPointF(control.left(), control.bottom() - 0.5),
                              QPointF(control.right(), control.bottom() - 0.5));
        }
    }

    const bool hasValue = !select->value().isEmpty();
    const QString displayText = hasValue ? select->displayText() : select->placeholder();
    QColor textColor = hasValue ? token.colorText : token.colorTextPlaceholder;
    if (disabled)
    {
        textColor = token.colorTextDisabled;
    }

    QFont font = painter->font();
    font.setPixelSize(m.fontSize);
    painter->setFont(font);
    painter->setPen(textColor);

    const QRectF textRect = control.adjusted(m.paddingX, 0, -(m.arrowWidth + m.paddingX), 0);

    if (select->isMultiple() && hasValue)
    {
        QStringList titles = select->displayText().split(QStringLiteral(", "));
        int x = textRect.left();
        int tagH = 22;
        int tagPadX = 8;
        int tagGap = 4;
        int y = textRect.center().y() - tagH / 2;

        QFont tagFont;
        tagFont.setPixelSize(12);
        QFontMetrics fm(tagFont);

        for (int i = 0; i < titles.size(); ++i)
        {
            const QString& title = titles[i];
            int textW = fm.horizontalAdvance(title);
            int tagW = textW + tagPadX * 2 + 16;

            if (x + tagW > textRect.right())
                break;

            QRectF tagRect(x, y, tagW, tagH);
            AntStyleBase::drawCrispRoundedRect(painter, tagRect.toRect(),
                Qt::NoPen, token.colorFillQuaternary, token.borderRadiusXS, token.borderRadiusXS);

            painter->setFont(tagFont);
            painter->setPen(token.colorText);
            painter->drawText(tagRect.adjusted(tagPadX, 0, -16, 0), Qt::AlignVCenter | Qt::AlignLeft, title);

            QPen xPen(token.colorTextTertiary, 1.5, Qt::SolidLine, Qt::RoundCap);
            painter->setPen(xPen);
            QPointF c(tagRect.right() - 10, tagRect.center().y());
            painter->drawLine(c + QPointF(-3, -3), c + QPointF(3, 3));
            painter->drawLine(c + QPointF(3, -3), c + QPointF(-3, 3));

            x += tagW + tagGap;
        }
    }
    else
    {
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, displayText);
    }

    QRectF arrowR(option->rect.width() - m.arrowWidth, 0, m.arrowWidth, option->rect.height());

    if (m.arrowWidth > 0)
    {
        if (hasValue && select->allowClear() && select->isHovered())
        {
            QRectF clearR = arrowR.adjusted(4, 0, -4, 0);
            painter->setPen(Qt::NoPen);
            painter->setBrush(token.colorBgBase);
            painter->drawEllipse(clearR.adjusted(1, 1, -1, -1));
            painter->setPen(QPen(token.colorTextTertiary, 1.5, Qt::SolidLine, Qt::RoundCap));
            painter->drawLine(clearR.center() + QPointF(-4, -4), clearR.center() + QPointF(4, 4));
            painter->drawLine(clearR.center() + QPointF(4, -4), clearR.center() + QPointF(-4, 4));
        }
        else
        {
            QPointF c = arrowR.center();
            painter->setPen(QPen(disabled ? token.colorTextDisabled : token.colorTextTertiary,
                                 1.7, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            QPainterPath arrow;
            if (select->isOpen())
            {
                arrow.moveTo(c.x() - 5, c.y() + 2);
                arrow.lineTo(c.x(), c.y() - 3);
                arrow.lineTo(c.x() + 5, c.y() + 2);
            }
            else
            {
                arrow.moveTo(c.x() - 5, c.y() - 2);
                arrow.lineTo(c.x(), c.y() + 3);
                arrow.lineTo(c.x() + 5, c.y() - 2);
            }
            painter->drawPath(arrow);
        }
    }

    painter->restore();
}
