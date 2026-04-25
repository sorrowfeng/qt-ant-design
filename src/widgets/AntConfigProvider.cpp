#include "AntConfigProvider.h"

#include "core/AntTheme.h"

AntConfigProvider::AntConfigProvider(QObject* parent)
    : QObject(parent)
{
}

Ant::ThemeMode AntConfigProvider::themeMode() const { return m_themeMode; }
void AntConfigProvider::setThemeMode(Ant::ThemeMode mode)
{
    if (m_themeMode == mode) return;
    m_themeMode = mode;
    Q_EMIT themeModeChanged(m_themeMode);
}

QColor AntConfigProvider::primaryColor() const { return m_primaryColor; }
void AntConfigProvider::setPrimaryColor(const QColor& color)
{
    if (m_primaryColor == color) return;
    m_primaryColor = color;
    Q_EMIT primaryColorChanged(m_primaryColor);
}

int AntConfigProvider::fontSize() const { return m_fontSize; }
void AntConfigProvider::setFontSize(int size)
{
    if (m_fontSize == size) return;
    m_fontSize = size;
    Q_EMIT fontSizeChanged(m_fontSize);
}

int AntConfigProvider::borderRadius() const { return m_borderRadius; }
void AntConfigProvider::setBorderRadius(int radius)
{
    if (m_borderRadius == radius) return;
    m_borderRadius = radius;
    Q_EMIT borderRadiusChanged(m_borderRadius);
}

void AntConfigProvider::apply()
{
    if (!m_primaryColor.isValid()) return;
    antTheme->setThemeMode(m_themeMode);
}
