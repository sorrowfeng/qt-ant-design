#include "AntCheckbox.h"

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include "core/AntTheme.h"
#include "styles/AntPalette.h"

namespace
{
constexpr int IndicatorSize = 16;
constexpr int TextSpacing = 8;
}

AntCheckbox::AntCheckbox(QWidget* parent)
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

AntCheckbox::AntCheckbox(const QString& text, QWidget* parent)
    : AntCheckbox(parent)
{
    setText(text);
}

bool AntCheckbox::isChecked() const { return m_checked; }

void AntCheckbox::setChecked(bool checked)
{
    if (m_checked == checked && !m_indeterminate)
    {
        return;
    }

    m_checked = checked;
    const bool wasIndeterminate = m_indeterminate;
    m_indeterminate = false;
    update();
    Q_EMIT checkedChanged(m_checked);
    Q_EMIT toggled(m_checked);
    Q_EMIT stateChanged(m_checked ? Qt::Checked : Qt::Unchecked);
    if (wasIndeterminate)
    {
        Q_EMIT indeterminateChanged(false);
    }
}

bool AntCheckbox::isIndeterminate() const { return m_indeterminate; }

void AntCheckbox::setIndeterminate(bool indeterminate)
{
    if (m_indeterminate == indeterminate)
    {
        return;
    }

    m_indeterminate = indeterminate;
    update();
    Q_EMIT indeterminateChanged(m_indeterminate);
    Q_EMIT stateChanged(m_indeterminate ? Qt::PartiallyChecked : (m_checked ? Qt::Checked : Qt::Unchecked));
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
    QFont f = font();
    f.setPixelSize(antTheme->tokens().fontSize);
    QFontMetrics fm(f);
    const int textWidth = m_text.isEmpty() ? 0 : TextSpacing + fm.horizontalAdvance(m_text);
    return QSize(IndicatorSize + textWidth, std::max(IndicatorSize, fm.height()));
}

QSize AntCheckbox::minimumSizeHint() const
{
    return QSize(IndicatorSize, IndicatorSize);
}

void AntCheckbox::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    const QRectF box = indicatorRect();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    painter.setPen(QPen(indicatorBorderColor(), token.lineWidth));
    painter.setBrush(indicatorBackgroundColor());
    painter.drawRoundedRect(box, token.borderRadiusSM, token.borderRadiusSM);

    if (m_checked && !m_indeterminate)
    {
        QPainterPath check;
        check.moveTo(box.left() + box.width() * 0.28, box.top() + box.height() * 0.52);
        check.lineTo(box.left() + box.width() * 0.43, box.top() + box.height() * 0.68);
        check.lineTo(box.left() + box.width() * 0.74, box.top() + box.height() * 0.32);
        painter.setPen(QPen(token.colorTextLightSolid, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(check);
    }
    else if (m_indeterminate)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(isEnabled() ? token.colorPrimary : token.colorTextDisabled);
        const QRectF mark(box.left() + 4, box.center().y() - 1.5, box.width() - 8, 3);
        painter.drawRoundedRect(mark, 1.5, 1.5);
    }

    if (hasFocus() && isEnabled())
    {
        QColor focus = AntPalette::alpha(token.colorPrimary, 0.22);
        painter.setPen(QPen(focus, token.controlOutlineWidth));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(box.adjusted(-2, -2, 2, 2), token.borderRadiusSM + 2, token.borderRadiusSM + 2);
    }

    if (!m_text.isEmpty())
    {
        QFont f = painter.font();
        f.setPixelSize(token.fontSize);
        painter.setFont(f);
        painter.setPen(isEnabled() ? token.colorText : token.colorTextDisabled);
        QRectF textRect(box.right() + TextSpacing, 0, width() - box.right() - TextSpacing, height());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }
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

void AntCheckbox::toggle()
{
    setChecked(!m_checked);
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
