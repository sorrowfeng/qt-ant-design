#include "Pages.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>

#include <QPainter>

#include "core/AntTheme.h"
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
    explicit ColoredLabel(const QString& text, const QColor& bg, int h = 46, bool lightText = true, QWidget* parent = nullptr)
        : QFrame(parent), m_bg(bg), m_h(h)
    {
        setContentsMargins(12, 0, 12, 0);
        setMinimumHeight(h);
        auto* lay = new QHBoxLayout(this);
        lay->setContentsMargins(0, 0, 0, 0);
        auto* lbl = new AntTypography(text, this);
        if (lightText)
        {
            lbl->setType(Ant::TypographyType::LightSolid);
        }
        lbl->setAlignment(Qt::AlignHCenter);
        lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        lay->addWidget(lbl, 1, Qt::AlignVCenter);
    }
    QSize sizeHint() const override { return {0, m_h}; }
    QSize minimumSizeHint() const override { return {0, m_h}; }
protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(m_bg);
        p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), 4, 4);
    }
private:
    QColor m_bg;
    int m_h;
};

class LayoutPanel : public QFrame
{
public:
    explicit LayoutPanel(const QColor& bg, int h = 0, bool bottomBorder = false, QWidget* parent = nullptr)
        : QFrame(parent), m_bg(bg), m_bottomBorder(bottomBorder)
    {
        if (h > 0)
        {
            setMinimumHeight(h);
            setMaximumHeight(h);
        }
        setSizePolicy(QSizePolicy::Expanding, h > 0 ? QSizePolicy::Fixed : QSizePolicy::Expanding);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.fillRect(rect(), m_bg);
        if (m_bottomBorder)
        {
            p.setPen(QPen(QColor(QStringLiteral("#f0f0f0")), 1));
            p.drawLine(rect().bottomLeft(), rect().bottomRight());
        }
    }

private:
    QColor m_bg;
    bool m_bottomBorder = false;
};

class SplitterDemoFrame : public QFrame
{
public:
    explicit SplitterDemoFrame(QWidget* parent = nullptr)
        : QFrame(parent)
    {
        setFixedHeight(200);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(token.colorBorder, token.lineWidth));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), token.borderRadiusLG, token.borderRadiusLG);
    }
};

class SplitterPanel : public QFrame
{
public:
    explicit SplitterPanel(const QString& text, const QColor& bg, QWidget* parent = nullptr)
        : QFrame(parent), m_bg(bg)
    {
        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->addStretch();
        layout->addWidget(new AntTypography(text, this));
        layout->addStretch();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.fillRect(rect(), m_bg);
    }

private:
    QColor m_bg;
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
        cl->setSpacing(0);
        auto* top = new AntTypography(QStringLiteral("Top content"));
        top->setParagraph(true);
        cl->addWidget(top);
        cl->addWidget(new AntDivider());
        auto* bottom = new AntTypography(QStringLiteral("Bottom content"));
        bottom->setParagraph(true);
        cl->addWidget(bottom);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Title"));
        auto* cl = card->bodyLayout();
        cl->setSpacing(0);
        auto* start = new AntDivider(QStringLiteral("Left Text"));
        start->setTitlePlacement(Ant::DividerTitlePlacement::Start);
        cl->addWidget(start);
        cl->addWidget(new AntDivider(QStringLiteral("Center Text")));
        auto* end = new AntDivider(QStringLiteral("Right Text"));
        end->setTitlePlacement(Ant::DividerTitlePlacement::End);
        cl->addWidget(end);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Dashed & Vertical"));
        auto* cl = card->bodyLayout();
        cl->setSpacing(0);
        auto* row = new QHBoxLayout();
        row->setSpacing(0);
        row->addWidget(new AntTypography(QStringLiteral("Text")));
        auto* v1 = new AntDivider();
        v1->setOrientation(Ant::Orientation::Vertical);
        row->addWidget(v1);
        auto* firstLink = new AntTypography(QStringLiteral("Link"));
        firstLink->setType(Ant::TypographyType::Link);
        row->addWidget(firstLink);
        auto* v2 = new AntDivider();
        v2->setOrientation(Ant::Orientation::Vertical);
        row->addWidget(v2);
        auto* secondLink = new AntTypography(QStringLiteral("Link"));
        secondLink->setType(Ant::TypographyType::Link);
        row->addWidget(secondLink);
        row->addStretch();
        cl->addLayout(row);
        auto* dashed = new AntDivider();
        dashed->setVariant(Ant::DividerVariant::Dashed);
        cl->addWidget(dashed);
        auto* dashedCaption = new AntTypography(QStringLiteral("Dashed divider above"));
        dashedCaption->setParagraph(true);
        cl->addWidget(dashedCaption);
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

    {
        auto* card = new AntCard(QStringLiteral("Horizontal"));
        auto* cl = card->bodyLayout();
        auto* horizontal = new AntFlex(page);
        horizontal->setGap(16);
        for (const QString& label : {QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")})
        {
            auto* button = new AntButton(label);
            button->setButtonType(Ant::ButtonType::Primary);
            horizontal->addWidget(button);
        }
        cl->addWidget(horizontal);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical"));
        auto* cl = card->bodyLayout();
        auto* vertical = new AntFlex(page);
        vertical->setVertical(true);
        vertical->setGap(8);
        for (const QString& label : {QStringLiteral("Top"), QStringLiteral("Middle"), QStringLiteral("Bottom")})
        {
            vertical->addWidget(new AntButton(label));
        }
        cl->addWidget(vertical);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Wrap"));
        auto* cl = card->bodyLayout();
        auto* wrap = new AntFlex(page);
        wrap->setWrap(true);
        wrap->setGap(8);
        for (int i = 0; i < 12; ++i)
        {
            auto* button = new AntButton(QStringLiteral("B%1").arg(i + 1));
            button->setButtonType(Ant::ButtonType::Primary);
            button->setFixedWidth(80);
            wrap->addWidget(button);
        }
        cl->addWidget(wrap);
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

    auto makeBlock = [](const QString& text, const QString& color, bool lightText = true) {
        return new ColoredLabel(text, QColor(color), 46, lightText);
    };
    auto makeTintBlock = [&makeBlock](const QString& text, const QString& color) {
        QColor bg(color);
        bg.setAlphaF(0.67);
        return new ColoredLabel(text, bg, 46, false);
    };

    {
        auto* card = new AntCard(QStringLiteral("24-Column Grid"));
        auto* cl = card->bodyLayout();
        cl->setSpacing(0);
        auto* row1 = new AntRow(page);
        row1->setGutter(8);
        row1->addWidget(makeBlock(QStringLiteral("6"), QStringLiteral("#1677ff")), 6);
        row1->addWidget(makeTintBlock(QStringLiteral("6"), QStringLiteral("#1677ff")), 6);
        row1->addWidget(makeBlock(QStringLiteral("6"), QStringLiteral("#1677ff")), 6);
        row1->addWidget(makeTintBlock(QStringLiteral("6"), QStringLiteral("#1677ff")), 6);
        cl->addWidget(row1);
        cl->addSpacing(8);
        auto* row2 = new AntRow(page);
        row2->setGutter(8);
        row2->addWidget(makeBlock(QStringLiteral("8"), QStringLiteral("#52c41a")), 8);
        row2->addWidget(makeTintBlock(QStringLiteral("16"), QStringLiteral("#52c41a")), 16);
        cl->addWidget(row2);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Offset"));
        auto* cl = card->bodyLayout();
        auto* row = new AntRow(page);
        row->setGutter(8);
        row->addWidget(makeBlock(QStringLiteral("8"), QStringLiteral("#fa8c16")), 8);
        row->addWidget(makeTintBlock(QStringLiteral("8 offset 8"), QStringLiteral("#fa8c16")), 8, 8);
        cl->addWidget(row);
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

    auto addText = [](QWidget* parent, const QString& text, Ant::TypographyType type = Ant::TypographyType::Default) {
        auto* label = new AntTypography(text, parent);
        label->setType(type);
        auto* row = new QHBoxLayout(parent);
        row->setContentsMargins(24, 0, 24, 0);
        row->addWidget(label);
        return label;
    };

    {
        auto* card = new AntCard(QStringLiteral("Basic Layout"));
        auto* cl = card->bodyLayout();
        auto* basicLayout = new AntLayout();
        basicLayout->setFixedHeight(254);
        basicLayout->setBorderRadius(antTheme->tokens().borderRadiusLG);

        auto* header = new AntLayoutHeader();
        addText(header, QStringLiteral("Header"), Ant::TypographyType::LightSolid);
        basicLayout->setHeader(header);

        auto* content = new AntLayoutContent();
        auto* contentLayout = new QHBoxLayout(content);
        contentLayout->setContentsMargins(24, 24, 24, 24);
        contentLayout->addWidget(new AntTypography(QStringLiteral("Content"), content));
        contentLayout->addStretch();
        basicLayout->setContent(content);

        auto* footer = new AntLayoutFooter();
        auto* footerLayout = new QHBoxLayout(footer);
        footerLayout->setContentsMargins(50, 24, 50, 24);
        footerLayout->addStretch();
        footerLayout->addWidget(new AntTypography(QStringLiteral("Footer"), footer));
        footerLayout->addStretch();
        basicLayout->setFooter(footer);

        cl->addWidget(basicLayout);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("With Sider"));
        auto* cl = card->bodyLayout();
        auto* siderLayout = new AntLayout();
        siderLayout->setFixedHeight(144);
        siderLayout->setBorderRadius(antTheme->tokens().borderRadiusLG);

        auto* sider = new AntLayoutSider();
        sider->setWidth(160);
        auto* nav = new QVBoxLayout(sider);
        nav->setContentsMargins(24, 24, 24, 0);
        nav->setSpacing(0);
        for (const QString& text : {QStringLiteral("nav 1"), QStringLiteral("nav 2"), QStringLiteral("nav 3")})
        {
            auto* item = new AntTypography(text, sider);
            item->setType(Ant::TypographyType::LightSolid);
            item->setMinimumHeight(38);
            nav->addWidget(item);
        }
        nav->addStretch();
        siderLayout->addSider(sider);

        auto* siderContent = new AntLayoutContent();
        auto* contentStack = new QVBoxLayout(siderContent);
        contentStack->setContentsMargins(0, 0, 0, 0);
        contentStack->setSpacing(0);
        auto* whiteHeader = new LayoutPanel(QColor(QStringLiteral("#ffffff")), 64, true, siderContent);
        addText(whiteHeader, QStringLiteral("Header"));
        contentStack->addWidget(whiteHeader);
        auto* whiteContent = new LayoutPanel(QColor(QStringLiteral("#ffffff")), 80, false, siderContent);
        auto* whiteContentLayout = new QHBoxLayout(whiteContent);
        whiteContentLayout->setContentsMargins(24, 24, 24, 24);
        whiteContentLayout->addWidget(new AntTypography(QStringLiteral("Content"), whiteContent));
        whiteContentLayout->addStretch();
        contentStack->addWidget(whiteContent);
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
        auto* card = new AntCard(QStringLiteral("Horizontal"));
        auto* cl = card->bodyLayout();
        auto* space = new AntSpace();
        for (const QString& label : {QStringLiteral("A"), QStringLiteral("B"), QStringLiteral("C")})
        {
            space->addItem(new AntButton(label));
        }
        cl->addWidget(space);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Vertical"));
        auto* cl = card->bodyLayout();
        auto* space = new AntSpace();
        space->setOrientation(Ant::Orientation::Vertical);
        for (const QString& label : {QStringLiteral("Top"), QStringLiteral("Middle"), QStringLiteral("Bottom")})
        {
            space->addItem(new AntButton(label));
        }
        cl->addWidget(space);
        layout->addWidget(card);
    }

    {
        auto* card = new AntCard(QStringLiteral("Size"));
        auto* cl = card->bodyLayout();
        cl->setSpacing(0);

        auto* small = new AntSpace();
        small->setSize(Ant::Size::Small);
        small->addItem(new AntButton(QStringLiteral("S")));
        small->addItem(new AntButton(QStringLiteral("S")));
        cl->addWidget(small);
        cl->addSpacing(16);

        auto* middle = new AntSpace();
        middle->setSize(Ant::Size::Middle);
        middle->addItem(new AntButton(QStringLiteral("M")));
        middle->addItem(new AntButton(QStringLiteral("M")));
        cl->addWidget(middle);
        cl->addSpacing(16);

        auto* large = new AntSpace();
        large->setSize(Ant::Size::Large);
        large->addItem(new AntButton(QStringLiteral("L")));
        large->addItem(new AntButton(QStringLiteral("L")));
        cl->addWidget(large);
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
        auto* card = new AntCard(QStringLiteral("Basic Splitter"));
        auto* cl = card->bodyLayout();
        cl->setSpacing(0);

        auto* frame = new SplitterDemoFrame(page);
        auto* frameLayout = new QVBoxLayout(frame);
        frameLayout->setContentsMargins(1, 1, 1, 1);
        frameLayout->setSpacing(0);

        auto* splitter = new AntSplitter(Qt::Horizontal, frame);
        auto* left = new SplitterPanel(QStringLiteral("Left Panel"), QColor(QStringLiteral("#f0f5ff")), splitter);
        auto* right = new SplitterPanel(QStringLiteral("Right Panel"), QColor(QStringLiteral("#fff6ff")), splitter);
        splitter->addWidget(left);
        splitter->addWidget(right);
        splitter->setSizes({1, 1});
        frameLayout->addWidget(splitter);

        cl->addWidget(frame);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
