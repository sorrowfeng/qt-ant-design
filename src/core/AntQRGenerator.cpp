#include "AntQRGenerator.h"

#include "AntTypes.h"

namespace Ant
{

// Capacity tables: [version][errorLevel] → {total, ecPerBlock, g1, b1, d1, g2, b2, d2}
// Versions 1-10, error levels L(0)/M(1)/Q(2)/H(3)
struct CapEntry
{
    int total, ecPerBlock, g1, b1, d1, g2, b2, d2;
};

static const CapEntry capTable[10][4] = {
    // V1
    {{26,7,1,1,19,0,0,0},{26,10,1,1,16,0,0,0},{26,13,1,1,13,0,0,0},{26,17,1,1,9,0,0,0}},
    // V2
    {{44,10,1,1,34,0,0,0},{44,16,1,1,28,0,0,0},{44,22,1,1,22,0,0,0},{44,28,1,1,16,0,0,0}},
    // V3
    {{70,15,1,1,55,0,0,0},{70,26,1,1,44,0,0,0},{70,18,2,2,17,0,0,0},{70,22,2,2,13,0,0,0}},
    // V4
    {{100,20,1,1,80,0,0,0},{100,18,2,2,32,0,0,0},{100,26,2,2,24,0,0,0},{100,16,4,4,9,0,0,0}},
    // V5
    {{134,26,1,1,108,0,0,0},{134,24,2,2,43,0,0,0},{134,18,2,2,15,2,2,16},{134,22,2,2,11,2,2,12}},
    // V6
    {{172,18,2,2,68,0,0,0},{172,16,4,4,27,0,0,0},{172,24,4,4,19,0,0,0},{172,28,4,4,15,0,0,0}},
    // V7
    {{196,20,2,2,78,0,0,0},{196,18,4,4,31,0,0,0},{196,18,2,2,14,4,4,15},{196,26,4,4,13,1,1,14}},
    // V8
    {{242,24,2,2,97,0,0,0},{242,22,2,2,38,2,2,39},{242,22,4,4,18,2,2,19},{242,26,4,4,14,2,2,15}},
    // V9
    {{292,30,2,2,116,0,0,0},{292,22,3,3,36,2,2,37},{292,20,4,4,16,4,4,17},{292,24,4,4,12,4,4,13}},
    // V10
    {{346,18,2,2,68,2,2,69},{346,26,4,4,43,1,1,44},{346,24,6,6,19,2,2,20},{346,28,6,6,15,2,2,16}},
};

AntQRGenerator::QRInfo AntQRGenerator::qrInfo(int version, QRCodeErrorLevel level)
{
    QRInfo info;
    info.version = version;
    info.dimension = 17 + version * 4;
    int el = static_cast<int>(level);
    const auto& c = capTable[version - 1][el];
    info.totalCodewords = c.total;
    info.ecCodewordsPerBlock = c.ecPerBlock;
    info.groups1 = c.g1;
    info.blocks1 = c.b1;
    info.dataPerBlock1 = c.d1;
    info.groups2 = c.g2;
    info.blocks2 = c.b2;
    info.dataPerBlock2 = c.d2;
    return info;
}

int AntQRGenerator::minVersion(int dataBytes, QRCodeErrorLevel level)
{
    for (int v = 1; v <= 10; ++v)
    {
        auto info = qrInfo(v, level);
        int capacity = info.blocks1 * info.dataPerBlock1 + info.blocks2 * info.dataPerBlock2;
        if (dataBytes <= capacity) return v;
    }
    return 10; // Max supported
}

QVector<int> AntQRGenerator::encodeData(const QString& data, int version, QRCodeErrorLevel level)
{
    QByteArray bytes = data.toUtf8();
    QVector<int> bits;

    // Byte mode indicator: 0100
    bits.append(0); bits.append(1); bits.append(0); bits.append(0);

    // Character count (8 bits for versions 1-9, 16 for 10+)
    int countBits = version <= 9 ? 8 : 16;
    int count = bytes.size();
    for (int i = countBits - 1; i >= 0; --i)
        bits.append((count >> i) & 1);

    // Data bytes
    for (int i = 0; i < bytes.size(); ++i)
    {
        unsigned char b = static_cast<unsigned char>(bytes[i]);
        for (int j = 7; j >= 0; --j)
            bits.append((b >> j) & 1);
    }

    // Terminator (up to 4 zero bits)
    auto info = qrInfo(version, level);
    int capacityBits = (info.blocks1 * info.dataPerBlock1 + info.blocks2 * info.dataPerBlock2) * 8;
    int termBits = qMin(4, capacityBits - bits.size());
    for (int i = 0; i < termBits; ++i)
        bits.append(0);

    // Pad to byte boundary
    while (bits.size() % 8 != 0)
        bits.append(0);

    // Pad bytes: 11101100, 00010001 alternating
    int padBytes = (capacityBits - bits.size()) / 8;
    for (int i = 0; i < padBytes; ++i)
    {
        int pad = (i % 2 == 0) ? 0xEC : 0x11;
        for (int j = 7; j >= 0; --j)
            bits.append((pad >> j) & 1);
    }

    // Convert bits to codewords
    QVector<int> codewords;
    for (int i = 0; i < bits.size(); i += 8)
    {
        int byte = 0;
        for (int j = 0; j < 8 && i + j < bits.size(); ++j)
            byte = (byte << 1) | bits[i + j];
        codewords.append(byte);
    }
    return codewords;
}

QVector<int> AntQRGenerator::interleaveBlocks(const QVector<int>& data, const QRInfo& info)
{
    QVector<QVector<int>> blocks;
    int pos = 0;

    // Group 1
    for (int b = 0; b < info.blocks1; ++b)
    {
        QVector<int> block;
        for (int i = 0; i < info.dataPerBlock1; ++i)
            block.append(data[pos++]);
        blocks.append(block);
    }
    // Group 2
    for (int b = 0; b < info.blocks2; ++b)
    {
        QVector<int> block;
        for (int i = 0; i < info.dataPerBlock2; ++i)
            block.append(data[pos++]);
        blocks.append(block);
    }

    // Interleave: take one byte from each block in turn
    QVector<int> result;
    int maxData = qMax(info.dataPerBlock1, info.dataPerBlock2);
    for (int i = 0; i < maxData; ++i)
    {
        for (const auto& block : blocks)
        {
            if (i < block.size()) result.append(block[i]);
        }
    }
    return result;
}

QVector<int> AntQRGenerator::interleaveEC(const QVector<QVector<int>>& ecBlocks, const QRInfo& info)
{
    QVector<int> result;
    int maxEC = info.ecCodewordsPerBlock;
    for (int i = 0; i < maxEC; ++i)
    {
        for (const auto& block : ecBlocks)
        {
            if (i < block.size()) result.append(block[i]);
        }
    }
    return result;
}

void AntQRGenerator::placeFinderPatterns(QVector<QVector<int>>& matrix)
{
    int dim = matrix.size();
    auto place = [&](int row, int col) {
        for (int r = -1; r <= 7; ++r)
        {
            for (int c = -1; c <= 7; ++c)
            {
                int rr = row + r, cc = col + c;
                if (rr >= 0 && rr < dim && cc >= 0 && cc < dim)
                {
                    const bool dark = r >= 0 && r <= 6 && c >= 0 && c <= 6 &&
                        (r == 0 || r == 6 || c == 0 || c == 6 ||
                         (r >= 2 && r <= 4 && c >= 2 && c <= 4));
                    matrix[rr][cc] = dark ? 1 : 0;
                }
            }
        }
    };
    place(0, 0);
    place(0, dim - 7);
    place(dim - 7, 0);
}

void AntQRGenerator::placeTimingPatterns(QVector<QVector<int>>& matrix)
{
    int dim = matrix.size();
    for (int i = 8; i < dim - 8; ++i)
    {
        int v = (i % 2 == 0) ? 2 : 0;
        if (matrix[6][i] == -1) matrix[6][i] = v;
        if (matrix[i][6] == -1) matrix[i][6] = v;
    }
}

void AntQRGenerator::placeAlignmentPatterns(QVector<QVector<int>>& matrix)
{
    int dim = matrix.size();
    int version = (dim - 17) / 4;
    if (version < 2) return;

    // Alignment pattern positions for versions 2-10
    static const int posTable[9][8] = {
        {6,18,0,0,0,0,0,0},      // V2
        {6,22,0,0,0,0,0,0},      // V3
        {6,26,0,0,0,0,0,0},      // V4
        {6,30,0,0,0,0,0,0},      // V5
        {6,34,0,0,0,0,0,0},      // V6
        {6,22,38,0,0,0,0,0},     // V7
        {6,24,42,0,0,0,0,0},     // V8
        {6,26,46,0,0,0,0,0},     // V9
        {6,28,50,0,0,0,0,0},     // V10
    };

    const int* positions = posTable[version - 2];
    int count = 0;
    while (count < 8 && positions[count] != 0) ++count;

    for (int i = 0; i < count; ++i)
    {
        for (int j = 0; j < count; ++j)
        {
            int row = positions[i], col = positions[j];
            // Skip if overlaps with finder pattern
            if ((row < 9 && col < 9) || (row < 9 && col > dim - 10) ||
                (row > dim - 10 && col < 9))
                continue;

            for (int r = -2; r <= 2; ++r)
            {
                for (int c = -2; c <= 2; ++c)
                {
                    int rr = row + r, cc = col + c;
                    if (rr >= 0 && rr < dim && cc >= 0 && cc < dim && matrix[rr][cc] == -1)
                    {
                        const bool dark = (r == -2 || r == 2 || c == -2 || c == 2) || (r == 0 && c == 0);
                        matrix[rr][cc] = dark ? 1 : 0;
                    }
                }
            }
        }
    }
}

void AntQRGenerator::placeReservedAreas(QVector<QVector<int>>& matrix)
{
    int dim = matrix.size();
    // Format info areas (mark as reserved = 3)
    for (int i = 0; i < 9; ++i)
    {
        if (matrix[i][8] == -1) matrix[i][8] = 0;
        if (matrix[8][i] == -1) matrix[8][i] = 0;
    }
    matrix[8][8] = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (matrix[dim - 1 - i][8] == -1) matrix[dim - 1 - i][8] = 0;
    }
    for (int i = 0; i < 8; ++i)
    {
        if (matrix[8][dim - 1 - i] == -1) matrix[8][dim - 1 - i] = 0;
    }

    // Version info areas (versions 7+)
    if (dim >= 45)
    {
        for (int i = 0; i < 6; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                if (matrix[i][dim - 11 + j] == -1) matrix[i][dim - 11 + j] = 0;
                if (matrix[dim - 11 + j][i] == -1) matrix[dim - 11 + j][i] = 0;
            }
        }
    }
}

void AntQRGenerator::placeData(QVector<QVector<int>>& matrix, const QVector<int>& codewords)
{
    int dim = matrix.size();
    int bitIndex = 0;
    bool upward = true;
    int col = dim - 1;

    while (col > 0)
    {
        if (col == 6) col = 5;
        for (int i = 0; i < dim; ++i)
        {
            int row = upward ? dim - 1 - i : i;
            for (int c = 0; c < 2; ++c)
            {
                int cc = col - c;
                if (matrix[row][cc] == -1)
                {
                    int bit = 0;
                    if (bitIndex < codewords.size() * 8)
                    {
                        bit = (codewords[bitIndex / 8] >> (7 - (bitIndex % 8))) & 1;
                        ++bitIndex;
                    }
                    matrix[row][cc] = bit ? 3 : 2; // Data module; 2=light, 3=dark.
                }
            }
        }
        upward = !upward;
        col -= 2;
    }
}

int AntQRGenerator::applyBestMask(QVector<QVector<int>>& matrix)
{
    int dim = matrix.size();
    int bestMask = 0;
    int bestPenalty = INT_MAX;
    QVector<QVector<int>> bestMatrix;

    for (int mask = 0; mask < 8; ++mask)
    {
        QVector<QVector<int>> test(dim, QVector<int>(dim, 0));
        for (int r = 0; r < dim; ++r)
            for (int c = 0; c < dim; ++c)
                test[r][c] = matrix[r][c];

        applyMask(test, mask);
        placeFormatInfo(test, 0, mask); // Temporary format for penalty eval
        int penalty = evaluatePenalty(test);
        if (penalty < bestPenalty)
        {
            bestPenalty = penalty;
            bestMask = mask;
            bestMatrix = test;
        }
    }
    matrix = bestMatrix;
    return bestMask;
}

void AntQRGenerator::applyMask(QVector<QVector<int>>& matrix, int maskPattern)
{
    int dim = matrix.size();
    for (int r = 0; r < dim; ++r)
    {
        for (int c = 0; c < dim; ++c)
        {
            if (matrix[r][c] < 2) continue; // Skip functional modules
            bool invert = false;
            switch (maskPattern)
            {
            case 0: invert = (r + c) % 2 == 0; break;
            case 1: invert = r % 2 == 0; break;
            case 2: invert = c % 3 == 0; break;
            case 3: invert = (r + c) % 3 == 0; break;
            case 4: invert = ((r / 2) + (c / 3)) % 2 == 0; break;
            case 5: invert = ((r * c) % 2) + ((r * c) % 3) == 0; break;
            case 6: invert = (((r * c) % 2) + ((r * c) % 3)) % 2 == 0; break;
            case 7: invert = (((r + c) % 2) + ((r * c) % 3)) % 2 == 0; break;
            }
            int val = matrix[r][c] == 3 ? 1 : 0;
            matrix[r][c] = (val ^ (invert ? 1 : 0)) ? 3 : 2;
        }
    }
}

int AntQRGenerator::evaluatePenalty(const QVector<QVector<int>>& matrix)
{
    int dim = matrix.size();
    int penalty = 0;

    // Condition 1: Adjacent modules in same color (row)
    for (int r = 0; r < dim; ++r)
    {
        int run = 0;
        int last = -1;
        for (int c = 0; c < dim; ++c)
        {
            int v = (matrix[r][c] == 1 || matrix[r][c] == 3) ? 1 : 0;
            if (v == last) { ++run; }
            else { if (run >= 5) penalty += 3 + (run - 5); run = 1; last = v; }
        }
        if (run >= 5) penalty += 3 + (run - 5);
    }
    // Columns
    for (int c = 0; c < dim; ++c)
    {
        int run = 0;
        int last = -1;
        for (int r = 0; r < dim; ++r)
        {
            int v = (matrix[r][c] == 1 || matrix[r][c] == 3) ? 1 : 0;
            if (v == last) { ++run; }
            else { if (run >= 5) penalty += 3 + (run - 5); run = 1; last = v; }
        }
        if (run >= 5) penalty += 3 + (run - 5);
    }

    // Condition 2: 2x2 blocks of same color
    for (int r = 0; r < dim - 1; ++r)
        for (int c = 0; c < dim - 1; ++c)
            if (((matrix[r][c] == 1 || matrix[r][c] == 3) ? 1 : 0) ==
                    ((matrix[r+1][c] == 1 || matrix[r+1][c] == 3) ? 1 : 0) &&
                ((matrix[r][c] == 1 || matrix[r][c] == 3) ? 1 : 0) ==
                    ((matrix[r][c+1] == 1 || matrix[r][c+1] == 3) ? 1 : 0) &&
                ((matrix[r][c] == 1 || matrix[r][c] == 3) ? 1 : 0) ==
                    ((matrix[r+1][c+1] == 1 || matrix[r+1][c+1] == 3) ? 1 : 0))
                penalty += 3;

    // Condition 3: Finder-like patterns (dark-light-dark-dark-dark-light-dark)
    static const int pattern[7] = {1,0,1,1,1,0,1};
    for (int r = 0; r < dim; ++r)
        for (int c = 0; c < dim - 6; ++c)
        {
            bool match = true;
            for (int k = 0; k < 7; ++k)
                if (((matrix[r][c+k] == 1 || matrix[r][c+k] == 3) ? 1 : 0) != pattern[k]) { match = false; break; }
            if (match) penalty += 40;
        }
    for (int c = 0; c < dim; ++c)
        for (int r = 0; r < dim - 6; ++r)
        {
            bool match = true;
            for (int k = 0; k < 7; ++k)
                if (((matrix[r+k][c] == 1 || matrix[r+k][c] == 3) ? 1 : 0) != pattern[k]) { match = false; break; }
            if (match) penalty += 40;
        }

    // Condition 4: Dark module ratio
    int darkCount = 0;
    for (int r = 0; r < dim; ++r)
        for (int c = 0; c < dim; ++c)
            if (matrix[r][c] == 1 || matrix[r][c] == 3) ++darkCount;
    double ratio = static_cast<double>(darkCount) / (dim * dim);
    int deviation = static_cast<int>(qAbs(ratio * 100 - 50) / 5);
    penalty += deviation * 10;

    return penalty;
}

int AntQRGenerator::formatBits(int errorLevel, int maskPattern)
{
    // Error level → indicator bits
    static const int elBits[4] = {1, 0, 3, 2}; // L, M, Q, H
    int data = (elBits[errorLevel] << 3) | maskPattern;
    // BCH(15,5) encoding
    int code = data << 10;
    int gen = 0x537; // 10100110111
    for (int i = 4; i >= 0; --i)
    {
        if (code & (1 << (i + 10)))
            code ^= gen << i;
    }
    return ((data << 10) | (code & 0x3FF)) ^ 0x5412; // XOR with mask 101010000010010
}

int AntQRGenerator::versionBits(int version)
{
    // BCH(18,6) encoding for version 7+
    int data = version;
    int code = data << 12;
    int gen = 0x1F25; // 1111100100101
    for (int i = 5; i >= 0; --i)
    {
        if (code & (1 << (i + 12)))
            code ^= gen << i;
    }
    return (data << 12) | (code & 0xFFF);
}

void AntQRGenerator::placeFormatInfo(QVector<QVector<int>>& matrix, int errorLevel, int maskPattern)
{
    int dim = matrix.size();
    int format = formatBits(errorLevel, maskPattern);

    // Place format bits
    for (int i = 0; i < 15; ++i)
    {
        int bit = (format >> i) & 1;

        // Around top-left finder
        if (i < 6) { matrix[i][8] = bit; }
        else if (i < 8) { matrix[i + 1][8] = bit; }
        else if (i < 9) { matrix[8][dim - i - 1] = bit; }
        else { matrix[8][14 - i] = bit; }

        // Dark module
        matrix[dim - 8][8] = 1;

        // Top-right / bottom-left
        if (i < 8)
        {
            matrix[8][dim - 1 - i] = bit;
            matrix[dim - 1 - i][8] = bit;
        }
        else
        {
            int idx = 14 - i;
            matrix[8][idx < 1 ? 0 : (idx < 2 ? 1 : idx < 3 ? 2 : idx < 4 ? 3 : idx < 5 ? 4 : idx < 6 ? 5 : idx < 7 ? 7 : 8)] = bit;
        }
    }
}

void AntQRGenerator::placeVersionInfo(QVector<QVector<int>>& matrix, int version)
{
    if (version < 7) return;
    int dim = matrix.size();
    int info = versionBits(version);

    for (int i = 0; i < 18; ++i)
    {
        int bit = (info >> i) & 1;
        int row = i / 3, col = i % 3;
        matrix[row][dim - 11 + col] = bit;
        matrix[dim - 11 + col][row] = bit;
    }
}

QVector<QVector<bool>> AntQRGenerator::generate(const QString& data, QRCodeErrorLevel errorLevel, int version)
{
    if (data.isEmpty())
        return {};

    QByteArray bytes = data.toUtf8();
    // Overhead: mode(4) + count(up to 16) + terminator(up to 4) + byte-pad(up to 7) ≤ 4 bytes
    int dataBytes = bytes.size() + 4;
    int ver = version > 0 ? version : minVersion(dataBytes, errorLevel);
    if (ver < 1) ver = 1;
    if (ver > 10) ver = 10;

    QRInfo info = qrInfo(ver, errorLevel);
    int dim = info.dimension;

    // Encode data
    QVector<int> rawCodewords = encodeData(data, ver, errorLevel);

    // Split into blocks and compute ECC
    int totalDataBlocks = info.blocks1 + info.blocks2;
    QVector<QVector<int>> dataBlocks;
    int pos = 0;
    for (int b = 0; b < info.blocks1; ++b)
    {
        QVector<int> block;
        for (int i = 0; i < info.dataPerBlock1; ++i)
            block.append(rawCodewords[pos++]);
        dataBlocks.append(block);
    }
    for (int b = 0; b < info.blocks2; ++b)
    {
        QVector<int> block;
        for (int i = 0; i < info.dataPerBlock2; ++i)
            block.append(rawCodewords[pos++]);
        dataBlocks.append(block);
    }

    // Generate ECC for each block
    QVector<QVector<int>> ecBlocks;
    for (const auto& block : dataBlocks)
        ecBlocks.append(reedSolomonEncode(block, info.ecCodewordsPerBlock));

    // Interleave data and ECC
    QVector<int> interleavedData = interleaveBlocks(rawCodewords, info);
    QVector<int> interleavedEC = interleaveEC(ecBlocks, info);
    QVector<int> allCodewords = interleavedData + interleavedEC;

    // Build matrix
    QVector<QVector<int>> matrix(dim, QVector<int>(dim, -1));

    placeFinderPatterns(matrix);
    placeTimingPatterns(matrix);
    placeAlignmentPatterns(matrix);
    placeReservedAreas(matrix);
    placeData(matrix, allCodewords);

    // Apply mask and add format/version info
    int bestMask = applyBestMask(matrix);
    // Re-apply the best mask (applyBestMask already did it, but format info needs correct mask)
    placeFormatInfo(matrix, static_cast<int>(errorLevel), bestMask);
    placeVersionInfo(matrix, ver);

    // Convert to bool matrix (dark = true)
    QVector<QVector<bool>> result(dim, QVector<bool>(dim, false));
    for (int r = 0; r < dim; ++r)
        for (int c = 0; c < dim; ++c)
            result[r][c] = matrix[r][c] == 1 || matrix[r][c] == 3;

    return result;
}

} // namespace Ant
