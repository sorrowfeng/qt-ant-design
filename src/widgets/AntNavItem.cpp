#include "AntNavItem.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>

#include "core/AntTheme.h"
#include "core/AntTypes.h"

AntNavItem::AntNavItem(const QString& text, QWidget* parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover);
    setFixedHeight(36);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(0);

    m_label = new QLabel(text);
    layout->addWidget(m_label);
}

bool AntNavItem::isActive() const { return m_active; }

void AntNavItem::setActive(bool active)
{
    if (m_active == active) return;
    m_active = active;
    update();
    Q_EMIT activeChanged(m_active);
}

QString AntNavItem::text() const { return m_label->text(); }

void AntNavItem::setText(const QString& text)
{
    if (m_label->text() == text) return;
    m_label->setText(text);
    Q_EMIT textChanged(text);
}

void AntNavItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    const auto& token = antTheme->tokens();
    const bool isDark = antTheme->themeMode() == Ant::ThemeMode::Dark;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Background
    QColor bg(Qt::transparent);
    if (m_active)
    {
        bg = isDark ? QColor("#111d35") : QColor("#e6f4ff");
    }
    else if (m_hovered)
    {
        bg = isDark ? QColor("#1f1f1f") : QColor("#f5f5f5");
    }
    p.fillRect(rect(), bg);

    // Left border
    if (m_active)
    {
        p.fillRect(0, 0, 3, height(), token.colorPrimary);
    }

    // Text color
    if (m_label)
    {
        QPalette pal = m_label->palette();
        if (m_active)
        {
            pal.setColor(QPalette::WindowText, token.colorPrimary);
            QFont f = m_label->font();
            f.setWeight(QFont::DemiBold);
            m_label->setFont(f);
        }
        else
        {
            pal.setColor(QPalette::WindowText, token.colorText);
            QFont f = m_label->font();
            f.setWeight(QFont::Normal);
            m_label->setFont(f);
        }
        m_label->setPalette(pal);
    }
}

void AntNavItem::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntNavItem::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}

void AntNavItem::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        Q_EMIT clicked();
    }
    QWidget::mousePressEvent(event);
}
