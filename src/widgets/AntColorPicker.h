#pragma once

#include <QDialog>
#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include "core/AntTypes.h"

class AntColorPicker : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QColor currentColor READ currentColor WRITE setCurrentColor NOTIFY currentColorChanged)

public:
    explicit AntColorPicker(QWidget* parent = nullptr);
    explicit AntColorPicker(const QColor& initial, QWidget* parent = nullptr);

    QColor currentColor() const;
    void setCurrentColor(const QColor& color);

    static QColor getColor(const QColor& initial = Qt::white, QWidget* parent = nullptr,
                           const QString& title = QString());

Q_SIGNALS:
    void colorSelected(const QColor& color);
    void currentColorChanged(const QColor& color);

private:
    void setupUi();
    void updateFromColor(const QColor& color);
    void updateSlidersFromColor();
    void updateEditFieldsFromColor();
    void updatePreviewFromColor();
    void syncColor();

    QColor m_currentColor = Qt::white;
    QColor m_previousColor = Qt::white;
    bool m_updating = false;

    // Sub-widgets
    QWidget* m_hsField = nullptr;
    QWidget* m_valueSlider = nullptr;
    QLineEdit* m_hexEdit = nullptr;
    QComboBox* m_modeCombo = nullptr;
    QSpinBox* m_rEdit = nullptr;
    QSpinBox* m_gEdit = nullptr;
    QSpinBox* m_bEdit = nullptr;
    QLabel* m_rLabel = nullptr;
    QLabel* m_gLabel = nullptr;
    QLabel* m_bLabel = nullptr;
    QWidget* m_previewOld = nullptr;
    QWidget* m_previewNew = nullptr;
    QWidget* m_presetGrid = nullptr;
    QWidget* m_customGrid = nullptr;
    QPushButton* m_addCustomBtn = nullptr;
    QPushButton* m_removeCustomBtn = nullptr;

    QList<QColor> m_customColors;
};
