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

    if (dark)
    {
        t.colorPrimaryBg = QColor("#111a2c");
        t.colorPrimaryBorder = QColor("#15325b");
        t.colorPrimaryHover = QColor("#3c89e8");
        t.colorPrimary = QColor("#1668dc");
        t.colorPrimaryActive = QColor("#1554ad");

        t.colorSuccessBg = QColor("#162312");
        t.colorSuccessHover = QColor("#306317");
        t.colorSuccess = QColor("#49aa19");
        t.colorSuccessActive = QColor("#3c8618");

        t.colorWarningBg = QColor("#2b2111");
        t.colorWarningHover = QColor("#7c5914");
        t.colorWarning = QColor("#d89614");
        t.colorWarningActive = QColor("#aa7714");

        t.colorErrorBg = QColor("#2c1618");
        t.colorErrorHover = QColor("#e86e6b");
        t.colorError = QColor("#dc4446");
        t.colorErrorActive = QColor("#ad393a");
    }
    else
    {
        t.colorPrimaryBg = QColor("#e6f4ff");
        t.colorPrimaryBorder = QColor("#91caff");
        t.colorPrimaryHover = QColor("#4096ff");
        t.colorPrimary = QColor("#1677ff");
        t.colorPrimaryActive = QColor("#0958d9");

        t.colorSuccessBg = QColor("#f6ffed");
        t.colorSuccessHover = QColor("#95de64");
        t.colorSuccess = QColor("#52c41a");
        t.colorSuccessActive = QColor("#389e0d");

        t.colorWarningBg = QColor("#fffbe6");
        t.colorWarningHover = QColor("#ffd666");
        t.colorWarning = QColor("#faad14");
        t.colorWarningActive = QColor("#d48806");

        t.colorErrorBg = QColor("#fff2f0");
        t.colorErrorHover = QColor("#ff7875");
        t.colorError = QColor("#ff4d4f");
        t.colorErrorActive = QColor("#d9363e");
    }

    t.colorLink = t.colorPrimary;
    t.colorLinkHover = t.colorPrimaryHover;
    t.colorLinkActive = t.colorPrimaryActive;

    const QColor bgBase = dark ? QColor("#000000") : QColor("#ffffff");
    const QColor textBase = dark ? QColor("#ffffff") : QColor("#000000");

    t.colorBgBase = bgBase;
    t.colorPrimaryLoading = AntPalette::mix(bgBase, t.colorPrimary, 0.65);
    t.colorPrimaryLoadingHover = AntPalette::mix(bgBase, t.colorPrimaryHover, 0.65);
    t.colorPrimaryLoadingActive = AntPalette::mix(bgBase, t.colorPrimaryActive, 0.65);
    t.colorTextLightSolid = QColor("#ffffff");
    t.colorText = blendOnBackground(textBase, bgBase, dark ? 0.85 : 0.88);
    t.colorTextSecondary = blendOnBackground(textBase, bgBase, 0.65);
    t.colorTextTertiary = blendOnBackground(textBase, bgBase, 0.45);
    t.colorTextDisabled = blendOnBackground(textBase, bgBase, 0.25);
    t.colorTextPlaceholder = t.colorTextDisabled;

    const QColor fillBase = dark ? QColor("#141414") : bgBase;
    t.colorFill = blendOnBackground(textBase, fillBase, dark ? 0.18 : 0.15);
    t.colorFillSecondary = blendOnBackground(textBase, fillBase, dark ? 0.12 : 0.06);
    t.colorFillTertiary = blendOnBackground(textBase, fillBase, dark ? 0.08 : 0.04);
    t.colorFillQuaternary = blendOnBackground(textBase, fillBase, dark ? 0.04 : 0.02);

    if (dark)
    {
        t.colorBgLayout = QColor("#000000");
        t.colorBgContainer = QColor("#141414");
        t.colorBgElevated = QColor("#1f1f1f");
        t.colorBgContainerDisabled = QColor("#262626");
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
