#include "AntList.h"

#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QWheelEvent>

#include "../styles/AntListStyle.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"
#include "styles/AntPalette.h"
#include "widgets/AntIcon.h"

namespace
{
const AntList* parentListForItem(const QWidget* widget)
{
    return widget ? qobject_cast<const AntList*>(widget->parentWidget()) : nullptr;
}

int listItemPaddingV(const AntList* list)
{
    const auto& token = antTheme->tokens();
    if (list)
    {
        switch (list->listSize())
        {
        case AntList::Small:
            return token.paddingXS;
        case AntList::Large:
            return token.padding;
        default:
            break;
        }
    }
    return token.paddingSM;
}

int listItemPaddingH(const AntList* list)
{
    const auto& token = antTheme->tokens();
    return list && list->isBordered() ? token.paddingLG : 0;
}

int tokenLineHeight(int fontSize)
{
    return qRound(fontSize * antTheme->tokens().lineHeight);
}
} // namespace

// ─── AntListItemMeta ───

AntListItemMeta::AntListItemMeta(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        update();
    });
}

void AntListItemMeta::setAvatar(QWidget* widget)
{
    if (m_avatar == widget)
    {
        return;
    }
    if (m_avatar)
    {
        m_avatar->setParent(nullptr);
    }
    m_avatar = widget;
    if (m_avatar)
    {
        m_avatar->setParent(this);
        m_avatar->show();
    }
    syncAvatarGeometry();
    updateGeometry();
    update();
}

QWidget* AntListItemMeta::avatar() const { return m_avatar.data(); }

void AntListItemMeta::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    updateGeometry();
    update();
}

QString AntListItemMeta::title() const { return m_title; }

void AntListItemMeta::setDescription(const QString& description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    updateGeometry();
    update();
}

QString AntListItemMeta::description() const { return m_description; }

QSize AntListItemMeta::sizeHint() const
{
    const auto& token = antTheme->tokens();
    const int avatarSpace = m_avatar ? m_avatar->sizeHint().width() + token.padding : 0;

    QFont titleFont = font();
    titleFont.setPixelSize(token.fontSize);
    titleFont.setWeight(QFont::DemiBold);
    QFontMetrics titleFm(titleFont);

    QFont descFont = font();
    descFont.setPixelSize(token.fontSize);
    QFontMetrics descFm(descFont);

    int height = tokenLineHeight(token.fontSize);
    if (!m_description.isEmpty())
    {
        height += token.paddingXXS + tokenLineHeight(token.fontSize);
    }

    int width = avatarSpace;
    width += qMax(titleFm.horizontalAdvance(m_title),
                  m_description.isEmpty() ? 0 : descFm.horizontalAdvance(m_description));
    width = qMax(width, 80);

    return QSize(width, qMax(height, m_avatar ? m_avatar->sizeHint().height() : 0));
}

void AntListItemMeta::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    const QRect tr = textRect();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont titleFont = painter.font();
    titleFont.setPixelSize(token.fontSize);
    titleFont.setWeight(QFont::DemiBold);
    painter.setFont(titleFont);
    painter.setPen(token.colorText);

    QFontMetrics titleFm(titleFont);
    const int titleLineHeight = tokenLineHeight(token.fontSize);
    const int titleBaseline = tr.top() + (titleLineHeight - titleFm.height()) / 2 + titleFm.ascent();
    painter.drawText(tr.left(), titleBaseline, m_title);

    if (!m_description.isEmpty())
    {
        QFont descFont = painter.font();
        descFont.setPixelSize(token.fontSize);
        descFont.setWeight(QFont::Normal);
        painter.setFont(descFont);
        painter.setPen(token.colorTextSecondary);

        QFontMetrics descFm(descFont);
        const int descTop = tr.top() + titleLineHeight + token.paddingXXS;
        const int descBaseline = descTop + (tokenLineHeight(token.fontSize) - descFm.height()) / 2 + descFm.ascent();
        painter.drawText(tr.left(), descBaseline, m_description);
    }
}

void AntListItemMeta::resizeEvent(QResizeEvent* event)
{
    syncAvatarGeometry();
    QWidget::resizeEvent(event);
}

QRect AntListItemMeta::avatarRect() const
{
    if (!m_avatar)
    {
        return {};
    }
    const QSize sz = m_avatar->sizeHint();
    return QRect(0, (height() - sz.height()) / 2, sz.width(), sz.height());
}

QRect AntListItemMeta::textRect() const
{
    const int left = m_avatar ? avatarRect().right() + antTheme->tokens().padding : 0;
    return QRect(left, 0, width() - left, height());
}

void AntListItemMeta::syncAvatarGeometry()
{
    if (m_avatar)
    {
        m_avatar->setGeometry(avatarRect());
        m_avatar->show();
    }
}

// ─── AntListItem ───

AntListItem::AntListItem(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        update();
    });
}

void AntListItem::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    updateGeometry();
    update();
    Q_EMIT textChanged(m_text);
    Q_EMIT changed();
}

QString AntListItem::text() const { return m_text; }

void AntListItem::setIcon(const QIcon& icon)
{
    m_icon = icon;
    m_iconSource = m_icon.isNull() ? IconSource::None : IconSource::QtIcon;
    m_iconType = Ant::IconType::None;
    m_iconName.clear();
    m_iconPixmap = QPixmap();
    syncAntIconWidget();
    updateGeometry();
    update();
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

QIcon AntListItem::icon() const { return m_icon; }

void AntListItem::setIcon(Ant::IconType iconType, Ant::IconTheme theme)
{
    if (m_iconSource == IconSource::AntIconType && m_iconType == iconType && m_iconTheme == theme)
    {
        return;
    }
    m_iconSource = iconType == Ant::IconType::None ? IconSource::None : IconSource::AntIconType;
    m_icon = QIcon();
    m_iconType = iconType;
    m_iconName.clear();
    m_iconPixmap = QPixmap();
    m_iconTheme = theme;
    syncAntIconWidget();
    updateGeometry();
    update();
    Q_EMIT iconSizeChanged(m_iconSize);
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

Ant::IconType AntListItem::iconType() const { return m_iconType; }

void AntListItem::setIconName(const QString& iconName, Ant::IconTheme theme)
{
    if (m_iconSource == IconSource::AntIconName && m_iconName == iconName && m_iconTheme == theme)
    {
        return;
    }
    m_iconSource = iconName.isEmpty() ? IconSource::None : IconSource::AntIconName;
    m_icon = QIcon();
    m_iconName = iconName;
    m_iconType = Ant::IconType::None;
    m_iconPixmap = QPixmap();
    m_iconTheme = theme;
    syncAntIconWidget();
    updateGeometry();
    update();
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

QString AntListItem::iconName() const { return m_iconName; }

void AntListItem::setIconTheme(Ant::IconTheme theme)
{
    if (m_iconTheme == theme)
    {
        return;
    }
    m_iconTheme = theme;
    syncAntIconWidget();
    update();
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

Ant::IconTheme AntListItem::iconTheme() const { return m_iconTheme; }

void AntListItem::setIconColor(const QColor& color)
{
    if (m_iconColor == color)
    {
        return;
    }
    m_iconColor = color;
    syncAntIconWidget();
    update();
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

QColor AntListItem::iconColor() const { return m_iconColor; }

void AntListItem::setIconTwoToneColor(const QColor& color)
{
    if (m_iconTwoToneColor == color)
    {
        return;
    }
    m_iconTwoToneColor = color;
    syncAntIconWidget();
    update();
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

QColor AntListItem::iconTwoToneColor() const { return m_iconTwoToneColor; }

void AntListItem::setIconPixmap(const QPixmap& pixmap)
{
    if (m_iconSource == IconSource::Pixmap && m_iconPixmap.cacheKey() == pixmap.cacheKey())
    {
        return;
    }
    m_iconPixmap = pixmap;
    m_iconSource = m_iconPixmap.isNull() ? IconSource::None : IconSource::Pixmap;
    m_icon = QIcon();
    m_iconType = Ant::IconType::None;
    m_iconName.clear();
    syncAntIconWidget();
    updateGeometry();
    update();
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

QPixmap AntListItem::iconPixmap() const { return m_iconPixmap; }

void AntListItem::setIconImage(const QImage& image)
{
    setIconPixmap(image.isNull() ? QPixmap() : QPixmap::fromImage(image));
}

QImage AntListItem::iconImage() const
{
    return m_iconPixmap.isNull() ? QImage() : m_iconPixmap.toImage();
}

void AntListItem::setIconSize(const QSize& size)
{
    const QSize normalized(qMax(0, size.width()), qMax(0, size.height()));
    if (m_iconSize == normalized)
    {
        return;
    }
    m_iconSize = normalized;
    syncAntIconWidget();
    updateGeometry();
    update();
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

QSize AntListItem::iconSize() const { return m_iconSize; }

bool AntListItem::hasIcon() const
{
    switch (m_iconSource)
    {
    case IconSource::QtIcon:
        return !m_icon.isNull();
    case IconSource::AntIconType:
        return m_iconType != Ant::IconType::None;
    case IconSource::AntIconName:
        return !m_iconName.isEmpty();
    case IconSource::Pixmap:
        return !m_iconPixmap.isNull();
    case IconSource::None:
    default:
        return false;
    }
}

void AntListItem::clearIcon()
{
    if (!hasIcon() && m_iconSource == IconSource::None)
    {
        return;
    }
    m_iconSource = IconSource::None;
    m_icon = QIcon();
    m_iconType = Ant::IconType::None;
    m_iconName.clear();
    m_iconPixmap = QPixmap();
    syncAntIconWidget();
    updateGeometry();
    update();
    Q_EMIT iconChanged();
    Q_EMIT changed();
}

void AntListItem::setData(int role, const QVariant& value)
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        setText(value.toString());
        return;
    }
    if (role == Qt::DecorationRole && value.canConvert<QIcon>())
    {
        setIcon(value.value<QIcon>());
        return;
    }
    if (role == Qt::DecorationRole && value.canConvert<QPixmap>())
    {
        setIconPixmap(value.value<QPixmap>());
        return;
    }
    if (role == Qt::DecorationRole && value.canConvert<QImage>())
    {
        setIconImage(value.value<QImage>());
        return;
    }
    if (role == Qt::CheckStateRole)
    {
        setCheckState(static_cast<Qt::CheckState>(value.toInt()));
        return;
    }

    if (m_roleData.value(role) == value)
    {
        return;
    }
    m_roleData.insert(role, value);
    Q_EMIT dataChanged(role, value);
    Q_EMIT changed();
}

QVariant AntListItem::data(int role) const
{
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        return m_text;
    }
    if (role == Qt::DecorationRole)
    {
        switch (m_iconSource)
        {
        case IconSource::Pixmap:
            return QVariant::fromValue(m_iconPixmap);
        case IconSource::AntIconType:
            return QVariant::fromValue(static_cast<int>(m_iconType));
        case IconSource::AntIconName:
            return m_iconName;
        case IconSource::QtIcon:
        case IconSource::None:
        default:
            return QVariant::fromValue(m_icon);
        }
    }
    if (role == Qt::CheckStateRole)
    {
        return m_checkState;
    }
    return m_roleData.value(role);
}

void AntListItem::setCheckState(Qt::CheckState state)
{
    if (m_checkState == state)
    {
        return;
    }
    m_checkState = state;
    update();
    Q_EMIT checkStateChanged(m_checkState);
    Q_EMIT changed();
}

Qt::CheckState AntListItem::checkState() const { return m_checkState; }

void AntListItem::setFlags(Qt::ItemFlags flags)
{
    if (m_flags == flags)
    {
        return;
    }
    m_flags = flags;
    update();
    Q_EMIT flagsChanged(m_flags);
    Q_EMIT changed();
}

Qt::ItemFlags AntListItem::flags() const { return m_flags; }

void AntListItem::setSelected(bool selected)
{
    if (m_selected == selected)
    {
        return;
    }
    m_selected = selected;
    update();
    Q_EMIT selectedChanged(m_selected);
}

bool AntListItem::isSelected() const { return m_selected; }

void AntListItem::setMeta(AntListItemMeta* meta)
{
    if (m_meta == meta)
    {
        return;
    }
    if (m_meta)
    {
        m_meta->setParent(nullptr);
    }
    m_meta = meta;
    if (m_meta)
    {
        m_meta->setParent(this);
        m_meta->show();
    }
    syncLayout();
    updateGeometry();
    update();
}

AntListItemMeta* AntListItem::meta() const { return m_meta.data(); }

void AntListItem::setExtraWidget(QWidget* widget)
{
    if (m_extra == widget)
    {
        return;
    }
    if (m_extra)
    {
        m_extra->setParent(nullptr);
    }
    m_extra = widget;
    if (m_extra)
    {
        m_extra->setParent(this);
        m_extra->show();
    }
    syncLayout();
    updateGeometry();
    update();
}

QWidget* AntListItem::extraWidget() const { return m_extra.data(); }

void AntListItem::addActionWidget(QWidget* widget)
{
    if (!widget)
    {
        return;
    }
    widget->setParent(this);
    widget->show();
    m_actions.append(widget);
    syncLayout();
    updateGeometry();
    update();
}

QList<QWidget*> AntListItem::actionWidgets() const
{
    QList<QWidget*> result;
    for (const auto& a : m_actions)
    {
        if (a)
        {
            result.append(a.data());
        }
    }
    return result;
}

void AntListItem::setContentWidget(QWidget* widget)
{
    if (m_content == widget)
    {
        return;
    }
    if (m_content)
    {
        m_content->setParent(nullptr);
    }
    m_content = widget;
    if (m_content)
    {
        m_content->setParent(this);
        m_content->show();
    }
    syncLayout();
    updateGeometry();
    update();
}

QWidget* AntListItem::contentWidget() const { return m_content.data(); }

QSize AntListItem::sizeHint() const
{
    const auto& token = antTheme->tokens();
    const int paddingV = listItemPaddingV(parentListForItem(this));

    int contentHeight = tokenLineHeight(token.fontSize);
    if (m_meta)
    {
        contentHeight = m_meta->sizeHint().height();
    }
    if (!m_text.isEmpty())
    {
        contentHeight = qMax(contentHeight, tokenLineHeight(token.fontSize));
    }
    if (hasIcon())
    {
        contentHeight = qMax(contentHeight, effectiveIconSize().height());
    }
    if (m_content)
    {
        contentHeight = qMax(contentHeight, m_content->sizeHint().height());
    }
    if (m_extra)
    {
        contentHeight = qMax(contentHeight, m_extra->sizeHint().height());
    }
    for (const auto& a : m_actions)
    {
        if (a)
        {
            contentHeight = qMax(contentHeight, a->sizeHint().height());
        }
    }

    return QSize(200, contentHeight + paddingV * 2);
}

void AntListItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (m_selected)
    {
        const QRectF selectedRect = rect().adjusted(2, 2, -2, -2);
        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorPrimaryBg);
        painter.drawRoundedRect(selectedRect, token.borderRadiusSM, token.borderRadiusSM);
    }

    if (usesPaintedIcon())
    {
        drawPaintedIcon(&painter, leadingIconRect());
    }

    if (!m_text.isEmpty() && !m_meta && !m_content)
    {
        const auto* list = parentListForItem(this);
        const int paddingV = listItemPaddingV(list);
        const int paddingH = listItemPaddingH(list);
        const int checkSize = m_checkState == Qt::Unchecked ? 0 : 14;
        int x = paddingH + token.paddingSM;
        const int centerY = height() / 2;

        if (checkSize > 0)
        {
            const QRect checkRect(x, centerY - checkSize / 2, checkSize, checkSize);
            painter.setPen(QPen(token.colorBorder, token.lineWidth));
            painter.setBrush(m_checkState == Qt::Checked ? token.colorPrimary : token.colorBgContainer);
            painter.drawRoundedRect(checkRect, token.borderRadiusSM / 2.0, token.borderRadiusSM / 2.0);
            x += checkSize + token.paddingXS;
        }

        if (hasIcon())
        {
            const QSize iconSize = effectiveIconSize();
            x += iconSize.width() + token.paddingXS;
        }

        QFont textFont = painter.font();
        textFont.setPixelSize(token.fontSize);
        painter.setFont(textFont);
        painter.setPen(isEnabled() ? token.colorText : token.colorTextDisabled);
        painter.drawText(QRect(x,
                               paddingV,
                               qMax(0, width() - x - paddingH - token.paddingSM),
                               height() - paddingV * 2),
                         Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                         m_text);
    }

    if (m_actions.size() <= 1)
    {
        return;
    }

    painter.setPen(QPen(token.colorSplit, token.lineWidth));

    const QRect ar = actionsRect();
    int x = ar.left();
    for (int i = 0; i < m_actions.size() - 1; ++i)
    {
        const auto& action = m_actions.at(i);
        if (!action)
        {
            continue;
        }
        x += action->sizeHint().width() + token.paddingXS;
        const int lineHeight = qMax(1, tokenLineHeight(token.fontSize) - token.paddingXS);
        const int lineTop = ar.center().y() - lineHeight / 2;
        painter.drawLine(QPoint(x, lineTop), QPoint(x, lineTop + lineHeight));
        x += token.paddingXS;
    }
}

void AntListItem::resizeEvent(QResizeEvent* event)
{
    syncLayout();
    QWidget::resizeEvent(event);
}

QRect AntListItem::metaRect() const
{
    if (!m_meta)
    {
        return {};
    }
    const auto& token = antTheme->tokens();
    const auto* list = parentListForItem(this);
    const int paddingV = listItemPaddingV(list);
    const int paddingH = listItemPaddingH(list);
    const QSize sz = m_meta->sizeHint();
    int left = paddingH;
    if (hasIcon())
    {
        const QRect iconRect = leadingIconRect();
        left = iconRect.left() + iconRect.width() + token.paddingXS;
    }
    int right = width() - paddingH;
    if (m_extra)
    {
        right = extraRect().left() - token.padding;
    }
    if (!m_actions.isEmpty())
    {
        right = qMin(right, actionsRect().left() - token.paddingXL);
    }
    return QRect(left, paddingV, qMax(0, qMin(sz.width(), right - left)), height() - paddingV * 2);
}

QRect AntListItem::extraRect() const
{
    if (!m_extra)
    {
        return {};
    }
    const auto& token = antTheme->tokens();
    const auto* list = parentListForItem(this);
    const int paddingV = listItemPaddingV(list);
    const int paddingH = listItemPaddingH(list);
    const QSize sz = m_extra->sizeHint();
    return QRect(width() - paddingH - sz.width(), paddingV, sz.width(), height() - paddingV * 2);
}

QRect AntListItem::actionsRect() const
{
    if (m_actions.isEmpty())
    {
        return {};
    }
    const auto& token = antTheme->tokens();
    const auto* list = parentListForItem(this);
    const int paddingH = listItemPaddingH(list);
    const int h = tokenLineHeight(token.fontSize);
    const int w = actionsWidth();
    return QRect(width() - paddingH - w, (height() - h) / 2, w, h);
}

int AntListItem::actionsWidth() const
{
    const auto& token = antTheme->tokens();
    int width = 0;
    int count = 0;
    for (const auto& action : m_actions)
    {
        if (!action)
        {
            continue;
        }
        width += action->sizeHint().width();
        ++count;
    }
    if (count > 1)
    {
        width += (count - 1) * token.padding;
    }
    return width;
}

QRect AntListItem::leadingIconRect() const
{
    if (!hasIcon())
    {
        return {};
    }

    const auto& token = antTheme->tokens();
    const auto* list = parentListForItem(this);
    const int paddingH = listItemPaddingH(list);
    const int checkSize = m_checkState == Qt::Unchecked ? 0 : 14;
    int x = paddingH + token.paddingSM;
    if (checkSize > 0)
    {
        x += checkSize + token.paddingXS;
    }

    const QSize iconSize = effectiveIconSize();
    return QRect(x, height() / 2 - iconSize.height() / 2, iconSize.width(), iconSize.height());
}

void AntListItem::syncAntIconWidget()
{
    if (!usesAntIconWidget())
    {
        if (m_antIconWidget)
        {
            m_antIconWidget->hide();
        }
        return;
    }

    if (!m_antIconWidget)
    {
        m_antIconWidget = new AntIcon(this);
        m_antIconWidget->hide();
    }

    if (m_iconSource == IconSource::AntIconType)
    {
        m_antIconWidget->setIconType(m_iconType);
    }
    else
    {
        m_antIconWidget->setIconName(m_iconName);
    }
    m_antIconWidget->setIconTheme(m_iconTheme);
    m_antIconWidget->setColor(m_iconColor);
    m_antIconWidget->setTwoToneColor(m_iconTwoToneColor);

    const QRect iconRect = leadingIconRect();
    const int squareSize = qMax(1, qMin(iconRect.width(), iconRect.height()));
    m_antIconWidget->setIconSize(squareSize);
    if (!iconRect.isEmpty())
    {
        m_antIconWidget->setGeometry(iconRect);
        m_antIconWidget->show();
    }
}

bool AntListItem::usesAntIconWidget() const
{
    return m_iconSource == IconSource::AntIconType || m_iconSource == IconSource::AntIconName;
}

bool AntListItem::usesPaintedIcon() const
{
    return m_iconSource == IconSource::QtIcon || m_iconSource == IconSource::Pixmap;
}

QSize AntListItem::effectiveIconSize() const
{
    if (m_iconSize.isValid() && !m_iconSize.isEmpty())
    {
        return m_iconSize;
    }
    return QSize(16, 16);
}

void AntListItem::drawPaintedIcon(QPainter* painter, const QRect& iconRect) const
{
    if (!painter || iconRect.isEmpty() || !usesPaintedIcon())
    {
        return;
    }

    if (m_iconSource == IconSource::QtIcon)
    {
        m_icon.paint(painter, iconRect);
        return;
    }

    if (m_iconPixmap.isNull())
    {
        return;
    }
    const QPixmap scaled = m_iconPixmap.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const QPoint topLeft(iconRect.left() + (iconRect.width() - scaled.width()) / 2,
                         iconRect.top() + (iconRect.height() - scaled.height()) / 2);
    painter->drawPixmap(topLeft, scaled);
}

void AntListItem::syncLayout()
{
    syncAntIconWidget();

    if (m_meta)
    {
        m_meta->setGeometry(metaRect());
        m_meta->show();
    }
    if (m_extra)
    {
        m_extra->setGeometry(extraRect());
        m_extra->show();
    }
    if (m_content)
    {
        const auto* list = parentListForItem(this);
        const int paddingV = listItemPaddingV(list);
        const int paddingH = listItemPaddingH(list);
        const QRect mr = metaRect();
        const QRect er = m_extra ? extraRect() : QRect(width(), 0, 0, 0);
        int left = m_meta ? mr.right() + antTheme->tokens().padding : paddingH;
        if (!m_meta && hasIcon())
        {
            const QRect iconRect = leadingIconRect();
            left = iconRect.left() + iconRect.width() + antTheme->tokens().paddingXS;
        }
        int right = er.left() - antTheme->tokens().padding;
        if (!m_actions.isEmpty())
        {
            right = qMin(right, actionsRect().left() - antTheme->tokens().paddingXL);
        }
        m_content->setGeometry(QRect(left, paddingV, qMax(0, right - left), height() - paddingV * 2));
        m_content->show();
    }

    if (!m_actions.isEmpty())
    {
        const QRect ar = actionsRect();
        int x = ar.left();
        for (const auto& a : m_actions)
        {
            if (a)
            {
                const QSize sz = a->sizeHint();
                a->setGeometry(x, ar.top(), sz.width(), ar.height());
                a->show();
                x += sz.width() + antTheme->tokens().padding;
            }
        }
    }
}

// ─── AntList ───

AntList::AntList(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntListStyle>(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
}

bool AntList::isBordered() const { return m_bordered; }

void AntList::setBordered(bool bordered)
{
    if (m_bordered == bordered)
    {
        return;
    }
    m_bordered = bordered;
    update();
    Q_EMIT borderedChanged(m_bordered);
}

bool AntList::isSplit() const { return m_split; }

void AntList::setSplit(bool split)
{
    if (m_split == split)
    {
        return;
    }
    m_split = split;
    update();
    Q_EMIT splitChanged(m_split);
}

int AntList::listSize() const { return m_listSize; }

void AntList::setListSize(int size)
{
    if (m_listSize == size)
    {
        return;
    }
    m_listSize = size;
    syncLayout();
    updateGeometry();
    update();
    Q_EMIT listSizeChanged(m_listSize);
}

void AntList::setHeaderWidget(QWidget* widget)
{
    if (m_header == widget)
    {
        return;
    }
    if (m_header)
    {
        m_header->setParent(nullptr);
    }
    m_header = widget;
    if (m_header)
    {
        m_header->setParent(this);
        m_header->show();
    }
    syncLayout();
    updateGeometry();
    update();
}

QWidget* AntList::headerWidget() const { return m_header.data(); }

void AntList::setFooterWidget(QWidget* widget)
{
    if (m_footer == widget)
    {
        return;
    }
    if (m_footer)
    {
        m_footer->setParent(nullptr);
    }
    m_footer = widget;
    if (m_footer)
    {
        m_footer->setParent(this);
        m_footer->show();
    }
    syncLayout();
    updateGeometry();
    update();
}

QWidget* AntList::footerWidget() const { return m_footer.data(); }

void AntList::addItem(AntListItem* item)
{
    if (!item)
    {
        return;
    }
    adoptItem(item);
    m_items.append(item);
    syncLayout();
    updateGeometry();
    update();
}

void AntList::addItem(const QString& text)
{
    auto* listItem = new AntListItem;
    listItem->setText(text);
    addItem(listItem);
}

void AntList::addItems(const QStringList& labels)
{
    if (labels.isEmpty())
    {
        return;
    }
    for (const QString& label : labels)
    {
        auto* listItem = new AntListItem;
        listItem->setText(label);
        adoptItem(listItem);
        m_items.append(listItem);
    }
    syncLayout();
    updateGeometry();
    update();
    setProperty("antListLastBulkOperation", QStringLiteral("addItems"));
    setProperty("antListLastBulkItemCount", labels.size());
    setProperty("antListLastBulkLayoutCount", 1);
}

void AntList::insertItem(int index, AntListItem* item)
{
    if (!item)
    {
        return;
    }
    index = qBound(0, index, m_items.size());
    adoptItem(item);
    m_items.insert(index, item);
    syncLayout();
    updateGeometry();
    update();
}

void AntList::insertItem(int index, const QString& text)
{
    auto* listItem = new AntListItem;
    listItem->setText(text);
    insertItem(index, listItem);
}

void AntList::insertItems(int index, const QStringList& labels)
{
    if (labels.isEmpty())
    {
        return;
    }
    int insertIndex = qBound(0, index, m_items.size());
    for (const QString& label : labels)
    {
        auto* listItem = new AntListItem;
        listItem->setText(label);
        adoptItem(listItem);
        m_items.insert(insertIndex, listItem);
        ++insertIndex;
    }
    syncLayout();
    updateGeometry();
    update();
    setProperty("antListLastBulkOperation", QStringLiteral("insertItems"));
    setProperty("antListLastBulkItemCount", labels.size());
    setProperty("antListLastBulkLayoutCount", 1);
}

void AntList::removeItem(AntListItem* item)
{
    if (!item)
    {
        return;
    }
    if (m_items.removeAll(item) > 0)
    {
        detachItem(item);
    }
    syncLayout();
    updateGeometry();
    update();
}

int AntList::itemCount() const { return m_items.size(); }

int AntList::count() const { return itemCount(); }

bool AntList::isEmpty() const { return m_items.isEmpty(); }

AntListItem* AntList::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return nullptr;
    }
    return m_items.at(index).data();
}

AntListItem* AntList::item(int index) const
{
    return itemAt(index);
}

AntListItem* AntList::itemAt(const QPoint& pos) const
{
    for (const auto& item : m_items)
    {
        if (item && item->isVisible() && item->geometry().contains(pos))
        {
            return item.data();
        }
    }
    return nullptr;
}

QRect AntList::visualItemRect(AntListItem* item) const
{
    if (!item || item->parentWidget() != this)
    {
        return {};
    }
    return item->geometry();
}

int AntList::row(const AntListItem* item) const
{
    if (!item)
    {
        return -1;
    }
    for (int i = 0; i < m_items.size(); ++i)
    {
        if (m_items.at(i).data() == item)
        {
            return i;
        }
    }
    return -1;
}

QList<AntListItem*> AntList::findItems(const QString& text, Qt::MatchFlags flags) const
{
    QList<AntListItem*> result;
    const Qt::CaseSensitivity cs = flags.testFlag(Qt::MatchCaseSensitive) ? Qt::CaseSensitive : Qt::CaseInsensitive;
    const bool contains = flags.testFlag(Qt::MatchContains);
    const bool startsWith = flags.testFlag(Qt::MatchStartsWith);
    const bool endsWith = flags.testFlag(Qt::MatchEndsWith);
    const bool wildcard = flags.testFlag(Qt::MatchWildcard);
    bool regexp = flags.testFlag(Qt::MatchRegularExpression);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    regexp = regexp || flags.testFlag(Qt::MatchRegExp);
#endif
    QRegularExpression regularExpression;
    if (regexp)
    {
        regularExpression = QRegularExpression(text, cs == Qt::CaseSensitive
                                                       ? QRegularExpression::NoPatternOption
                                                       : QRegularExpression::CaseInsensitiveOption);
    }
    else if (wildcard)
    {
        regularExpression = QRegularExpression(QRegularExpression::wildcardToRegularExpression(text),
                                               cs == Qt::CaseSensitive
                                                   ? QRegularExpression::NoPatternOption
                                                   : QRegularExpression::CaseInsensitiveOption);
    }

    for (const auto& item : m_items)
    {
        if (!item)
        {
            continue;
        }

        const QString itemText = item->text();
        bool match = false;
        if (regexp || wildcard)
        {
            match = regularExpression.match(itemText).hasMatch();
        }
        else if (contains)
        {
            match = itemText.contains(text, cs);
        }
        else if (startsWith)
        {
            match = itemText.startsWith(text, cs);
        }
        else if (endsWith)
        {
            match = itemText.endsWith(text, cs);
        }
        else
        {
            match = itemText.compare(text, cs) == 0;
        }

        if (match)
        {
            result.append(item.data());
        }
    }
    return result;
}

void AntList::sortItems(Qt::SortOrder order)
{
    std::stable_sort(m_items.begin(), m_items.end(), [order](const QPointer<AntListItem>& left, const QPointer<AntListItem>& right) {
        const QString leftText = left ? left->text() : QString();
        const QString rightText = right ? right->text() : QString();
        const int cmp = QString::localeAwareCompare(leftText, rightText);
        return order == Qt::AscendingOrder ? cmp < 0 : cmp > 0;
    });
    syncLayout();
    updateGeometry();
    update();
}

AntListItem* AntList::takeItem(int index)
{
    if (index < 0 || index >= m_items.size())
    {
        return nullptr;
    }

    AntListItem* item = m_items.takeAt(index).data();
    if (item)
    {
        detachItem(item);
    }
    syncLayout();
    updateGeometry();
    update();
    return item;
}

void AntList::clearItems()
{
    for (const auto& item : m_items)
    {
        if (item)
        {
            detachItem(item.data());
        }
    }
    m_items.clear();
    m_currentItem = nullptr;
    m_verticalScrollOffset = 0;
    syncLayout();
    updateGeometry();
    update();
}

void AntList::clear()
{
    clearItems();
}

AntListItem* AntList::currentItem() const
{
    return m_currentItem.data();
}

int AntList::currentRow() const
{
    return row(m_currentItem.data());
}

void AntList::setCurrentItem(AntListItem* item)
{
    setCurrentItemInternal(item);
}

void AntList::setCurrentRow(int row)
{
    setCurrentItemInternal(itemAt(row));
}

QAbstractItemView::SelectionMode AntList::selectionMode() const
{
    return m_selectionMode;
}

void AntList::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    if (m_selectionMode == mode)
    {
        return;
    }
    m_selectionMode = mode;
    if (m_selectionMode == QAbstractItemView::NoSelection)
    {
        if (clearSelectionExcept(nullptr))
        {
            Q_EMIT itemSelectionChanged();
        }
    }
    else if (m_selectionMode == QAbstractItemView::SingleSelection && selectedItems().size() > 1)
    {
        if (clearSelectionExcept(m_currentItem.data()))
        {
            Q_EMIT itemSelectionChanged();
        }
    }
}

QList<AntListItem*> AntList::selectedItems() const
{
    QList<AntListItem*> selected;
    for (const auto& item : m_items)
    {
        if (item && item->isSelected())
        {
            selected.append(item.data());
        }
    }
    return selected;
}

void AntList::setItemSelected(AntListItem* item, bool selected)
{
    if (setItemSelectedInternal(item, selected, selected && m_selectionMode == QAbstractItemView::SingleSelection))
    {
        Q_EMIT itemSelectionChanged();
    }
}

int AntList::verticalScrollOffset() const
{
    return m_verticalScrollOffset;
}

int AntList::maximumScrollOffset() const
{
    const int viewportHeight = qMax(0, contentRect().height());
    return qMax(0, itemsHeight() - viewportHeight);
}

void AntList::setVerticalScrollOffset(int offset)
{
    const int nextOffset = qBound(0, offset, maximumScrollOffset());
    if (m_verticalScrollOffset == nextOffset)
    {
        return;
    }
    m_verticalScrollOffset = nextOffset;
    syncLayout();
    update();
}

void AntList::scrollToItem(AntListItem* item)
{
    const int itemRow = row(item);
    if (itemRow < 0)
    {
        return;
    }

    int itemTop = 0;
    for (int i = 0; i < itemRow; ++i)
    {
        AntListItem* current = itemAt(i);
        if (current && !current->isHidden())
        {
            itemTop += current->sizeHint().height();
        }
    }

    const int itemHeight = item->sizeHint().height();
    const int itemBottom = itemTop + itemHeight;
    const int viewportHeight = qMax(0, contentRect().height());
    int nextOffset = m_verticalScrollOffset;
    if (itemTop < m_verticalScrollOffset)
    {
        nextOffset = itemTop;
    }
    else if (itemBottom > m_verticalScrollOffset + viewportHeight)
    {
        nextOffset = itemBottom - viewportHeight;
    }
    setVerticalScrollOffset(nextOffset);
}

void AntList::adoptItem(AntListItem* item)
{
    if (!item)
    {
        return;
    }
    item->setParent(this);
    item->show();
    disconnect(item, &AntListItem::changed, this, nullptr);
    connect(item, &AntListItem::changed, this, [this, item]() {
        handleItemChanged(item);
    });
}

void AntList::detachItem(AntListItem* item)
{
    if (!item)
    {
        return;
    }
    disconnect(item, &AntListItem::changed, this, nullptr);
    const bool wasSelected = item->isSelected();
    if (m_currentItem == item)
    {
        AntListItem* previous = m_currentItem.data();
        m_currentItem = nullptr;
        Q_EMIT currentItemChanged(nullptr, previous);
        Q_EMIT currentRowChanged(-1);
    }
    if (wasSelected)
    {
        item->setSelected(false);
        Q_EMIT itemSelectionChanged();
    }
    item->setParent(nullptr);
}

void AntList::handleItemChanged(AntListItem* item)
{
    if (!item || row(item) < 0)
    {
        return;
    }
    syncLayout();
    updateGeometry();
    update();
    Q_EMIT itemChanged(item);
}

void AntList::setCurrentItemInternal(AntListItem* item)
{
    if (item && row(item) < 0)
    {
        item = nullptr;
    }
    if (m_currentItem == item)
    {
        if (item && m_selectionMode != QAbstractItemView::NoSelection && !item->isSelected())
        {
            setItemSelected(item, true);
        }
        if (item)
        {
            scrollToItem(item);
        }
        return;
    }

    AntListItem* previous = m_currentItem.data();
    m_currentItem = item;
    bool selectionChanged = false;
    if (m_selectionMode != QAbstractItemView::NoSelection && item)
    {
        selectionChanged = setItemSelectedInternal(item, true, m_selectionMode == QAbstractItemView::SingleSelection);
    }
    Q_EMIT currentItemChanged(item, previous);
    Q_EMIT currentRowChanged(row(item));
    if (selectionChanged)
    {
        Q_EMIT itemSelectionChanged();
    }
    if (item)
    {
        scrollToItem(item);
    }
}

bool AntList::setItemSelectedInternal(AntListItem* item, bool selected, bool clearOthers)
{
    if (!item || row(item) < 0 || m_selectionMode == QAbstractItemView::NoSelection)
    {
        return false;
    }
    if (selected && !(item->flags() & Qt::ItemIsSelectable))
    {
        return false;
    }

    bool changed = false;
    if (selected && clearOthers)
    {
        changed = clearSelectionExcept(item);
    }
    if (item->isSelected() != selected)
    {
        item->setSelected(selected);
        changed = true;
    }
    return changed;
}

bool AntList::clearSelectionExcept(AntListItem* keepItem)
{
    bool changed = false;
    for (const auto& item : m_items)
    {
        if (item && item != keepItem && item->isSelected())
        {
            item->setSelected(false);
            changed = true;
        }
    }
    return changed;
}

AntListItem* AntList::enabledSelectableItem(int startRow, int direction) const
{
    if (m_items.isEmpty() || direction == 0)
    {
        return nullptr;
    }

    int index = qBound(0, startRow, m_items.size() - 1);
    for (int step = 0; step < m_items.size(); ++step)
    {
        AntListItem* candidate = itemAt(index);
        if (candidate && candidate->isVisible() &&
            candidate->isEnabled() &&
            (candidate->flags() & Qt::ItemIsEnabled) &&
            (candidate->flags() & Qt::ItemIsSelectable))
        {
            return candidate;
        }
        index += direction;
        if (index < 0 || index >= m_items.size())
        {
            return nullptr;
        }
    }
    return nullptr;
}

QSize AntList::sizeHint() const
{
    const Metrics m = metrics();
    const int edge = m_bordered ? 2 : 0;
    int height = edge;
    if (m_header)
    {
        height += m.headerHeight;
    }
    for (const auto& item : m_items)
    {
        if (item && !item->isHidden())
        {
            height += item->sizeHint().height();
        }
    }
    if (m_footer)
    {
        height += m.footerHeight;
    }

    return QSize(360, height);
}

QSize AntList::minimumSizeHint() const
{
    return QSize(200, 60);
}

void AntList::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        AntListItem* clickedItem = itemAt(event->pos());
        if (clickedItem && clickedItem->isEnabled() &&
            (clickedItem->flags() & Qt::ItemIsEnabled))
        {
            setFocus(Qt::MouseFocusReason);
            const bool wasSelected = clickedItem->isSelected();
            setCurrentItemInternal(clickedItem);

            if (m_selectionMode == QAbstractItemView::MultiSelection ||
                (m_selectionMode == QAbstractItemView::ExtendedSelection && event->modifiers().testFlag(Qt::ControlModifier)))
            {
                setItemSelected(clickedItem, !wasSelected);
            }
            else if (m_selectionMode == QAbstractItemView::ExtendedSelection ||
                     m_selectionMode == QAbstractItemView::ContiguousSelection)
            {
                if (setItemSelectedInternal(clickedItem, true, true))
                {
                    Q_EMIT itemSelectionChanged();
                }
            }

            Q_EMIT itemClicked(clickedItem);
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntList::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        AntListItem* clickedItem = itemAt(event->pos());
        if (clickedItem && clickedItem->isEnabled() &&
            (clickedItem->flags() & Qt::ItemIsEnabled))
        {
            setCurrentItemInternal(clickedItem);
            Q_EMIT itemDoubleClicked(clickedItem);
            Q_EMIT itemActivated(clickedItem);
            event->accept();
            return;
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}

void AntList::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up)
    {
        const int direction = event->key() == Qt::Key_Down ? 1 : -1;
        const int start = currentRow() < 0 ? (direction > 0 ? 0 : m_items.size() - 1) : currentRow() + direction;
        if (AntListItem* nextItem = enabledSelectableItem(start, direction))
        {
            setCurrentItemInternal(nextItem);
            event->accept();
            return;
        }
    }
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && m_currentItem)
    {
        Q_EMIT itemActivated(m_currentItem.data());
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void AntList::wheelEvent(QWheelEvent* event)
{
    const int maxOffset = maximumScrollOffset();
    if (maxOffset <= 0)
    {
        QWidget::wheelEvent(event);
        return;
    }

    int delta = event->pixelDelta().y();
    if (delta == 0)
    {
        const int angleDelta = event->angleDelta().y();
        if (angleDelta == 0)
        {
            QWidget::wheelEvent(event);
            return;
        }
        const int wheelSteps = angleDelta / 120;
        const int direction = wheelSteps != 0 ? wheelSteps : (angleDelta > 0 ? 1 : -1);
        delta = direction * qMax(24, metrics().fontSize * 3);
    }

    setVerticalScrollOffset(m_verticalScrollOffset - delta);
    event->accept();
}

void AntList::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntList::resizeEvent(QResizeEvent* event)
{
    syncLayout();
    QWidget::resizeEvent(event);
}

AntList::Metrics AntList::metrics() const
{
    Metrics m;
    switch (m_listSize)
    {
    case Small:
        m.itemPaddingV = antTheme->tokens().paddingXS;
        m.fontSize = 14;
        break;
    case Large:
        m.itemPaddingV = antTheme->tokens().padding;
        m.fontSize = 16;
        break;
    default:
        m.itemPaddingV = antTheme->tokens().paddingSM;
        m.fontSize = 14;
        break;
    }
    m.padding = m_bordered ? 1 : 0;
    m.itemPaddingH = m_bordered ? antTheme->tokens().paddingLG : 0;
    m.headerHeight = tokenLineHeight(antTheme->tokens().fontSize) + antTheme->tokens().paddingSM * 2;
    m.footerHeight = m.headerHeight;
    m.radius = antTheme->tokens().borderRadiusLG;
    return m;
}

QRect AntList::headerRect() const
{
    if (!m_header)
    {
        return {};
    }
    const Metrics m = metrics();
    return QRect(m.padding, m.padding, width() - m.padding * 2, m.headerHeight);
}

QRect AntList::footerRect() const
{
    if (!m_footer)
    {
        return {};
    }
    const Metrics m = metrics();
    return QRect(m.padding, height() - m.footerHeight - m.padding, width() - m.padding * 2, m.footerHeight);
}

QRect AntList::contentRect() const
{
    const Metrics m = metrics();
    int top = m.padding;
    int bottom = height() - m.padding;
    if (m_header)
    {
        top = headerRect().bottom();
    }
    if (m_footer)
    {
        bottom = footerRect().top();
    }
    return QRect(0, top, width(), bottom - top);
}

int AntList::itemsHeight() const
{
    int height = 0;
    for (const auto& item : m_items)
    {
        if (item && !item->isHidden())
        {
            height += item->sizeHint().height();
        }
    }
    return height;
}

void AntList::syncLayout()
{
    if (m_header)
    {
        m_header->setGeometry(headerRect());
        m_header->show();
    }
    if (m_footer)
    {
        m_footer->setGeometry(footerRect());
        m_footer->show();
    }

    const QRect cr = contentRect();
    m_verticalScrollOffset = qBound(0, m_verticalScrollOffset, maximumScrollOffset());
    int y = cr.top() - m_verticalScrollOffset;

    for (const auto& item : m_items)
    {
        if (item)
        {
            if (item->isHidden())
            {
                continue;
            }
            const int h = item->sizeHint().height();
            item->setGeometry(cr.left(), y, cr.width(), h);
            item->show();
            y += h;
        }
    }
}
