#pragma once

#include "core/QtAntDesignExport.h"

#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;

class QT_ANT_DESIGN_EXPORT AntPagination : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int current READ current WRITE setCurrent NOTIFY currentChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)
    Q_PROPERTY(int total READ total WRITE setTotal NOTIFY totalChanged)
    Q_PROPERTY(bool disabled READ isDisabled WRITE setDisabled NOTIFY disabledChanged)
    Q_PROPERTY(bool simple READ isSimple WRITE setSimple NOTIFY simpleChanged)
    Q_PROPERTY(bool showLessItems READ isShowLessItems WRITE setShowLessItems NOTIFY showLessItemsChanged)
    Q_PROPERTY(bool showTotal READ isShowTotal WRITE setShowTotal NOTIFY showTotalChanged)
    Q_PROPERTY(bool showQuickJumper READ isShowQuickJumper WRITE setShowQuickJumper NOTIFY showQuickJumperChanged)
    Q_PROPERTY(bool showSizeChanger READ isShowSizeChanger WRITE setShowSizeChanger NOTIFY showSizeChangerChanged)
    Q_PROPERTY(Ant::Size paginationSize READ paginationSize WRITE setPaginationSize NOTIFY paginationSizeChanged)

public:
    explicit AntPagination(QWidget* parent = nullptr);

    int current() const;
    void setCurrent(int current);

    int pageSize() const;
    void setPageSize(int pageSize);

    int total() const;
    void setTotal(int total);

    bool isDisabled() const;
    void setDisabled(bool disabled);

    bool isSimple() const;
    void setSimple(bool simple);

    bool isShowLessItems() const;
    void setShowLessItems(bool show);

    bool isShowTotal() const;
    void setShowTotal(bool show);

    bool isShowQuickJumper() const;
    void setShowQuickJumper(bool show);

    bool isShowSizeChanger() const;
    void setShowSizeChanger(bool show);

    Ant::Size paginationSize() const;
    void setPaginationSize(Ant::Size size);

    int pageCount() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void currentChanged(int current);
    void pageSizeChanged(int pageSize);
    void totalChanged(int total);
    void disabledChanged(bool disabled);
    void simpleChanged(bool simple);
    void showLessItemsChanged(bool show);
    void showTotalChanged(bool show);
    void showQuickJumperChanged(bool show);
    void showSizeChangerChanged(bool show);
    void paginationSizeChanged(Ant::Size size);
    void change(int page, int pageSize);
    void showSizeChange(int current, int pageSize);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    enum class ItemKind
    {
        Prev,
        Next,
        Page,
        JumpPrev,
        JumpNext,
        Text,
        SizeChanger,
        QuickJumper,
    };

    struct PageItem
    {
        ItemKind kind = ItemKind::Page;
        int page = 0;
        QString text;
        QRect rect;
        bool enabled = true;
        bool active = false;
    };

    QVector<PageItem> pageItems() const;
    int itemAt(const QPoint& pos) const;
    int itemSize() const;
    int itemSpacing() const;
    int fontSize() const;
    int rangeStart() const;
    int rangeEnd() const;
    QColor itemTextColor(const PageItem& item, bool hovered) const;
    QColor itemBackgroundColor(const PageItem& item, bool hovered) const;
    void drawItem(QPainter& painter, const PageItem& item, bool hovered) const;
    void activateItem(const PageItem& item);
    void normalizeCurrent();
    void updatePaginationGeometry();

    int m_current = 1;
    int m_pageSize = 10;
    int m_total = 0;
    bool m_disabled = false;
    bool m_simple = false;
    bool m_showLessItems = false;
    bool m_showTotal = false;
    bool m_showQuickJumper = false;
    bool m_showSizeChanger = false;
    Ant::Size m_paginationSize = Ant::Size::Middle;
    int m_hoveredIndex = -1;
};
