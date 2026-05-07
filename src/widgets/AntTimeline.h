#pragma once

#include "core/QtAntDesignExport.h"

#include <QVector>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;

struct AntTimelineItem
{
    QString title;
    QString content;
    QString color;        // "blue", "red", "green", "gray", or custom hex
    bool loading = false;
};

class QT_ANT_DESIGN_EXPORT AntTimeline : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::TimelineMode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(Ant::TimelineOrientation orientation READ orientation WRITE setOrientation NOTIFY orientationChanged)
    Q_PROPERTY(Ant::TimelineDotVariant dotVariant READ dotVariant WRITE setDotVariant NOTIFY dotVariantChanged)
    Q_PROPERTY(bool reverse READ isReverse WRITE setReverse NOTIFY reverseChanged)

public:
    explicit AntTimeline(QWidget* parent = nullptr);

    Ant::TimelineMode mode() const;
    void setMode(Ant::TimelineMode mode);

    Ant::TimelineOrientation orientation() const;
    void setOrientation(Ant::TimelineOrientation orientation);

    Ant::TimelineDotVariant dotVariant() const;
    void setDotVariant(Ant::TimelineDotVariant variant);

    bool isReverse() const;
    void setReverse(bool reverse);

    int count() const;
    AntTimelineItem itemAt(int index) const;
    void addItem(const AntTimelineItem& item);
    void addItem(const QString& title, const QString& content = QString(),
                 const QString& color = QStringLiteral("blue"), bool loading = false);
    void insertItem(int index, const AntTimelineItem& item);
    void removeItem(int index);
    void clearItems();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void modeChanged(Ant::TimelineMode mode);
    void orientationChanged(Ant::TimelineOrientation orientation);
    void dotVariantChanged(Ant::TimelineDotVariant variant);
    void reverseChanged(bool reverse);
    void itemClicked(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QVector<AntTimelineItem> m_items;
    Ant::TimelineMode m_mode = Ant::TimelineMode::Start;
    Ant::TimelineOrientation m_orientation = Ant::TimelineOrientation::Vertical;
    Ant::TimelineDotVariant m_dotVariant = Ant::TimelineDotVariant::Outlined;
    bool m_reverse = false;
};
