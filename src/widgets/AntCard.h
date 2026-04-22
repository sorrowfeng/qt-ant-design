#pragma once

#include <QFrame>
#include <QTimer>

#include "core/AntTypes.h"

class QHBoxLayout;
class QLabel;
class QVBoxLayout;

class AntCard : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString extra READ extra WRITE setExtra NOTIFY extraChanged)
    Q_PROPERTY(bool bordered READ isBordered WRITE setBordered NOTIFY borderedChanged)
    Q_PROPERTY(bool hoverable READ isHoverable WRITE setHoverable NOTIFY hoverableChanged)
    Q_PROPERTY(bool loading READ isLoading WRITE setLoading NOTIFY loadingChanged)
    Q_PROPERTY(Ant::CardSize cardSize READ cardSize WRITE setCardSize NOTIFY cardSizeChanged)

public:
    explicit AntCard(QWidget* parent = nullptr);
    explicit AntCard(const QString& title, QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);
    QString extra() const;
    void setExtra(const QString& extra);
    bool isBordered() const;
    void setBordered(bool bordered);
    bool isHoverable() const;
    void setHoverable(bool hoverable);
    bool isLoading() const;
    void setLoading(bool loading);
    Ant::CardSize cardSize() const;
    void setCardSize(Ant::CardSize size);

    QWidget* bodyWidget() const;
    QVBoxLayout* bodyLayout() const;
    void setBodyWidget(QWidget* widget);
    void setCoverWidget(QWidget* widget);
    void addActionWidget(QWidget* widget);
    void clearActions();

Q_SIGNALS:
    void titleChanged(const QString& title);
    void extraChanged(const QString& extra);
    void borderedChanged(bool bordered);
    void hoverableChanged(bool hoverable);
    void loadingChanged(bool loading);
    void cardSizeChanged(Ant::CardSize size);

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void rebuildChrome();
    void updateTheme();
    void drawSpinner(QPainter& painter, const QRectF& rect) const;

    QVBoxLayout* m_rootLayout = nullptr;
    QWidget* m_header = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_extraLabel = nullptr;
    QWidget* m_cover = nullptr;
    QWidget* m_body = nullptr;
    QVBoxLayout* m_bodyLayout = nullptr;
    QWidget* m_actions = nullptr;
    QHBoxLayout* m_actionsLayout = nullptr;

    QString m_title;
    QString m_extra;
    bool m_bordered = true;
    bool m_hoverable = false;
    bool m_loading = false;
    bool m_hovered = false;
    int m_spinnerAngle = 0;
    Ant::CardSize m_cardSize = Ant::CardSize::Default;
    QTimer m_spinnerTimer;
};
