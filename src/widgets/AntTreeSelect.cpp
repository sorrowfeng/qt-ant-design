#include "AntTreeSelect.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QLineEdit>

#include <algorithm>

#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"
#include "../styles/AntTreeSelectStyle.h"

namespace
{
constexpr int kPopupShadowMargin = 32;
constexpr int kPopupInnerPadding = 4;
constexpr int kTreeRowHeight = 28;

int visibleTreeRows(const QVector<AntTreeNode>& nodes)
{
    int rows = 0;
    for (const auto& node : nodes)
    {
        ++rows;
        if (node.expanded)
        {
            rows += visibleTreeRows(node.children);
        }
    }
    return rows;
}
} // namespace

class AntTreeSelect::TreeSelectPopup : public QFrame
{
public:
    TreeSelectPopup(AntTreeSelect* owner)
        : QFrame(owner, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
        , m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(kPopupShadowMargin + kPopupInnerPadding,
                                   kPopupShadowMargin + kPopupInnerPadding,
                                   kPopupShadowMargin + kPopupInnerPadding,
                                   kPopupShadowMargin + kPopupInnerPadding);
        layout->setSpacing(0);

        m_treeWidget = new AntTree(this);
        m_treeWidget->setShowLine(false);
        m_treeWidget->setShowIcon(false);
        layout->addWidget(m_treeWidget);

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

        connect(m_treeWidget, &AntTree::nodeExpanded, this, [this]() {
            refreshSizeFromTree();
        });

        refreshFromOwner();
    }

    void refreshFromOwner()
    {
        syncSearchEditor();
        bool dataChanged = false;
        if (m_treeWidget)
        {
            if (m_appliedCheckable != m_owner->m_treeCheckable)
            {
                m_treeWidget->setCheckable(m_owner->m_treeCheckable);
                m_appliedCheckable = m_owner->m_treeCheckable;
            }

            if (m_appliedTreeDataRevision != m_owner->treeDataRevision())
            {
                m_treeWidget->setTreeData(m_owner->m_treeData);
                m_appliedTreeDataRevision = m_owner->treeDataRevision();
                ++m_treeRebuildCount;
                dataChanged = true;
            }
        }

        const int ownerRows = m_owner->visibleTreeRowCount();
        const int rows = dataChanged || !m_treeWidget
            ? ownerRows
            : std::max(1, visibleTreeRows(m_treeWidget->treeData()));
        applyPopupMetrics(rows);
        syncOwnerPerfCounters();
    }

    void refreshSizeFromTree()
    {
        const int rows = std::max(1, visibleTreeRows(m_treeWidget ? m_treeWidget->treeData() : QVector<AntTreeNode>{}));
        applyPopupMetrics(rows);
        if (m_owner)
        {
            m_owner->setProperty("antTreeSelectLastPopupUpdateMode", QStringLiteral("expand"));
            m_owner->setProperty("antTreeSelectVisibleRowCount", rows);
        }
        syncOwnerPerfCounters();
    }

    void invalidateOwnerData()
    {
        m_appliedTreeDataRevision = -1;
    }

private:
    void syncSearchEditor()
    {
        auto* box = qobject_cast<QVBoxLayout*>(layout());
        if (!box)
        {
            return;
        }

        if (m_owner->m_showSearch)
        {
            if (!m_searchEdit)
            {
                m_searchEdit = new QLineEdit(this);
                m_searchEdit->setPlaceholderText(QStringLiteral("Search..."));
                m_searchEdit->setFixedHeight(32);
                box->insertWidget(0, m_searchEdit);
            }
            return;
        }

        if (m_searchEdit)
        {
            box->removeWidget(m_searchEdit);
            m_searchEdit->deleteLater();
            m_searchEdit = nullptr;
        }
    }

    void applyPopupMetrics(int rows)
    {
        const int panelWidth = std::max(240, m_owner ? m_owner->width() : 240);
        int panelHeight = rows * kTreeRowHeight + kPopupInnerPadding * 2;
        if (m_searchEdit)
        {
            panelHeight += 32 + kPopupInnerPadding;
        }
        const QSize targetSize(panelWidth + kPopupShadowMargin * 2, panelHeight + kPopupShadowMargin * 2);
        if (size() == targetSize)
        {
            ++m_sizeSkipCount;
        }
        else
        {
            setFixedSize(targetSize);
        }

        if (m_treeWidget)
        {
            const int treeHeight = rows * kTreeRowHeight;
            if (m_treeWidget->height() == treeHeight)
            {
                ++m_treeHeightSkipCount;
            }
            else
            {
                m_treeWidget->setFixedHeight(treeHeight);
            }
        }

        const QPoint targetPos = m_owner
            ? m_owner->mapToGlobal(QPoint(-kPopupShadowMargin, m_owner->height() + 4 - kPopupShadowMargin))
            : QPoint();
        if (m_hasTargetPos && m_lastTargetPos == targetPos)
        {
            ++m_geometrySkipCount;
        }
        else
        {
            m_lastTargetPos = targetPos;
            m_hasTargetPos = true;
        }
        if (pos() != targetPos)
        {
            move(targetPos);
        }

        if (m_owner)
        {
            m_owner->setProperty("antTreeSelectVisibleRowCount", rows);
            m_owner->setProperty("antTreeSelectLastPopupUpdateMode", QStringLiteral("refresh"));
        }
    }

    void syncOwnerPerfCounters() const
    {
        if (!m_owner)
        {
            return;
        }
        m_owner->setProperty("antTreeSelectPopupTreeRebuildCount", m_treeRebuildCount);
        m_owner->setProperty("antTreeSelectPopupGeometrySkipCount", m_geometrySkipCount);
        m_owner->setProperty("antTreeSelectPopupSizeSkipCount", m_sizeSkipCount);
        m_owner->setProperty("antTreeSelectPopupTreeHeightSkipCount", m_treeHeightSkipCount);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const auto& token = antTheme->tokens();

        QPainterPath path;
        const QRect panelRect = rect().adjusted(kPopupShadowMargin, kPopupShadowMargin,
                                                -kPopupShadowMargin, -kPopupShadowMargin);
        antTheme->drawEffectShadow(&painter, panelRect, 10, token.borderRadiusLG, 0.45);
        path.addRoundedRect(QRectF(panelRect).adjusted(0.5, 0.5, -0.5, -0.5),
                            token.borderRadiusLG, token.borderRadiusLG);

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
        if (m_owner && m_owner->m_open)
        {
            m_owner->m_open = false;
            Q_EMIT m_owner->openChanged(false);
            m_owner->update();
        }
    }

private:
    AntTreeSelect* m_owner;
    AntTree* m_treeWidget = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    int m_appliedTreeDataRevision = -1;
    bool m_appliedCheckable = true;
    int m_treeRebuildCount = 0;
    int m_geometrySkipCount = 0;
    int m_sizeSkipCount = 0;
    int m_treeHeightSkipCount = 0;
    QPoint m_lastTargetPos;
    bool m_hasTargetPos = false;
};

AntTreeSelect::AntTreeSelect(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    auto* s = new AntTreeSelectStyle(style());
    s->setParent(this);
    setStyle(s);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    syncTreeSelectPerfCounters();

    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateTriggerLayout();
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        updateTriggerRegion(rect(), QStringLiteral("theme"));
        if (m_popup && m_popup->isVisible())
        {
            m_popup->refreshFromOwner();
            m_popup->update();
        }
    });
}

AntTreeSelect::~AntTreeSelect()
{
    if (m_popup)
    {
        AntPopupMotion::stop(m_popup);
        delete m_popup;
        m_popup = nullptr;
    }
}

QVector<AntTreeNode> AntTreeSelect::treeData() const
{
    return m_treeData;
}

void AntTreeSelect::setTreeData(const QVector<AntTreeNode>& data)
{
    m_treeData = data;
    ++m_treeDataRevision;
    if (m_treeDataRevision <= 0)
    {
        m_treeDataRevision = 1;
    }
    invalidateTreeCaches();
    invalidateTitleCache();
    updateDisplayText();
    if (m_popup)
    {
        m_popup->invalidateOwnerData();
        m_popup->refreshFromOwner();
    }
    updateTriggerRegion(rect(), QStringLiteral("treeData"));
}

QStringList AntTreeSelect::value() const
{
    return m_value;
}

void AntTreeSelect::setValue(const QStringList& keys)
{
    if (m_value == keys)
    {
        return;
    }
    m_value = keys;
    if (!updateDisplayText())
    {
        updateTriggerRegion(rect(), QStringLiteral("value"));
    }
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
        updateTriggerRegion(rect(), QStringLiteral("placeholder"));
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
        updateTriggerRegion(arrowRect().united(clearButtonRect()), QStringLiteral("allowClear"), true);
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
        updateDisplayText();
        updateTriggerRegion(rect(), QStringLiteral("multiple"));
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
        if (m_popup)
        {
            m_popup->refreshFromOwner();
        }
        updateTriggerRegion(rect(), QStringLiteral("treeCheckable"));
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
        if (m_popup)
        {
            m_popup->refreshFromOwner();
        }
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
        invalidateTriggerLayout();
        updateGeometry();
        updateTriggerRegion(rect(), QStringLiteral("size"));
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
        updateTriggerRegion(rect(), QStringLiteral("status"));
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
        updateTriggerRegion(rect(), QStringLiteral("variant"));
        Q_EMIT variantChanged(variant);
    }
}

bool AntTreeSelect::isOpen() const
{
    return m_open;
}

void AntTreeSelect::setOpen(bool open)
{
    if (open)
    {
        showPopup();
    }
    else
    {
        hidePopup();
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
    return triggerLayout().sizeHint;
}

QSize AntTreeSelect::minimumSizeHint() const
{
    return triggerLayout().minimumSizeHint;
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
        updateTriggerRegion(clearButtonRect().united(arrowRect()), QStringLiteral("clearHover"), true);
    }
}

void AntTreeSelect::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    if (m_hovered)
    {
        m_hovered = false;
        updateTriggerRegion(clearButtonRect().united(arrowRect()), QStringLiteral("clearLeave"), true);
    }
}

void AntTreeSelect::resizeEvent(QResizeEvent* event)
{
    invalidateTriggerLayout();
    QWidget::resizeEvent(event);
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
    if (m_open && m_popup && m_popup->isVisible())
    {
        return;
    }

    if (!m_popup)
    {
        m_popup = new TreeSelectPopup(this);
    }

    m_popup->refreshFromOwner();
    AntPopupMotion::show(m_popup);
    const bool wasOpen = m_open;
    m_open = true;
    if (!wasOpen)
    {
        Q_EMIT openChanged(true);
    }
    updateTriggerRegion(arrowRect(), QStringLiteral("open"));
}

void AntTreeSelect::hidePopup()
{
    if (!m_open && (!m_popup || !m_popup->isVisible()))
    {
        return;
    }

    if (m_popup && m_popup->isVisible())
    {
        AntPopupMotion::hide(m_popup);
    }
    const bool wasOpen = m_open;
    m_open = false;
    if (wasOpen)
    {
        Q_EMIT openChanged(false);
    }
    updateTriggerRegion(arrowRect(), QStringLiteral("open"));
}

bool AntTreeSelect::updateDisplayText()
{
    const QString oldDisplayText = m_displayText;
    if (m_value.isEmpty())
    {
        m_displayText.clear();
    }
    else
    {
        QStringList titles = findNodeTitles(m_treeData, m_value);
        m_displayText = titles.join(QStringLiteral(", "));
    }
    if (m_displayText == oldDisplayText)
    {
        return false;
    }
    updateTriggerRegion(rect(), QStringLiteral("display"));
    return true;
}

QString AntTreeSelect::findNodeTitle(const QVector<AntTreeNode>& nodes, const QString& key) const
{
    Q_UNUSED(nodes)
    ensureTitleCache();
    return m_titleCache.value(key);
}

QStringList AntTreeSelect::findNodeTitles(const QVector<AntTreeNode>& nodes, const QStringList& keys) const
{
    Q_UNUSED(nodes)
    ensureTitleCache();
    QStringList titles;
    titles.reserve(keys.size());
    for (const auto& key : keys)
    {
        titles.append(m_titleCache.value(key));
    }
    return titles;
}

const AntTreeSelect::TriggerLayout& AntTreeSelect::triggerLayout() const
{
    if (m_triggerLayout.valid
        && m_triggerLayout.widgetSize == size()
        && m_triggerLayout.selectSize == m_selectSize)
    {
        ++m_triggerLayoutCacheHitCount;
        syncTreeSelectPerfCounters();
        return m_triggerLayout;
    }

    const auto& token = antTheme->tokens();
    m_triggerLayout = TriggerLayout{};
    m_triggerLayout.widgetSize = size();
    m_triggerLayout.selectSize = m_selectSize;
    switch (m_selectSize)
    {
    case Ant::Size::Large:
        m_triggerLayout.height = token.controlHeightLG;
        m_triggerLayout.fontSize = token.fontSizeLG;
        break;
    case Ant::Size::Small:
        m_triggerLayout.height = token.controlHeightSM;
        m_triggerLayout.fontSize = token.fontSizeSM;
        break;
    default:
        m_triggerLayout.height = token.controlHeight;
        m_triggerLayout.fontSize = token.fontSize;
        break;
    }
    m_triggerLayout.paddingX = 12;
    m_triggerLayout.arrowWidth = 24;
    m_triggerLayout.radius = token.borderRadius;
    m_triggerLayout.triggerRect = rect();
    m_triggerLayout.arrowRect = QRect(width() - m_triggerLayout.arrowWidth,
                                      0,
                                      m_triggerLayout.arrowWidth,
                                      height());
    const int clearSize = 16;
    m_triggerLayout.clearButtonRect = QRect(m_triggerLayout.arrowRect.center().x() - clearSize / 2,
                                            (height() - clearSize) / 2,
                                            clearSize,
                                            clearSize);
    m_triggerLayout.sizeHint = QSize(220, m_triggerLayout.height);
    m_triggerLayout.minimumSizeHint = QSize(80, m_triggerLayout.height);
    m_triggerLayout.valid = true;
    ++m_triggerLayoutBuildCount;
    syncTreeSelectPerfCounters();
    return m_triggerLayout;
}

int AntTreeSelect::treeDataRevision() const
{
    return m_treeDataRevision;
}

int AntTreeSelect::visibleTreeRowCount() const
{
    if (m_cachedVisibleRowRevision == m_treeDataRevision)
    {
        ++m_visibleRowsCacheHitCount;
        syncTreeSelectPerfCounters();
        return m_cachedVisibleRows;
    }

    m_cachedVisibleRows = std::max(1, visibleTreeRows(m_treeData));
    m_cachedVisibleRowRevision = m_treeDataRevision;
    ++m_visibleRowsBuildCount;
    syncTreeSelectPerfCounters();
    return m_cachedVisibleRows;
}

void AntTreeSelect::invalidateTriggerLayout() const
{
    m_triggerLayout.valid = false;
    syncTreeSelectPerfCounters();
}

void AntTreeSelect::invalidateTreeCaches() const
{
    m_cachedVisibleRowRevision = -1;
    syncTreeSelectPerfCounters();
}

void AntTreeSelect::invalidateTitleCache() const
{
    m_titleCacheRevision = -1;
    m_titleCache.clear();
    syncTreeSelectPerfCounters();
}

void AntTreeSelect::ensureTitleCache() const
{
    if (m_titleCacheRevision == m_treeDataRevision)
    {
        return;
    }

    m_titleCache.clear();
    collectNodeTitles(m_treeData);
    m_titleCacheRevision = m_treeDataRevision;
    ++m_titleCacheBuildCount;
    syncTreeSelectPerfCounters();
}

void AntTreeSelect::collectNodeTitles(const QVector<AntTreeNode>& nodes) const
{
    for (const auto& node : nodes)
    {
        m_titleCache.insert(node.key, node.title);
        if (!node.children.isEmpty())
        {
            collectNodeTitles(node.children);
        }
    }
}

void AntTreeSelect::updateTriggerRegion(const QRect& dirty, const QString& mode, bool clearScoped)
{
    QRect updateRect = dirty;
    if (!updateRect.isValid() || updateRect.isEmpty())
    {
        updateRect = rect();
    }
    ++m_triggerUpdateCount;
    if (clearScoped)
    {
        ++m_clearRegionUpdateCount;
    }
    setProperty("antTreeSelectLastUpdateMode", mode);
    syncTreeSelectPerfCounters();
    update(updateRect);
}

void AntTreeSelect::syncTreeSelectPerfCounters() const
{
    auto* self = const_cast<AntTreeSelect*>(this);
    self->setProperty("antTreeSelectTriggerLayoutBuildCount", m_triggerLayoutBuildCount);
    self->setProperty("antTreeSelectTriggerLayoutCacheHitCount", m_triggerLayoutCacheHitCount);
    self->setProperty("antTreeSelectVisibleRowsBuildCount", m_visibleRowsBuildCount);
    self->setProperty("antTreeSelectVisibleRowsCacheHitCount", m_visibleRowsCacheHitCount);
    self->setProperty("antTreeSelectVisibleRowCount", m_cachedVisibleRows);
    self->setProperty("antTreeSelectTitleCacheBuildCount", m_titleCacheBuildCount);
    self->setProperty("antTreeSelectTriggerUpdateCount", m_triggerUpdateCount);
    self->setProperty("antTreeSelectClearRegionUpdateCount", m_clearRegionUpdateCount);
}

QRect AntTreeSelect::triggerRect() const
{
    return triggerLayout().triggerRect;
}

QRect AntTreeSelect::clearButtonRect() const
{
    return triggerLayout().clearButtonRect;
}

QRect AntTreeSelect::arrowRect() const
{
    return triggerLayout().arrowRect;
}

bool AntTreeSelect::isOverClear(const QPoint& pos) const
{
    return clearButtonRect().contains(pos);
}
