#include "AntFlex.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

AntFlex::AntFlex(QWidget* parent)
    : QWidget(parent)
{
    rebuildLayout();
}

bool AntFlex::vertical() const { return m_vertical; }
void AntFlex::setVertical(bool v)
{
    if (m_vertical == v) return;
    m_vertical = v;
    rebuildLayout();
    Q_EMIT verticalChanged(m_vertical);
}

int AntFlex::gap() const { return m_gap; }
void AntFlex::setGap(int px)
{
    if (m_gap == px) return;
    m_gap = px;
    if (m_layout) m_layout->setSpacing(px);
    Q_EMIT gapChanged(m_gap);
}

bool AntFlex::wrap() const { return m_wrap; }
void AntFlex::setWrap(bool w)
{
    if (m_wrap == w) return;
    m_wrap = w;
    rebuildLayout();
    Q_EMIT wrapChanged(m_wrap);
}

void AntFlex::addWidget(QWidget* widget)
{
    m_children.append(widget);
    m_layout->addWidget(widget);
}

void AntFlex::addStretch()
{
    if (auto* hb = qobject_cast<QHBoxLayout*>(m_layout))
        hb->addStretch();
    else if (auto* vb = qobject_cast<QVBoxLayout*>(m_layout))
        vb->addStretch();
}

void AntFlex::rebuildLayout()
{
    delete m_layout;
    m_layout = m_vertical ? static_cast<QLayout*>(new QVBoxLayout(this))
                          : static_cast<QLayout*>(new QHBoxLayout(this));
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(m_gap);

    for (auto* w : m_children)
        m_layout->addWidget(w);
}
