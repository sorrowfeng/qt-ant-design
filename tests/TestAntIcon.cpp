#include <QImage>
#include <QPainter>
#include <QPixmapCache>
#include <QSignalSpy>
#include <QTest>
#include <QList>

#include "widgets/AntIcon.h"

class TestAntIcon : public QObject
{
    Q_OBJECT
private slots:
    void propertiesAndSignals();
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

QTEST_MAIN(TestAntIcon)
#include "TestAntIcon.moc"
