#pragma once

#include <QWidget>

class AntMasonry : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)

public:
    explicit AntMasonry(QWidget* parent = nullptr);

    int columns() const;
    void setColumns(int cols);

    int spacing() const;
    void setSpacing(int px);

    void addWidget(QWidget* widget);
    void clear();

Q_SIGNALS:
    void columnsChanged(int cols);
    void spacingChanged(int px);

protected:
    void resizeEvent(QResizeEvent*) override;

private:
    void relayout();

    int m_columns = 3;
    int m_spacing = 8;
    QList<QWidget*> m_items;
};
