#pragma once

#include "QtAntDesignExport.h"

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "AntTypes.h"

namespace Ant
{

class QT_ANT_DESIGN_EXPORT AntQRGenerator
{
public:
    static constexpr int MinimumSupportedVersion = 1;
    static constexpr int MaximumSupportedVersion = 10;

    enum class GenerationError
    {
        None,
        EmptyData,
        InvalidErrorLevel,
        InvalidVersion,
        DataTooLong
    };

    struct GenerationResult
    {
        QVector<QVector<bool>> matrix;
        GenerationError error = GenerationError::None;
        int version = 0;
        int capacityBytes = 0;

        bool succeeded() const
        {
            return error == GenerationError::None && !matrix.isEmpty();
        }
    };

    static bool isSupportedErrorLevel(QRCodeErrorLevel errorLevel);
    static bool isSupportedVersion(int version);
    static int maximumDataBytes(int version, QRCodeErrorLevel errorLevel);
    // Byte mode only. Version 0 selects the smallest supported version (V1-V10).
    static GenerationResult tryGenerate(const QString& data, QRCodeErrorLevel errorLevel, int version = 0);
    // Compatibility API: returns an empty matrix when tryGenerate() reports an error.
    static QVector<QVector<bool>> generate(const QString& data, QRCodeErrorLevel errorLevel, int version = 0);

private:
    AntQRGenerator() = default;

    struct QRInfo
    {
        int version;
        int dimension;
        int totalCodewords;
        int ecCodewordsPerBlock;
        int groups1;
        int blocks1;
        int dataPerBlock1;
        int groups2;
        int blocks2;
        int dataPerBlock2;
    };

    // GF(256) arithmetic
    static int gfMul(int a, int b);
    static QVector<int> gfPolyMul(const QVector<int>& p, const QVector<int>& q);
    static QVector<int> reedSolomonGenerator(int degree);
    static QVector<int> reedSolomonEncode(const QVector<int>& data, int ecCodewords);

    // Version / capacity tables
    static QRInfo qrInfo(int version, QRCodeErrorLevel level);
    static int minVersion(qint64 dataBytes, QRCodeErrorLevel level);

    // Data encoding
    static QVector<int> encodeData(const QByteArray& data, int version, QRCodeErrorLevel level);
    static QVector<int> interleaveBlocks(const QVector<int>& data, const QRInfo& info);
    static QVector<int> interleaveEC(const QVector<QVector<int>>& ecBlocks, const QRInfo& info);

    // Module placement
    static void placeFinderPatterns(QVector<QVector<int>>& matrix);
    static void placeTimingPatterns(QVector<QVector<int>>& matrix);
    static void placeAlignmentPatterns(QVector<QVector<int>>& matrix);
    static void placeReservedAreas(QVector<QVector<int>>& matrix);
    static void placeData(QVector<QVector<int>>& matrix, const QVector<int>& codewords);
    static int applyBestMask(QVector<QVector<int>>& matrix, int errorLevel);
    static void applyMask(QVector<QVector<int>>& matrix, int maskPattern);
    static int evaluatePenalty(const QVector<QVector<int>>& matrix);
    static void placeFormatInfo(QVector<QVector<int>>& matrix, int errorLevel, int maskPattern);
    static void placeVersionInfo(QVector<QVector<int>>& matrix, int version);

    static int formatBits(int errorLevel, int maskPattern);
    static int versionBits(int version);
};

inline int AntQRGenerator::gfMul(int a, int b)
{
    int result = 0;
    while (b != 0)
    {
        if ((b & 1) != 0)
            result ^= a;
        b >>= 1;
        a <<= 1;
        if ((a & 0x100) != 0)
            a ^= 0x11D;
    }
    return result;
}

inline QVector<int> AntQRGenerator::gfPolyMul(const QVector<int>& p, const QVector<int>& q)
{
    QVector<int> result(p.size() + q.size() - 1, 0);
    for (int i = 0; i < p.size(); ++i)
    {
        for (int j = 0; j < q.size(); ++j)
        {
            result[i + j] ^= gfMul(p[i], q[j]);
        }
    }
    return result;
}

inline QVector<int> AntQRGenerator::reedSolomonGenerator(int degree)
{
    // Generator polynomial: (x - alpha^0) ... (x - alpha^(degree - 1)).
    // Addition and subtraction are both XOR in GF(256), so every factor is
    // represented as {1, alpha^i}, with coefficients ordered highest first.
    QVector<int> result = {1};
    int root = 1;
    for (int i = 0; i < degree; ++i)
    {
        result = gfPolyMul(result, {1, root});
        root = gfMul(root, 2);
    }
    return result;
}

inline QVector<int> AntQRGenerator::reedSolomonEncode(const QVector<int>& data, int ecCodewords)
{
    QVector<int> gen = reedSolomonGenerator(ecCodewords);
    QVector<int> result(data.size() + ecCodewords, 0);
    for (int i = 0; i < data.size(); ++i) result[i] = data[i];

    for (int i = 0; i < data.size(); ++i)
    {
        int factor = result[i];
        if (factor != 0)
        {
            for (int j = 0; j < gen.size(); ++j)
            {
                result[i + j] ^= gfMul(gen[j], factor);
            }
        }
    }
    return result.mid(data.size(), ecCodewords);
}

} // namespace Ant
