#pragma once

#include <QWidget>

class QListWidget;
class AntButton;
class QPaintEvent;

class AntTransfer : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList sourceItems READ sourceItems NOTIFY itemsChanged)
    Q_PROPERTY(QStringList targetItems READ targetItems NOTIFY itemsChanged)

public:
    explicit AntTransfer(QWidget* parent = nullptr);

    QStringList sourceItems() const;
    QStringList targetItems() const;

    void setSourceItems(const QStringList& items);
    void setTargetItems(const QStringList& items);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void itemsChanged();

private:
    void paintEvent(QPaintEvent* event) override;

    void doTransfer(bool toTarget);
    void updateButtons();

    QListWidget* m_sourceList = nullptr;
    QListWidget* m_targetList = nullptr;
    AntButton* m_toTargetBtn = nullptr;
    AntButton* m_toSourceBtn = nullptr;
};
