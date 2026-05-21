#pragma once

#include "core/QtAntDesignExport.h"

#include <QHash>
#include <QRect>
#include <QSize>
#include <QWidget>
#include <QVector>

#include "core/AntTypes.h"
#include "AntTree.h"

class AntTreeSelectStyle;
class QResizeEvent;

class QT_ANT_DESIGN_EXPORT AntTreeSelect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString placeholder READ placeholder WRITE setPlaceholder NOTIFY placeholderChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool multiple READ isMultiple WRITE setMultiple NOTIFY multipleChanged)
    Q_PROPERTY(bool treeCheckable READ isTreeCheckable WRITE setTreeCheckable NOTIFY treeCheckableChanged)
    Q_PROPERTY(bool showSearch READ isShowSearch WRITE setShowSearch NOTIFY showSearchChanged)
    Q_PROPERTY(Ant::Size selectSize READ selectSize WRITE setSelectSize NOTIFY selectSizeChanged)
    Q_PROPERTY(Ant::Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(bool isOpen READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntTreeSelect(QWidget* parent = nullptr);
    ~AntTreeSelect() override;

    QVector<AntTreeNode> treeData() const;
    void setTreeData(const QVector<AntTreeNode>& data);

    QStringList value() const;
    void setValue(const QStringList& keys);

    QString placeholder() const;
    void setPlaceholder(const QString& text);

    bool allowClear() const;
    void setAllowClear(bool enable);

    bool isMultiple() const;
    void setMultiple(bool enable);

    bool isTreeCheckable() const;
    void setTreeCheckable(bool enable);

    bool isShowSearch() const;
    void setShowSearch(bool enable);

    Ant::Size selectSize() const;
    void setSelectSize(Ant::Size size);

    Ant::Status status() const;
    void setStatus(Ant::Status status);

    Ant::Variant variant() const;
    void setVariant(Ant::Variant variant);

    bool isOpen() const;
    void setOpen(bool open);

    QString displayText() const;
    bool isHovered() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void valueChanged(const QStringList& keys);
    void openChanged(bool open);
    void placeholderChanged(const QString& text);
    void allowClearChanged(bool enable);
    void multipleChanged(bool enable);
    void treeCheckableChanged(bool enable);
    void showSearchChanged(bool enable);
    void selectSizeChanged(Ant::Size size);
    void statusChanged(Ant::Status status);
    void variantChanged(Ant::Variant variant);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    friend class AntTreeSelectStyle;
    class TreeSelectPopup;

    struct TriggerLayout
    {
        QSize widgetSize;
        Ant::Size selectSize = Ant::Size::Middle;
        int height = 32;
        int fontSize = 14;
        int paddingX = 12;
        int arrowWidth = 24;
        int radius = 6;
        QRect triggerRect;
        QRect clearButtonRect;
        QRect arrowRect;
        QSize sizeHint;
        QSize minimumSizeHint;
        bool valid = false;
    };

    void showPopup();
    void hidePopup();
    bool updateDisplayText();
    QString findNodeTitle(const QVector<AntTreeNode>& nodes, const QString& key) const;
    QStringList findNodeTitles(const QVector<AntTreeNode>& nodes, const QStringList& keys) const;
    const TriggerLayout& triggerLayout() const;
    QRect triggerRect() const;
    QRect clearButtonRect() const;
    QRect arrowRect() const;
    bool isOverClear(const QPoint& pos) const;
    int treeDataRevision() const;
    int visibleTreeRowCount() const;
    void invalidateTriggerLayout() const;
    void invalidateTreeCaches() const;
    void invalidateTitleCache() const;
    void ensureTitleCache() const;
    void collectNodeTitles(const QVector<AntTreeNode>& nodes) const;
    void updateTriggerRegion(const QRect& dirty, const QString& mode, bool clearScoped = false);
    void syncTreeSelectPerfCounters() const;

    QVector<AntTreeNode> m_treeData;
    QStringList m_value;
    QString m_placeholder;
    bool m_allowClear = false;
    bool m_multiple = false;
    bool m_treeCheckable = false;
    bool m_showSearch = false;
    Ant::Size m_selectSize = Ant::Size::Middle;
    Ant::Status m_status = Ant::Status::Normal;
    Ant::Variant m_variant = Ant::Variant::Outlined;
    bool m_open = false;
    bool m_hovered = false;
    QString m_displayText;
    TreeSelectPopup* m_popup = nullptr;
    int m_treeDataRevision = 0;
    mutable TriggerLayout m_triggerLayout;
    mutable int m_triggerLayoutBuildCount = 0;
    mutable int m_triggerLayoutCacheHitCount = 0;
    mutable int m_cachedVisibleRowRevision = -1;
    mutable int m_cachedVisibleRows = 1;
    mutable int m_visibleRowsBuildCount = 0;
    mutable int m_visibleRowsCacheHitCount = 0;
    mutable int m_titleCacheRevision = -1;
    mutable int m_titleCacheBuildCount = 0;
    mutable QHash<QString, QString> m_titleCache;
    int m_triggerUpdateCount = 0;
    int m_clearRegionUpdateCount = 0;
};
