#pragma once

#include <QMenuBar>

class QMenu;

class AntMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    explicit AntMenuBar(QWidget* parent = nullptr);

    QMenu* addMenu(const QString& title);
    QMenu* addMenu(const QIcon& icon, const QString& title);
};
