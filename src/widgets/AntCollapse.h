#pragma once

#include <QWidget>
#include <QVariantAnimation>

class QVBoxLayout;

class AntCollapsePanel : public QWidget
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
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    void updateAnimation();
    void applyExpanded(bool expanded, bool animate = true);

    QString m_title;
    bool m_expanded = false;
    bool m_hovered = false;
    QWidget* m_content = nullptr;
    QVBoxLayout* m_layout = nullptr;
    QVariantAnimation* m_animation = nullptr;
    int m_contentHeight = 0;
};

class AntCollapse : public QWidget
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
    void onPanelExpanded(AntCollapsePanel* panel, bool expanded);

    bool m_accordion = false;
    bool m_bordered = true;
    QVBoxLayout* m_layout = nullptr;
    QList<AntCollapsePanel*> m_panels;
};
