#include "AntTreeSelect.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QLineEdit>

#include "core/AntTheme.h"
#include "../styles/AntTreeSelectStyle.h"

class AntTreeSelect::TreeSelectPopup : public QFrame
{
public:
    TreeSelectPopup(AntTreeSelect* owner)
        : QFrame(owner, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
        , m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setFixedSize(300, 320);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setSpacing(0);

        if (m_owner->m_showSearch)
        {
            m_searchEdit = new QLineEdit(this);
            m_searchEdit->setPlaceholderText(QStringLiteral("Search..."));
            m_searchEdit->setFixedHeight(32);
            layout->addWidget(m_searchEdit);
        }

        m_treeWidget = new AntTree(this);
        m_treeWidget->setCheckable(m_owner->m_treeCheckable);
        m_treeWidget->setShowLine(true);
        m_treeWidget->setTreeData(m_owner->m_treeData);
        layout->addWidget(m_treeWidget, 1);

        connect(m_treeWidget, &AntTree::nodeSelected, this, [this](const QString& key) {
            if (!m_owner->m_multiple)
            {
                m_owner->m_value = {key};
                m_owner->updateDisplayText();
                m_owner->hidePopup();
                Q_EMIT m_owner->valueChanged(m_owner->m_value);
            }
        });

        connect(m_treeWidget, &AntTree::nodeChecked, this, [this](const QStringList& keys) {
            if (m_owner->m_multiple)
            {
                m_owner->m_value = keys;
                m_owner->updateDisplayText();
                Q_EMIT m_owner->valueChanged(m_owner->m_value);
            }
        });
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const auto& token = antTheme->tokens();

        QPainterPath path;
        path.addRoundedRect(rect().adjusted(1, 1, -1, -1), token.borderRadius, token.borderRadius);

        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorBgElevated);
        painter.drawPath(path);

        painter.setPen(QPen(token.colorBorderSecondary, 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(path);
    }

    void hideEvent(QHideEvent* event) override
    {
        QFrame::hideEvent(event);
        if (m_owner)
        {
            m_owner->m_open = false;
            Q_EMIT m_owner->openChanged(false);
        }
    }

private:
    AntTreeSelect* m_owner;
    AntTree* m_treeWidget = nullptr;
    QLineEdit* m_searchEdit = nullptr;
};

AntTreeSelect::AntTreeSelect(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    auto* s = new AntTreeSelectStyle(style());
    s->setParent(this);
    setStyle(s);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QVector<AntTreeNode> AntTreeSelect::treeData() const
{
    return m_treeData;
}

void AntTreeSelect::setTreeData(const QVector<AntTreeNode>& data)
{
    m_treeData = data;
    update();
}

QStringList AntTreeSelect::value() const
{
    return m_value;
}

void AntTreeSelect::setValue(const QStringList& keys)
{
    m_value = keys;
    updateDisplayText();
    update();
    Q_EMIT valueChanged(m_value);
}

QString AntTreeSelect::placeholder() const
{
    return m_placeholder;
}

void AntTreeSelect::setPlaceholder(const QString& text)
{
    if (m_placeholder != text)
    {
        m_placeholder = text;
        update();
        Q_EMIT placeholderChanged(text);
    }
}

bool AntTreeSelect::allowClear() const
{
    return m_allowClear;
}

void AntTreeSelect::setAllowClear(bool enable)
{
    if (m_allowClear != enable)
    {
        m_allowClear = enable;
        update();
        Q_EMIT allowClearChanged(enable);
    }
}

bool AntTreeSelect::isMultiple() const
{
    return m_multiple;
}

void AntTreeSelect::setMultiple(bool enable)
{
    if (m_multiple != enable)
    {
        m_multiple = enable;
        update();
        Q_EMIT multipleChanged(enable);
    }
}

bool AntTreeSelect::isTreeCheckable() const
{
    return m_treeCheckable;
}

void AntTreeSelect::setTreeCheckable(bool enable)
{
    if (m_treeCheckable != enable)
    {
        m_treeCheckable = enable;
        update();
        Q_EMIT treeCheckableChanged(enable);
    }
}

bool AntTreeSelect::isShowSearch() const
{
    return m_showSearch;
}

void AntTreeSelect::setShowSearch(bool enable)
{
    if (m_showSearch != enable)
    {
        m_showSearch = enable;
        Q_EMIT showSearchChanged(enable);
    }
}

Ant::Size AntTreeSelect::selectSize() const
{
    return m_selectSize;
}

void AntTreeSelect::setSelectSize(Ant::Size size)
{
    if (m_selectSize != size)
    {
        m_selectSize = size;
        updateGeometry();
        update();
        Q_EMIT selectSizeChanged(size);
    }
}

Ant::Status AntTreeSelect::status() const
{
    return m_status;
}

void AntTreeSelect::setStatus(Ant::Status status)
{
    if (m_status != status)
    {
        m_status = status;
        update();
        Q_EMIT statusChanged(status);
    }
}

Ant::Variant AntTreeSelect::variant() const
{
    return m_variant;
}

void AntTreeSelect::setVariant(Ant::Variant variant)
{
    if (m_variant != variant)
    {
        m_variant = variant;
        update();
        Q_EMIT variantChanged(variant);
    }
}

bool AntTreeSelect::isOpen() const
{
    return m_open;
}

void AntTreeSelect::setOpen(bool open)
{
    if (m_open != open)
    {
        m_open = open;
        update();
        Q_EMIT openChanged(open);
    }
}

QString AntTreeSelect::displayText() const
{
    return m_displayText;
}

bool AntTreeSelect::isHovered() const
{
    return m_hovered;
}

QSize AntTreeSelect::sizeHint() const
{
    const auto& token = antTheme->tokens();
    int height = token.controlHeight;
    if (m_selectSize == Ant::Size::Large)
    {
        height = token.controlHeightLG;
    }
    else if (m_selectSize == Ant::Size::Small)
    {
        height = token.controlHeightSM;
    }
    return QSize(220, height);
}

QSize AntTreeSelect::minimumSizeHint() const
{
    return QSize(80, sizeHint().height());
}

void AntTreeSelect::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTreeSelect::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (m_allowClear && !m_value.isEmpty() && isOverClear(event->pos()))
    {
        m_value.clear();
        updateDisplayText();
        update();
        Q_EMIT valueChanged(m_value);
        return;
    }

    if (m_open)
    {
        hidePopup();
    }
    else
    {
        showPopup();
    }
}

void AntTreeSelect::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    bool overClear = m_allowClear && !m_value.isEmpty() && isOverClear(event->pos());
    if (overClear != m_hovered)
    {
        m_hovered = overClear;
        update();
    }
}

void AntTreeSelect::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    if (m_hovered)
    {
        m_hovered = false;
        update();
    }
}

void AntTreeSelect::wheelEvent(QWheelEvent* event)
{
    Q_UNUSED(event)
}

void AntTreeSelect::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && m_open)
    {
        hidePopup();
    }
    else if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return)
    {
        if (!m_open)
            showPopup();
    }
}

void AntTreeSelect::showPopup()
{
    if (!m_popup)
    {
        m_popup = new TreeSelectPopup(this);
    }

    const QPoint pos = mapToGlobal(QPoint(0, height()));
    m_popup->move(pos);
    m_popup->show();
    m_open = true;
    Q_EMIT openChanged(true);
    update();
}

void AntTreeSelect::hidePopup()
{
    if (m_popup)
    {
        m_popup->hide();
    }
    m_open = false;
    Q_EMIT openChanged(false);
    update();
}

void AntTreeSelect::updateDisplayText()
{
    if (m_value.isEmpty())
    {
        m_displayText.clear();
    }
    else
    {
        QStringList titles = findNodeTitles(m_treeData, m_value);
        m_displayText = titles.join(QStringLiteral(", "));
    }
    update();
}

QString AntTreeSelect::findNodeTitle(const QVector<AntTreeNode>& nodes, const QString& key) const
{
    for (const auto& node : nodes)
    {
        if (node.key == key)
            return node.title;
        if (!node.children.isEmpty())
        {
            QString title = findNodeTitle(node.children, key);
            if (!title.isEmpty())
                return title;
        }
    }
    return {};
}

QStringList AntTreeSelect::findNodeTitles(const QVector<AntTreeNode>& nodes, const QStringList& keys) const
{
    QStringList titles;
    for (const auto& key : keys)
    {
        titles.append(findNodeTitle(nodes, key));
    }
    return titles;
}

QRect AntTreeSelect::triggerRect() const
{
    return rect();
}

QRect AntTreeSelect::clearButtonRect() const
{
    int h = height();
    int s = 16;
    return QRect(width() - 40, (h - s) / 2, s, s);
}

QRect AntTreeSelect::arrowRect() const
{
    int h = height();
    int s = 16;
    return QRect(width() - 24, (h - s) / 2, s, s);
}

bool AntTreeSelect::isOverClear(const QPoint& pos) const
{
    return clearButtonRect().contains(pos);
}
