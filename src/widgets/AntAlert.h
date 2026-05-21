#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QHash>
#include <QPixmap>
#include <QPointer>
#include <QRect>
#include <QSize>
#include <QWidget>

#include "core/AntTypes.h"

class AntAlertStyle;
class AntIcon;
class QHBoxLayout;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QVBoxLayout;

class QT_ANT_DESIGN_EXPORT AntAlert : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(Ant::AlertType alertType READ alertType WRITE setAlertType NOTIFY alertTypeChanged)
    Q_PROPERTY(bool showIcon READ showIcon WRITE setShowIcon NOTIFY showIconChanged)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable NOTIFY closableChanged)
    Q_PROPERTY(bool banner READ isBanner WRITE setBanner NOTIFY bannerChanged)

public:
    explicit AntAlert(QWidget* parent = nullptr);
    explicit AntAlert(const QString& title, QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    QString description() const;
    void setDescription(const QString& description);

    Ant::AlertType alertType() const;
    void setAlertType(Ant::AlertType type);

    bool showIcon() const;
    void setShowIcon(bool showIcon);

    bool isClosable() const;
    void setClosable(bool closable);

    bool isBanner() const;
    void setBanner(bool banner);

    QWidget* actionWidget() const;
    void setActionWidget(QWidget* widget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void descriptionChanged(const QString& description);
    void alertTypeChanged(Ant::AlertType type);
    void showIconChanged(bool showIcon);
    void closableChanged(bool closable);
    void bannerChanged(bool banner);
    void closeRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    friend class AntAlertStyle;

    struct Metrics
    {
        int minHeight = 40;
        int radius = 8;
        int paddingX = 16;
        int paddingY = 10;
        int iconSize = 16;
        int titleFontSize = 14;
        int descFontSize = 14;
        int closeSize = 20;
        int actionSpacing = 12;
    };

    struct AlertLayout
    {
        bool valid = false;
        QSize widgetSize;
        QString title;
        QString description;
        Ant::AlertType alertType = Ant::AlertType::Info;
        bool showIcon = false;
        bool closable = false;
        bool banner = false;
        QSize actionSize;
        Metrics metrics;
        QRect bodyRect;
        QRect contentRect;
        QRect closeRect;
        QRect actionRect;
        QRect iconRect;
        QRect titleRect;
        QRect descriptionRect;
        QSize sizeHint;
        QSize minimumSizeHint;
        bool hasDescription = false;
        bool showIconEffective = false;
        int textLeft = 0;
        int textRight = 0;
    };

    Metrics metrics() const;
    QColor backgroundColor() const;
    QColor borderColor() const;
    QColor iconColor() const;
    QColor titleColor() const;
    QColor descriptionColor() const;
    QRect closeRect() const;
    QRect actionRect() const;
    QRect contentRect() const;
    const AlertLayout& alertLayout() const;
    void invalidateAlertLayout() const;
    QPixmap cachedIconPixmap(Ant::IconType iconType, const QColor& color, int iconSize) const;
    void clearIconPixmapCache() const;
    void requestAlertUpdate(const QRect& region, const QString& mode, bool closeScoped = false);
    void syncAlertPerfCounters() const;
    void syncLayout();
    Ant::IconType iconTypeForAlert() const;

    QString m_title;
    QString m_description;
    Ant::AlertType m_alertType = Ant::AlertType::Info;
    bool m_showIcon = false;
    bool m_closable = false;
    bool m_banner = false;
    bool m_hoverClose = false;
    QPointer<QWidget> m_actionWidget;
    mutable AlertLayout m_layoutCache;
    mutable QHash<QString, QPixmap> m_iconPixmapCache;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    mutable int m_iconPixmapBuildCount = 0;
    mutable int m_iconPixmapCacheHitCount = 0;
    mutable int m_regionUpdateCount = 0;
    mutable int m_closeRegionUpdateCount = 0;
    QString m_lastUpdateMode;
};
