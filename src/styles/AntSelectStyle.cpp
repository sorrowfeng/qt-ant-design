#include "AntSelectStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include <algorithm>

#include "styles/AntPalette.h"
#include "widgets/AntSelect.h"
#include <QFontMetrics>

namespace
{
struct SelectMetrics
{
    int height = 32;
    int fontSize = 14;
    int radius = 6;
    int paddingX = 11;
    int arrowWidth = 28;
};

SelectMetrics metricsFor(const AntSelect* select)
{
    const auto& token = antTheme->tokens();
    SelectMetrics metrics;
    metrics.height = token.controlHeight;
    metrics.fontSize = token.fontSize;
    metrics.radius = token.borderRadius;
    metrics.paddingX = token.paddingSM - token.lineWidth;
    metrics.arrowWidth = token.fontSize + token.paddingXS * 2;

    if (!select)
    {
        return metrics;
    }

    if (select->selectSize() == Ant::Size::Large)
    {
        metrics.height = token.controlHeightLG;
        metrics.fontSize = token.fontSizeLG;
    }
    else if (select->selectSize() == Ant::Size::Small)
    {
        metrics.height = token.controlHeightSM;
        metrics.fontSize = token.fontSizeSM;
        metrics.radius = token.borderRadiusSM;
        metrics.paddingX = token.paddingXS;
    }

    return metrics;
}

QRectF controlRectFor(const AntSelect* select, const QRect& rect)
{
    const SelectMetrics metrics = metricsFor(select);
    return QRectF(1, (rect.height() - metrics.height) / 2.0, rect.width() - 2, metrics.height);
}

QRectF clearButtonRectFor(const AntSelect* select, const QRect& rect)
{
    const SelectMetrics metrics = metricsFor(select);
    const QRectF control = controlRectFor(select, rect);
    const qreal size = std::min<qreal>(18, control.height() - 8);
    return QRectF(control.right() - metrics.paddingX - size,
                  control.center().y() - size / 2.0,
                  size,
                  size);
}

QColor borderColorFor(const AntSelect* select)
{
    const auto& token = antTheme->tokens();
    if (!select || !select->isEnabled())
    {
        return token.colorBorderDisabled;
    }
    if (select->status() == Ant::Status::Error)
    {
        return (select->isHoveredState() || select->hasFocus() || select->isOpen())
            ? token.colorErrorHover
            : token.colorError;
    }
    if (select->status() == Ant::Status::Warning)
    {
        return (select->isHoveredState() || select->hasFocus() || select->isOpen())
            ? token.colorWarningHover
            : token.colorWarning;
    }
    if (select->isHoveredState() || select->hasFocus() || select->isOpen())
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor backgroundColorFor(const AntSelect* select)
{
    const auto& token = antTheme->tokens();
    if (!select || !select->isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (select->variant() == Ant::Variant::Filled)
    {
        return (select->hasFocus() || select->isOpen()) ? token.colorBgContainer
                                                        : (select->isHoveredState() ? token.colorFillTertiary : token.colorFillQuaternary);
    }
    if (select->variant() == Ant::Variant::Borderless
        || select->variant() == Ant::Variant::Underlined)
    {
        return QColor(0, 0, 0, 0);
    }
    return token.colorBgContainer;
}

bool canClear(const AntSelect* select)
{
    if (!select || !select->isEnabled() || !select->allowClear() || select->isHoveredState() == false || select->isLoading())
    {
        return false;
    }
    if (select->selectMode() != Ant::SelectMode::Single)
    {
        return !select->selectedIndices().isEmpty();
    }
    return select->currentIndex() >= 0;
}

struct TagLayout
{
    QRectF rect;
    QString text;
    int index;
};

QList<TagLayout> layoutTags(const AntSelect* select, const QRectF& textArea, int fontSize, int tagHeight, int tagHPad, int tagVPad, int gap, int maxCount)
{
    QList<TagLayout> tags;
    const auto& token = antTheme->tokens();
    QFont f;
    f.setPixelSize(fontSize);
    const QFontMetrics fm(f);

    const QList<int> indices = select->selectedIndices();
    const int total = indices.size();
    const int showCount = (maxCount > 0 && total > maxCount) ? maxCount : total;

    qreal x = textArea.left();
    qreal y = textArea.top();
    const qreal maxX = textArea.right();

    for (int i = 0; i < showCount; ++i)
    {
        const int idx = indices.at(i);
        if (idx < 0 || idx >= select->count())
            continue;
        const QString text = select->optionAt(idx).label;
        const int textW = fm.horizontalAdvance(text);
        const int tagW = textW + tagHPad * 2 + 16;

        if (x + tagW > maxX && x > textArea.left())
        {
            x = textArea.left();
            y += tagHeight + tagVPad;
        }

        if (y + tagHeight <= textArea.bottom() + tagVPad)
        {
            tags.append({QRectF(x, y, tagW, tagHeight), text, idx});
        }
        x += tagW + gap;
    }

    // Overflow tag
    if (maxCount > 0 && total > maxCount)
    {
        const QString overflowText = QStringLiteral("+ %1 ...").arg(total - maxCount);
        const int overW = fm.horizontalAdvance(overflowText) + tagHPad * 2;
        if (x + overW > maxX && x > textArea.left())
        {
            x = textArea.left();
            y += tagHeight + tagVPad;
        }
        if (y + tagHeight <= textArea.bottom() + tagVPad)
        {
            tags.append({QRectF(x, y, overW, tagHeight), overflowText, -1});
        }
    }

    return tags;
}

void drawTag(QPainter* painter, const QRectF& rect, const QString& text, int fontSize, bool disabled, bool closable)
{
    const auto& token = antTheme->tokens();
    const QColor tagBg = disabled ? token.colorBgContainerDisabled : token.colorFillTertiary;
    const QColor tagText = disabled ? token.colorTextDisabled : token.colorText;

    AntStyleBase::drawCrispRoundedRect(painter, rect.toRect(), Qt::NoPen,
        tagBg, token.borderRadiusSM, token.borderRadiusSM);

    QFont f = painter->font();
    f.setPixelSize(fontSize);
    f.setWeight(QFont::Normal);
    painter->setFont(f);
    painter->setPen(tagText);
    const QRectF textRect = rect.adjusted(6, 0, closable ? -18 : -6, 0);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    if (closable)
    {
        const QPointF center(rect.right() - 9, rect.center().y());
        painter->setPen(QPen(disabled ? token.colorTextDisabled : token.colorTextTertiary,
                             1.2,
                             Qt::SolidLine,
                             Qt::RoundCap));
        painter->drawLine(center + QPointF(-3, -3), center + QPointF(3, 3));
        painter->drawLine(center + QPointF(3, -3), center + QPointF(-3, 3));
    }
}
}

AntSelectStyle::AntSelectStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntSelect>();
}

void AntSelectStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntSelect*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover, true);
    }
}

void AntSelectStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntSelect*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntSelectStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntSelect*>(widget))
    {
        drawSelect(option, painter, widget);
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

bool AntSelectStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* select = qobject_cast<AntSelect*>(watched);
    if (select && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(select);
        option.rect = select->rect();
        if (select->isHoveredState())
        {
            option.state |= QStyle::State_MouseOver;
        }
        if (select->isPressedState())
        {
            option.state |= QStyle::State_Sunken;
        }
        if (select->isOpen())
        {
            option.state |= QStyle::State_On;
        }

        QPainter painter(select);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, select);
        return false;
    }

    return QProxyStyle::eventFilter(watched, event);
}

void AntSelectStyle::drawSelect(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* select = qobject_cast<const AntSelect*>(widget);
    if (!select || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const SelectMetrics metrics = metricsFor(select);
    const QRectF control = controlRectFor(select, option->rect);
    const QRectF clearRect = clearButtonRectFor(select, option->rect);
    const bool disabled = !option->state.testFlag(QStyle::State_Enabled);
    const bool focused = option->state.testFlag(QStyle::State_HasFocus) || select->isOpen();
    const QColor borderColor = borderColorFor(select);
    const QColor backgroundColor = backgroundColorFor(select);
    const bool multiMode = select->selectMode() != Ant::SelectMode::Single;

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    if (focused
        && !disabled
        && select->variant() != Ant::Variant::Borderless
        && select->variant() != Ant::Variant::Underlined)
    {
        const QColor outline = AntPalette::alpha(borderColor, 0.16);
        AntStyleBase::drawCrispRoundedRect(painter, control.adjusted(-1, -1, 1, 1).toRect(),
            QPen(outline, token.controlOutlineWidth), Qt::NoBrush, metrics.radius + 1, metrics.radius + 1);
    }

    if (select->variant() != Ant::Variant::Borderless
        && select->variant() != Ant::Variant::Underlined)
    {
        const bool filled = select->variant() == Ant::Variant::Filled;
        const QPen pen = filled && !focused ? Qt::NoPen : QPen(borderColor, token.lineWidth);
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), pen,
            backgroundColor, metrics.radius, metrics.radius);
    }
    else
    {
        AntStyleBase::drawCrispRoundedRect(painter, control.toRect(), Qt::NoPen,
            backgroundColor, metrics.radius, metrics.radius);
        if (select->variant() == Ant::Variant::Underlined)
        {
            painter->setPen(QPen(borderColor, focused ? 2 : token.lineWidth));
            painter->drawLine(QPointF(control.left(), control.bottom() - 0.5),
                              QPointF(control.right(), control.bottom() - 0.5));
        }
    }

    if (multiMode && !select->selectedIndices().isEmpty())
    {
        // Draw tags in multi/tags mode
        const int tagFontSize = metrics.fontSize - 2;
        const int tagHeight = tagFontSize + token.paddingXXS * 2 + 4;
        const int gap = token.paddingXXS;
        const QRectF textArea = control.adjusted(metrics.paddingX, token.paddingXXS, -(metrics.arrowWidth + metrics.paddingX), -token.paddingXXS);

        const QList<TagLayout> tags = layoutTags(select, textArea, tagFontSize, tagHeight, 6, 2, gap, select->maxTagCount());
        for (const auto& tag : tags)
        {
            drawTag(painter, tag.rect, tag.text, tagFontSize, disabled, tag.index >= 0);
        }
    }
    else if (multiMode && select->selectedIndices().isEmpty())
    {
        // Placeholder text for empty multi-mode
        QFont font = painter->font();
        font.setPixelSize(metrics.fontSize);
        painter->setFont(font);
        painter->setPen(disabled ? token.colorTextDisabled : token.colorTextPlaceholder);
        const QRectF textRect = control.adjusted(metrics.paddingX, 0, -(metrics.arrowWidth + metrics.paddingX), 0);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, select->placeholderText());
    }
    else
    {
        // Single mode text
        const bool hasValue = select->currentIndex() >= 0;
        const QString displayText = hasValue ? select->currentText() : select->placeholderText();
        QColor textColor = hasValue ? token.colorText : token.colorTextPlaceholder;
        if (disabled)
        {
            textColor = token.colorTextDisabled;
        }

        QFont font = painter->font();
        font.setPixelSize(metrics.fontSize);
        painter->setFont(font);
        painter->setPen(textColor);
        const QRectF textRect = control.adjusted(metrics.paddingX, 0, -(metrics.arrowWidth + metrics.paddingX), 0);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, displayText);
    }

    if (select->isLoading())
    {
        painter->setPen(QPen(disabled ? token.colorTextDisabled : token.colorTextTertiary,
                             1.6,
                             Qt::SolidLine,
                             Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        painter->drawArc(clearRect.adjusted(2, 2, -2, -2), select->loadingAngle() * 16, 270 * 16);
    }
    else if (canClear(select))
    {
        const QPointF center = clearRect.center();
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorTextTertiary);
        painter->drawEllipse(center, 5, 5);
        painter->setPen(QPen(token.colorBgContainer, 1.3, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(center + QPointF(-2.2, -2.2), center + QPointF(2.2, 2.2));
        painter->drawLine(center + QPointF(2.2, -2.2), center + QPointF(-2.2, 2.2));
    }
    else
    {
        painter->save();
        painter->translate(clearRect.center());
        painter->rotate(select->arrowRotation());
        painter->translate(-clearRect.center());
        painter->setPen(QPen(disabled ? token.colorTextDisabled : token.colorTextTertiary,
                             1.7,
                             Qt::SolidLine,
                             Qt::RoundCap,
                             Qt::RoundJoin));
        QPainterPath arrow;
        arrow.moveTo(clearRect.center().x() - 5, clearRect.center().y() - 2);
        arrow.lineTo(clearRect.center().x(), clearRect.center().y() + 3);
        arrow.lineTo(clearRect.center().x() + 5, clearRect.center().y() - 2);
        painter->drawPath(arrow);
        painter->restore();
    }

    painter->restore();
}
