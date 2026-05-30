#include "AntMentions.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QSizePolicy>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

#include "../styles/AntAutoCompleteStyle.h"
#include "core/AntPopupMotion.h"
#include "core/AntStyleBase.h"
#include "core/AntTheme.h"
#include "core/AntThemeRefresh_p.h"

namespace
{
constexpr int kPopupShadowMargin = 32;
constexpr int kPopupInnerPadding = 4;
constexpr int kOptionHeight = 32;

class SuggestionPopupFrame : public QFrame
{
public:
    explicit SuggestionPopupFrame(QWidget* parent = nullptr, Qt::WindowFlags flags = {})
        : QFrame(parent, flags)
    {
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const QRect panelRect = rect().adjusted(kPopupShadowMargin, kPopupShadowMargin,
                                                -kPopupShadowMargin, -kPopupShadowMargin);
        antTheme->drawEffectShadow(&painter, panelRect, 10, token.borderRadiusLG, 0.45);
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(QRectF(panelRect).adjusted(0.5, 0.5, -0.5, -0.5),
                                token.borderRadiusLG, token.borderRadiusLG);
    }
};

} // namespace

class AntMentionItem : public QWidget
{
public:
    explicit AntMentionItem(QWidget* parent)
        : QWidget(parent)
    {
        setFixedHeight(kOptionHeight);
        setCursor(Qt::PointingHandCursor);
    }

    bool setMentionText(const QString& text)
    {
        if (m_text == text)
        {
            return false;
        }
        m_text = text;
        setProperty("antMentionItemText", m_text);
        update();
        return true;
    }

    void setItemIndex(int index)
    {
        m_index = index;
        setObjectName(QStringLiteral("AntMentionItem_%1").arg(index));
    }

    bool setHighlighted(bool highlighted)
    {
        if (m_highlighted == highlighted)
        {
            return false;
        }
        m_highlighted = highlighted;
        setProperty("antMentionItemHighlighted", m_highlighted);
        update();
        return true;
    }

    std::function<void(int)> onClicked;
    std::function<void(int)> onHovered;

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const bool active = m_highlighted || underMouse();
        if (active)
        {
            p.setPen(Qt::NoPen);
            p.setBrush(token.colorFillTertiary);
            p.drawRoundedRect(rect().adjusted(4, 2, -4, -2), 4, 4);
        }

        QFont f = font();
        f.setPixelSize(token.fontSize);
        p.setFont(f);
        p.setPen(token.colorText);
        p.drawText(rect().adjusted(12, 0, -12, 0), Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }

    void mousePressEvent(QMouseEvent* e) override
    {
        if (e->button() == Qt::LeftButton && onClicked)
        {
            onClicked(m_index);
        }
    }

    void enterEvent(AntEnterEvent*) override
    {
        if (onHovered)
        {
            onHovered(m_index);
        }
        update();
    }

    void leaveEvent(QEvent*) override { update(); }

private:
    QString m_text;
    int m_index = -1;
    bool m_highlighted = false;
};

AntMentions::AntMentions(QWidget* parent)
    : QWidget(parent)
{
    auto* s = new AntAutoCompleteStyle(style());
    s->setParent(this);
    setStyle(s);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(sizeHint().height());

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setFrame(false);
    applyLineEditTheme();
    m_lineEdit->installEventFilter(this);

    connect(m_lineEdit, &QLineEdit::textEdited, this, [this]() {
        scheduleSuggestionRefresh();
        Q_EMIT textChanged(m_lineEdit->text());
    });

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 4, 8, 0);
    layout->addWidget(m_lineEdit);
    layout->setAlignment(m_lineEdit, Qt::AlignTop);

    m_popup = new SuggestionPopupFrame(this, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_popup->setObjectName(QStringLiteral("AntMentionsPopup"));
    m_popup->setAttribute(Qt::WA_TranslucentBackground);
    m_popup->setAttribute(Qt::WA_ShowWithoutActivating);
    m_popupLayout = new QVBoxLayout(m_popup);
    m_popupLayout->setContentsMargins(kPopupShadowMargin + kPopupInnerPadding,
                                      kPopupShadowMargin + kPopupInnerPadding,
                                      kPopupShadowMargin + kPopupInnerPadding,
                                      kPopupShadowMargin + kPopupInnerPadding);
    m_popupLayout->setSpacing(0);

    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(0);
    connect(m_filterTimer, &QTimer::timeout, this, &AntMentions::refreshSuggestions);

    connect(antTheme, &AntTheme::themeModeAboutToChange, this, [this](Ant::ThemeMode) {
        AntThemeRefresh::cacheGeometryHints(this);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        applyLineEditTheme();
        const int nextHeight = sizeHint().height();
        const bool fixedHeightChanged = minimumHeight() != nextHeight || maximumHeight() != nextHeight;
        if (fixedHeightChanged)
        {
            setFixedHeight(nextHeight);
        }
        setProperty("antMentionsThemeFixedHeightChanged", fixedHeightChanged);
        for (AntMentionItem* item : std::as_const(m_itemWidgets))
        {
            if (item)
            {
                item->update();
            }
        }
        AntThemeRefresh::updateGeometryIfSizeHintChanged(this);
        update();
        if (m_popup && m_popup->isVisible())
        {
            m_popup->update();
        }
    });
    syncMentionsPerfCounters();
}

QString AntMentions::placeholderText() const { return m_placeholderText; }
void AntMentions::setPlaceholderText(const QString& text)
{
    if (m_placeholderText == text)
    {
        return;
    }
    m_placeholderText = text;
    m_lineEdit->setPlaceholderText(text);
    Q_EMIT placeholderTextChanged(text);
}

QString AntMentions::prefix() const { return m_prefix; }
void AntMentions::setPrefix(const QString& prefix)
{
    if (m_prefix == prefix)
    {
        return;
    }
    m_prefix = prefix;
    m_prefixPos = -1;
    if (m_open || (m_lineEdit && m_lineEdit->text().contains(m_prefix)))
    {
        scheduleSuggestionRefresh();
    }
    Q_EMIT prefixChanged(prefix);
}

int AntMentions::rows() const { return m_rows; }
void AntMentions::setRows(int rows)
{
    rows = std::max(1, rows);
    if (m_rows == rows)
    {
        return;
    }
    m_rows = rows;
    setFixedHeight(sizeHint().height());
    updateGeometry();
    update();
    Q_EMIT rowsChanged(m_rows);
}

QString AntMentions::text() const { return m_lineEdit->text(); }

void AntMentions::setSuggestions(const QStringList& items)
{
    if (m_suggestions == items)
    {
        return;
    }
    m_suggestions = items;
    ++m_suggestionsRevision;
    invalidateSuggestionCache();
    scheduleSuggestionRefresh();
}

void AntMentions::addSuggestion(const QString& text)
{
    m_suggestions.append(text);
    ++m_suggestionsRevision;
    invalidateSuggestionCache();
    scheduleSuggestionRefresh();
}

QSize AntMentions::sizeHint() const
{
    const auto& token = antTheme->tokens();
    if (m_rows <= 1)
    {
        return QSize(220, token.controlHeight);
    }
    return QSize(400, token.controlHeight + (m_rows - 1) * 23);
}

void AntMentions::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const auto& token = antTheme->tokens();
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    const QRect frame = rect().adjusted(0, 0, -1, -1);
    AntStyleBase::drawCrispRoundedRect(&painter, frame, QPen(token.colorBorder, token.lineWidth),
        token.colorBgContainer, token.borderRadius, token.borderRadius);
}

bool AntMentions::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_lineEdit && event->type() == QEvent::KeyPress)
    {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        if (m_open && m_visibleSuggestionCount > 0)
        {
            if (keyEvent->key() == Qt::Key_Down)
            {
                setHighlightedIndex((m_highlightedIndex + 1) % m_visibleSuggestionCount);
                return true;
            }
            if (keyEvent->key() == Qt::Key_Up)
            {
                const int previous = m_highlightedIndex < 0 ? 0 : m_highlightedIndex;
                setHighlightedIndex((previous + m_visibleSuggestionCount - 1) % m_visibleSuggestionCount);
                return true;
            }
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            {
                selectSuggestion(m_highlightedIndex < 0 ? 0 : m_highlightedIndex);
                return true;
            }
        }
        if (m_open && keyEvent->key() == Qt::Key_Escape)
        {
            hidePopup();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntMentions::applyLineEditTheme()
{
    if (!m_lineEdit)
    {
        return;
    }
    const auto& token = antTheme->tokens();
    QFont font = m_lineEdit->font();
    if (font.pixelSize() != token.fontSize)
    {
        font.setPixelSize(token.fontSize);
        m_lineEdit->setFont(font);
    }

    QPalette palette = m_lineEdit->palette();
    palette.setColor(QPalette::Base, Qt::transparent);
    palette.setColor(QPalette::Text, token.colorText);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    palette.setColor(QPalette::PlaceholderText, token.colorTextPlaceholder);
#endif
    if (m_lineEdit->palette() != palette)
    {
        m_lineEdit->setPalette(palette);
    }
}

void AntMentions::scheduleSuggestionRefresh()
{
    if (!m_filterTimer)
    {
        refreshSuggestions();
        return;
    }
    if (!m_filterTimer->isActive())
    {
        m_filterTimer->start();
    }
}

void AntMentions::refreshSuggestions()
{
    ++m_refreshCount;
    syncMentionsPerfCounters();

    bool ok = false;
    const QString filter = currentFilterText(&ok);
    if (!ok)
    {
        hidePopup();
        return;
    }

    const QStringList matched = filteredSuggestions(filter);
    if (matched.isEmpty())
    {
        hidePopup();
        return;
    }

    syncPopupRows(matched);
    updatePopupGeometry(m_visibleSuggestionCount);
    showPopup();
}

QString AntMentions::currentFilterText(bool* ok) const
{
    if (ok)
    {
        *ok = false;
    }
    if (!m_lineEdit || m_prefix.isEmpty())
    {
        return QString();
    }

    const QString text = m_lineEdit->text();
    const int lastPrefix = text.lastIndexOf(m_prefix);
    if (lastPrefix < 0 || (lastPrefix > 0 && text.at(lastPrefix - 1) != QLatin1Char(' ')))
    {
        return QString();
    }

    const_cast<AntMentions*>(this)->m_prefixPos = lastPrefix;
    if (ok)
    {
        *ok = true;
    }
    return text.mid(lastPrefix + m_prefix.size());
}

QStringList AntMentions::filteredSuggestions(const QString& filter)
{
    if (m_cachedFilterRevision == m_suggestionsRevision && m_cachedFilter == filter)
    {
        return m_filteredSuggestions;
    }

    m_cachedFilter = filter;
    m_cachedFilterRevision = m_suggestionsRevision;
    m_filteredSuggestions.clear();
    for (const QString& suggestion : std::as_const(m_suggestions))
    {
        if (filter.isEmpty() || suggestion.contains(filter, Qt::CaseInsensitive))
        {
            m_filteredSuggestions.append(suggestion);
        }
    }
    ++m_filterResolveCount;
    syncMentionsPerfCounters();
    return m_filteredSuggestions;
}

void AntMentions::syncPopupRows(const QStringList& matched)
{
    const int visibleCount = std::min(static_cast<int>(matched.size()), 6);
    ensurePopupRows(visibleCount);

    int rowTextChanges = 0;
    for (int i = 0; i < m_itemWidgets.size(); ++i)
    {
        AntMentionItem* item = m_itemWidgets.at(i);
        if (!item)
        {
            continue;
        }
        const bool visible = i < visibleCount;
        item->setVisible(visible);
        item->setItemIndex(i);
        if (visible && item->setMentionText(matched.at(i)))
        {
            ++rowTextChanges;
        }
    }

    if (rowTextChanges > 0)
    {
        m_rowTextApplyCount += rowTextChanges;
    }

    const int oldVisibleCount = m_visibleSuggestionCount;
    m_visibleSuggestionCount = visibleCount;
    if (visibleCount <= 0)
    {
        setHighlightedIndex(-1);
    }
    else if (oldVisibleCount != visibleCount || m_highlightedIndex < 0 || m_highlightedIndex >= visibleCount)
    {
        setHighlightedIndex(0);
    }
    else
    {
        updateHighlightedRows(m_highlightedIndex, m_highlightedIndex);
    }
    syncMentionsPerfCounters();
}

void AntMentions::ensurePopupRows(int count)
{
    while (m_itemWidgets.size() < count)
    {
        auto* item = new AntMentionItem(m_popup);
        item->onClicked = [this](int index) {
            selectSuggestion(index);
        };
        item->onHovered = [this](int index) {
            setHighlightedIndex(index);
        };
        m_popupLayout->addWidget(item);
        m_itemWidgets.append(item);
        ++m_popupRowBuildCount;
    }
    syncMentionsPerfCounters();
}

void AntMentions::updatePopupGeometry(int visibleCount)
{
    if (!m_popup || visibleCount <= 0)
    {
        return;
    }

    const QPoint pos = mapToGlobal(QPoint(-kPopupShadowMargin, height() + 4 - kPopupShadowMargin));
    const QRect nextGeometry(pos.x(),
                             pos.y(),
                             width() + kPopupShadowMargin * 2,
                             visibleCount * kOptionHeight + kPopupInnerPadding * 2 + kPopupShadowMargin * 2);
    if (m_lastPopupGeometry == nextGeometry && m_popup->geometry() == nextGeometry)
    {
        return;
    }
    m_lastPopupGeometry = nextGeometry;
    m_popup->setGeometry(nextGeometry);
    ++m_popupGeometryApplyCount;
    syncMentionsPerfCounters();
}

void AntMentions::setHighlightedIndex(int index)
{
    if (m_visibleSuggestionCount <= 0)
    {
        index = -1;
    }
    else
    {
        index = qBound(0, index, m_visibleSuggestionCount - 1);
    }

    if (m_highlightedIndex == index)
    {
        return;
    }

    const int previous = m_highlightedIndex;
    m_highlightedIndex = index;
    updateHighlightedRows(previous, m_highlightedIndex);
}

void AntMentions::updateHighlightedRows(int previous, int current)
{
    int changedRows = 0;
    if (previous >= 0 && previous < m_itemWidgets.size())
    {
        if (m_itemWidgets.at(previous)->setHighlighted(previous == current))
        {
            ++changedRows;
        }
    }
    if (current >= 0 && current < m_itemWidgets.size() && current != previous)
    {
        if (m_itemWidgets.at(current)->setHighlighted(true))
        {
            ++changedRows;
        }
    }
    if (changedRows > 0)
    {
        m_highlightedRowUpdateCount += changedRows;
        syncMentionsPerfCounters();
    }
}

void AntMentions::showPopup()
{
    if (!m_popup || m_visibleSuggestionCount <= 0)
    {
        return;
    }
    if (m_open)
    {
        m_popup->update();
        return;
    }
    m_open = true;
    AntPopupMotion::show(m_popup);
}

void AntMentions::hidePopup()
{
    if (!m_open)
    {
        return;
    }
    m_open = false;
    m_visibleSuggestionCount = 0;
    setHighlightedIndex(-1);
    AntPopupMotion::hide(m_popup);
}

void AntMentions::selectSuggestion(int index)
{
    if (index < 0 || index >= m_visibleSuggestionCount || index >= m_filteredSuggestions.size())
    {
        return;
    }

    const QString selectedText = m_filteredSuggestions.at(index);
    QString currentText = m_lineEdit->text();
    currentText = currentText.left(m_prefixPos) + m_prefix + selectedText + QStringLiteral(" ");
    m_lineEdit->setText(currentText);
    hidePopup();
    m_lineEdit->setFocus();
    Q_EMIT mentionSelected(selectedText);
}

void AntMentions::invalidateSuggestionCache()
{
    m_cachedFilterRevision = -1;
    m_cachedFilter.clear();
    m_filteredSuggestions.clear();
    syncMentionsPerfCounters();
}

void AntMentions::syncMentionsPerfCounters() const
{
    auto* self = const_cast<AntMentions*>(this);
    self->setProperty("antMentionsFilterResolveCount", m_filterResolveCount);
    self->setProperty("antMentionsPopupRowBuildCount", m_popupRowBuildCount);
    self->setProperty("antMentionsRowTextApplyCount", m_rowTextApplyCount);
    self->setProperty("antMentionsPopupGeometryApplyCount", m_popupGeometryApplyCount);
    self->setProperty("antMentionsHighlightedRowUpdateCount", m_highlightedRowUpdateCount);
    self->setProperty("antMentionsRefreshCount", m_refreshCount);
    self->setProperty("antMentionsVisibleSuggestionCount", m_visibleSuggestionCount);
    self->setProperty("antMentionsDebounceIntervalMs", m_filterTimer ? m_filterTimer->interval() : 0);
}
