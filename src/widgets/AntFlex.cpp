#include "AntFlex.h"

#include <QHBoxLayout>
#include <QLayoutItem>
#include <QStyle>
#include <QVBoxLayout>

namespace
{
void addWidgetToFlexLayout(QLayout* layout, QWidget* widget)
{
    if (auto* horizontal = qobject_cast<QHBoxLayout*>(layout))
    {
        horizontal->addWidget(widget, 0, Qt::AlignLeft);
        return;
    }
    if (auto* vertical = qobject_cast<QVBoxLayout*>(layout))
    {
        vertical->addWidget(widget);
        return;
    }
    layout->addWidget(widget);
}

class FlexWrapLayout : public QLayout
{
public:
    explicit FlexWrapLayout(bool allowWrap, QWidget* parent = nullptr)
        : QLayout(parent)
        , m_allowWrap(allowWrap)
    {
    }

    ~FlexWrapLayout() override
    {
        QLayoutItem* item = nullptr;
        while ((item = takeAt(0)) != nullptr)
        {
            delete item;
        }
    }

    void addItem(QLayoutItem* item) override
    {
        m_items.append(item);
    }

    int count() const override
    {
        return m_items.size();
    }

    QLayoutItem* itemAt(int index) const override
    {
        return m_items.value(index);
    }

    QLayoutItem* takeAt(int index) override
    {
        return index >= 0 && index < m_items.size() ? m_items.takeAt(index) : nullptr;
    }

    Qt::Orientations expandingDirections() const override
    {
        return {};
    }

    bool hasHeightForWidth() const override
    {
        return true;
    }

    int heightForWidth(int width) const override
    {
        return doLayout(QRect(0, 0, width, 0), true);
    }

    QSize sizeHint() const override
    {
        const QMargins margins = contentsMargins();
        int width = margins.left() + margins.right();
        int height = 0;
        for (const QLayoutItem* item : m_items)
        {
            const QSize itemSize = item->sizeHint();
            if (item != m_items.constFirst())
            {
                width += spacing();
            }
            width += itemSize.width();
            height = qMax(height, itemSize.height());
        }
        return QSize(width, height + margins.top() + margins.bottom());
    }

    QSize minimumSize() const override
    {
        QSize size;
        for (const QLayoutItem* item : m_items)
        {
            size = size.expandedTo(item->minimumSize());
        }
        const QMargins margins = contentsMargins();
        size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
        return size;
    }

    void setGeometry(const QRect& rect) override
    {
        QLayout::setGeometry(rect);
        doLayout(rect, false);
    }

private:
    int doLayout(const QRect& rect, bool testOnly) const
    {
        const QMargins margins = contentsMargins();
        const QRect effectiveRect = rect.adjusted(margins.left(), margins.top(), -margins.right(), -margins.bottom());
        int x = effectiveRect.x();
        int y = effectiveRect.y();
        int lineHeight = 0;

        for (QLayoutItem* item : m_items)
        {
            const QSize itemSize = item->sizeHint();
            const int nextX = x + itemSize.width() + spacing();
            if (m_allowWrap && nextX - spacing() > effectiveRect.right() && lineHeight > 0)
            {
                x = effectiveRect.x();
                y += lineHeight + spacing();
                lineHeight = 0;
            }

            if (!testOnly)
            {
                item->setGeometry(QRect(QPoint(x, y), itemSize));
            }

            x += itemSize.width() + spacing();
            lineHeight = qMax(lineHeight, itemSize.height());
        }

        return y + lineHeight - rect.y() + margins.bottom();
    }

    QList<QLayoutItem*> m_items;
    bool m_allowWrap = false;
};
}

AntFlex::AntFlex(QWidget* parent)
    : QWidget(parent)
{
    rebuildLayout();
}

bool AntFlex::vertical() const { return m_vertical; }
void AntFlex::setVertical(bool v)
{
    if (m_vertical == v) return;
    m_vertical = v;
    rebuildLayout();
    Q_EMIT verticalChanged(m_vertical);
}

int AntFlex::gap() const { return m_gap; }
void AntFlex::setGap(int px)
{
    if (m_gap == px) return;
    m_gap = px;
    if (m_layout) m_layout->setSpacing(px);
    Q_EMIT gapChanged(m_gap);
}

bool AntFlex::wrap() const { return m_wrap; }
void AntFlex::setWrap(bool w)
{
    if (m_wrap == w) return;
    m_wrap = w;
    rebuildLayout();
    Q_EMIT wrapChanged(m_wrap);
}

void AntFlex::addWidget(QWidget* widget)
{
    if (!widget)
    {
        return;
    }
    m_children.append(widget);
    addWidgetToFlexLayout(m_layout, widget);
}

void AntFlex::addStretch()
{
    if (dynamic_cast<FlexWrapLayout*>(m_layout))
    {
        return;
    }
    if (auto* hb = qobject_cast<QHBoxLayout*>(m_layout))
        hb->addStretch();
    else if (auto* vb = qobject_cast<QVBoxLayout*>(m_layout))
        vb->addStretch();
}

void AntFlex::rebuildLayout()
{
    delete m_layout;
    if (!m_vertical)
    {
        m_layout = new FlexWrapLayout(m_wrap, this);
    }
    else
    {
        m_layout = new QVBoxLayout(this);
    }
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(m_gap);

    for (auto* w : m_children)
        addWidgetToFlexLayout(m_layout, w);
}
