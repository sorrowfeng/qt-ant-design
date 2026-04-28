#include "Pages.h"

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QList>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPair>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntAlert.h"
#include "widgets/AntButton.h"
#include "widgets/AntCard.h"
#include "widgets/AntDrawer.h"
#include "widgets/AntInput.h"
#include "widgets/AntMessage.h"
#include "widgets/AntModal.h"
#include "widgets/AntNotification.h"
#include "widgets/AntPopconfirm.h"
#include "widgets/AntProgress.h"
#include "widgets/AntResult.h"
#include "widgets/AntSelect.h"
#include "widgets/AntSkeleton.h"
#include "widgets/AntSpin.h"
#include "widgets/AntTour.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWatermark.h"

namespace
{
class RoundedDemoSurface : public QWidget
{
public:
    explicit RoundedDemoSurface(const QString& text,
                                const QColor& fill,
                                const QColor& border,
                                QWidget* parent = nullptr)
        : QWidget(parent)
        , m_text(text)
        , m_fill(fill)
        , m_border(border)
    {
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        const QRectF r = rect().adjusted(0.5, 0.5, -0.5, -0.5);
        QPainterPath path;
        path.addRoundedRect(r, 8, 8);

        if (m_fill.isValid())
        {
            painter.fillPath(path, m_fill);
        }
        if (m_border.isValid())
        {
            painter.setPen(QPen(m_border, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(path);
        }

        painter.setPen(antTheme->tokens().colorTextSecondary);
        painter.drawText(rect(), Qt::AlignCenter, m_text);
    }

private:
    QString m_text;
    QColor m_fill;
    QColor m_border;
};

class SpinContentSurface : public RoundedDemoSurface
{
public:
    explicit SpinContentSurface(QWidget* parent = nullptr)
        : RoundedDemoSurface(QStringLiteral("Content"), QColor(QStringLiteral("#f5f5f5")), QColor(), parent)
        , m_spinner(new AntSpin(this))
    {
        setFixedHeight(82);
        m_spinner->setDescription(QStringLiteral("Loading..."));
        m_spinner->resize(120, 64);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        const QRectF r = rect().adjusted(0.5, 0.5, -0.5, -0.5);
        QPainterPath path;
        path.addRoundedRect(r, 8, 8);
        painter.fillPath(path, QColor(QStringLiteral("#f5f5f5")));

        painter.setPen(antTheme->tokens().colorText);
        painter.drawText(rect().adjusted(30, 30, -30, -30), Qt::AlignLeft | Qt::AlignTop, QStringLiteral("Content"));
    }

    void resizeEvent(QResizeEvent* event) override
    {
        RoundedDemoSurface::resizeEvent(event);
        m_spinner->move((width() - m_spinner->width()) / 2, (height() - m_spinner->height()) / 2);
    }

private:
    AntSpin* m_spinner = nullptr;
};
} // namespace

namespace example::pages
{
QWidget* createProgressPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Line"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* p80 = new AntProgress();
        p80->setPercent(80);
        auto* active = new AntProgress();
        active->setPercent(60);
        active->setStatus(Ant::ProgressStatus::Active);
        auto* success = new AntProgress();
        success->setPercent(100);
        auto* exception = new AntProgress();
        exception->setPercent(50);
        exception->setStatus(Ant::ProgressStatus::Exception);

        cl->addWidget(p80);
        cl->addWidget(active);
        cl->addWidget(success);
        cl->addWidget(exception);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Circle"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* circleRow = new QHBoxLayout();
        circleRow->setSpacing(24);
        auto* circle = new AntProgress();
        circle->setProgressType(Ant::ProgressType::Circle);
        circle->setPercent(75);
        auto* circleSuccess = new AntProgress();
        circleSuccess->setProgressType(Ant::ProgressType::Circle);
        circleSuccess->setPercent(100);
        auto* circleException = new AntProgress();
        circleException->setProgressType(Ant::ProgressType::Circle);
        circleException->setPercent(50);
        circleException->setStatus(Ant::ProgressStatus::Exception);
        circleRow->addWidget(circle);
        circleRow->addWidget(circleSuccess);
        circleRow->addWidget(circleException);
        circleRow->addStretch();
        cl->addLayout(circleRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createResultPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Success"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* success = new AntResult(page);
        success->setStatus(Ant::AlertType::Success);
        success->setTitle(QStringLiteral("Successfully"));
        success->setSubTitle(QStringLiteral("Operation completed."));
        auto* successBtn = new AntButton(QStringLiteral("Go Home"), success);
        successBtn->setButtonType(Ant::ButtonType::Primary);
        success->setExtraWidget(successBtn);

        cl->addWidget(success);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Error"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* error = new AntResult(page);
        error->setStatus(Ant::AlertType::Error);
        error->setTitle(QStringLiteral("Submission Failed"));
        error->setSubTitle(QStringLiteral("Please check and modify."));

        cl->addWidget(error);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSkeletonPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* active = new AntSkeleton(page);
        active->setActive(true);

        cl->addWidget(active);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Avatar & Title"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* article = new AntSkeleton(page);
        article->setActive(true);
        article->setAvatarVisible(true);
        article->setAvatarShape(Ant::AvatarShape::Circle);
        article->setParagraphRows(2);

        cl->addWidget(article);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSpinPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(12);
        auto* middle = new AntSpin();
        auto* large = new AntSpin();
        large->setSpinSize(Ant::Size::Large);
        sizeRow->addWidget(middle);
        sizeRow->addWidget(large);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Content"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* surface = new SpinContentSurface(page);
        cl->addWidget(surface);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createTourPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* desc = makeSecondaryText(QStringLiteral("Tour component requires step-by-step interaction. Click the buttons below to see the demo."), page);
        auto* stepRow = new QHBoxLayout();
        stepRow->setSpacing(12);
        stepRow->addWidget(new AntButton(QStringLiteral("Step 1")));
        stepRow->addWidget(new AntButton(QStringLiteral("Step 2")));
        stepRow->addWidget(new AntButton(QStringLiteral("Step 3")));
        stepRow->addStretch();

        cl->addWidget(desc);
        cl->addLayout(stepRow);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createWatermarkPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        cl->setAlignment(Qt::AlignTop);

        auto* wm1 = new AntWatermark();
        wm1->setContent(QStringLiteral("Ant Design"));
        wm1->setFixedHeight(200);
        auto* wm1Layout = new QVBoxLayout(wm1);
        wm1Layout->setContentsMargins(0, 0, 0, 0);
        auto* content = new RoundedDemoSurface(QStringLiteral("Watermark content"),
                                               QColor(),
                                               QColor(QStringLiteral("#f0f0f0")),
                                               wm1);
        wm1Layout->addWidget(content);
        cl->addWidget(wm1);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
