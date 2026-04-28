#include "AntRadioStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionButton>

#include <algorithm>
#include <utility>

#include "widgets/AntRadio.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int RadioSize = 16;
constexpr int TextSpacing = 8;
constexpr qreal DotRatio = 0.5;

QPainterPath segmentPath(const QRectF& rect, qreal radius, bool roundLeft, bool roundRight)
{
    const qreal r = std::min(radius, rect.height() / 2.0);
    QPainterPath path;
    path.moveTo(rect.left() + (roundLeft ? r : 0), rect.top());
    path.lineTo(rect.right() - (roundRight ? r : 0), rect.top());
    if (roundRight)
    {
        path.quadTo(rect.right(), rect.top(), rect.right(), rect.top() + r);
    }
    path.lineTo(rect.right(), rect.bottom() - (roundRight ? r : 0));
    if (roundRight)
    {
        path.quadTo(rect.right(), rect.bottom(), rect.right() - r, rect.bottom());
    }
    path.lineTo(rect.left() + (roundLeft ? r : 0), rect.bottom());
    if (roundLeft)
    {
        path.quadTo(rect.left(), rect.bottom(), rect.left(), rect.bottom() - r);
    }
    path.lineTo(rect.left(), rect.top() + (roundLeft ? r : 0));
    if (roundLeft)
    {
        path.quadTo(rect.left(), rect.top(), rect.left() + r, rect.top());
    }
    path.closeSubpath();
    return path;
}

std::pair<bool, bool> roundedEdgesFor(const AntRadio* radio)
{
    if (!radio || !radio->parentWidget())
    {
        return {true, true};
    }

    auto radios = radio->parentWidget()->findChildren<AntRadio*>(QString(), Qt::FindDirectChildrenOnly);
    radios.erase(std::remove_if(radios.begin(), radios.end(), [](const AntRadio* item) {
                    return !item->isButtonStyle();
                }),
                 radios.end());
    std::sort(radios.begin(), radios.end(), [](const AntRadio* a, const AntRadio* b) {
        return a->geometry().left() < b->geometry().left();
    });

    if (radios.size() <= 1)
    {
        return {true, true};
    }
    return {radios.first() == radio, radios.last() == radio};
}
}

AntRadioStyle::AntRadioStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntRadio>();
}

void AntRadioStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntRadio*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntRadioStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntRadio*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntRadioStyle::drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::CE_RadioButton)
    {
        const auto* radio = qobject_cast<const AntRadio*>(widget);
        const auto* bopt = qstyleoption_cast<const QStyleOptionButton*>(option);
        if (!radio || !bopt || !painter)
        {
            return;
        }

        const auto& token = antTheme->tokens();
        const bool enabled = bopt->state.testFlag(QStyle::State_Enabled);
        const bool hovered = bopt->state.testFlag(QStyle::State_MouseOver);
        const bool pressed = bopt->state.testFlag(QStyle::State_Sunken);
        if (radio->isButtonStyle())
        {
            const QRectF frame = QRectF(bopt->rect).adjusted(0.5, 0.5, -0.5, -0.5);
            const auto [roundLeft, roundRight] = roundedEdgesFor(radio);
            const QPainterPath path = segmentPath(frame, token.borderRadius, roundLeft, roundRight);
            QColor border = token.colorBorder;
            QColor bg = token.colorBgContainer;
            QColor text = token.colorText;
            if (!enabled)
            {
                border = token.colorBorderDisabled;
                bg = token.colorBgContainerDisabled;
                text = token.colorTextDisabled;
            }
            else if (radio->isChecked())
            {
                border = token.colorPrimary;
                bg = pressed ? token.colorPrimaryActive : (hovered ? token.colorPrimaryHover : token.colorPrimary);
                text = token.colorTextLightSolid;
            }
            else if (hovered)
            {
                border = token.colorPrimary;
                text = token.colorPrimary;
            }

            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
            painter->setPen(QPen(border, token.lineWidth));
            painter->setBrush(bg);
            painter->drawPath(path);

            QFont f = painter->font();
            f.setPixelSize(token.fontSize);
            painter->setFont(f);
            painter->setPen(text);
            painter->drawText(frame, Qt::AlignCenter, radio->text());
            painter->restore();
            return;
        }

        const QRectF circle(0.5, (bopt->rect.height() - RadioSize) / 2.0 + 0.5, RadioSize - 1, RadioSize - 1);

        QColor border;
        QColor background;
        if (!enabled)
        {
            border = token.colorBorder;
            background = token.colorBgContainerDisabled;
        }
        else if (radio->isChecked())
        {
            border = hovered ? token.colorPrimaryHover : token.colorPrimary;
            background = token.colorBgContainer;
            Q_UNUSED(pressed)
        }
        else
        {
            border = hovered ? token.colorPrimary : token.colorBorder;
            background = token.colorBgContainer;
        }

        painter->save();
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        painter->setPen(QPen(border, token.lineWidth));
        painter->setBrush(background);
        painter->drawEllipse(circle);

        if (radio->isChecked())
        {
            const qreal dotSize = RadioSize * DotRatio;
            const QRectF dot(circle.center().x() - dotSize / 2.0, circle.center().y() - dotSize / 2.0, dotSize, dotSize);
            painter->setPen(Qt::NoPen);
            painter->setBrush(enabled ? token.colorPrimary : token.colorTextDisabled);
            painter->drawEllipse(dot);
        }

        if (bopt->state.testFlag(QStyle::State_HasFocus) && enabled)
        {
            const QColor focus = AntPalette::alpha(token.colorPrimary, 0.22);
            painter->setPen(QPen(focus, token.controlOutlineWidth));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(circle.adjusted(-2, -2, 2, 2));
        }

        if (!radio->text().isEmpty())
        {
            QFont f = painter->font();
            f.setPixelSize(token.fontSize);
            painter->setFont(f);
            painter->setPen(enabled ? token.colorText : token.colorTextDisabled);
            const QRectF textRect(circle.right() + TextSpacing, 0, bopt->rect.width() - circle.right() - TextSpacing, bopt->rect.height());
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, radio->text());
        }
        painter->restore();
        return;
    }

    QProxyStyle::drawControl(element, option, painter, widget);
}

int AntRadioStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    if (qobject_cast<const AntRadio*>(widget))
    {
        switch (metric)
        {
        case QStyle::PM_ExclusiveIndicatorWidth:
        case QStyle::PM_ExclusiveIndicatorHeight:
            return RadioSize;
        case QStyle::PM_RadioButtonLabelSpacing:
            return TextSpacing;
        default:
            break;
        }
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

bool AntRadioStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* radio = qobject_cast<AntRadio*>(watched);
    if (radio && event->type() == QEvent::Paint)
    {
        QStyleOptionButton option;
        option.initFrom(radio);
        option.rect = radio->rect();
        option.text = radio->text();
        option.state |= radio->isChecked() ? QStyle::State_On : QStyle::State_Off;
        if (radio->isHoveredState())
        {
            option.state |= QStyle::State_MouseOver;
        }
        if (radio->isPressedState())
        {
            option.state |= QStyle::State_Sunken;
        }

        QPainter painter(radio);
        drawControl(QStyle::CE_RadioButton, &option, &painter, radio);
        return false;
    }
    return QProxyStyle::eventFilter(watched, event);
}
