#pragma once

#include "core/QtAntDesignExport.h"

#include <QFont>
#include <QVector>
#include <QWidget>

class QEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

struct AntBreadcrumbItem
{
    QString title;
    QString href;
    QString iconText;
    bool disabled = false;
    bool separatorOnly = false;
    QString separator;
};

class QT_ANT_DESIGN_EXPORT AntBreadcrumb : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString separator READ separator WRITE setSeparator NOTIFY separatorChanged)

public:
    explicit AntBreadcrumb(QWidget* parent = nullptr);

    QString separator() const;
    void setSeparator(const QString& separator);

    void addItem(const QString& title,
                 const QString& href = QString(),
                 const QString& iconText = QString(),
                 bool disabled = false);
    void addSeparator(const QString& separator);
    void clearItems();

    int count() const;
    AntBreadcrumbItem itemAt(int index) const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void separatorChanged(const QString& separator);
    void itemClicked(int index, const QString& title, const QString& href);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    friend class AntBreadcrumbStyle;

    QVector<QRect> itemRects() const;
    int itemIndexAt(const QPoint& pos) const;
    bool isLastRouteItem(int index) const;
    int itemWidth(const AntBreadcrumbItem& item) const;
    QColor itemColor(const AntBreadcrumbItem& item, int index, bool hovered) const;
    void updateBreadcrumbGeometry();
    void invalidateBreadcrumbLayoutCache() const;
    void ensureBreadcrumbLayoutCache() const;
    QRect itemDirtyRect(int index) const;
    void syncBreadcrumbPerfCounters() const;

    QVector<AntBreadcrumbItem> m_items;
    QString m_separator = QStringLiteral("/");
    int m_hoveredIndex = -1;

    mutable bool m_layoutCacheValid = false;
    mutable QVector<QRect> m_cachedItemRects;
    mutable QVector<int> m_cachedItemWidths;
    mutable int m_cachedTotalWidth = 0;
    mutable int m_cachedHeight = 0;
    mutable QFont m_cachedFont;
    mutable QString m_cachedSeparator;
    mutable int m_cachedTokenFontSize = 0;
    mutable int m_cachedPaddingXS = 0;
    mutable int m_cachedMarginXS = 0;
    mutable int m_layoutCacheHitCount = 0;
    mutable int m_layoutCacheMissCount = 0;
    mutable int m_hoverScopedUpdateCount = 0;
    mutable int m_geometryUpdateCount = 0;
};
