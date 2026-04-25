#pragma once

#include <QSplitter>

class AntSplitterHandle;

class AntSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit AntSplitter(QWidget* parent = nullptr);
    explicit AntSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
    QSplitterHandle* createHandle() override;
};

class AntSplitterHandle : public QSplitterHandle
{
    Q_OBJECT

public:
    AntSplitterHandle(Qt::Orientation orientation, AntSplitter* parent);

protected:
    void paintEvent(QPaintEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    bool m_hovered = false;
};
