#pragma once

#include <QPointer>
#include <QWidget>

#include "core/AntTypes.h"

class AntIcon;
class QVBoxLayout;
class QResizeEvent;
class QPaintEvent;

class AntResult : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::AlertType status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString subTitle READ subTitle WRITE setSubTitle NOTIFY subTitleChanged)
    Q_PROPERTY(bool iconVisible READ isIconVisible WRITE setIconVisible NOTIFY iconVisibleChanged)

public:
    explicit AntResult(QWidget* parent = nullptr);
    explicit AntResult(const QString& title, QWidget* parent = nullptr);

    Ant::AlertType status() const;
    void setStatus(Ant::AlertType status);

    QString title() const;
    void setTitle(const QString& title);

    QString subTitle() const;
    void setSubTitle(const QString& subTitle);

    bool isIconVisible() const;
    void setIconVisible(bool visible);

    QWidget* extraWidget() const;
    void setExtraWidget(QWidget* widget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void statusChanged(Ant::AlertType status);
    void titleChanged(const QString& title);
    void subTitleChanged(const QString& subTitle);
    void iconVisibleChanged(bool visible);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Metrics
    {
        int iconSize = 72;
        int padding = 32;
        int titleFontSize = 24;
        int subTitleFontSize = 14;
        int spacing = 12;
        int extraSpacing = 24;
    };

    Metrics metrics() const;
    QRect iconRect() const;
    QRect titleRect() const;
    QRect subTitleRect() const;
    QRect extraRect() const;
    void syncExtraGeometry();
    QColor iconColor() const;
    Ant::IconType iconTypeForStatus() const;

    Ant::AlertType m_status = Ant::AlertType::Info;
    QString m_title;
    QString m_subTitle;
    bool m_iconVisible = true;
    QPointer<QWidget> m_extraWidget;
};
