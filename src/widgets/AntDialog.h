#pragma once

#include "core/QtAntDesignExport.h"

#include <QByteArray>
#include <QDialog>
#include <QRect>
#include <QSize>

#include "core/AntTypes.h"

struct AntThemeTokens;
class QEvent;
class QHideEvent;
class QMouseEvent;
class QMoveEvent;
class QResizeEvent;
class QShowEvent;
class QVBoxLayout;
class QWidget;

class QT_ANT_DESIGN_EXPORT AntDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool titleBarVisible READ isTitleBarVisible WRITE setTitleBarVisible NOTIFY titleBarVisibleChanged)
    Q_PROPERTY(bool closeButtonVisible READ isCloseButtonVisible WRITE setCloseButtonVisible NOTIFY closeButtonVisibleChanged)
    Q_PROPERTY(int titleBarHeight READ titleBarHeight WRITE setTitleBarHeight NOTIFY titleBarHeightChanged)
    Q_PROPERTY(int cornerRadius READ cornerRadius WRITE setCornerRadius NOTIFY cornerRadiusChanged)

public:
    enum class TitleBarButton
    {
        None,
        Close,
    };
    Q_ENUM(TitleBarButton)

    explicit AntDialog(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::Dialog);
    ~AntDialog() override = default;

    static constexpr int TitleBarHeight = 40;
    static constexpr int TitleBarButtonWidth = 46;

    const AntThemeTokens& tokens() const;
    Ant::ThemeMode currentTheme() const;

    QWidget* contentWidget() const;
    void setContentWidget(QWidget* widget);

    bool isTitleBarVisible() const;
    void setTitleBarVisible(bool visible);

    bool isCloseButtonVisible() const;
    void setCloseButtonVisible(bool visible);

    int titleBarHeight() const;
    void setTitleBarHeight(int height);

    int cornerRadius() const;
    void setCornerRadius(int radius);
    bool usesRoundedCorners() const;
    bool usesLegacyOpaquePath() const;

    QRect titleBarRect() const;
    QRect titleBarTextRect() const;
    QRect titleBarCloseButtonRect() const;
    TitleBarButton hoveredTitleBarButton() const;
    TitleBarButton pressedTitleBarButton() const;

    virtual void refreshAntStyle();

Q_SIGNALS:
    void titleBarVisibleChanged(bool visible);
    void closeButtonVisibleChanged(bool visible);
    void titleBarHeightChanged(int height);
    void cornerRadiusChanged(int radius);

protected:
    virtual void onThemeChanged(Ant::ThemeMode mode);

    bool event(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool nativeEvent(const QByteArray& eventType, void* message, AntNativeEventResult* result) override;

private:
    friend class AntDialogStyle;

    void initializeAntStyle();
    void updateRoundedCornerPolicy();
    int effectiveCornerRadius() const;
    int shadowMargin() const;
    QRect surfaceRect() const;
    void updateChromeMargins();
    void applyNativeWindowFrame();
    void updateLegacySoftwareShadow();
    void hideLegacySoftwareShadow();
    void handleThemeChanged(Ant::ThemeMode mode);
    bool refreshCachedHints();
    void refreshThemeCache();
    void scheduleChildSync();
    void syncChildControls();
    void applyDialogPalette(QWidget* widget);
    void updateTitleBarHover(const QPoint& pos);
    void setHoveredTitleBarButton(TitleBarButton button);
    void clearTitleBarHover();
    void clearTitleBarPress();
    void syncDialogPerfCounters() const;

    QVBoxLayout* m_rootLayout = nullptr;
    QWidget* m_contentWidget = nullptr;
    bool m_titleBarVisible = true;
    bool m_closeButtonVisible = true;
    int m_titleBarHeight = TitleBarHeight;
    int m_cornerRadius = 8;
    bool m_useRoundedCorners = true;
    TitleBarButton m_hoveredButton = TitleBarButton::None;
    TitleBarButton m_pressedButton = TitleBarButton::None;
    bool m_dragging = false;
    QPoint m_dragStartGlobalPos;
    QPoint m_dragStartDialogPos;

    QStyle* m_cachedStyle = nullptr;
    QSize m_cachedSizeHint;
    QSize m_cachedMinimumSizeHint;
    qint64 m_cachedPaletteKey = 0;
    QWidget* m_legacySoftwareShadow = nullptr;
    bool m_childSyncQueued = false;
    int m_childSyncCount = 0;
    int m_themeChangeCount = 0;
    int m_repolishCount = 0;
    int m_updateGeometryCount = 0;
    int m_surfaceUpdateCount = 0;
};
