#pragma once

#include <QElapsedTimer>
#include <QList>
#include <QWidget>

#include "core/AntTypes.h"

class QEnterEvent;
class QEvent;
class QHideEvent;
class QMouseEvent;
class QPaintEvent;
class QPainter;
class QShowEvent;
class QTimer;

class AntNotification : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(Ant::MessageType notificationType READ notificationType WRITE setNotificationType NOTIFY notificationTypeChanged)
    Q_PROPERTY(Ant::NotificationPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(bool pauseOnHover READ pauseOnHover WRITE setPauseOnHover NOTIFY pauseOnHoverChanged)
    Q_PROPERTY(bool showProgress READ showProgress WRITE setShowProgress NOTIFY showProgressChanged)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable NOTIFY closableChanged)

public:
    explicit AntNotification(QWidget* parent = nullptr);

    static AntNotification* open(const QString& title,
                                 const QString& description,
                                 Ant::MessageType type = Ant::MessageType::Info,
                                 QWidget* anchor = nullptr,
                                 int durationMs = 4500,
                                 Ant::NotificationPlacement placement = Ant::NotificationPlacement::TopRight);
    static AntNotification* info(const QString& title,
                                 const QString& description,
                                 QWidget* anchor = nullptr,
                                 int durationMs = 4500,
                                 Ant::NotificationPlacement placement = Ant::NotificationPlacement::TopRight);
    static AntNotification* success(const QString& title,
                                    const QString& description,
                                    QWidget* anchor = nullptr,
                                    int durationMs = 4500,
                                    Ant::NotificationPlacement placement = Ant::NotificationPlacement::TopRight);
    static AntNotification* warning(const QString& title,
                                    const QString& description,
                                    QWidget* anchor = nullptr,
                                    int durationMs = 4500,
                                    Ant::NotificationPlacement placement = Ant::NotificationPlacement::TopRight);
    static AntNotification* error(const QString& title,
                                  const QString& description,
                                  QWidget* anchor = nullptr,
                                  int durationMs = 4500,
                                  Ant::NotificationPlacement placement = Ant::NotificationPlacement::TopRight);
    static void closeAll();

    QString title() const;
    void setTitle(const QString& title);

    QString description() const;
    void setDescription(const QString& description);

    Ant::MessageType notificationType() const;
    void setNotificationType(Ant::MessageType type);

    Ant::NotificationPlacement placement() const;
    void setPlacement(Ant::NotificationPlacement placement);

    int duration() const;
    void setDuration(int durationMs);

    bool pauseOnHover() const;
    void setPauseOnHover(bool pause);

    bool showProgress() const;
    void setShowProgress(bool show);

    bool isClosable() const;
    void setClosable(bool closable);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void descriptionChanged(const QString& description);
    void notificationTypeChanged(Ant::MessageType type);
    void placementChanged(Ant::NotificationPlacement placement);
    void durationChanged(int durationMs);
    void pauseOnHoverChanged(bool pause);
    void showProgressChanged(bool show);
    void closableChanged(bool closable);
    void clicked();
    void closed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    static QList<AntNotification*>& activeNotifications();
    static void relayoutNotifications(QWidget* anchor = nullptr);

    QRectF noticeRect() const;
    QRectF closeButtonRect() const;
    QColor accentColor() const;
    qreal progressRatio() const;
    void startCloseTimer();
    void pauseCloseTimer();
    void resumeCloseTimer();
    void updateSpinnerState();
    void drawTypeIcon(QPainter& painter, const QRectF& rect) const;
    void drawSpinner(QPainter& painter, const QRectF& rect) const;

    QString m_title;
    QString m_description;
    Ant::MessageType m_notificationType = Ant::MessageType::Info;
    Ant::NotificationPlacement m_placement = Ant::NotificationPlacement::TopRight;
    int m_duration = 4500;
    int m_remainingMs = 4500;
    bool m_pauseOnHover = true;
    bool m_showProgress = false;
    bool m_closable = true;
    bool m_hovered = false;
    bool m_closeHovered = false;
    int m_spinnerAngle = 0;
    QWidget* m_anchor = nullptr;
    QTimer* m_closeTimer = nullptr;
    QTimer* m_progressTimer = nullptr;
    QTimer* m_spinnerTimer = nullptr;
    QElapsedTimer m_countdown;
};
