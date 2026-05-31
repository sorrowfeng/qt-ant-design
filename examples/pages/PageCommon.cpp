#include "PageCommon.h"

#include <QVBoxLayout>

#include "widgets/AntScrollArea.h"
#include "widgets/AntWidget.h"

namespace example::pages
{
QWidget* wrapPage(QWidget* page)
{
    constexpr int resizeGuardMargin = 9;

    auto* host = new AntWidget();
    auto* layout = new QVBoxLayout(host);
    layout->setContentsMargins(0, 0, resizeGuardMargin, resizeGuardMargin);
    layout->setSpacing(0);

    auto* scroll = new AntScrollArea(host);
    scroll->setWidgetResizable(true);
    scroll->setAutoHideScrollBar(false);
    scroll->setWidget(page);
    layout->addWidget(scroll);
    return host;
}

AntTypography* makeText(const QString& text, QWidget* parent, Ant::TypographyType type)
{
    auto* label = new AntTypography(text, parent);
    label->setType(type);
    return label;
}

AntTypography* makeSecondaryText(const QString& text, QWidget* parent)
{
    return makeText(text, parent, Ant::TypographyType::Secondary);
}

AntTypography* makeParagraph(const QString& text, QWidget* parent, Ant::TypographyType type)
{
    auto* paragraph = makeText(text, parent, type);
    paragraph->setParagraph(true);
    return paragraph;
}
}
