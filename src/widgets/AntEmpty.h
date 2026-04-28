#pragma once

#include <QPointer>
#include <QWidget>

class AntEmpty : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(bool imageVisible READ imageVisible WRITE setImageVisible NOTIFY imageVisibleChanged)
    Q_PROPERTY(bool simple READ isSimple WRITE setSimple NOTIFY simpleChanged)
    Q_PROPERTY(QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged)

public:
    explicit AntEmpty(QWidget* parent = nullptr);

    QString description() const;
    void setDescription(const QString& description);

    bool imageVisible() const;
    void setImageVisible(bool visible);

    bool isSimple() const;
    void setSimple(bool simple);

    QSize imageSize() const;
    void setImageSize(const QSize& size);

    QWidget* extraWidget() const;
    void setExtraWidget(QWidget* widget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void descriptionChanged(const QString& description);
    void imageVisibleChanged(bool visible);
    void simpleChanged(bool simple);
    void imageSizeChanged(const QSize& size);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QRect imageRect() const;
    QRect descriptionRect() const;
    QRect extraRect() const;
    void syncExtraGeometry();
    QSize effectiveImageSize() const;

    QString m_description = QStringLiteral("No data");
    bool m_imageVisible = true;
    bool m_simple = false;
    QSize m_imageSize;
    QPointer<QWidget> m_extraWidget;
};
