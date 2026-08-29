#pragma once

#include "QtAntDesignExport.h"

#include <QObject>
#include <QColor>
#include <QHash>
#include <QString>
#include <QVariant>

#include "AntTypes.h"

struct AntThemeTokens
{
    QColor colorPrimary;
    QColor colorPrimaryHover;
    QColor colorPrimaryActive;
    QColor colorPrimaryBg;
    QColor colorPrimaryBorder;
    QColor colorPrimaryLoading;
    QColor colorPrimaryLoadingHover;
    QColor colorPrimaryLoadingActive;

    QColor colorSuccess;
    QColor colorSuccessHover;
    QColor colorSuccessActive;
    QColor colorSuccessBg;

    QColor colorWarning;
    QColor colorWarningHover;
    QColor colorWarningActive;
    QColor colorWarningBg;

    QColor colorError;
    QColor colorErrorHover;
    QColor colorErrorActive;
    QColor colorErrorBg;

    QColor colorLink;
    QColor colorLinkHover;
    QColor colorLinkActive;

    QColor colorText;
    QColor colorTextSecondary;
    QColor colorTextTertiary;
    QColor colorTextDisabled;
    QColor colorTextPlaceholder;
    QColor colorTextLightSolid;

    QColor colorBgBase;
    QColor colorBgLayout;
    QColor colorBgContainer;
    QColor colorBgElevated;
    QColor colorBgContainerDisabled;
    QColor colorFill;
    QColor colorFillSecondary;
    QColor colorFillTertiary;
    QColor colorFillQuaternary;

    QColor colorBorder;
    QColor colorBorderSecondary;
    QColor colorBorderDisabled;
    QColor colorSplit;
    QColor colorShadow;
    QColor colorRateStar;
    // Always-dark menu surface (MenuTheme::Dark is mode-independent).
    QColor colorBgMenuDark;
    // Typography <mark> highlight background (antd gold-2 in light, dark-gold in dark).
    QColor colorMarkBg;

    int fontSize = Ant::FontSize;
    int fontSizeSM = Ant::FontSizeSmall;
    int fontSizeLG = Ant::FontSizeLarge;
    int fontSizeXL = 20;
    int lineWidth = Ant::LineWidth;
    qreal lineHeight = 1.5715;

    int borderRadius = Ant::BorderRadius;
    int borderRadiusSM = 4;
    int borderRadiusLG = 8;
    int borderRadiusXS = 2;

    int sizeUnit = Ant::SizeUnit;
    int sizeStep = Ant::SizeStep;
    int paddingXXS = 4;
    int paddingXS = 8;
    int paddingSM = 12;
    int padding = 16;
    int paddingMD = 20;
    int paddingLG = 24;
    int paddingXL = 32;
    int marginXS = 8;
    int marginSM = 12;
    int margin = 16;
    int marginLG = 24;

    int controlHeight = Ant::ControlHeight;
    int controlHeightSM = Ant::ControlHeightSmall;
    int controlHeightLG = Ant::ControlHeightLarge;
    int controlOutlineWidth = 2;
    int lineWidthFocus = 3;
};

class QPainter;

class QT_ANT_DESIGN_EXPORT AntTheme : public QObject
{
    Q_OBJECT

public:
    static AntTheme* instance();

    Ant::ThemeMode themeMode() const;
    // Single source of truth for "is the global theme dark". All styles and
    // widgets must use this instead of re-comparing themeMode() inline.
    bool isDarkMode() const { return m_themeMode == Ant::ThemeMode::Dark; }
    const AntThemeTokens& tokens() const;
    const AntThemeTokens& tokens(Ant::ThemeMode mode) const;

    QColor primaryColor() const;
    QColor primaryHoverColor() const;
    QColor primaryActiveColor() const;
    QColor successColor() const;
    QColor warningColor() const;
    QColor errorColor() const;
    QColor linkColor() const;
    QColor textColor() const;
    QColor secondaryTextColor() const;
    QColor disabledTextColor() const;
    QColor placeholderTextColor() const;
    QColor backgroundColor() const;
    QColor layoutBackgroundColor() const;
    QColor containerColor() const;
    QColor elevatedColor() const;
    QColor borderColor() const;
    QColor secondaryBorderColor() const;
    QColor disabledBackgroundColor() const;

    int fontSize() const;
    int smallFontSize() const;
    int largeFontSize() const;
    int borderRadius() const;
    int smallBorderRadius() const;
    int largeBorderRadius() const;
    int spacingUnit() const;

    // Atomically apply the global theme configuration. An invalid primary
    // color restores the built-in color for each theme mode. Font size and
    // border radius are seed values; their related size tokens are derived
    // from them for both the light and dark token sets.
    void applyConfiguration(Ant::ThemeMode mode,
                            const QColor& primaryColor = QColor(),
                            int fontSize = Ant::FontSize,
                            int borderRadius = Ant::BorderRadius,
                            Ant::ThemeDensity density = Ant::ThemeDensity::Default);

    // 当前主题密度（default / compact，对应上游 theme algorithm）。
    Ant::ThemeDensity density() const { return m_densityOverride; }

    // ---- 组件级 token 覆盖（对应上游 theme.components.{Component}）----
    // 以 "组件名.token 名" 为键（如 "Button"/"borderRadius"），样式类在读取
    // 对应 token 时优先取覆盖值。设置或清空后触发 themeAboutToChange/themeChanged。
    void setComponentToken(const QString& component, const QString& token, const QVariant& value);
    QVariant componentToken(const QString& component, const QString& token) const;
    void clearComponentTokens(const QString& component = QString());
    QStringList componentTokenKeys() const;

    QColor hoverColor(const QColor& base) const;
    QColor activeColor(const QColor& base) const;
    QColor disabledColor(const QColor& foreground) const;

    void drawEffectShadow(QPainter* painter, const QRect& rect, int shadowWidth, int radius, qreal strength = 1.0) const;

public Q_SLOTS:
    void setThemeMode(Ant::ThemeMode mode);
    void toggleThemeMode();

Q_SIGNALS:
    // Emitted before any mode or token change. Internal styles use this to
    // cache geometry hints for both ordinary mode changes and ConfigProvider
    // token updates.
    void themeAboutToChange();
    void themeModeAboutToChange(Ant::ThemeMode mode);
    void themeModeChanged(Ant::ThemeMode mode);
    void themeChanged();

private:
    explicit AntTheme(QObject* parent = nullptr);

    static AntThemeTokens createTokens(Ant::ThemeMode mode);
    static void applyTokenOverrides(AntThemeTokens& tokens,
                                    Ant::ThemeMode mode,
                                    const QColor& primaryColor,
                                    int fontSize,
                                    int borderRadius,
                                    Ant::ThemeDensity density);
    void rebuildTokens();

    Ant::ThemeMode m_themeMode = Ant::ThemeMode::Default;
    QColor m_primaryColorOverride;
    int m_fontSizeOverride = Ant::FontSize;
    int m_borderRadiusOverride = Ant::BorderRadius;
    Ant::ThemeDensity m_densityOverride = Ant::ThemeDensity::Default;
    AntThemeTokens m_lightTokens;
    AntThemeTokens m_darkTokens;
    QHash<QString, QVariant> m_componentTokens;
};

#define antTheme AntTheme::instance()
