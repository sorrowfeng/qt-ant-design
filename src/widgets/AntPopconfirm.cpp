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
    installAntStyle<AntPopconfirmStyle>(this);
    m_popover = new AntPopover(this);
    m_popover->setTrigger(Ant::PopoverTrigger::Click);
    m_popover->setTitle(QString());
    m_popover->setTitleIconType(Ant::IconType::ExclamationCircle);
    connect(m_popover, &AntPopover::openChanged, this, &AntPopconfirm::openChanged);

    rebuildActionWidget();
    syncPopoverContent();
    syncPopconfirmPerfCounters();
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
        m_popover->setTarget(nullptr);
    }
    else
    {
        m_popover->setTarget(m_target.data());
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
    return m_target.data();
}

void AntPopconfirm::setTarget(QWidget* target)
{
    if (m_target == target)
    {
        return;
    }
    m_target = target;
    m_popover->setTarget(m_disabled ? nullptr : m_target.data());
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
    const QString key = m_cancelText + QLatin1Char('|') + m_okText + QLatin1Char('|') +
                        QString::number(m_showCancel ? 1 : 0);
    if (m_actionContainer && m_actionSyncKey == key)
    {
        ++m_actionSyncSkipCount;
        m_lastSyncMode = QStringLiteral("actionSkip");
        syncPopconfirmPerfCounters();
        return;
    }

    if (!m_actionContainer)
    {
        m_actionContainer = new QWidget(m_popover);
        m_actionContainer->setObjectName(QStringLiteral("AntPopconfirmActionContainer"));
        auto* layout = new QHBoxLayout(m_actionContainer);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);

        m_cancelButton = new AntButton(m_cancelText, m_actionContainer);
        m_cancelButton->setObjectName(QStringLiteral("AntPopconfirmCancelButton"));
        m_cancelButton->setButtonType(Ant::ButtonType::Default);
        m_cancelButton->setButtonSize(Ant::Size::Small);
        connect(m_cancelButton, &AntButton::clicked, this, [this]() {
            m_popover->setOpen(false);
            Q_EMIT cancelRequested();
        });
        layout->addWidget(m_cancelButton);

        m_okButton = new AntButton(m_okText, m_actionContainer);
        m_okButton->setObjectName(QStringLiteral("AntPopconfirmOkButton"));
        m_okButton->setButtonType(Ant::ButtonType::Primary);
        m_okButton->setButtonSize(Ant::Size::Small);
        connect(m_okButton, &AntButton::clicked, this, [this]() {
            m_popover->setOpen(false);
            Q_EMIT confirmRequested();
        });
        layout->addWidget(m_okButton);

        ++m_actionBuildCount;
    }

    if (m_cancelButton)
    {
        m_cancelButton->setText(m_cancelText);
        m_cancelButton->setVisible(m_showCancel);
    }
    if (m_okButton)
    {
        m_okButton->setText(m_okText);
    }

    if (m_popover->actionWidget() != m_actionContainer)
    {
        m_popover->setActionWidget(m_actionContainer);
        ++m_actionAttachCount;
    }
    else
    {
        m_actionContainer->updateGeometry();
        m_popover->adjustSize();
        m_popover->update();
    }

    m_actionSyncKey = key;
    ++m_actionSyncApplyCount;
    m_lastSyncMode = QStringLiteral("action");
    syncPopconfirmPerfCounters();
}

void AntPopconfirm::syncPopoverContent()
{
    const QString titleText = m_title.isEmpty() ? QStringLiteral("Are you sure?") : m_title;
    if (m_syncedTitle == titleText && m_syncedDescription == m_description)
    {
        ++m_contentSyncSkipCount;
        m_lastSyncMode = QStringLiteral("contentSkip");
        syncPopconfirmPerfCounters();
        return;
    }

    m_popover->setTitle(titleText);
    m_popover->setContent(m_description);
    m_syncedTitle = titleText;
    m_syncedDescription = m_description;
    ++m_contentSyncApplyCount;
    m_lastSyncMode = QStringLiteral("content");
    syncPopconfirmPerfCounters();
}

void AntPopconfirm::syncPopconfirmPerfCounters() const
{
    auto* that = const_cast<AntPopconfirm*>(this);
    that->setProperty("antPopconfirmActionBuildCount", m_actionBuildCount);
    that->setProperty("antPopconfirmActionSyncApplyCount", m_actionSyncApplyCount);
    that->setProperty("antPopconfirmActionSyncSkipCount", m_actionSyncSkipCount);
    that->setProperty("antPopconfirmActionAttachCount", m_actionAttachCount);
    that->setProperty("antPopconfirmContentSyncApplyCount", m_contentSyncApplyCount);
    that->setProperty("antPopconfirmContentSyncSkipCount", m_contentSyncSkipCount);
    that->setProperty("antPopconfirmLastSyncMode", m_lastSyncMode);
}
