#include "Pages.h"

#include <QFrame>
#include <QFont>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QList>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPair>
#include <QSize>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntAvatar.h"
#include "widgets/AntBadge.h"
#include "widgets/AntCalendar.h"
#include "widgets/AntCard.h"
#include "widgets/AntCarousel.h"
#include "widgets/AntCollapse.h"
#include "widgets/AntDescriptions.h"
#include "widgets/AntEmpty.h"
#include "widgets/AntIcon.h"
#include "widgets/AntImage.h"
#include "widgets/AntList.h"
#include "widgets/AntTag.h"
#include "widgets/AntTypography.h"

namespace
{
class CardCoverPreview : public QWidget
{
public:
    explicit CardCoverPreview(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setFixedSize(300, 160);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath coverPath;
        const qreal radius = 8.0;
        const QRectF r = rect();
        coverPath.moveTo(r.left(), r.bottom());
        coverPath.lineTo(r.left(), r.top() + radius);
        coverPath.quadTo(r.left(), r.top(), r.left() + radius, r.top());
        coverPath.lineTo(r.right() - radius, r.top());
        coverPath.quadTo(r.right(), r.top(), r.right(), r.top() + radius);
        coverPath.lineTo(r.right(), r.bottom());
        coverPath.closeSubpath();

        QLinearGradient gradient(r.topLeft(), r.bottomRight());
        gradient.setColorAt(0.0, QColor(QStringLiteral("#1e3cb4")));
        gradient.setColorAt(0.35, QColor(QStringLiteral("#643cc8")));
        gradient.setColorAt(0.7, QColor(QStringLiteral("#c850a0")));
        gradient.setColorAt(1.0, QColor(QStringLiteral("#f07860")));
        painter.fillPath(coverPath, gradient);

        QColor circle(QStringLiteral("#ffffff"));
        circle.setAlphaF(0.9);
        painter.setPen(Qt::NoPen);
        painter.setBrush(circle);
        painter.drawEllipse(QRectF(width() / 2.0 - 30.0, height() / 2.0 - 30.0, 60.0, 60.0));
    }
};

class CarouselSlidePreview : public QWidget
{
public:
    CarouselSlidePreview(const QString& text, const QColor& from, const QColor& to, QWidget* parent = nullptr)
        : QWidget(parent)
        , m_text(text)
        , m_from(from)
        , m_to(to)
    {
        setMinimumHeight(160);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QRectF r = rect();
        QPainterPath path;
        path.addRoundedRect(r, 8.0, 8.0);

        QLinearGradient gradient(r.topLeft(), r.bottomRight());
        gradient.setColorAt(0.0, m_from);
        gradient.setColorAt(1.0, m_to);
        painter.fillPath(path, gradient);

        QFont labelFont = painter.font();
        labelFont.setPixelSize(24);
        labelFont.setWeight(QFont::Medium);
        painter.setFont(labelFont);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, m_text);
    }

private:
    QString m_text;
    QColor m_from;
    QColor m_to;
};

AntIcon* makeCardActionIcon(Ant::IconType type)
{
    auto* icon = new AntIcon(type);
    icon->setIconSize(16);
    icon->setColor(antTheme->tokens().colorTextTertiary);
    return icon;
}

AntIcon* makeEllipsisActionIcon()
{
    auto* icon = new AntIcon();
    icon->setIconSize(16);
    icon->setIconTheme(Ant::IconTheme::Filled);
    icon->setColor(antTheme->tokens().colorTextTertiary);
    QPainterPath dots;
    dots.addEllipse(QRectF(7, 14, 4, 4));
    dots.addEllipse(QRectF(14, 14, 4, 4));
    dots.addEllipse(QRectF(21, 14, 4, 4));
    icon->setCustomPath(dots);
    return icon;
}

QWidget* makeCollapseContent(const QString& text, QWidget* parent)
{
    auto* content = new QWidget(parent);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(example::pages::makeText(text, content));
    return content;
}
}

namespace example::pages
{
QWidget* createAvatarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(16);
        for (const int size : {64, 40, 32, 24})
        {
            auto* avatar = new AntAvatar();
            avatar->setIconText(QStringLiteral("user"));
            avatar->setCustomSize(size);
            basicRow->addWidget(avatar);
        }
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Type"));
        auto* cl = card->bodyLayout();
        auto* typeRow = new QHBoxLayout();
        typeRow->setSpacing(16);
        auto* user = new AntAvatar(QStringLiteral("U"));
        user->setBackgroundColor(QColor(QStringLiteral("#1677ff")));
        typeRow->addWidget(user);
        auto* green = new AntAvatar();
        green->setIconText(QStringLiteral("user"));
        green->setBackgroundColor(QColor(QStringLiteral("#87d068")));
        typeRow->addWidget(green);
        auto* image = new AntAvatar();
        image->setImagePath(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
        typeRow->addWidget(image);
        auto* orange = new AntAvatar(QStringLiteral("K"));
        orange->setBackgroundColor(QColor(QStringLiteral("#f56a00")));
        typeRow->addWidget(orange);
        typeRow->addStretch();
        cl->addLayout(typeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Group"));
        auto* cl = card->bodyLayout();
        auto* groupRow = new QHBoxLayout();
        groupRow->setSpacing(28);

        auto* group1 = new AntAvatarGroup(page);
        group1->setMaxCount(3);
        auto* avatarA = new AntAvatar(QStringLiteral("A"));
        avatarA->setBackgroundColor(QColor(QStringLiteral("#1677ff")));
        group1->addAvatar(avatarA);
        auto* avatarB = new AntAvatar(QStringLiteral("B"));
        avatarB->setBackgroundColor(QColor(QStringLiteral("#87d068")));
        group1->addAvatar(avatarB);
        auto* avatarC = new AntAvatar(QStringLiteral("C"));
        avatarC->setBackgroundColor(QColor(QStringLiteral("#f56a00")));
        group1->addAvatar(avatarC);
        group1->addAvatar(new AntAvatar(QStringLiteral("D")));
        group1->addAvatar(new AntAvatar(QStringLiteral("E")));
        groupRow->addWidget(group1);
        groupRow->addStretch();
        cl->addLayout(groupRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createBadgePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto makeAvatar = [](const QString& iconText, Ant::AvatarShape shape = Ant::AvatarShape::Square) {
        auto* avatar = new AntAvatar();
        avatar->setShape(shape);
        avatar->setIconText(iconText);
        return avatar;
    };

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basicRow = new QHBoxLayout();
        basicRow->setSpacing(8);

        auto* count12 = new AntBadge(12);
        count12->setContentWidget(makeAvatar(QStringLiteral("user")));
        basicRow->addWidget(count12);

        auto* count39 = new AntBadge(39);
        count39->setContentWidget(makeAvatar(QStringLiteral("bell")));
        basicRow->addWidget(count39);

        auto* dot = new AntBadge();
        dot->setDot(true);
        dot->setContentWidget(makeAvatar(QStringLiteral("user"), Ant::AvatarShape::Circle));
        basicRow->addWidget(dot);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Status"));
        auto* cl = card->bodyLayout();
        auto* statusRow = new QHBoxLayout();
        statusRow->setSpacing(14);
        const QList<QPair<QString, Ant::BadgeStatus>> statuses = {
            {QString(), Ant::BadgeStatus::Success},
            {QString(), Ant::BadgeStatus::Error},
            {QString(), Ant::BadgeStatus::Default},
            {QString(), Ant::BadgeStatus::Processing},
            {QString(), Ant::BadgeStatus::Warning},
        };
        for (const auto& item : statuses)
        {
            auto* badge = new AntBadge();
            badge->setStatus(item.second);
            badge->setText(item.first);
            statusRow->addWidget(badge);
        }
        statusRow->addStretch();
        cl->addLayout(statusRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createCalendarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();

        auto* cal = new AntCalendar(page);
        cl->addWidget(cal);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createCardPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();

        auto* basic = new AntCard(QStringLiteral("Card Title"), page);
        basic->setExtra(QStringLiteral("More"));
        basic->setFixedSize(340, 140);
        basic->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card content"), basic));
        cl->addWidget(basic, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Cover & Meta"));
        auto* cl = card->bodyLayout();

        auto* meta = new AntCard(page);
        meta->setFixedWidth(300);
        meta->setHoverable(true);
        meta->setCoverWidget(new CardCoverPreview(meta));
        meta->setMetaTitle(QStringLiteral("ant design"));
        meta->setMetaDescription(QStringLiteral("description information"));
        meta->bodyWidget()->hide();
        meta->addActionWidget(makeCardActionIcon(Ant::IconType::Edit));
        meta->addActionWidget(makeCardActionIcon(Ant::IconType::Delete));
        meta->addActionWidget(makeEllipsisActionIcon());
        cl->addWidget(meta, 0, Qt::AlignLeft);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createCarouselPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();

        auto* carousel = new AntCarousel(page);
        carousel->setFixedHeight(160);
        carousel->addSlide(new CarouselSlidePreview(QStringLiteral("Slide 1"), QColor(QStringLiteral("#1677ff")), QColor(QStringLiteral("#36cfc9")), carousel));
        carousel->addSlide(new CarouselSlidePreview(QStringLiteral("Slide 2"), QColor(QStringLiteral("#722ed1")), QColor(QStringLiteral("#eb2f96")), carousel));
        carousel->addSlide(new CarouselSlidePreview(QStringLiteral("Slide 3"), QColor(QStringLiteral("#fa8c16")), QColor(QStringLiteral("#fadb14")), carousel));

        cl->addWidget(carousel);
        cl->addStretch();
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createCollapsePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();

        auto* collapse = new AntCollapse(page);

        auto* p1 = collapse->addPanel(QStringLiteral("Header 1"));
        p1->setContentWidget(makeCollapseContent(QStringLiteral("Content 1"), collapse));
        p1->setExpanded(true);

        auto* p2 = collapse->addPanel(QStringLiteral("Header 2"));
        p2->setContentWidget(makeCollapseContent(QStringLiteral("Content 2"), collapse));

        auto* p3 = collapse->addPanel(QStringLiteral("Header 3"));
        p3->setContentWidget(makeCollapseContent(QStringLiteral("Content 3"), collapse));

        cl->addWidget(collapse);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Accordion"));
        auto* cl = card->bodyLayout();

        auto* collapse = new AntCollapse(page);
        collapse->setAccordion(true);

        auto* p1 = collapse->addPanel(QStringLiteral("Panel 1"));
        p1->setContentWidget(makeCollapseContent(QStringLiteral("Content 1"), collapse));
        p1->setExpanded(true);

        auto* p2 = collapse->addPanel(QStringLiteral("Panel 2"));
        p2->setContentWidget(makeCollapseContent(QStringLiteral("Content 2"), collapse));

        cl->addWidget(collapse);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createDescriptionsPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntDescriptions(page);
        basic->setBordered(true);
        basic->setColumnCount(2);
        basic->addItem(QStringLiteral("Name"), QStringLiteral("Cloud Database"));

        auto* statusItem = new AntDescriptionsItem(QStringLiteral("Status"), QString(), basic);
        auto* status = new AntBadge(statusItem);
        status->setStatus(Ant::BadgeStatus::Processing);
        status->setText(QStringLiteral("Running"));
        statusItem->setContentWidget(status);
        basic->addItem(statusItem);

        basic->addItem(QStringLiteral("Type"), QStringLiteral("PostgreSQL"));
        basic->addItem(QStringLiteral("IP"), QStringLiteral("192.168.1.1"));
        cl->addWidget(basic);
        cl->addStretch();
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createEmptyPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basic = new AntEmpty(page);
        cl->addWidget(basic, 0, Qt::AlignHCenter | Qt::AlignTop);
        cl->addStretch();
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Custom Description"));
        auto* cl = card->bodyLayout();
        auto* custom = new AntEmpty(page);
        custom->setDescription(QStringLiteral("No Data Available"));
        cl->addWidget(custom, 0, Qt::AlignHCenter | Qt::AlignTop);
        cl->addStretch();
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createImagePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();

        auto* img = new AntImage(page);
        img->setSrc(QStringLiteral(":/qt-ant-design/images/image-basic.png"));
        img->setImgWidth(200);
        cl->addWidget(img, 0, Qt::AlignLeft | Qt::AlignTop);
        cl->addStretch();

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createListPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* basicList = new AntList(page);
        basicList->setBordered(true);

        for (int i = 1; i <= 3; ++i)
        {
            auto* item = new AntListItem(basicList);
            auto* text = new AntTypography(QStringLiteral("Item %1").arg(i), item);
            item->setContentWidget(text);
            basicList->addItem(item);
        }

        cl->addWidget(basicList);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Actions"));
        auto* cl = card->bodyLayout();
        auto* actionList = new AntList(page);
        actionList->setBordered(true);

        for (int i = 1; i <= 2; ++i)
        {
            auto* item = new AntListItem(actionList);
            auto* meta = new AntListItemMeta(item);
            meta->setTitle(QStringLiteral("Ant Design Title %1").arg(i));
            meta->setDescription(QStringLiteral("Description %1").arg(i));
            item->setMeta(meta);

            auto* edit = new AntTypography(QStringLiteral("Edit"), item);
            edit->setType(Ant::TypographyType::Link);
            edit->setHref(QStringLiteral("#"));
            item->addActionWidget(edit);
            auto* more = new AntTypography(QStringLiteral("More"), item);
            more->setType(Ant::TypographyType::Link);
            more->setHref(QStringLiteral("#"));
            item->addActionWidget(more);
            actionList->addItem(item);
        }

        cl->addWidget(actionList);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
