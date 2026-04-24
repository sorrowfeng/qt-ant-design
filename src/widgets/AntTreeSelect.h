#pragma once

#include <QWidget>
#include <QVector>
#include "core/AntTypes.h"
#include "AntTree.h"

class AntTreeSelectStyle;

class AntTreeSelect : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString placeholder READ placeholder WRITE setPlaceholder NOTIFY placeholderChanged)
    Q_PROPERTY(bool allowClear READ allowClear WRITE setAllowClear NOTIFY allowClearChanged)
    Q_PROPERTY(bool multiple READ isMultiple WRITE setMultiple NOTIFY multipleChanged)
    Q_PROPERTY(bool treeCheckable READ isTreeCheckable WRITE setTreeCheckable NOTIFY treeCheckableChanged)
    Q_PROPERTY(bool showSearch READ isShowSearch WRITE setShowSearch NOTIFY showSearchChanged)
    Q_PROPERTY(Ant::SelectSize selectSize READ selectSize WRITE setSelectSize NOTIFY selectSizeChanged)
    Q_PROPERTY(Ant::SelectStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Ant::SelectVariant variant READ variant WRITE setVariant NOTIFY variantChanged)
    Q_PROPERTY(bool isOpen READ isOpen WRITE setOpen NOTIFY openChanged)

public:
    explicit AntTreeSelect(QWidget* parent = nullptr);
    ~AntTreeSelect() override = default;

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

    Ant::SelectSize selectSize() const;
    void setSelectSize(Ant::SelectSize size);

    Ant::SelectStatus status() const;
    void setStatus(Ant::SelectStatus status);

    Ant::SelectVariant variant() const;
    void setVariant(Ant::SelectVariant variant);

    bool isOpen() const;
    void setOpen(bool open);

    QString displayText() const;
    bool isHovered() const;

Q_SIGNALS:
    void valueChanged(const QStringList& keys);
    void openChanged(bool open);
    void placeholderChanged(const QString& text);
    void allowClearChanged(bool enable);
    void multipleChanged(bool enable);
    void treeCheckableChanged(bool enable);
    void showSearchChanged(bool enable);
    void selectSizeChanged(Ant::SelectSize size);
    void statusChanged(Ant::SelectStatus status);
    void variantChanged(Ant::SelectVariant variant);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    friend class AntTreeSelectStyle;
    class TreeSelectPopup;

    void showPopup();
    void hidePopup();
    void updateDisplayText();
    QString findNodeTitle(const QVector<AntTreeNode>& nodes, const QString& key) const;
    QStringList findNodeTitles(const QVector<AntTreeNode>& nodes, const QStringList& keys) const;
    QRect triggerRect() const;
    QRect clearButtonRect() const;
    QRect arrowRect() const;
    bool isOverClear(const QPoint& pos) const;

    QVector<AntTreeNode> m_treeData;
    QStringList m_value;
    QString m_placeholder;
    bool m_allowClear = false;
    bool m_multiple = false;
    bool m_treeCheckable = false;
    bool m_showSearch = false;
    Ant::SelectSize m_selectSize = Ant::SelectSize::Middle;
    Ant::SelectStatus m_status = Ant::SelectStatus::Normal;
    Ant::SelectVariant m_variant = Ant::SelectVariant::Outlined;
    bool m_open = false;
    bool m_hovered = false;
    QString m_displayText;
    TreeSelectPopup* m_popup = nullptr;
};
