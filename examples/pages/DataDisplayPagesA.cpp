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
#include <QPoint>
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
#include "widgets/AntButton.h"
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
        basicRow->setSpacing(12);
        basicRow->addWidget(new AntAvatar(QStringLiteral("U")));
        auto* icon = new AntAvatar();
        icon->setIconText(QStringLiteral("@"));
        basicRow->addWidget(icon);
        auto* square = new AntAvatar(QStringLiteral("AD"));
        square->setShape(Ant::AvatarShape::Square);
        basicRow->addWidget(square);
        auto* custom = new AntAvatar(QStringLiteral("Qt"));
        custom->setCustomSize(48);
        basicRow->addWidget(custom);
        basicRow->addStretch();
        cl->addLayout(basicRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Size"));
        auto* cl = card->bodyLayout();
        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(12);
        auto* large = new AntAvatar(QStringLiteral("L"));
        large->setAvatarSize(Ant::Size::Large);
        auto* middle = new AntAvatar(QStringLiteral("M"));
        middle->setAvatarSize(Ant::Size::Middle);
        auto* small = new AntAvatar(QStringLiteral("S"));
        small->setAvatarSize(Ant::Size::Small);
        sizeRow->addWidget(large);
        sizeRow->addWidget(middle);
        sizeRow->addWidget(small);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Autoset Font Size"));
        auto* cl = card->bodyLayout();
        auto* textRow = new QHBoxLayout();
        textRow->setSpacing(12);
        textRow->addWidget(new AntAvatar(QStringLiteral("USER")));
        auto* longName = new AntAvatar(QStringLiteral("DESIGN"));
        longName->setCustomSize(56);
        longName->setGap(6);
        textRow->addWidget(longName);
        auto* tinyGap = new AntAvatar(QStringLiteral("ANTD"));
        tinyGap->setGap(2);
        textRow->addWidget(tinyGap);
        textRow->addStretch();
        cl->addLayout(textRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Badge"));
        auto* cl = card->bodyLayout();
        auto* badgeRow = new QHBoxLayout();
        badgeRow->setSpacing(28);
        auto* countBadge = new AntBadge(3);
        countBadge->setContentWidget(new AntAvatar(QStringLiteral("A")));
        badgeRow->addWidget(countBadge);
        auto* dotBadge = new AntBadge();
        dotBadge->setDot(true);
        auto* dotAvatar = new AntAvatar(QStringLiteral("B"));
        dotAvatar->setShape(Ant::AvatarShape::Square);
        dotBadge->setContentWidget(dotAvatar);
        badgeRow->addWidget(dotBadge);
        badgeRow->addStretch();
        cl->addLayout(badgeRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Group"));
        auto* cl = card->bodyLayout();
        auto* groupRow = new QHBoxLayout();
        groupRow->setSpacing(28);

        auto* group1 = new AntAvatarGroup(page);
        group1->setMaxCount(3);
        group1->addAvatar(new AntAvatar(QStringLiteral("A")));
        group1->addAvatar(new AntAvatar(QStringLiteral("B")));
        group1->addAvatar(new AntAvatar(QStringLiteral("C")));
        group1->addAvatar(new AntAvatar(QStringLiteral("D")));
        group1->addAvatar(new AntAvatar(QStringLiteral("E")));
        groupRow->addWidget(group1);

        auto* group2 = new AntAvatarGroup(page);
        group2->setMaxCount(4);
        group2->setAvatarSize(Ant::Size::Small);
        for (const QString& name : {QStringLiteral("X"), QStringLiteral("Y"), QStringLiteral("Z"), QStringLiteral("W"), QStringLiteral("V"), QStringLiteral("U")})
        {
            auto* av = new AntAvatar(name);
            av->setShape(Ant::AvatarShape::Square);
            group2->addAvatar(av);
        }
        groupRow->addWidget(group2);
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

    auto makeAnchor = [](const QString& text) {
        auto* button = new AntButton(text);
        button->setButtonType(Ant::ButtonType::Default);
        button->setFixedSize(86, 40);
        return button;
    };

    {
        auto* card = new AntCard(QStringLiteral("Count"));
        auto* cl = card->bodyLayout();
        auto* countRow = new QHBoxLayout();
        countRow->setSpacing(28);

        auto* basic = new AntBadge(5);
        basic->setContentWidget(makeAnchor(QStringLiteral("Inbox")));
        countRow->addWidget(basic);

        auto* zero = new AntBadge(0);
        zero->setShowZero(true);
        zero->setContentWidget(makeAnchor(QStringLiteral("Zero")));
        countRow->addWidget(zero);

        auto* overflow = new AntBadge(120);
        overflow->setOverflowCount(99);
        overflow->setContentWidget(makeAnchor(QStringLiteral("Tasks")));
        countRow->addWidget(overflow);
        countRow->addStretch();
        cl->addLayout(countRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Dot and Size"));
        auto* cl = card->bodyLayout();
        auto* dotRow = new QHBoxLayout();
        dotRow->setSpacing(28);

        auto* dot = new AntBadge();
        dot->setDot(true);
        dot->setContentWidget(makeAnchor(QStringLiteral("Notice")));
        dotRow->addWidget(dot);

        auto* small = new AntBadge(8);
        small->setBadgeSize(Ant::Size::Small);
        small->setContentWidget(makeAnchor(QStringLiteral("Small")));
        dotRow->addWidget(small);

        auto* color = new AntBadge(3);
        color->setColor(QStringLiteral("success"));
        color->setOffset(QPoint(-8, 6));
        color->setContentWidget(makeAnchor(QStringLiteral("Offset")));
        dotRow->addWidget(color);
        dotRow->addStretch();
        cl->addLayout(dotRow);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Status"));
        auto* cl = card->bodyLayout();
        auto* statusCol = new QVBoxLayout();
        statusCol->setSpacing(10);
        const QList<QPair<QString, Ant::BadgeStatus>> statuses = {
            {QStringLiteral("Success"), Ant::BadgeStatus::Success},
            {QStringLiteral("Processing"), Ant::BadgeStatus::Processing},
            {QStringLiteral("Default"), Ant::BadgeStatus::Default},
            {QStringLiteral("Error"), Ant::BadgeStatus::Error},
            {QStringLiteral("Warning"), Ant::BadgeStatus::Warning},
        };
        for (const auto& item : statuses)
        {
            auto* badge = new AntBadge();
            badge->setStatus(item.second);
            badge->setText(item.first);
            statusCol->addWidget(badge);
        }
        cl->addLayout(statusCol);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Ribbon"));
        auto* cl = card->bodyLayout();
        auto* ribbonRow = new QHBoxLayout();
        ribbonRow->setSpacing(28);

        auto* ribbonCard1 = new AntCard(QStringLiteral("Pushes Open"), page);
        ribbonCard1->setFixedWidth(280);
        ribbonCard1->setMinimumHeight(120);
        ribbonCard1->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card with a default ribbon badge in the corner.")));
        auto* ribbon1 = new AntBadge();
        ribbon1->setBadgeMode(Ant::BadgeMode::Ribbon);
        ribbon1->setRibbonText(QStringLiteral("Ribbon"));
        ribbon1->setContentWidget(ribbonCard1);
        ribbonRow->addWidget(ribbon1);

        auto* ribbonCard2 = new AntCard(QStringLiteral("Pushes Open"), page);
        ribbonCard2->setFixedWidth(280);
        ribbonCard2->setMinimumHeight(120);
        ribbonCard2->bodyLayout()->addWidget(new AntTypography(QStringLiteral("Card with a colored ribbon badge.")));
        auto* ribbon2 = new AntBadge();
        ribbon2->setBadgeMode(Ant::BadgeMode::Ribbon);
        ribbon2->setRibbonText(QStringLiteral("Success"));
        ribbon2->setRibbonColor(QStringLiteral("success"));
        ribbon2->setContentWidget(ribbonCard2);
        ribbonRow->addWidget(ribbon2);
        ribbonRow->addStretch();
        cl->addLayout(ribbonRow);
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
        auto* card = new AntCard(QStringLiteral("AntCalendar"));
        auto* cl = card->bodyLayout();

        auto* cal = new AntCalendar(page);
        cl->addWidget(cal);

        auto* dateLabel = makeText(QStringLiteral("Selected: ") + cal->selectedDate().toString(QStringLiteral("yyyy-MM-dd")), page);
        QObject::connect(cal, &AntCalendar::clicked, dateLabel, [dateLabel, cal]() {
            dateLabel->setText(QStringLiteral("Selected: ") + cal->selectedDate().toString(QStringLiteral("yyyy-MM-dd")));
        });
        cl->addWidget(dateLabel);

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
        auto* card = new AntCard(QStringLiteral("AntImage"));
        auto* cl = card->bodyLayout();

        auto* img = new AntImage(page);
        img->setAlt(QStringLiteral("No image loaded"));
        img->setImgWidth(200);
        img->setImgHeight(200);
        cl->addWidget(img);

        auto* tip = makeSecondaryText(QStringLiteral("Set src to load an image. Click to preview."), page);
        cl->addWidget(tip);

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
        basicList->setFixedWidth(480);

        for (int i = 1; i <= 4; ++i)
        {
            auto* item = new AntListItem(basicList);
            auto* meta = new AntListItemMeta(item);
            meta->setTitle(QStringLiteral("Ant Design List Item %1").arg(i));
            meta->setDescription(QStringLiteral("Description for item %1").arg(i));
            item->setMeta(meta);
            basicList->addItem(item);
        }

        cl->addWidget(basicList);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Header and Footer"));
        auto* cl = card->bodyLayout();
        auto* headerList = new AntList(page);
        headerList->setBordered(true);
        headerList->setFixedWidth(480);

        auto* header = new AntTypography(QStringLiteral("Header"), headerList);
        header->setTitle(true);
        header->setTitleLevel(Ant::TypographyTitleLevel::H5);
        header->setFixedHeight(40);
        headerList->setHeaderWidget(header);

        auto* footer = new AntTypography(QStringLiteral("Footer"), headerList);
        footer->setType(Ant::TypographyType::Secondary);
        footer->setFixedHeight(40);
        headerList->setFooterWidget(footer);

        for (int i = 1; i <= 3; ++i)
        {
            auto* item = new AntListItem(headerList);
            auto* meta = new AntListItemMeta(item);
            meta->setTitle(QStringLiteral("Item %1").arg(i));
            item->setMeta(meta);
            headerList->addItem(item);
        }

        cl->addWidget(headerList);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Split and Size"));
        auto* cl = card->bodyLayout();
        auto* splitRow = new QHBoxLayout();
        splitRow->setSpacing(28);

        auto* noSplit = new AntList(page);
        noSplit->setBordered(true);
        noSplit->setSplit(false);
        noSplit->setFixedWidth(240);
        for (int i = 1; i <= 3; ++i)
        {
            auto* item = new AntListItem(noSplit);
            auto* meta = new AntListItemMeta(item);
            meta->setTitle(QStringLiteral("No Split %1").arg(i));
            item->setMeta(meta);
            noSplit->addItem(item);
        }

        auto* smallList = new AntList(page);
        smallList->setBordered(true);
        smallList->setListSize(AntList::Small);
        smallList->setFixedWidth(240);
        for (int i = 1; i <= 3; ++i)
        {
            auto* item = new AntListItem(smallList);
            auto* meta = new AntListItemMeta(item);
            meta->setTitle(QStringLiteral("Small Item %1").arg(i));
            item->setMeta(meta);
            smallList->addItem(item);
        }

        auto* largeList = new AntList(page);
        largeList->setBordered(true);
        largeList->setListSize(AntList::Large);
        largeList->setFixedWidth(240);
        for (int i = 1; i <= 3; ++i)
        {
            auto* item = new AntListItem(largeList);
            auto* meta = new AntListItemMeta(item);
            meta->setTitle(QStringLiteral("Large Item %1").arg(i));
            item->setMeta(meta);
            largeList->addItem(item);
        }

        splitRow->addWidget(noSplit);
        splitRow->addWidget(smallList);
        splitRow->addWidget(largeList);
        splitRow->addStretch();
        cl->addLayout(splitRow);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
