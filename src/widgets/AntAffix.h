#pragma once

#include "core/QtAntDesignExport.h"

#include <QObject>
#include <QWidget>

class QAbstractScrollArea;

class QT_ANT_DESIGN_EXPORT AntAffix : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int offsetTop READ offsetTop WRITE setOffsetTop NOTIFY offsetTopChanged)
    Q_PROPERTY(int offsetBottom READ offsetBottom WRITE setOffsetBottom NOTIFY offsetBottomChanged)

public:
    explicit AntAffix(QObject* parent = nullptr);
    ~AntAffix() override;

    int offsetTop() const;
    void setOffsetTop(int offset);
    int offsetBottom() const;
    void setOffsetBottom(int offset);

    QWidget* affixedWidget() const;
    void setAffixedWidget(QWidget* widget);
    QWidget* scrollTarget() const;
    void setScrollTarget(QWidget* target);

    bool isAffixed() const;

Q_SIGNALS:
    void offsetTopChanged(int);
    void offsetBottomChanged(int);
    void affixStateChanged(bool affixed);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void findScrollContainer();
    void attachScrollMonitor();
    void detachScrollMonitor();
    void checkAffixState();
    void applyAffixed();
    void removeAffixed();

    int m_offsetTop = 0;
    int m_offsetBottom = 0;
    bool m_hasOffsetTop = false;
    bool m_hasOffsetBottom = false;
    QWidget* m_scrollTarget = nullptr;
    QWidget* m_affixedWidget = nullptr;
    QWidget* m_scrollViewport = nullptr;
    QWidget* m_placeholder = nullptr;
    bool m_isAffixed = false;
    QPoint m_originalPos;
    QWidget* m_originalParent = nullptr;
    QSize m_originalSize;
};
