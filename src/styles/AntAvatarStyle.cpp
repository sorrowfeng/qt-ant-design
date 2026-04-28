#include "AntAvatarStyle.h"

#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include <algorithm>

#include "widgets/AntAvatar.h"

namespace
{

int avatarExtent(const AntAvatar* avatar)
{
    if (avatar->customSize() > 0)
    {
        return avatar->customSize();
    }
    const auto& token = antTheme->tokens();
    switch (avatar->avatarSize())
    {
    case Ant::Size::Large:
        return token.controlHeightLG;
    case Ant::Size::Small:
        return token.controlHeightSM;
    case Ant::Size::Middle:
        return token.controlHeight;
    }
    return token.controlHeight;
}

int avatarTextFontSize(const AntAvatar* avatar)
{
    const auto& token = antTheme->tokens();
    if (avatar->avatarSize() == Ant::Size::Small)
    {
        return token.fontSizeSM;
    }
    return token.fontSize;
}

int avatarIconFontSize(const AntAvatar* avatar)
{
    const auto& token = antTheme->tokens();
    if (avatar->avatarSize() == Ant::Size::Large || avatar->customSize() >= token.controlHeightLG)
    {
        return token.fontSizeXL;
    }
    if (avatar->avatarSize() == Ant::Size::Small && avatar->customSize() == 0)
    {
        return token.fontSize;
    }
    return (token.fontSizeLG + token.fontSizeXL) / 2;
}

QPainterPath avatarClipPath(const AntAvatar* avatar, const QRectF& rect)
{
    QPainterPath path;
    if (avatar->shape() == Ant::AvatarShape::Circle)
    {
        path.addEllipse(rect);
    }
    else
    {
        path.addRoundedRect(rect, antTheme->tokens().borderRadius, antTheme->tokens().borderRadius);
    }
    return path;
}

QRectF avatarImageSourceRect(const QPixmap& pixmap, const QSizeF& targetSize)
{
    const QSizeF source = pixmap.size();
    const qreal sourceRatio = source.width() / source.height();
    const qreal targetRatio = targetSize.width() / targetSize.height();
    if (sourceRatio > targetRatio)
    {
        const qreal width = source.height() * targetRatio;
        return QRectF((source.width() - width) / 2.0, 0, width, source.height());
    }
    const qreal height = source.width() / targetRatio;
    return QRectF(0, (source.height() - height) / 2.0, source.width(), height);
}

void drawAvatarUserIcon(QPainter* painter, const QRectF& rect, const QColor& color)
{
    const qreal size = qMin(rect.width(), rect.height()) * 0.72;
    const QPointF center = rect.center();
    painter->save();
    painter->setPen(QPen(color, qMax<qreal>(1.7, size / 16.0), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawEllipse(QRectF(center.x() - size * 0.16, center.y() - size * 0.30, size * 0.32, size * 0.32));

    QPainterPath shoulders;
    shoulders.moveTo(center.x() - size * 0.28, center.y() + size * 0.24);
    shoulders.cubicTo(center.x() - size * 0.24, center.y() + size * 0.02,
                      center.x() + size * 0.24, center.y() + size * 0.02,
                      center.x() + size * 0.28, center.y() + size * 0.24);
    painter->drawPath(shoulders);
    painter->restore();
}

void drawAvatarBellIcon(QPainter* painter, const QRectF& rect, const QColor& color)
{
    const qreal size = qMin(rect.width(), rect.height()) * 0.72;
    const QPointF center = rect.center();
    painter->save();
    painter->setPen(QPen(color, qMax<qreal>(1.5, size / 18.0), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(Qt::NoBrush);
    QPainterPath bell;
    bell.moveTo(center.x() - size * 0.26, center.y() + size * 0.12);
    bell.lineTo(center.x() + size * 0.26, center.y() + size * 0.12);
    bell.cubicTo(center.x() + size * 0.18, center.y() - size * 0.02,
                 center.x() + size * 0.20, center.y() - size * 0.28,
                 center.x(), center.y() - size * 0.28);
    bell.cubicTo(center.x() - size * 0.20, center.y() - size * 0.28,
                 center.x() - size * 0.18, center.y() - size * 0.02,
                 center.x() - size * 0.26, center.y() + size * 0.12);
    painter->drawPath(bell);
    painter->drawLine(QPointF(center.x() - size * 0.10, center.y() + size * 0.22),
                      QPointF(center.x() + size * 0.10, center.y() + size * 0.22));
    painter->restore();
}

} // namespace

AntAvatarStyle::AntAvatarStyle(QStyle* style)
    : AntStyleBase(style)
{
    connectThemeUpdate<AntAvatar>();
}

void AntAvatarStyle::polish(QWidget* widget)
{
    QProxyStyle::polish(widget);
    if (qobject_cast<AntAvatar*>(widget))
    {
        widget->installEventFilter(this);
        widget->setAttribute(Qt::WA_Hover);
    }
}

void AntAvatarStyle::unpolish(QWidget* widget)
{
    if (qobject_cast<AntAvatar*>(widget))
    {
        widget->removeEventFilter(this);
    }
    QProxyStyle::unpolish(widget);
}

void AntAvatarStyle::drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    if (element == QStyle::PE_Widget && qobject_cast<const AntAvatar*>(widget))
    {
        drawAvatar(option, painter, widget);
        return;
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QSize AntAvatarStyle::sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

bool AntAvatarStyle::eventFilter(QObject* watched, QEvent* event)
{
    auto* avatar = qobject_cast<AntAvatar*>(watched);
    if (avatar && event->type() == QEvent::Paint)
    {
        QStyleOption option;
        option.initFrom(avatar);
        option.rect = avatar->rect();
        QPainter painter(avatar);
        drawPrimitive(QStyle::PE_Widget, &option, &painter, avatar);
        return true;
    }
    return QProxyStyle::eventFilter(watched, event);
}

void AntAvatarStyle::drawAvatar(const QStyleOption* option, QPainter* painter, const QWidget* widget) const
{
    const auto* avatar = qobject_cast<const AntAvatar*>(widget);
    if (!avatar || !painter || !option)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    const QRectF avatarRect = QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5);

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const QPainterPath path = avatarClipPath(avatar, avatarRect);

    // Load pixmap from imagePath (replicating m_pixmap access)
    const QString imagePath = avatar->imagePath();
    QPixmap pixmap;
    if (!imagePath.isEmpty())
    {
        pixmap = QPixmap(imagePath);
    }

    if (!pixmap.isNull())
    {
        painter->save();
        painter->setClipPath(path);
        painter->drawPixmap(avatarRect, pixmap, avatarImageSourceRect(pixmap, avatarRect.size()));
        painter->restore();
        painter->restore();
        return;
    }

    const bool enabled = option->state & QStyle::State_Enabled;
    QColor avatarBg = avatar->backgroundColor();
    if (!avatarBg.isValid())
    {
        avatarBg = token.colorTextPlaceholder;
    }
    painter->setPen(Qt::NoPen);
    painter->setBrush(enabled ? avatarBg : token.colorBgContainerDisabled);
    painter->drawPath(path);

    const QString iconText = avatar->iconText();
    const QString text = avatar->text();
    const QString content = !iconText.isEmpty() ? iconText : text;
    if (content.isEmpty())
    {
        painter->restore();
        return;
    }

    QFont f = painter->font();
    f.setPixelSize(!iconText.isEmpty() ? avatarIconFontSize(avatar) : avatarTextFontSize(avatar));
    f.setWeight(QFont::DemiBold);
    if (iconText.isEmpty())
    {
        const int gap = avatar->gap();
        const int available = std::max(1, option->rect.width() - gap * 2);
        const int textWidth = QFontMetrics(f).horizontalAdvance(content);
        if (textWidth > available)
        {
            const qreal scale = static_cast<qreal>(available) / textWidth;
            f.setPixelSize(std::max(8, static_cast<int>(f.pixelSize() * scale)));
        }
    }

    painter->setFont(f);
    painter->setPen(enabled ? token.colorTextLightSolid : token.colorTextDisabled);
    if (!iconText.isEmpty() && iconText.compare(QStringLiteral("user"), Qt::CaseInsensitive) == 0)
    {
        drawAvatarUserIcon(painter, avatarRect, enabled ? token.colorTextLightSolid : token.colorTextDisabled);
        painter->restore();
        return;
    }
    if (!iconText.isEmpty() && iconText.compare(QStringLiteral("bell"), Qt::CaseInsensitive) == 0)
    {
        drawAvatarBellIcon(painter, avatarRect, enabled ? token.colorTextLightSolid : token.colorTextDisabled);
        painter->restore();
        return;
    }
    painter->drawText(avatarRect, Qt::AlignCenter, content.left(4));

    painter->restore();
}
