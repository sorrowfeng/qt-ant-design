#pragma once

#include <QPointer>
#include <QWidget>

#include "core/AntTypes.h"

class AntButton;
class AntPopover;

class AntPopconfirm : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString okText READ okText WRITE setOkText NOTIFY okTextChanged)
    Q_PROPERTY(QString cancelText READ cancelText WRITE setCancelText NOTIFY cancelTextChanged)
    Q_PROPERTY(bool showCancel READ showCancel WRITE setShowCancel NOTIFY showCancelChanged)
    Q_PROPERTY(bool disabled READ isDisabled WRITE setDisabled NOTIFY disabledChanged)
    Q_PROPERTY(Ant::TooltipPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)

public:
    explicit AntPopconfirm(QWidget* parent = nullptr);

    QString title() const;
    void setTitle(const QString& title);

    QString description() const;
    void setDescription(const QString& description);

    QString okText() const;
    void setOkText(const QString& text);

    QString cancelText() const;
    void setCancelText(const QString& text);

    bool showCancel() const;
    void setShowCancel(bool show);

    bool isDisabled() const;
    void setDisabled(bool disabled);

    Ant::TooltipPlacement placement() const;
    void setPlacement(Ant::TooltipPlacement placement);

    QWidget* target() const;
    void setTarget(QWidget* target);

    bool isOpen() const;
    void setOpen(bool open);

Q_SIGNALS:
    void titleChanged(const QString& title);
    void descriptionChanged(const QString& description);
    void okTextChanged(const QString& text);
    void cancelTextChanged(const QString& text);
    void showCancelChanged(bool show);
    void disabledChanged(bool disabled);
    void placementChanged(Ant::TooltipPlacement placement);
    void confirmRequested();
    void cancelRequested();
    void openChanged(bool open);

private:
    void rebuildActionWidget();
    void syncPopoverContent();

    AntPopover* m_popover = nullptr;
    QPointer<QWidget> m_target;
    QWidget* m_actionContainer = nullptr;
    AntButton* m_cancelButton = nullptr;
    AntButton* m_okButton = nullptr;
    QString m_title;
    QString m_description;
    QString m_okText = QStringLiteral("OK");
    QString m_cancelText = QStringLiteral("Cancel");
    bool m_showCancel = true;
    bool m_disabled = false;
};
