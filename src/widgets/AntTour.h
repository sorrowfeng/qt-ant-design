#pragma once

#include "core/QtAntDesignExport.h"

#include <QObject>
#include <QList>
#include <QPointer>

class QWidget;
class QDialog;

struct AntTourStep
{
    QWidget* target = nullptr;
    QString title;
    QString description;
    Qt::Alignment placement = Qt::AlignBottom;
};

class QT_ANT_DESIGN_EXPORT AntTour : public QObject
{
    Q_OBJECT

public:
    explicit AntTour(QObject* parent = nullptr);
    ~AntTour() override;

    void addStep(const AntTourStep& step);
    void start(int index = 0);
    void next();
    void prev();
    void close();

Q_SIGNALS:
    void finished();
    void stepChanged(int index);

private:
    void showStep(int index);
    void syncTourPerfCounters() const;
    QPointer<QDialog> m_overlay;
    QList<AntTourStep> m_steps;
    QList<QPointer<QWidget>> m_stepTargets;
    int m_current = -1;
};
