#pragma once

#include "core/QtAntDesignExport.h"

#include <QWidget>

class QListWidget;
class AntButton;
class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

class QT_ANT_DESIGN_EXPORT AntTransfer : public QWidget
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
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

    void doTransfer(bool toTarget);
    void updateButtons();
    int rowAt(const QPoint& pos, bool sourcePanel) const;
    QRect panelRect(bool sourcePanel) const;
    QRect headerCheckRect(bool sourcePanel) const;
    int visibleRowCount() const;
    int maxScrollOffset(bool sourcePanel) const;
    int scrollOffset(bool sourcePanel) const;
    void setScrollOffset(bool sourcePanel, int offset);
    void togglePanelSelection(bool sourcePanel);

    QListWidget* m_sourceList = nullptr;
    QListWidget* m_targetList = nullptr;
    AntButton* m_toTargetBtn = nullptr;
    AntButton* m_toSourceBtn = nullptr;
    QStringList m_selectedSourceItems;
    QStringList m_selectedTargetItems;
    int m_sourceScrollOffset = 0;
    int m_targetScrollOffset = 0;
};
