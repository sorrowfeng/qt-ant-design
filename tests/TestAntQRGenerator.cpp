#include <QByteArray>
#include <QRandomGenerator>
#include <QTest>
#include <QVector>

#include <limits>

#include "core/AntQRGenerator.h"

namespace
{
using BoolMatrix = QVector<QVector<bool>>;

struct BlockInfo
{
    int total;
    int ecPerBlock;
    int blocks1;
    int data1;
    int blocks2;
    int data2;
};

// Model 2 reference block structure for versions 1-10, ordered L/M/Q/H.
static const BlockInfo blockTable[10][4] = {
    {{26,7,1,19,0,0}, {26,10,1,16,0,0}, {26,13,1,13,0,0}, {26,17,1,9,0,0}},
    {{44,10,1,34,0,0}, {44,16,1,28,0,0}, {44,22,1,22,0,0}, {44,28,1,16,0,0}},
    {{70,15,1,55,0,0}, {70,26,1,44,0,0}, {70,18,2,17,0,0}, {70,22,2,13,0,0}},
    {{100,20,1,80,0,0}, {100,18,2,32,0,0}, {100,26,2,24,0,0}, {100,16,4,9,0,0}},
    {{134,26,1,108,0,0}, {134,24,2,43,0,0}, {134,18,2,15,2,16}, {134,22,2,11,2,12}},
    {{172,18,2,68,0,0}, {172,16,4,27,0,0}, {172,24,4,19,0,0}, {172,28,4,15,0,0}},
    {{196,20,2,78,0,0}, {196,18,4,31,0,0}, {196,18,2,14,4,15}, {196,26,4,13,1,14}},
    {{242,24,2,97,0,0}, {242,22,2,38,2,39}, {242,22,4,18,2,19}, {242,26,4,14,2,15}},
    {{292,30,2,116,0,0}, {292,22,3,36,2,37}, {292,20,4,16,4,17}, {292,24,4,12,4,13}},
    {{346,18,2,68,2,69}, {346,26,4,43,1,44}, {346,24,6,19,2,20}, {346,28,6,15,2,16}},
};

struct DecodeResult
{
    bool valid = false;
    QByteArray payload;
    QString error;
    int mask = -1;
};

int formatBits(Ant::QRCodeErrorLevel level, int mask)
{
    static const int levelBits[4] = {1, 0, 3, 2};
    const int data = (levelBits[static_cast<int>(level)] << 3) | mask;
    int remainder = data << 10;
    for (int bit = 14; bit >= 10; --bit)
        if ((remainder & (1 << bit)) != 0)
            remainder ^= 0x537 << (bit - 10);
    return ((data << 10) | (remainder & 0x3FF)) ^ 0x5412;
}

int versionBits(int version)
{
    int remainder = version << 12;
    for (int bit = 17; bit >= 12; --bit)
        if ((remainder & (1 << bit)) != 0)
            remainder ^= 0x1F25 << (bit - 12);
    return (version << 12) | (remainder & 0xFFF);
}

bool maskBit(int mask, int row, int column)
{
    switch (mask)
    {
    case 0: return (row + column) % 2 == 0;
    case 1: return row % 2 == 0;
    case 2: return column % 3 == 0;
    case 3: return (row + column) % 3 == 0;
    case 4: return (row / 2 + column / 3) % 2 == 0;
    case 5: return (row * column) % 2 + (row * column) % 3 == 0;
    case 6: return ((row * column) % 2 + (row * column) % 3) % 2 == 0;
    case 7: return ((row + column) % 2 + (row * column) % 3) % 2 == 0;
    default: return false;
    }
}

QVector<int> alignmentPositions(int version)
{
    static const int table[9][3] = {
        {6,18,0}, {6,22,0}, {6,26,0}, {6,30,0}, {6,34,0},
        {6,22,38}, {6,24,42}, {6,26,46}, {6,28,50},
    };
    QVector<int> result;
    if (version >= 2)
        for (int value : table[version - 2])
            if (value != 0)
                result.append(value);
    return result;
}

BoolMatrix functionMap(int version)
{
    const int dimension = 17 + version * 4;
    BoolMatrix result(dimension, QVector<bool>(dimension, false));
    const auto mark = [&](int row, int column) {
        if (row >= 0 && row < dimension && column >= 0 && column < dimension)
            result[row][column] = true;
    };
    const auto finder = [&](int row, int column) {
        for (int dr = -1; dr <= 7; ++dr)
            for (int dc = -1; dc <= 7; ++dc)
                mark(row + dr, column + dc);
    };
    finder(0, 0);
    finder(0, dimension - 7);
    finder(dimension - 7, 0);

    for (int i = 8; i < dimension - 8; ++i)
    {
        mark(6, i);
        mark(i, 6);
    }
    const QVector<int> positions = alignmentPositions(version);
    for (int row : positions)
        for (int column : positions)
        {
            if ((row < 9 && column < 9) || (row < 9 && column > dimension - 10) ||
                (row > dimension - 10 && column < 9))
                continue;
            for (int dr = -2; dr <= 2; ++dr)
                for (int dc = -2; dc <= 2; ++dc)
                    mark(row + dr, column + dc);
        }

    for (int i = 0; i <= 5; ++i) mark(i, 8);
    mark(7, 8);
    mark(8, 8);
    mark(8, 7);
    for (int i = 9; i < 15; ++i) mark(8, 14 - i);
    for (int i = 0; i < 8; ++i) mark(8, dimension - 1 - i);
    for (int i = 8; i < 15; ++i) mark(dimension - 15 + i, 8);
    mark(dimension - 8, 8);

    if (version >= 7)
        for (int i = 0; i < 18; ++i)
        {
            const int a = dimension - 11 + i % 3;
            const int b = i / 3;
            mark(b, a);
            mark(a, b);
        }
    return result;
}

void writeFormat(BoolMatrix& matrix, Ant::QRCodeErrorLevel level, int mask)
{
    const int dimension = matrix.size();
    const int bits = formatBits(level, mask);
    for (int i = 0; i <= 5; ++i) matrix[i][8] = ((bits >> i) & 1) != 0;
    matrix[7][8] = ((bits >> 6) & 1) != 0;
    matrix[8][8] = ((bits >> 7) & 1) != 0;
    matrix[8][7] = ((bits >> 8) & 1) != 0;
    for (int i = 9; i < 15; ++i) matrix[8][14 - i] = ((bits >> i) & 1) != 0;
    for (int i = 0; i < 8; ++i) matrix[8][dimension - 1 - i] = ((bits >> i) & 1) != 0;
    for (int i = 8; i < 15; ++i) matrix[dimension - 15 + i][8] = ((bits >> i) & 1) != 0;
    matrix[dimension - 8][8] = true;
}

int firstFormatCopy(const BoolMatrix& matrix)
{
    int result = 0;
    for (int i = 0; i <= 5; ++i) result |= static_cast<int>(matrix[i][8]) << i;
    result |= static_cast<int>(matrix[7][8]) << 6;
    result |= static_cast<int>(matrix[8][8]) << 7;
    result |= static_cast<int>(matrix[8][7]) << 8;
    for (int i = 9; i < 15; ++i) result |= static_cast<int>(matrix[8][14 - i]) << i;
    return result;
}

int secondFormatCopy(const BoolMatrix& matrix)
{
    const int dimension = matrix.size();
    int result = 0;
    for (int i = 0; i < 8; ++i)
        result |= static_cast<int>(matrix[8][dimension - 1 - i]) << i;
    for (int i = 8; i < 15; ++i)
        result |= static_cast<int>(matrix[dimension - 15 + i][8]) << i;
    return result;
}

QString validatePatterns(const BoolMatrix& matrix, int version)
{
    const int dimension = matrix.size();
    const auto finder = [&](int row, int column) -> QString {
        for (int dr = -1; dr <= 7; ++dr)
            for (int dc = -1; dc <= 7; ++dc)
            {
                const int rr = row + dr;
                const int cc = column + dc;
                if (rr < 0 || rr >= dimension || cc < 0 || cc >= dimension)
                    continue;
                const bool inside = dr >= 0 && dr <= 6 && dc >= 0 && dc <= 6;
                const bool expected = inside &&
                    (dr == 0 || dr == 6 || dc == 0 || dc == 6 ||
                     (dr >= 2 && dr <= 4 && dc >= 2 && dc <= 4));
                if (matrix[rr][cc] != expected)
                    return QStringLiteral("finder/separator mismatch at (%1,%2)").arg(rr).arg(cc);
            }
        return {};
    };
    for (const auto& origin : {qMakePair(0, 0), qMakePair(0, dimension - 7),
                               qMakePair(dimension - 7, 0)})
    {
        const QString error = finder(origin.first, origin.second);
        if (!error.isEmpty()) return error;
    }

    for (int i = 8; i < dimension - 8; ++i)
        if (matrix[6][i] != (i % 2 == 0) || matrix[i][6] != (i % 2 == 0))
            return QStringLiteral("timing pattern mismatch at index %1").arg(i);

    const QVector<int> positions = alignmentPositions(version);
    for (int row : positions)
        for (int column : positions)
        {
            if ((row < 9 && column < 9) || (row < 9 && column > dimension - 10) ||
                (row > dimension - 10 && column < 9))
                continue;
            for (int dr = -2; dr <= 2; ++dr)
                for (int dc = -2; dc <= 2; ++dc)
                {
                    const bool expected = dr == -2 || dr == 2 || dc == -2 || dc == 2 ||
                                          (dr == 0 && dc == 0);
                    if (matrix[row + dr][column + dc] != expected)
                        return QStringLiteral("alignment mismatch at (%1,%2)")
                            .arg(row + dr).arg(column + dc);
                }
        }
    if (!matrix[dimension - 8][8])
        return QStringLiteral("fixed dark module is light");
    return {};
}

int penaltyScore(const BoolMatrix& matrix)
{
    const int dimension = matrix.size();
    int score = 0;
    const auto lineScore = [&](bool column, int line) {
        int subtotal = 0;
        int last = -1;
        int run = 0;
        for (int offset = 0; offset < dimension; ++offset)
        {
            const int value = column ? matrix[offset][line] : matrix[line][offset];
            if (value == last) ++run;
            else
            {
                if (run >= 5) subtotal += 3 + run - 5;
                last = value;
                run = 1;
            }
        }
        if (run >= 5) subtotal += 3 + run - 5;
        return subtotal;
    };
    for (int i = 0; i < dimension; ++i)
        score += lineScore(false, i) + lineScore(true, i);

    for (int row = 0; row < dimension - 1; ++row)
        for (int column = 0; column < dimension - 1; ++column)
        {
            const bool value = matrix[row][column];
            if (matrix[row + 1][column] == value && matrix[row][column + 1] == value &&
                matrix[row + 1][column + 1] == value)
                score += 3;
        }

    const auto finderLikeScore = [&](bool column, int line) {
        int subtotal = 0;
        for (int offset = 0; offset <= dimension - 11; ++offset)
        {
            int bits = 0;
            for (int i = 0; i < 11; ++i)
                bits = (bits << 1) | static_cast<int>(column ? matrix[offset + i][line]
                                                             : matrix[line][offset + i]);
            if (bits == 0x05D || bits == 0x5D0)
                subtotal += 40;
        }
        return subtotal;
    };
    for (int i = 0; i < dimension; ++i)
        score += finderLikeScore(false, i) + finderLikeScore(true, i);

    int dark = 0;
    for (const auto& row : matrix)
        for (bool module : row)
            dark += module ? 1 : 0;
    const int total = dimension * dimension;
    score += (qAbs(dark * 20 - total * 10) / total) * 10;
    return score;
}

int gfMultiply(int a, int b)
{
    int result = 0;
    while (b != 0)
    {
        if ((b & 1) != 0) result ^= a;
        b >>= 1;
        a <<= 1;
        if ((a & 0x100) != 0) a ^= 0x11D;
    }
    return result;
}

int gfPowerOfTwo(int exponent)
{
    int result = 1;
    for (int i = 0; i < exponent; ++i) result = gfMultiply(result, 2);
    return result;
}

DecodeResult decodeByteMode(const BoolMatrix& matrix, int version, Ant::QRCodeErrorLevel level)
{
    DecodeResult result;
    const auto fail = [&](const QString& error) {
        result.error = error;
        return result;
    };
    const int dimension = 17 + version * 4;
    if (matrix.size() != dimension) return fail(QStringLiteral("unexpected dimension"));
    for (const auto& row : matrix)
        if (row.size() != dimension) return fail(QStringLiteral("matrix is not square"));

    const QString patternError = validatePatterns(matrix, version);
    if (!patternError.isEmpty()) return fail(patternError);

    const int firstFormat = firstFormatCopy(matrix);
    if (firstFormat != secondFormatCopy(matrix))
        return fail(QStringLiteral("format information copies differ"));
    for (int mask = 0; mask < 8; ++mask)
        if (firstFormat == formatBits(level, mask)) result.mask = mask;
    if (result.mask < 0)
        return fail(QStringLiteral("format information BCH value is invalid"));

    if (version >= 7)
    {
        int first = 0;
        int second = 0;
        for (int i = 0; i < 18; ++i)
        {
            const int a = dimension - 11 + i % 3;
            const int b = i / 3;
            first |= static_cast<int>(matrix[b][a]) << i;
            second |= static_cast<int>(matrix[a][b]) << i;
        }
        if (first != versionBits(version) || second != first)
            return fail(QStringLiteral("version information BCH value is invalid"));
    }

    const BoolMatrix function = functionMap(version);
    BoolMatrix unmasked = matrix;
    for (int row = 0; row < dimension; ++row)
        for (int column = 0; column < dimension; ++column)
            if (!function[row][column])
                unmasked[row][column] = matrix[row][column] ^ maskBit(result.mask, row, column);

    int bestMask = -1;
    int bestPenalty = std::numeric_limits<int>::max();
    for (int mask = 0; mask < 8; ++mask)
    {
        BoolMatrix candidate = unmasked;
        for (int row = 0; row < dimension; ++row)
            for (int column = 0; column < dimension; ++column)
                if (!function[row][column])
                    candidate[row][column] = unmasked[row][column] ^ maskBit(mask, row, column);
        writeFormat(candidate, level, mask);
        const int penalty = penaltyScore(candidate);
        if (penalty < bestPenalty)
        {
            bestPenalty = penalty;
            bestMask = mask;
        }
    }
    if (result.mask != bestMask)
        return fail(QStringLiteral("mask %1 is not minimum-penalty mask %2")
                    .arg(result.mask).arg(bestMask));

    QVector<int> bits;
    bool upward = true;
    for (int column = dimension - 1; column > 0; column -= 2)
    {
        if (column == 6) --column;
        for (int i = 0; i < dimension; ++i)
        {
            const int row = upward ? dimension - 1 - i : i;
            for (int offset = 0; offset < 2; ++offset)
                if (!function[row][column - offset])
                    bits.append(unmasked[row][column - offset] ? 1 : 0);
        }
        upward = !upward;
    }

    const BlockInfo& info = blockTable[version - 1][static_cast<int>(level)];
    static const int remainderBits[10] = {0, 7, 7, 7, 7, 7, 0, 0, 0, 0};
    if (bits.size() != info.total * 8 + remainderBits[version - 1])
        return fail(QStringLiteral("unexpected data/remainder module count"));
    for (int i = info.total * 8; i < bits.size(); ++i)
        if (bits[i] != 0) return fail(QStringLiteral("non-zero remainder bit"));

    QVector<int> codewords(info.total, 0);
    for (int i = 0; i < info.total; ++i)
        for (int bit = 0; bit < 8; ++bit)
            codewords[i] = (codewords[i] << 1) | bits[i * 8 + bit];

    QVector<int> sizes;
    for (int i = 0; i < info.blocks1; ++i) sizes.append(info.data1);
    for (int i = 0; i < info.blocks2; ++i) sizes.append(info.data2);
    QVector<QVector<int>> dataBlocks;
    for (int size : sizes) dataBlocks.append(QVector<int>(size, 0));
    const int dataCount = info.blocks1 * info.data1 + info.blocks2 * info.data2;
    int position = 0;
    for (int i = 0; i < qMax(info.data1, info.data2); ++i)
        for (int block = 0; block < sizes.size(); ++block)
            if (i < sizes[block]) dataBlocks[block][i] = codewords[position++];
    if (position != dataCount) return fail(QStringLiteral("data interleave mismatch"));

    QVector<QVector<int>> ecBlocks(sizes.size(), QVector<int>(info.ecPerBlock, 0));
    for (int i = 0; i < info.ecPerBlock; ++i)
        for (int block = 0; block < sizes.size(); ++block)
            ecBlocks[block][i] = codewords[position++];
    if (position != info.total) return fail(QStringLiteral("EC interleave mismatch"));

    for (int block = 0; block < sizes.size(); ++block)
    {
        const QVector<int> complete = dataBlocks[block] + ecBlocks[block];
        for (int rootIndex = 0; rootIndex < info.ecPerBlock; ++rootIndex)
        {
            const int root = gfPowerOfTwo(rootIndex);
            int syndrome = 0;
            for (int codeword : complete) syndrome = gfMultiply(syndrome, root) ^ codeword;
            if (syndrome != 0)
                return fail(QStringLiteral("non-zero RS syndrome %1 in block %2/root %3")
                            .arg(syndrome).arg(block).arg(rootIndex));
        }
    }

    QVector<int> rawData;
    for (const auto& block : dataBlocks) rawData += block;
    int bitPosition = 0;
    const auto readBits = [&](int count, bool& ok) {
        int value = 0;
        if (bitPosition + count > rawData.size() * 8)
        {
            ok = false;
            return value;
        }
        for (int i = 0; i < count; ++i, ++bitPosition)
            value = (value << 1) |
                    ((rawData[bitPosition / 8] >> (7 - bitPosition % 8)) & 1);
        return value;
    };
    bool ok = true;
    if (readBits(4, ok) != 0x4 || !ok) return fail(QStringLiteral("not byte mode"));
    const int byteCount = readBits(version <= 9 ? 8 : 16, ok);
    if (!ok || bitPosition + byteCount * 8 > rawData.size() * 8)
        return fail(QStringLiteral("invalid byte count"));
    for (int i = 0; i < byteCount; ++i)
        result.payload.append(static_cast<char>(readBits(8, ok)));
    if (!ok) return fail(QStringLiteral("truncated payload"));
    result.valid = true;
    return result;
}

QByteArray capacityPayload(int size)
{
    QByteArray result;
    result.reserve(size);
    for (int i = 0; i < size; ++i) result.append(static_cast<char>('!' + i % 90));
    return result;
}
} // namespace

class TestAntQRGenerator : public QObject
{
    Q_OBJECT
private slots:
    void maximumCapacityRoundTrip_data();
    void maximumCapacityRoundTrip();
    void utf8RoundTrip();
    void randomizedUtf8Properties();
};

void TestAntQRGenerator::maximumCapacityRoundTrip_data()
{
    QTest::addColumn<int>("version");
    QTest::addColumn<int>("levelValue");
    static const char* names[4] = {"L", "M", "Q", "H"};
    for (int version = 1; version <= 10; ++version)
        for (int level = 0; level < 4; ++level)
        {
            const QByteArray name = QByteArrayLiteral("V") + QByteArray::number(version) + '-' + names[level];
            QTest::newRow(name.constData()) << version << level;
        }
}

void TestAntQRGenerator::maximumCapacityRoundTrip()
{
    QFETCH(int, version);
    QFETCH(int, levelValue);
    const auto level = static_cast<Ant::QRCodeErrorLevel>(levelValue);
    const int capacity = Ant::AntQRGenerator::maximumDataBytes(version, level);
    const QByteArray payload = capacityPayload(capacity);
    const auto generated = Ant::AntQRGenerator::tryGenerate(QString::fromLatin1(payload), level, version);
    QVERIFY2(generated.succeeded(), "maximum-capacity payload must generate");
    const DecodeResult decoded = decodeByteMode(generated.matrix, version, level);
    QVERIFY2(decoded.valid, qPrintable(decoded.error));
    QCOMPARE(decoded.payload, payload);

    const auto oversized = Ant::AntQRGenerator::tryGenerate(
        QString::fromLatin1(payload + 'X'), level, version);
    QCOMPARE(static_cast<int>(oversized.error),
             static_cast<int>(Ant::AntQRGenerator::GenerationError::DataTooLong));
    QVERIFY(oversized.matrix.isEmpty());
}

void TestAntQRGenerator::utf8RoundTrip()
{
    const QString value = QStringLiteral("QR Model 2: \u4e2d\u6587/\u03ba\u03cc\u03c3\u03bc\u03b5/\u65e5\u672c\u8a9e");
    const auto generated = Ant::AntQRGenerator::tryGenerate(value, Ant::QRCodeErrorLevel::Q);
    QVERIFY(generated.succeeded());
    const DecodeResult decoded = decodeByteMode(
        generated.matrix, generated.version, Ant::QRCodeErrorLevel::Q);
    QVERIFY2(decoded.valid, qPrintable(decoded.error));
    QCOMPARE(decoded.payload, value.toUtf8());
}

void TestAntQRGenerator::randomizedUtf8Properties()
{
    QRandomGenerator random(0x71524f50u);
    const QStringList fragments{
        QStringLiteral("A"),
        QStringLiteral("9"),
        QStringLiteral("中"),
        QStringLiteral("κόσμε"),
        QString::fromUtf8("\xF0\x9F\xA7\xAA")
    };

    for (int iteration = 0; iteration < 512; ++iteration)
    {
        QString value;
        const int fragmentCount = static_cast<int>(random.bounded(220u));
        for (int index = 0; index < fragmentCount; ++index)
        {
            value += fragments.at(static_cast<int>(random.bounded(
                static_cast<quint32>(fragments.size()))));
        }

        const int rawLevel = static_cast<int>(random.bounded(6u)) - 1;
        const int requestedVersion = static_cast<int>(random.bounded(15u)) - 2;
        const auto level = static_cast<Ant::QRCodeErrorLevel>(rawLevel);
        const auto generated = Ant::AntQRGenerator::tryGenerate(value, level, requestedVersion);

        if (!Ant::AntQRGenerator::isSupportedErrorLevel(level))
        {
            QCOMPARE(generated.error, Ant::AntQRGenerator::GenerationError::InvalidErrorLevel);
            QVERIFY(generated.matrix.isEmpty());
            continue;
        }
        if (requestedVersion != 0 && !Ant::AntQRGenerator::isSupportedVersion(requestedVersion))
        {
            QCOMPARE(generated.error, Ant::AntQRGenerator::GenerationError::InvalidVersion);
            QVERIFY(generated.matrix.isEmpty());
            continue;
        }
        if (value.isEmpty())
        {
            QCOMPARE(generated.error, Ant::AntQRGenerator::GenerationError::EmptyData);
            QVERIFY(generated.matrix.isEmpty());
            continue;
        }
        if (!generated.succeeded())
        {
            QCOMPARE(generated.error, Ant::AntQRGenerator::GenerationError::DataTooLong);
            QVERIFY(generated.matrix.isEmpty());
            continue;
        }

        QVERIFY(Ant::AntQRGenerator::isSupportedVersion(generated.version));
        if (requestedVersion != 0)
        {
            QCOMPARE(generated.version, requestedVersion);
        }
        const DecodeResult decoded = decodeByteMode(generated.matrix, generated.version, level);
        QVERIFY2(decoded.valid, qPrintable(decoded.error));
        QCOMPARE(decoded.payload, value.toUtf8());
    }
}

QTEST_APPLESS_MAIN(TestAntQRGenerator)
#include "TestAntQRGenerator.moc"
