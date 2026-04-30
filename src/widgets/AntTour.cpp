#include "AntTour.h"

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QVBoxLayout>

#include "AntButton.h"
#include "core/AntTheme.h"

namespace
{

class TourTooltip : public QWidget
{
public:
    explicit TourTooltip(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const QRect cardRect = rect().adjusted(0, 8, 0, 0);
        antTheme->drawEffectShadow(&painter, cardRect.adjusted(2, 2, -2, -2), 10, token.borderRadiusLG, 0.65);

        QPainterPath bubble;
        bubble.addRoundedRect(QRectF(cardRect), token.borderRadiusLG, token.borderRadiusLG);
        QPainterPath arrow;
        const qreal arrowX = width() / 2.0;
        arrow.moveTo(arrowX - 8, cardRect.top());
        arrow.lineTo(arrowX, cardRect.top() - 8);
        arrow.lineTo(arrowX + 8, cardRect.top());
        arrow.closeSubpath();
        bubble.addPath(arrow);

        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorBgElevated);
        painter.drawPath(bubble);
    }
};

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

        tooltip = new TourTooltip(this);
        tooltip->setFixedWidth(180);
        auto* lay = new QVBoxLayout(tooltip);
        lay->setContentsMargins(16, 20, 16, 16);
        lay->setSpacing(10);

        auto* titleRow = new QHBoxLayout();
        titleRow->setContentsMargins(0, 0, 0, 0);
        titleRow->setSpacing(8);
        titleLabel = new QLabel(tooltip);
        QFont f = titleLabel->font();
        f.setWeight(QFont::DemiBold);
        f.setPixelSize(antTheme->tokens().fontSize);
        titleLabel->setFont(f);
        titleLabel->setStyleSheet(QStringLiteral("background: transparent;"));
        closeBtn = new AntButton();
        closeBtn->setButtonType(Ant::ButtonType::Text);
        closeBtn->setButtonSize(Ant::Size::Small);
        closeBtn->setButtonIconType(Ant::IconType::Close);
        closeBtn->setFixedSize(24, 24);
        titleRow->addWidget(titleLabel);
        titleRow->addStretch();
        titleRow->addWidget(closeBtn);
        lay->addLayout(titleRow);

        descLabel = new QLabel(tooltip);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet(QStringLiteral("background: transparent;"));
        lay->addWidget(descLabel);

        auto* btnRow = new QHBoxLayout();
        btnRow->setContentsMargins(0, 0, 0, 0);
        btnRow->setSpacing(8);
        prevBtn = new AntButton(QStringLiteral("Prev"));
        prevBtn->setButtonSize(Ant::Size::Small);
        nextBtn = new AntButton(QStringLiteral("Next"));
        nextBtn->setButtonType(Ant::ButtonType::Primary);
        nextBtn->setButtonSize(Ant::Size::Small);
        btnRow->addStretch();
        btnRow->addWidget(prevBtn);
        btnRow->addWidget(nextBtn);
        lay->addLayout(btnRow);

        tooltip->adjustSize();
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
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.fillRect(rect(), QColor(0, 0, 0, 115));

        if (target && target->isVisible())
        {
            QPoint tl = target->mapToGlobal(QPoint(0, 0));
            tl = mapFromGlobal(tl);
            QRect targetR(tl, target->size());
            const QRect spotlight = targetR.adjusted(-8, -8, 8, 8);
            const int radius = token.borderRadius;

            // Clear the mask over the target
            p.setCompositionMode(QPainter::CompositionMode_Clear);
            QPainterPath clearPath;
            clearPath.addRoundedRect(QRectF(spotlight), radius, radius);
            p.fillPath(clearPath, Qt::black);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);

            // Draw highlight border
            p.setPen(QPen(token.colorBgContainer, 2));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(QRectF(spotlight).adjusted(1, 1, -1, -1), radius, radius);
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
    const auto& token = antTheme->tokens();
    overlay->titleLabel->setText(step.title);
    overlay->descLabel->setText(step.description);
    QPalette titlePalette = overlay->titleLabel->palette();
    titlePalette.setColor(QPalette::WindowText, token.colorText);
    overlay->titleLabel->setPalette(titlePalette);
    QPalette descPalette = overlay->descLabel->palette();
    descPalette.setColor(QPalette::WindowText, token.colorText);
    overlay->descLabel->setPalette(descPalette);
    overlay->setTarget(step.target);

    // Position tooltip near target
    if (step.target)
    {
        QPoint tp = step.target->mapToGlobal(QPoint(0, 0));
        tp = m_overlay->mapFromGlobal(tp);
        QRect targetR(tp, step.target->size());
        overlay->tooltip->adjustSize();
        int tx = targetR.center().x() - overlay->tooltip->width() / 2;
        int ty = targetR.bottom() + 20;
        if (tx < 16) tx = 16;
        if (tx + overlay->tooltip->width() > m_overlay->width() - 16)
        {
            tx = m_overlay->width() - overlay->tooltip->width() - 16;
        }
        if (ty + overlay->tooltip->height() > m_overlay->height() - 16)
        {
            ty = targetR.top() - overlay->tooltip->height() - 20;
        }
        overlay->tooltip->move(tx, ty);
    }

    bool isLast = (index == m_steps.size() - 1);
    overlay->nextBtn->setText(isLast ? QStringLiteral("Finish") : QStringLiteral("Next"));
    overlay->prevBtn->setVisible(index > 0);
    overlay->update();

    Q_EMIT stepChanged(m_current);
}
