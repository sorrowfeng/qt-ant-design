#include "AntList.h"

#include <QFontMetrics>
#include <QPainter>
#include <QResizeEvent>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

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
    const int avatarSpace = m_avatar ? m_avatar->sizeHint().width() + 12 : 0;

    QFont titleFont = font();
    titleFont.setPixelSize(token.fontSize);
    titleFont.setWeight(QFont::DemiBold);
    QFontMetrics titleFm(titleFont);

    QFont descFont = font();
    descFont.setPixelSize(token.fontSizeSM);
    QFontMetrics descFm(descFont);

    int height = titleFm.height();
    if (!m_description.isEmpty())
    {
        height += 4 + descFm.height();
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
    const int titleHeight = titleFm.height();
    painter.drawText(tr.left(), tr.top() + titleFm.ascent(), m_title);

    if (!m_description.isEmpty())
    {
        QFont descFont = painter.font();
        descFont.setPixelSize(token.fontSizeSM);
        descFont.setWeight(QFont::Normal);
        painter.setFont(descFont);
        painter.setPen(token.colorTextSecondary);

        QFontMetrics descFm(descFont);
        painter.drawText(tr.left(), tr.top() + titleHeight + 4 + descFm.ascent(), m_description);
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
    const int left = m_avatar ? avatarRect().right() + 12 : 0;
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
    const int paddingV = token.paddingSM;
    const int paddingH = token.padding;

    int contentHeight = 0;
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
    contentHeight = qMax(contentHeight, token.fontSize + 4);

    return QSize(200, contentHeight + paddingV * 2);
}

void AntListItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
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
    const int paddingV = token.paddingSM;
    const QSize sz = m_meta->sizeHint();
    const int extraRight = m_extra ? extraRect().left() - 12 : width();
    return QRect(token.padding, paddingV, qMin(sz.width(), extraRight - token.padding), height() - paddingV * 2);
}

QRect AntListItem::extraRect() const
{
    if (!m_extra)
    {
        return {};
    }
    const auto& token = antTheme->tokens();
    const int paddingV = token.paddingSM;
    const QSize sz = m_extra->sizeHint();
    return QRect(width() - token.padding - sz.width(), paddingV, sz.width(), height() - paddingV * 2);
}

QRect AntListItem::actionsRect() const
{
    if (m_actions.isEmpty())
    {
        return {};
    }
    const auto& token = antTheme->tokens();
    return QRect(token.padding, height() - token.paddingSM - 24, width() - token.padding * 2, 24);
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
        const QRect mr = metaRect();
        const QRect er = m_extra ? extraRect() : QRect(width(), 0, 0, 0);
        m_content->setGeometry(QRect(mr.right() + 12, mr.top(), er.left() - mr.right() - 24, mr.height()));
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
                x += sz.width() + 8;
            }
        }
    }
}

// ─── AntList ───

AntList::AntList(QWidget* parent)
    : QWidget(parent)
{
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        syncLayout();
        updateGeometry();
        update();
    });
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

AntListItem* AntList::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size())
    {
        return nullptr;
    }
    return m_items.at(index).data();
}

QSize AntList::sizeHint() const
{
    const Metrics m = metrics();
    int height = m.padding;
    if (m_header)
    {
        height += m.headerHeight;
    }
    for (const auto& item : m_items)
    {
        if (item)
        {
            height += item->sizeHint().height();
            if (m_split && item != m_items.last())
            {
                height += 1;
            }
        }
    }
    if (m_footer)
    {
        height += m.footerHeight;
    }
    height += m.padding;

    return QSize(360, height);
}

QSize AntList::minimumSizeHint() const
{
    return QSize(200, 60);
}

void AntList::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const Metrics m = metrics();
    const auto& token = antTheme->tokens();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    if (m_bordered)
    {
        painter.setPen(QPen(token.colorBorder, token.lineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), m.radius, m.radius);
    }

    if (m_header)
    {
        const QRect hr = headerRect();
        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorFillQuaternary);
        if (m_bordered)
        {
            painter.drawRect(hr);
        }
        else
        {
            painter.drawRoundedRect(hr, m.radius, m.radius);
        }
        painter.setPen(QPen(token.colorBorder, token.lineWidth));
        painter.drawLine(hr.bottomLeft(), hr.bottomRight());
    }

    if (m_split && m_items.size() > 1)
    {
        painter.setPen(QPen(token.colorSplit, token.lineWidth));
        const QRect cr = contentRect();
        int y = cr.top();
        for (int i = 0; i < m_items.size() - 1; ++i)
        {
            const auto& item = m_items.at(i);
            if (item)
            {
                y += item->sizeHint().height();
                painter.drawLine(cr.left() + m.itemPaddingH, y, cr.right() - m.itemPaddingH, y);
                y += 1;
            }
        }
    }

    if (m_footer)
    {
        const QRect fr = footerRect();
        painter.setPen(QPen(token.colorBorder, token.lineWidth));
        painter.drawLine(fr.topLeft(), fr.topRight());
    }
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
        m.itemPaddingV = 8;
        m.fontSize = 14;
        break;
    case Large:
        m.itemPaddingV = 16;
        m.fontSize = 16;
        break;
    default:
        m.itemPaddingV = 12;
        m.fontSize = 14;
        break;
    }
    return m;
}

QRect AntList::headerRect() const
{
    if (!m_header)
    {
        return {};
    }
    const Metrics m = metrics();
    return QRect(1, 1, width() - 2, m.headerHeight);
}

QRect AntList::footerRect() const
{
    if (!m_footer)
    {
        return {};
    }
    const Metrics m = metrics();
    return QRect(1, height() - m.footerHeight - 1, width() - 2, m.footerHeight);
}

QRect AntList::contentRect() const
{
    const Metrics m = metrics();
    int top = m.padding;
    int bottom = height() - m.padding;
    if (m_header)
    {
        top = headerRect().bottom() + 1;
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
            if (m_split && item != m_items.last())
            {
                y += 1;
            }
        }
    }
}
