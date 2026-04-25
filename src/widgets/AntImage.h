#pragma once

#include <QWidget>
#include <QPixmap>

class AntImage : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString src READ src WRITE setSrc NOTIFY srcChanged)
    Q_PROPERTY(QString alt READ alt WRITE setAlt NOTIFY altChanged)
    Q_PROPERTY(bool preview READ preview WRITE setPreview NOTIFY previewChanged)
    Q_PROPERTY(int imgWidth READ imgWidth WRITE setImgWidth NOTIFY imgWidthChanged)
    Q_PROPERTY(int imgHeight READ imgHeight WRITE setImgHeight NOTIFY imgHeightChanged)

public:
    explicit AntImage(QWidget* parent = nullptr);

    QString src() const;
    void setSrc(const QString& path);

    QString alt() const;
    void setAlt(const QString& text);

    bool preview() const;
    void setPreview(bool enable);

    int imgWidth() const;
    void setImgWidth(int w);

    int imgHeight() const;
    void setImgHeight(int h);

    QSize sizeHint() const override;

Q_SIGNALS:
    void srcChanged(const QString& path);
    void altChanged(const QString& text);
    void previewChanged(bool enable);
    void imgWidthChanged(int w);
    void imgHeightChanged(int h);
    void clicked();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    void showPreviewDialog();

    QString m_src;
    QString m_alt = QStringLiteral("Image");
    bool m_preview = true;
    int m_imgWidth = 0;
    int m_imgHeight = 0;
    bool m_hovered = false;
    QPixmap m_pixmap;
    bool m_loaded = false;
};
