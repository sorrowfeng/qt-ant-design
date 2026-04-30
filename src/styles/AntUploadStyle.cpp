#include "AntUploadStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "core/AntStyleBase.h"
#include "styles/AntIconPainter.h"
#include "styles/AntPalette.h"
#include "widgets/AntUpload.h"

namespace
{
constexpr int TriggerHeight = 32;
constexpr int TriggerMinWidth = 160;
constexpr int FileItemHeight = 28;
constexpr int PictureItemHeight = 48;
constexpr int CardSize = 100;
constexpr int CardGap = 8;
constexpr int GridColumns = 4;

void drawDashedBorder(QPainter* painter, const QRect& rect, qreal radius, const QColor& color)
{
    QPen pen(color, 1, Qt::DashLine);
    pen.setDashPattern({4, 4});
    AntStyleBase::drawCrispRoundedRect(painter, rect, pen, Qt::NoBrush, radius, radius);
}
} // namespace

AntUploadStyle::AntUploadStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntUpload>();
}

void AntUploadStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntUpload*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntUploadStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntUpload*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntUploadStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntUpload*>(widget))
    {
        drawUpload(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntUploadStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntUploadStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* upload = qobject_cast<AntUpload*>(watched);
    if (upload && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(upload);
        option.rect = upload->rect();
        QPainter painter(upload);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, upload);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntUploadStyle::drawUpload(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* upload = qobject_cast<const AntUpload*>(widget);
    if (!upload || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const auto listType = upload->listType();
    const auto files = upload->fileList();
    const bool disabled = upload->isDisabled();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (listType == Ant::UploadListType::PictureCard)
    {
        const int cols = GridColumns;
        const bool canAdd = !disabled && (upload->maxCount() <= 0 || files.size() < upload->maxCount());

        for (int i = 0; i < files.size(); ++i)
        {
            const int col = i % cols;
            const int row = i / cols;
            const QRect cardRect(col * (CardSize + CardGap), row * (CardSize + CardGap), CardSize, CardSize);
            drawPictureCardItem(painter, cardRect, files[i], upload->m_hoveredItemIndex == i);
        }

        if (canAdd)
        {
            const int triggerIndex = files.size();
            const int col = triggerIndex % cols;
            const int row = triggerIndex / cols;
            const QRect triggerCardRect(col * (CardSize + CardGap), row * (CardSize + CardGap), CardSize, CardSize);
            drawTriggerArea(painter, triggerCardRect, upload->m_triggerHovered, false);
        }
    }
    else
    {
        const QRect triggerR = upload->triggerRect();
        drawTriggerArea(painter, triggerR, upload->m_triggerHovered || upload->m_dragOver, disabled, upload->isDraggerMode());

        int y = triggerR.bottom() + 9;
        for (int i = 0; i < files.size(); ++i)
        {
            const int itemH = listType == Ant::UploadListType::Picture ? PictureItemHeight : FileItemHeight;
            const QRect itemRect(0, y, option->rect.width(), itemH);

            if (listType == Ant::UploadListType::Picture)
            {
                drawPictureFileItem(painter, itemRect, files[i], upload->m_hoveredItemIndex == i);
            }
            else
            {
                const bool hovered = upload->m_hoveredItemIndex == i;
                bool removeHovered = false;
                if (hovered)
                {
                    const int btnSize = 14;
                    const QRect removeBtnRect(
                        itemRect.right() - 8 - btnSize,
                        itemRect.top() + (itemRect.height() - btnSize) / 2,
                        btnSize, btnSize);
                    removeHovered = removeBtnRect.contains(upload->m_mousePos);
                }
                drawTextFileItem(painter, itemRect, files[i], hovered, removeHovered);
            }

            y += itemH;
        }
    }

    painter->restore();
}

void AntUploadStyle::drawTriggerArea(QPainter* painter, const QRect& rect, bool hovered, bool disabled, bool dragger) const
{
    const auto& token = antTheme->tokens();

    if (rect.width() == CardSize && rect.height() == CardSize)
    {
        const QColor borderColor = disabled ? token.colorBorderDisabled
                                            : (hovered ? token.colorPrimary : token.colorBorder);
        drawDashedBorder(painter, rect, token.borderRadius, borderColor);

        const QColor iconColor = disabled ? token.colorTextDisabled : token.colorTextTertiary;
        drawPlusIcon(painter, rect.center(), 16, iconColor);
        return;
    }

    const QColor borderColor = disabled ? token.colorBorderDisabled
                                        : (hovered ? token.colorPrimary : token.colorBorder);

    if (dragger)
    {
        AntStyleBase::drawCrispRoundedRect(painter, rect, QPen(borderColor, token.lineWidth, Qt::DashLine),
            hovered ? token.colorPrimaryBg : token.colorFillQuaternary, token.borderRadius, token.borderRadius);

        AntIconPainter::drawIcon(*painter,
                                 Ant::IconType::CloudUpload,
                                 QRectF(rect.center().x() - 18, rect.top() + 22, 36, 36),
                                 disabled ? token.colorTextDisabled : token.colorPrimary);

        QFont titleFont = painter->font();
        titleFont.setPixelSize(token.fontSize);
        painter->setFont(titleFont);
        painter->setPen(disabled ? token.colorTextDisabled : token.colorText);
        painter->drawText(QRect(rect.left() + 16, rect.top() + 72, rect.width() - 32, 22),
                          Qt::AlignCenter, QStringLiteral("Click or drag file to this area to upload"));
        return;
    }

    AntStyleBase::drawCrispRoundedRect(painter, rect, QPen(borderColor, token.lineWidth),
        disabled ? token.colorBgContainerDisabled : token.colorBgContainer, token.borderRadius, token.borderRadius);

    const QColor iconColor = disabled ? token.colorTextDisabled : (hovered ? token.colorPrimary : token.colorText);
    const int iconSize = 14;
    const int textGap = 8;
    const QString text = QStringLiteral("Click to Upload");

    QFont font = painter->font();
    font.setPixelSize(token.fontSize);
    painter->setFont(font);
    const QFontMetrics fm(font);
    const int textWidth = fm.horizontalAdvance(text);
    const int totalWidth = iconSize + textGap + textWidth;
    const int startX = rect.left() + (rect.width() - totalWidth) / 2;

    AntIconPainter::drawIcon(*painter,
                             Ant::IconType::CloudUpload,
                             QRectF(startX, rect.center().y() - iconSize / 2, iconSize, iconSize),
                             iconColor);

    painter->setPen(iconColor);
    painter->drawText(QRect(startX + iconSize + textGap, rect.top(), textWidth, rect.height()),
                      Qt::AlignLeft | Qt::AlignVCenter, text);
}

void AntUploadStyle::drawTextFileItem(QPainter* painter, const QRect& itemRect, const AntUploadFile& file,
                                       bool hovered, bool removeHovered) const
{
    const auto& token = antTheme->tokens();

    if (hovered)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorFillTertiary);
        painter->drawRect(itemRect);
    }

    const int iconSize = 14;
    const int iconLeft = 8;
    const QPoint iconCenter(iconLeft + iconSize / 2, itemRect.center().y());
    const QColor stateColor = file.status == Ant::UploadFileStatus::Error ? token.colorError : token.colorPrimary;
    if (file.status == Ant::UploadFileStatus::Uploading)
    {
        drawLoadingIcon(painter, iconCenter, iconSize, token.colorTextTertiary);
    }
    else
    {
        drawPaperClipIcon(painter, iconCenter, iconSize, stateColor);
    }

    const int textLeft = iconLeft + iconSize + 8;
    const int rightReserved = 56;
    const int textWidth = itemRect.width() - textLeft - rightReserved;

    QFont nameFont = painter->font();
    nameFont.setPixelSize(token.fontSize);
    painter->setFont(nameFont);
    const QFontMetrics nameFm(nameFont);

    const QString displayName = truncatedName(file.name, textWidth, nameFm);
    const QColor nameColor = file.status == Ant::UploadFileStatus::Error
        ? token.colorError
        : (file.status == Ant::UploadFileStatus::Done ? token.colorPrimary : token.colorText);
    painter->setPen(nameColor);
    painter->drawText(QRect(textLeft, itemRect.top(), textWidth, itemRect.height()),
                      Qt::AlignLeft | Qt::AlignVCenter, displayName);

    if (file.status == Ant::UploadFileStatus::Uploading)
    {
        const int barY = itemRect.bottom() - 4;
        const int barH = 2;
        drawProgressBar(painter, QRect(textLeft, barY, textWidth, barH), file.percent);
    }

    const int statusIconSize = 14;
    const int statusX = itemRect.right() - rightReserved + 8;
    const int statusY = itemRect.top() + (itemRect.height() - statusIconSize) / 2;
    const QPoint statusCenter(statusX + statusIconSize / 2, statusY + statusIconSize / 2);

    if (file.status == Ant::UploadFileStatus::Done)
    {
        drawCheckIcon(painter, statusCenter, statusIconSize);
    }
    else if (file.status == Ant::UploadFileStatus::Error)
    {
        drawErrorIcon(painter, statusCenter, statusIconSize);
    }

    if (hovered && file.status != Ant::UploadFileStatus::Uploading)
    {
        const int btnSize = 14;
        const int btnX = itemRect.right() - 8 - btnSize;
        const int btnY = itemRect.top() + (itemRect.height() - btnSize) / 2;
        const QPoint btnCenter(btnX + btnSize / 2, btnY + btnSize / 2);
        const QColor closeColor = removeHovered ? token.colorError : token.colorTextTertiary;
        drawCloseIcon(painter, btnCenter, btnSize, closeColor);
    }
}

void AntUploadStyle::drawPictureFileItem(QPainter* painter, const QRect& itemRect, const AntUploadFile& file,
                                          bool hovered) const
{
    const auto& token = antTheme->tokens();

    if (hovered)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorFillTertiary);
        painter->drawRect(itemRect);
    }

    const int thumbSize = 40;
    const int thumbLeft = 4;
    const int thumbTop = itemRect.top() + (itemRect.height() - thumbSize) / 2;
    const QRect thumbRect(thumbLeft, thumbTop, thumbSize, thumbSize);

    AntStyleBase::drawCrispRoundedRect(painter, thumbRect,
        QPen(token.colorBorderSecondary, 1), token.colorFillTertiary,
        token.borderRadiusSM, token.borderRadiusSM);

    if (!file.thumbUrl.isEmpty())
    {
        const QPixmap pixmap(file.thumbUrl);
        if (!pixmap.isNull())
        {
            painter->drawPixmap(thumbRect, pixmap);
        }
    }
    else
    {
        QFont iconFont = painter->font();
        iconFont.setPixelSize(16);
        painter->setFont(iconFont);
        painter->setPen(token.colorTextTertiary);
        painter->drawText(thumbRect, Qt::AlignCenter, fileIconText(file.name));
    }

    const int textLeft = thumbLeft + thumbSize + 8;
    const int textWidth = itemRect.width() - textLeft - 8;

    QFont nameFont = painter->font();
    nameFont.setPixelSize(token.fontSize);
    painter->setFont(nameFont);
    const QFontMetrics nameFm(nameFont);
    const QString displayName = truncatedName(file.name, textWidth, nameFm);
    painter->setPen(token.colorText);
    painter->drawText(QRect(textLeft, itemRect.top(), textWidth, itemRect.height() / 2),
                      Qt::AlignLeft | Qt::AlignBottom, displayName);

    if (file.status == Ant::UploadFileStatus::Uploading)
    {
        const int barY = itemRect.top() + itemRect.height() / 2 + 4;
        const int barH = 4;
        drawProgressBar(painter, QRect(textLeft, barY, textWidth, barH), file.percent);
    }
}

void AntUploadStyle::drawPictureCardItem(QPainter* painter, const QRect& cardRect, const AntUploadFile& file,
                                          bool hovered) const
{
    const auto& token = antTheme->tokens();
    const QRect cr = cardRect;

    AntStyleBase::drawCrispRoundedRect(painter, cr,
        QPen(token.colorBorderSecondary, 1), token.colorBgContainer,
        token.borderRadius, token.borderRadius);

    if (!file.thumbUrl.isEmpty())
    {
        const QPixmap pixmap(file.thumbUrl);
        if (!pixmap.isNull())
        {
            painter->save();
            QPainterPath clipPath;
            clipPath.addRoundedRect(cr, token.borderRadius, token.borderRadius);
            painter->setClipPath(clipPath);
            painter->drawPixmap(cardRect, pixmap);
            painter->restore();
        }
    }
    else
    {
        painter->setPen(token.colorTextTertiary);
        QFont iconFont = painter->font();
        iconFont.setPixelSize(24);
        painter->setFont(iconFont);
        painter->drawText(cardRect, Qt::AlignCenter, fileIconText(file.name));
    }

    const int overlayH = 24;
    const QRect overlayRect(cardRect.left(), cardRect.bottom() - overlayH + 1, cardRect.width(), overlayH);
    QColor overlayBg(0, 0, 0, 140);
    painter->setPen(Qt::NoPen);
    painter->setBrush(overlayBg);
    painter->drawRect(overlayRect);

    QFont nameFont = painter->font();
    nameFont.setPixelSize(token.fontSizeSM);
    painter->setFont(nameFont);
    const QFontMetrics fm(nameFont);
    const QString displayName = truncatedName(file.name, cardRect.width() - 8, fm);
    painter->setPen(Qt::white);
    painter->drawText(overlayRect.adjusted(4, 0, -4, 0), Qt::AlignLeft | Qt::AlignVCenter, displayName);

    if (file.status == Ant::UploadFileStatus::Uploading)
    {
        QColor mask(0, 0, 0, 120);
        AntStyleBase::drawCrispRoundedRect(painter, cr, Qt::NoPen, mask,
            token.borderRadius, token.borderRadius);

        const int spinnerSize = 28;
        const QRect spinnerRect(
            cardRect.center().x() - spinnerSize / 2,
            cardRect.center().y() - spinnerSize / 2,
            spinnerSize, spinnerSize);

        painter->setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(spinnerRect);

        QFont pctFont = painter->font();
        pctFont.setPixelSize(token.fontSizeSM);
        painter->setFont(pctFont);
        painter->setPen(Qt::white);
        painter->drawText(cardRect, Qt::AlignCenter, QString::number(file.percent) + QStringLiteral("%"));
    }

    if (hovered && file.status != Ant::UploadFileStatus::Uploading)
    {
        QColor hoverOverlay(0, 0, 0, 100);
        AntStyleBase::drawCrispRoundedRect(painter, cr, Qt::NoPen, hoverOverlay,
            token.borderRadius, token.borderRadius);

        const int actionIconSize = 18;
        const int actionGap = 16;
        const int actionY = cardRect.center().y() - actionIconSize / 2;

        const QPoint previewCenter(cardRect.center().x() - actionIconSize / 2 - actionGap / 2, actionY + actionIconSize / 2);
        const QPoint deleteCenter(cardRect.center().x() + actionIconSize / 2 + actionGap / 2, actionY + actionIconSize / 2);

        drawEyeIcon(painter, previewCenter, actionIconSize, Qt::white);
        drawDeleteIcon(painter, deleteCenter, actionIconSize, Qt::white);
    }

    if (file.status == Ant::UploadFileStatus::Done)
    {
        const int badgeSize = 18;
        const QRect badgeRect(cardRect.right() - badgeSize, cardRect.top(), badgeSize, badgeSize);
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorSuccess);
        painter->drawEllipse(badgeRect);
        drawCheckIcon(painter, badgeRect.center(), badgeSize - 6);
    }
    else if (file.status == Ant::UploadFileStatus::Error)
    {
        const int badgeSize = 18;
        const QRect badgeRect(cardRect.right() - badgeSize, cardRect.top(), badgeSize, badgeSize);
        painter->setPen(Qt::NoPen);
        painter->setBrush(token.colorError);
        painter->drawEllipse(badgeRect);
        drawErrorIcon(painter, badgeRect.center(), badgeSize - 6);
    }
}

void AntUploadStyle::drawProgressBar(QPainter* painter, const QRect& rect, int percent) const
{
    const auto& token = antTheme->tokens();
    const int radius = rect.height() / 2;

    AntStyleBase::drawCrispRoundedRect(painter, rect, Qt::NoPen, token.colorBorderSecondary, radius, radius);

    if (percent > 0)
    {
        const int fillWidth = rect.width() * qBound(0, percent, 100) / 100;
        const QRect fillRect(rect.left(), rect.top(), fillWidth, rect.height());
        AntStyleBase::drawCrispRoundedRect(painter, fillRect, Qt::NoPen, token.colorPrimary, radius, radius);
    }
}

void AntUploadStyle::drawCheckIcon(QPainter* painter, const QPoint& center, int size) const
{
    const auto& token = antTheme->tokens();
    painter->setPen(QPen(token.colorSuccess, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    const int half = size / 2;
    const int quarter = size / 4;
    painter->drawLine(center.x() - half + 2, center.y(),
                      center.x() - quarter + 1, center.y() + half - 2);
    painter->drawLine(center.x() - quarter + 1, center.y() + half - 2,
                      center.x() + half - 2, center.y() - half + 2);
}

void AntUploadStyle::drawErrorIcon(QPainter* painter, const QPoint& center, int size) const
{
    const auto& token = antTheme->tokens();
    painter->setPen(QPen(token.colorError, 2, Qt::SolidLine, Qt::RoundCap));
    const int half = size / 2 - 1;
    painter->drawLine(center.x() - half, center.y() - half, center.x() + half, center.y() + half);
    painter->drawLine(center.x() + half, center.y() - half, center.x() - half, center.y() + half);
}

void AntUploadStyle::drawCloseIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const
{
    painter->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap));
    const int half = size / 2 - 2;
    painter->drawLine(center.x() - half, center.y() - half, center.x() + half, center.y() + half);
    painter->drawLine(center.x() + half, center.y() - half, center.x() - half, center.y() + half);
}

void AntUploadStyle::drawPlusIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const
{
    painter->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap));
    const int half = size / 2;
    painter->drawLine(center.x() - half, center.y(), center.x() + half, center.y());
    painter->drawLine(center.x(), center.y() - half, center.x(), center.y() + half);
}

void AntUploadStyle::drawUploadIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const
{
    painter->setPen(QPen(color, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    const int half = size / 2;
    const int trayY = center.y() + half / 3;
    painter->drawLine(center.x() - half + 3, trayY, center.x() + half - 3, trayY);
    painter->drawLine(center.x() - half + 3, trayY, center.x() - half + 3, trayY - half / 3);
    painter->drawLine(center.x() + half - 3, trayY, center.x() + half - 3, trayY - half / 3);
    painter->drawLine(center.x(), center.y() + half / 3, center.x(), center.y() - half + 3);
    painter->drawLine(center.x(), center.y() - half + 3, center.x() - 5, center.y() - half + 8);
    painter->drawLine(center.x(), center.y() - half + 3, center.x() + 5, center.y() - half + 8);
}

void AntUploadStyle::drawInboxIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const
{
    painter->setPen(QPen(color, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);

    const int half = size / 2;
    QPainterPath path;
    path.moveTo(center.x() - half + 4, center.y() + half - 5);
    path.lineTo(center.x() - half + 8, center.y() - 2);
    path.lineTo(center.x() - 5, center.y() - 2);
    path.lineTo(center.x() - 2, center.y() + 4);
    path.lineTo(center.x() + 2, center.y() + 4);
    path.lineTo(center.x() + 5, center.y() - 2);
    path.lineTo(center.x() + half - 8, center.y() - 2);
    path.lineTo(center.x() + half - 4, center.y() + half - 5);
    path.lineTo(center.x() + half - 5, center.y() + half - 2);
    path.lineTo(center.x() - half + 5, center.y() + half - 2);
    path.closeSubpath();
    painter->drawPath(path);
}

void AntUploadStyle::drawPaperClipIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const
{
    painter->setPen(QPen(color, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);

    const qreal half = size / 2.0;
    QPainterPath path;
    path.moveTo(center.x() - half * 0.25, center.y() + half * 0.55);
    path.lineTo(center.x() + half * 0.42, center.y() - half * 0.12);
    path.quadTo(center.x() + half * 0.72, center.y() - half * 0.42,
                center.x() + half * 0.42, center.y() - half * 0.72);
    path.quadTo(center.x() + half * 0.12, center.y() - half * 1.02,
                center.x() - half * 0.18, center.y() - half * 0.72);
    path.lineTo(center.x() - half * 0.76, center.y() - half * 0.14);
    path.quadTo(center.x() - half * 1.04, center.y() + half * 0.14,
                center.x() - half * 0.76, center.y() + half * 0.42);
    path.quadTo(center.x() - half * 0.48, center.y() + half * 0.70,
                center.x() - half * 0.20, center.y() + half * 0.42);
    path.lineTo(center.x() + half * 0.34, center.y() - half * 0.12);
    painter->drawPath(path);
}

void AntUploadStyle::drawLoadingIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const
{
    painter->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);
    const QRectF arcRect(center.x() - size / 2.0, center.y() - size / 2.0, size, size);
    painter->drawArc(arcRect, 30 * 16, 270 * 16);
}

void AntUploadStyle::drawFileIcon(QPainter* painter, const QRect& rect, const QColor& color) const
{
    const auto& token = antTheme->tokens();
    const int radius = token.borderRadiusXS;

    AntStyleBase::drawCrispRoundedRect(painter, rect, Qt::NoPen, token.colorPrimaryBg, radius, radius);

    const QRect foldRect(rect.right() - rect.width() / 3, rect.top(), rect.width() / 3, rect.height() / 3);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    QPainterPath foldPath;
    foldPath.moveTo(foldRect.bottomLeft());
    foldPath.lineTo(QPoint(foldRect.left(), foldRect.top()));
    foldPath.lineTo(QPoint(foldRect.right(), foldRect.top()));
    foldPath.lineTo(foldRect.bottomRight());
    foldPath.closeSubpath();
    painter->drawPath(foldPath);

    AntStyleBase::drawCrispRoundedRect(painter, rect.adjusted(0, rect.height() / 3 - 1, 0, -2),
        QPen(color, 1.2), Qt::NoBrush, radius, radius);
}

void AntUploadStyle::drawEyeIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const
{
    const int half = size / 2;
    const int eyeW = half;
    const int eyeH = half / 2 + 1;

    painter->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);

    QPainterPath eyePath;
    eyePath.moveTo(center.x() - eyeW, center.y());
    eyePath.quadTo(center.x(), center.y() - eyeH * 2, center.x() + eyeW, center.y());
    eyePath.quadTo(center.x(), center.y() + eyeH * 2, center.x() - eyeW, center.y());
    painter->drawPath(eyePath);

    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(center, 2, 2);
}

void AntUploadStyle::drawDeleteIcon(QPainter* painter, const QPoint& center, int size, const QColor& color) const
{
    const int half = size / 2;
    painter->setPen(QPen(color, 1.5, Qt::SolidLine, Qt::RoundCap));
    painter->setBrush(Qt::NoBrush);

    const int topY = center.y() - half + 2;
    const int bottomY = center.y() + half - 2;
    const int leftX = center.x() - half + 2;
    const int rightX = center.x() + half - 2;

    painter->drawLine(leftX, topY, rightX, topY);
    painter->drawLine(center.x() - 1, topY, center.x() - 1, bottomY);
    painter->drawLine(center.x() + 1, topY, center.x() + 1, bottomY);
    painter->drawLine(leftX - 1, topY + 2, rightX + 1, topY + 2);

    QPainterPath binPath;
    const int binTop = topY + 4;
    binPath.moveTo(leftX - 1, binTop);
    binPath.lineTo(leftX + 1, bottomY);
    binPath.lineTo(rightX - 1, bottomY);
    binPath.lineTo(rightX + 1, binTop);
    binPath.closeSubpath();
    painter->drawPath(binPath);
}

QString AntUploadStyle::fileIconText(const QString& fileName)
{
    const QString suffix = fileName.section(QLatin1Char('.'), -1).toLower();
    if (suffix == QStringLiteral("pdf"))
    {
        return QStringLiteral("PDF");
    }
    if (suffix == QStringLiteral("doc") || suffix == QStringLiteral("docx"))
    {
        return QStringLiteral("DOC");
    }
    if (suffix == QStringLiteral("xls") || suffix == QStringLiteral("xlsx"))
    {
        return QStringLiteral("XLS");
    }
    if (suffix == QStringLiteral("zip") || suffix == QStringLiteral("rar") || suffix == QStringLiteral("7z"))
    {
        return QStringLiteral("ZIP");
    }
    if (suffix == QStringLiteral("mp4") || suffix == QStringLiteral("avi") || suffix == QStringLiteral("mov"))
    {
        return QStringLiteral("VID");
    }
    if (suffix == QStringLiteral("mp3") || suffix == QStringLiteral("wav") || suffix == QStringLiteral("flac"))
    {
        return QStringLiteral("AUD");
    }
    if (suffix == QStringLiteral("txt") || suffix == QStringLiteral("md"))
    {
        return QStringLiteral("TXT");
    }
    return QStringLiteral("FILE");
}

QString AntUploadStyle::truncatedName(const QString& name, int maxWidth, const QFontMetrics& fm)
{
    if (fm.horizontalAdvance(name) <= maxWidth)
    {
        return name;
    }
    return fm.elidedText(name, Qt::ElideMiddle, maxWidth);
}
