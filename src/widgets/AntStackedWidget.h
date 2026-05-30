#pragma once

#include "core/QtAntDesignExport.h"

#include <QStackedWidget>

#include "core/AntTypes.h"

class QT_ANT_DESIGN_EXPORT AntStackedWidget : public QStackedWidget
{
    Q_OBJECT
    Q_PROPERTY(Ant::Variant variant READ variant WRITE setVariant NOTIFY variantChanged)

public:
    explicit AntStackedWidget(QWidget* parent = nullptr);

    Ant::Variant variant() const;
    void setVariant(Ant::Variant variant);

Q_SIGNALS:
    void variantChanged(Ant::Variant variant);

protected:
    bool event(QEvent* event) override;

private:
    void updateFrameMargins();
    void syncStackedWidgetPerfCounters() const;

    Ant::Variant m_variant = Ant::Variant::Outlined;
    int m_marginRefreshCount = 0;
};
