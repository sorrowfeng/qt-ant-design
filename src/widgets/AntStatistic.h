#pragma once

#include <QPointer>
#include <QWidget>

class QPaintEvent;

class AntStatistic : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int precision READ precision WRITE setPrecision NOTIFY precisionChanged)
    Q_PROPERTY(QString groupSeparator READ groupSeparator WRITE setGroupSeparator NOTIFY groupSeparatorChanged)
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY prefixChanged)
    Q_PROPERTY(QString suffix READ suffix WRITE setSuffix NOTIFY suffixChanged)

public:
    explicit AntStatistic(QWidget* parent = nullptr);
    explicit AntStatistic(const QString& title, QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    double value() const;
    void setValue(double value);

    int precision() const;
    void setPrecision(int precision);

    QString groupSeparator() const;
    void setGroupSeparator(const QString& separator);

    QString prefix() const;
    void setPrefix(const QString& prefix);

    QString suffix() const;
    void setSuffix(const QString& suffix);

    void setValueWidget(QWidget* widget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void valueChanged(double value);
    void precisionChanged(int precision);
    void groupSeparatorChanged(const QString& separator);
    void prefixChanged(const QString& prefix);
    void suffixChanged(const QString& suffix);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Metrics
    {
        int padding = 16;
        int titleFontSize = 14;
        int valueFontSize = 24;
        int prefixFontSize = 16;
        int suffixFontSize = 14;
        int spacing = 4;
    };

    Metrics metrics() const;
    QRect titleRect() const;
    QRect valueRect() const;
    QString formattedValue() const;
    void syncValueWidgetGeometry();

    QString m_title;
    double m_value = 0.0;
    int m_precision = 0;
    QString m_groupSeparator = QStringLiteral(",");
    QString m_prefix;
    QString m_suffix;
    QPointer<QWidget> m_valueWidget;
};
