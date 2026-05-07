#pragma once

#include "core/QtAntDesignExport.h"

#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;

class QT_ANT_DESIGN_EXPORT AntFlex : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool vertical READ vertical WRITE setVertical NOTIFY verticalChanged)
    Q_PROPERTY(int gap READ gap WRITE setGap NOTIFY gapChanged)
    Q_PROPERTY(bool wrap READ wrap WRITE setWrap NOTIFY wrapChanged)

public:
    explicit AntFlex(QWidget* parent = nullptr);

    bool vertical() const;
    void setVertical(bool v);

    int gap() const;
    void setGap(int px);

    bool wrap() const;
    void setWrap(bool w);

    void addWidget(QWidget* widget);
    void addStretch();

Q_SIGNALS:
    void verticalChanged(bool v);
    void gapChanged(int px);
    void wrapChanged(bool w);

private:
    void rebuildLayout();

    bool m_vertical = false;
    int m_gap = 8;
    bool m_wrap = false;
    QList<QWidget*> m_children;
    QLayout* m_layout = nullptr;
};
