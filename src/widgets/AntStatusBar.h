#pragma once

#include "core/QtAntDesignExport.h"

#include <QFont>
#include <QRect>
#include <QVector>
#include <QWidget>

class AntStatusBarStyle;
class QEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QTimer;

struct AntStatusBarItem
{
    QString text;
    QString icon;
    QString tooltip;
    int stretch = 0;

    bool operator==(const AntStatusBarItem& other) const
    {
        return text == other.text &&
               icon == other.icon &&
               tooltip == other.tooltip &&
               stretch == other.stretch;
    }
};

class QT_ANT_DESIGN_EXPORT AntStatusBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(bool sizeGrip READ hasSizeGrip WRITE setSizeGrip NOTIFY sizeGripChanged)

public:
    explicit AntStatusBar(QWidget* parent = nullptr);

    QString message() const;
    void setMessage(const QString& message);
    QString currentMessage() const;
    void showMessage(const QString& message, int timeout = 0);
    void clearMessage();

    bool hasSizeGrip() const;
    void setSizeGrip(bool enabled);

    int addItem(const QString& text,
                const QString& icon = QString(),
                const QString& tooltip = QString(),
                int stretch = 0);
    int addPermanentItem(const QString& text,
                         const QString& icon = QString(),
                         const QString& tooltip = QString(),
                         int stretch = 0);
    void removeItem(int index);

    int itemCount() const;
    int permanentItemCount() const;
    AntStatusBarItem itemAt(int index) const;
    AntStatusBarItem permanentItemAt(int index) const;

    int hoveredRegularIndex() const;
    int hoveredPermanentIndex() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void messageChanged(const QString& message);
    void sizeGripChanged(bool enabled);
    void itemClicked(int index);

protected:
    void changeEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    friend class AntStatusBarStyle;

    int regularItemIndexAt(const QPoint& pos) const;
    int permanentItemIndexAt(const QPoint& pos) const;
    void invalidateLayoutCache() const;
    void ensureLayoutCache() const;
    void updateStatusRegion(const QRect& rect);
    void syncStatusBarPerfCounters() const;

    const QVector<QRect>& regularItemRects() const;
    const QVector<QRect>& permanentItemRects() const;
    const QVector<int>& regularDividerXs() const;
    const QVector<int>& permanentDividerXs() const;
    QRect messageAreaRect() const;

    QVector<AntStatusBarItem> m_items;
    QVector<AntStatusBarItem> m_permanentItems;
    QString m_message;
    bool m_sizeGrip = true;
    int m_hoveredRegularIndex = -1;
    int m_hoveredPermanentIndex = -1;
    QTimer* m_messageTimer = nullptr;
    mutable bool m_layoutCacheDirty = true;
    mutable int m_layoutCacheWidth = -1;
    mutable int m_layoutCacheHeight = -1;
    mutable bool m_layoutCacheSizeGrip = true;
    mutable QFont m_layoutCacheFont;
    mutable QVector<AntStatusBarItem> m_layoutCacheItems;
    mutable QVector<AntStatusBarItem> m_layoutCachePermanentItems;
    mutable QVector<QRect> m_regularItemRects;
    mutable QVector<QRect> m_permanentItemRects;
    mutable QVector<int> m_regularDividerXs;
    mutable QVector<int> m_permanentDividerXs;
    mutable QRect m_messageRect;
    mutable int m_layoutBuildCount = 0;
    int m_regionUpdateCount = 0;
    int m_messageRegionUpdateCount = 0;
};
