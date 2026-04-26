#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <QPainter>

#include "core/AntTypes.h"
#include "widgets/AntCard.h"
#include "widgets/AntButton.h"
#include "widgets/AntDivider.h"
#include "widgets/AntFlex.h"
#include "widgets/AntGrid.h"
#include "widgets/AntLayout.h"
#include "widgets/AntSpace.h"
#include "widgets/AntSplitter.h"
#include "widgets/AntTypography.h"

namespace
{
class ColoredLabel : public QFrame
{
public:
    explicit ColoredLabel(const QString& text, const QColor& bg, int h = 40, QWidget* parent = nullptr)
        : QFrame(parent), m_bg(bg), m_h(h)
    {
        setContentsMargins(14, 0, 14, 0);
        setMinimumHeight(h);
        auto* lay = new QHBoxLayout(this);
        lay->setContentsMargins(0, 0, 0, 0);
        auto* lbl = new QLabel(text, this);
        lbl->setAlignment(Qt::AlignCenter);
        lay->addWidget(lbl);
    }
    QSize sizeHint() const override { return {0, m_h}; }
protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(m_bg);
        p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 8, 8);
    }
private:
    QColor m_bg;
    int m_h;
};
}

namespace example::pages
{
QWidget* createDividerPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Horizontal"));
        auto* cl = card->bodyLayout();
        cl->addWidget(new AntTypography(QStringLiteral("Ant Design")));
        cl->addWidget(new AntDivider());
        cl->addWidget(new AntTypography(QStringLiteral("Qt Widgets")));
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Text"));
        auto* cl = card->bodyLayout();
        cl->addWidget(new AntDivider(QStringLiteral("Center Text")));
        auto* start = new AntDivider(QStringLiteral("Start Text"));
        start->setTitlePlacement(Ant::DividerTitlePlacement::Start);
        cl->addWidget(start);
        auto* end = new AntDivider(QStringLiteral("End Text"));
        end->setTitlePlacement(Ant::DividerTitlePlacement::End);
        end->setPlain(false);
        cl->addWidget(end);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Variant and Size"));
        auto* cl = card->bodyLayout();
        auto* dashed = new AntDivider(QStringLiteral("Dashed"));
        dashed->setVariant(Ant::DividerVariant::Dashed);
        dashed->setDividerSize(Ant::Size::Small);
        cl->addWidget(dashed);
        auto* dotted = new AntDivider(QStringLiteral("Dotted"));
        dotted->setVariant(Ant::DividerVariant::Dotted);
        dotted->setDividerSize(Ant::Size::Middle);
        cl->addWidget(dotted);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical"));
        auto* cl = card->bodyLayout();
        auto* row = new QHBoxLayout();
        row->setSpacing(0);
        row->addWidget(new AntTypography(QStringLiteral("Text")));
        auto* v1 = new AntDivider();
        v1->setOrientation(Ant::Orientation::Vertical);
        row->addWidget(v1);
        row->addWidget(new AntTypography(QStringLiteral("Link")));
        auto* v2 = new AntDivider();
        v2->setOrientation(Ant::Orientation::Vertical);
        v2->setVariant(Ant::DividerVariant::Dashed);
        row->addWidget(v2);
        row->addWidget(new AntTypography(QStringLiteral("Action")));
        row->addStretch();
        cl->addLayout(row);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createFlexPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto makeChip = [](const QString& text, const QString& color) {
        return new ColoredLabel(text, QColor(color));
    };

    {
        auto* card = new AntCard(QStringLiteral("Horizontal"));
        auto* cl = card->bodyLayout();
        auto* horizontal = new AntFlex(page);
        horizontal->setGap(12);
        horizontal->addWidget(makeChip(QStringLiteral("Design"), QStringLiteral("#1677ff")));
        horizontal->addWidget(makeChip(QStringLiteral("Build"), QStringLiteral("#13c2c2")));
        horizontal->addWidget(makeChip(QStringLiteral("Review"), QStringLiteral("#52c41a")));
        horizontal->addStretch();
        cl->addWidget(horizontal);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical"));
        auto* cl = card->bodyLayout();
        auto* vertical = new AntFlex(page);
        vertical->setVertical(true);
        vertical->setGap(10);
        vertical->addWidget(makeChip(QStringLiteral("Token sync"), QStringLiteral("#722ed1")));
        vertical->addWidget(makeChip(QStringLiteral("Motion polish"), QStringLiteral("#eb2f96")));
        vertical->addWidget(makeChip(QStringLiteral("Example coverage"), QStringLiteral("#fa8c16")));
        cl->addWidget(vertical);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createGridPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto makeBlock = [](const QString& text, const QString& color) {
        return new ColoredLabel(text, QColor(color), 48);
    };

    {
        auto* card = new AntCard(QStringLiteral("24-Column Grid"));
        auto* cl = card->bodyLayout();
        auto* row1 = new AntRow(page);
        row1->setGutter(12);
        row1->addWidget(makeBlock(QStringLiteral("span 8"), QStringLiteral("#1677ff")), 8);
        row1->addWidget(makeBlock(QStringLiteral("span 8"), QStringLiteral("#13c2c2")), 8);
        row1->addWidget(makeBlock(QStringLiteral("span 8"), QStringLiteral("#52c41a")), 8);
        cl->addWidget(row1);
        cl->addSpacing(8);
        auto* row2 = new AntRow(page);
        row2->setGutter(12);
        row2->addWidget(makeBlock(QStringLiteral("span 6"), QStringLiteral("#722ed1")), 6);
        row2->addWidget(makeBlock(QStringLiteral("span 6 offset 2"), QStringLiteral("#eb2f96")), 6, 2);
        row2->addWidget(makeBlock(QStringLiteral("span 10"), QStringLiteral("#fa8c16")), 10);
        cl->addWidget(row2);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createLayoutPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic Layout"));
        auto* cl = card->bodyLayout();
        auto* basicLayout = new AntLayout();
        basicLayout->setFixedHeight(200);
        auto* header = new AntLayoutHeader();
        auto* headerLabel = new AntTypography(QStringLiteral("Header"), header);
        headerLabel->setType(Ant::TypographyType::LightSolid);
        headerLabel->setStrong(true);
        headerLabel->setGeometry(16, 16, 200, 32);
        basicLayout->setHeader(header);
        auto* content = new AntLayoutContent();
        auto* contentLabel = new AntTypography(QStringLiteral("Content"), content);
        contentLabel->setGeometry(16, 16, 200, 30);
        basicLayout->setContent(content);
        auto* footer = new AntLayoutFooter();
        auto* footerLabel = new AntTypography(QStringLiteral("Footer"), footer);
        footerLabel->setGeometry(16, 8, 200, 32);
        basicLayout->setFooter(footer);
        cl->addWidget(basicLayout);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Sider"));
        auto* cl = card->bodyLayout();
        auto* siderLayout = new AntLayout();
        siderLayout->setFixedHeight(200);
        auto* siderHeader = new AntLayoutHeader();
        auto* siderHeaderLabel = new AntTypography(QStringLiteral("Header"), siderHeader);
        siderHeaderLabel->setType(Ant::TypographyType::LightSolid);
        siderHeaderLabel->setStrong(true);
        siderHeaderLabel->setGeometry(16, 16, 200, 32);
        siderLayout->setHeader(siderHeader);
        auto* sider = new AntLayoutSider();
        sider->setWidth(200);
        sider->setSiderTheme(Ant::LayoutSiderTheme::Light);
        auto* siderLabel = new AntTypography(QStringLiteral("Sider"), sider);
        siderLabel->setGeometry(16, 8, 200, 30);
        siderLayout->addSider(sider);
        auto* siderContent = new AntLayoutContent();
        auto* siderContentLabel = new AntTypography(QStringLiteral("Content"), siderContent);
        siderContentLabel->setGeometry(16, 16, 200, 30);
        siderLayout->setContent(siderContent);
        cl->addWidget(siderLayout);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSpacePage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Horizontal (Small)"));
        auto* cl = card->bodyLayout();
        auto* hSmall = new AntSpace();
        hSmall->setSize(Ant::Size::Small);
        for (int i = 0; i < 4; ++i)
        {
            auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
            btn->setButtonType(Ant::ButtonType::Primary);
            hSmall->addItem(btn);
        }
        cl->addWidget(hSmall);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Horizontal (Middle)"));
        auto* cl = card->bodyLayout();
        auto* hMiddle = new AntSpace();
        hMiddle->setSize(Ant::Size::Middle);
        for (int i = 0; i < 4; ++i)
        {
            auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
            hMiddle->addItem(btn);
        }
        cl->addWidget(hMiddle);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Horizontal (Large)"));
        auto* cl = card->bodyLayout();
        auto* hLarge = new AntSpace();
        hLarge->setSize(Ant::Size::Large);
        for (int i = 0; i < 3; ++i)
        {
            auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
            hLarge->addItem(btn);
        }
        cl->addWidget(hLarge);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical"));
        auto* cl = card->bodyLayout();
        auto* vSpace = new AntSpace();
        vSpace->setOrientation(Ant::Orientation::Vertical);
        vSpace->setSize(Ant::Size::Middle);
        for (int i = 0; i < 3; ++i)
        {
            auto* btn = new AntButton(QStringLiteral("Button %1").arg(i + 1));
            btn->setButtonType(Ant::ButtonType::Primary);
            btn->setBlock(true);
            vSpace->addItem(btn);
        }
        cl->addWidget(vSpace);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createSplitterPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("Basic"));
        auto* cl = card->bodyLayout();
        auto* splitter = new AntSplitter(Qt::Horizontal, page);
        auto* left = new QTextEdit(splitter);
        left->setPlainText(QStringLiteral("Left panel"));
        auto* right = new QTextEdit(splitter);
        right->setPlainText(QStringLiteral("Right panel"));
        splitter->addWidget(left);
        splitter->addWidget(right);
        splitter->setSizes({200, 200});
        cl->addWidget(splitter);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
