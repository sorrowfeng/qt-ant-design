#include "PageCommon.h"

#include <QFrame>
#include <QScrollArea>

#include "core/AntTypes.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntTypography.h"

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

AntTypography* createSectionTitle(const QString& title)
{
    auto* typo = new AntTypography(title);
    typo->setTitle(true);
    typo->setTitleLevel(Ant::TypographyTitleLevel::H5);
    return typo;
}
}
