#include "AntStackedWidget.h"

#include <QEvent>
#include <QPalette>
#include <QSizePolicy>

#include "../styles/AntStackedWidgetStyle.h"
#include "core/AntTheme.h"

AntStackedWidget::AntStackedWidget(QWidget* parent)
    : QStackedWidget(parent)
{
    installAntStyle<AntStackedWidgetStyle>(this);
    setAttribute(Qt::WA_Hover);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    updateFrameMargins();
    syncStackedWidgetPerfCounters();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateFrameMargins();
        update();
    });
}

Ant::Variant AntStackedWidget::variant() const
{
    return m_variant;
}

void AntStackedWidget::setVariant(Ant::Variant variant)
{
    if (m_variant == variant)
    {
        return;
    }

    m_variant = variant;
    updateFrameMargins();
    update();
    syncStackedWidgetPerfCounters();
    Q_EMIT variantChanged(m_variant);
}

bool AntStackedWidget::event(QEvent* event)
{
    if (event->type() == QEvent::LayoutRequest ||
        event->type() == QEvent::StyleChange ||
        event->type() == QEvent::Polish)
    {
        updateFrameMargins();
    }
    return QStackedWidget::event(event);
}

void AntStackedWidget::updateFrameMargins()
{
    const int lineWidth = antTheme->tokens().lineWidth;
    const int margin = m_variant == Ant::Variant::Borderless ? 0 : qMax(1, lineWidth);
    const QMargins current = contentsMargins();
    const QMargins desired(margin, margin, margin, margin);
    if (current == desired)
    {
        return;
    }

    setContentsMargins(desired);
    ++m_marginRefreshCount;
    syncStackedWidgetPerfCounters();
}

void AntStackedWidget::syncStackedWidgetPerfCounters() const
{
    auto* self = const_cast<AntStackedWidget*>(this);
    self->setProperty("antStackedWidgetVariant", static_cast<int>(m_variant));
    self->setProperty("antStackedWidgetMarginRefreshCount", m_marginRefreshCount);
}
