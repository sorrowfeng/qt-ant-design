#pragma once

#include <QColor>
#include <QPointer>
#include <QRect>
#include <QWidget>

class QPaintEvent;

// Click-wave overlay: draws an expanding soft glow around a target widget (or
// a specific rect within it), matching Ant Design's `.wave-motion-appear-active`
// (box-shadow 0 -> 6px over 0.4s, opacity 0.2 -> 0 over 2s).
class AntWave : public QWidget
{
    Q_OBJECT

public:
    // Spawn a wave tied to `target`'s full geometry. Self-deletes after finish.
    static void trigger(QWidget* target, const QColor& color = QColor(), int radius = 6, bool quick = false);

    // Spawn a wave around a specific rect (in `target`'s local coordinates).
    // Useful when only a portion of the widget visually "responds" — e.g. the
    // 16x16 check box of an AntCheckbox rather than the whole label row.
    static void triggerRect(QWidget* target, const QRect& localRect,
                             const QColor& color = QColor(), int radius = 6, bool quick = false);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    AntWave(QWidget* target, const QRect& localRect, const QColor& color, int radius, bool quick);
    void tick(qreal t);

    QPointer<QWidget> m_target;
    QRect m_localRect; // target-local rect to wave around
    QColor m_color;
    int m_radius = 6;
    bool m_quick = false;
    qreal m_progress = 0.0;
};
