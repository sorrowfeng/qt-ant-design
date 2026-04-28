#pragma once

#include <QObject>
#include <QColor>

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

class AntTheme : public QObject
{
    Q_OBJECT

public:
    static AntTheme* instance();

    Ant::ThemeMode themeMode() const;
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

    QColor hoverColor(const QColor& base) const;
    QColor activeColor(const QColor& base) const;
    QColor disabledColor(const QColor& foreground) const;

    void drawEffectShadow(QPainter* painter, const QRect& rect, int shadowWidth, int radius, qreal strength = 1.0) const;

public Q_SLOTS:
    void setThemeMode(Ant::ThemeMode mode);
    void toggleThemeMode();

Q_SIGNALS:
    void themeModeChanged(Ant::ThemeMode mode);
    void themeChanged();

private:
    explicit AntTheme(QObject* parent = nullptr);

    static AntThemeTokens createTokens(Ant::ThemeMode mode);

    Ant::ThemeMode m_themeMode = Ant::ThemeMode::Default;
    AntThemeTokens m_lightTokens;
    AntThemeTokens m_darkTokens;
};

#define antTheme AntTheme::instance()
