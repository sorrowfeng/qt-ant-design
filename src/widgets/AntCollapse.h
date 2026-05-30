#pragma once

#include "core/QtAntDesignExport.h"

#include <QRect>
#include <QSize>
#include <QWidget>
#include <QVariantAnimation>

class QEvent;
class QVBoxLayout;

class QT_ANT_DESIGN_EXPORT AntCollapsePanel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)

public:
    explicit AntCollapsePanel(const QString& title, QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    QWidget* contentWidget() const;
    void setContentWidget(QWidget* widget);

    QSize sizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void expandedChanged(bool expanded);
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void enterEvent(AntEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    void updateAnimation();
    void applyExpanded(bool expanded, bool animate = true);
    void invalidateSizeHintCache() const;
    QRect headerRect() const;
    QRect contentPaintRect(int contentHeight) const;
    void refreshStaticContentHeight(const QString& mode);
    void requestPanelUpdate(const QRect& region, const QString& mode, bool contentScoped = false);
    void syncPanelPerfCounters() const;

    QString m_title;
    bool m_expanded = false;
    bool m_hovered = false;
    QWidget* m_content = nullptr;
    QVBoxLayout* m_layout = nullptr;
    QVariantAnimation* m_animation = nullptr;
    int m_contentHeight = 0;
    mutable bool m_sizeHintDirty = true;
    mutable QSize m_cachedSizeHint;
    mutable int m_sizeHintBuildCount = 0;
    mutable int m_sizeHintHitCount = 0;
    int m_layoutUpdateCount = 0;
    int m_panelRegionUpdateCount = 0;
    int m_contentRegionUpdateCount = 0;
    QString m_lastUpdateMode;
};

class QT_ANT_DESIGN_EXPORT AntCollapse : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool accordion READ accordion WRITE setAccordion NOTIFY accordionChanged)
    Q_PROPERTY(bool bordered READ bordered WRITE setBordered NOTIFY borderedChanged)

public:
    explicit AntCollapse(QWidget* parent = nullptr);

    bool accordion() const;
    void setAccordion(bool accordion);

    bool bordered() const;
    void setBordered(bool bordered);

    AntCollapsePanel* addPanel(const QString& title);

    QSize sizeHint() const override;

Q_SIGNALS:
    void accordionChanged(bool accordion);
    void borderedChanged(bool bordered);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    friend class AntCollapsePanel;

    void onPanelExpanded(AntCollapsePanel* panel, bool expanded);
    void invalidateSizeHintCache() const;
    void syncCollapsePerfCounters() const;

    bool m_accordion = false;
    bool m_bordered = true;
    QVBoxLayout* m_layout = nullptr;
    QList<AntCollapsePanel*> m_panels;
    mutable bool m_sizeHintDirty = true;
    mutable QSize m_cachedSizeHint;
    mutable int m_sizeHintBuildCount = 0;
    mutable int m_sizeHintHitCount = 0;
};
