#pragma once

#include <QColor>
#include <QPainterPath>
#include <QTimer>
#include <QWidget>

#include "core/AntTypes.h"

class AntIcon : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::IconType iconType READ iconType WRITE setIconType NOTIFY iconTypeChanged)
    Q_PROPERTY(Ant::IconTheme iconTheme READ iconTheme WRITE setIconTheme NOTIFY iconThemeChanged)
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor twoToneColor READ twoToneColor WRITE setTwoToneColor NOTIFY twoToneColorChanged)
    Q_PROPERTY(int rotate READ rotate WRITE setRotate NOTIFY rotateChanged)
    Q_PROPERTY(bool spin READ isSpin WRITE setSpin NOTIFY spinChanged)

public:
    explicit AntIcon(QWidget* parent = nullptr);
    explicit AntIcon(Ant::IconType iconType, QWidget* parent = nullptr);

    Ant::IconType iconType() const;
    void setIconType(Ant::IconType iconType);

    Ant::IconTheme iconTheme() const;
    void setIconTheme(Ant::IconTheme iconTheme);

    int iconSize() const;
    void setIconSize(int iconSize);

    QColor color() const;
    void setColor(const QColor& color);

    QColor twoToneColor() const;
    void setTwoToneColor(const QColor& color);

    int rotate() const;
    void setRotate(int rotate);

    bool isSpin() const;
    void setSpin(bool spin);

    void setCustomPath(const QPainterPath& primaryPath, const QPainterPath& secondaryPath = QPainterPath());
    void clearCustomPath();
    bool hasCustomPath() const;
    QPainterPath customPrimaryPath() const;
    QPainterPath customSecondaryPath() const;
    int spinAngle() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    struct IconPaths
    {
        QPainterPath primary;
        QPainterPath secondary;
        bool useStroke = false;
    };

    static IconPaths builtinPaths(Ant::IconType type, Ant::IconTheme theme);
    static QPainterPath transformPath(const QPainterPath& path, const QRectF& targetRect);

Q_SIGNALS:
    void iconTypeChanged(Ant::IconType iconType);
    void iconThemeChanged(Ant::IconTheme iconTheme);
    void iconSizeChanged(int iconSize);
    void colorChanged(const QColor& color);
    void twoToneColorChanged(const QColor& color);
    void rotateChanged(int rotate);
    void spinChanged(bool spin);

protected:
    void paintEvent(QPaintEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    QColor effectivePrimaryColor() const;
    QColor effectiveSecondaryColor() const;
    void updateTimerState();

    Ant::IconType m_iconType = Ant::IconType::None;
    Ant::IconTheme m_iconTheme = Ant::IconTheme::Outlined;
    int m_iconSize = 14;
    QColor m_color;
    QColor m_twoToneColor;
    int m_rotate = 0;
    bool m_spin = false;
    int m_spinAngle = 0;
    bool m_hasCustomPath = false;
    QPainterPath m_customPrimaryPath;
    QPainterPath m_customSecondaryPath;
    QTimer m_spinTimer;
};
