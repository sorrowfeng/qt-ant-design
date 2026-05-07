#include "AntList.h"

#include <QFontMetrics>
#include <QPainter>
#include <QResizeEvent>

#include "../styles/AntListStyle.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

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
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
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
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

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

    if (m_actions.size() <= 1)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
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
    int right = width() - paddingH;
    if (m_extra)
    {
        right = extraRect().left() - token.padding;
    }
    if (!m_actions.isEmpty())
    {
        right = qMin(right, actionsRect().left() - token.paddingXL);
    }
    return QRect(paddingH, paddingV, qMax(0, qMin(sz.width(), right - paddingH)), height() - paddingV * 2);
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

void AntListItem::syncLayout()
{
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
        const int left = m_meta ? mr.right() + antTheme->tokens().padding : paddingH;
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
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
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
    item->setParent(this);
    m_items.append(item);
    syncLayout();
    updateGeometry();
    update();
}

void AntList::insertItem(int index, AntListItem* item)
{
    if (!item)
    {
        return;
    }
    index = qBound(0, index, m_items.size());
    item->setParent(this);
    m_items.insert(index, item);
    syncLayout();
    updateGeometry();
    update();
}

void AntList::removeItem(AntListItem* item)
{
    if (!item)
    {
        return;
    }
    m_items.removeAll(item);
    item->setParent(nullptr);
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

AntListItem* AntList::takeItem(int index)
{
    if (index < 0 || index >= m_items.size())
    {
        return nullptr;
    }

    AntListItem* item = m_items.takeAt(index).data();
    if (item)
    {
        item->setParent(nullptr);
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
            item->setParent(nullptr);
        }
    }
    m_items.clear();
    syncLayout();
    updateGeometry();
    update();
}

void AntList::clear()
{
    clearItems();
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
        if (item)
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
    int y = cr.top();

    for (const auto& item : m_items)
    {
        if (item)
        {
            const int h = item->sizeHint().height();
            item->setGeometry(cr.left(), y, cr.width(), h);
            item->show();
            y += h;
        }
    }
}
