#include "AntResult.h"

#include <QFontMetrics>
#include <QPainter>
#include <QResizeEvent>

#include "AntIcon.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"
#include "styles/AntResultStyle.h"

AntResult::AntResult(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntResultStyle(style()));
}

AntResult::AntResult(const QString& title, QWidget* parent)
    : AntResult(parent)
{
    m_title = title;
}

Ant::AlertType AntResult::status() const { return m_status; }

void AntResult::setStatus(Ant::AlertType status)
{
    if (m_status == status)
    {
        return;
    }
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

QString AntResult::title() const { return m_title; }

void AntResult::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    updateGeometry();
    update();
    Q_EMIT titleChanged(m_title);
}

QString AntResult::subTitle() const { return m_subTitle; }

void AntResult::setSubTitle(const QString& subTitle)
{
    if (m_subTitle == subTitle)
    {
        return;
    }
    m_subTitle = subTitle;
    updateGeometry();
    update();
    Q_EMIT subTitleChanged(m_subTitle);
}

bool AntResult::isIconVisible() const { return m_iconVisible; }

void AntResult::setIconVisible(bool visible)
{
    if (m_iconVisible == visible)
    {
        return;
    }
    m_iconVisible = visible;
    updateGeometry();
    update();
    Q_EMIT iconVisibleChanged(m_iconVisible);
}

QWidget* AntResult::extraWidget() const
{
    return m_extraWidget.data();
}

void AntResult::setExtraWidget(QWidget* widget)
{
    if (m_extraWidget == widget)
    {
        return;
    }
    if (m_extraWidget)
    {
        m_extraWidget->setParent(nullptr);
    }
    m_extraWidget = widget;
    if (m_extraWidget)
    {
        m_extraWidget->setParent(this);
        m_extraWidget->show();
    }
    syncExtraGeometry();
    updateGeometry();
    update();
}

QSize AntResult::sizeHint() const
{
    const Metrics m = metrics();
    const auto& token = antTheme->tokens();
    const int contentWidth = qMax(240, QWidget::width() > 0 ? QWidget::width() : 400);

    QFont titleFont = font();
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(QFont::DemiBold);
    QFontMetrics titleFm(titleFont);

    QFont subFont = font();
    subFont.setPixelSize(m.subTitleFontSize);
    QFontMetrics subFm(subFont);

    int height = m.padding;

    if (m_iconVisible)
    {
        height += m.iconSize + m.spacing;
    }

    const int textWidth = qMax(120, contentWidth - m.padding * 2);
    height += titleFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_title).height();

    if (!m_subTitle.isEmpty())
    {
        height += m.spacing;
        height += subFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_subTitle).height();
    }

    if (m_extraWidget)
    {
        height += m.extraSpacing;
        height += m_extraWidget->sizeHint().height();
    }

    height += m.padding;

    return QSize(contentWidth, height);
}

QSize AntResult::minimumSizeHint() const
{
    return QSize(180, 120);
}

void AntResult::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntResult::resizeEvent(QResizeEvent* event)
{
    syncExtraGeometry();
    QWidget::resizeEvent(event);
}

AntResult::Metrics AntResult::metrics() const
{
    return {};
}

QRect AntResult::iconRect() const
{
    const Metrics m = metrics();
    const int top = m.padding;
    return QRect((width() - m.iconSize) / 2, top, m.iconSize, m.iconSize);
}

QRect AntResult::titleRect() const
{
    const Metrics m = metrics();
    const auto& token = antTheme->tokens();

    QFont titleFont = font();
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(QFont::DemiBold);
    QFontMetrics titleFm(titleFont);

    int top = m.padding;
    if (m_iconVisible)
    {
        top += m.iconSize + m.spacing;
    }

    const int textWidth = qMax(120, width() - m.padding * 2);
    const int textHeight = titleFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_title).height();

    return QRect(m.padding, top, textWidth, textHeight);
}

QRect AntResult::subTitleRect() const
{
    const Metrics m = metrics();

    QFont subFont = font();
    subFont.setPixelSize(m.subTitleFontSize);
    QFontMetrics subFm(subFont);

    const QRect tr = titleRect();
    const int top = tr.bottom() + m.spacing;
    const int textWidth = qMax(120, width() - m.padding * 2);
    const int textHeight = subFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_subTitle).height();

    return QRect(m.padding, top, textWidth, textHeight);
}

QRect AntResult::extraRect() const
{
    if (!m_extraWidget)
    {
        return {};
    }
    const Metrics m = metrics();
    const QSize size = m_extraWidget->sizeHint();

    int top = m.padding;
    if (m_iconVisible)
    {
        top += m.iconSize + m.spacing;
    }

    QFont titleFont = font();
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(QFont::DemiBold);
    QFontMetrics titleFm(titleFont);

    QFont subFont = font();
    subFont.setPixelSize(m.subTitleFontSize);
    QFontMetrics subFm(subFont);

    const int textWidth = qMax(120, width() - m.padding * 2);
    top += titleFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_title).height();

    if (!m_subTitle.isEmpty())
    {
        top += m.spacing;
        top += subFm.boundingRect(QRect(0, 0, textWidth, 200), Qt::AlignCenter | Qt::TextWordWrap, m_subTitle).height();
    }

    top += m.extraSpacing;

    return QRect((width() - size.width()) / 2, top, size.width(), size.height());
}

void AntResult::syncExtraGeometry()
{
    if (m_extraWidget)
    {
        m_extraWidget->setGeometry(extraRect());
        m_extraWidget->show();
    }
}

QColor AntResult::iconColor() const
{
    const auto& token = antTheme->tokens();
    switch (m_status)
    {
    case Ant::AlertType::Success:
        return token.colorSuccess;
    case Ant::AlertType::Warning:
        return token.colorWarning;
    case Ant::AlertType::Error:
        return token.colorError;
    case Ant::AlertType::Info:
    default:
        return token.colorPrimary;
    }
}

Ant::IconType AntResult::iconTypeForStatus() const
{
    switch (m_status)
    {
    case Ant::AlertType::Success:
        return Ant::IconType::CheckCircle;
    case Ant::AlertType::Error:
        return Ant::IconType::CloseCircle;
    case Ant::AlertType::Warning:
        return Ant::IconType::ExclamationCircle;
    case Ant::AlertType::Info:
    default:
        return Ant::IconType::InfoCircle;
    }
}
