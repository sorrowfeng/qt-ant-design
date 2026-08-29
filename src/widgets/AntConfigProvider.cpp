#include "AntConfigProvider.h"

#include "core/AntTheme.h"

#include <QApplication>
#include <QTimer>

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
    scheduleConfigChanged();
}

QColor AntConfigProvider::primaryColor() const { return m_primaryColor; }
void AntConfigProvider::setPrimaryColor(const QColor& color)
{
    if (m_primaryColor == color) return;
    m_primaryColor = color;
    Q_EMIT primaryColorChanged(m_primaryColor);
    scheduleConfigChanged();
}

int AntConfigProvider::fontSize() const { return m_fontSize; }
void AntConfigProvider::setFontSize(int size)
{
    size = qBound(1, size, Ant::MaximumThemeFontSize);
    if (m_fontSize == size) return;
    m_fontSize = size;
    Q_EMIT fontSizeChanged(m_fontSize);
    scheduleConfigChanged();
}

int AntConfigProvider::borderRadius() const { return m_borderRadius; }
void AntConfigProvider::setBorderRadius(int radius)
{
    radius = qBound(0, radius, Ant::MaximumThemeBorderRadius);
    if (m_borderRadius == radius) return;
    m_borderRadius = radius;
    Q_EMIT borderRadiusChanged(m_borderRadius);
    scheduleConfigChanged();
}

int AntConfigProvider::revision() const { return m_revision; }

Ant::ThemeDensity AntConfigProvider::density() const { return m_density; }
void AntConfigProvider::setDensity(Ant::ThemeDensity density)
{
    if (m_density == density) return;
    m_density = density;
    Q_EMIT densityChanged(m_density);
    scheduleConfigChanged();
}

Qt::LayoutDirection AntConfigProvider::direction() const { return m_direction; }
void AntConfigProvider::setDirection(Qt::LayoutDirection direction)
{
    if (m_direction == direction) return;
    m_direction = direction;
    Q_EMIT directionChanged(m_direction);
    scheduleConfigChanged();
}

void AntConfigProvider::apply()
{
    antTheme->applyConfiguration(m_themeMode, m_primaryColor, m_fontSize, m_borderRadius, m_density);
    if (QApplication* app = qobject_cast<QApplication*>(QCoreApplication::instance()))
    {
        app->setLayoutDirection(m_direction);
    }
}

void AntConfigProvider::scheduleConfigChanged()
{
    if (m_configChangedScheduled)
    {
        return;
    }

    m_configChangedScheduled = true;
    QTimer::singleShot(0, this, [this]() {
        m_configChangedScheduled = false;
        ++m_revision;
        Q_EMIT configChanged();
    });
}
