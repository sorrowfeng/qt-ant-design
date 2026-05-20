#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QPlainTextEdit>

#include "core/AntTypes.h"

class QMouseEvent;
class QEvent;

class QT_ANT_DESIGN_EXPORT AntPlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText NOTIFY placeholderTextChanged)

public:
    explicit AntPlainTextEdit(QWidget* parent = nullptr);
    explicit AntPlainTextEdit(const QString& text, QWidget* parent = nullptr);

    Ant::Variant variant() const;
    void setVariant(Ant::Variant variant);

    QString placeholderText() const;
    void setPlaceholderText(const QString& text);

Q_SIGNALS:
    void variantChanged(Ant::Variant variant);
    void placeholderTextChanged(const QString& text);

protected:
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QRect resizeGripRect() const;
    bool handleResizeGripMouseEvent(QMouseEvent* event, const QPoint& widgetPos);
    void applyVisualState();
    void setResizeGripHovered(bool hovered);
    void updateResizeGripRegion();
    void syncPlainTextEditPerfCounters() const;

    Ant::Variant m_variant = Ant::Variant::Outlined;
    QString m_placeholderText;
    bool m_focused = false;
    bool m_resizing = false;
    bool m_resizeGripHovered = false;
    QPoint m_resizeStartGlobal;
    QSize m_resizeStartSize;
    int m_cachedVisualFontSize = -1;
    QColor m_cachedTextColor;
    QColor m_cachedPlaceholderColor;
    QColor m_cachedDisabledTextColor;
    int m_visualStateApplyCount = 0;
    int m_resizeGripCursorUpdateCount = 0;
    int m_resizeGripDirtyUpdateCount = 0;
};
