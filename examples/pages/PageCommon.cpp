#include "PageCommon.h"

#include <QFrame>
#include <QScrollArea>

#include "widgets/AntScrollBar.h"

namespace example::pages
{
QScrollArea* wrapPage(QWidget* page)
{
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));
    scroll->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal));
    scroll->setWidget(page);
    return scroll;
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
