#include "AntMentions.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

#include <algorithm>

#include "../styles/AntAutoCompleteStyle.h"
#include "core/AntPopupMotion.h"
#include "core/AntStyleBase.h"
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

class MentionItem : public QWidget
{
public:
    MentionItem(const QString& text, QWidget* parent)
        : QWidget(parent), m_text(text)
    {
        setFixedHeight(kOptionHeight);
        setCursor(Qt::PointingHandCursor);
    }

    std::function<void()> onClicked;

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        bool hovered = underMouse();
        if (hovered)
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
        if (e->button() == Qt::LeftButton && onClicked) onClicked();
    }

    void enterEvent(QEnterEvent*) override { update(); }
    void leaveEvent(QEvent*) override { update(); }

private:
    QString m_text;
};

} // namespace

AntMentions::AntMentions(QWidget* parent)
    : QWidget(parent)
{
    auto* s = new AntAutoCompleteStyle(style());
    s->setParent(this);
    setStyle(s);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(sizeHint().height());

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setFrame(false);
    {
        QFont f = m_lineEdit->font();
        f.setPixelSize(antTheme->tokens().fontSize);
        m_lineEdit->setFont(f);
        QPalette p = m_lineEdit->palette();
        p.setColor(QPalette::Base, Qt::transparent);
        p.setColor(QPalette::Text, antTheme->tokens().colorText);
        p.setColor(QPalette::PlaceholderText, antTheme->tokens().colorTextPlaceholder);
        m_lineEdit->setPalette(p);
    }

    connect(m_lineEdit, &QLineEdit::textEdited, this, [this]() {
        checkForPrefix();
        Q_EMIT textChanged(m_lineEdit->text());
    });

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 4, 8, 0);
    layout->addWidget(m_lineEdit);
    layout->setAlignment(m_lineEdit, Qt::AlignTop);

    m_popup = new SuggestionPopupFrame(this, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_popup->setAttribute(Qt::WA_TranslucentBackground);
    m_popup->setAttribute(Qt::WA_ShowWithoutActivating);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        setFixedHeight(sizeHint().height());
        updateGeometry();
        update();
    });
}

QString AntMentions::placeholderText() const { return m_placeholderText; }
void AntMentions::setPlaceholderText(const QString& text)
{
    m_placeholderText = text;
    m_lineEdit->setPlaceholderText(text);
    Q_EMIT placeholderTextChanged(text);
}

QString AntMentions::prefix() const { return m_prefix; }
void AntMentions::setPrefix(const QString& prefix)
{
    m_prefix = prefix;
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
    m_suggestions = items;
}

void AntMentions::addSuggestion(const QString& text)
{
    m_suggestions.append(text);
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

void AntMentions::checkForPrefix()
{
    const QString t = m_lineEdit->text();
    int lastPrefix = t.lastIndexOf(m_prefix);
    if (lastPrefix >= 0 && (lastPrefix == 0 || t[lastPrefix - 1] == QLatin1Char(' ')))
    {
        m_prefixPos = lastPrefix;
        QString filter = t.mid(lastPrefix + 1).toLower();

        // Show all suggestions if no filter, otherwise filter
        QList<QString> matched;
        for (const auto& s : m_suggestions)
        {
            if (filter.isEmpty() || s.toLower().contains(filter))
                matched.append(s);
        }
        if (!matched.isEmpty())
        {
            // Rebuild popup
            while (QLayoutItem* item = m_popup->layout() ? m_popup->layout()->takeAt(0) : nullptr)
            {
                if (item->widget()) delete item->widget();
                delete item;
            }
            delete m_popup->layout();
            auto* lay = new QVBoxLayout(m_popup);
            lay->setContentsMargins(kPopupShadowMargin + kPopupInnerPadding,
                                    kPopupShadowMargin + kPopupInnerPadding,
                                    kPopupShadowMargin + kPopupInnerPadding,
                                    kPopupShadowMargin + kPopupInnerPadding);
            lay->setSpacing(0);

            int count = std::min(static_cast<int>(matched.size()), 6);
            for (int i = 0; i < count; ++i)
            {
                auto* item = new MentionItem(matched[i], m_popup);
                item->onClicked = [this, text = matched[i]]() {
                    QString t = m_lineEdit->text();
                    t = t.left(m_prefixPos) + m_prefix + text + QStringLiteral(" ");
                    m_lineEdit->setText(t);
                    hidePopup();
                    m_lineEdit->setFocus();
                    Q_EMIT mentionSelected(text);
                };
                lay->addWidget(item);
            }

            const QPoint pos = mapToGlobal(QPoint(-kPopupShadowMargin, height() + 4));
            m_popup->setGeometry(pos.x(), pos.y(), width() + kPopupShadowMargin * 2,
                                 count * kOptionHeight + kPopupInnerPadding * 2 + kPopupShadowMargin * 2);
            if (m_open)
            {
                m_popup->update();
            }
            else
            {
                AntPopupMotion::show(m_popup);
            }
            m_open = true;
            return;
        }
    }
    hidePopup();
}

void AntMentions::showPopup()
{
    if (!m_open) return;
    AntPopupMotion::show(m_popup);
}

void AntMentions::hidePopup()
{
    if (!m_open) return;
    m_open = false;
    AntPopupMotion::hide(m_popup);
}
