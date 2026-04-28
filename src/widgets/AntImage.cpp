#include "AntImage.h"

#include <QApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QVBoxLayout>
#include <QWindow>

#include "AntButton.h"
#include "core/AntTheme.h"

namespace
{

QSize imageTargetSize(const QPixmap& pixmap, int requestedWidth, int requestedHeight)
{
    const QSize natural = pixmap.isNull() ? QSize(200, 200) : pixmap.size();

    if (requestedWidth > 0 && requestedHeight > 0)
    {
        return QSize(requestedWidth, requestedHeight);
    }
    if (requestedWidth > 0)
    {
        const int height = natural.width() > 0 ? qRound(static_cast<qreal>(requestedWidth) * natural.height() / natural.width())
                                               : requestedWidth;
        return QSize(requestedWidth, qMax(1, height));
    }
    if (requestedHeight > 0)
    {
        const int width = natural.height() > 0 ? qRound(static_cast<qreal>(requestedHeight) * natural.width() / natural.height())
                                               : requestedHeight;
        return QSize(qMax(1, width), requestedHeight);
    }
    return natural;
}

class ImagePreviewDialog : public QDialog
{
public:
    explicit ImagePreviewDialog(const QList<QPixmap>& pixmaps, int startIndex, QWidget* parent = nullptr)
        : QDialog(parent, Qt::FramelessWindowHint)
        , m_pixmaps(pixmaps)
        , m_currentIndex(startIndex)
    {
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_DeleteOnClose);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 16, 16, 16);

        // Top row: prev / counter / close
        auto* topRow = new QHBoxLayout();
        if (m_pixmaps.size() > 1)
        {
            m_prevBtn = new AntButton(QStringLiteral("◀"), this);
            m_prevBtn->setButtonType(Ant::ButtonType::Text);
            m_prevBtn->setButtonSize(Ant::Size::Small);
            m_prevBtn->setFixedSize(32, 32);
            topRow->addWidget(m_prevBtn);
        }
        topRow->addStretch();
        m_counterLabel = new QLabel(this);
        m_counterLabel->setAlignment(Qt::AlignCenter);
        topRow->addWidget(m_counterLabel);
        topRow->addStretch();
        auto* closeBtn = new AntButton(QStringLiteral("✕"), this);
        closeBtn->setButtonType(Ant::ButtonType::Text);
        closeBtn->setButtonSize(Ant::Size::Small);
        closeBtn->setFixedSize(32, 32);
        topRow->addWidget(closeBtn);
        if (m_pixmaps.size() > 1)
        {
            m_nextBtn = new AntButton(QStringLiteral("▶"), this);
            m_nextBtn->setButtonType(Ant::ButtonType::Text);
            m_nextBtn->setButtonSize(Ant::Size::Small);
            m_nextBtn->setFixedSize(32, 32);
            topRow->addWidget(m_nextBtn);
        }
        layout->addLayout(topRow);

        m_imgLabel = new QLabel(this);
        m_imgLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(m_imgLabel);

        connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
        if (m_prevBtn)
            connect(m_prevBtn, &QPushButton::clicked, this, [this]() { navigate(-1); });
        if (m_nextBtn)
            connect(m_nextBtn, &QPushButton::clicked, this, [this]() { navigate(1); });

        updateDisplay();
    }

private:
    void navigate(int delta)
    {
        const int count = m_pixmaps.size();
        if (count <= 1) return;
        m_currentIndex = (m_currentIndex + delta + count) % count;
        updateDisplay();
    }

    void updateDisplay()
    {
        const QScreen* screen = QApplication::primaryScreen();
        const QSize maxSz = screen ? screen->availableSize() : QSize(1920, 1080);
        const QPixmap& pix = m_pixmaps[m_currentIndex];
        QSize scaled = pix.size().scaled(maxSz * 0.85, Qt::KeepAspectRatio);
        m_imgLabel->setPixmap(pix.scaled(scaled, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_counterLabel->setText(QStringLiteral("%1 / %2").arg(m_currentIndex + 1).arg(m_pixmaps.size()));
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
        else if (e->key() == Qt::Key_Left) navigate(-1);
        else if (e->key() == Qt::Key_Right) navigate(1);
        QDialog::keyPressEvent(e);
    }

    QList<QPixmap> m_pixmaps;
    int m_currentIndex = 0;
    QLabel* m_imgLabel = nullptr;
    QLabel* m_counterLabel = nullptr;
    AntButton* m_prevBtn = nullptr;
    AntButton* m_nextBtn = nullptr;
};

} // namespace

static void initAntImageResources()
{
    Q_INIT_RESOURCE(qt_ant_design);
}

AntImage::AntImage(QWidget* parent)
    : QWidget(parent)
{
    initAntImageResources();
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

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
    return imageTargetSize(m_pixmap, m_imgWidth, m_imgHeight);
}

void AntImage::paintEvent(QPaintEvent*)
{
    const auto& token = antTheme->tokens();
    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QRectF r = rect();

    if (m_loaded)
    {
        const bool hasExplicitBox = m_imgWidth > 0 && m_imgHeight > 0;
        const QPixmap scaled = m_pixmap.scaled(r.size().toSize(),
                                               hasExplicitBox ? Qt::IgnoreAspectRatio : Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
        const QPointF topLeft(r.x() + (r.width() - scaled.width()) / 2.0,
                              r.y() + (r.height() - scaled.height()) / 2.0);
        p.drawPixmap(topLeft, scaled);
    }
    else
    {
        p.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        p.setBrush(token.colorFillQuaternary);
        p.drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), token.borderRadius, token.borderRadius);

        QFont f = p.font();
        f.setPixelSize(token.fontSizeSM);
        p.setFont(f);
        p.setPen(token.colorTextPlaceholder);
        p.drawText(r, Qt::AlignCenter, m_alt);
    }

    // Preview overlay on hover
    if (m_hovered && m_preview && m_loaded)
    {
        p.fillRect(r, QColor(0, 0, 0, 115));

        const QPointF center = r.center();
        QPainterPath eye;
        eye.moveTo(center.x() - 17.0, center.y() - 10.0);
        eye.cubicTo(center.x() - 8.0, center.y() - 20.0, center.x() + 8.0, center.y() - 20.0,
                    center.x() + 17.0, center.y() - 10.0);
        eye.cubicTo(center.x() + 8.0, center.y(), center.x() - 8.0, center.y(), center.x() - 17.0,
                    center.y() - 10.0);

        p.setPen(token.colorTextLightSolid);
        p.setBrush(Qt::NoBrush);
        p.drawPath(eye);
        p.setBrush(token.colorTextLightSolid);
        p.drawEllipse(QRectF(center.x() - 3.5, center.y() - 13.5, 7.0, 7.0));

        QFont f = p.font();
        f.setPixelSize(token.fontSize);
        p.setFont(f);
        p.drawText(QRectF(r.left(), center.y() + 4.0, r.width(), 22.0), Qt::AlignCenter,
                   QStringLiteral("Preview"));
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

void AntImage::setPreviewGroup(const QList<AntImage*>& group)
{
    m_previewGroup = group;
}

void AntImage::showPreviewDialog()
{
    showPreviewDialogAt(-1);
}

void AntImage::showPreviewDialogAt(int index)
{
    QList<QPixmap> pixmaps;
    int startIndex = 0;

    if (m_previewGroup.isEmpty())
    {
        pixmaps.append(m_pixmap);
        startIndex = 0;
    }
    else
    {
        for (int i = 0; i < m_previewGroup.size(); ++i)
        {
            AntImage* img = m_previewGroup[i];
            if (img && img->m_loaded)
            {
                pixmaps.append(img->m_pixmap);
                if (img == this)
                    startIndex = pixmaps.size() - 1;
            }
        }
        if (index >= 0 && index < pixmaps.size())
            startIndex = index;
    }

    if (pixmaps.isEmpty())
        return;

    auto* dlg = new ImagePreviewDialog(pixmaps, startIndex, window());
    dlg->exec();
}
