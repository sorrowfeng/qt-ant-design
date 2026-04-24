#include "AntPopconfirm.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "AntButton.h"
#include "AntPopover.h"
#include "core/AntTheme.h"
#include "styles/AntPopconfirmStyle.h"

AntPopconfirm::AntPopconfirm(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntPopconfirmStyle(style()));
    m_popover = new AntPopover(this);
    m_popover->setTrigger(Ant::PopoverTrigger::Click);
    m_popover->setTitle(QString());
    connect(m_popover, &AntPopover::openChanged, this, &AntPopconfirm::openChanged);

    rebuildActionWidget();
    syncPopoverContent();
}

QString AntPopconfirm::title() const { return m_title; }

void AntPopconfirm::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    syncPopoverContent();
    Q_EMIT titleChanged(m_title);
}

QString AntPopconfirm::description() const { return m_description; }

void AntPopconfirm::setDescription(const QString& description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    syncPopoverContent();
    Q_EMIT descriptionChanged(m_description);
}

QString AntPopconfirm::okText() const { return m_okText; }

void AntPopconfirm::setOkText(const QString& text)
{
    if (m_okText == text)
    {
        return;
    }
    m_okText = text;
    rebuildActionWidget();
    Q_EMIT okTextChanged(m_okText);
}

QString AntPopconfirm::cancelText() const { return m_cancelText; }

void AntPopconfirm::setCancelText(const QString& text)
{
    if (m_cancelText == text)
    {
        return;
    }
    m_cancelText = text;
    rebuildActionWidget();
    Q_EMIT cancelTextChanged(m_cancelText);
}

bool AntPopconfirm::showCancel() const { return m_showCancel; }

void AntPopconfirm::setShowCancel(bool show)
{
    if (m_showCancel == show)
    {
        return;
    }
    m_showCancel = show;
    rebuildActionWidget();
    Q_EMIT showCancelChanged(m_showCancel);
}

bool AntPopconfirm::isDisabled() const { return m_disabled; }

void AntPopconfirm::setDisabled(bool disabled)
{
    if (m_disabled == disabled)
    {
        return;
    }
    m_disabled = disabled;
    if (m_disabled)
    {
        m_popover->setOpen(false);
    }
    Q_EMIT disabledChanged(m_disabled);
}

Ant::TooltipPlacement AntPopconfirm::placement() const
{
    return m_popover->placement();
}

void AntPopconfirm::setPlacement(Ant::TooltipPlacement placement)
{
    if (m_popover->placement() == placement)
    {
        return;
    }
    m_popover->setPlacement(placement);
    Q_EMIT placementChanged(placement);
}

QWidget* AntPopconfirm::target() const
{
    return m_popover->target();
}

void AntPopconfirm::setTarget(QWidget* target)
{
    m_popover->setTarget(target);
}

bool AntPopconfirm::isOpen() const
{
    return m_popover->isOpen();
}

void AntPopconfirm::setOpen(bool open)
{
    if (m_disabled)
    {
        m_popover->setOpen(false);
        return;
    }
    m_popover->setOpen(open);
}

void AntPopconfirm::rebuildActionWidget()
{
    if (m_actionContainer)
    {
        m_actionContainer->deleteLater();
        m_actionContainer = nullptr;
        m_cancelButton = nullptr;
        m_okButton = nullptr;
    }

    m_actionContainer = new QWidget(m_popover);
    auto* layout = new QHBoxLayout(m_actionContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    if (m_showCancel)
    {
        m_cancelButton = new AntButton(m_cancelText, m_actionContainer);
        m_cancelButton->setButtonType(Ant::ButtonType::Default);
        m_cancelButton->setButtonSize(Ant::ButtonSize::Small);
        connect(m_cancelButton, &AntButton::clicked, this, [this]() {
            m_popover->setOpen(false);
            Q_EMIT cancelRequested();
        });
        layout->addWidget(m_cancelButton);
    }

    m_okButton = new AntButton(m_okText, m_actionContainer);
    m_okButton->setButtonType(Ant::ButtonType::Primary);
    m_okButton->setButtonSize(Ant::ButtonSize::Small);
    connect(m_okButton, &AntButton::clicked, this, [this]() {
        m_popover->setOpen(false);
        Q_EMIT confirmRequested();
    });
    layout->addWidget(m_okButton);

    m_popover->setActionWidget(m_actionContainer);
}

void AntPopconfirm::syncPopoverContent()
{
    const QString titleText = m_title.isEmpty() ? QStringLiteral("Are you sure?") : m_title;
    m_popover->setTitle(titleText);
    m_popover->setContent(m_description);
}
