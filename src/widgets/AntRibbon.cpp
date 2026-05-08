#include "AntRibbon.h"

#include <QAction>
#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

#include "AntScrollBar.h"
#include "core/AntTheme.h"

namespace
{
constexpr int kGroupMinHeight = 132;
constexpr int kGroupTitleHeight = 24;
constexpr int kLargeButtonWidth = 92;
constexpr int kLargeButtonHeight = 92;
constexpr int kSmallButtonHeight = 34;
constexpr int kSmallColumnWidth = 156;
constexpr int kButtonRadius = 6;

class AntRibbonToolButton : public QToolButton
{
public:
    AntRibbonToolButton(QAction* action, Ant::RibbonItemSize size, QWidget* parent)
        : QToolButton(parent),
          m_size(size)
    {
        setDefaultAction(action);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(true);
        setAttribute(Qt::WA_Hover, true);
        setAutoRaise(false);
        setToolButtonStyle(size == Ant::RibbonItemSize::Large ? Qt::ToolButtonTextUnderIcon
                                                              : Qt::ToolButtonTextBesideIcon);
        setIconSize(size == Ant::RibbonItemSize::Large ? QSize(28, 28) : QSize(16, 16));
        setFixedSize(sizeHint());
    }

    QSize sizeHint() const override
    {
        return m_size == Ant::RibbonItemSize::Large ? QSize(kLargeButtonWidth, kLargeButtonHeight)
                                                    : QSize(kSmallColumnWidth, kSmallButtonHeight);
    }

    QSize minimumSizeHint() const override
    {
        return sizeHint();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        const auto& token = antTheme->tokens();
        const bool hovered = underMouse();
        const bool pressed = isDown();
        const bool enabled = isEnabled();

        QColor bg = Qt::transparent;
        QColor border = Qt::transparent;
        QColor text = enabled ? token.colorText : token.colorTextDisabled;
        if (hovered && enabled)
        {
            bg = token.colorFillTertiary;
            border = token.colorBorderSecondary;
            text = token.colorPrimaryHover;
        }
        if (pressed && enabled)
        {
            bg = token.colorFillSecondary;
            border = token.colorPrimaryActive;
            text = token.colorPrimaryActive;
        }

        const QRectF box = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
        painter.setPen(border.alpha() == 0 ? Qt::NoPen : QPen(border, 1));
        painter.setBrush(bg);
        painter.drawRoundedRect(box, kButtonRadius, kButtonRadius);

        QFont f = font();
        f.setPixelSize(m_size == Ant::RibbonItemSize::Large ? token.fontSize : token.fontSizeSM);
        painter.setFont(f);
        painter.setPen(text);

        const QString label = textForPaint();
        const QIcon actionIcon = icon();
        if (m_size == Ant::RibbonItemSize::Large)
        {
            QRect iconRect(rect().left() + 16, rect().top() + 14, rect().width() - 32, 30);
            if (!actionIcon.isNull())
            {
                actionIcon.paint(&painter,
                                 iconRect,
                                 Qt::AlignCenter,
                                 enabled ? QIcon::Normal : QIcon::Disabled,
                                 QIcon::Off);
            }
            QRect textRect = rect().adjusted(8, 48, -8, -8);
            painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, label);
        }
        else
        {
            QRect content = rect().adjusted(9, 0, -9, 0);
            if (!actionIcon.isNull())
            {
                const QRect iconRect(content.left(), content.center().y() - 8, 16, 16);
                actionIcon.paint(&painter,
                                 iconRect,
                                 Qt::AlignCenter,
                                 enabled ? QIcon::Normal : QIcon::Disabled,
                                 QIcon::Off);
                content.adjust(22, 0, 0, 0);
            }
            painter.drawText(content, Qt::AlignVCenter | Qt::AlignLeft, label);
        }
    }

private:
    QString textForPaint() const
    {
        return defaultAction() ? defaultAction()->text() : text();
    }

    Ant::RibbonItemSize m_size = Ant::RibbonItemSize::Large;
};

}

AntRibbonGroup::AntRibbonGroup(QWidget* parent)
    : QWidget(parent)
{
    init();
}

AntRibbonGroup::AntRibbonGroup(const QString& title, QWidget* parent)
    : QWidget(parent),
      m_title(title)
{
    init();
}

void AntRibbonGroup::init()
{
    setObjectName(QStringLiteral("AntRibbonGroup"));
    setAttribute(Qt::WA_Hover, true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setContentsMargins(10, 8, 10, 6);
    m_rootLayout->setSpacing(5);

    m_content = new QWidget(this);
    m_content->setAttribute(Qt::WA_TranslucentBackground, true);
    m_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_contentLayout = new QHBoxLayout(m_content);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(8);

    m_titleLabel = new QLabel(m_title, this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setFixedHeight(kGroupTitleHeight);

    m_rootLayout->addWidget(m_content, 1);
    m_rootLayout->addWidget(m_titleLabel);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        syncTheme();
        update();
    });
    syncTheme();
}

QString AntRibbonGroup::title() const
{
    return m_title;
}

void AntRibbonGroup::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    if (m_titleLabel)
    {
        m_titleLabel->setText(m_title);
    }
    updateGeometry();
    update();
    Q_EMIT titleChanged(m_title);
}

void AntRibbonGroup::addLargeAction(QAction* action)
{
    addActionInternal(action, Ant::RibbonItemSize::Large);
}

void AntRibbonGroup::addSmallAction(QAction* action)
{
    addActionInternal(action, Ant::RibbonItemSize::Small);
}

QAction* AntRibbonGroup::addAction(const QString& text, const QIcon& icon, Ant::RibbonItemSize size)
{
    auto* action = new QAction(icon, text, this);
    addActionInternal(action, size);
    return action;
}

QAction* AntRibbonGroup::addWidget(QWidget* widget, Ant::RibbonItemSize size)
{
    if (!widget)
    {
        return nullptr;
    }

    auto* action = new QWidgetAction(this);
    action->setText(widget->windowTitle());
    action->setProperty("antRibbonWidgetAction", true);
    addItemWidget(widget, size);
    return action;
}

void AntRibbonGroup::clearItems()
{
    for (QWidget* widget : std::as_const(m_itemWidgets))
    {
        if (widget)
        {
            widget->deleteLater();
        }
    }
    m_itemWidgets.clear();
    clearSmallColumns();
    m_itemCount = 0;
    updateGeometry();
    update();
}

int AntRibbonGroup::itemCount() const
{
    return m_itemCount;
}

QSize AntRibbonGroup::sizeHint() const
{
    return QSize(qMax(112, m_contentLayout ? m_contentLayout->sizeHint().width() + 20 : 112), kGroupMinHeight);
}

QSize AntRibbonGroup::minimumSizeHint() const
{
    return QSize(qMin(sizeHint().width(), 120), kGroupMinHeight);
}

void AntRibbonGroup::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const auto& token = antTheme->tokens();

    const QRectF bounds = rect().adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setPen(QPen(token.colorBorderSecondary, 1));
    painter.setBrush(token.colorBgContainer);
    painter.drawRoundedRect(bounds, token.borderRadiusSM, token.borderRadiusSM);

    const QRect titleRect(6, height() - kGroupTitleHeight - 4, width() - 12, kGroupTitleHeight);
    painter.setPen(token.colorBorderSecondary);
    painter.drawLine(titleRect.topLeft(), titleRect.topRight());
}

void AntRibbonGroup::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange)
    {
        syncTheme();
        updateGeometry();
        update();
    }
    QWidget::changeEvent(event);
}

void AntRibbonGroup::addActionInternal(QAction* action, Ant::RibbonItemSize size)
{
    if (!action)
    {
        return;
    }

    auto* button = new AntRibbonToolButton(action, size, this);
    button->setPopupMode(QToolButton::InstantPopup);

    connect(action, &QAction::triggered, this, [this, action]() {
        Q_EMIT actionTriggered(action);
    });
    addItemWidget(button, size);
}

void AntRibbonGroup::addItemWidget(QWidget* widget, Ant::RibbonItemSize size)
{
    if (!widget)
    {
        return;
    }

    widget->setParent(m_content);
    widget->setVisible(true);
    if (size == Ant::RibbonItemSize::Large)
    {
        widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        m_contentLayout->addWidget(widget, 0, Qt::AlignTop);
    }
    else
    {
        widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        if (widget->minimumHeight() < kSmallButtonHeight)
        {
            widget->setMinimumHeight(kSmallButtonHeight);
        }
        QVBoxLayout* column = ensureSmallColumn();
        column->addWidget(widget);
    }

    m_itemWidgets.append(widget);
    ++m_itemCount;
    updateGeometry();
    update();
}

QVBoxLayout* AntRibbonGroup::ensureSmallColumn()
{
    QWidget* columnWidget = nullptr;
    if (!m_smallColumns.isEmpty())
    {
        QWidget* last = m_smallColumns.last();
        if (last->property("antRibbonSmallCount").toInt() < 3)
        {
            columnWidget = last;
        }
    }

    if (!columnWidget)
    {
        columnWidget = new QWidget(m_content);
        columnWidget->setProperty("antRibbonSmallCount", 0);
        columnWidget->setMinimumWidth(kSmallColumnWidth);
        columnWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        auto* column = new QVBoxLayout(columnWidget);
        column->setContentsMargins(0, 0, 0, 0);
        column->setSpacing(5);
        m_contentLayout->addWidget(columnWidget, 0, Qt::AlignTop);
        m_smallColumns.append(columnWidget);
    }

    columnWidget->setProperty("antRibbonSmallCount", columnWidget->property("antRibbonSmallCount").toInt() + 1);
    return qobject_cast<QVBoxLayout*>(columnWidget->layout());
}

void AntRibbonGroup::clearSmallColumns()
{
    for (QWidget* column : std::as_const(m_smallColumns))
    {
        if (column)
        {
            column->deleteLater();
        }
    }
    m_smallColumns.clear();
}

void AntRibbonGroup::syncTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, isEnabled() ? token.colorText : token.colorTextDisabled);
    setPalette(pal);

    if (m_titleLabel)
    {
        QPalette labelPal = m_titleLabel->palette();
        labelPal.setColor(QPalette::WindowText, isEnabled() ? token.colorTextSecondary : token.colorTextDisabled);
        m_titleLabel->setPalette(labelPal);
        QFont f = m_titleLabel->font();
        f.setPixelSize(token.fontSizeSM);
        m_titleLabel->setFont(f);
    }
}

AntRibbonPage::AntRibbonPage(QWidget* parent)
    : QWidget(parent)
{
    init();
}

AntRibbonPage::AntRibbonPage(const QString& title, const QString& key, QWidget* parent)
    : QWidget(parent),
      m_title(title),
      m_key(key)
{
    init();
}

void AntRibbonPage::init()
{
    setObjectName(QStringLiteral("AntRibbonPage"));
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(12, 8, 12, 8);
    m_layout->setSpacing(10);
    m_layout->addStretch();
}

QString AntRibbonPage::title() const
{
    return m_title;
}

void AntRibbonPage::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    updateGeometry();
    Q_EMIT titleChanged(m_title);
}

QString AntRibbonPage::key() const
{
    return m_key;
}

void AntRibbonPage::setKey(const QString& key)
{
    if (m_key == key)
    {
        return;
    }
    m_key = key;
    Q_EMIT keyChanged(m_key);
}

AntRibbonGroup* AntRibbonPage::addGroup(const QString& title)
{
    return insertGroup(m_groups.size(), title);
}

AntRibbonGroup* AntRibbonPage::insertGroup(int index, const QString& title)
{
    auto* group = new AntRibbonGroup(title, this);
    index = qBound(0, index, m_groups.size());
    m_groups.insert(index, group);
    m_layout->insertWidget(index, group);
    connectGroup(group);
    updateGeometry();
    return group;
}

void AntRibbonPage::removeGroup(int index)
{
    if (index < 0 || index >= m_groups.size())
    {
        return;
    }
    AntRibbonGroup* group = m_groups.takeAt(index);
    m_layout->removeWidget(group);
    group->deleteLater();
    updateGeometry();
}

void AntRibbonPage::clearGroups()
{
    while (!m_groups.isEmpty())
    {
        removeGroup(m_groups.size() - 1);
    }
}

int AntRibbonPage::groupCount() const
{
    return m_groups.size();
}

AntRibbonGroup* AntRibbonPage::groupAt(int index) const
{
    return index >= 0 && index < m_groups.size() ? m_groups.at(index) : nullptr;
}

QSize AntRibbonPage::sizeHint() const
{
    return m_layout ? m_layout->sizeHint().expandedTo(QSize(1, kGroupMinHeight + 12)) : QSize(1, kGroupMinHeight + 12);
}

QSize AntRibbonPage::minimumSizeHint() const
{
    return sizeHint();
}

void AntRibbonPage::connectGroup(AntRibbonGroup* group)
{
    connect(group, &AntRibbonGroup::actionTriggered, this, &AntRibbonPage::actionTriggered);
}

AntRibbon::AntRibbon(QWidget* parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("AntRibbon"));
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_stack = new QStackedWidget;
    m_stack->setObjectName(QStringLiteral("AntRibbonStack"));
    m_stack->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName(QStringLiteral("AntRibbonScrollArea"));
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal));
    m_scrollArea->setWidget(m_stack);

    m_popup = new QFrame(this, Qt::Popup);
    m_popup->setObjectName(QStringLiteral("AntRibbonPopup"));
    m_popup->setAttribute(Qt::WA_Hover, true);
    m_popup->installEventFilter(this);
    auto* popupLayout = new QVBoxLayout(m_popup);
    popupLayout->setContentsMargins(0, 0, 0, 0);
    popupLayout->setSpacing(0);

    m_indicatorAnimation = new QPropertyAnimation(this, "indicatorRect", this);
    m_indicatorAnimation->setDuration(180);
    m_indicatorAnimation->setEasingCurve(QEasingCurve::OutCubic);

    m_contentAnimation = new QPropertyAnimation(this, "contentHeight", this);
    m_contentAnimation->setDuration(220);
    m_contentAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_contentAnimation, &QPropertyAnimation::finished, this, [this]() {
        updateScrollAreaGeometry();
        updateGeometry();
    });

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        syncTheme();
        update();
    });
    syncTheme();
}

AntRibbonPage* AntRibbon::addPage(const QString& title, const QString& key)
{
    return insertPage(m_pages.size(), title, key);
}

AntRibbonPage* AntRibbon::insertPage(int index, const QString& title, const QString& key)
{
    auto* page = new AntRibbonPage(title, normalizedKey(title, key));
    index = qBound(0, index, m_pages.size());
    m_pages.insert(index, page);
    m_stack->insertWidget(index, page);
    connectPage(page);

    if (m_currentPageIndex < 0)
    {
        setCurrentPageIndex(index);
    }
    else if (index <= m_currentPageIndex)
    {
        ++m_currentPageIndex;
        syncIndicatorRect();
    }

    updateStackSize();
    updateGeometry();
    update();
    return page;
}

void AntRibbon::removePage(const QString& key)
{
    for (int i = 0; i < m_pages.size(); ++i)
    {
        if (m_pages.at(i)->key() == key)
        {
            AntRibbonPage* page = m_pages.takeAt(i);
            m_stack->removeWidget(page);
            page->deleteLater();
            if (m_pages.isEmpty())
            {
                m_currentPageIndex = -1;
                if (m_indicatorAnimation)
                {
                    m_indicatorAnimation->stop();
                }
                m_indicatorReady = false;
                setIndicatorRect(QRectF());
                Q_EMIT currentPageChanged(-1);
                Q_EMIT currentPageKeyChanged(QString());
            }
            else if (m_currentPageIndex >= m_pages.size())
            {
                setCurrentPageIndex(m_pages.size() - 1);
            }
            else if (i == m_currentPageIndex)
            {
                updatePageVisibility();
                syncIndicatorRect();
                Q_EMIT currentPageChanged(m_currentPageIndex);
                Q_EMIT currentPageKeyChanged(currentPageKey());
            }
            updateStackSize();
            updateGeometry();
            update();
            return;
        }
    }
}

void AntRibbon::clearPages()
{
    while (!m_pages.isEmpty())
    {
        AntRibbonPage* page = m_pages.takeLast();
        m_stack->removeWidget(page);
        page->deleteLater();
    }
    m_currentPageIndex = -1;
    if (m_indicatorAnimation)
    {
        m_indicatorAnimation->stop();
    }
    m_indicatorReady = false;
    setIndicatorRect(QRectF());
    hidePopup();
    updateStackSize();
    updateGeometry();
    update();
    Q_EMIT currentPageChanged(-1);
    Q_EMIT currentPageKeyChanged(QString());
}

int AntRibbon::pageCount() const
{
    return m_pages.size();
}

AntRibbonPage* AntRibbon::pageAt(int index) const
{
    return index >= 0 && index < m_pages.size() ? m_pages.at(index) : nullptr;
}

AntRibbonPage* AntRibbon::pageByKey(const QString& key) const
{
    for (AntRibbonPage* page : m_pages)
    {
        if (page && page->key() == key)
        {
            return page;
        }
    }
    return nullptr;
}

int AntRibbon::currentPageIndex() const
{
    return m_currentPageIndex;
}

void AntRibbon::setCurrentPageIndex(int index)
{
    if (index < 0 || index >= m_pages.size() || m_currentPageIndex == index)
    {
        return;
    }

    const QRectF previousIndicator =
        m_indicatorReady ? m_indicatorRect : targetIndicatorRect(m_currentPageIndex);
    m_currentPageIndex = index;
    updatePageVisibility();
    updateStackSize();
    animateIndicator(previousIndicator, targetIndicatorRect(m_currentPageIndex));
    update();
    Q_EMIT currentPageChanged(m_currentPageIndex);
    Q_EMIT currentPageKeyChanged(currentPageKey());
}

QString AntRibbon::currentPageKey() const
{
    AntRibbonPage* page = pageAt(m_currentPageIndex);
    return page ? page->key() : QString();
}

void AntRibbon::setCurrentPageKey(const QString& key)
{
    for (int i = 0; i < m_pages.size(); ++i)
    {
        if (m_pages.at(i)->key() == key)
        {
            setCurrentPageIndex(i);
            return;
        }
    }
}

bool AntRibbon::isCollapsed() const
{
    return m_collapsed;
}

void AntRibbon::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed)
    {
        return;
    }

    m_collapsed = collapsed;
    if (!m_collapsed)
    {
        hidePopup();
        restoreScrollAreaParent();
        if (m_scrollArea)
        {
            m_scrollArea->setVisible(true);
        }
    }

    if (m_contentAnimation)
    {
        m_contentAnimation->stop();
        m_contentAnimation->setStartValue(m_contentHeight);
        m_contentAnimation->setEndValue(m_collapsed ? 0.0 : qreal(ContentHeight));
        m_contentAnimation->start();
    }
    else
    {
        setContentHeight(m_collapsed ? 0.0 : qreal(ContentHeight));
    }
    update();
    Q_EMIT collapsedChanged(m_collapsed);
}

bool AntRibbon::isCollapseButtonVisible() const
{
    return m_collapseButtonVisible;
}

void AntRibbon::setCollapseButtonVisible(bool visible)
{
    if (m_collapseButtonVisible == visible)
    {
        return;
    }
    m_collapseButtonVisible = visible;
    syncIndicatorRect();
    update();
    Q_EMIT collapseButtonVisibleChanged(m_collapseButtonVisible);
}

QRectF AntRibbon::indicatorRect() const
{
    return m_indicatorRect;
}

void AntRibbon::setIndicatorRect(const QRectF& rect)
{
    if (m_indicatorRect == rect)
    {
        return;
    }
    m_indicatorRect = rect;
    update();
}

qreal AntRibbon::contentHeight() const
{
    return m_contentHeight;
}

void AntRibbon::setContentHeight(qreal height)
{
    const qreal normalized = qBound<qreal>(0.0, height, qreal(ContentHeight));
    if (qFuzzyCompare(m_contentHeight + 1.0, normalized + 1.0))
    {
        return;
    }
    m_contentHeight = normalized;
    updateScrollAreaGeometry();
    updateGeometry();
    update();
}

QSize AntRibbon::sizeHint() const
{
    return QSize(760, TabBarHeight + qRound(m_contentHeight));
}

QSize AntRibbon::minimumSizeHint() const
{
    return QSize(260, sizeHint().height());
}

bool AntRibbon::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_popup && event->type() == QEvent::Hide)
    {
        restoreScrollAreaParent();
    }
    return QWidget::eventFilter(watched, event);
}

void AntRibbon::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const auto& token = antTheme->tokens();

    painter.fillRect(rect(), token.colorBgContainer);
    painter.fillRect(QRect(0, 0, width(), TabBarHeight), token.colorBgElevated);
    painter.setPen(QPen(token.colorBorderSecondary, 1));
    painter.drawLine(0, TabBarHeight - 1, width(), TabBarHeight - 1);
    if (m_contentHeight > 0.5)
    {
        const int contentBottom = TabBarHeight + qRound(m_contentHeight) - 1;
        painter.drawLine(0, contentBottom, width(), contentBottom);
    }

    const QVector<QRect> tabs = tabRects();
    for (int i = 0; i < tabs.size(); ++i)
    {
        const QRect rect = tabs.at(i);
        const bool active = i == m_currentPageIndex;
        const bool hovered = i == m_hoveredTab;
        if (active || hovered)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(active ? token.colorBgContainer : token.colorFillSecondary);
            painter.drawRoundedRect(QRectF(rect).adjusted(0.5, 6.5, -0.5, 4.5), token.borderRadiusSM, token.borderRadiusSM);
        }

        painter.setPen(active ? token.colorPrimary : token.colorText);
        QFont f = painter.font();
        f.setPixelSize(token.fontSize);
        f.setWeight(active ? QFont::DemiBold : QFont::Normal);
        painter.setFont(f);
        painter.drawText(rect.adjusted(12, 0, -12, 0), Qt::AlignCenter, m_pages.at(i)->title());

    }

    if (!m_indicatorRect.isNull() && m_indicatorRect.isValid())
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorPrimary);
        painter.drawRoundedRect(m_indicatorRect, 1.0, 1.0);
    }

    if (m_collapseButtonVisible)
    {
        const QRect buttonRect = collapseButtonRect();
        if (m_collapseHovered)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(token.colorFillSecondary);
            painter.drawRoundedRect(QRectF(buttonRect).adjusted(4.5, 7.5, -4.5, -7.5), token.borderRadiusSM, token.borderRadiusSM);
        }
        painter.setPen(QPen(token.colorTextSecondary, 1.5));
        const QPoint c = buttonRect.center();
        QPolygon poly;
        if (m_collapsed)
        {
            poly << QPoint(c.x() - 5, c.y() - 2) << QPoint(c.x(), c.y() + 3) << QPoint(c.x() + 5, c.y() - 2);
        }
        else
        {
            poly << QPoint(c.x() - 5, c.y() + 2) << QPoint(c.x(), c.y() - 3) << QPoint(c.x() + 5, c.y() + 2);
        }
        painter.drawPolyline(poly);
    }
}

void AntRibbon::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateScrollAreaGeometry();
    updateStackSize();
    if (!m_indicatorAnimation || m_indicatorAnimation->state() != QAbstractAnimation::Running)
    {
        syncIndicatorRect();
    }
}

void AntRibbon::mouseMoveEvent(QMouseEvent* event)
{
    const int oldTab = m_hoveredTab;
    const bool oldCollapse = m_collapseHovered;
    m_hoveredTab = tabAt(event->pos());
    m_collapseHovered = m_collapseButtonVisible && collapseButtonRect().contains(event->pos());
    if (oldTab != m_hoveredTab || oldCollapse != m_collapseHovered)
    {
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntRibbon::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (m_collapseButtonVisible && collapseButtonRect().contains(event->pos()))
        {
            setCollapsed(!m_collapsed);
            return;
        }

        const int tab = tabAt(event->pos());
        if (tab >= 0)
        {
            setCurrentPageIndex(tab);
            Q_EMIT pageClicked(currentPageKey());
            if (m_collapsed)
            {
                showCurrentPagePopup();
            }
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntRibbon::leaveEvent(QEvent* event)
{
    m_hoveredTab = -1;
    m_collapseHovered = false;
    update();
    QWidget::leaveEvent(event);
}

void AntRibbon::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange)
    {
        syncTheme();
        update();
    }
    QWidget::changeEvent(event);
}

QString AntRibbon::normalizedKey(const QString& title, const QString& key) const
{
    QString base = key.isEmpty() ? title.toLower().simplified().replace(QLatin1Char(' '), QLatin1Char('-')) : key;
    if (base.isEmpty())
    {
        base = QStringLiteral("page");
    }

    QString candidate = base;
    int suffix = 2;
    while (pageByKey(candidate))
    {
        candidate = QStringLiteral("%1-%2").arg(base).arg(suffix++);
    }
    return candidate;
}

QVector<QRect> AntRibbon::tabRects() const
{
    QVector<QRect> rects;
    QFontMetrics fm(font());
    int x = 8;
    const int rightLimit = m_collapseButtonVisible ? width() - CollapseButtonWidth - 8 : width() - 8;
    for (AntRibbonPage* page : m_pages)
    {
        const int tabWidth = qMax(72, fm.horizontalAdvance(page->title()) + 32);
        if (x + tabWidth > rightLimit)
        {
            rects.append(QRect(x, 0, qMax(48, rightLimit - x), TabBarHeight));
            break;
        }
        rects.append(QRect(x, 0, tabWidth, TabBarHeight));
        x += tabWidth + 4;
    }
    return rects;
}

QRect AntRibbon::collapseButtonRect() const
{
    return QRect(width() - CollapseButtonWidth - 4, 0, CollapseButtonWidth, TabBarHeight);
}

int AntRibbon::tabAt(const QPoint& pos) const
{
    const QVector<QRect> tabs = tabRects();
    for (int i = 0; i < tabs.size(); ++i)
    {
        if (tabs.at(i).contains(pos))
        {
            return i;
        }
    }
    return -1;
}

QRectF AntRibbon::targetIndicatorRect(int index) const
{
    const QVector<QRect> tabs = tabRects();
    if (index < 0 || index >= tabs.size())
    {
        return QRectF();
    }

    const QRect tab = tabs.at(index);
    const int inset = qMin(14, qMax(8, tab.width() / 5));
    return QRectF(tab.left() + inset, TabBarHeight - 3, qMax(8, tab.width() - inset * 2), 2);
}

void AntRibbon::syncIndicatorRect()
{
    if (m_indicatorAnimation)
    {
        m_indicatorAnimation->stop();
    }
    const QRectF target = targetIndicatorRect(m_currentPageIndex);
    m_indicatorReady = !target.isNull();
    setIndicatorRect(target);
}

void AntRibbon::animateIndicator(const QRectF& from, const QRectF& to)
{
    if (to.isNull())
    {
        if (m_indicatorAnimation)
        {
            m_indicatorAnimation->stop();
        }
        m_indicatorReady = false;
        setIndicatorRect(QRectF());
        return;
    }

    if (!m_indicatorAnimation || !m_indicatorReady || from.isNull())
    {
        m_indicatorReady = true;
        setIndicatorRect(to);
        return;
    }

    m_indicatorAnimation->stop();
    m_indicatorReady = true;
    m_indicatorAnimation->setStartValue(from);
    m_indicatorAnimation->setEndValue(to);
    m_indicatorAnimation->start();
}

void AntRibbon::updatePageVisibility()
{
    if (m_currentPageIndex >= 0 && m_currentPageIndex < m_stack->count())
    {
        m_stack->setCurrentIndex(m_currentPageIndex);
    }
}

void AntRibbon::updateStackSize()
{
    AntRibbonPage* page = pageAt(m_currentPageIndex);
    const QSize pageHint = page ? page->sizeHint() : QSize(1, ContentHeight);
    const int targetWidth = qMax(pageHint.width(), m_scrollArea ? m_scrollArea->viewport()->width() : width());
    m_stack->setFixedSize(targetWidth, ContentHeight);
}

void AntRibbon::updateScrollAreaGeometry()
{
    if (m_scrollAreaInPopup)
    {
        return;
    }
    m_scrollArea->setParent(this);
    const int height = qRound(m_contentHeight);
    m_scrollArea->setVisible(height > 0);
    m_scrollArea->setGeometry(0, TabBarHeight, width(), height);
}

void AntRibbon::showCurrentPagePopup()
{
    if (!m_collapsed || !m_popup || !m_scrollArea)
    {
        return;
    }

    restoreScrollAreaParent();
    m_scrollArea->setParent(m_popup);
    m_scrollArea->setMinimumHeight(ContentHeight);
    m_scrollArea->setMaximumHeight(ContentHeight);
    auto* layout = qobject_cast<QVBoxLayout*>(m_popup->layout());
    layout->addWidget(m_scrollArea);
    m_scrollAreaInPopup = true;
    m_scrollArea->setVisible(true);
    updateStackSize();

    const int popupHeight = ContentHeight;
    m_popup->resize(width(), popupHeight);
    m_popup->move(mapToGlobal(QPoint(0, TabBarHeight)));
    m_popup->show();
    m_popup->raise();
}

void AntRibbon::hidePopup()
{
    if (m_popup && m_popup->isVisible())
    {
        m_popup->hide();
    }
    restoreScrollAreaParent();
}

void AntRibbon::restoreScrollAreaParent()
{
    if (!m_scrollAreaInPopup || !m_scrollArea)
    {
        return;
    }

    if (auto* layout = qobject_cast<QVBoxLayout*>(m_popup->layout()))
    {
        layout->removeWidget(m_scrollArea);
    }
    m_scrollArea->setParent(this);
    m_scrollArea->setMinimumHeight(0);
    m_scrollArea->setMaximumHeight(QWIDGETSIZE_MAX);
    m_scrollAreaInPopup = false;
    updateScrollAreaGeometry();
}

void AntRibbon::connectPage(AntRibbonPage* page)
{
    connect(page, &AntRibbonPage::titleChanged, this, [this]() {
        updateGeometry();
        update();
    });
    connect(page, &AntRibbonPage::keyChanged, this, [this]() {
        Q_EMIT currentPageKeyChanged(currentPageKey());
    });
    connect(page, &AntRibbonPage::actionTriggered, this, [this](QAction* action) {
        if (m_collapsed)
        {
            hidePopup();
        }
        Q_EMIT actionTriggered(action);
    });
}

void AntRibbon::syncTheme()
{
    const auto& token = antTheme->tokens();
    QPalette pal = palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::WindowText, token.colorText);
    setPalette(pal);
    if (m_popup)
    {
        QPalette popupPal = m_popup->palette();
        popupPal.setColor(QPalette::Window, token.colorBgContainer);
        m_popup->setPalette(popupPal);
        m_popup->setAutoFillBackground(true);
    }
}
