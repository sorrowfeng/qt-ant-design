#pragma once

#include "core/QtAntDesignExport.h"

#include <QPalette>
#include <QString>
#include <QToolBar>

class QActionEvent;
class QToolButton;

class QT_ANT_DESIGN_EXPORT AntToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit AntToolBar(QWidget* parent = nullptr);
    explicit AntToolBar(const QString& title, QWidget* parent = nullptr);

protected:
    void actionEvent(QActionEvent* event) override;

private:
    bool updateToolBarPalette(bool force = false);
    QString toolBarPaletteKey() const;
    void updateActionButtons(bool forceGeometry = false);
    void updateActionButton(QToolButton* button, bool forceGeometry);
    void syncToolBarPerfCounters() const;

    QPalette m_themedPalette;
    QString m_themedPaletteKey;
    int m_buttonScanCount = 0;
    int m_buttonSyncCount = 0;
    int m_buttonGeometryUpdateCount = 0;
    int m_buttonPaintUpdateCount = 0;
    int m_paletteApplyCount = 0;
};
