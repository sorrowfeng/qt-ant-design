#pragma once

#include <QBuffer>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QSize>
#include <QString>
#include <QUrl>

namespace AntImageDecode
{

constexpr qint64 MaxEncodedBytes = 32LL * 1024 * 1024;
constexpr qint64 MaxPixelCount = 64LL * 1024 * 1024;
constexpr qint64 MaxDecodedBytes = 256LL * 1024 * 1024;
constexpr qint64 WorstCaseDecodedBytesPerPixel = 16;
constexpr int MaxDimension = 32768;
constexpr int MaxThumbnailLogicalDimension = 256;
constexpr int MaxThumbnailPhysicalDimension = 1024;
constexpr qint64 MaxThumbnailDecodedBytes = 4LL * 1024 * 1024;

struct SourceStamp
{
    QString normalizedPath;
    qint64 encodedBytes = -1;
    qint64 modifiedMs = 0;
    bool valid = false;

    bool operator==(const SourceStamp& other) const
    {
        return valid == other.valid
            && normalizedPath == other.normalizedPath
            && encodedBytes == other.encodedBytes
            && modifiedMs == other.modifiedMs;
    }

    bool operator!=(const SourceStamp& other) const { return !(*this == other); }
};

struct DecodeMetadata
{
    SourceStamp stamp;
    QByteArray format;
    QSize sourceSize;
    qint64 estimatedDecodedBytes = 0;
};

inline QString localPath(const QString& source)
{
    if (source.startsWith(QStringLiteral(":/")))
    {
        return source;
    }

    const QUrl url(source);
    if (url.isValid() && url.isLocalFile())
    {
        return url.toLocalFile();
    }
    return source;
}

inline SourceStamp sourceStamp(const QString& source, QString* error = nullptr)
{
    SourceStamp stamp;
    const QString path = localPath(source);
    if (path.isEmpty())
    {
        if (error)
            *error = QStringLiteral("image source is empty");
        return stamp;
    }

    if (path.startsWith(QStringLiteral(":/")))
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            if (error)
                *error = QStringLiteral("image source cannot be opened");
            return stamp;
        }
        stamp.normalizedPath = path;
        stamp.encodedBytes = file.size();
        stamp.valid = true;
        return stamp;
    }

    const QFileInfo info(path);
    if (!info.exists() || !info.isFile() || !info.isReadable())
    {
        if (error)
            *error = QStringLiteral("image source is not a readable file");
        return stamp;
    }

    QString normalized = info.canonicalFilePath();
    if (normalized.isEmpty())
        normalized = info.absoluteFilePath();

    stamp.normalizedPath = QDir::cleanPath(normalized);
    stamp.encodedBytes = info.size();
    stamp.modifiedMs = info.lastModified().toMSecsSinceEpoch();
    stamp.valid = true;
    return stamp;
}

inline bool checkedDecodedBytes(const QSize& size, qint64* pixelCount, qint64* decodedBytes)
{
    if (!size.isValid() || size.isEmpty())
        return false;

    const qint64 width = size.width();
    const qint64 height = size.height();
    if (width <= 0 || height <= 0 || width > MaxDimension || height > MaxDimension)
        return false;
    if (width > MaxPixelCount / height)
        return false;

    const qint64 pixels = width * height;
    if (pixels > MaxPixelCount || pixels > MaxDecodedBytes / WorstCaseDecodedBytesPerPixel)
        return false;

    if (pixelCount)
        *pixelCount = pixels;
    if (decodedBytes)
        *decodedBytes = pixels * WorstCaseDecodedBytesPerPixel;
    return true;
}

inline bool read(const QString& source,
                 const QSize& requestedPhysicalSize,
                 QImage* result,
                 DecodeMetadata* metadata,
                 QString* error)
{
    if (result)
        *result = {};
    if (metadata)
        *metadata = {};
    if (error)
        error->clear();

    QString stampError;
    const SourceStamp stamp = sourceStamp(source, &stampError);
    if (!stamp.valid)
    {
        if (error)
            *error = stampError;
        return false;
    }
    QFile file(stamp.normalizedPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        if (error)
            *error = QStringLiteral("image source cannot be opened");
        return false;
    }

    SourceStamp openedStamp = stamp;
    openedStamp.encodedBytes = file.size();
    const QFileInfo openedInfo(file.fileName());
    openedStamp.modifiedMs = openedInfo.lastModified().toMSecsSinceEpoch();
    if (metadata)
        metadata->stamp = openedStamp;
    if (openedStamp.encodedBytes < 0 || openedStamp.encodedBytes > MaxEncodedBytes)
    {
        if (error)
            *error = QStringLiteral("image exceeds encoded byte budget (32 MiB)");
        return false;
    }

    QByteArray encodedSnapshot = file.read(MaxEncodedBytes + 1);
    if (file.error() != QFileDevice::NoError)
    {
        if (error)
            *error = QStringLiteral("image source cannot be read");
        return false;
    }
    openedStamp.encodedBytes = encodedSnapshot.size();
    if (metadata)
        metadata->stamp = openedStamp;
    if (encodedSnapshot.size() > MaxEncodedBytes)
    {
        if (error)
            *error = QStringLiteral("image exceeds encoded byte budget (32 MiB)");
        return false;
    }

    QBuffer snapshotBuffer(&encodedSnapshot);
    if (!snapshotBuffer.open(QIODevice::ReadOnly))
    {
        if (error)
            *error = QStringLiteral("image snapshot cannot be opened");
        return false;
    }

    QImageReader reader(&snapshotBuffer);
    reader.setAutoTransform(true);
    const QByteArray format = reader.format().toLower();
    if (metadata)
        metadata->format = format;
    if (format.isEmpty())
    {
        if (error)
            *error = QStringLiteral("image format is unsupported or invalid");
        return false;
    }

    const QSize sourceSize = reader.size();
    if (metadata)
        metadata->sourceSize = sourceSize;
    qint64 estimatedDecodedBytes = 0;
    if (!checkedDecodedBytes(sourceSize, nullptr, &estimatedDecodedBytes))
    {
        if (error)
            *error = QStringLiteral("image dimensions, pixel count, or worst-case decoded bytes exceed budget (32768 px / 64 Mi pixels / 256 MiB)");
        return false;
    }
    if (metadata)
        metadata->estimatedDecodedBytes = estimatedDecodedBytes;

    QSize physicalTarget = requestedPhysicalSize;
    if (physicalTarget.isValid() && !physicalTarget.isEmpty())
    {
        physicalTarget.setWidth(qBound(1, physicalTarget.width(), MaxThumbnailPhysicalDimension));
        physicalTarget.setHeight(qBound(1, physicalTarget.height(), MaxThumbnailPhysicalDimension));
        physicalTarget = sourceSize.scaled(physicalTarget, Qt::KeepAspectRatio);
        if (physicalTarget.isValid() && !physicalTarget.isEmpty())
            reader.setScaledSize(physicalTarget);
    }

    QImage image = reader.read();
    if (image.isNull())
    {
        if (error)
            *error = reader.errorString().isEmpty() ? QStringLiteral("image decoding failed") : reader.errorString();
        return false;
    }

    if (physicalTarget.isValid() && !physicalTarget.isEmpty() && image.size() != physicalTarget)
        image = image.scaled(physicalTarget, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    const qint64 actualBytes = image.sizeInBytes();
    const qint64 resultBudget = physicalTarget.isValid() && !physicalTarget.isEmpty()
        ? MaxThumbnailDecodedBytes
        : MaxDecodedBytes;
    if (actualBytes < 0 || actualBytes > resultBudget)
    {
        if (error)
            *error = physicalTarget.isValid() && !physicalTarget.isEmpty()
                ? QStringLiteral("decoded thumbnail exceeds byte budget (4 MiB)")
                : QStringLiteral("decoded image exceeds byte budget (256 MiB)");
        return false;
    }

    if (metadata)
    {
        metadata->stamp = openedStamp;
        metadata->format = format;
        metadata->sourceSize = sourceSize;
        metadata->estimatedDecodedBytes = estimatedDecodedBytes;
    }
    if (result)
        *result = image;
    return true;
}

} // namespace AntImageDecode
