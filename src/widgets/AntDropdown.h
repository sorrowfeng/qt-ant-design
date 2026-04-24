#pragma once

#include <QPointer>
#include <QStringList>
#include <QWidget>

#include "core/AntTypes.h"

class AntMenu;
class QFrame;
class QTimer;

class AntDropdown : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList itemLabels READ itemLabels WRITE setItemLabels NOTIFY itemLabelsChanged)
    Q_PROPERTY(Ant::DropdownPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(Ant::DropdownTrigger trigger READ trigger WRITE setTrigger NOTIFY triggerChanged)
    Q_PROPERTY(bool arrowVisible READ arrowVisible WRITE setArrowVisible NOTIFY arrowVisibleChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntDropdown(QWidget* parent = nullptr);
    ~AntDropdown() override;

    QStringList itemLabels() const;
    void setItemLabels(const QStringList& labels);

    Ant::DropdownPlacement placement() const;
    void setPlacement(Ant::DropdownPlacement placement);

    Ant::DropdownTrigger trigger() const;
    void setTrigger(Ant::DropdownTrigger trigger);

    bool arrowVisible() const;
    void setArrowVisible(bool visible);

    bool isOpen() const;
    void setOpen(bool open);

    QWidget* target() const;
    void setTarget(QWidget* target);

    AntMenu* menu() const;
    void clearItems();
    void addItem(const QString& key, const QString& label, const QString& iconText = QString(), bool disabled = false);
    void addDivider();

Q_SIGNALS:
    void itemLabelsChanged(const QStringList& labels);
    void placementChanged(Ant::DropdownPlacement placement);
    void triggerChanged(Ant::DropdownTrigger trigger);
    void arrowVisibleChanged(bool visible);
    void openChanged(bool open);
    void itemTriggered(const QString& key);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    class PopupFrame;

    QRect popupGeometry(const QRect& targetRect, const QSize& popupSize, Ant::DropdownPlacement placement) const;
    Ant::DropdownPlacement resolvedPlacement(const QRect& targetRect, const QSize& popupSize, const QRect& screenRect) const;
    void updatePopupGeometry(const QPoint& contextPos = QPoint());
    void installTarget(QWidget* target);
    void uninstallTarget();
    void handleTargetEnter();
    void handleTargetLeave();
    void handlePopupEnter();
    void handlePopupLeave();

    QStringList m_itemLabels;
    Ant::DropdownPlacement m_placement = Ant::DropdownPlacement::BottomLeft;
    Ant::DropdownTrigger m_trigger = Ant::DropdownTrigger::Hover;
    bool m_arrowVisible = false;
    bool m_open = false;
    QPointer<QWidget> m_target;
    PopupFrame* m_popup = nullptr;
    AntMenu* m_menu = nullptr;
    QTimer* m_openTimer = nullptr;
    QTimer* m_closeTimer = nullptr;
    QPoint m_lastContextPos;
};
