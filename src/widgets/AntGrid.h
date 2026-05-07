#pragma once

#include "core/QtAntDesignExport.h"

#include <QWidget>

class QGridLayout;

class QT_ANT_DESIGN_EXPORT AntCol : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int span READ span WRITE setSpan NOTIFY spanChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)

public:
    explicit AntCol(int span = 24, QWidget* parent = nullptr);

    int span() const;
    void setSpan(int span);

    int offset() const;
    void setOffset(int offset);

Q_SIGNALS:
    void spanChanged(int span);
    void offsetChanged(int offset);

private:
    int m_span = 24;
    int m_offset = 0;
};

class QT_ANT_DESIGN_EXPORT AntRow : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int gutter READ gutter WRITE setGutter NOTIFY gutterChanged)

public:
    explicit AntRow(QWidget* parent = nullptr);

    int gutter() const;
    void setGutter(int px);

    void addWidget(QWidget* widget, int span = 24, int offset = 0);

Q_SIGNALS:
    void gutterChanged(int px);

private:
    void relayout();

    int m_gutter = 0;
    QGridLayout* m_grid = nullptr;
    struct ColInfo { QWidget* widget = nullptr; int span = 24; int offset = 0; };
    QList<ColInfo> m_cols;
};
