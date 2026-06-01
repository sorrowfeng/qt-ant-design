#include <QImage>
#include <QPainter>
#include <QPixmapCache>
#include <QSignalSpy>
#include <QTest>
#include <QList>
#include <QtMath>

#include <limits>

#include "widgets/AntIcon.h"

class TestAntIcon : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
    void resourceIconRasterReferenceMatchesWidgetRender();
    void resourceIconHighDpiCacheUsesFullPixmapSource();
};

void TestAntIcon::propertiesAndSignals()
{
    auto renderIcon = [](AntIcon& icon) {
        QImage image(icon.size(), QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(icon.devicePixelRatioF());
        image.fill(Qt::transparent);
        QPainter painter(&image);
        icon.render(&painter);
        painter.end();
        return image;
    };

    auto hasPaintedPixel = [](const QImage& image) {
        for (int y = 0; y < image.height(); ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                if (qAlpha(image.pixel(x, y)) > 0)
                {
                    return true;
                }
            }
        }
        return false;
    };

    auto* w = new AntIcon;
    QCOMPARE(w->iconType(), Ant::IconType::None);
    QCOMPARE(w->iconName(), QString());
    QCOMPARE(w->resolvedIconName(), QString());
    QCOMPARE(w->iconTheme(), Ant::IconTheme::Outlined);
    QVERIFY(w->iconSize() > 0);
    QCOMPARE(w->rotate(), 0);
    QCOMPARE(w->isSpin(), false);

    QSignalSpy typeSpy(w, &AntIcon::iconTypeChanged);
    QSignalSpy nameSpy(w, &AntIcon::iconNameChanged);
    w->setIconType(Ant::IconType::Search);
    QCOMPARE(w->iconType(), Ant::IconType::Search);
    QCOMPARE(w->iconName(), QStringLiteral("Search"));
    QCOMPARE(w->resolvedIconName(), QStringLiteral("SearchOutlined"));
    QCOMPARE(typeSpy.count(), 1);
    QCOMPARE(nameSpy.count(), 1);

    QSignalSpy themeSpy(w, &AntIcon::iconThemeChanged);
    w->setIconTheme(Ant::IconTheme::Filled);
    QCOMPARE(w->iconTheme(), Ant::IconTheme::Filled);
    QCOMPARE(w->resolvedIconName(), QStringLiteral("SearchFilled"));
    QCOMPARE(themeSpy.count(), 1);

    QSignalSpy sizeSpy(w, &AntIcon::iconSizeChanged);
    w->setIconSize(24);
    QCOMPARE(w->iconSize(), 24);
    QCOMPARE(sizeSpy.count(), 1);

    QSignalSpy spinSpy(w, &AntIcon::spinChanged);
    w->setSpin(true);
    QCOMPARE(w->isSpin(), true);
    QCOMPARE(spinSpy.count(), 1);

    QSignalSpy rotateSpy(w, &AntIcon::rotateChanged);
    w->setRotate(90);
    QCOMPARE(w->rotate(), 90);
    QCOMPARE(rotateSpy.count(), 1);

    QPainterPath primaryPath;
    primaryPath.addRect(QRectF(0, 0, 32, 32));
    QPainterPath secondaryPath;
    secondaryPath.addEllipse(QRectF(8, 8, 16, 16));
    w->setCustomPath(primaryPath, secondaryPath);
    QVERIFY(w->hasCustomPath());
    QVERIFY(!w->customPrimaryPath().isEmpty());
    QVERIFY(!w->customSecondaryPath().isEmpty());
    w->clearCustomPath();
    QVERIFY(!w->hasCustomPath());

    QSignalSpy namedIconSpy(w, &AntIcon::iconNameChanged);
    w->setIconName(QStringLiteral("GithubFilled"));
    QCOMPARE(w->iconType(), Ant::IconType::None);
    QCOMPARE(w->iconName(), QStringLiteral("Github"));
    QCOMPARE(w->iconTheme(), Ant::IconTheme::Filled);
    QCOMPARE(w->resolvedIconName(), QStringLiteral("GithubFilled"));
    QCOMPARE(namedIconSpy.count(), 1);

    w->setIconName(QStringLiteral("AccountBookTwoTone.svg"));
    QCOMPARE(w->iconName(), QStringLiteral("AccountBook"));
    QCOMPARE(w->iconTheme(), Ant::IconTheme::TwoTone);
    QCOMPARE(w->resolvedIconName(), QStringLiteral("AccountBookTwoTone"));

    const QStringList builtinIconNames = AntIcon::builtinIconNames();
    QCOMPARE(builtinIconNames.size(), 831);
    QVERIFY(builtinIconNames.contains(QStringLiteral("AccountBookOutlined")));
    QVERIFY(builtinIconNames.contains(QStringLiteral("GithubFilled")));
    QVERIFY(builtinIconNames.contains(QStringLiteral("HeartTwoTone")));

    AntIcon official(QStringLiteral("GithubFilled"));
    official.setIconSize(24);
    official.resize(24, 24);
    QVERIFY(hasPaintedPixel(renderIcon(official)));

    QPixmapCache::clear();
    AntIcon cachedOfficial(QStringLiteral("GithubFilled"));
    cachedOfficial.setIconSize(24);
    cachedOfficial.setColor(QColor("#123456"));
    cachedOfficial.resize(24, 24);
    QVERIFY(hasPaintedPixel(renderIcon(cachedOfficial)));
    QCOMPARE(cachedOfficial.property("antIconRenderCacheSource").toString(), QStringLiteral("resource"));
    QCOMPARE(cachedOfficial.property("antIconRenderCacheHit").toBool(), false);

    const QString firstCacheKey = cachedOfficial.property("antIconRenderCacheKey").toString();
    QVERIFY(!firstCacheKey.isEmpty());
    QVERIFY(hasPaintedPixel(renderIcon(cachedOfficial)));
    QCOMPARE(cachedOfficial.property("antIconRenderCacheHit").toBool(), true);
    QCOMPARE(cachedOfficial.property("antIconRenderCacheKey").toString(), firstCacheKey);

    cachedOfficial.setRotate(90);
    QVERIFY(hasPaintedPixel(renderIcon(cachedOfficial)));
    QCOMPARE(cachedOfficial.property("antIconRenderCacheHit").toBool(), true);
    QCOMPARE(cachedOfficial.property("antIconRenderCacheKey").toString(), firstCacheKey);

    const int spinAngleBefore = cachedOfficial.spinAngle();
    cachedOfficial.setSpin(true);
    QTRY_VERIFY(cachedOfficial.spinAngle() != spinAngleBefore);
    QVERIFY(hasPaintedPixel(renderIcon(cachedOfficial)));
    QCOMPARE(cachedOfficial.property("antIconRenderCacheHit").toBool(), true);
    QCOMPARE(cachedOfficial.property("antIconRenderCacheKey").toString(), firstCacheKey);
    cachedOfficial.setSpin(false);

    const QList<Ant::IconType> outlinedReferenceIcons = {
        Ant::IconType::Home,
        Ant::IconType::User,
        Ant::IconType::Search,
        Ant::IconType::Setting,
        Ant::IconType::Star,
        Ant::IconType::Heart,
        Ant::IconType::Bell,
        Ant::IconType::Mail,
        Ant::IconType::Calendar,
        Ant::IconType::ClockCircle,
        Ant::IconType::Check,
        Ant::IconType::Close,
        Ant::IconType::Plus,
        Ant::IconType::Edit,
        Ant::IconType::Delete,
        Ant::IconType::CloudUpload,
    };
    for (const Ant::IconType iconType : outlinedReferenceIcons)
    {
        QVERIFY(!AntIcon::builtinPaths(iconType, Ant::IconTheme::Outlined).primary.isEmpty());
    }
}

void TestAntIcon::resourceIconRasterReferenceMatchesWidgetRender()
{
    auto renderIconAtDpr = [](AntIcon& icon, qreal dpr) {
        const QSize logicalSize = icon.size();
        QImage image(QSize(qRound(logicalSize.width() * dpr), qRound(logicalSize.height() * dpr)),
                     QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        icon.render(&painter, QPoint(), QRegion(), QWidget::RenderFlags());
        painter.end();
        return image;
    };

    auto alphaBounds = [](const QImage& image) {
        QRect bounds;
        bool initialized = false;
        for (int y = 0; y < image.height(); ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                if (image.pixelColor(x, y).alpha() <= 16)
                {
                    continue;
                }
                const QRect pixelRect(x, y, 1, 1);
                bounds = initialized ? bounds.united(pixelRect) : pixelRect;
                initialized = true;
            }
        }
        return initialized ? bounds : QRect();
    };

    auto paintedAlphaPixels = [](const QImage& image) {
        int pixels = 0;
        for (int y = 0; y < image.height(); ++y)
        {
            for (int x = 0; x < image.width(); ++x)
            {
                if (image.pixelColor(x, y).alpha() > 16)
                {
                    ++pixels;
                }
            }
        }
        return pixels;
    };

    auto meanPixelDifference = [](const QImage& actual, const QImage& expected) {
        if (actual.size() != expected.size() || actual.isNull() || expected.isNull())
        {
            return std::numeric_limits<qreal>::max();
        }
        quint64 total = 0;
        for (int y = 0; y < actual.height(); ++y)
        {
            for (int x = 0; x < actual.width(); ++x)
            {
                const QColor a = actual.pixelColor(x, y);
                const QColor e = expected.pixelColor(x, y);
                total += qAbs(a.red() - e.red());
                total += qAbs(a.green() - e.green());
                total += qAbs(a.blue() - e.blue());
                total += qAbs(a.alpha() - e.alpha());
            }
        }
        return static_cast<qreal>(total) / static_cast<qreal>(actual.width() * actual.height() * 4);
    };

    const QStringList problemIcons = {
        QStringLiteral("UserOutlined"),
        QStringLiteral("SettingOutlined"),
        QStringLiteral("HeartOutlined"),
        QStringLiteral("AccountBookOutlined"),
        QStringLiteral("AccountBookTwoTone"),
        QStringLiteral("DotNetOutlined"),
        QStringLiteral("TwitchFilled"),
        QStringLiteral("XFilled"),
    };
    const QColor iconColor(QStringLiteral("#1f1f1f"));
    constexpr qreal maxCacheDiff = 0.5;

    for (const QString& iconName : problemIcons)
    {
        QPixmapCache::clear();
        AntIcon icon(iconName);
        icon.setIconSize(24);
        icon.setColor(iconColor);
        icon.resize(24, 24);

        const QImage firstRender = renderIconAtDpr(icon, 1.0);
        QCOMPARE(icon.property("antIconRenderCacheSource").toString(), QStringLiteral("resource"));
        QCOMPARE(icon.property("antIconRenderCacheHit").toBool(), false);
        const QRect firstBounds = alphaBounds(firstRender);
        QVERIFY2(firstBounds.isValid(),
                 qPrintable(QStringLiteral("%1 should render visible path geometry").arg(iconName)));
        QVERIFY2(firstBounds.width() >= 8 && firstBounds.height() >= 8,
                 qPrintable(QStringLiteral("%1 visible bounds are unexpectedly small: %2x%3")
                                .arg(iconName)
                                .arg(firstBounds.width())
                                .arg(firstBounds.height())));
        QVERIFY2(paintedAlphaPixels(firstRender) >= 24,
                 qPrintable(QStringLiteral("%1 should have enough painted pixels for an official icon").arg(iconName)));

        const QImage cachedRender = renderIconAtDpr(icon, 1.0);
        QCOMPARE(icon.property("antIconRenderCacheHit").toBool(), true);
        const qreal cachedDiff = meanPixelDifference(cachedRender, firstRender);
        QVERIFY2(cachedDiff < maxCacheDiff,
                 qPrintable(QStringLiteral("%1 cache hit must match cache miss rendering. diff=%2").arg(iconName).arg(cachedDiff)));
    }
}

void TestAntIcon::resourceIconHighDpiCacheUsesFullPixmapSource()
{
    auto renderIconAtDpr = [](AntIcon& icon, qreal dpr) {
        const QSize logicalSize = icon.size();
        QImage image(QSize(qRound(logicalSize.width() * dpr), qRound(logicalSize.height() * dpr)),
                     QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        icon.render(&painter, QPoint(), QRegion(), QWidget::RenderFlags());
        painter.end();
        return image;
    };

    auto meanPixelDifference = [](const QImage& actual, const QImage& expected) {
        if (actual.size() != expected.size() || actual.isNull() || expected.isNull())
        {
            return std::numeric_limits<qreal>::max();
        }
        quint64 total = 0;
        for (int y = 0; y < actual.height(); ++y)
        {
            for (int x = 0; x < actual.width(); ++x)
            {
                const QColor a = actual.pixelColor(x, y);
                const QColor e = expected.pixelColor(x, y);
                total += qAbs(a.red() - e.red());
                total += qAbs(a.green() - e.green());
                total += qAbs(a.blue() - e.blue());
                total += qAbs(a.alpha() - e.alpha());
            }
        }
        return static_cast<qreal>(total) / static_cast<qreal>(actual.width() * actual.height() * 4);
    };

    constexpr qreal highDpr = 2.0;
    const QColor iconColor(QStringLiteral("#123456"));
    QPixmapCache::clear();

    AntIcon icon(QStringLiteral("GithubFilled"));
    icon.setIconSize(24);
    icon.setColor(iconColor);
    icon.resize(24, 24);

    QImage reference = renderIconAtDpr(icon, 1.0);
    reference.setDevicePixelRatio(1.0);
    reference = reference.scaled(QSize(qRound(reference.width() * highDpr), qRound(reference.height() * highDpr)),
                                 Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation);

    QPixmapCache::clear();
    const QImage firstRender = renderIconAtDpr(icon, highDpr);
    QCOMPARE(icon.property("antIconRenderCacheSource").toString(), QStringLiteral("resource"));
    QCOMPARE(icon.property("antIconRenderCacheHit").toBool(), false);
    QImage firstPhysical = firstRender;
    firstPhysical.setDevicePixelRatio(1.0);
    const qreal firstDiff = meanPixelDifference(firstPhysical, reference);
    QVERIFY2(firstDiff < 18.0,
             qPrintable(QStringLiteral("High-DPI icon cache miss must render the full icon shape, not a cropped source pixmap. diff=%1")
                        .arg(firstDiff)));

    const QString cacheKey = icon.property("antIconRenderCacheKey").toString();
    QVERIFY(!cacheKey.isEmpty());
    const QImage cachedRender = renderIconAtDpr(icon, highDpr);
    QCOMPARE(icon.property("antIconRenderCacheHit").toBool(), true);
    QCOMPARE(icon.property("antIconRenderCacheKey").toString(), cacheKey);
    QImage cachedPhysical = cachedRender;
    cachedPhysical.setDevicePixelRatio(1.0);
    const qreal cachedDiff = meanPixelDifference(cachedPhysical, firstPhysical);
    QVERIFY2(cachedDiff < 0.5,
             qPrintable(QStringLiteral("High-DPI icon cache hit must match the cache miss rendering. diff=%1")
                        .arg(cachedDiff)));
}

QTEST_MAIN(TestAntIcon)
#include "TestAntIcon.moc"
