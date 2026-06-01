#include "AntToolBar.h"

#include <QAction>
#include <QActionEvent>
#include <QLayout>
#include <QPalette>
#include <QToolButton>
#include <QVariant>

#include "../styles/AntToolBarStyle.h"
#include "AntToolTip.h"
#include "core/AntTheme.h"

namespace
{
QString colorKey(const QColor& color)
{
    return QString::number(color.rgba(), 16);
}

QString cleanActionText(QString text)
{
    const QChar escapedAmpersand(1);
    text.replace(QStringLiteral("&&"), QString(escapedAmpersand));
    text.remove(QLatin1Char('&'));
    text.replace(QString(escapedAmpersand), QStringLiteral("&"));
    return text.trimmed();
}

QString actionToolTipText(const QToolButton* button)
{
    if (!button)
    {
        return {};
    }

    const QAction* action = button->defaultAction();
    if (action)
    {
        const QString actionToolTip = action->toolTip().trimmed();
        if (!actionToolTip.isEmpty())
        {
            return actionToolTip;
        }

        const QString statusTip = action->statusTip().trimmed();
        if (!statusTip.isEmpty())
        {
            return statusTip;
        }

        const QString text = cleanActionText(action->text());
        if (!text.isEmpty())
        {
            return text;
        }
    }

    const QString buttonToolTip = button->toolTip().trimmed();
    if (!buttonToolTip.isEmpty())
    {
        return buttonToolTip;
    }

    return cleanActionText(button->text());
}

AntToolTip* ensureActionToolTip(QToolButton* button)
{
    if (!button)
    {
        return nullptr;
    }

    auto* toolTip = button->findChild<AntToolTip*>(QStringLiteral("AntToolBarActionToolTip"),
                                                   Qt::FindDirectChildrenOnly);
    if (!toolTip)
    {
        toolTip = new AntToolTip(button);
        toolTip->setObjectName(QStringLiteral("AntToolBarActionToolTip"));
        toolTip->setPlacement(Ant::TooltipPlacement::Top);
        toolTip->setTarget(button);
    }
    return toolTip;
}

void syncActionToolTip(QToolButton* button)
{
    AntToolTip* toolTip = ensureActionToolTip(button);
    if (!button || !toolTip)
    {
        return;
    }

    const QString title = actionToolTipText(button);
    toolTip->setTitle(title);
    button->setProperty("antToolBarActionToolTipText", title);
    if (!button->toolTip().isEmpty())
    {
        button->setToolTip(QString());
    }
}

void connectActionToolTipSync(QToolButton* button)
{
    if (!button || button->property("antToolBarActionToolTipConnection").toBool())
    {
        return;
    }

    QAction* action = button->defaultAction();
    if (!action)
    {
        return;
    }

    QObject::connect(action, &QAction::changed, button, [button]() {
        syncActionToolTip(button);
    });
    button->setProperty("antToolBarActionToolTipConnection", true);
}
} // namespace

AntToolBar::AntToolBar(QWidget* parent)
    : QToolBar(parent)
{
    auto* s = new AntToolBarStyle(style());
    s->setParent(this);
    setStyle(s);

    setMovable(true);
    setFloatable(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_TranslucentBackground);

    if (auto* lay = layout())
    {
        lay->setSpacing(8);
        lay->setContentsMargins(4, 4, 4, 4);
    }

    updateToolBarPalette(true);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateToolBarPalette();
        updateActionButtons();
        update();
    });

    connect(this, &QToolBar::topLevelChanged, this, [this](bool floating) {
        Q_UNUSED(floating)
        update();
    });
}

AntToolBar::AntToolBar(const QString& title, QWidget* parent)
    : AntToolBar(parent)
{
    setWindowTitle(title);
}

void AntToolBar::actionEvent(QActionEvent* event)
{
    QToolBar::actionEvent(event);
    if (!event)
    {
        return;
    }

    if (event->type() == QEvent::ActionAdded || event->type() == QEvent::ActionChanged)
    {
        auto* button = qobject_cast<QToolButton*>(widgetForAction(event->action()));
        if (button)
        {
            updateActionButton(button, true);
            return;
        }
        updateActionButtons(true);
        return;
    }

    if (event->type() == QEvent::ActionRemoved)
    {
        update();
    }
}

bool AntToolBar::updateToolBarPalette(bool force)
{
    const auto& token = antTheme->tokens();
    const QString nextKey = toolBarPaletteKey();
    if (!force && m_themedPaletteKey == nextKey)
    {
        return false;
    }

    QPalette nextPalette = palette();
    nextPalette.setColor(QPalette::ButtonText, token.colorText);
    nextPalette.setColor(QPalette::WindowText, token.colorText);
    nextPalette.setColor(QPalette::Button, token.colorBgContainer);
    nextPalette.setColor(QPalette::Text, token.colorText);

    const bool changed = force || m_themedPaletteKey != nextKey || m_themedPalette != nextPalette;
    m_themedPalette = nextPalette;
    m_themedPaletteKey = nextKey;
    if (changed)
    {
        setPalette(m_themedPalette);
        ++m_paletteApplyCount;
        syncToolBarPerfCounters();
    }
    return changed;
}

QString AntToolBar::toolBarPaletteKey() const
{
    const auto& token = antTheme->tokens();
    return colorKey(token.colorText) + QLatin1Char('|') +
           colorKey(token.colorBgContainer);
}

void AntToolBar::updateActionButtons(bool forceGeometry)
{
    ++m_buttonScanCount;
    const auto buttons = findChildren<QToolButton*>(QString(), Qt::FindDirectChildrenOnly);
    for (QToolButton* button : buttons)
    {
        updateActionButton(button, forceGeometry);
    }
    syncToolBarPerfCounters();
}

void AntToolBar::updateActionButton(QToolButton* button, bool forceGeometry)
{
    if (!button)
    {
        return;
    }

    if (m_themedPaletteKey.isEmpty())
    {
        updateToolBarPalette(true);
    }

    bool geometryChanged = forceGeometry;
    bool visualChanged = forceGeometry;

    if (!button->property("antToolBarButton").toBool())
    {
        button->setProperty("antToolBarButton", true);
        geometryChanged = true;
        visualChanged = true;
    }
    if (button->autoRaise())
    {
        button->setAutoRaise(false);
        visualChanged = true;
    }
    if (button->toolButtonStyle() != Qt::ToolButtonTextBesideIcon)
    {
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        geometryChanged = true;
        visualChanged = true;
    }
    if (button->cursor().shape() != Qt::PointingHandCursor)
    {
        button->setCursor(Qt::PointingHandCursor);
    }
    if (!button->hasMouseTracking())
    {
        button->setMouseTracking(true);
    }
    if (!button->testAttribute(Qt::WA_Hover))
    {
        button->setAttribute(Qt::WA_Hover, true);
    }
    if (button->property("antToolBarPaletteKey").toString() != m_themedPaletteKey)
    {
        button->setPalette(m_themedPalette);
        button->setProperty("antToolBarPaletteKey", m_themedPaletteKey);
        visualChanged = true;
    }
    if (button->style() != style())
    {
        button->setStyle(style());
        geometryChanged = true;
        visualChanged = true;
    }
    connectActionToolTipSync(button);
    syncActionToolTip(button);

    ++m_buttonSyncCount;
    if (geometryChanged)
    {
        button->setProperty("antToolBarButtonTextMetricKey", QVariant());
        button->updateGeometry();
        ++m_buttonGeometryUpdateCount;
    }
    if (geometryChanged || visualChanged)
    {
        button->update();
        ++m_buttonPaintUpdateCount;
    }
    syncToolBarPerfCounters();
}

void AntToolBar::syncToolBarPerfCounters() const
{
    auto* self = const_cast<AntToolBar*>(this);
    self->setProperty("antToolBarButtonScanCount", m_buttonScanCount);
    self->setProperty("antToolBarButtonSyncCount", m_buttonSyncCount);
    self->setProperty("antToolBarButtonGeometryUpdateCount", m_buttonGeometryUpdateCount);
    self->setProperty("antToolBarButtonPaintUpdateCount", m_buttonPaintUpdateCount);
    self->setProperty("antToolBarPaletteApplyCount", m_paletteApplyCount);
}
