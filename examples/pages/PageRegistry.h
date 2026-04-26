#pragma once

#include <QString>
#include <QVector>
#include <functional>

class QWidget;

namespace example::pages
{
struct PageEntry
{
    QString category;
    QString name;
    std::function<QWidget*(QWidget* owner)> factory;
};

QVector<PageEntry> buildPageRegistry();
}
