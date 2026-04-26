#include "AntMentions.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

#include "../styles/AntAutoCompleteStyle.h"
#include "core/AntTheme.h"

namespace
{

class MentionItem : public QWidget
{
public:
    MentionItem(const QString& text, QWidget* parent)
        : QWidget(parent), m_text(text)
    {
        setFixedHeight(32);
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
    setFixedHeight(antTheme->tokens().controlHeight);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setFrame(false);
    {
        QPalette p = m_lineEdit->palette();
        p.setColor(QPalette::Base, Qt::transparent);
        m_lineEdit->setPalette(p);
    }

    connect(m_lineEdit, &QLineEdit::textEdited, this, [this]() {
        checkForPrefix();
        Q_EMIT textChanged(m_lineEdit->text());
    });

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->addWidget(m_lineEdit);

    m_popup = new QFrame(nullptr, Qt::ToolTip | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_popup->setAttribute(Qt::WA_TranslucentBackground);
    m_popup->setAttribute(Qt::WA_ShowWithoutActivating);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
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

QString AntMentions::text() const { return m_lineEdit->text(); }

void AntMentions::setSuggestions(const QStringList& items)
{
    m_suggestions = items;
}

void AntMentions::addSuggestion(const QString& text)
{
    m_suggestions.append(text);
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
                if (item->widget()) item->widget()->deleteLater();
                delete item;
            }
            delete m_popup->layout();
            auto* lay = new QVBoxLayout(m_popup);
            lay->setContentsMargins(4, 4, 4, 4);
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

            QPoint pos = mapToGlobal(QPoint(0, height()));
            m_popup->setGeometry(pos.x(), pos.y(), width(), count * 32 + 8);
            m_popup->show();
            m_open = true;
            return;
        }
    }
    hidePopup();
}

void AntMentions::showPopup()
{
    if (!m_open) return;
    m_popup->show();
}

void AntMentions::hidePopup()
{
    if (!m_open) return;
    m_open = false;
    m_popup->hide();
}
