#pragma once

#include "core/QtAntDesignExport.h"

#include <QSplitter>

class AntSplitterHandle;

class QT_ANT_DESIGN_EXPORT AntSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit AntSplitter(QWidget* parent = nullptr);
    explicit AntSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);

protected:
    QSplitterHandle* createHandle() override;
};

class QT_ANT_DESIGN_EXPORT AntSplitterHandle : public QSplitterHandle
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
