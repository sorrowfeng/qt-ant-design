#include "AntRadio.h"

#include <QEvent>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QSizePolicy>

#include <algorithm>

#include "../styles/AntRadioStyle.h"
#include "core/AntTheme.h"
#include "core/AntWave.h"

namespace
{
constexpr int RadioSize = 16;
constexpr int TextSpacing = 8;
}

AntRadio::AntRadio(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    auto* radioStyle = new AntRadioStyle(style());
    radioStyle->setParent(this);
    setStyle(radioStyle);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this]() {
        update();
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        const QSize oldHint = sizeHint();
        invalidateLayoutCache();
        if (oldHint != sizeHint())
        {
            updateGeometry();
        }
        update();
    });
    syncRadioPerfCounters();
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
    updateVisualStateRegion();
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
    invalidateLayoutCache();
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
    invalidateLayoutCache();
    updateGeometry();
    update();
    Q_EMIT buttonStyleChanged(m_buttonStyle);
}

void AntRadio::toggle()
{
    setChecked(!m_checked);
}

void AntRadio::click()
{
    if (!isEnabled())
    {
        return;
    }
    toggleFromUser();
    Q_EMIT clicked(m_checked);
}

QSize AntRadio::sizeHint() const
{
    return layoutCache().sizeHint;
}

QSize AntRadio::minimumSizeHint() const
{
    return layoutCache().minimumSizeHint;
}

void AntRadio::enterEvent(AntEnterEvent* event)
{
    if (m_hovered)
    {
        QWidget::enterEvent(event);
        return;
    }
    m_hovered = true;
    updateVisualStateRegion();
    QWidget::enterEvent(event);
}

void AntRadio::leaveEvent(QEvent* event)
{
    if (!m_hovered && !m_pressed)
    {
        QWidget::leaveEvent(event);
        return;
    }
    const QRect oldStateRect = visualStateRect();
    m_hovered = false;
    m_pressed = false;
    updateVisualStateRegion(oldStateRect);
    QWidget::leaveEvent(event);
}

void AntRadio::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        if (m_pressed)
        {
            event->accept();
            return;
        }
        m_pressed = true;
        updateVisualStateRegion();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntRadio::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_pressed)
    {
        const QRect oldStateRect = visualStateRect();
        m_pressed = false;
        const bool wasChecked = m_checked;
        if (rect().contains(event->pos()))
        {
            toggleFromUser();
            Q_EMIT clicked(m_checked);
            if (m_buttonStyle)
            {
                AntWave::triggerRect(this, rect().adjusted(1, 1, -1, -1),
                                     antTheme->tokens().colorPrimary,
                                     antTheme->tokens().borderRadius);
            }
            else if (!wasChecked && m_checked)
            {
                const QRect box = indicatorRect().toRect();
                // Radio is circular — use radius = size/2 for a perfect ring.
                AntWave::triggerRect(this, box, antTheme->tokens().colorPrimary, box.width() / 2, true);
            }
        }
        updateVisualStateRegion(oldStateRect);
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

void AntRadio::moveEvent(QMoveEvent* event)
{
    if (m_buttonStyle)
    {
        invalidateLayoutCache();
    }
    QWidget::moveEvent(event);
}

void AntRadio::resizeEvent(QResizeEvent* event)
{
    invalidateLayoutCache();
    QWidget::resizeEvent(event);
}

const AntRadio::LayoutCache& AntRadio::layoutCache() const
{
    const auto& token = antTheme->tokens();
    if (m_layoutCache.valid
        && m_layoutCache.widgetSize == size()
        && m_layoutCache.text == m_text
        && m_layoutCache.buttonStyle == m_buttonStyle
        && m_layoutCache.fontSize == token.fontSize
        && m_layoutCache.controlHeight == token.controlHeight
        && m_layoutCache.borderRadius == token.borderRadius)
    {
        return m_layoutCache;
    }

    m_layoutCache.widgetSize = size();
    m_layoutCache.text = m_text;
    m_layoutCache.buttonStyle = m_buttonStyle;
    m_layoutCache.fontSize = token.fontSize;
    m_layoutCache.controlHeight = token.controlHeight;
    m_layoutCache.borderRadius = token.borderRadius;

    QFontMetrics fm(radioFont());
    if (m_buttonStyle)
    {
        m_layoutCache.sizeHint = QSize(fm.horizontalAdvance(m_text) + 30, token.controlHeight);
        m_layoutCache.minimumSizeHint = QSize(48, token.controlHeight);
        m_layoutCache.buttonFrame = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        m_layoutCache.buttonEdges = buttonSegmentEdges();
        const qreal radius = std::min<qreal>(token.borderRadius, m_layoutCache.buttonFrame.height() / 2.0);
        const bool roundLeft = m_layoutCache.buttonEdges.first;
        const bool roundRight = m_layoutCache.buttonEdges.second;
        QPainterPath path;
        path.moveTo(m_layoutCache.buttonFrame.left() + (roundLeft ? radius : 0), m_layoutCache.buttonFrame.top());
        path.lineTo(m_layoutCache.buttonFrame.right() - (roundRight ? radius : 0), m_layoutCache.buttonFrame.top());
        if (roundRight)
        {
            path.quadTo(m_layoutCache.buttonFrame.right(), m_layoutCache.buttonFrame.top(),
                        m_layoutCache.buttonFrame.right(), m_layoutCache.buttonFrame.top() + radius);
        }
        path.lineTo(m_layoutCache.buttonFrame.right(), m_layoutCache.buttonFrame.bottom() - (roundRight ? radius : 0));
        if (roundRight)
        {
            path.quadTo(m_layoutCache.buttonFrame.right(), m_layoutCache.buttonFrame.bottom(),
                        m_layoutCache.buttonFrame.right() - radius, m_layoutCache.buttonFrame.bottom());
        }
        path.lineTo(m_layoutCache.buttonFrame.left() + (roundLeft ? radius : 0), m_layoutCache.buttonFrame.bottom());
        if (roundLeft)
        {
            path.quadTo(m_layoutCache.buttonFrame.left(), m_layoutCache.buttonFrame.bottom(),
                        m_layoutCache.buttonFrame.left(), m_layoutCache.buttonFrame.bottom() - radius);
        }
        path.lineTo(m_layoutCache.buttonFrame.left(), m_layoutCache.buttonFrame.top() + (roundLeft ? radius : 0));
        if (roundLeft)
        {
            path.quadTo(m_layoutCache.buttonFrame.left(), m_layoutCache.buttonFrame.top(),
                        m_layoutCache.buttonFrame.left() + radius, m_layoutCache.buttonFrame.top());
        }
        path.closeSubpath();
        m_layoutCache.buttonPath = path;
    }
    else
    {
        const int textWidth = m_text.isEmpty() ? 0 : TextSpacing + fm.horizontalAdvance(m_text);
        m_layoutCache.sizeHint = QSize(RadioSize + textWidth, std::max(RadioSize, fm.height()));
        m_layoutCache.minimumSizeHint = QSize(RadioSize, RadioSize);
        m_layoutCache.indicatorRect = QRectF(0.5, (height() - RadioSize) / 2.0 + 0.5, RadioSize - 1, RadioSize - 1);
        m_layoutCache.textRect = QRectF(m_layoutCache.indicatorRect.right() + TextSpacing,
                                        0,
                                        width() - m_layoutCache.indicatorRect.right() - TextSpacing,
                                        height());
        m_layoutCache.buttonFrame = QRectF();
        m_layoutCache.buttonPath = QPainterPath();
        m_layoutCache.buttonEdges = {true, true};
    }

    m_layoutCache.valid = true;
    ++m_layoutBuildCount;
    ++m_sizeHintResolveCount;
    syncRadioPerfCounters();
    return m_layoutCache;
}

QPair<bool, bool> AntRadio::buttonSegmentEdges() const
{
    if (!m_buttonStyle || !parentWidget())
    {
        return {true, true};
    }

    auto radios = parentWidget()->findChildren<AntRadio*>(QString(), Qt::FindDirectChildrenOnly);
    radios.erase(std::remove_if(radios.begin(), radios.end(), [](const AntRadio* item) {
                    return !item->isButtonStyle();
                }),
                 radios.end());
    std::sort(radios.begin(), radios.end(), [](const AntRadio* a, const AntRadio* b) {
        if (a->geometry().top() == b->geometry().top())
        {
            return a->geometry().left() < b->geometry().left();
        }
        return a->geometry().top() < b->geometry().top();
    });

    ++m_buttonEdgeResolveCount;
    syncRadioPerfCounters();

    if (radios.size() <= 1)
    {
        return {true, true};
    }
    return {radios.first() == this, radios.last() == this};
}

QRectF AntRadio::indicatorRect() const
{
    if (m_buttonStyle)
    {
        return QRectF();
    }
    return layoutCache().indicatorRect;
}

QRectF AntRadio::textRect() const
{
    return layoutCache().textRect;
}

QRectF AntRadio::buttonFrameRect() const
{
    return layoutCache().buttonFrame;
}

const QPainterPath& AntRadio::buttonSegmentPath() const
{
    return layoutCache().buttonPath;
}

QFont AntRadio::radioFont() const
{
    QFont f = font();
    f.setPixelSize(antTheme->tokens().fontSize);
    return f;
}

QRect AntRadio::visualStateRect() const
{
    if (m_buttonStyle)
    {
        return rect();
    }
    return indicatorRect().toAlignedRect().adjusted(-4, -4, 4, 4).intersected(rect());
}

void AntRadio::invalidateLayoutCache() const
{
    m_layoutCache.valid = false;
    syncRadioPerfCounters();
}

void AntRadio::updateVisualStateRegion(const QRect& oldRect)
{
    QRect dirty = visualStateRect();
    if (oldRect.isValid())
    {
        dirty = dirty.united(oldRect);
    }
    if (!dirty.isValid() || dirty.isEmpty())
    {
        dirty = rect();
    }
    ++m_regionUpdateCount;
    setProperty("antRadioLastUpdateMode", m_buttonStyle ? QStringLiteral("button") : QStringLiteral("indicator"));
    syncRadioPerfCounters();
    update(dirty);
}

void AntRadio::syncRadioPerfCounters() const
{
    auto* self = const_cast<AntRadio*>(this);
    self->setProperty("antRadioLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antRadioSizeHintResolveCount", m_sizeHintResolveCount);
    self->setProperty("antRadioButtonEdgeResolveCount", m_buttonEdgeResolveCount);
    self->setProperty("antRadioRegionUpdateCount", m_regionUpdateCount);
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
