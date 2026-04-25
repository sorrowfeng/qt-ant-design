#pragma once

#include <QObject>
#include <QList>

class QWidget;
class QDialog;

struct AntTourStep
{
    QWidget* target = nullptr;
    QString title;
    QString description;
    Qt::Alignment placement = Qt::AlignBottom;
};

class AntTour : public QObject
{
    Q_OBJECT

public:
    explicit AntTour(QObject* parent = nullptr);

    void addStep(const AntTourStep& step);
    void start();
    void next();
    void prev();
    void close();

Q_SIGNALS:
    void finished();
    void stepChanged(int index);

private:
    void showStep(int index);
    QDialog* m_overlay = nullptr;
    QList<AntTourStep> m_steps;
    int m_current = -1;
};
