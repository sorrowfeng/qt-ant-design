#pragma once

#include "core/QtAntDesignExport.h"

#include <QColor>
#include <QSplitter>

class AntSplitterHandle;

class QT_ANT_DESIGN_EXPORT AntSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit AntSplitter(QWidget* parent = nullptr);
    explicit AntSplitter(Qt::Orientation orientation, QWidget* parent = nullptr);

    QColor handleColor(bool hovered) const;
    int handleThemeRevision() const;

protected:
    QSplitterHandle* createHandle() override;

private:
    void initialize();
    void refreshHandlePalette();
    void updateHandles();
    void syncPerfCounters() const;

    QColor m_handleColor;
    QColor m_hoverHandleColor;
    int m_handleThemeRevision = 0;
    int m_handlePaletteRefreshCount = 0;
};

class QT_ANT_DESIGN_EXPORT AntSplitterHandle : public QSplitterHandle
{
    Q_OBJECT

public:
    AntSplitterHandle(Qt::Orientation orientation, AntSplitter* parent);

protected:
    void paintEvent(QPaintEvent*) override;
    void enterEvent(AntEnterEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    QColor resolvedColor();
    void invalidateColorCache();
    void syncPerfCounters() const;

    bool m_hovered = false;
    bool m_cachedHovered = false;
    int m_cachedThemeRevision = -1;
    QColor m_cachedColor;
    int m_paintCount = 0;
    int m_colorResolveCount = 0;
    int m_hoverUpdateCount = 0;
};
