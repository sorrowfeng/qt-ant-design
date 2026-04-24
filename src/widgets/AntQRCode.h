#pragma once

#include <QIcon>
#include <QWidget>

#include "core/AntTypes.h"

class AntQRCode : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int qrSize READ qrSize WRITE setQrSize NOTIFY qrSizeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor NOTIFY bgColorChanged)
    Q_PROPERTY(Ant::QRCodeErrorLevel errorLevel READ errorLevel WRITE setErrorLevel NOTIFY errorLevelChanged)
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
    Q_PROPERTY(bool bordered READ isBordered WRITE setBordered NOTIFY borderedChanged)
    Q_PROPERTY(Ant::QRCodeStatus status READ status WRITE setStatus NOTIFY statusChanged)

public:
    explicit AntQRCode(QWidget* parent = nullptr);

    QString value() const;
    void setValue(const QString& value);
    int qrSize() const;
    void setQrSize(int size);
    QColor color() const;
    void setColor(const QColor& color);
    QColor bgColor() const;
    void setBgColor(const QColor& color);
    Ant::QRCodeErrorLevel errorLevel() const;
    void setErrorLevel(Ant::QRCodeErrorLevel level);
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    int iconSize() const;
    void setIconSize(int size);
    bool isBordered() const;
    void setBordered(bool bordered);
    Ant::QRCodeStatus status() const;
    void setStatus(Ant::QRCodeStatus status);

    void refresh();
    QVector<QVector<bool>> qrMatrix() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void valueChanged(const QString&);
    void qrSizeChanged(int);
    void colorChanged(const QColor&);
    void bgColorChanged(const QColor&);
    void errorLevelChanged(Ant::QRCodeErrorLevel);
    void iconSizeChanged(int);
    void borderedChanged(bool);
    void statusChanged(Ant::QRCodeStatus);
    void refreshClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void regenerateMatrix();

    QString m_value;
    int m_qrSize = 160;
    QColor m_color;
    QColor m_bgColor;
    Ant::QRCodeErrorLevel m_errorLevel = Ant::QRCodeErrorLevel::M;
    QIcon m_icon;
    int m_iconSize = 40;
    bool m_bordered = true;
    Ant::QRCodeStatus m_status = Ant::QRCodeStatus::Active;
    QVector<QVector<bool>> m_qrMatrix;
};
