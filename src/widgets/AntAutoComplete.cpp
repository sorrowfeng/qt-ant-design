#include "AntAutoComplete.h"

#include <QFrame>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QScrollBar>
#include <QVBoxLayout>

#include "../styles/AntAutoCompleteStyle.h"
#include "core/AntPopupMotion.h"
#include "core/AntTheme.h"

namespace
{
constexpr int kPopupShadowMargin = 8;
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

class SuggestionItem : public QWidget
{
public:
    SuggestionItem(const QString& text, int index, QWidget* parent)
        : QWidget(parent), m_text(text), m_index(index)
    {
        setAttribute(Qt::WA_Hover);
        setMouseTracking(true);
        setFixedHeight(kOptionHeight);
        setCursor(Qt::PointingHandCursor);
    }

    int itemIndex() const { return m_index; }

    void setHighlighted(bool h)
    {
        if (m_highlighted != h) { m_highlighted = h; update(); }
    }

    bool isHighlighted() const { return m_highlighted; }

std::function<void(int)> onClicked;

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        if (m_highlighted || underMouse())
        {
            p.setPen(Qt::NoPen);
            p.setBrush(m_highlighted ? token.colorFillQuaternary : token.colorFillTertiary);
            p.drawRoundedRect(rect().adjusted(4, 2, -4, -2), 4, 4);
        }

        QFont f = font();
        f.setPixelSize(token.fontSize);
        p.setFont(f);
        p.setPen(token.colorText);
        p.drawText(rect().adjusted(12, 0, -12, 0), Qt::AlignLeft | Qt::AlignVCenter, m_text);
    }

    void enterEvent(QEnterEvent*) override { update(); }
    void leaveEvent(QEvent*) override { update(); }
    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
            if (onClicked) onClicked(m_index);
        QWidget::mousePressEvent(event);
    }

private:
    QString m_text;
    int m_index;
    bool m_highlighted = false;
};

} // namespace

AntAutoComplete::AntAutoComplete(QWidget* parent)
    : QWidget(parent)
{
    auto* s = new AntAutoCompleteStyle(style());
    s->setParent(this);
    setStyle(s);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(antTheme->tokens().controlHeight);
    setMouseTracking(true);

    // Internal line edit
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setFrame(false);
    {
        QPalette p = m_lineEdit->palette();
        p.setColor(QPalette::Base, Qt::transparent);
        m_lineEdit->setPalette(p);
    }
    m_lineEdit->installEventFilter(this);
    m_lineEdit->setMouseTracking(true);

    connect(m_lineEdit, &QLineEdit::textEdited, this, [this]() {
        filterSuggestions();
        Q_EMIT textChanged(m_lineEdit->text());
    });
    connect(m_lineEdit, &QLineEdit::returnPressed, this, [this]() {
        selectHighlighted();
    });

    // Popup — ToolTip avoids stealing keyboard focus from the lineEdit
    m_popup = new SuggestionPopupFrame(nullptr, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_popup->setAttribute(Qt::WA_TranslucentBackground);
    m_popup->setAttribute(Qt::WA_ShowWithoutActivating);
    m_popup->setMouseTracking(true);
    m_popup->installEventFilter(this);

    // Close popup when clicking outside
    qApp->installEventFilter(this);
    m_popupLayout = new QVBoxLayout(m_popup);
    m_popupLayout->setContentsMargins(kPopupShadowMargin + kPopupInnerPadding,
                                      kPopupShadowMargin + kPopupInnerPadding,
                                      kPopupShadowMargin + kPopupInnerPadding,
                                      kPopupShadowMargin + kPopupInnerPadding);
    m_popupLayout->setSpacing(0);

    // Layout
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(12, 0, 8, 0);
    mainLayout->addWidget(m_lineEdit);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

AntAutoComplete::~AntAutoComplete()
{
    if (qApp)
    {
        qApp->removeEventFilter(this);
    }
    delete m_popup;
    m_popup = nullptr;
}

QString AntAutoComplete::placeholderText() const { return m_lineEdit->placeholderText(); }
void AntAutoComplete::setPlaceholderText(const QString& text)
{
    m_lineEdit->setPlaceholderText(text);
    Q_EMIT placeholderTextChanged(text);
}

Qt::CaseSensitivity AntAutoComplete::caseSensitivity() const { return m_caseSensitivity; }
void AntAutoComplete::setCaseSensitivity(Qt::CaseSensitivity cs)
{
    if (m_caseSensitivity == cs) return;
    m_caseSensitivity = cs;
    Q_EMIT caseSensitivityChanged(m_caseSensitivity);
}

int AntAutoComplete::maxVisibleItems() const { return m_maxVisibleItems; }
void AntAutoComplete::setMaxVisibleItems(int n)
{
    n = std::max(3, n);
    if (m_maxVisibleItems == n) return;
    m_maxVisibleItems = n;
    Q_EMIT maxVisibleItemsChanged(m_maxVisibleItems);
}

QString AntAutoComplete::text() const { return m_lineEdit->text(); }
void AntAutoComplete::setText(const QString& text) { m_lineEdit->setText(text); }

QString AntAutoComplete::addSuggestion(const QString& text, const QVariant& data)
{
    m_suggestions.append({text, data});
    return text;
}

void AntAutoComplete::removeSuggestion(int index)
{
    if (index >= 0 && index < m_suggestions.size())
        m_suggestions.removeAt(index);
}

void AntAutoComplete::clearSuggestions()
{
    m_suggestions.clear();
    m_filtered.clear();
}

int AntAutoComplete::suggestionCount() const { return m_suggestions.size(); }

void AntAutoComplete::filterSuggestions()
{
    const QString input = m_lineEdit->text();
    if (input.isEmpty())
    {
        m_filtered.clear();
        hidePopup();
        return;
    }

    m_filtered.clear();
    for (const auto& s : m_suggestions)
    {
        if (s.text.contains(input, m_caseSensitivity))
            m_filtered.append(s);
    }
    m_highlightedIndex = m_filtered.isEmpty() ? -1 : 0;

    if (m_filtered.isEmpty())
        hidePopup();
    else
        showPopup();
}

void AntAutoComplete::showPopup()
{
    const bool wasOpen = m_open;
    m_open = true;

    // Rebuild items
    while (m_popupLayout->count() > 0)
    {
        auto* item = m_popupLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    int count = std::min(static_cast<int>(m_filtered.size()), m_maxVisibleItems);
    for (int i = 0; i < count; ++i)
    {
        auto* item = new SuggestionItem(m_filtered[i].text, i, m_popup);
        item->setHighlighted(i == m_highlightedIndex);
        item->onClicked = [this](int idx) {
            m_highlightedIndex = idx;
            selectHighlighted();
        };
        m_popupLayout->addWidget(item);
    }
    m_popupLayout->addStretch();

    updatePopupGeometry();
    if (!wasOpen)
    {
        AntPopupMotion::show(m_popup);
    }
    else
    {
        m_popup->update();
    }
    m_popup->raise();
}

void AntAutoComplete::hidePopup()
{
    if (!m_open) return;
    m_open = false;
    AntPopupMotion::hide(m_popup);
    m_highlightedIndex = -1;
}

void AntAutoComplete::updatePopupGeometry()
{
    int count = std::min(static_cast<int>(m_filtered.size()), m_maxVisibleItems);
    const int h = count * kOptionHeight + kPopupInnerPadding * 2 + kPopupShadowMargin * 2;
    const QPoint pos = mapToGlobal(QPoint(-kPopupShadowMargin, height() + 4));
    m_popup->setGeometry(pos.x(), pos.y(), width() + kPopupShadowMargin * 2, h);
}

void AntAutoComplete::selectHighlighted()
{
    if (m_highlightedIndex >= 0 && m_highlightedIndex < m_filtered.size())
    {
        const auto& sel = m_filtered[m_highlightedIndex];
        m_lineEdit->setText(sel.text);
        hidePopup();
        Q_EMIT suggestionClicked(sel.text, sel.data);
    }
}

bool AntAutoComplete::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_lineEdit)
    {
        if (event->type() == QEvent::FocusIn) { m_focused = true; update(); }
        if (event->type() == QEvent::FocusOut) { m_focused = false; update(); }
        if (event->type() == QEvent::KeyPress)
        {
            auto* ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Down)
            {
                if (m_filtered.isEmpty()) return false;
                m_highlightedIndex = (m_highlightedIndex + 1) % m_filtered.size();
                showPopup();
                return true;
            }
            if (ke->key() == Qt::Key_Up)
            {
                if (m_filtered.isEmpty()) return false;
                m_highlightedIndex = (m_highlightedIndex - 1 + m_filtered.size()) % m_filtered.size();
                showPopup();
                return true;
            }
            if (ke->key() == Qt::Key_Escape)
            {
                hidePopup();
                return true;
            }
        }
    }
    if (watched == m_popup && event->type() == QEvent::Hide)
    {
        m_open = false;
    }
    // Close popup when clicking outside the widget + popup
    if (m_open && event->type() == QEvent::MouseButtonPress)
    {
        auto* me = static_cast<QMouseEvent*>(event);
        const QPoint globalPos = me->globalPos();
        if (!rect().contains(mapFromGlobal(globalPos)) && !m_popup->rect().contains(m_popup->mapFromGlobal(globalPos)))
        {
            hidePopup();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntAutoComplete::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // Position lineEdit to fill widget
    m_lineEdit->setGeometry(rect().adjusted(12, 0, -8, 0));
    if (m_open)
        updatePopupGeometry();
}
