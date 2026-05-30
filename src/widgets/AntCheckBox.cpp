#include "AntCheckBox.h"

#include <algorithm>
#include <QEvent>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QSizePolicy>

#include "../styles/AntCheckBoxStyle.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"
#include "core/AntWave.h"

namespace
{
constexpr int IndicatorSize = 16;
}

AntCheckBox::AntCheckBox(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    auto* checkboxStyle = new AntCheckBoxStyle(style());
    checkboxStyle->setParent(this);
    setStyle(checkboxStyle);

    connect(antTheme, &AntTheme::themeModeChanged, this, [this]() {
        invalidateLayoutCache();
        update();
    });
    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        invalidateLayoutCache();
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        update();
    });
    syncCheckBoxPerfCounters();
}

AntCheckBox::AntCheckBox(const QString& text, QWidget* parent)
    : AntCheckBox(parent)
{
    setText(text);
}

bool AntCheckBox::isChecked() const { return m_checked; }

void AntCheckBox::setChecked(bool checked)
{
    setCheckState(checked ? Qt::Checked : Qt::Unchecked);
}

Qt::CheckState AntCheckBox::checkState() const
{
    if (m_indeterminate)
    {
        return Qt::PartiallyChecked;
    }
    return m_checked ? Qt::Checked : Qt::Unchecked;
}

void AntCheckBox::setCheckState(Qt::CheckState state)
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

    updateIndicatorRegion();
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

bool AntCheckBox::isIndeterminate() const { return m_indeterminate; }

void AntCheckBox::setIndeterminate(bool indeterminate)
{
    if (indeterminate)
    {
        setCheckState(Qt::PartiallyChecked);
        return;
    }
    setCheckState(m_checked ? Qt::Checked : Qt::Unchecked);
}

bool AntCheckBox::isTristate() const { return m_tristate; }

void AntCheckBox::setTristate(bool tristate)
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

void AntCheckBox::toggle()
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

void AntCheckBox::click()
{
    if (!isEnabled())
    {
        return;
    }
    toggle();
    Q_EMIT clicked(m_checked);
}

QString AntCheckBox::text() const { return m_text; }

void AntCheckBox::setText(const QString& text)
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

QSize AntCheckBox::sizeHint() const
{
    return layoutData().sizeHint;
}

QSize AntCheckBox::minimumSizeHint() const
{
    return QSize(16, 16);
}

void AntCheckBox::enterEvent(AntEnterEvent* event)
{
    if (!m_hovered)
    {
        m_hovered = true;
        updateIndicatorRegion();
    }
    QWidget::enterEvent(event);
}

void AntCheckBox::leaveEvent(QEvent* event)
{
    const bool changed = m_hovered || m_pressed;
    m_hovered = false;
    m_pressed = false;
    if (changed)
    {
        updateIndicatorRegion();
    }
    QWidget::leaveEvent(event);
}

void AntCheckBox::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && isEnabled())
    {
        m_pressed = true;
        updateIndicatorRegion();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntCheckBox::mouseReleaseEvent(QMouseEvent* event)
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
        updateIndicatorRegion();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void AntCheckBox::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    else if (event->type() == QEvent::FontChange || event->type() == QEvent::ApplicationFontChange || event->type() == QEvent::StyleChange)
    {
        invalidateLayoutCache();
        updateGeometry();
        update();
    }
    QWidget::changeEvent(event);
}

void AntCheckBox::keyPressEvent(QKeyEvent* event)
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

QRectF AntCheckBox::indicatorRect() const
{
    return layoutData().indicatorRect;
}

QColor AntCheckBox::indicatorBorderColor() const
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

QColor AntCheckBox::indicatorBackgroundColor() const
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

bool AntCheckBox::isHoveredState() const
{
    return m_hovered;
}

bool AntCheckBox::isPressedState() const
{
    return m_pressed;
}

const AntCheckBox::LayoutData& AntCheckBox::layoutData() const
{
    QFont f = font();
    f.setPixelSize(antTheme->tokens().fontSize);
    const QSize widgetSize = size();
    if (m_layoutCacheValid &&
        m_layoutCacheRevision == m_layoutRevision &&
        m_layoutCacheWidgetSize == widgetSize &&
        m_layoutCacheFont == f)
    {
        ++m_layoutCacheHitCount;
        syncCheckBoxPerfCounters();
        return m_cachedLayout;
    }

    constexpr int textSpacing = 8;
    QFontMetrics fm(f);
    const int textWidth = m_text.isEmpty() ? 0 : textSpacing + fm.horizontalAdvance(m_text);
    const int hintHeight = std::max(IndicatorSize, fm.height());

    LayoutData data;
    data.indicatorRect = QRectF(0.5, (height() - IndicatorSize) / 2.0 + 0.5, IndicatorSize - 1, IndicatorSize - 1);
    data.textRect = QRectF(data.indicatorRect.right() + textSpacing,
                           0,
                           std::max<qreal>(0, width() - data.indicatorRect.right() - textSpacing),
                           height());
    data.sizeHint = QSize(IndicatorSize + textWidth, hintHeight);

    const QRectF mark = data.indicatorRect.adjusted(3.5, 3.5, -3.0, -3.0);
    data.checkPath.moveTo(mark.left(), mark.center().y());
    data.checkPath.lineTo(mark.left() + mark.width() * 0.36, mark.bottom() - 1.0);
    data.checkPath.lineTo(mark.right(), mark.top() + 1.0);
    data.indeterminateRect = QRectF(data.indicatorRect.left() + 4,
                                    data.indicatorRect.center().y() - 1.5,
                                    data.indicatorRect.width() - 8,
                                    3);

    m_cachedLayout = data;
    m_layoutCacheRevision = m_layoutRevision;
    m_layoutCacheWidgetSize = widgetSize;
    m_layoutCacheFont = f;
    m_layoutCacheValid = true;
    ++m_layoutBuildCount;
    syncCheckBoxPerfCounters();
    return m_cachedLayout;
}

QRect AntCheckBox::indicatorDirtyRect() const
{
    return layoutData().indicatorRect.toAlignedRect().adjusted(-4, -4, 4, 4).intersected(rect());
}

void AntCheckBox::invalidateLayoutCache()
{
    ++m_layoutRevision;
    if (m_layoutRevision == 0)
    {
        m_layoutRevision = 1;
    }
    m_layoutCacheValid = false;
}

void AntCheckBox::updateIndicatorRegion()
{
    const QRect dirty = indicatorDirtyRect();
    if (dirty.isNull())
    {
        update();
        return;
    }
    ++m_indicatorScopedUpdateCount;
    syncCheckBoxPerfCounters();
    update(dirty);
}

void AntCheckBox::syncCheckBoxPerfCounters() const
{
    auto* self = const_cast<AntCheckBox*>(this);
    self->setProperty("antCheckBoxLayoutCacheHitCount", m_layoutCacheHitCount);
    self->setProperty("antCheckBoxLayoutBuildCount", m_layoutBuildCount);
    self->setProperty("antCheckBoxIndicatorScopedUpdateCount", m_indicatorScopedUpdateCount);
}
