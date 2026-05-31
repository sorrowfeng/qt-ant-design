#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QIcon>
#include <QImage>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QWidget>

#include "core/AntTypes.h"

class AntIcon;
class QEnterEvent;
class QEvent;
class QHBoxLayout;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;

class QT_ANT_DESIGN_EXPORT AntNavItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

public:
    explicit AntNavItem(const QString& text, QWidget* parent = nullptr);

    bool isActive() const;
    void setActive(bool active);

    QString text() const;
    void setText(const QString& text);

    void setIcon(const QIcon& icon);
    QIcon icon() const;
    void setIcon(Ant::IconType iconType, Ant::IconTheme theme = Ant::IconTheme::Outlined);
    Ant::IconType iconType() const;
    void setIconName(const QString& iconName, Ant::IconTheme theme = Ant::IconTheme::Outlined);
    QString iconName() const;
    void setIconTheme(Ant::IconTheme theme);
    Ant::IconTheme iconTheme() const;
    void setIconColor(const QColor& color);
    QColor iconColor() const;
    void setIconTwoToneColor(const QColor& color);
    QColor iconTwoToneColor() const;
    void setIconPixmap(const QPixmap& pixmap);
    QPixmap iconPixmap() const;
    void setIconImage(const QImage& image);
    QImage iconImage() const;
    void setIconSize(const QSize& size);
    QSize iconSize() const;
    bool hasIcon() const;
    void clearIcon();

Q_SIGNALS:
    void activeChanged(bool active);
    void textChanged(const QString& text);
    void iconChanged();
    void iconSizeChanged(const QSize& size);
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(AntEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    enum class IconSource
    {
        None,
        QtIcon,
        AntIconType,
        AntIconName,
        Pixmap,
    };

    void syncVisualState();
    void syncIconWidgets();
    bool usesAntIconWidget() const;
    bool usesImageLabel() const;
    QSize effectiveIconSize() const;
    QPixmap scaledPaintedIconPixmap() const;
    QColor resolvedIconColor() const;
    void invalidatePaintCache() const;
    void ensurePaintCache() const;
    void syncNavPerfCounters() const;

    QHBoxLayout* m_layout = nullptr;
    AntIcon* m_iconWidget = nullptr;
    QLabel* m_imageLabel = nullptr;
    QLabel* m_label = nullptr;
    QIcon m_icon;
    IconSource m_iconSource = IconSource::None;
    Ant::IconType m_iconType = Ant::IconType::None;
    QString m_iconName;
    Ant::IconTheme m_iconTheme = Ant::IconTheme::Outlined;
    QColor m_iconColor;
    QColor m_iconTwoToneColor;
    QPixmap m_iconPixmap;
    QSize m_iconSize = QSize(16, 16);
    bool m_active = false;
    bool m_hovered = false;
    QColor m_cachedLabelColor;
    int m_cachedLabelWeight = -1;
    int m_visualStateApplyCount = 0;
    mutable bool m_paintCacheValid = false;
    mutable QSize m_cachedPaintSize;
    mutable bool m_cachedPaintActive = false;
    mutable bool m_cachedPaintHovered = false;
    mutable QColor m_cachedBackground;
    mutable QColor m_cachedIndicatorColor;
    mutable QRect m_cachedIndicatorRect;
    mutable int m_paintCacheBuildCount = 0;
};
