#include "AntRadio.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

#include "../styles/AntRadioStyle.h"
#include "core/AntTheme.h"
#include "core/AntWave.h"

namespace
{
constexpr int RadioSize = 16;
}

AntRadio::AntRadio(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);

    auto* radioStyle = new AntRadioStyle(style());
    radioStyle->setParent(this);
    setStyle(radioStyle);

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

bool AntRadio::isButtonStyle() const { return m_buttonStyle; }

void AntRadio::setButtonStyle(bool buttonStyle)
{
    if (m_buttonStyle == buttonStyle)
    {
        return;
    }
    m_buttonStyle = buttonStyle;
    updateGeometry();
    update();
    Q_EMIT buttonStyleChanged(m_buttonStyle);
}

QSize AntRadio::sizeHint() const
{
    QFont f = font();
    f.setPixelSize(antTheme->tokens().fontSize);
    QFontMetrics fm(f);
    if (m_buttonStyle)
    {
        return QSize(fm.horizontalAdvance(m_text) + 30, antTheme->tokens().controlHeight);
    }
    constexpr int radioSize = 16;
    constexpr int textSpacing = 8;
    const int textWidth = m_text.isEmpty() ? 0 : textSpacing + fm.horizontalAdvance(m_text);
    return QSize(radioSize + textWidth, std::max(radioSize, fm.height()));
}

QSize AntRadio::minimumSizeHint() const
{
    return m_buttonStyle ? QSize(48, antTheme->tokens().controlHeight) : QSize(16, 16);
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
        const bool wasChecked = m_checked;
        if (rect().contains(event->pos()))
        {
            toggleFromUser();
            Q_EMIT clicked(m_checked);
            if (!wasChecked && m_checked)
            {
                const QRect box = indicatorRect().toRect();
                // Radio is circular — use radius = size/2 for a perfect ring.
                AntWave::triggerRect(this, box, antTheme->tokens().colorPrimary, box.width() / 2);
            }
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
    if (m_buttonStyle)
    {
        return QRectF();
    }
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
        return token.colorBgContainer;
    }
    return token.colorBgContainer;
}

bool AntRadio::isHoveredState() const
{
    return m_hovered;
}

bool AntRadio::isPressedState() const
{
    return m_pressed;
}
