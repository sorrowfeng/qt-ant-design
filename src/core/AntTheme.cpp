#include "AntTheme.h"

#include <QPainter>
#include <QPainterPath>

#include "styles/AntPalette.h"

namespace
{
QColor blendOnBackground(const QColor& foreground, const QColor& background, qreal opacity)
{
    return AntPalette::mix(background, foreground, opacity);
}

QColor tokenShadow(Ant::ThemeMode mode)
{
    return mode == Ant::ThemeMode::Dark ? QColor(255, 255, 255, 51) : QColor(0, 0, 0);
}
}

AntTheme* AntTheme::instance()
{
    static AntTheme theme;
    return &theme;
}

AntTheme::AntTheme(QObject* parent)
    : QObject(parent),
      m_lightTokens(createTokens(Ant::ThemeMode::Default)),
      m_darkTokens(createTokens(Ant::ThemeMode::Dark))
{
}

Ant::ThemeMode AntTheme::themeMode() const
{
    return m_themeMode;
}

const AntThemeTokens& AntTheme::tokens() const
{
    return tokens(m_themeMode);
}

const AntThemeTokens& AntTheme::tokens(Ant::ThemeMode mode) const
{
    return mode == Ant::ThemeMode::Dark ? m_darkTokens : m_lightTokens;
}

QColor AntTheme::primaryColor() const { return tokens().colorPrimary; }
QColor AntTheme::primaryHoverColor() const { return tokens().colorPrimaryHover; }
QColor AntTheme::primaryActiveColor() const { return tokens().colorPrimaryActive; }
QColor AntTheme::successColor() const { return tokens().colorSuccess; }
QColor AntTheme::warningColor() const { return tokens().colorWarning; }
QColor AntTheme::errorColor() const { return tokens().colorError; }
QColor AntTheme::linkColor() const { return tokens().colorLink; }
QColor AntTheme::textColor() const { return tokens().colorText; }
QColor AntTheme::secondaryTextColor() const { return tokens().colorTextSecondary; }
QColor AntTheme::disabledTextColor() const { return tokens().colorTextDisabled; }
QColor AntTheme::placeholderTextColor() const { return tokens().colorTextPlaceholder; }
QColor AntTheme::backgroundColor() const { return tokens().colorBgBase; }
QColor AntTheme::layoutBackgroundColor() const { return tokens().colorBgLayout; }
QColor AntTheme::containerColor() const { return tokens().colorBgContainer; }
QColor AntTheme::elevatedColor() const { return tokens().colorBgElevated; }
QColor AntTheme::borderColor() const { return tokens().colorBorder; }
QColor AntTheme::secondaryBorderColor() const { return tokens().colorBorderSecondary; }
QColor AntTheme::disabledBackgroundColor() const { return tokens().colorBgContainerDisabled; }

int AntTheme::fontSize() const { return tokens().fontSize; }
int AntTheme::smallFontSize() const { return tokens().fontSizeSM; }
int AntTheme::largeFontSize() const { return tokens().fontSizeLG; }
int AntTheme::borderRadius() const { return tokens().borderRadius; }
int AntTheme::smallBorderRadius() const { return tokens().borderRadiusSM; }
int AntTheme::largeBorderRadius() const { return tokens().borderRadiusLG; }
int AntTheme::spacingUnit() const { return tokens().sizeUnit; }

QColor AntTheme::hoverColor(const QColor& base) const
{
    return AntPalette::hoverColor(base, m_themeMode);
}

QColor AntTheme::activeColor(const QColor& base) const
{
    return AntPalette::activeColor(base, m_themeMode);
}

QColor AntTheme::disabledColor(const QColor& foreground) const
{
    return AntPalette::disabledColor(foreground, tokens().colorBgContainer);
}

void AntTheme::drawEffectShadow(QPainter* painter, const QRect& rect, int shadowWidth, int radius, qreal strength) const
{
    if (!painter || shadowWidth <= 0)
    {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QColor shadow = tokens().colorShadow;
    for (int i = 0; i < shadowWidth; ++i)
    {
        const qreal progress = 1.0 - static_cast<qreal>(i) / shadowWidth;
        shadow.setAlphaF(std::clamp(0.08 * progress * strength, 0.0, 0.35));
        painter->setPen(QPen(shadow, 1));
        painter->setBrush(Qt::NoBrush);
        const QRectF layer = rect.adjusted(shadowWidth - i, shadowWidth - i, -(shadowWidth - i), -(shadowWidth - i));
        painter->drawRoundedRect(layer, radius + i, radius + i);
    }
    painter->restore();
}

void AntTheme::setThemeMode(Ant::ThemeMode mode)
{
    if (m_themeMode == mode)
    {
        return;
    }

    m_themeMode = mode;
    Q_EMIT themeModeChanged(m_themeMode);
    Q_EMIT themeChanged();
}

void AntTheme::toggleThemeMode()
{
    setThemeMode(m_themeMode == Ant::ThemeMode::Dark ? Ant::ThemeMode::Default : Ant::ThemeMode::Dark);
}

AntThemeTokens AntTheme::createTokens(Ant::ThemeMode mode)
{
    AntThemeTokens t;
    const bool dark = mode == Ant::ThemeMode::Dark;

    const QColor primary("#1677ff");
    const QColor success("#52c41a");
    const QColor warning("#faad14");
    const QColor error("#ff4d4f");

    const auto primaryPalette = AntPalette::generate(primary, mode);
    const auto successPalette = AntPalette::generate(success, mode);
    const auto warningPalette = AntPalette::generate(warning, mode);
    const auto errorPalette = AntPalette::generate(error, mode);

    t.colorPrimaryBg = primaryPalette[0];
    t.colorPrimaryBorder = primaryPalette[2];
    t.colorPrimaryHover = primaryPalette[4];
    t.colorPrimary = primaryPalette[5];
    t.colorPrimaryActive = primaryPalette[6];

    t.colorSuccessBg = successPalette[0];
    t.colorSuccessHover = successPalette[3];
    t.colorSuccess = successPalette[5];
    t.colorSuccessActive = successPalette[6];

    t.colorWarningBg = warningPalette[0];
    t.colorWarningHover = warningPalette[3];
    t.colorWarning = warningPalette[5];
    t.colorWarningActive = warningPalette[6];

    t.colorErrorBg = errorPalette[0];
    t.colorErrorHover = errorPalette[4];
    t.colorError = errorPalette[5];
    t.colorErrorActive = errorPalette[6];

    t.colorLink = t.colorPrimary;
    t.colorLinkHover = t.colorPrimaryHover;
    t.colorLinkActive = t.colorPrimaryActive;

    const QColor bgBase = dark ? QColor("#000000") : QColor("#ffffff");
    const QColor textBase = dark ? QColor("#ffffff") : QColor("#000000");

    t.colorBgBase = bgBase;
    t.colorPrimaryLoading = AntPalette::mix(bgBase, t.colorPrimary, 0.65);
    t.colorPrimaryLoadingHover = AntPalette::mix(bgBase, t.colorPrimary, 0.50);
    t.colorPrimaryLoadingActive = AntPalette::shade(t.colorPrimaryLoading, 0.10);
    t.colorTextLightSolid = QColor("#ffffff");
    t.colorText = blendOnBackground(textBase, bgBase, dark ? 0.85 : 0.88);
    t.colorTextSecondary = blendOnBackground(textBase, bgBase, 0.65);
    t.colorTextTertiary = blendOnBackground(textBase, bgBase, 0.45);
    t.colorTextDisabled = blendOnBackground(textBase, bgBase, 0.25);
    t.colorTextPlaceholder = t.colorTextDisabled;

    t.colorFill = blendOnBackground(textBase, bgBase, dark ? 0.30 : 0.15);
    t.colorFillSecondary = blendOnBackground(textBase, bgBase, dark ? 0.12 : 0.06);
    t.colorFillTertiary = blendOnBackground(textBase, bgBase, dark ? 0.08 : 0.04);
    t.colorFillQuaternary = blendOnBackground(textBase, bgBase, dark ? 0.04 : 0.02);

    if (dark)
    {
        t.colorBgLayout = QColor("#000000");
        t.colorBgContainer = QColor("#141414");
        t.colorBgElevated = QColor("#1f1f1f");
        t.colorBgContainerDisabled = QColor("#2a2a2a");
        t.colorBorder = QColor("#424242");
        t.colorBorderSecondary = QColor("#303030");
        t.colorBorderDisabled = QColor("#424242");
        t.colorSplit = QColor("#303030");
    }
    else
    {
        t.colorBgLayout = QColor("#f5f5f5");
        t.colorBgContainer = QColor("#ffffff");
        t.colorBgElevated = QColor("#ffffff");
        t.colorBgContainerDisabled = QColor("#f5f5f5");
        t.colorBorder = QColor("#d9d9d9");
        t.colorBorderSecondary = QColor("#f0f0f0");
        t.colorBorderDisabled = QColor("#d9d9d9");
        t.colorSplit = QColor("#f0f0f0");
    }

    t.colorShadow = tokenShadow(mode);
    t.colorRateStar = dark ? QColor(212, 177, 6) : QColor(250, 219, 20);

    return t;
}
