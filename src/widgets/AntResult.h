#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QFont>
#include <QPixmap>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QWidget>

#include "core/AntTypes.h"

class AntResultStyle;
class QResizeEvent;
class QPaintEvent;

class QT_ANT_DESIGN_EXPORT AntResult : public QWidget
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
    friend class AntResultStyle;

    struct Metrics
    {
        int iconSize = 72;
        int padding = 32;
        int titleFontSize = 24;
        int subTitleFontSize = 14;
        int spacing = 12;
        int extraSpacing = 24;
    };

    struct ResultLayoutCache
    {
        bool valid = false;
        QSize widgetSize;
        QFont font;
        Ant::ThemeMode themeMode = Ant::ThemeMode::Default;
        int tokenFontSize = 0;
        QString title;
        QString subTitle;
        Ant::AlertType status = Ant::AlertType::Info;
        bool iconVisible = true;
        bool hasExtraWidget = false;
        QSize extraSize;
        Metrics metrics;
        QSize sizeHint;
        QSize minimumSizeHint;
        QRect iconRect;
        QRect titleRect;
        QRect subTitleRect;
        QRect extraRect;
        QColor iconColor;
        QColor titleColor;
        QColor subTitleColor;
        QColor iconSecondaryColor;
        Ant::IconType iconType = Ant::IconType::InfoCircle;
    };

    struct ResultIconPixmapCache
    {
        bool valid = false;
        qreal devicePixelRatio = 1.0;
        QSize logicalSize;
        Ant::AlertType status = Ant::AlertType::Info;
        Ant::ThemeMode themeMode = Ant::ThemeMode::Default;
        QColor iconColor;
        QColor secondaryColor;
        Ant::IconType iconType = Ant::IconType::InfoCircle;
        QPixmap pixmap;
    };

    Metrics metrics() const;
    const ResultLayoutCache& resultLayout() const;
    QPixmap statusIconPixmap(qreal devicePixelRatio) const;
    void invalidateResultLayout() const;
    void invalidateResultIconPixmap() const;
    void requestResultUpdate(const QRect& region, const QString& mode);
    void syncResultPerfCounters() const;
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
    mutable ResultLayoutCache m_layoutCache;
    mutable ResultIconPixmapCache m_iconPixmapCache;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    mutable int m_iconPixmapBuildCount = 0;
    mutable int m_iconPixmapCacheHitCount = 0;
    int m_extraGeometryApplyCount = 0;
    int m_extraGeometrySkipCount = 0;
    int m_regionUpdateCount = 0;
    QString m_lastUpdateMode;
};
