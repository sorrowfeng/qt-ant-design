#include "AntAlert.h"

#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>

#include "AntButton.h"
#include "AntIcon.h"
#include "core/AntTheme.h"
#include "styles/AntPalette.h"

AntAlert::AntAlert(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

AntAlert::AntAlert(const QString& title, QWidget* parent)
    : AntAlert(parent)
{
    m_title = title;
}

QString AntAlert::title() const { return m_title; }

void AntAlert::setTitle(const QString& title)
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

QString AntAlert::description() const { return m_description; }

void AntAlert::setDescription(const QString& description)
{
    if (m_description == description)
    {
        return;
    }
    m_description = description;
    updateGeometry();
    update();
    Q_EMIT descriptionChanged(m_description);
}

Ant::AlertType AntAlert::alertType() const { return m_alertType; }

void AntAlert::setAlertType(Ant::AlertType type)
{
    if (m_alertType == type)
    {
        return;
    }
    m_alertType = type;
    update();
    Q_EMIT alertTypeChanged(m_alertType);
}

bool AntAlert::showIcon() const { return m_showIcon; }

void AntAlert::setShowIcon(bool showIcon)
{
    if (m_showIcon == showIcon)
    {
        return;
    }
    m_showIcon = showIcon;
    updateGeometry();
    update();
    Q_EMIT showIconChanged(m_showIcon);
}

bool AntAlert::isClosable() const { return m_closable; }

void AntAlert::setClosable(bool closable)
{
    if (m_closable == closable)
    {
        return;
    }
    m_closable = closable;
    updateGeometry();
    update();
    Q_EMIT closableChanged(m_closable);
}

bool AntAlert::isBanner() const { return m_banner; }

void AntAlert::setBanner(bool banner)
{
    if (m_banner == banner)
    {
        return;
    }
    m_banner = banner;
    if (m_banner && !m_showIcon)
    {
        m_showIcon = true;
        Q_EMIT showIconChanged(m_showIcon);
    }
    if (m_banner && m_alertType == Ant::AlertType::Info)
    {
        m_alertType = Ant::AlertType::Warning;
        Q_EMIT alertTypeChanged(m_alertType);
    }
    updateGeometry();
    update();
    Q_EMIT bannerChanged(m_banner);
}

QWidget* AntAlert::actionWidget() const
{
    return m_actionWidget.data();
}

void AntAlert::setActionWidget(QWidget* widget)
{
    if (m_actionWidget == widget)
    {
        return;
    }

    if (m_actionWidget && m_actionWidget->parent() == this)
    {
        m_actionWidget->deleteLater();
    }

    m_actionWidget = widget;
    if (m_actionWidget)
    {
        m_actionWidget->setParent(this);
        m_actionWidget->show();
    }
    syncLayout();
    updateGeometry();
    update();
}

QSize AntAlert::sizeHint() const
{
    const Metrics m = metrics();
    const auto& token = antTheme->tokens();
    const int width = qMax(220, QWidget::width() > 0 ? QWidget::width() : 420);

    QFont titleFont = font();
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(m_description.isEmpty() ? QFont::Normal : QFont::DemiBold);
    QFontMetrics titleFm(titleFont);

    QFont descFont = font();
    descFont.setPixelSize(m.descFontSize);
    QFontMetrics descFm(descFont);

    int left = m.paddingX;
    if (m_showIcon || m_banner)
    {
        left += m.iconSize + 12;
    }
    int right = m.paddingX;
    if (m_closable)
    {
        right += m.closeSize + 8;
    }
    if (m_actionWidget)
    {
        right += m_actionWidget->sizeHint().width() + m.actionSpacing;
    }
    const int textWidth = qMax(120, width - left - right);

    int contentHeight = qMax(m.minHeight - m.paddingY * 2, titleFm.height());
    if (!m_description.isEmpty())
    {
        const QRect descBounds = descFm.boundingRect(QRect(0, 0, textWidth, 1000),
                                                     Qt::TextWordWrap,
                                                     m_description);
        contentHeight = titleFm.height() + 6 + descBounds.height();
    }

    return QSize(width, qMax(m.minHeight, contentHeight + m.paddingY * 2));
}

QSize AntAlert::minimumSizeHint() const
{
    return QSize(180, metrics().minHeight);
}

void AntAlert::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const Metrics m = metrics();
    const auto& token = antTheme->tokens();
    const QRect body = rect().adjusted(0, 0, -1, -1);
    const QRect content = contentRect();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(QPen(borderColor(), token.lineWidth));
    painter.setBrush(backgroundColor());
    painter.drawRoundedRect(body, m.radius, m.radius);

    int textLeft = content.left();
    if (m_showIcon || m_banner)
    {
        const QRect iconRect(textLeft,
                             content.top() + (m_description.isEmpty() ? (content.height() - m.iconSize) / 2 : 2),
                             m.iconSize,
                             m.iconSize);
        AntIcon icon(iconTypeForAlert());
        icon.setIconTheme(Ant::IconTheme::Filled);
        icon.setColor(iconColor());
        icon.setIconSize(m.iconSize);
        icon.resize(iconRect.size());

        QPixmap pixmap(iconRect.size() * devicePixelRatioF());
        pixmap.setDevicePixelRatio(devicePixelRatioF());
        pixmap.fill(Qt::transparent);
        icon.render(&pixmap);
        painter.drawPixmap(iconRect.topLeft(), pixmap);
        textLeft = iconRect.right() + 12;
    }

    int textRight = content.right();
    if (m_actionWidget)
    {
        textRight = actionRect().left() - m.actionSpacing;
    }
    if (m_closable)
    {
        textRight = qMin(textRight, closeRect().left() - 8);
        const QRect close = closeRect();
        if (m_hoverClose)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(AntPalette::alpha(token.colorTextTertiary, 0.12));
            painter.drawRoundedRect(close, token.borderRadiusXS, token.borderRadiusXS);
        }
        painter.setPen(QPen(token.colorTextTertiary, 1.5, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(close.center() + QPoint(-4, -4), close.center() + QPoint(4, 4));
        painter.drawLine(close.center() + QPoint(4, -4), close.center() + QPoint(-4, 4));
    }

    QFont titleFont = painter.font();
    titleFont.setPixelSize(m.titleFontSize);
    titleFont.setWeight(m_description.isEmpty() ? QFont::Normal : QFont::DemiBold);
    painter.setFont(titleFont);
    painter.setPen(titleColor());

    QRect titleRect(textLeft,
                    content.top(),
                    qMax(40, textRight - textLeft),
                    m_description.isEmpty() ? content.height() : titleFont.pixelSize() + 6);
    painter.drawText(titleRect, Qt::AlignLeft | (m_description.isEmpty() ? Qt::AlignVCenter : Qt::AlignTop), m_title);

    if (!m_description.isEmpty())
    {
        QFont descFont = painter.font();
        descFont.setPixelSize(m.descFontSize);
        descFont.setWeight(QFont::Normal);
        painter.setFont(descFont);
        painter.setPen(descriptionColor());
        QRect descRect(textLeft,
                       titleRect.bottom() + 6,
                       qMax(40, textRight - textLeft),
                       content.bottom() - titleRect.bottom() - 6);
        painter.drawText(descRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, m_description);
    }
}

void AntAlert::resizeEvent(QResizeEvent* event)
{
    syncLayout();
    QWidget::resizeEvent(event);
}

void AntAlert::mouseMoveEvent(QMouseEvent* event)
{
    const bool hover = m_closable && closeRect().contains(event->pos());
    if (m_hoverClose != hover)
    {
        m_hoverClose = hover;
        update(closeRect());
    }
    QWidget::mouseMoveEvent(event);
}

void AntAlert::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_closable && closeRect().contains(event->pos()))
    {
        Q_EMIT closeRequested();
        hide();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntAlert::leaveEvent(QEvent* event)
{
    if (m_hoverClose)
    {
        m_hoverClose = false;
        update(closeRect());
    }
    QWidget::leaveEvent(event);
}

AntAlert::Metrics AntAlert::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    m.minHeight = m_description.isEmpty() ? token.controlHeightLG : 72;
    m.radius = token.borderRadiusLG;
    m.paddingX = m_banner ? token.paddingLG : token.padding;
    m.paddingY = m_description.isEmpty() ? token.paddingXS : token.paddingSM;
    m.iconSize = m_description.isEmpty() ? 16 : 18;
    m.titleFontSize = token.fontSize;
    m.descFontSize = token.fontSize;
    m.closeSize = 20;
    return m;
}

QColor AntAlert::backgroundColor() const
{
    const auto& token = antTheme->tokens();
    switch (m_alertType)
    {
    case Ant::AlertType::Success:
        return token.colorSuccessBg;
    case Ant::AlertType::Warning:
        return token.colorWarningBg;
    case Ant::AlertType::Error:
        return token.colorErrorBg;
    case Ant::AlertType::Info:
    default:
        return token.colorPrimaryBg;
    }
}

QColor AntAlert::borderColor() const
{
    switch (m_alertType)
    {
    case Ant::AlertType::Success:
        return AntPalette::tint(antTheme->tokens().colorSuccess, 0.62);
    case Ant::AlertType::Warning:
        return AntPalette::tint(antTheme->tokens().colorWarning, 0.62);
    case Ant::AlertType::Error:
        return AntPalette::tint(antTheme->tokens().colorError, 0.62);
    case Ant::AlertType::Info:
    default:
        return AntPalette::tint(antTheme->tokens().colorPrimary, 0.62);
    }
}

QColor AntAlert::iconColor() const
{
    switch (m_alertType)
    {
    case Ant::AlertType::Success:
        return antTheme->tokens().colorSuccess;
    case Ant::AlertType::Warning:
        return antTheme->tokens().colorWarning;
    case Ant::AlertType::Error:
        return antTheme->tokens().colorError;
    case Ant::AlertType::Info:
    default:
        return antTheme->tokens().colorPrimary;
    }
}

QColor AntAlert::titleColor() const
{
    return antTheme->tokens().colorText;
}

QColor AntAlert::descriptionColor() const
{
    return antTheme->tokens().colorTextSecondary;
}

QRect AntAlert::closeRect() const
{
    const Metrics m = metrics();
    const QRect content = contentRect();
    return QRect(width() - m.paddingX - m.closeSize,
                 content.top(),
                 m.closeSize,
                 m.closeSize);
}

QRect AntAlert::actionRect() const
{
    if (!m_actionWidget)
    {
        return QRect();
    }
    const Metrics m = metrics();
    const QSize size = m_actionWidget->sizeHint();
    int x = width() - m.paddingX - size.width() - (m_closable ? (m.closeSize + 8) : 0);
    int y = contentRect().top() + (contentRect().height() - size.height()) / 2;
    return QRect(x, y, size.width(), size.height());
}

QRect AntAlert::contentRect() const
{
    const Metrics m = metrics();
    return rect().adjusted(m.paddingX, m.paddingY, -m.paddingX, -m.paddingY);
}

void AntAlert::syncLayout()
{
    if (m_actionWidget)
    {
        m_actionWidget->setGeometry(actionRect());
        m_actionWidget->show();
    }
}

Ant::IconType AntAlert::iconTypeForAlert() const
{
    switch (m_alertType)
    {
    case Ant::AlertType::Success:
        return Ant::IconType::CheckCircle;
    case Ant::AlertType::Warning:
        return Ant::IconType::ExclamationCircle;
    case Ant::AlertType::Error:
        return Ant::IconType::CloseCircle;
    case Ant::AlertType::Info:
    default:
        return Ant::IconType::InfoCircle;
    }
}
