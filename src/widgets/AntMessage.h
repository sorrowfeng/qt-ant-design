#pragma once

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

class AntMessage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(Ant::MessageType messageType READ messageType WRITE setMessageType NOTIFY messageTypeChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(bool pauseOnHover READ pauseOnHover WRITE setPauseOnHover NOTIFY pauseOnHoverChanged)

public:
    explicit AntMessage(QWidget* parent = nullptr);

    static AntMessage* open(const QString& text,
                            Ant::MessageType type = Ant::MessageType::Info,
                            QWidget* anchor = nullptr,
                            int durationMs = 3000);
    static AntMessage* info(const QString& text, QWidget* anchor = nullptr, int durationMs = 3000);
    static AntMessage* success(const QString& text, QWidget* anchor = nullptr, int durationMs = 3000);
    static AntMessage* warning(const QString& text, QWidget* anchor = nullptr, int durationMs = 3000);
    static AntMessage* error(const QString& text, QWidget* anchor = nullptr, int durationMs = 3000);
    static AntMessage* loading(const QString& text, QWidget* anchor = nullptr, int durationMs = 0);

    QString text() const;
    void setText(const QString& text);

    Ant::MessageType messageType() const;
    void setMessageType(Ant::MessageType type);

    int duration() const;
    void setDuration(int durationMs);

    bool pauseOnHover() const;
    void setPauseOnHover(bool pause);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void textChanged(const QString& text);
    void messageTypeChanged(Ant::MessageType type);
    void durationChanged(int durationMs);
    void pauseOnHoverChanged(bool pause);
    void closed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    static QList<AntMessage*>& activeMessages();
    static void relayoutMessages(QWidget* anchor = nullptr);

    QColor accentColor() const;
    QString iconText() const;
    void startTimers();
    void updateLoadingState();
    void drawLoadingIcon(QPainter& painter, const QRectF& rect) const;

    QString m_text;
    Ant::MessageType m_messageType = Ant::MessageType::Info;
    int m_duration = 3000;
    bool m_pauseOnHover = true;
    bool m_hovered = false;
    int m_loadingAngle = 0;
    QWidget* m_anchor = nullptr;
    QTimer* m_closeTimer = nullptr;
    QTimer* m_loadingTimer = nullptr;
};
