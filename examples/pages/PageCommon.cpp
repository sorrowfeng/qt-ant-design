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
}
