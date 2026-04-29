#pragma once

#include <QToolBar>

class QActionEvent;

class AntToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit AntToolBar(QWidget* parent = nullptr);
    explicit AntToolBar(const QString& title, QWidget* parent = nullptr);

protected:
    void actionEvent(QActionEvent* event) override;

private:
    void updateActionButtons();
};
