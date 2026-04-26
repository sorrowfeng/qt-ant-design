#include "AntQRCodeStyle.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include <cmath>

#include "core/AntStyleBase.h"
#include "widgets/AntQRCode.h"

namespace
{

int qrModuleCount(const AntQRCode* qr)
{
    const auto& matrix = qr->qrMatrix();
    return matrix.isEmpty() ? 0 : matrix.size();
}

qreal qrModuleSize(const AntQRCode* qr, const QRect& contentRect)
{
    const int count = qrModuleCount(qr);
    if (count == 0) return 0;
    return static_cast<qreal>(contentRect.width()) / count;
}

QRect qrContentRect(const AntQRCode* qr, const QRect& widgetRect)
{
    const int borderPadding = qr->isBordered() ? 12 : 0;
    return widgetRect.adjusted(borderPadding, borderPadding, -borderPadding, -borderPadding);
}

void drawQRMatrix(QPainter* painter, const AntQRCode* qr, const QRect& contentRect)
{
    const auto& matrix = qr->qrMatrix();
    const int count = matrix.size();
    if (count == 0) return;

    const qreal moduleSize = qrModuleSize(qr, contentRect);
    const auto& token = antTheme->tokens();

    QColor fg = qr->color();
    if (!fg.isValid()) fg = token.colorText;

    for (int r = 0; r < count; ++r)
    {
        for (int c = 0; c < count; ++c)
        {
            if (matrix[r][c])
            {
                QRectF moduleRect(contentRect.x() + c * moduleSize,
                                  contentRect.y() + r * moduleSize,
                                  moduleSize, moduleSize);
                painter->fillRect(moduleRect.adjusted(0.5, 0.5, -0.5, -0.5), fg);
            }
        }
    }
}

void drawQRIcon(QPainter* painter, const AntQRCode* qr, const QRect& contentRect)
{
    const auto& icon = qr->icon();
    if (icon.isNull()) return;

    const int iconSize = qr->iconSize();
    const auto& token = antTheme->tokens();
    const QPoint center = contentRect.center();
    const int halfBg = iconSize / 2 + 4;
    const QRect bgRect(center.x() - halfBg, center.y() - halfBg, halfBg * 2, halfBg * 2);

    AntStyleBase::drawCrispRoundedRect(painter, bgRect, Qt::NoPen, token.colorBgContainer, 4, 4);

    const QRect iconRect(center.x() - iconSize / 2, center.y() - iconSize / 2, iconSize, iconSize);
    icon.paint(painter, iconRect, Qt::AlignCenter);
}

void drawQRStatusOverlay(QPainter* painter, const AntQRCode* qr)
{
    const auto status = qr->status();
    if (status == Ant::QRCodeStatus::Active) return;

    const auto& token = antTheme->tokens();
    const QRectF r = qr->rect();

    // Semi-transparent overlay
    QColor overlay = token.colorBgContainer;
    overlay.setAlphaF(0.85);
    painter->fillRect(r, overlay);

    QFont f = painter->font();
    f.setPixelSize(token.fontSize);
    painter->setFont(f);

    switch (status)
    {
    case Ant::QRCodeStatus::Expired:
    {
        QColor textCol = token.colorTextSecondary;
        painter->setPen(textCol);
        painter->drawText(r.adjusted(0, -20, 0, -20), Qt::AlignCenter, QStringLiteral("Expired"));

        // Refresh button area
        QPoint center = r.center().toPoint();
        QRectF refreshBtn(center.x() - 28, center.y() + 4, 56, 28);
        AntStyleBase::drawCrispRoundedRect(painter, refreshBtn.toRect(),
            QPen(token.colorPrimary, 1), Qt::NoBrush, 4, 4);
        painter->setPen(token.colorPrimary);
        f.setPixelSize(token.fontSizeSM);
        painter->setFont(f);
        painter->drawText(refreshBtn, Qt::AlignCenter, QStringLiteral("Refresh"));
        break;
    }
    case Ant::QRCodeStatus::Loading:
    {
        painter->setPen(token.colorPrimary);
        painter->drawText(r.adjusted(0, 16, 0, 16), Qt::AlignCenter, QStringLiteral("Loading..."));

        // Simple spinning dots
        for (int i = 0; i < 8; ++i)
        {
            qreal angle = 2.0 * M_PI * i / 8.0;
            qreal x = r.center().x() + 16 * qCos(angle);
            qreal y = r.center().y() - 10 + 16 * qSin(angle);
            qreal alpha = 0.15 + 0.85 * (1.0 - static_cast<qreal>(i) / 8.0);
            QColor dotColor = token.colorPrimary;
            dotColor.setAlphaF(alpha);
            painter->setPen(Qt::NoPen);
            painter->setBrush(dotColor);
            painter->drawEllipse(QPointF(x, y), 3, 3);
        }
        break;
    }
    case Ant::QRCodeStatus::Scanned:
    {
        QPoint center = r.center().toPoint();
        // Checkmark
        painter->setPen(QPen(token.colorSuccess, 3, Qt::SolidLine, Qt::RoundCap));
        painter->setBrush(Qt::NoBrush);
        QPoint p1(center.x() - 12, center.y() - 4);
        QPoint p2(center.x() - 2, center.y() + 8);
        QPoint p3(center.x() + 14, center.y() - 10);
        QPainterPath path;
        path.moveTo(p1);
        path.lineTo(p2);
        path.lineTo(p3);
        painter->drawPath(path);

        painter->setPen(token.colorTextSecondary);
        f.setPixelSize(token.fontSize);
        painter->setFont(f);
        painter->drawText(r.adjusted(0, 24, 0, 24), Qt::AlignCenter, QStringLiteral("Scanned"));
        break;
    }
    default:
        break;
    }
}

} // namespace

AntQRCodeStyle::AntQRCodeStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntQRCode>();
}

void AntQRCodeStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntQRCode*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntQRCodeStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntQRCode*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntQRCodeStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntQRCode*>(widget))
    {
        drawQRCode(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntQRCodeStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntQRCodeStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* qr = qobject_cast<AntQRCode*>(watched);
    if (qr && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(qr);
        option.rect = qr->rect();
        QPainter painter(qr);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, qr);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntQRCodeStyle::drawQRCode(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* qr = qobject_cast<const AntQRCode*>(widget);
    if (!qr || !painter || !option) return;

    const auto& token = antTheme->tokens();
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);

    const QRectF r = option->rect;

    // Background
    QColor bg = qr->bgColor();
    if (!bg.isValid()) bg = token.colorBgContainer;
    painter->fillRect(r, bg);

    if (qr->value().isEmpty())
    {
        // Placeholder: dashed border
        QPen dashPen(token.colorBorder, 1, Qt::DashLine);
        AntStyleBase::drawCrispRoundedRect(painter, r.adjusted(4, 4, -4, -4).toRect(),
            dashPen, Qt::NoBrush, 8, 8);
        painter->restore();
        return;
    }

    // QR modules
    QRect contentRect = qrContentRect(qr, option->rect);
    drawQRMatrix(painter, qr, contentRect);

    // Center icon
    drawQRIcon(painter, qr, contentRect);

    // Border
    if (qr->isBordered())
    {
        int borderPad = (qr->qrSize() - contentRect.width()) / 2 - 2;
        AntStyleBase::drawCrispRoundedRect(painter, r.adjusted(borderPad, borderPad, -borderPad, -borderPad).toRect(),
            QPen(token.colorBorder, 1), Qt::NoBrush, 4, 4);
    }

    // Status overlay
    drawQRStatusOverlay(painter, qr);

    painter->restore();
}
