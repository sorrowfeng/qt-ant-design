#pragma once

#include <QToolBar>

class AntToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit AntToolBar(QWidget* parent = nullptr);
    explicit AntToolBar(const QString& title, QWidget* parent = nullptr);
};
