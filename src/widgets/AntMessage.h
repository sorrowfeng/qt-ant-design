#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QEvent>
#include <QList>
#include <QPixmap>
#include <QPointer>
#include <QRectF>
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

class AntMessageStyle;

class QT_ANT_DESIGN_EXPORT AntMessage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(Ant::MessageType messageType READ messageType WRITE setMessageType NOTIFY messageTypeChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(bool pauseOnHover READ pauseOnHover WRITE setPauseOnHover NOTIFY pauseOnHoverChanged)

public:
    explicit AntMessage(QWidget* parent = nullptr);
    ~AntMessage() override;

    static AntMessage* open(const QString& text,
                            Ant::MessageType type = Ant::MessageType::Info,
                            QWidget* anchor = nullptr,
                            int durationMs = 3000,
                            Ant::Placement placement = Ant::Placement::Top);
    static AntMessage* info(const QString& text, QWidget* anchor = nullptr, int durationMs = 3000, Ant::Placement placement = Ant::Placement::Top);
    static AntMessage* success(const QString& text, QWidget* anchor = nullptr, int durationMs = 3000, Ant::Placement placement = Ant::Placement::Top);
    static AntMessage* warning(const QString& text, QWidget* anchor = nullptr, int durationMs = 3000, Ant::Placement placement = Ant::Placement::Top);
    static AntMessage* error(const QString& text, QWidget* anchor = nullptr, int durationMs = 3000, Ant::Placement placement = Ant::Placement::Top);
    static AntMessage* loading(const QString& text, QWidget* anchor = nullptr, int durationMs = 0, Ant::Placement placement = Ant::Placement::Top);

    QString text() const;
    void setText(const QString& text);

    Ant::MessageType messageType() const;
    void setMessageType(Ant::MessageType type);

    int duration() const;
    void setDuration(int durationMs);

    bool pauseOnHover() const;
    void setPauseOnHover(bool pause);

    int loadingAngle() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void textChanged(const QString& text);
    void messageTypeChanged(Ant::MessageType type);
    void durationChanged(int durationMs);
    void pauseOnHoverChanged(bool pause);
    void closed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void enterEvent(AntEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    friend class AntMessageStyle;

    struct MessageLayout
    {
        bool valid = false;
        QSize widgetSize;
        QString text;
        Ant::MessageType messageType = Ant::MessageType::Info;
        Ant::ThemeMode themeMode = Ant::ThemeMode::Default;
        QString fontKey;
        QSize sizeHint;
        QSize minimumSizeHint;
        QRectF bubbleRect;
        QRectF iconRect;
        QRectF textRect;
        QColor accentColor;
        int radius = 8;
    };

    static QList<AntMessage*>& activeMessages();
    static void relayoutMessages(QWidget* anchor = nullptr);

    Ant::Placement m_placement = Ant::Placement::Top;

    const MessageLayout& messageLayout() const;
    void invalidateMessageLayout() const;
    QPixmap cachedShadowPixmap() const;
    void clearShadowCache() const;
    void applyMessageSizeHint();
    void requestMessageUpdate(const QRect& region, const QString& mode, bool spinnerScoped = false);
    void syncMessagePerfCounters() const;
    QColor accentColor() const;
    void startTimers();
    void updateLoadingState();
    void drawLoadingIcon(QPainter& painter, const QRectF& rect) const;
    void installAnchorWatcher(QWidget* anchor);
    void uninstallAnchorWatcher();
    bool anchorReady() const;
    void handleAnchorChanged(QEvent::Type type);

    QString m_text;
    Ant::MessageType m_messageType = Ant::MessageType::Info;
    int m_duration = 3000;
    bool m_pauseOnHover = true;
    bool m_hovered = false;
    int m_loadingAngle = 0;
    QPointer<QWidget> m_anchor;
    QPointer<QWidget> m_anchorWindow;
    QTimer* m_closeTimer = nullptr;
    QTimer* m_loadingTimer = nullptr;
    mutable MessageLayout m_layoutCache;
    mutable QPixmap m_shadowPixmapCache;
    mutable QString m_shadowPixmapCacheKey;
    mutable int m_layoutBuildCount = 0;
    mutable int m_layoutCacheHitCount = 0;
    mutable int m_shadowBuildCount = 0;
    mutable int m_shadowCacheHitCount = 0;
    mutable int m_sizeApplyCount = 0;
    mutable int m_sizeSkipCount = 0;
    int m_regionUpdateCount = 0;
    int m_spinnerRegionUpdateCount = 0;
    int m_relayoutMoveCount = 0;
    int m_relayoutSkipCount = 0;
    QString m_lastUpdateMode;
};
