#include "AntTour.h"

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>

#include "AntButton.h"
#include "core/AntTheme.h"

namespace
{

class TourOverlay : public QDialog
{
public:
    QLabel* titleLabel = nullptr;
    QLabel* descLabel = nullptr;
    AntButton* prevBtn = nullptr;
    AntButton* nextBtn = nullptr;
    AntButton* closeBtn = nullptr;
    QWidget* target = nullptr;
    QWidget* tooltip = nullptr;

    TourOverlay(QWidget* parent)
        : QDialog(parent, Qt::FramelessWindowHint)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_ShowWithoutActivating);
        setModal(true);

        tooltip = new QWidget(this);
        tooltip->setFixedWidth(280);
        auto* lay = new QVBoxLayout(tooltip);
        titleLabel = new QLabel(tooltip);
        QFont f = titleLabel->font(); f.setBold(true); f.setPixelSize(16); titleLabel->setFont(f);
        lay->addWidget(titleLabel);
        descLabel = new QLabel(tooltip);
        descLabel->setWordWrap(true);
        lay->addWidget(descLabel);
        auto* btnRow = new QHBoxLayout();
        prevBtn = new AntButton(QStringLiteral("Prev"));
        prevBtn->setButtonSize(Ant::Size::Small);
        nextBtn = new AntButton(QStringLiteral("Next"));
        nextBtn->setButtonType(Ant::ButtonType::Primary);
        nextBtn->setButtonSize(Ant::Size::Small);
        closeBtn = new AntButton(QStringLiteral("Close"));
        closeBtn->setButtonType(Ant::ButtonType::Text);
        closeBtn->setButtonSize(Ant::Size::Small);
        btnRow->addWidget(prevBtn);
        btnRow->addWidget(nextBtn);
        btnRow->addWidget(closeBtn);
        lay->addLayout(btnRow);
    }

    void setTarget(QWidget* t)
    {
        target = t;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.fillRect(rect(), QColor(0, 0, 0, 120));

        if (target && target->isVisible())
        {
            QPoint tl = target->mapToGlobal(QPoint(0, 0));
            tl = mapFromGlobal(tl);
            QRect targetR(tl, target->size());

            // Clear the mask over the target
            p.setCompositionMode(QPainter::CompositionMode_Clear);
            p.fillRect(targetR.adjusted(-8, -8, 8, 8), Qt::black);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);

            // Draw highlight border
            p.setPen(QPen(token.colorPrimary, 2));
            p.setBrush(Qt::NoBrush);
            p.drawRect(targetR.adjusted(-8, -8, 8, 8));
        }
    }
};

} // namespace

AntTour::AntTour(QObject* parent) : QObject(parent) {}

void AntTour::addStep(const AntTourStep& step) { m_steps.append(step); }

void AntTour::start()
{
    if (m_steps.isEmpty()) return;
    auto* topWidget = QApplication::activeWindow();
    if (!topWidget) { for (auto* w : QApplication::topLevelWidgets()) { if (w->isWindow()) { topWidget = w; break; } } }
    if (!topWidget) return;

    m_overlay = new TourOverlay(topWidget);
    m_overlay->resize(topWidget->size());
    m_overlay->move(topWidget->mapToGlobal(QPoint(0, 0)));

    connect(dynamic_cast<TourOverlay*>(m_overlay)->prevBtn, &QPushButton::clicked, this, &AntTour::prev);
    connect(dynamic_cast<TourOverlay*>(m_overlay)->nextBtn, &QPushButton::clicked, this, &AntTour::next);
    connect(dynamic_cast<TourOverlay*>(m_overlay)->closeBtn, &QPushButton::clicked, this, &AntTour::close);

    showStep(0);
    m_overlay->show();
}

void AntTour::next()
{
    if (m_current + 1 < m_steps.size()) showStep(m_current + 1);
    else close();
}

void AntTour::prev()
{
    if (m_current > 0) showStep(m_current - 1);
}

void AntTour::close()
{
    if (m_overlay) { m_overlay->close(); m_overlay->deleteLater(); m_overlay = nullptr; }
    m_current = -1;
    Q_EMIT finished();
}

void AntTour::showStep(int index)
{
    if (!m_overlay || index < 0 || index >= m_steps.size()) return;
    m_current = index;
    const auto& step = m_steps[m_current];

    auto* overlay = dynamic_cast<TourOverlay*>(m_overlay);
    overlay->titleLabel->setText(step.title);
    overlay->descLabel->setText(step.description);
    overlay->setTarget(step.target);

    // Position tooltip near target
    if (step.target)
    {
        QPoint tp = step.target->mapToGlobal(QPoint(0, 0));
        tp = m_overlay->mapFromGlobal(tp);
        QRect targetR(tp, step.target->size());
        int tx = targetR.center().x() - 140;
        int ty = targetR.bottom() + 16;
        if (ty + 160 > m_overlay->height()) ty = targetR.top() - 176;
        overlay->tooltip->move(tx, ty);
    }

    bool isLast = (index == m_steps.size() - 1);
    overlay->nextBtn->setText(isLast ? QStringLiteral("Finish") : QStringLiteral("Next"));
    overlay->prevBtn->setVisible(index > 0);
    overlay->update();

    Q_EMIT stepChanged(m_current);
}
