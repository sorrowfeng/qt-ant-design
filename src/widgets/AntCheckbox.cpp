#include "AntCheckbox.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>

#include "../styles/AntCheckboxStyle.h"
#include "core/AntTheme.h"
#include "core/AntWave.h"

namespace
{
constexpr int IndicatorSize = 16;
}

AntCheckbox::AntCheckbox(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);

    auto* checkboxStyle = new AntCheckboxStyle(style());
    checkboxStyle->setParent(this);
    setStyle(checkboxStyle);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this]() {
        update();
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateGeometry();
        update();
    });
}

AntCheckbox::AntCheckbox(const QString& text, QWidget* parent)
    : AntCheckbox(parent)
{
    setText(text);
}

bool AntCheckbox::isChecked() const { return m_checked; }

void AntCheckbox::setChecked(bool checked)
{
    setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

Qt::CheckState AntCheckbox::checkState() const
{
    if (m_indeterminate)
    {
        return Qt::PartiallyChecked;
    }
    return m_checked ? Qt::Checked : Qt::Unchecked;
}

void AntCheckbox::setCheckState(Qt::CheckState state)
{
    const Qt::CheckState oldState = checkState();
    const bool oldChecked = m_checked;
    const bool oldIndeterminate = m_indeterminate;

    if (state == Qt::PartiallyChecked)
    {
        if (!m_tristate)
        {
            setTristate(true);
        }
        m_indeterminate = true;
    }
    else
    {
        m_checked = state == Qt::Checked;
        m_indeterminate = false;
    }

    const Qt::CheckState newState = checkState();
    if (oldState == newState && oldChecked == m_checked && oldIndeterminate == m_indeterminate)
    {
        return;
    }

    update();
    if (oldChecked != m_checked)
    {
        Q_EMIT checkedChanged(m_checked);
        Q_EMIT toggled(m_checked);
    }
    if (oldIndeterminate != m_indeterminate)
    {
        Q_EMIT indeterminateChanged(m_indeterminate);
    }
    if (oldState != newState)
    {
        Q_EMIT checkStateChanged(newState);
        Q_EMIT stateChanged(newState);
    }
}

bool AntCheckbox::isIndeterminate() const { return m_indeterminate; }

void AntCheckbox::setIndeterminate(bool indeterminate)
{
    if (indeterminate)
    {
        setCheckState(Qt::PartiallyChecked);
        return;
    }
    setCheckState(m_checked ? Qt::Checked : Qt::Unchecked);
}

bool AntCheckbox::isTristate() const { return m_tristate; }

void AntCheckbox::setTristate(bool tristate)
{
    if (m_tristate == tristate)
    {
        return;
    }
    m_tristate = tristate;
    if (!m_tristate && m_indeterminate)
    {
        setCheckState(m_checked ? Qt::Checked : Qt::Unchecked);
    }
    Q_EMIT tristateChanged(m_tristate);
}

void AntCheckbox::toggle()
{
    if (m_tristate)
    {
        switch (checkState())
        {
        case Qt::Unchecked:
            setCheckState(Qt::PartiallyChecked);
            break;
        case Qt::PartiallyChecked:
            setCheckState(Qt::Checked);
            break;
        case Qt::Checked:
            setCheckState(Qt::Unchecked);
            break;
        }
        return;
    }
    setChecked(!m_checked);
}

void AntCheckbox::click()
{
    if (!isEnabled())
    {
        return;
    }
    toggle();
    Q_EMIT clicked(m_checked);
}

QString AntCheckbox::text() const { return m_text; }

void AntCheckbox::setText(const QString& text)
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

QSize AntCheckbox::sizeHint() const
{
    constexpr int indicatorSize = 16;
    constexpr int textSpacing = 8;
    QFont f = font();
    f.setPixelSize(antTheme->tokens().fontSize);
    QFontMetrics fm(f);
    const int textWidth = m_text.isEmpty() ? 0 : textSpacing + fm.horizontalAdvance(m_text);
    return QSize(indicatorSize + textWidth, std::max(indicatorSize, fm.height()));
}

QSize AntCheckbox::minimumSizeHint() const
{
    return QSize(16, 16);
}

void AntCheckbox::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntCheckbox::leaveEvent(QEvent* event)
{
    m_hovered = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(event);
}

void AntCheckbox::mousePressEvent(QMouseEvent* event)
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

void AntCheckbox::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        m_pressed = false;
        if (rect().contains(event->pos()))
        {
            toggle();
            Q_EMIT clicked(m_checked);
            if (m_checked)
            {
                // Wave only around the 16x16 indicator (not the whole label row)
                const QRect box = indicatorRect().toRect();
                AntWave::triggerRect(this, box, antTheme->tokens().colorPrimary, 2, true);
            }
        }
        update();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntCheckbox::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::changeEvent(event);
}

void AntCheckbox::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && isEnabled())
    {
        toggle();
        Q_EMIT clicked(m_checked);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

QRectF AntCheckbox::indicatorRect() const
{
    return QRectF(0.5, (height() - IndicatorSize) / 2.0 + 0.5, IndicatorSize - 1, IndicatorSize - 1);
}

QColor AntCheckbox::indicatorBorderColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBorder;
    }
    if (m_checked && !m_indeterminate)
    {
        return m_hovered ? token.colorPrimaryHover : token.colorPrimary;
    }
    return m_hovered ? token.colorPrimary : token.colorBorder;
}

QColor AntCheckbox::indicatorBackgroundColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (m_checked && !m_indeterminate)
    {
        return m_hovered || m_pressed ? token.colorPrimaryHover : token.colorPrimary;
    }
    return token.colorBgContainer;
}

bool AntCheckbox::isHoveredState() const
{
    return m_hovered;
}

bool AntCheckbox::isPressedState() const
{
    return m_pressed;
}
