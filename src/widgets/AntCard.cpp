#include "AntCard.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

#include "core/AntTheme.h"

AntCard::AntCard(QWidget* parent)
    : QFrame(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFrameShape(QFrame::NoFrame);

    m_rootLayout = new QVBoxLayout(this);
    m_rootLayout->setSpacing(0);
    m_rootLayout->setContentsMargins(1, 1, 1, 1);

    m_header = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);
    m_titleLabel = new QLabel(m_header);
    m_extraLabel = new QLabel(m_header);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    headerLayout->addWidget(m_titleLabel, 1);
    headerLayout->addWidget(m_extraLabel);

    m_body = new QWidget(this);
    m_bodyLayout = new QVBoxLayout(m_body);
    m_bodyLayout->setSpacing(8);

    m_actions = new QWidget(this);
    m_actionsLayout = new QHBoxLayout(m_actions);
    m_actionsLayout->setContentsMargins(0, 0, 0, 0);
    m_actionsLayout->setSpacing(0);

    m_rootLayout->addWidget(m_header);
    m_rootLayout->addWidget(m_body, 1);
    m_rootLayout->addWidget(m_actions);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateTheme();
        update();
    });
    connect(&m_spinnerTimer, &QTimer::timeout, this, [this]() {
        m_spinnerAngle = (m_spinnerAngle + 30) % 360;
        update();
    });

    rebuildChrome();
    updateTheme();
}

AntCard::AntCard(const QString& title, QWidget* parent)
    : AntCard(parent)
{
    setTitle(title);
}

QString AntCard::title() const { return m_title; }

void AntCard::setTitle(const QString& title)
{
    if (m_title == title)
        return;
    m_title = title;
    m_titleLabel->setText(title);
    rebuildChrome();
    Q_EMIT titleChanged(m_title);
}

QString AntCard::extra() const { return m_extra; }

void AntCard::setExtra(const QString& extra)
{
    if (m_extra == extra)
        return;
    m_extra = extra;
    m_extraLabel->setText(extra);
    rebuildChrome();
    Q_EMIT extraChanged(m_extra);
}

bool AntCard::isBordered() const { return m_bordered; }

void AntCard::setBordered(bool bordered)
{
    if (m_bordered == bordered)
        return;
    m_bordered = bordered;
    update();
    Q_EMIT borderedChanged(m_bordered);
}

bool AntCard::isHoverable() const { return m_hoverable; }

void AntCard::setHoverable(bool hoverable)
{
    if (m_hoverable == hoverable)
        return;
    m_hoverable = hoverable;
    setCursor(hoverable ? Qt::PointingHandCursor : Qt::ArrowCursor);
    update();
    Q_EMIT hoverableChanged(m_hoverable);
}

bool AntCard::isLoading() const { return m_loading; }

void AntCard::setLoading(bool loading)
{
    if (m_loading == loading)
        return;
    m_loading = loading;
    m_loading ? m_spinnerTimer.start(80) : m_spinnerTimer.stop();
    update();
    Q_EMIT loadingChanged(m_loading);
}

Ant::CardSize AntCard::cardSize() const { return m_cardSize; }

void AntCard::setCardSize(Ant::CardSize size)
{
    if (m_cardSize == size)
        return;
    m_cardSize = size;
    rebuildChrome();
    updateTheme();
    Q_EMIT cardSizeChanged(m_cardSize);
}

QWidget* AntCard::bodyWidget() const { return m_body; }
QVBoxLayout* AntCard::bodyLayout() const { return m_bodyLayout; }

void AntCard::setBodyWidget(QWidget* widget)
{
    while (QLayoutItem* item = m_bodyLayout->takeAt(0))
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    if (widget)
    {
        widget->setParent(m_body);
        m_bodyLayout->addWidget(widget);
    }
}

void AntCard::setCoverWidget(QWidget* widget)
{
    if (m_cover)
    {
        m_rootLayout->removeWidget(m_cover);
        m_cover->deleteLater();
        m_cover = nullptr;
    }
    m_cover = widget;
    if (m_cover)
    {
        m_cover->setParent(this);
        m_rootLayout->insertWidget(m_header->isVisible() ? 1 : 0, m_cover);
    }
}

void AntCard::addActionWidget(QWidget* widget)
{
    if (!widget)
        return;
    widget->setParent(m_actions);
    m_actionsLayout->addWidget(widget, 1);
    rebuildChrome();
}

void AntCard::clearActions()
{
    while (QLayoutItem* item = m_actionsLayout->takeAt(0))
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    rebuildChrome();
}

void AntCard::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    const auto& token = antTheme->tokens();
    const int radius = token.borderRadiusLG;
    QRect cardRect = rect().adjusted(0, 0, -1, -1);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    if (m_hoverable && m_hovered)
    {
        antTheme->drawEffectShadow(&painter, rect(), 12, radius, 1.35);
        cardRect.adjust(2, 2, -2, -2);
    }

    painter.setPen(m_bordered ? QPen(token.colorBorderSecondary, token.lineWidth) : Qt::NoPen);
    painter.setBrush(token.colorBgContainer);
    painter.drawRoundedRect(QRectF(cardRect), radius, radius);

    if (m_header->isVisible())
    {
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.drawLine(cardRect.left() + 1, m_header->geometry().bottom(), cardRect.right() - 1, m_header->geometry().bottom());
    }
    if (m_actions->isVisible())
    {
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.drawLine(cardRect.left() + 1, m_actions->geometry().top(), cardRect.right() - 1, m_actions->geometry().top());
    }
    if (m_loading)
    {
        QColor mask = token.colorBgContainer;
        mask.setAlphaF(0.72);
        painter.setPen(Qt::NoPen);
        painter.setBrush(mask);
        painter.drawRoundedRect(QRectF(cardRect), radius, radius);
        drawSpinner(painter, QRectF(width() / 2.0 - 14, height() / 2.0 - 14, 28, 28));
    }
}

void AntCard::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QFrame::enterEvent(event);
}

void AntCard::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QFrame::leaveEvent(event);
}

void AntCard::rebuildChrome()
{
    const auto& token = antTheme->tokens();
    const bool small = m_cardSize == Ant::CardSize::Small;
    const int headerHeight = small ? 38 : 56;
    const int headerPadding = small ? token.paddingSM : token.paddingLG;
    const int bodyPadding = small ? token.paddingSM : token.paddingLG;

    m_header->setVisible(!m_title.isEmpty() || !m_extra.isEmpty());
    m_extraLabel->setVisible(!m_extra.isEmpty());
    m_actions->setVisible(m_actionsLayout->count() > 0);
    m_header->setMinimumHeight(headerHeight);
    m_header->setMaximumHeight(headerHeight);

    if (auto* layout = qobject_cast<QHBoxLayout*>(m_header->layout()))
        layout->setContentsMargins(headerPadding, 0, headerPadding, 0);
    m_bodyLayout->setContentsMargins(bodyPadding, bodyPadding, bodyPadding, bodyPadding);
    m_actions->setMinimumHeight(48);
    m_actions->setMaximumHeight(48);
}

void AntCard::updateTheme()
{
    const auto& token = antTheme->tokens();
    const bool small = m_cardSize == Ant::CardSize::Small;
    QFont titleFont = m_titleLabel->font();
    titleFont.setPixelSize(small ? token.fontSize : token.fontSizeLG);
    titleFont.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(QStringLiteral("QLabel { background: transparent; color: %1; }").arg(token.colorText.name(QColor::HexArgb)));

    QFont extraFont = m_extraLabel->font();
    extraFont.setPixelSize(token.fontSize);
    m_extraLabel->setFont(extraFont);
    m_extraLabel->setStyleSheet(QStringLiteral("QLabel { background: transparent; color: %1; }").arg(token.colorTextSecondary.name(QColor::HexArgb)));

    m_header->setStyleSheet(QStringLiteral("background: transparent;"));
    m_body->setStyleSheet(QStringLiteral("background: transparent;"));
    m_actions->setStyleSheet(QStringLiteral("background: transparent;"));
}

void AntCard::drawSpinner(QPainter& painter, const QRectF& rect) const
{
    painter.save();
    painter.setPen(QPen(antTheme->tokens().colorPrimary, 3, Qt::SolidLine, Qt::RoundCap));
    painter.setBrush(Qt::NoBrush);
    painter.drawArc(rect, m_spinnerAngle * 16, 280 * 16);
    painter.restore();
}
