#include "AntAvatarStyle.h"

#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include <algorithm>

#include "core/AntTheme.h"
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
    case Ant::AvatarSize::Large:
        return token.controlHeightLG;
    case Ant::AvatarSize::Small:
        return token.controlHeightSM;
    case Ant::AvatarSize::Middle:
        return token.controlHeight;
    }
    return token.controlHeight;
}

int avatarTextFontSize(const AntAvatar* avatar)
{
    const auto& token = antTheme->tokens();
    if (avatar->avatarSize() == Ant::AvatarSize::Small)
    {
        return token.fontSizeSM;
    }
    return token.fontSize;
}

int avatarIconFontSize(const AntAvatar* avatar)
{
    const auto& token = antTheme->tokens();
    if (avatar->avatarSize() == Ant::AvatarSize::Large || avatar->customSize() >= token.controlHeightLG)
    {
        return token.fontSizeXL;
    }
    if (avatar->avatarSize() == Ant::AvatarSize::Small && avatar->customSize() == 0)
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

} // namespace

AntAvatarStyle::AntAvatarStyle(QStyle* style)
    : QProxyStyle(style)
{
    connect(antTheme, &AntTheme::themeModeChanged, this, [this](Ant::ThemeMode) {
        const auto widgets = QApplication::allWidgets();
        for (QWidget* w : widgets)
        {
            if (qobject_cast<AntAvatar*>(w) && w->style() == this)
            {
                unpolish(w);
                polish(w);
                w->updateGeometry();
                w->update();
            }
        }
    });
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
    painter->setPen(Qt::NoPen);
    painter->setBrush(enabled ? token.colorTextPlaceholder : token.colorBgContainerDisabled);
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
    painter->drawText(avatarRect, Qt::AlignCenter, content.left(4));

    painter->restore();
}
