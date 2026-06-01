#include "AntFileDialogStyle.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStyleOptionHeader>
#include <QStyleOptionToolButton>
#include <QStyleOptionViewItem>
#include <QToolButton>

#include "styles/AntPalette.h"
#include "widgets/AntFileDialog.h"

namespace
{
constexpr int kControlRadius = 6;

bool stateEnabled(const QStyleOption* option)
{
    return option && option->state.testFlag(QStyle::State_Enabled);
}

bool stateHovered(const QStyleOption* option)
{
    return option && option->state.testFlag(QStyle::State_MouseOver);
}

bool statePressed(const QStyleOption* option)
{
    return option && (option->state.testFlag(QStyle::State_Sunken) ||
                      option->state.testFlag(QStyle::State_On));
}

QColor buttonBackground(const QStyleOption* option, bool primary)
{
    const auto& token = antTheme->tokens();
    const bool enabled = stateEnabled(option);
    if (!enabled)
    {
        return token.colorBgContainerDisabled;
    }
    if (primary)
    {
        if (statePressed(option)) return token.colorPrimaryActive;
        if (stateHovered(option)) return token.colorPrimaryHover;
        return token.colorPrimary;
    }
    if (statePressed(option)) return token.colorFill;
    if (stateHovered(option)) return token.colorFillTertiary;
    return token.colorBgContainer;
}

QColor buttonBorder(const QStyleOption* option, bool primary)
{
    const auto& token = antTheme->tokens();
    if (!stateEnabled(option))
    {
        return token.colorBorderDisabled;
    }
    if (primary)
    {
        return Qt::transparent;
    }
    if (stateHovered(option))
    {
        return token.colorPrimaryHover;
    }
    return token.colorBorder;
}

QColor buttonTextColor(const QStyleOption* option, bool primary)
{
    const auto& token = antTheme->tokens();
    if (!stateEnabled(option))
    {
        return token.colorTextDisabled;
    }
    if (primary)
    {
        return token.colorTextLightSolid;
    }
    if (stateHovered(option) || statePressed(option))
    {
        return token.colorPrimaryHover;
    }
    return token.colorText;
}

QRect visualRectFor(const QStyleOption* option)
{
    return option ? option->rect.adjusted(1, 1, -1, -1) : QRect();
}
} // namespace

AntFileDialogStyle::AntFileDialogStyle(QStyle* style)
    : AntDialogStyle(style)
{
}

void AntFileDialogStyle::polish(QWidget* widget)
{
    AntDialogStyle::polish(widget);
    if (isFileDialogScoped(widget))
    {
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntFileDialogStyle::unpolish(QWidget* widget)
{
    AntDialogStyle::unpolish(widget);
}

void AntFileDialogStyle::onThemeUpdate(QWidget* w)
{
    AntDialogStyle::onThemeUpdate(w);
}

void AntFileDialogStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                                       QPainter* painter, const QWidget* widget) const
{
    if (isFileDialogScoped(widget))
    {
        switch (element)
        {
        case QStyle::PE_Widget:
            if (qobject_cast<const AntFileDialog*>(widget))
            {
                drawDialogSurface(option, painter);
                return;
            }
            break;
        case QStyle::PE_PanelLineEdit:
        case QStyle::PE_FrameLineEdit:
            drawLineEditPanel(option, painter, widget);
            return;
        case QStyle::PE_PanelButtonCommand:
            drawButtonPanel(option, painter, widget, qobject_cast<const QPushButton*>(widget) &&
                                                  qobject_cast<const QPushButton*>(widget)->isDefault());
            return;
        case QStyle::PE_PanelButtonTool:
            drawButtonPanel(option, painter, widget, false);
            return;
        case QStyle::PE_FrameFocusRect:
            return;
        default:
            break;
        }
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void AntFileDialogStyle::drawControl(ControlElement element, const QStyleOption* option,
                                     QPainter* painter, const QWidget* widget) const
{
    if (isFileDialogScoped(widget))
    {
        switch (element)
        {
        case QStyle::CE_PushButton:
            drawPushButton(option, painter, widget);
            return;
        case QStyle::CE_ItemViewItem:
            drawItemViewItem(option, painter, widget);
            return;
        case QStyle::CE_Header:
            drawHeaderSection(option, painter, widget);
            drawHeaderLabel(option, painter, widget);
            return;
        case QStyle::CE_HeaderSection:
            drawHeaderSection(option, painter, widget);
            return;
        case QStyle::CE_HeaderLabel:
            drawHeaderLabel(option, painter, widget);
            return;
        default:
            break;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

void AntFileDialogStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                                            QPainter* painter, const QWidget* widget) const
{
    if (isFileDialogScoped(widget))
    {
        switch (control)
        {
        case QStyle::CC_ToolButton:
            drawToolButton(option, painter, widget);
            return;
        case QStyle::CC_ComboBox:
            drawComboBox(option, painter, widget);
            return;
        default:
            break;
        }
    }
    QProxyStyle::drawComplexControl(control, option, painter, widget);
}

int AntFileDialogStyle::pixelMetric(PixelMetric metric, const QStyleOption* option,
                                    const QWidget* widget) const
{
    if (isFileDialogScoped(widget))
    {
        const auto& token = antTheme->tokens();
        switch (metric)
        {
        case QStyle::PM_DefaultFrameWidth:
            return token.lineWidth;
        case QStyle::PM_ButtonMargin:
            return token.paddingXS;
        case QStyle::PM_LayoutHorizontalSpacing:
        case QStyle::PM_LayoutVerticalSpacing:
            return token.marginXS;
        default:
            break;
        }
    }
    return QProxyStyle::pixelMetric(metric, option, widget);
}

QSize AntFileDialogStyle::sizeFromContents(ContentsType type, const QStyleOption* option,
                                           const QSize& size, const QWidget* widget) const
{
    if (isFileDialogScoped(widget))
    {
        const auto& token = antTheme->tokens();
        switch (type)
        {
        case QStyle::CT_PushButton:
            return QSize(qMax(size.width() + token.padding * 2, 74), token.controlHeight);
        case QStyle::CT_ToolButton:
            return QSize(qMax(size.width() + token.paddingXS * 2, token.controlHeight),
                         token.controlHeight);
        case QStyle::CT_ComboBox:
        case QStyle::CT_LineEdit:
            return QSize(size.width() + token.paddingSM * 2, token.controlHeight);
        default:
            break;
        }
    }
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntFileDialogStyle::isFileDialogScoped(const QWidget* widget) const
{
    for (const QWidget* current = widget; current; current = current->parentWidget())
    {
        if (qobject_cast<const AntFileDialog*>(current))
        {
            return true;
        }
    }
    return false;
}

void AntFileDialogStyle::drawDialogSurface(const QStyleOption* option, QPainter* painter) const
{
    if (!option || !painter)
    {
        return;
    }
    painter->fillRect(option->rect, antTheme->tokens().colorBgContainer);
}

void AntFileDialogStyle::drawLineEditPanel(const QStyleOption* option, QPainter* painter,
                                           const QWidget* widget) const
{
    Q_UNUSED(widget)
    if (!option || !painter)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    QColor border = stateEnabled(option) ? token.colorBorder : token.colorBorderDisabled;
    if (stateHovered(option) && stateEnabled(option))
    {
        border = token.colorPrimaryHover;
    }
    const QColor bg = stateEnabled(option) ? token.colorBgContainer : token.colorBgContainerDisabled;

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    AntStyleBase::drawCrispRoundedRect(painter, visualRectFor(option),
                                       QPen(border, token.lineWidth), bg,
                                       kControlRadius, kControlRadius);
    painter->restore();
}

void AntFileDialogStyle::drawButtonPanel(const QStyleOption* option, QPainter* painter,
                                         const QWidget* widget, bool primary) const
{
    Q_UNUSED(widget)
    if (!option || !painter)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    const QColor border = buttonBorder(option, primary);
    AntStyleBase::drawCrispRoundedRect(painter, visualRectFor(option),
                                       border.alpha() > 0 ? QPen(border, token.lineWidth) : Qt::NoPen,
                                       buttonBackground(option, primary),
                                       kControlRadius, kControlRadius);
    painter->restore();
}

void AntFileDialogStyle::drawPushButton(const QStyleOption* option, QPainter* painter,
                                        const QWidget* widget) const
{
    const auto* buttonOption = qstyleoption_cast<const QStyleOptionButton*>(option);
    if (!buttonOption || !painter)
    {
        QProxyStyle::drawControl(QStyle::CE_PushButton, option, painter, widget);
        return;
    }

    const bool primary = buttonOption->features.testFlag(QStyleOptionButton::DefaultButton) ||
                         (qobject_cast<const QPushButton*>(widget) &&
                          qobject_cast<const QPushButton*>(widget)->isDefault());
    drawButtonPanel(option, painter, widget, primary);

    const auto& token = antTheme->tokens();
    const QRect content = option->rect.adjusted(token.paddingXS, 0, -token.paddingXS, 0);
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setPen(buttonTextColor(option, primary));

    QRect textRect = content;
    if (!buttonOption->icon.isNull())
    {
        const QSize iconSize = buttonOption->iconSize.isValid()
            ? buttonOption->iconSize
            : QSize(token.fontSizeLG, token.fontSizeLG);
        const int totalWidth = iconSize.width() + token.paddingXXS +
                               painter->fontMetrics().horizontalAdvance(buttonOption->text);
        int x = content.x() + (content.width() - totalWidth) / 2;
        const QRect iconRect(x, content.y() + (content.height() - iconSize.height()) / 2,
                             iconSize.width(), iconSize.height());
        buttonOption->icon.paint(painter, iconRect, Qt::AlignCenter,
                                 stateEnabled(option) ? QIcon::Normal : QIcon::Disabled);
        textRect.setLeft(iconRect.right() + token.paddingXXS + 1);
    }
    painter->drawText(textRect, Qt::AlignCenter, buttonOption->text);
    painter->restore();
}

void AntFileDialogStyle::drawToolButton(const QStyleOptionComplex* option, QPainter* painter,
                                        const QWidget* widget) const
{
    const auto* toolOption = qstyleoption_cast<const QStyleOptionToolButton*>(option);
    if (!toolOption || !painter)
    {
        QProxyStyle::drawComplexControl(QStyle::CC_ToolButton, option, painter, widget);
        return;
    }

    drawButtonPanel(option, painter, widget, false);

    const auto& token = antTheme->tokens();
    const QColor textColor = buttonTextColor(option, false);
    const QRect content = option->rect.adjusted(token.paddingXXS, token.paddingXXS,
                                                -token.paddingXXS, -token.paddingXXS);

    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setPen(textColor);

    if (!toolOption->icon.isNull())
    {
        const QSize iconSize = toolOption->iconSize.isValid()
            ? toolOption->iconSize
            : QSize(token.fontSizeLG, token.fontSizeLG);
        QRect iconRect(content.center().x() - iconSize.width() / 2,
                       content.center().y() - iconSize.height() / 2,
                       iconSize.width(), iconSize.height());
        if (!toolOption->text.isEmpty())
        {
            iconRect.moveLeft(content.left());
        }
        toolOption->icon.paint(painter, iconRect, Qt::AlignCenter,
                               stateEnabled(option) ? QIcon::Normal : QIcon::Disabled);
    }

    if (!toolOption->text.isEmpty())
    {
        painter->drawText(content, Qt::AlignCenter, toolOption->text);
    }

    if (toolOption->features.testFlag(QStyleOptionToolButton::MenuButtonPopup) ||
        toolOption->features.testFlag(QStyleOptionToolButton::HasMenu))
    {
        const QPointF c(option->rect.right() - token.paddingXS, option->rect.center().y() + 1);
        QPolygonF arrow;
        arrow << QPointF(c.x() - 4, c.y() - 2)
              << QPointF(c.x() + 4, c.y() - 2)
              << QPointF(c.x(), c.y() + 3);
        painter->setBrush(textColor);
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(arrow);
    }

    painter->restore();
}

void AntFileDialogStyle::drawComboBox(const QStyleOptionComplex* option, QPainter* painter,
                                      const QWidget* widget) const
{
    const auto* comboOption = qstyleoption_cast<const QStyleOptionComboBox*>(option);
    if (!comboOption || !painter)
    {
        QProxyStyle::drawComplexControl(QStyle::CC_ComboBox, option, painter, widget);
        return;
    }

    drawLineEditPanel(option, painter, widget);

    const auto& token = antTheme->tokens();
    const QRect textRect = subControlRect(QStyle::CC_ComboBox, option,
                                          QStyle::SC_ComboBoxEditField, widget)
                               .adjusted(token.paddingXS, 0, -token.paddingLG, 0);
    const QColor textColor = stateEnabled(option) ? token.colorText : token.colorTextDisabled;

    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setPen(textColor);
    if (!comboOption->currentIcon.isNull())
    {
        const QRect iconRect(textRect.left(), textRect.center().y() - 8, 16, 16);
        comboOption->currentIcon.paint(painter, iconRect, Qt::AlignCenter,
                                       stateEnabled(option) ? QIcon::Normal : QIcon::Disabled);
        painter->drawText(textRect.adjusted(20, 0, 0, 0),
                          Qt::AlignLeft | Qt::AlignVCenter,
                          comboOption->currentText);
    }
    else
    {
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, comboOption->currentText);
    }

    const QPointF c(option->rect.right() - token.paddingSM, option->rect.center().y() + 1);
    QPolygonF arrow;
    arrow << QPointF(c.x() - 4, c.y() - 2)
          << QPointF(c.x() + 4, c.y() - 2)
          << QPointF(c.x(), c.y() + 3);
    painter->setBrush(textColor);
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(arrow);
    painter->restore();
}

void AntFileDialogStyle::drawItemViewItem(const QStyleOption* option, QPainter* painter,
                                          const QWidget* widget) const
{
    const auto* viewOption = qstyleoption_cast<const QStyleOptionViewItem*>(option);
    if (!viewOption || !painter)
    {
        QProxyStyle::drawControl(QStyle::CE_ItemViewItem, option, painter, widget);
        return;
    }

    QStyleOptionViewItem itemOption(*viewOption);
    const auto& token = antTheme->tokens();
    itemOption.palette.setColor(QPalette::Text, token.colorText);
    itemOption.palette.setColor(QPalette::HighlightedText, token.colorText);
    itemOption.palette.setColor(QPalette::Highlight, token.colorPrimaryBg);

    if (itemOption.state.testFlag(QStyle::State_Selected) ||
        itemOption.state.testFlag(QStyle::State_MouseOver))
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        const QColor bg = itemOption.state.testFlag(QStyle::State_Selected)
            ? token.colorPrimaryBg
            : token.colorFillQuaternary;
        AntStyleBase::drawCrispRoundedRect(painter, itemOption.rect.adjusted(2, 2, -2, -2),
                                           Qt::NoPen, bg, token.borderRadiusSM, token.borderRadiusSM);
        painter->restore();
    }
    itemOption.state &= ~QStyle::State_Selected;
    QProxyStyle::drawControl(QStyle::CE_ItemViewItem, &itemOption, painter, widget);
}

void AntFileDialogStyle::drawHeaderSection(const QStyleOption* option, QPainter* painter,
                                           const QWidget* widget) const
{
    Q_UNUSED(widget)
    if (!option || !painter)
    {
        return;
    }
    const auto& token = antTheme->tokens();
    painter->save();
    painter->fillRect(option->rect, token.colorBgContainer);
    painter->setPen(QPen(token.colorSplit, token.lineWidth));
    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
    painter->restore();
}

void AntFileDialogStyle::drawHeaderLabel(const QStyleOption* option, QPainter* painter,
                                         const QWidget* widget) const
{
    Q_UNUSED(widget)
    const auto* headerOption = qstyleoption_cast<const QStyleOptionHeader*>(option);
    if (!headerOption || !painter)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    QRect textRect = headerOption->rect.adjusted(token.paddingSM, 0, -token.paddingSM, 0);
    const bool hasSortIndicator =
        headerOption->sortIndicator != QStyleOptionHeader::None && headerOption->section != -1;
    if (hasSortIndicator)
    {
        textRect.adjust(0, 0, -(token.fontSizeSM + token.paddingXXS), 0);
    }

    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setPen(stateEnabled(option) ? token.colorTextSecondary : token.colorTextDisabled);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter,
                      headerOption->text);

    if (hasSortIndicator)
    {
        const QRect arrowRect(headerOption->rect.right() - token.paddingSM - token.fontSizeSM,
                              headerOption->rect.center().y() - token.fontSizeSM / 2,
                              token.fontSizeSM,
                              token.fontSizeSM);
        const QPointF c = arrowRect.center();
        QPolygonF arrow;
        if (headerOption->sortIndicator == QStyleOptionHeader::SortDown)
        {
            arrow << QPointF(c.x() - 4, c.y() - 2)
                  << QPointF(c.x() + 4, c.y() - 2)
                  << QPointF(c.x(), c.y() + 3);
        }
        else
        {
            arrow << QPointF(c.x() - 4, c.y() + 2)
                  << QPointF(c.x() + 4, c.y() + 2)
                  << QPointF(c.x(), c.y() - 3);
        }
        painter->setBrush(token.colorTextSecondary);
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(arrow);
    }
    painter->restore();
}
