#include "AntTypographyStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
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
        f.setPixelSize(token.fontSize);
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
    if (typo->type() == Ant::TypographyType::Link)
    {
        f.setUnderline(true);
    }

    return f;
}

QColor textColorForType(const AntTypography* typo)
{
    const auto& token = antTheme->tokens();
    if (typo->isDisabled())
    {
        return token.colorTextDisabled;
    }
    switch (typo->type())
    {
    case Ant::TypographyType::Secondary:
        return token.colorTextSecondary;
    case Ant::TypographyType::Success:
        return token.colorSuccess;
    case Ant::TypographyType::Warning:
        return token.colorWarning;
    case Ant::TypographyType::Danger:
        return token.colorError;
    case Ant::TypographyType::LightSolid:
        return token.colorTextLightSolid;
    case Ant::TypographyType::Link:
        return token.colorLink;
    case Ant::TypographyType::Default:
    default:
        return token.colorText;
    }
}

QColor markBackgroundColor()
{
    const auto& token = antTheme->tokens();
    if (antTheme->themeMode() == Ant::ThemeMode::Dark)
    {
        return AntPalette::alpha(token.colorWarning, 0.28);
    }
    return AntPalette::tint(token.colorWarning, 0.55);
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
        widget->setCursor(typo->type() == Ant::TypographyType::Link
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
    const QColor color = textColorForType(typo);

    const bool hasCopyBtn = typo->isCopyable();
    const int copyBtnWidth = hasCopyBtn ? fm.horizontalAdvance(QStringLiteral("Copy")) + token.paddingXS * 2 : 0;

    QRect textRect = option->rect;
    if (hasCopyBtn)
    {
        textRect.setRight(textRect.right() - copyBtnWidth);
    }

    // Code mode: draw rounded rect background with monospace font
    if (typo->isCode())
    {
        const int codePad = token.paddingXXS;
        QRect codeRect(textRect.left(), textRect.top(),
                       fm.horizontalAdvance(text) + codePad * 2,
                       fm.height() + codePad * 2);

        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorFillQuaternary);
        painter->drawRoundedRect(codeRect, token.borderRadiusXS, token.borderRadiusXS);

        painter->setFont(font);
        painter->setPen(color);
        QRect innerRect = codeRect.adjusted(codePad, codePad, -codePad, -codePad);
        painter->drawText(innerRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
    // Mark mode: draw yellow highlight background
    else if (typo->isMark())
    {
        QRect markRect = fm.boundingRect(textRect, Qt::TextWordWrap, text);
        markRect.adjust(-2, -1, 2, 1);

        painter->setPen(Qt::NoPen);
        painter->setBrush(markBackgroundColor());
        painter->drawRect(markRect);

        painter->setFont(font);
        painter->setPen(color);
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text);
    }
    // Paragraph or ellipsis modes: word wrap
    else if (typo->isParagraph() || typo->isEllipsis())
    {
        int flags = Qt::AlignLeft | Qt::AlignTop;

        if (typo->isEllipsis())
        {
            const int maxH = fm.height() * typo->ellipsisRows();
            QRect elideRect(textRect.left(), textRect.top(), textRect.width(), maxH);

            QString elidedText = fm.elidedText(text, Qt::ElideRight, textRect.width() * typo->ellipsisRows());

            painter->setFont(font);
            painter->setPen(color);

            if (typo->isDelete())
            {
                QRect br = fm.boundingRect(elideRect, Qt::TextWordWrap, elidedText);
                painter->drawText(elideRect, flags | Qt::TextWordWrap, elidedText);
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
                QRect br = fm.boundingRect(textRect, Qt::TextWordWrap, text);
                painter->drawText(textRect, flags | Qt::TextWordWrap, text);
                const int lineY = textRect.top() + br.center().y();
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

        const int textH = fm.height();
        QRect singleRect(textRect.left(), textRect.top(), textRect.width(), textH);

        painter->drawText(singleRect, Qt::AlignLeft | Qt::AlignVCenter, text);

        // Delete: draw strikethrough line
        if (typo->isDelete())
        {
            const int textW = fm.horizontalAdvance(text);
            const int lineY = singleRect.top() + textH / 2;
            painter->setPen(QPen(color, 1));
            painter->drawLine(singleRect.left(), lineY, singleRect.left() + textW, lineY);
        }
    }

    // Draw copy button
    if (hasCopyBtn)
    {
        QRect btnRect = option->rect;
        btnRect.setLeft(btnRect.right() - copyBtnWidth);

        painter->setFont(widgetFont);
        painter->setPen(token.colorPrimary);
        painter->drawText(btnRect, Qt::AlignCenter, QStringLiteral("Copy"));
    }

    painter->restore();
}
