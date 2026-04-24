#pragma once

#include <QWidget>
#include <QStringList>
#include <QSize>
#include <QPoint>

struct AntWatermarkFont
{
    QColor color;
    int fontSize = 16;
    int fontWeight = 400;
    QString fontFamily;

    bool operator==(const AntWatermarkFont& other) const
    {
        return color == other.color && fontSize == other.fontSize &&
               fontWeight == other.fontWeight && fontFamily == other.fontFamily;
    }
    bool operator!=(const AntWatermarkFont& other) const { return !(*this == other); }
};

class AntWatermark : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QStringList content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(qreal rotate READ rotate WRITE setRotate NOTIFY rotateChanged)

public:
    explicit AntWatermark(QWidget* parent = nullptr);

    QStringList content() const;
    void setContent(const QStringList& lines);
    void setContent(const QString& text);

    qreal rotate() const;
    void setRotate(qreal degrees);

    AntWatermarkFont watermarkFont() const;
    void setWatermarkFont(const AntWatermarkFont& f);

    QSize gap() const;
    void setGap(const QSize& gap);

    QPoint offset() const;
    void setOffset(const QPoint& offset);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void contentChanged(const QStringList&);
    void rotateChanged(qreal);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QStringList m_content;
    qreal m_rotate = -22.0;
    AntWatermarkFont m_font;
    QSize m_gap = QSize(100, 100);
    QPoint m_offset = QPoint(0, 0);
};
