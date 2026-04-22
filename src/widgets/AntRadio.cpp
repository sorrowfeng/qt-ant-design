#include "AntRadio.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int RadioSize = 16;
constexpr int TextSpacing = 8;
constexpr qreal DotRatio = 0.5;
}

AntRadio::AntRadio(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this]() {
        update();
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

AntRadio::AntRadio(const QString& text, QWidget* parent)
    : AntRadio(parent)
{
    setText(text);
}

bool AntRadio::isChecked() const { return m_checked; }

void AntRadio::setChecked(bool checked)
{
    if (m_checked == checked)
    {
        return;
    }

    m_checked = checked;
    if (m_checked && m_autoExclusive)
    {
        uncheckSiblings();
    }
    update();
    Q_EMIT checkedChanged(m_checked);
    Q_EMIT toggled(m_checked);
}

QString AntRadio::text() const { return m_text; }

void AntRadio::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    updateGeometry();
    update();
    Q_EMIT textChanged(m_text);
}

QVariant AntRadio::value() const { return m_value; }

void AntRadio::setValue(const QVariant& value)
{
    if (m_value == value)
    {
        return;
    }
    m_value = value;
    Q_EMIT valueChanged(m_value);
}

bool AntRadio::autoExclusive() const { return m_autoExclusive; }

void AntRadio::setAutoExclusive(bool autoExclusive)
{
    if (m_autoExclusive == autoExclusive)
    {
        return;
    }
    m_autoExclusive = autoExclusive;
    if (m_checked && m_autoExclusive)
    {
        uncheckSiblings();
    }
    Q_EMIT autoExclusiveChanged(m_autoExclusive);
}

QSize AntRadio::sizeHint() const
{
    QFont f = font();
    f.setPixelSize(antTheme->tokens().fontSize);
    QFontMetrics fm(f);
    const int textWidth = m_text.isEmpty() ? 0 : TextSpacing + fm.horizontalAdvance(m_text);
    return QSize(RadioSize + textWidth, std::max(RadioSize, fm.height()));
}

QSize AntRadio::minimumSizeHint() const
{
    return QSize(RadioSize, RadioSize);
}

void AntRadio::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    const auto& token = antTheme->tokens();
    const QRectF circle = indicatorRect();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    painter.setPen(QPen(indicatorBorderColor(), token.lineWidth));
    painter.setBrush(indicatorBackgroundColor());
    painter.drawEllipse(circle);

    if (m_checked)
    {
        const qreal dotSize = RadioSize * DotRatio;
        QRectF dot(circle.center().x() - dotSize / 2.0, circle.center().y() - dotSize / 2.0, dotSize, dotSize);
        painter.setPen(Qt::NoPen);
        painter.setBrush(isEnabled() ? token.colorTextLightSolid : token.colorTextDisabled);
        painter.drawEllipse(dot);
    }

    if (hasFocus() && isEnabled())
    {
        QColor focus = AntPalette::alpha(token.colorPrimary, 0.22);
        painter.setPen(QPen(focus, token.controlOutlineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(circle.adjusted(-2, -2, 2, 2));
    }

    if (!m_text.isEmpty())
    {
        QFont f = painter.font();
        f.setPixelSize(token.fontSize);
        painter.setFont(f);
        painter.setPen(isEnabled() ? token.colorText : token.colorTextDisabled);
        QRectF textRect(circle.right() + TextSpacing, 0, width() - circle.right() - TextSpacing, height());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
}

void AntRadio::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntRadio::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void AntRadio::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        m_pressed = true;
        update();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntRadio::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        if (rect().contains(event->pos()))
        {
            toggleFromUser();
            Q_EMIT clicked(m_checked);
        }
        update();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntRadio::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::changeEvent(event);
}

void AntRadio::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && isEnabled())
    {
        toggleFromUser();
        Q_EMIT clicked(m_checked);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

QRectF AntRadio::indicatorRect() const
{
    return QRectF(0.5, (height() - RadioSize) / 2.0 + 0.5, RadioSize - 1, RadioSize - 1);
}

void AntRadio::toggleFromUser()
{
    if (!m_checked)
    {
        setChecked(true);
    }
}

void AntRadio::uncheckSiblings()
{
    QWidget* owner = parentWidget();
    if (!owner)
    {
        return;
    }

    const auto radios = owner->findChildren<AntRadio*>(QString(), Qt::FindDirectChildrenOnly);
    for (AntRadio* radio : radios)
    {
        if (radio != this && radio->m_autoExclusive && radio->m_checked)
        {
            radio->setChecked(false);
        }
    }
}

QColor AntRadio::indicatorBorderColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBorder;
    }
    if (m_checked)
    {
        return m_hovered ? token.colorPrimaryHover : token.colorPrimary;
    }
    return m_hovered ? token.colorPrimary : token.colorBorder;
}

QColor AntRadio::indicatorBackgroundColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (m_checked)
    {
        return m_hovered || m_pressed ? token.colorPrimaryHover : token.colorPrimary;
    }
    return token.colorBgContainer;
}
