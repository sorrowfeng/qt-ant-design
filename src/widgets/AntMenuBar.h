#pragma once

#include "core/QtAntDesignExport.h"

#include <QMenuBar>

class QMenu;

class QT_ANT_DESIGN_EXPORT AntMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    explicit AntMenuBar(QWidget* parent = nullptr);

    QMenu* addMenu(const QString& title);
    QMenu* addMenu(const QIcon& icon, const QString& title);
};
