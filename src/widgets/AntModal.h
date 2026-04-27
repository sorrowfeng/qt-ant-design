#pragma once

#include <QPointer>
#include <QWidget>

#include "core/AntTypes.h"

class AntButton;
class AntIcon;
class QAbstractButton;
class QColor;
class QEvent;
class QKeyEvent;
class QLabel;
class QMouseEvent;
class QPaintEvent;
class QShowEvent;
class QVariantAnimation;
class QGraphicsOpacityEffect;

class AntModal : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(bool open READ isOpen WRITE setOpen NOTIFY openChanged)
    Q_PROPERTY(bool closable READ isClosable WRITE setClosable NOTIFY closableChanged)
    Q_PROPERTY(bool maskClosable READ isMaskClosable WRITE setMaskClosable NOTIFY maskClosableChanged)
    Q_PROPERTY(bool centered READ isCentered WRITE setCentered NOTIFY centeredChanged)
    Q_PROPERTY(int dialogWidth READ dialogWidth WRITE setDialogWidth NOTIFY dialogWidthChanged)
    Q_PROPERTY(QString okText READ okText WRITE setOkText NOTIFY okTextChanged)
    Q_PROPERTY(QString cancelText READ cancelText WRITE setCancelText NOTIFY cancelTextChanged)
    Q_PROPERTY(bool showCancel READ showCancel WRITE setShowCancel NOTIFY showCancelChanged)

public:
    explicit AntModal(QWidget* parent = nullptr);

    // Command-style static API
    static AntModal* info(const QString& title, const QString& content, QWidget* parent = nullptr);
    static AntModal* success(const QString& title, const QString& content, QWidget* parent = nullptr);
    static AntModal* warning(const QString& title, const QString& content, QWidget* parent = nullptr);
    static AntModal* error(const QString& title, const QString& content, QWidget* parent = nullptr);
    static AntModal* confirm(const QString& title, const QString& content, QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    QString content() const;
    void setContent(const QString& content);

    bool isOpen() const;
    void setOpen(bool open);

    bool isClosable() const;
    void setClosable(bool closable);

    bool isMaskClosable() const;
    void setMaskClosable(bool closable);

    bool isCentered() const;
    void setCentered(bool centered);

    int dialogWidth() const;
    void setDialogWidth(int width);

    QString okText() const;
    void setOkText(const QString& text);

    QString cancelText() const;
    void setCancelText(const QString& text);

    bool showCancel() const;
    void setShowCancel(bool show);

    Ant::IconType commandIconType() const;
    void setCommandIconType(Ant::IconType iconType);

    QWidget* contentWidget() const;
    void setContentWidget(QWidget* widget);

    QWidget* footerWidget() const;
    void setFooterWidget(QWidget* widget);

    // Animation progress: 0 = fully closed (transparent mask, dialog scaled
    // down), 1 = fully open. Read by AntModalStyle when painting the mask.
    qreal animationProgress() const;

Q_SIGNALS:
    void titleChanged(const QString& title);
    void contentChanged(const QString& content);
    void openChanged(bool open);
    void closableChanged(bool closable);
    void maskClosableChanged(bool closable);
    void centeredChanged(bool centered);
    void dialogWidthChanged(int width);
    void okTextChanged(const QString& text);
    void cancelTextChanged(const QString& text);
    void showCancelChanged(bool show);
    void confirmed();
    void canceled();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void ensureHostWidget();
    void releaseHostWidget();
    void syncBody();
    void syncFooter();
    void syncTheme();
    void updateOverlayGeometry();
    void updateDialogGeometry();
    void applyAnimationProgress();
    void startOpenAnimation();
    void startCloseAnimation();
    void closeByCancel();
    QColor commandIconColor() const;

    QString m_title;
    QString m_content;
    bool m_open = false;
    bool m_closable = true;
    bool m_maskClosable = true;
    bool m_centered = true;
    int m_dialogWidth = 520;
    QString m_okText = QStringLiteral("OK");
    QString m_cancelText = QStringLiteral("Cancel");
    bool m_showCancel = true;
    Ant::IconType m_commandIconType = Ant::IconType::None;
    QPointer<QWidget> m_hostWidget;
    QWidget* m_dialog = nullptr;
    QWidget* m_headerWidget = nullptr;
    QWidget* m_bodyWidget = nullptr;
    QWidget* m_footerWidgetHost = nullptr;
    QWidget* m_customContentWidget = nullptr;
    QWidget* m_customFooterWidget = nullptr;
    QWidget* m_defaultFooterWidget = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_contentLabel = nullptr;
    AntIcon* m_commandIcon = nullptr;
    QAbstractButton* m_closeButton = nullptr;
    AntButton* m_cancelButton = nullptr;
    AntButton* m_okButton = nullptr;

    QVariantAnimation* m_animation = nullptr;
    QGraphicsOpacityEffect* m_dialogOpacity = nullptr;
    qreal m_animProgress = 0.0;
};
