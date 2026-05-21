#include "AntTour.h"

#include <QApplication>
#include <QDialog>
#include <QEvent>
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
    QRect targetRect;
    QRect spotlightRect;
    QString cachedTitle;
    QString cachedDescription;
    QString cachedNextText;
    bool cachedPrevVisible = false;
    int targetGeometryApplyCount = 0;
    int targetGeometrySkipCount = 0;
    int tooltipGeometryApplyCount = 0;
    int tooltipGeometrySkipCount = 0;
    int contentApplyCount = 0;
    int contentSkipCount = 0;
    int dirtyUpdateCount = 0;
    QString lastUpdateMode;

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

    ~TourOverlay() override
    {
        if (target)
        {
            target->removeEventFilter(this);
        }
    }

    QRect setTarget(QWidget* t)
    {
        if (target == t)
        {
            return updateTargetGeometry();
        }
        if (target)
        {
            target->removeEventFilter(this);
        }
        target = t;
        if (target)
        {
            target->installEventFilter(this);
        }
        return updateTargetGeometry();
    }

    QRect updateTargetGeometry()
    {
        QRect nextTargetRect;
        QRect nextSpotlightRect;
        if (target && target->isVisible())
        {
            QPoint tl = target->mapToGlobal(QPoint(0, 0));
            tl = mapFromGlobal(tl);
            nextTargetRect = QRect(tl, target->size());
            nextSpotlightRect = nextTargetRect.adjusted(-8, -8, 8, 8);
        }

        if (nextTargetRect == targetRect && nextSpotlightRect == spotlightRect)
        {
            ++targetGeometrySkipCount;
            lastUpdateMode = QStringLiteral("targetSkip");
            return QRect();
        }

        const QRect dirty = spotlightRect.united(nextSpotlightRect).adjusted(-6, -6, 6, 6).intersected(rect());
        targetRect = nextTargetRect;
        spotlightRect = nextSpotlightRect;
        ++targetGeometryApplyCount;
        ++dirtyUpdateCount;
        lastUpdateMode = QStringLiteral("target");
        if (!dirty.isEmpty())
        {
            update(dirty);
        }
        else
        {
            update();
        }
        return dirty;
    }

    bool syncContent(const AntTourStep& step, bool isLast, bool prevVisible)
    {
        const QString nextText = isLast ? QStringLiteral("Finish") : QStringLiteral("Next");
        if (cachedTitle == step.title &&
            cachedDescription == step.description &&
            cachedNextText == nextText &&
            cachedPrevVisible == prevVisible)
        {
            ++contentSkipCount;
            lastUpdateMode = QStringLiteral("contentSkip");
            return false;
        }

        cachedTitle = step.title;
        cachedDescription = step.description;
        cachedNextText = nextText;
        cachedPrevVisible = prevVisible;
        titleLabel->setText(step.title);
        descLabel->setText(step.description);
        nextBtn->setText(nextText);
        prevBtn->setVisible(prevVisible);
        ++contentApplyCount;
        lastUpdateMode = QStringLiteral("content");
        return true;
    }

    bool syncTooltipGeometry(const QRect& geometry)
    {
        if (tooltip->geometry() == geometry)
        {
            ++tooltipGeometrySkipCount;
            lastUpdateMode = QStringLiteral("tooltipSkip");
            return false;
        }
        tooltip->setGeometry(geometry);
        ++tooltipGeometryApplyCount;
        lastUpdateMode = QStringLiteral("tooltip");
        return true;
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched == target)
        {
            switch (event->type())
            {
            case QEvent::Move:
            case QEvent::Resize:
            case QEvent::Show:
            case QEvent::Hide:
                updateTargetGeometry();
                break;
            case QEvent::Destroy:
                target = nullptr;
                updateTargetGeometry();
                break;
            default:
                break;
            }
        }
        return QDialog::eventFilter(watched, event);
    }

    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.fillRect(rect(), QColor(0, 0, 0, 115));

        if (target && target->isVisible() && !spotlightRect.isEmpty())
        {
            const int radius = token.borderRadius;

            // Clear the mask over the target
            p.setCompositionMode(QPainter::CompositionMode_Clear);
            QPainterPath clearPath;
            clearPath.addRoundedRect(QRectF(spotlightRect), radius, radius);
            p.fillPath(clearPath, Qt::black);
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);

            // Draw highlight border
            p.setPen(QPen(token.colorBgContainer, 2));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(QRectF(spotlightRect).adjusted(1, 1, -1, -1), radius, radius);
        }
    }
};

} // namespace

AntTour::AntTour(QObject* parent) : QObject(parent) {}

void AntTour::addStep(const AntTourStep& step) { m_steps.append(step); }

void AntTour::start(int index)
{
    if (m_steps.isEmpty()) return;
    if (index < 0 || index >= m_steps.size()) return;
    if (m_overlay)
    {
        showStep(index);
        m_overlay->raise();
        return;
    }

    auto* topWidget = QApplication::activeWindow();
    if (!topWidget) { for (auto* w : QApplication::topLevelWidgets()) { if (w->isWindow()) { topWidget = w; break; } } }
    if (!topWidget) return;

    m_overlay = new TourOverlay(topWidget);
    m_overlay->resize(topWidget->size());
    m_overlay->move(topWidget->mapToGlobal(QPoint(0, 0)));

    connect(dynamic_cast<TourOverlay*>(m_overlay)->prevBtn, &QPushButton::clicked, this, &AntTour::prev);
    connect(dynamic_cast<TourOverlay*>(m_overlay)->nextBtn, &QPushButton::clicked, this, &AntTour::next);
    connect(dynamic_cast<TourOverlay*>(m_overlay)->closeBtn, &QPushButton::clicked, this, &AntTour::close);

    showStep(index);
    m_overlay->show();
    syncTourPerfCounters();
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
    syncTourPerfCounters();
    Q_EMIT finished();
}

void AntTour::showStep(int index)
{
    if (!m_overlay || index < 0 || index >= m_steps.size()) return;
    m_current = index;
    const auto& step = m_steps[m_current];

    auto* overlay = dynamic_cast<TourOverlay*>(m_overlay);
    const auto& token = antTheme->tokens();
    bool isLast = (index == m_steps.size() - 1);
    const bool contentChanged = overlay->syncContent(step, isLast, index > 0);
    QPalette titlePalette = overlay->titleLabel->palette();
    titlePalette.setColor(QPalette::WindowText, token.colorText);
    overlay->titleLabel->setPalette(titlePalette);
    QPalette descPalette = overlay->descLabel->palette();
    descPalette.setColor(QPalette::WindowText, token.colorText);
    overlay->descLabel->setPalette(descPalette);
    overlay->setTarget(step.target);

    // Position tooltip near target
    if (step.target && overlay->targetRect.isValid())
    {
        if (contentChanged)
        {
            overlay->tooltip->adjustSize();
        }
        const QRect targetR = overlay->targetRect;
        const QSize tooltipSize = overlay->tooltip->size();
        int tx = targetR.center().x() - tooltipSize.width() / 2;
        int ty = targetR.bottom() + 20;
        if (tx < 16) tx = 16;
        if (tx + tooltipSize.width() > m_overlay->width() - 16)
        {
            tx = m_overlay->width() - tooltipSize.width() - 16;
        }
        if (ty + tooltipSize.height() > m_overlay->height() - 16)
        {
            ty = targetR.top() - tooltipSize.height() - 20;
        }
        overlay->syncTooltipGeometry(QRect(QPoint(tx, ty), tooltipSize));
    }

    syncTourPerfCounters();
    Q_EMIT stepChanged(m_current);
}

void AntTour::syncTourPerfCounters() const
{
    auto* self = const_cast<AntTour*>(this);
    const auto* overlay = dynamic_cast<const TourOverlay*>(m_overlay);
    self->setProperty("antTourOverlayVisible", overlay && overlay->isVisible());
    self->setProperty("antTourCurrentIndex", m_current);
    self->setProperty("antTourTargetGeometryApplyCount", overlay ? overlay->targetGeometryApplyCount : 0);
    self->setProperty("antTourTargetGeometrySkipCount", overlay ? overlay->targetGeometrySkipCount : 0);
    self->setProperty("antTourTooltipGeometryApplyCount", overlay ? overlay->tooltipGeometryApplyCount : 0);
    self->setProperty("antTourTooltipGeometrySkipCount", overlay ? overlay->tooltipGeometrySkipCount : 0);
    self->setProperty("antTourContentApplyCount", overlay ? overlay->contentApplyCount : 0);
    self->setProperty("antTourContentSkipCount", overlay ? overlay->contentSkipCount : 0);
    self->setProperty("antTourDirtyUpdateCount", overlay ? overlay->dirtyUpdateCount : 0);
    self->setProperty("antTourLastUpdateMode", overlay ? overlay->lastUpdateMode : QString());
}
