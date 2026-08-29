#pragma once

#include "core/QtAntDesignExport.h"
#include "core/AntCompat.h"
#include "core/AntTypes.h"

#include <QColor>
#include <QSize>
#include <QWidget>

class QVBoxLayout;
class QVariantAnimation;
class AntBorderBeamStyle;

// Ant Design 6.4+ BorderBeam: 沿容器边框运动的光束装饰。
// 组件负责公开 API、内容托管和动画状态，绘制由 AntBorderBeamStyle 承担。
class QT_ANT_DESIGN_EXPORT AntBorderBeam : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int count READ count WRITE setCount NOTIFY countChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(int beamLength READ beamLength WRITE setBeamLength NOTIFY beamLengthChanged)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius NOTIFY borderRadiusChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(bool activeOnHover READ isActiveOnHover WRITE setActiveOnHover NOTIFY activeOnHoverChanged)
    Q_PROPERTY(qreal phase READ phase NOTIFY phaseChanged)

public:
    explicit AntBorderBeam(QWidget* parent = nullptr);
    // content 为托管内容，parent 为 Qt 父对象。
    AntBorderBeam(QWidget* content, QWidget* parent);
    ~AntBorderBeam() override;

    // 光束颜色；默认 QColor() 表示跟随主题 colorPrimary。
    QColor color() const;
    void setColor(const QColor& color);

    // 同时运动的光束数量（1-8）。
    int count() const;
    void setCount(int count);

    // 光束绕边框一周的时长（毫秒）。
    int duration() const;
    void setDuration(int msecs);

    // 光束弧长（像素）。
    int beamLength() const;
    void setBeamLength(int pixels);

    // 光束线宽（像素）。
    int lineWidth() const;
    void setLineWidth(int pixels);

    // 边框圆角；-1 表示跟随主题 borderRadiusLG token。
    int borderRadius() const;
    void setBorderRadius(int radius);

    // 光束动画是否运行。
    bool isRunning() const;
    void setRunning(bool running);

    // 仅在 hover 时运行动画。
    bool isActiveOnHover() const;
    void setActiveOnHover(bool hoverOnly);

    // 当前动画相位（0..1），动画运行期间持续变化。
    qreal phase() const;

    // 内容托管：Beam 仅负责边框装饰，业务内容放入内部布局。
    void setContentWidget(QWidget* widget);
    QWidget* contentWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void colorChanged(const QColor& color);
    void countChanged(int count);
    void durationChanged(int duration);
    void beamLengthChanged(int length);
    void lineWidthChanged(int width);
    void borderRadiusChanged(int radius);
    void runningChanged(bool running);
    void activeOnHoverChanged(bool activeOnHover);
    void phaseChanged(qreal phase);

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void hideEvent(QHideEvent* event) override;
    void enterEvent(AntEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    friend class AntBorderBeamStyle;

    void updateAnimationState();
    int contentMargin() const;
    QColor resolvedBeamColor() const;
    int resolvedBorderRadius() const;

    QColor m_color;
    int m_count = 1;
    int m_duration = 3000;
    int m_beamLength = 48;
    int m_lineWidth = 2;
    int m_borderRadius = -1;
    bool m_running = true;
    bool m_activeOnHover = false;
    bool m_hovered = false;

    qreal m_phase = 0.0;
    QVariantAnimation* m_animation = nullptr;
    QVBoxLayout* m_contentLayout = nullptr;
    QWidget* m_content = nullptr;
};
