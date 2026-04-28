#pragma once

#include <QPointer>
#include <QWidget>

class QLabel;
class QGridLayout;
class QVBoxLayout;

class AntDescriptionsItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(int span READ span WRITE setSpan NOTIFY spanChanged)

public:
    explicit AntDescriptionsItem(QWidget* parent = nullptr);
    explicit AntDescriptionsItem(const QString& label, const QString& content = QString(), QWidget* parent = nullptr);

    QString label() const;
    void setLabel(const QString& label);

    QString content() const;
    void setContent(const QString& content);

    int span() const;
    void setSpan(int span);

    QWidget* contentWidget() const;
    void setContentWidget(QWidget* widget);

Q_SIGNALS:
    void labelChanged(const QString& label);
    void contentChanged(const QString& content);
    void spanChanged(int span);

private:
    QString m_label;
    QString m_content;
    int m_span = 1;
    QPointer<QWidget> m_contentWidget;
};

class AntDescriptions : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString extra READ extra WRITE setExtra NOTIFY extraChanged)
    Q_PROPERTY(int columnCount READ columnCount WRITE setColumnCount NOTIFY columnCountChanged)
    Q_PROPERTY(bool bordered READ isBordered WRITE setBordered NOTIFY borderedChanged)
    Q_PROPERTY(bool vertical READ isVertical WRITE setVertical NOTIFY verticalChanged)

public:
    explicit AntDescriptions(QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    QString extra() const;
    void setExtra(const QString& extra);

    int columnCount() const;
    void setColumnCount(int count);

    bool isBordered() const;
    void setBordered(bool bordered);

    bool isVertical() const;
    void setVertical(bool vertical);

    QList<AntDescriptionsItem*> items() const;
    void addItem(AntDescriptionsItem* item);
    AntDescriptionsItem* addItem(const QString& label, const QString& content, int span = 1);
    void clearItems();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void extraChanged(const QString& extra);
    void columnCountChanged(int count);
    void borderedChanged(bool bordered);
    void verticalChanged(bool vertical);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void rebuildLayout();
    void rebuildGrid();
    void updateTheme();
    QWidget* buildLabelCell(const QString& text);
    QWidget* buildContentCell(AntDescriptionsItem* item);

    QString m_title;
    QString m_extra;
    int m_columnCount = 3;
    bool m_bordered = false;
    bool m_vertical = false;
    QWidget* m_header = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_extraLabel = nullptr;
    QWidget* m_body = nullptr;
    QGridLayout* m_grid = nullptr;
    QVBoxLayout* m_root = nullptr;
    QList<AntDescriptionsItem*> m_items;
};
