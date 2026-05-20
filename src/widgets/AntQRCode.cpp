#include "AntQRCode.h"

#include <QMouseEvent>
#include <QPainter>
#include <QtMath>

#include "core/AntQRGenerator.h"
#include "core/AntTheme.h"
#include "styles/AntQRCodeStyle.h"

namespace
{
const QString kDefaultQRCodeValue = QStringLiteral("https://github.com/sorrowfeng/qt-ant-design");
}

AntQRCode::AntQRCode(QWidget* parent)
    : QWidget(parent)
{
    installAntStyle<AntQRCodeStyle>(this);
    m_value = kDefaultQRCodeValue;
    regenerateMatrix();
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

QString AntQRCode::value() const { return m_value; }

void AntQRCode::setValue(const QString& value)
{
    if (m_value == value) return;
    m_value = value;
    regenerateMatrix();
    update();
    Q_EMIT valueChanged(m_value);
}

int AntQRCode::qrSize() const { return m_qrSize; }

void AntQRCode::setQrSize(int size)
{
    if (m_qrSize == size) return;
    m_qrSize = size;
    updateGeometry();
    update();
    Q_EMIT qrSizeChanged(m_qrSize);
}

QColor AntQRCode::color() const { return m_color; }

void AntQRCode::setColor(const QColor& color)
{
    if (m_color == color) return;
    m_color = color;
    update();
    Q_EMIT colorChanged(m_color);
}

QColor AntQRCode::bgColor() const { return m_bgColor; }

void AntQRCode::setBgColor(const QColor& color)
{
    if (m_bgColor == color) return;
    m_bgColor = color;
    update();
    Q_EMIT bgColorChanged(m_bgColor);
}

Ant::QRCodeErrorLevel AntQRCode::errorLevel() const { return m_errorLevel; }

void AntQRCode::setErrorLevel(Ant::QRCodeErrorLevel level)
{
    if (m_errorLevel == level) return;
    m_errorLevel = level;
    regenerateMatrix();
    update();
    Q_EMIT errorLevelChanged(m_errorLevel);
}

QIcon AntQRCode::icon() const { return m_icon; }

void AntQRCode::setIcon(const QIcon& icon)
{
    m_icon = icon;
    update();
}

int AntQRCode::iconSize() const { return m_iconSize; }

void AntQRCode::setIconSize(int size)
{
    if (m_iconSize == size) return;
    m_iconSize = size;
    update();
    Q_EMIT iconSizeChanged(m_iconSize);
}

bool AntQRCode::isBordered() const { return m_bordered; }

void AntQRCode::setBordered(bool bordered)
{
    if (m_bordered == bordered) return;
    m_bordered = bordered;
    update();
    Q_EMIT borderedChanged(m_bordered);
}

Ant::QRCodeStatus AntQRCode::status() const { return m_status; }

void AntQRCode::setStatus(Ant::QRCodeStatus status)
{
    if (m_status == status) return;
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

void AntQRCode::refresh()
{
    regenerateMatrix();
    update();
}

QVector<QVector<bool>> AntQRCode::qrMatrix() const { return m_qrMatrix; }

QPixmap AntQRCode::cachedQrPixmap(const QSize& logicalSize, qreal devicePixelRatio, const QColor& foreground) const
{
    const QSize boundedSize(qMax(1, logicalSize.width()), qMax(1, logicalSize.height()));
    const qreal boundedDpr = qMax<qreal>(1.0, devicePixelRatio);
    const int dprKey = qRound(boundedDpr * 1000.0);
    const QRgb colorKey = foreground.rgba();

    const bool cacheValid = !m_cachedQrPixmap.isNull() &&
                            m_cachedQrPixmapSize == boundedSize &&
                            m_cachedQrPixmapDpr == dprKey &&
                            m_cachedQrPixmapColor == colorKey &&
                            m_cachedQrPixmapRevision == m_matrixRevision;
    if (cacheValid)
    {
        return m_cachedQrPixmap;
    }

    const QSize pixelSize(qMax(1, qCeil(boundedSize.width() * boundedDpr)),
                          qMax(1, qCeil(boundedSize.height() * boundedDpr)));
    QPixmap pixmap(pixelSize);
    pixmap.setDevicePixelRatio(boundedDpr);
    pixmap.fill(Qt::transparent);

    const int count = m_qrMatrix.size();
    if (count > 0)
    {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, false);
        const qreal moduleW = static_cast<qreal>(boundedSize.width()) / count;
        const qreal moduleH = static_cast<qreal>(boundedSize.height()) / count;
        for (int row = 0; row < count; ++row)
        {
            for (int col = 0; col < count; ++col)
            {
                if (m_qrMatrix.at(row).at(col))
                {
                    painter.fillRect(QRectF(col * moduleW, row * moduleH, moduleW, moduleH), foreground);
                }
            }
        }
    }

    m_cachedQrPixmap = pixmap;
    m_cachedQrPixmapSize = boundedSize;
    m_cachedQrPixmapDpr = dprKey;
    m_cachedQrPixmapColor = colorKey;
    m_cachedQrPixmapRevision = m_matrixRevision;
    return m_cachedQrPixmap;
}

QSize AntQRCode::sizeHint() const
{
    return QSize(m_qrSize, m_qrSize);
}

QSize AntQRCode::minimumSizeHint() const
{
    return QSize(80, 80);
}

void AntQRCode::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntQRCode::mousePressEvent(QMouseEvent* event)
{
    if (m_status == Ant::QRCodeStatus::Expired)
    {
        const QPoint center = rect().center();
        const QRectF refreshArea(center.x() - 30, center.y() - 30, 60, 60);
        if (refreshArea.contains(event->pos()))
        {
            refresh();
            Q_EMIT refreshClicked();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void AntQRCode::regenerateMatrix()
{
    m_qrMatrix = Ant::AntQRGenerator::generate(m_value, m_errorLevel);
    ++m_matrixRevision;
    invalidateQrPixmapCache();
}

void AntQRCode::invalidateQrPixmapCache()
{
    m_cachedQrPixmap = QPixmap();
    m_cachedQrPixmapSize = QSize();
    m_cachedQrPixmapDpr = 0;
    m_cachedQrPixmapColor = 0;
    m_cachedQrPixmapRevision = -1;
}
