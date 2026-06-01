#include <QImage>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QSignalSpy>
#include <QSvgRenderer>
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

    auto renderSvgRasterReferenceAtDpr = [](const QString& iconName,
                                            const QSize& logicalSize,
                                            qreal dpr,
                                            const QColor& primaryColor,
                                            const QColor& secondaryColor) {
        QFile file(QStringLiteral(":/qt-ant-design/icons/antd/%1.svg").arg(iconName));
        if (!file.open(QIODevice::ReadOnly))
        {
            return QImage();
        }
        QString svg = QString::fromUtf8(file.readAll());
        svg.replace(QStringLiteral("__PRIMARY__"), primaryColor.name(QColor::HexRgb));
        svg.replace(QStringLiteral("__SECONDARY__"), secondaryColor.name(QColor::HexRgb));

        QSvgRenderer renderer(svg.toUtf8());
        if (!renderer.isValid())
        {
            return QImage();
        }

        const QRectF iconRect(1.0, 1.0, logicalSize.width() - 2.0, logicalSize.height() - 2.0);
        const QSize pixelSize(qMax(1, qRound(iconRect.width() * dpr)), qMax(1, qRound(iconRect.height() * dpr)));
        QPixmap pixmap(pixelSize);
        pixmap.fill(Qt::transparent);
        {
            QPainter pixmapPainter(&pixmap);
            pixmapPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
            renderer.render(&pixmapPainter, QRectF(QPointF(0, 0), QSizeF(pixelSize)));
        }

        QImage image(QSize(qRound(logicalSize.width() * dpr), qRound(logicalSize.height() * dpr)),
                     QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        painter.drawPixmap(iconRect, pixmap, QRectF(pixmap.rect()));
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

    const QStringList problemIcons = {
        QStringLiteral("UserOutlined"),
        QStringLiteral("SettingOutlined"),
        QStringLiteral("HeartOutlined"),
        QStringLiteral("AccountBookOutlined"),
    };
    const QColor iconColor(QStringLiteral("#1f1f1f"));
    constexpr qreal maxRasterDiff = 8.0;

    for (const QString& iconName : problemIcons)
    {
        QPixmapCache::clear();
        AntIcon icon(iconName);
        icon.setIconSize(24);
        icon.setColor(iconColor);
        icon.resize(24, 24);

        const QImage reference = renderSvgRasterReferenceAtDpr(icon.resolvedIconName(),
                                                               icon.size(),
                                                               1.0,
                                                               iconColor,
                                                               iconColor);
        QVERIFY2(!reference.isNull(), qPrintable(QStringLiteral("Failed to render raster reference for %1").arg(iconName)));

        const QImage firstRender = renderIconAtDpr(icon, 1.0);
        QCOMPARE(icon.property("antIconRenderCacheSource").toString(), QStringLiteral("resource"));
        QCOMPARE(icon.property("antIconRenderCacheHit").toBool(), false);
        const qreal firstDiff = meanPixelDifference(firstRender, reference);
        QVERIFY2(firstDiff < maxRasterDiff,
                 qPrintable(QStringLiteral("%1 cache miss must match the raster SVG path. diff=%2").arg(iconName).arg(firstDiff)));

        const QImage cachedRender = renderIconAtDpr(icon, 1.0);
        QCOMPARE(icon.property("antIconRenderCacheHit").toBool(), true);
        const qreal cachedDiff = meanPixelDifference(cachedRender, reference);
        QVERIFY2(cachedDiff < maxRasterDiff,
                 qPrintable(QStringLiteral("%1 cache hit must match the raster SVG path. diff=%2").arg(iconName).arg(cachedDiff)));
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

    auto renderSvgReferenceAtDpr = [](const QString& iconName, const QSize& logicalSize, qreal dpr, const QColor& color) {
        QFile file(QStringLiteral(":/qt-ant-design/icons/antd/%1.svg").arg(iconName));
        if (!file.open(QIODevice::ReadOnly))
        {
            return QImage();
        }
        QString svg = QString::fromUtf8(file.readAll());
        svg.replace(QStringLiteral("__PRIMARY__"), color.name(QColor::HexRgb));
        svg.replace(QStringLiteral("__SECONDARY__"), color.name(QColor::HexRgb));

        QSvgRenderer renderer(svg.toUtf8());
        if (!renderer.isValid())
        {
            return QImage();
        }

        const QRectF iconRect(1.0, 1.0, logicalSize.width() - 2.0, logicalSize.height() - 2.0);
        const QSize pixelSize(qMax(1, qRound(iconRect.width() * dpr)), qMax(1, qRound(iconRect.height() * dpr)));
        QPixmap pixmap(pixelSize);
        pixmap.fill(Qt::transparent);
        {
            QPainter pixmapPainter(&pixmap);
            pixmapPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
            renderer.render(&pixmapPainter, QRectF(QPointF(0, 0), QSizeF(pixelSize)));
        }

        QImage image(QSize(qRound(logicalSize.width() * dpr), qRound(logicalSize.height() * dpr)),
                     QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(dpr);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        painter.drawPixmap(iconRect, pixmap, QRectF(pixmap.rect()));
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

    const QImage reference = renderSvgReferenceAtDpr(QStringLiteral("GithubFilled"), icon.size(), highDpr, iconColor);
    QVERIFY(!reference.isNull());
    const QImage firstRender = renderIconAtDpr(icon, highDpr);
    QCOMPARE(icon.property("antIconRenderCacheSource").toString(), QStringLiteral("resource"));
    QCOMPARE(icon.property("antIconRenderCacheHit").toBool(), false);
    const qreal firstDiff = meanPixelDifference(firstRender, reference);
    QVERIFY2(firstDiff < 4.0,
             qPrintable(QStringLiteral("High-DPI icon cache miss must render the full source pixmap, not a cropped top-left quadrant. diff=%1")
                        .arg(firstDiff)));

    const QString cacheKey = icon.property("antIconRenderCacheKey").toString();
    QVERIFY(!cacheKey.isEmpty());
    const QImage cachedRender = renderIconAtDpr(icon, highDpr);
    QCOMPARE(icon.property("antIconRenderCacheHit").toBool(), true);
    QCOMPARE(icon.property("antIconRenderCacheKey").toString(), cacheKey);
    const qreal cachedDiff = meanPixelDifference(cachedRender, reference);
    QVERIFY2(cachedDiff < 4.0,
             qPrintable(QStringLiteral("High-DPI icon cache hit must render the full source pixmap, not a cropped top-left quadrant. diff=%1")
                        .arg(cachedDiff)));
}

QTEST_MAIN(TestAntIcon)
#include "TestAntIcon.moc"
