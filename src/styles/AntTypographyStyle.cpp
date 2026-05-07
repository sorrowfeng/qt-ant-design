#include "AntTypographyStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "styles/AntPalette.h"
#include "widgets/AntTypography.h"

namespace
{

int titleFontSizeForLevel(Ant::TypographyTitleLevel level)
{
    switch (level)
    {
    case Ant::TypographyTitleLevel::H1:
        return 38;
    case Ant::TypographyTitleLevel::H2:
        return 30;
    case Ant::TypographyTitleLevel::H3:
        return 24;
    case Ant::TypographyTitleLevel::H4:
        return 20;
    case Ant::TypographyTitleLevel::H5:
        return 16;
    default:
        return 14;
    }
}

QFont buildFont(const AntTypography* typo, const QFont& baseFont)
{
    const auto& token = antTheme->tokens();
    QFont f = baseFont;

    if (typo->isTitle())
    {
        f.setPixelSize(titleFontSizeForLevel(typo->titleLevel()));
        f.setWeight(QFont::DemiBold);
    }
    else if (typo->isCode())
    {
        f.setFamily(QStringLiteral("Consolas, Courier New, monospace"));
        f.setPixelSize(qMax(1, qRound(token.fontSize * 0.85)));
    }
    else
    {
        f.setPixelSize(token.fontSize);
    }

    if (typo->isStrong())
    {
        f.setWeight(QFont::DemiBold);
    }
    if (typo->isItalic())
    {
        f.setItalic(true);
    }
    if (typo->isUnderline())
    {
        f.setUnderline(true);
    }
    return f;
}

QColor textColorForType(const AntTypography* typo, const QStyleOption* option)
{
    const auto& token = antTheme->tokens();
    if (typo->isDisabled())
    {
        return token.colorTextDisabled;
    }
    switch (typo->type())
    {
    case Ant::TypographyType::Secondary:
        return token.colorTextTertiary;
    case Ant::TypographyType::Success:
        return token.colorSuccess;
    case Ant::TypographyType::Warning:
        return token.colorWarning;
    case Ant::TypographyType::Danger:
        if (!typo->href().isEmpty())
        {
            if (typo->isPressed() || (option->state & QStyle::State_Sunken))
            {
                return token.colorErrorActive;
            }
            if (option->state & (QStyle::State_MouseOver | QStyle::State_HasFocus))
            {
                return token.colorErrorHover;
            }
        }
        return token.colorError;
    case Ant::TypographyType::LightSolid:
        return token.colorTextLightSolid;
    case Ant::TypographyType::Link:
        if (typo->isPressed() || (option->state & QStyle::State_Sunken))
        {
            return token.colorLinkActive;
        }
        if (option->state & (QStyle::State_MouseOver | QStyle::State_HasFocus))
        {
            return token.colorLinkHover;
        }
        return token.colorLink;
    case Ant::TypographyType::Default:
    default:
        return token.colorText;
    }
}

QColor markBackgroundColor()
{
    return QColor("#fff1b8");
}

void drawCopyIcon(QPainter* painter, const QRect& rect, const QColor& color)
{
    if (!painter || rect.isEmpty())
    {
        return;
    }

    const int iconSize = qMin(rect.width(), rect.height());
    const QRect iconRect(rect.left() + (rect.width() - iconSize) / 2,
                         rect.top() + (rect.height() - iconSize) / 2,
                         iconSize,
                         iconSize);
    QRectF back(iconRect.left() + iconSize * 0.18,
                iconRect.top() + iconSize * 0.08,
                iconSize * 0.52,
                iconSize * 0.62);
    QRectF front(iconRect.left() + iconSize * 0.32,
                 iconRect.top() + iconSize * 0.25,
                 iconSize * 0.52,
                 iconSize * 0.62);

    painter->setPen(QPen(color, qMax<qreal>(1.2, iconSize * 0.09), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawRoundedRect(back, 1.5, 1.5);
    painter->drawRoundedRect(front, 1.5, 1.5);
}

void drawCheckIcon(QPainter* painter, const QRect& rect, const QColor& color)
{
    if (!painter || rect.isEmpty())
    {
        return;
    }

    const int iconSize = qMin(rect.width(), rect.height());
    const QRectF iconRect(rect.left() + (rect.width() - iconSize) / 2.0,
                          rect.top() + (rect.height() - iconSize) / 2.0,
                          iconSize,
                          iconSize);
    QPainterPath check;
    check.moveTo(iconRect.left() + iconSize * 0.18, iconRect.top() + iconSize * 0.54);
    check.lineTo(iconRect.left() + iconSize * 0.42, iconRect.top() + iconSize * 0.76);
    check.lineTo(iconRect.left() + iconSize * 0.82, iconRect.top() + iconSize * 0.28);

    painter->setPen(QPen(color, qMax<qreal>(1.4, iconSize * 0.10), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(check);
}

QRect typographyAlignedRect(const QRect& bounds, const QSize& requestedSize, Qt::Alignment alignment);

QRect copyIconRect(const QRect& optionRect, const QFontMetrics& fm, const QString& text, int reservedWidth, Qt::Alignment alignment)
{
    const auto& token = antTheme->tokens();
    const int iconSize = token.fontSize;
    const int gap = token.paddingXXS;
    QRect textArea(optionRect.left(), optionRect.top(), qMax(0, optionRect.width() - reservedWidth), optionRect.height());
    const QRect textBounds = fm.boundingRect(textArea, alignment | Qt::TextSingleLine, text);
    const int preferredX = textBounds.right() + 1 + gap;
    const int x = qBound(optionRect.left(), preferredX, optionRect.right() - reservedWidth + 1);
    const int y = typographyAlignedRect(optionRect, QSize(reservedWidth, iconSize + gap), alignment).top();
    return QRect(x, y, iconSize + gap, iconSize + gap);
}

Qt::Alignment normalizedAlignment(Qt::Alignment alignment)
{
    const Qt::Alignment horizontal = alignment & Qt::AlignHorizontal_Mask;
    const Qt::Alignment vertical = alignment & Qt::AlignVertical_Mask;
    return (horizontal == 0 ? Qt::AlignLeft : horizontal) |
           (vertical == 0 ? Qt::AlignVCenter : vertical);
}

QRect typographyAlignedRect(const QRect& bounds, const QSize& requestedSize, Qt::Alignment alignment)
{
    const QSize size(qMin(bounds.width(), requestedSize.width()),
                     qMin(bounds.height(), requestedSize.height()));
    int x = bounds.left();
    if (alignment & Qt::AlignRight)
    {
        x = bounds.right() - size.width() + 1;
    }
    else if (alignment & Qt::AlignHCenter)
    {
        x = bounds.left() + (bounds.width() - size.width()) / 2;
    }

    int y = bounds.top();
    if (alignment & Qt::AlignBottom)
    {
        y = bounds.bottom() - size.height() + 1;
    }
    else if (alignment & Qt::AlignVCenter)
    {
        y = bounds.top() + (bounds.height() - size.height()) / 2;
    }

    return QRect(QPoint(x, y), size);
}

} // namespace

AntTypographyStyle::AntTypographyStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntTypography>();
}

void AntTypographyStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (auto* typo = qobject_cast<AntTypography*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
        widget->setMouseTracking(true);
        widget->setCursor(typo->isDisabled()
                              ? Qt::ForbiddenCursor
                              : typo->type() == Ant::TypographyType::Link
                              ? Qt::PointingHandCursor
                              : Qt::ArrowCursor);
    }
}

void AntTypographyStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntTypography*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntTypographyStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntTypography*>(widget))
    {
        drawTypography(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntTypographyStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntTypographyStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* typo = qobject_cast<AntTypography*>(watched);
    if (typo && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(typo);
        option.rect = typo->rect();
        QPainter painter(typo);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, typo);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntTypographyStyle::drawTypography(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* typo = qobject_cast<const AntTypography*>(widget);
    if (!typo || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const QFont widgetFont = typo->font();
    const QString text = typo->text();

    if (text.isEmpty())
    {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont font = buildFont(typo, widgetFont);
    QFontMetrics fm(font);
    const QColor color = textColorForType(typo, option);
    const Qt::Alignment align = normalizedAlignment(typo->alignment());

    const bool hasCopyBtn = typo->isCopyable();
    const int copyBtnWidth = hasCopyBtn ? token.fontSize + token.paddingXXS * 2 : 0;

    QRect textRect = option->rect;
    if (hasCopyBtn)
    {
        textRect.setRight(textRect.right() - copyBtnWidth);
    }

    // Code mode: draw rounded rect background with monospace font
    if (typo->isCode())
    {
        const int codePadX = qMax(1, qRound(font.pixelSize() * 0.4));
        const int codePadTop = qMax(1, qRound(font.pixelSize() * 0.2));
        const int codePadBottom = qMax(1, qRound(font.pixelSize() * 0.1));
        const QSize codeSize(fm.horizontalAdvance(text) + codePadX * 2 + token.lineWidth * 2,
                             fm.height() + codePadTop + codePadBottom + token.lineWidth * 2);
        QRect codeRect = typographyAlignedRect(textRect, codeSize, align);

        painter->setPen(QPen(AntPalette::alpha(QColor(100, 100, 100), 0.2), token.lineWidth));
        painter->setBrush(AntPalette::alpha(QColor(150, 150, 150), 0.1));
        painter->drawRoundedRect(QRectF(codeRect).adjusted(0.5, 0.5, -0.5, -0.5), 3, 3);

        painter->setFont(font);
        painter->setPen(color);
        QRect innerRect = codeRect.adjusted(codePadX + token.lineWidth,
                                            codePadTop + token.lineWidth,
                                            -(codePadX + token.lineWidth),
                                            -(codePadBottom + token.lineWidth));
        painter->drawText(innerRect, align, text);
    }
    // Mark mode: draw yellow highlight background
    else if (typo->isMark())
    {
        QRect markRect = fm.boundingRect(textRect, align | Qt::TextWordWrap, text);
        markRect.adjust(-2, -1, 2, 1);

        painter->setPen(Qt::NoPen);
        painter->setBrush(markBackgroundColor());
        painter->drawRect(markRect);

        painter->setFont(font);
        painter->setPen(QColor("#000000"));
        painter->drawText(textRect, align | Qt::TextWordWrap, text);
    }
    // Paragraph or ellipsis modes: word wrap
    else if (typo->isParagraph() || typo->isEllipsis())
    {
        int flags = align;

        if (typo->isEllipsis())
        {
            const int maxH = fm.height() * typo->ellipsisRows();
            QRect elideRect = typographyAlignedRect(textRect, QSize(textRect.width(), maxH), align);

            QString elidedText = fm.elidedText(text, Qt::ElideRight, textRect.width() * typo->ellipsisRows());

            painter->setFont(font);
            painter->setPen(color);

            if (typo->isDelete())
            {
                QRect br;
                painter->drawText(elideRect, flags | Qt::TextWordWrap, elidedText, &br);
                const int lineY = br.center().y();
                painter->setPen(QPen(color, 1));
                painter->drawLine(br.left(), lineY, br.right(), lineY);
            }
            else
            {
                painter->drawText(elideRect, flags | Qt::TextWordWrap, elidedText);
            }
        }
        else
        {
            painter->setFont(font);
            painter->setPen(color);

            if (typo->isDelete())
            {
                QRect br;
                painter->drawText(textRect, flags | Qt::TextWordWrap, text, &br);
                const int lineY = br.center().y();
                painter->setPen(QPen(color, 1));
                painter->drawLine(br.left(), lineY, br.right(), lineY);
            }
            else
            {
                painter->drawText(textRect, flags | Qt::TextWordWrap, text);
            }
        }
    }
    // Single-line modes (default, title, text)
    else
    {
        painter->setFont(font);
        painter->setPen(color);

        // Delete: draw strikethrough line
        if (typo->isDelete())
        {
            QRect textBounds;
            painter->drawText(textRect, align | Qt::TextSingleLine, text, &textBounds);
            const int lineY = textBounds.center().y();
            painter->setPen(QPen(color, 1));
            painter->drawLine(textBounds.left(), lineY, textBounds.right(), lineY);
        }
        else
        {
            painter->drawText(textRect, align | Qt::TextSingleLine, text);
        }
    }

    // Draw copy icon
    if (hasCopyBtn)
    {
        QColor copyColor = token.colorLink;
        if (typo->isDisabled())
        {
            copyColor = token.colorTextDisabled;
        }
        else if (typo->isCopied())
        {
            copyColor = token.colorSuccess;
        }
        else if (typo->isCopyPressed())
        {
            copyColor = token.colorLinkActive;
        }
        else if (typo->isCopyHovered())
        {
            copyColor = token.colorLinkHover;
        }

        const QRect iconRect = copyIconRect(option->rect, fm, text, copyBtnWidth, align);
        if (typo->isCopied())
        {
            drawCheckIcon(painter, iconRect, copyColor);
        }
        else
        {
            drawCopyIcon(painter, iconRect, copyColor);
        }
    }

    painter->restore();
}
