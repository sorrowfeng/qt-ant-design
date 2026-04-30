#include "AntIcon.h"

#include <QDir>
#include <QEvent>
#include <QPainter>
#include <QPainterPathStroker>
#include <QTransform>

#include "core/AntTheme.h"
#include "styles/AntIconStyle.h"
#include "styles/AntPalette.h"

namespace
{
QPainterPath createCirclePath(qreal x, qreal y, qreal w, qreal h)
{
    QPainterPath path;
    path.addEllipse(QRectF(x, y, w, h));
    return path;
}

QPainterPath createRoundedRectPath(qreal x, qreal y, qreal w, qreal h, qreal radius)
{
    QPainterPath path;
    path.addRoundedRect(QRectF(x, y, w, h), radius, radius);
    return path;
}

QPainterPath createLinePath(const QPointF& start, const QPointF& end)
{
    QPainterPath path;
    path.moveTo(start);
    path.lineTo(end);
    return path;
}

QPainterPath createPolylinePath(const QVector<QPointF>& points)
{
    QPainterPath path;
    if (points.isEmpty())
    {
        return path;
    }

    path.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i)
    {
        path.lineTo(points.at(i));
    }
    return path;
}

QPainterPath createSearchLensPath()
{
    QPainterPath path;
    path.addEllipse(QRectF(6, 6, 16, 16));
    path.moveTo(20, 20);
    path.lineTo(27, 27);
    return path;
}

QPainterPath createCrossPath()
{
    QPainterPath path;
    path.moveTo(9, 9);
    path.lineTo(23, 23);
    path.moveTo(23, 9);
    path.lineTo(9, 23);
    return path;
}

QPainterPath createCheckPath()
{
    return createPolylinePath({QPointF(7, 17), QPointF(13, 23), QPointF(25, 10)});
}

QPainterPath createChevronPath(Ant::IconType type)
{
    switch (type)
    {
    case Ant::IconType::Down:
        return createPolylinePath({QPointF(8, 12), QPointF(16, 20), QPointF(24, 12)});
    case Ant::IconType::Up:
        return createPolylinePath({QPointF(8, 20), QPointF(16, 12), QPointF(24, 20)});
    case Ant::IconType::Left:
        return createPolylinePath({QPointF(20, 8), QPointF(12, 16), QPointF(20, 24)});
    case Ant::IconType::Right:
        return createPolylinePath({QPointF(12, 8), QPointF(20, 16), QPointF(12, 24)});
    default:
        return QPainterPath();
    }
}

QPainterPath createStarPath()
{
    QPainterPath path;
    path.moveTo(16, 4);
    path.lineTo(19.53, 11.15);
    path.lineTo(27.42, 12.29);
    path.lineTo(21.71, 17.85);
    path.lineTo(23.06, 25.71);
    path.lineTo(16, 22);
    path.lineTo(8.94, 25.71);
    path.lineTo(10.29, 17.85);
    path.lineTo(4.58, 12.29);
    path.lineTo(12.47, 11.15);
    path.closeSubpath();
    return path;
}

QPainterPath createSettingPath()
{
    QPainterPath path;
    path.addEllipse(QRectF(12, 12, 8, 8));
    path.moveTo(16, 4);
    path.lineTo(16, 8);
    path.moveTo(16, 24);
    path.lineTo(16, 28);
    path.moveTo(4, 16);
    path.lineTo(8, 16);
    path.moveTo(24, 16);
    path.lineTo(28, 16);
    path.moveTo(7.5, 7.5);
    path.lineTo(10.5, 10.5);
    path.moveTo(21.5, 21.5);
    path.lineTo(24.5, 24.5);
    path.moveTo(24.5, 7.5);
    path.lineTo(21.5, 10.5);
    path.moveTo(10.5, 21.5);
    path.lineTo(7.5, 24.5);
    path.addRoundedRect(QRectF(8, 8, 16, 16), 8, 8);
    return path;
}

QPainterPath createHeartPath()
{
    QPainterPath path;
    path.moveTo(16, 27);
    path.cubicTo(9.2, 22.8, 4.5, 18.2, 4.5, 12.5);
    path.cubicTo(4.5, 8.4, 7.5, 5.5, 11.2, 5.5);
    path.cubicTo(13.2, 5.5, 15.0, 6.5, 16, 8.1);
    path.cubicTo(17.0, 6.5, 18.8, 5.5, 20.8, 5.5);
    path.cubicTo(24.5, 5.5, 27.5, 8.4, 27.5, 12.5);
    path.cubicTo(27.5, 18.2, 22.8, 22.8, 16, 27);
    path.closeSubpath();
    return path;
}

QPainterPath createBellPath()
{
    QPainterPath path;
    path.moveTo(8, 23);
    path.lineTo(24, 23);
    path.moveTo(10, 23);
    path.cubicTo(11.2, 20.5, 11.5, 17.8, 11.5, 14);
    path.cubicTo(11.5, 10.3, 13.4, 7.5, 16, 7.5);
    path.cubicTo(18.6, 7.5, 20.5, 10.3, 20.5, 14);
    path.cubicTo(20.5, 17.8, 20.8, 20.5, 22, 23);
    path.moveTo(13.5, 25);
    path.cubicTo(14.1, 26.6, 15, 27.5, 16, 27.5);
    path.cubicTo(17, 27.5, 17.9, 26.6, 18.5, 25);
    return path;
}

QPainterPath createMailPath()
{
    QPainterPath path;
    path.addRoundedRect(QRectF(5, 8, 22, 16), 2, 2);
    path.moveTo(6.5, 10);
    path.lineTo(16, 17);
    path.lineTo(25.5, 10);
    path.moveTo(13.5, 15.2);
    path.lineTo(6.5, 22);
    path.moveTo(18.5, 15.2);
    path.lineTo(25.5, 22);
    return path;
}

QPainterPath createEditPath()
{
    QPainterPath path;
    path.addRoundedRect(QRectF(6, 22, 20, 3), 1, 1);
    path.moveTo(9, 20);
    path.lineTo(10.5, 15);
    path.lineTo(21.5, 4);
    path.lineTo(26, 8.5);
    path.lineTo(15, 19.5);
    path.lineTo(9, 20);
    path.moveTo(19.5, 6);
    path.lineTo(24, 10.5);
    return path;
}

QPainterPath createDeletePath()
{
    QPainterPath path;
    path.moveTo(7, 10);
    path.lineTo(25, 10);
    path.moveTo(13, 7);
    path.lineTo(19, 7);
    path.moveTo(10, 10);
    path.lineTo(11.5, 27);
    path.lineTo(20.5, 27);
    path.lineTo(22, 10);
    path.moveTo(13.5, 14);
    path.lineTo(14.2, 23);
    path.moveTo(18.5, 14);
    path.lineTo(17.8, 23);
    return path;
}

QPainterPath createCloudUploadPath()
{
    QPainterPath path;
    path.moveTo(10, 23);
    path.cubicTo(6.8, 23, 4.5, 20.7, 4.5, 17.8);
    path.cubicTo(4.5, 15.2, 6.4, 13.1, 8.9, 12.7);
    path.cubicTo(10.2, 8.3, 13.7, 6, 17.5, 6);
    path.cubicTo(21.8, 6, 25.4, 9.4, 25.7, 13.7);
    path.cubicTo(27.8, 14.4, 29.5, 16.2, 29.5, 18.4);
    path.cubicTo(29.5, 21.0, 27.4, 23, 24.5, 23);
    path.moveTo(16, 25);
    path.lineTo(16, 14);
    path.moveTo(11.5, 18.5);
    path.lineTo(16, 14);
    path.lineTo(20.5, 18.5);
    return path;
}

QPainterPath createHomePath()
{
    QPainterPath path;
    path.moveTo(6, 15);
    path.lineTo(16, 6);
    path.lineTo(26, 15);
    path.lineTo(26, 26);
    path.lineTo(19, 26);
    path.lineTo(19, 19);
    path.lineTo(13, 19);
    path.lineTo(13, 26);
    path.lineTo(6, 26);
    path.closeSubpath();
    return path;
}

QPainterPath createUserPath()
{
    QPainterPath path;
    path.addEllipse(QRectF(10, 5, 12, 12));
    path.moveTo(6, 27);
    path.cubicTo(7.5, 22.5, 11, 20, 16, 20);
    path.cubicTo(21, 20, 24.5, 22.5, 26, 27);
    return path;
}

QPainterPath createCalendarOutlinePath()
{
    QPainterPath path;
    path.addRoundedRect(QRectF(5, 7, 22, 20), 2, 2);
    path.moveTo(10, 4);
    path.lineTo(10, 10);
    path.moveTo(22, 4);
    path.lineTo(22, 10);
    path.moveTo(5, 13);
    path.lineTo(27, 13);
    path.moveTo(10, 17);
    path.lineTo(10, 17.1);
    path.moveTo(16, 17);
    path.lineTo(16, 17.1);
    path.moveTo(22, 17);
    path.lineTo(22, 17.1);
    path.moveTo(10, 22);
    path.lineTo(10, 22.1);
    path.moveTo(16, 22);
    path.lineTo(16, 22.1);
    return path;
}

QPainterPath createClockOutlinePath()
{
    QPainterPath path;
    path.addEllipse(QRectF(5, 5, 22, 22));
    path.moveTo(16, 10);
    path.lineTo(16, 16);
    path.lineTo(20.5, 18.5);
    return path;
}

QPainterPath createInfoCirclePath(bool warning)
{
    QPainterPath path;
    path.addEllipse(QRectF(4.5, 4.5, 23, 23));
    if (warning)
    {
        path.moveTo(16, 10);
        path.lineTo(16, 18);
        path.moveTo(16, 22);
        path.lineTo(16, 22.1);
    }
    else
    {
        path.moveTo(16, 13);
        path.lineTo(16, 21);
        path.moveTo(16, 9.5);
        path.lineTo(16, 9.6);
    }
    return path;
}

QPainterPath createMinusPath()
{
    return createLinePath(QPointF(8, 16), QPointF(24, 16));
}

QPainterPath createPlusPath()
{
    QPainterPath path = createMinusPath();
    path.moveTo(16, 8);
    path.lineTo(16, 24);
    return path;
}

QPainterPath createLoadingPath()
{
    QPainterPath path;
    path.arcMoveTo(QRectF(5, 5, 22, 22), 35);
    path.arcTo(QRectF(5, 5, 22, 22), 35, 290);
    return path;
}

QString themeSuffix(Ant::IconTheme theme)
{
    switch (theme)
    {
    case Ant::IconTheme::Filled:
        return QStringLiteral("Filled");
    case Ant::IconTheme::TwoTone:
        return QStringLiteral("TwoTone");
    case Ant::IconTheme::Outlined:
    default:
        return QStringLiteral("Outlined");
    }
}

QString normalizedIconBaseName(const QString& iconName, Ant::IconTheme* parsedTheme)
{
    QString normalized = iconName.trimmed();
    const int slashIndex = qMax(normalized.lastIndexOf(QLatin1Char('/')), normalized.lastIndexOf(QLatin1Char('\\')));
    if (slashIndex >= 0)
    {
        normalized = normalized.mid(slashIndex + 1);
    }
    if (normalized.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive))
    {
        normalized.chop(4);
    }

    if (normalized.endsWith(QStringLiteral("TwoTone")))
    {
        normalized.chop(7);
        if (parsedTheme)
        {
            *parsedTheme = Ant::IconTheme::TwoTone;
        }
    }
    else if (normalized.endsWith(QStringLiteral("Outlined")))
    {
        normalized.chop(8);
        if (parsedTheme)
        {
            *parsedTheme = Ant::IconTheme::Outlined;
        }
    }
    else if (normalized.endsWith(QStringLiteral("Filled")))
    {
        normalized.chop(6);
        if (parsedTheme)
        {
            *parsedTheme = Ant::IconTheme::Filled;
        }
    }

    return normalized;
}
}

AntIcon::AntIcon(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntIconStyle>(this);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_spinTimer.setInterval(16);
    connect(&m_spinTimer, &QTimer::timeout, this, [this]() {
        m_spinAngle = (m_spinAngle + 6) % 360;
        update();
    });
}

AntIcon::AntIcon(Ant::IconType iconType, QWidget* parent)
    : AntIcon(parent)
{
    m_iconType = iconType;
    m_iconName = iconNameForType(iconType);
}

AntIcon::AntIcon(const QString& iconName, QWidget* parent)
    : AntIcon(parent)
{
    Ant::IconTheme parsedTheme = m_iconTheme;
    m_iconName = normalizedIconBaseName(iconName, &parsedTheme);
    m_iconTheme = parsedTheme;
}

Ant::IconType AntIcon::iconType() const { return m_iconType; }

void AntIcon::setIconType(Ant::IconType iconType)
{
    const QString newIconName = iconNameForType(iconType);
    if (m_iconType == iconType && m_iconName == newIconName)
    {
        return;
    }
    const bool typeChanged = m_iconType != iconType;
    const bool nameChanged = m_iconName != newIconName;
    m_iconType = iconType;
    m_iconName = newIconName;
    m_hasCustomPath = false;
    update();
    if (typeChanged)
    {
        Q_EMIT iconTypeChanged(m_iconType);
    }
    if (nameChanged)
    {
        Q_EMIT iconNameChanged(m_iconName);
    }
}

QString AntIcon::iconName() const { return m_iconName; }

void AntIcon::setIconName(const QString& iconName)
{
    Ant::IconTheme parsedTheme = m_iconTheme;
    const QString newIconName = normalizedIconBaseName(iconName, &parsedTheme);
    if (m_iconType == Ant::IconType::None && m_iconName == newIconName && m_iconTheme == parsedTheme)
    {
        return;
    }

    const bool typeChanged = m_iconType != Ant::IconType::None;
    const bool nameChanged = m_iconName != newIconName;
    const bool themeChanged = m_iconTheme != parsedTheme;

    m_iconType = Ant::IconType::None;
    m_iconName = newIconName;
    m_iconTheme = parsedTheme;
    m_hasCustomPath = false;
    update();

    if (typeChanged)
    {
        Q_EMIT iconTypeChanged(m_iconType);
    }
    if (themeChanged)
    {
        Q_EMIT iconThemeChanged(m_iconTheme);
    }
    if (nameChanged)
    {
        Q_EMIT iconNameChanged(m_iconName);
    }
}

QString AntIcon::resolvedIconName() const
{
    const QString baseName = !m_iconName.isEmpty() ? m_iconName : iconNameForType(m_iconType);
    if (baseName.isEmpty())
    {
        return QString();
    }
    return baseName + themeSuffix(m_iconTheme);
}

Ant::IconTheme AntIcon::iconTheme() const { return m_iconTheme; }

void AntIcon::setIconTheme(Ant::IconTheme iconTheme)
{
    if (m_iconTheme == iconTheme)
    {
        return;
    }
    m_iconTheme = iconTheme;
    update();
    Q_EMIT iconThemeChanged(m_iconTheme);
}

int AntIcon::iconSize() const { return m_iconSize; }

void AntIcon::setIconSize(int iconSize)
{
    const int clamped = qMax(8, iconSize);
    if (m_iconSize == clamped)
    {
        return;
    }
    m_iconSize = clamped;
    updateGeometry();
    update();
    Q_EMIT iconSizeChanged(m_iconSize);
}

QColor AntIcon::color() const { return m_color; }

void AntIcon::setColor(const QColor& color)
{
    if (m_color == color)
    {
        return;
    }
    m_color = color;
    update();
    Q_EMIT colorChanged(m_color);
}

QColor AntIcon::twoToneColor() const { return m_twoToneColor; }

void AntIcon::setTwoToneColor(const QColor& color)
{
    if (m_twoToneColor == color)
    {
        return;
    }
    m_twoToneColor = color;
    update();
    Q_EMIT twoToneColorChanged(m_twoToneColor);
}

int AntIcon::rotate() const { return m_rotate; }

void AntIcon::setRotate(int rotate)
{
    rotate %= 360;
    if (rotate < 0)
    {
        rotate += 360;
    }
    if (m_rotate == rotate)
    {
        return;
    }
    m_rotate = rotate;
    update();
    Q_EMIT rotateChanged(m_rotate);
}

bool AntIcon::isSpin() const { return m_spin; }

void AntIcon::setSpin(bool spin)
{
    if (m_spin == spin)
    {
        return;
    }
    m_spin = spin;
    updateTimerState();
    update();
    Q_EMIT spinChanged(m_spin);
}

void AntIcon::setCustomPath(const QPainterPath& primaryPath, const QPainterPath& secondaryPath)
{
    m_customPrimaryPath = primaryPath;
    m_customSecondaryPath = secondaryPath;
    m_hasCustomPath = !m_customPrimaryPath.isEmpty() || !m_customSecondaryPath.isEmpty();
    update();
}

void AntIcon::clearCustomPath()
{
    m_customPrimaryPath = QPainterPath();
    m_customSecondaryPath = QPainterPath();
    m_hasCustomPath = false;
    update();
}

bool AntIcon::hasCustomPath() const { return m_hasCustomPath; }

QPainterPath AntIcon::customPrimaryPath() const { return m_customPrimaryPath; }

QPainterPath AntIcon::customSecondaryPath() const { return m_customSecondaryPath; }

int AntIcon::spinAngle() const { return m_spinAngle; }

QSize AntIcon::sizeHint() const
{
    return QSize(m_iconSize, m_iconSize);
}

QSize AntIcon::minimumSizeHint() const
{
    return QSize(8, 8);
}

void AntIcon::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntIcon::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        update();
    }
    QWidget::changeEvent(event);
}

AntIcon::IconPaths AntIcon::builtinPaths(Ant::IconType type, Ant::IconTheme theme)
{
    IconPaths result;
    result.useStroke = theme == Ant::IconTheme::Outlined;
    QPainterPathStroker stroker;
    stroker.setWidth(2.8);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);

    switch (type)
    {
    case Ant::IconType::Search:
        result.primary = createSearchLensPath();
        break;
    case Ant::IconType::Close:
        result.primary = createCrossPath();
        break;
    case Ant::IconType::Plus:
        result.primary = createPlusPath();
        break;
    case Ant::IconType::Minus:
        result.primary = createMinusPath();
        break;
    case Ant::IconType::Check:
        result.primary = createCheckPath();
        break;
    case Ant::IconType::Loading:
        result.primary = createLoadingPath();
        result.useStroke = true;
        break;
    case Ant::IconType::Down:
    case Ant::IconType::Up:
    case Ant::IconType::Left:
    case Ant::IconType::Right:
        result.primary = createChevronPath(type);
        break;
    case Ant::IconType::Home:
        result.primary = createHomePath();
        break;
    case Ant::IconType::User:
        result.primary = createUserPath();
        break;
    case Ant::IconType::Star:
        result.primary = createStarPath();
        result.useStroke = theme == Ant::IconTheme::Outlined;
        break;
    case Ant::IconType::Setting:
        result.primary = createSettingPath();
        result.useStroke = true;
        break;
    case Ant::IconType::Heart:
        result.primary = createHeartPath();
        result.useStroke = theme == Ant::IconTheme::Outlined;
        break;
    case Ant::IconType::Bell:
        result.primary = createBellPath();
        result.useStroke = true;
        break;
    case Ant::IconType::Mail:
        result.primary = createMailPath();
        result.useStroke = true;
        break;
    case Ant::IconType::Edit:
        result.primary = createEditPath();
        result.useStroke = true;
        break;
    case Ant::IconType::Delete:
        result.primary = createDeletePath();
        result.useStroke = true;
        break;
    case Ant::IconType::CloudUpload:
        result.primary = createCloudUploadPath();
        result.useStroke = true;
        break;
    case Ant::IconType::Calendar:
        if (theme == Ant::IconTheme::Outlined)
        {
            result.primary = createCalendarOutlinePath();
            result.useStroke = true;
        }
        else
        {
            result.primary = createRoundedRectPath(5, 7, 22, 20, 2);
            result.secondary = createRoundedRectPath(8, 15, 16, 9, 1);
        }
        break;
    case Ant::IconType::ClockCircle:
        if (theme == Ant::IconTheme::Outlined)
        {
            result.primary = createClockOutlinePath();
            result.useStroke = true;
        }
        else
        {
            result.primary = createCirclePath(4.5, 4.5, 23, 23);
            QPainterPath hand;
            hand.moveTo(16, 10);
            hand.lineTo(16, 16);
            hand.lineTo(20.5, 18.5);
            result.secondary = stroker.createStroke(hand);
            result.useStroke = false;
        }
        break;
    case Ant::IconType::InfoCircle:
    case Ant::IconType::ExclamationCircle:
    case Ant::IconType::CloseCircle:
    case Ant::IconType::CheckCircle:
        if (theme == Ant::IconTheme::Outlined)
        {
            if (type == Ant::IconType::InfoCircle)
            {
                result.primary = createInfoCirclePath(false);
            }
            else if (type == Ant::IconType::ExclamationCircle)
            {
                result.primary = createInfoCirclePath(true);
            }
            else
            {
                result.primary = createCirclePath(4.5, 4.5, 23, 23);
                result.secondary = type == Ant::IconType::CloseCircle ? createCrossPath() : createCheckPath();
            }
            result.useStroke = true;
        }
        else
        {
            result.primary = createCirclePath(4.5, 4.5, 23, 23);
            if (type == Ant::IconType::InfoCircle)
            {
                QPainterPath symbol;
                symbol.addRect(QRectF(15, 13, 2, 8));
                symbol.addEllipse(QRectF(15, 9.2, 2, 2));
                result.secondary = symbol;
            }
            else if (type == Ant::IconType::ExclamationCircle)
            {
                QPainterPath symbol;
                symbol.addRect(QRectF(15, 10, 2, 8));
                symbol.addEllipse(QRectF(15, 21, 2, 2));
                result.secondary = symbol;
            }
            else if (type == Ant::IconType::CloseCircle)
            {
                QPainterPath symbol;
                symbol.addPolygon(QPolygonF({QPointF(11, 10), QPointF(16, 15), QPointF(21, 10), QPointF(22, 11), QPointF(17, 16), QPointF(22, 21), QPointF(21, 22), QPointF(16, 17), QPointF(11, 22), QPointF(10, 21), QPointF(15, 16), QPointF(10, 11)}));
                result.secondary = symbol;
            }
            else
            {
                QPainterPath symbol;
                symbol.moveTo(10, 16);
                symbol.lineTo(14, 20);
                symbol.lineTo(22, 12);
                result.secondary = stroker.createStroke(symbol);
            }
        }
        break;
    case Ant::IconType::None:
    default:
        break;
    }

    if (theme == Ant::IconTheme::Filled)
    {
        result.useStroke = false;
    }
    if (theme == Ant::IconTheme::TwoTone)
    {
        result.useStroke = false;
    }
    if (!result.useStroke)
    {
        switch (type)
        {
        case Ant::IconType::Search:
        case Ant::IconType::Close:
        case Ant::IconType::Plus:
        case Ant::IconType::Minus:
        case Ant::IconType::Check:
        case Ant::IconType::Loading:
        case Ant::IconType::Down:
        case Ant::IconType::Up:
        case Ant::IconType::Left:
        case Ant::IconType::Right:
        case Ant::IconType::Setting:
        case Ant::IconType::Bell:
        case Ant::IconType::Mail:
        case Ant::IconType::Edit:
        case Ant::IconType::Delete:
        case Ant::IconType::CloudUpload:
            result.primary = stroker.createStroke(result.primary);
            break;
        default:
            break;
        }
    }
    return result;
}

QString AntIcon::iconNameForType(Ant::IconType type)
{
    switch (type)
    {
    case Ant::IconType::Search:
        return QStringLiteral("Search");
    case Ant::IconType::Close:
        return QStringLiteral("Close");
    case Ant::IconType::Plus:
        return QStringLiteral("Plus");
    case Ant::IconType::Minus:
        return QStringLiteral("Minus");
    case Ant::IconType::Check:
        return QStringLiteral("Check");
    case Ant::IconType::InfoCircle:
        return QStringLiteral("InfoCircle");
    case Ant::IconType::ExclamationCircle:
        return QStringLiteral("ExclamationCircle");
    case Ant::IconType::CloseCircle:
        return QStringLiteral("CloseCircle");
    case Ant::IconType::CheckCircle:
        return QStringLiteral("CheckCircle");
    case Ant::IconType::Loading:
        return QStringLiteral("Loading");
    case Ant::IconType::Down:
        return QStringLiteral("Down");
    case Ant::IconType::Up:
        return QStringLiteral("Up");
    case Ant::IconType::Left:
        return QStringLiteral("Left");
    case Ant::IconType::Right:
        return QStringLiteral("Right");
    case Ant::IconType::Calendar:
        return QStringLiteral("Calendar");
    case Ant::IconType::ClockCircle:
        return QStringLiteral("ClockCircle");
    case Ant::IconType::User:
        return QStringLiteral("User");
    case Ant::IconType::Home:
        return QStringLiteral("Home");
    case Ant::IconType::Star:
        return QStringLiteral("Star");
    case Ant::IconType::Setting:
        return QStringLiteral("Setting");
    case Ant::IconType::Heart:
        return QStringLiteral("Heart");
    case Ant::IconType::Bell:
        return QStringLiteral("Bell");
    case Ant::IconType::Mail:
        return QStringLiteral("Mail");
    case Ant::IconType::Edit:
        return QStringLiteral("Edit");
    case Ant::IconType::Delete:
        return QStringLiteral("Delete");
    case Ant::IconType::CloudUpload:
        return QStringLiteral("CloudUpload");
    case Ant::IconType::None:
    default:
        return QString();
    }
}

QStringList AntIcon::builtinIconNames()
{
    static QStringList names;
    static bool initialized = false;
    if (!initialized)
    {
        QDir dir(QStringLiteral(":/qt-ant-design/icons/antd"));
        const QStringList entries = dir.entryList(QStringList{QStringLiteral("*.svg")}, QDir::Files, QDir::Name);
        names.reserve(entries.size());
        for (QString entry : entries)
        {
            entry.chop(4);
            names.append(entry);
        }
        initialized = true;
    }
    return names;
}

QPainterPath AntIcon::transformPath(const QPainterPath& path, const QRectF& targetRect)
{
    if (path.isEmpty())
    {
        return path;
    }

    QTransform transform;
    transform.translate(targetRect.x(), targetRect.y());
    transform.scale(targetRect.width() / 32.0, targetRect.height() / 32.0);
    return transform.map(path);
}

QColor AntIcon::effectivePrimaryColor() const
{
    const auto& token = antTheme->tokens();
    QColor base = m_color.isValid() ? m_color : token.colorText;
    if (!isEnabled())
    {
        base = token.colorTextDisabled;
    }
    if (m_iconTheme == Ant::IconTheme::TwoTone)
    {
        return m_twoToneColor.isValid() ? m_twoToneColor : token.colorPrimary;
    }
    return base;
}

QColor AntIcon::effectiveSecondaryColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
    {
        return token.colorBgContainerDisabled;
    }
    if (m_iconTheme == Ant::IconTheme::TwoTone)
    {
        const QColor tone = m_twoToneColor.isValid() ? m_twoToneColor : token.colorPrimary;
        return AntPalette::tint(tone, 0.72);
    }
    if (m_iconTheme == Ant::IconTheme::Filled)
    {
        return token.colorTextLightSolid;
    }
    return m_color.isValid() ? m_color : token.colorText;
}

void AntIcon::updateTimerState()
{
    if (m_spin)
    {
        if (!m_spinTimer.isActive())
        {
            m_spinTimer.start();
        }
    }
    else
    {
        m_spinTimer.stop();
        m_spinAngle = 0;
    }
}
