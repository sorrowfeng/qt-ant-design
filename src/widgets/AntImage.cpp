#include "AntImage.h"

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>

#include "AntButton.h"
#include "core/AntTheme.h"

namespace
{

class ImagePreviewDialog : public QDialog
{
public:
    explicit ImagePreviewDialog(const QPixmap& pix, QWidget* parent = nullptr)
        : QDialog(parent, Qt::FramelessWindowHint)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_DeleteOnClose);

        const QScreen* screen = QApplication::primaryScreen();
        const QSize maxSz = screen ? screen->availableSize() : QSize(1920, 1080);
        QSize scaled = pix.size().scaled(maxSz * 0.9, Qt::KeepAspectRatio);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 16, 16, 16);

        // Close button
        auto* closeBtn = new AntButton(QStringLiteral("✕"), this);
        closeBtn->setButtonType(Ant::ButtonType::Text);
        closeBtn->setButtonSize(Ant::ButtonSize::Small);
        closeBtn->setFixedSize(32, 32);
        auto* topRow = new QHBoxLayout();
        topRow->addStretch();
        topRow->addWidget(closeBtn);
        layout->addLayout(topRow);

        auto* imgLabel = new QLabel(this);
        imgLabel->setPixmap(pix.scaled(scaled, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imgLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(imgLabel);

        connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);

        setFixedSize(scaled.width() + 32, scaled.height() + 60);
        move((maxSz.width() - width()) / 2, (maxSz.height() - height()) / 2);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), QColor(0, 0, 0, 190));
        p.setPen(QPen(token.colorBorderSecondary, 1));
        p.setBrush(token.colorBgElevated);
        p.drawRoundedRect(rect().adjusted(8, 8, -8, -8), 8, 8);
    }

    void keyPressEvent(QKeyEvent* e) override
    {
        if (e->key() == Qt::Key_Escape) close();
        QDialog::keyPressEvent(e);
    }
};

} // namespace

AntImage::AntImage(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    connect(antTheme, &AntTheme::themeChanged, this, [this]() { update(); });
}

QString AntImage::src() const { return m_src; }

void AntImage::setSrc(const QString& path)
{
    if (m_src == path) return;
    m_src = path;
    m_loaded = m_pixmap.load(path);
    updateGeometry();
    update();
    Q_EMIT srcChanged(m_src);
}

QString AntImage::alt() const { return m_alt; }
void AntImage::setAlt(const QString& text)
{
    if (m_alt == text) return;
    m_alt = text;
    update();
    Q_EMIT altChanged(m_alt);
}

bool AntImage::preview() const { return m_preview; }
void AntImage::setPreview(bool enable)
{
    if (m_preview == enable) return;
    m_preview = enable;
    Q_EMIT previewChanged(m_preview);
}

int AntImage::imgWidth() const { return m_imgWidth; }
void AntImage::setImgWidth(int w)
{
    if (m_imgWidth == w) return;
    m_imgWidth = w;
    updateGeometry();
    update();
    Q_EMIT imgWidthChanged(m_imgWidth);
}

int AntImage::imgHeight() const { return m_imgHeight; }
void AntImage::setImgHeight(int h)
{
    if (m_imgHeight == h) return;
    m_imgHeight = h;
    updateGeometry();
    update();
    Q_EMIT imgHeightChanged(m_imgHeight);
}

QSize AntImage::sizeHint() const
{
    if (m_loaded && m_imgWidth == 0 && m_imgHeight == 0)
        return m_pixmap.size();
    int w = m_imgWidth > 0 ? m_imgWidth : (m_loaded ? m_pixmap.width() : 200);
    int h = m_imgHeight > 0 ? m_imgHeight : (m_loaded ? m_pixmap.height() : 200);
    return QSize(w, h);
}

void AntImage::paintEvent(QPaintEvent*)
{
    const auto& token = antTheme->tokens();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QRectF r = rect();
    p.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
    p.setBrush(token.colorFillQuaternary);
    p.drawRoundedRect(r, token.borderRadius, token.borderRadius);

    if (m_loaded)
    {
        QRectF imgRect = r.adjusted(4, 4, -4, -4);
        p.drawPixmap(imgRect.toRect(), m_pixmap.scaled(imgRect.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else
    {
        QFont f = p.font();
        f.setPixelSize(token.fontSizeSM);
        p.setFont(f);
        p.setPen(token.colorTextPlaceholder);
        p.drawText(r, Qt::AlignCenter, m_alt);
    }

    // Preview overlay on hover
    if (m_hovered && m_preview && m_loaded)
    {
        p.fillRect(r, QColor(0, 0, 0, 30));
        p.setPen(token.colorTextLightSolid);
        p.drawText(r, Qt::AlignCenter, QStringLiteral("⿡\nPreview"));
    }
}

void AntImage::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        Q_EMIT clicked();
        if (m_preview && m_loaded)
            showPreviewDialog();
    }
    QWidget::mousePressEvent(e);
}

void AntImage::enterEvent(QEnterEvent*)
{
    m_hovered = true;
    update();
}

void AntImage::leaveEvent(QEvent*)
{
    m_hovered = false;
    update();
}

void AntImage::showPreviewDialog()
{
    auto* dlg = new ImagePreviewDialog(m_pixmap, window());
    dlg->exec();
}
