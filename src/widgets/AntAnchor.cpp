#include "AntAnchor.h"

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

#include "core/AntTheme.h"

AntAnchor::AntAnchor(QWidget* parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(2);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

void AntAnchor::setScrollArea(QScrollArea* area)
{
    if (m_scrollArea)
        m_scrollArea->verticalScrollBar()->disconnect(this);

    m_scrollArea = area;

    if (m_scrollArea)
    {
        connect(m_scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
            int best = -1;
            for (int i = 0; i < m_links.size(); ++i)
            {
                if (value >= m_links[i].targetY - 16)
                    best = i;
            }
            if (best != m_activeIndex)
            {
                m_activeIndex = best;
                update();
                Q_EMIT activeIndexChanged(m_activeIndex);
            }
        });
    }
}

int AntAnchor::activeIndex() const { return m_activeIndex; }

void AntAnchor::addLink(const QString& title, int targetY)
{
    m_links.append({title, targetY});

    auto* label = new QLabel(title, this);
    label->setCursor(Qt::PointingHandCursor);
    label->setContentsMargins(16, 4, 8, 4);
    label->setProperty("anchorIndex", m_links.size() - 1);
    label->installEventFilter(this);

    QFont f = label->font();
    f.setPixelSize(antTheme->tokens().fontSizeSM);
    label->setFont(f);

    m_layout->addWidget(label);
}

bool AntAnchor::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        auto* label = qobject_cast<QLabel*>(watched);
        if (label)
        {
            int idx = label->property("anchorIndex").toInt();
            if (idx >= 0 && idx < m_links.size())
            {
                m_activeIndex = idx;
                update();
                Q_EMIT linkClicked(idx, m_links[idx].targetY);
                Q_EMIT activeIndexChanged(m_activeIndex);
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntAnchor::paintEvent(QPaintEvent*)
{
    // Re-color labels based on active state
    const auto& token = antTheme->tokens();
    for (int i = 0; i < m_layout->count(); ++i)
    {
        if (auto* label = qobject_cast<QLabel*>(m_layout->itemAt(i)->widget()))
        {
            QColor c = (i == m_activeIndex) ? token.colorPrimary : token.colorTextSecondary;
            label->setStyleSheet(QStringLiteral("color:%1; border-left:2px solid %2;")
                                     .arg(c.name(), i == m_activeIndex ? c.name() : QStringLiteral("transparent")));
        }
    }
}
