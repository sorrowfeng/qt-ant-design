#pragma once

#include "core/QtAntDesignExport.h"

#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>

#include "core/AntTheme.h"

class AntSelect;
class QEnterEvent;
class QEvent;
class QHideEvent;
class QMouseEvent;
class QPaintEvent;

class QT_ANT_DESIGN_EXPORT AntSelectPopup : public QFrame
{
public:
    static constexpr int ShadowMargin = 32;

    explicit AntSelectPopup(AntSelect* owner)
        : QFrame(owner, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint),
          m_owner(owner)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
        setObjectName(QStringLiteral("AntSelectPopup"));
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        const QRectF panel = QRectF(rect()).adjusted(ShadowMargin,
                                                     ShadowMargin,
                                                     -ShadowMargin,
                                                     -ShadowMargin);
        antTheme->drawEffectShadow(&painter, panel.toAlignedRect(), 10, token.borderRadiusLG, 0.55);
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(panel, token.borderRadiusLG, token.borderRadiusLG);
    }

    void hideEvent(QHideEvent* event) override
    {
        if (m_owner && m_owner->isOpen())
        {
            m_owner->setOpen(false);
        }
        QFrame::hideEvent(event);
    }

private:
    AntSelect* m_owner = nullptr;
};

class QT_ANT_DESIGN_EXPORT AntSelectOptionWidget : public QWidget
{
public:
    AntSelectOptionWidget(AntSelect* select, int index, QWidget* parent)
        : QWidget(parent),
          m_select(select),
          m_index(index)
    {
        setAttribute(Qt::WA_Hover, true);
        setMouseTracking(true);
        setFixedHeight(antTheme->tokens().controlHeight);
        setCursor(Qt::PointingHandCursor);
    }

    void setIndex(int index)
    {
        m_index = index;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    AntSelect* m_select = nullptr;
    int m_index = -1;
    bool m_hovered = false;
};
