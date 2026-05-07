#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QPainterPath>
#include <QPixmap>
#include <QWidget>

#include "core/AntTypes.h"

class QPaintEvent;
class QResizeEvent;
class AntAvatar;

class QT_ANT_DESIGN_EXPORT AntAvatarGroup : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int maxCount READ maxCount WRITE setMaxCount NOTIFY maxCountChanged)
    Q_PROPERTY(Ant::Size avatarSize READ avatarSize WRITE setAvatarSize NOTIFY avatarSizeChanged)

public:
    explicit AntAvatarGroup(QWidget* parent = nullptr);

    int maxCount() const;
    void setMaxCount(int maxCount);

    Ant::Size avatarSize() const;
    void setAvatarSize(Ant::Size size);

    void addAvatar(AntAvatar* avatar);
    void removeAvatar(AntAvatar* avatar);
    QList<AntAvatar*> avatars() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void maxCountChanged(int maxCount);
    void avatarSizeChanged(Ant::Size size);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void relayout();

    int m_maxCount = 0; // 0 = no limit
    Ant::Size m_avatarSize = Ant::Size::Middle;
    QList<AntAvatar*> m_avatars;
    AntAvatar* m_overflowAvatar = nullptr;
};

class QT_ANT_DESIGN_EXPORT AntAvatar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString iconText READ iconText WRITE setIconText NOTIFY iconTextChanged)
    Q_PROPERTY(QString imagePath READ imagePath WRITE setImagePath NOTIFY imagePathChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(int gap READ gap WRITE setGap NOTIFY gapChanged)
    Q_PROPERTY(int customSize READ customSize WRITE setCustomSize NOTIFY customSizeChanged)
    Q_PROPERTY(Ant::AvatarShape shape READ shape WRITE setShape NOTIFY shapeChanged)
    Q_PROPERTY(Ant::Size avatarSize READ avatarSize WRITE setAvatarSize NOTIFY avatarSizeChanged)

public:
    explicit AntAvatar(QWidget* parent = nullptr);
    explicit AntAvatar(const QString& text, QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& text);

    QString iconText() const;
    void setIconText(const QString& iconText);

    QString imagePath() const;
    void setImagePath(const QString& imagePath);

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor& color);

    int gap() const;
    void setGap(int gap);

    int customSize() const;
    void setCustomSize(int size);

    Ant::AvatarShape shape() const;
    void setShape(Ant::AvatarShape shape);

    Ant::Size avatarSize() const;
    void setAvatarSize(Ant::Size size);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void textChanged(const QString& text);
    void iconTextChanged(const QString& iconText);
    void imagePathChanged(const QString& imagePath);
    void backgroundColorChanged(const QColor& color);
    void gapChanged(int gap);
    void customSizeChanged(int size);
    void shapeChanged(Ant::AvatarShape shape);
    void avatarSizeChanged(Ant::Size size);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int avatarExtent() const;
    int textFontSize() const;
    int iconFontSize() const;
    QPainterPath clipPath(const QRectF& rect) const;
    QRectF imageSourceRect(const QPixmap& pixmap, const QSizeF& targetSize) const;

    QString m_text;
    QString m_iconText;
    QString m_imagePath;
    QColor m_backgroundColor;
    QPixmap m_pixmap;
    int m_gap = 4;
    int m_customSize = 0;
    Ant::AvatarShape m_shape = Ant::AvatarShape::Circle;
    Ant::Size m_avatarSize = Ant::Size::Middle;
};
