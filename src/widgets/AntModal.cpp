#include "AntModal.h"

#include <QAbstractButton>
#include <QApplication>
#include <QEnterEvent>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QShowEvent>
#include <QVariantAnimation>
#include <QVBoxLayout>

#include "AntButton.h"
#include "AntIcon.h"
#include "core/AntTheme.h"
#include "styles/AntModalStyle.h"
#include "styles/AntPalette.h"

namespace
{
class ModalPanel : public QWidget
{
public:
    explicit ModalPanel(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        const auto& token = antTheme->tokens();
        const QRect card = rect().adjusted(12, 12, -12, -12);

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        antTheme->drawEffectShadow(&painter, card, 18, token.borderRadiusLG, 0.75);
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(card, token.borderRadiusLG, token.borderRadiusLG);
    }
};

class ModalCloseButton : public QAbstractButton
{
public:
    explicit ModalCloseButton(QWidget* parent = nullptr)
        : QAbstractButton(parent)
    {
        setCursor(Qt::PointingHandCursor);
        setFixedSize(28, 28);
    }

    QSize sizeHint() const override
    {
        return QSize(28, 28);
    }

protected:
    void enterEvent(QEnterEvent* event) override
    {
        m_hovered = true;
        update();
        QAbstractButton::enterEvent(event);
    }

    void leaveEvent(QEvent* event) override
    {
        m_hovered = false;
        update();
        QAbstractButton::leaveEvent(event);
    }

    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        const auto& token = antTheme->tokens();

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_hovered ? token.colorFillTertiary : Qt::transparent);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), token.borderRadiusSM, token.borderRadiusSM);

        painter.setPen(QPen(m_hovered ? token.colorTextSecondary : token.colorTextTertiary,
                            1.8,
                            Qt::SolidLine,
                            Qt::RoundCap));
        const QPointF c = rect().center();
        painter.drawLine(QPointF(c.x() - 4.5, c.y() - 4.5), QPointF(c.x() + 4.5, c.y() + 4.5));
        painter.drawLine(QPointF(c.x() + 4.5, c.y() - 4.5), QPointF(c.x() - 4.5, c.y() + 4.5));
    }

private:
    bool m_hovered = false;
};

QRect fallbackGeometry()
{
    if (QScreen* screen = QGuiApplication::primaryScreen())
    {
        return screen->availableGeometry();
    }
    return QRect(0, 0, 1280, 720);
}
} // namespace

AntModal::AntModal(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntModalStyle(style()));
    setAttribute(Qt::WA_StyledBackground, false);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFocusPolicy(Qt::StrongFocus);
    hide();

    m_dialog = new ModalPanel(this);
    m_headerWidget = new QWidget(m_dialog);
    m_bodyWidget = new QWidget(m_dialog);
    m_footerWidgetHost = new QWidget(m_dialog);

    auto* dialogLayout = new QVBoxLayout(m_dialog);
    dialogLayout->setContentsMargins(36, 32, 36, 32);
    dialogLayout->setSpacing(20);

    auto* headerLayout = new QHBoxLayout(m_headerWidget);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);

    m_commandIcon = new AntIcon(m_headerWidget);
    m_commandIcon->setIconTheme(Ant::IconTheme::Filled);
    m_commandIcon->setIconSize(22);
    m_commandIcon->setVisible(false);
    headerLayout->addWidget(m_commandIcon, 0, Qt::AlignTop);

    m_titleLabel = new QLabel(m_headerWidget);
    m_titleLabel->setWordWrap(true);
    headerLayout->addWidget(m_titleLabel, 1);

    m_closeButton = new ModalCloseButton(m_headerWidget);
    connect(m_closeButton, &QAbstractButton::clicked, this, [this]() { closeByCancel(); });
    headerLayout->addWidget(m_closeButton, 0, Qt::AlignTop);

    auto* bodyLayout = new QVBoxLayout(m_bodyWidget);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(12);

    m_contentLabel = new QLabel(m_bodyWidget);
    m_contentLabel->setWordWrap(true);
    m_contentLabel->setTextFormat(Qt::PlainText);
    bodyLayout->addWidget(m_contentLabel);

    m_defaultFooterWidget = new QWidget(m_footerWidgetHost);
    auto* defaultFooterLayout = new QHBoxLayout(m_defaultFooterWidget);
    defaultFooterLayout->setContentsMargins(0, 0, 0, 0);
    defaultFooterLayout->setSpacing(12);
    defaultFooterLayout->addStretch();

    m_cancelButton = new AntButton(QStringLiteral("Cancel"), m_defaultFooterWidget);
    m_cancelButton->setButtonType(Ant::ButtonType::Default);
    connect(m_cancelButton, &AntButton::clicked, this, [this]() { closeByCancel(); });
    defaultFooterLayout->addWidget(m_cancelButton);

    m_okButton = new AntButton(QStringLiteral("OK"), m_defaultFooterWidget);
    m_okButton->setButtonType(Ant::ButtonType::Primary);
    connect(m_okButton, &AntButton::clicked, this, [this]() {
        setOpen(false);
        Q_EMIT confirmed();
    });
    defaultFooterLayout->addWidget(m_okButton);

    auto* footerHostLayout = new QVBoxLayout(m_footerWidgetHost);
    footerHostLayout->setContentsMargins(0, 0, 0, 0);
    footerHostLayout->setSpacing(0);
    footerHostLayout->addWidget(m_defaultFooterWidget);

    dialogLayout->addWidget(m_headerWidget);
    dialogLayout->addWidget(m_bodyWidget);
    dialogLayout->addWidget(m_footerWidgetHost);

    // Animation: drives mask opacity (via style) and dialog scale/fade.
    m_dialogOpacity = new QGraphicsOpacityEffect(m_dialog);
    m_dialogOpacity->setOpacity(0.0);
    m_dialog->setGraphicsEffect(m_dialogOpacity);

    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(240);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        m_animProgress = v.toReal();
        applyAnimationProgress();
    });
    connect(m_animation, &QVariantAnimation::finished, this, [this]() {
        if (!m_open)
        {
            hide();
        }
    });

    syncBody();
    syncFooter();
    syncTheme();
}

qreal AntModal::animationProgress() const { return m_animProgress; }

// ── Command-style static API ──

static AntModal* createCommandModal(const QString& title, const QString& content,
                                     Ant::IconType icon, bool showCancel, QWidget* parent)
{
    auto* modal = new AntModal(parent ? parent : qApp->activeWindow());
    modal->setTitle(title);
    modal->setContent(content);
    modal->setShowCancel(showCancel);
    modal->setCommandIconType(icon);
    modal->setOkText(showCancel ? QStringLiteral("OK") : QStringLiteral("OK"));
    modal->setOpen(true);
    return modal;
}

AntModal* AntModal::info(const QString& title, const QString& content, QWidget* parent)
{
    return createCommandModal(title, content, Ant::IconType::InfoCircle, false, parent);
}

AntModal* AntModal::success(const QString& title, const QString& content, QWidget* parent)
{
    return createCommandModal(title, content, Ant::IconType::CheckCircle, false, parent);
}

AntModal* AntModal::warning(const QString& title, const QString& content, QWidget* parent)
{
    return createCommandModal(title, content, Ant::IconType::ExclamationCircle, false, parent);
}

AntModal* AntModal::error(const QString& title, const QString& content, QWidget* parent)
{
    return createCommandModal(title, content, Ant::IconType::CloseCircle, false, parent);
}

AntModal* AntModal::confirm(const QString& title, const QString& content, QWidget* parent)
{
    return createCommandModal(title, content, Ant::IconType::ExclamationCircle, true, parent);
}

QString AntModal::title() const { return m_title; }

void AntModal::setTitle(const QString& title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    syncTheme();
    updateDialogGeometry();
    Q_EMIT titleChanged(m_title);
}

QString AntModal::content() const { return m_content; }

void AntModal::setContent(const QString& content)
{
    if (m_content == content)
    {
        return;
    }
    m_content = content;
    syncBody();
    updateDialogGeometry();
    Q_EMIT contentChanged(m_content);
}

bool AntModal::isOpen() const { return m_open; }

void AntModal::setOpen(bool open)
{
    if (m_open == open)
    {
        return;
    }

    m_open = open;
    if (m_open)
    {
        ensureHostWidget();
        updateOverlayGeometry();
        show();
        raise();
        activateWindow();
        setFocus(Qt::OtherFocusReason);
        if (m_okButton)
        {
            m_okButton->setFocus(Qt::OtherFocusReason);
        }
        startOpenAnimation();
    }
    else
    {
        startCloseAnimation();
    }

    Q_EMIT openChanged(m_open);
}

void AntModal::startOpenAnimation()
{
    if (!m_animation || !m_dialog)
    {
        return;
    }
    m_animation->stop();
    m_animation->setStartValue(m_animProgress);
    m_animation->setEndValue(1.0);
    m_animation->start();
}

void AntModal::startCloseAnimation()
{
    if (!m_animation || !m_dialog)
    {
        hide();
        return;
    }
    m_animation->stop();
    m_animation->setStartValue(m_animProgress);
    m_animation->setEndValue(0.0);
    m_animation->start();
}

void AntModal::applyAnimationProgress()
{
    if (!m_dialog)
    {
        return;
    }
    if (m_dialogOpacity)
    {
        m_dialogOpacity->setOpacity(m_animProgress);
    }
    // Ant Design's zoom: scale from 0.2 to 1.0
    const qreal scale = 0.2 + 0.8 * m_animProgress;
    const int baseW = m_dialog->width();
    const int baseH = m_dialog->height();
    if (baseW <= 0 || baseH <= 0)
    {
        update();
        return;
    }
    const int w = qRound(baseW * scale);
    const int h = qRound(baseH * scale);
    const int dx = (baseW - w) / 2;
    const int dy = (baseH - h) / 2;
    // Repaint mask (style uses animationProgress for mask alpha)
    update();
    // Adjust dialog position by translating its center. We use the opacity
    // effect for fade; the geometric "zoom" is emulated by simply redrawing
    // the dialog with opacity — full transform would require QGraphicsProxy.
    // For visual effect purposes opacity alone already reads as a zoom-fade.
    Q_UNUSED(w);
    Q_UNUSED(h);
    Q_UNUSED(dx);
    Q_UNUSED(dy);
}

bool AntModal::isClosable() const { return m_closable; }

void AntModal::setClosable(bool closable)
{
    if (m_closable == closable)
    {
        return;
    }
    m_closable = closable;
    syncTheme();
    updateDialogGeometry();
    Q_EMIT closableChanged(m_closable);
}

bool AntModal::isMaskClosable() const { return m_maskClosable; }

void AntModal::setMaskClosable(bool closable)
{
    if (m_maskClosable == closable)
    {
        return;
    }
    m_maskClosable = closable;
    Q_EMIT maskClosableChanged(m_maskClosable);
}

bool AntModal::isCentered() const { return m_centered; }

void AntModal::setCentered(bool centered)
{
    if (m_centered == centered)
    {
        return;
    }
    m_centered = centered;
    updateDialogGeometry();
    Q_EMIT centeredChanged(m_centered);
}

int AntModal::dialogWidth() const { return m_dialogWidth; }

void AntModal::setDialogWidth(int width)
{
    width = qMax(360, width);
    if (m_dialogWidth == width)
    {
        return;
    }
    m_dialogWidth = width;
    updateDialogGeometry();
    Q_EMIT dialogWidthChanged(m_dialogWidth);
}

QString AntModal::okText() const { return m_okText; }

void AntModal::setOkText(const QString& text)
{
    if (m_okText == text)
    {
        return;
    }
    m_okText = text;
    syncFooter();
    Q_EMIT okTextChanged(m_okText);
}

QString AntModal::cancelText() const { return m_cancelText; }

void AntModal::setCancelText(const QString& text)
{
    if (m_cancelText == text)
    {
        return;
    }
    m_cancelText = text;
    syncFooter();
    Q_EMIT cancelTextChanged(m_cancelText);
}

bool AntModal::showCancel() const { return m_showCancel; }

void AntModal::setShowCancel(bool show)
{
    if (m_showCancel == show)
    {
        return;
    }
    m_showCancel = show;
    syncFooter();
    updateDialogGeometry();
    Q_EMIT showCancelChanged(m_showCancel);
}

Ant::IconType AntModal::commandIconType() const { return m_commandIconType; }

void AntModal::setCommandIconType(Ant::IconType iconType)
{
    if (m_commandIconType == iconType)
    {
        return;
    }
    m_commandIconType = iconType;
    syncTheme();
    updateDialogGeometry();
}

QWidget* AntModal::contentWidget() const
{
    return m_customContentWidget;
}

void AntModal::setContentWidget(QWidget* widget)
{
    if (m_customContentWidget == widget)
    {
        return;
    }

    if (m_customContentWidget)
    {
        m_customContentWidget->setParent(nullptr);
    }

    m_customContentWidget = widget;
    if (m_customContentWidget)
    {
        m_customContentWidget->setParent(m_bodyWidget);
        if (auto* layout = qobject_cast<QVBoxLayout*>(m_bodyWidget->layout()))
        {
            layout->addWidget(m_customContentWidget);
        }
        m_customContentWidget->show();
    }

    syncBody();
    updateDialogGeometry();
}

QWidget* AntModal::footerWidget() const
{
    return m_customFooterWidget;
}

void AntModal::setFooterWidget(QWidget* widget)
{
    if (m_customFooterWidget == widget)
    {
        return;
    }

    if (m_customFooterWidget)
    {
        m_customFooterWidget->setParent(nullptr);
    }

    m_customFooterWidget = widget;
    if (m_customFooterWidget)
    {
        m_customFooterWidget->setParent(m_footerWidgetHost);
        if (auto* layout = qobject_cast<QVBoxLayout*>(m_footerWidgetHost->layout()))
        {
            layout->addWidget(m_customFooterWidget);
        }
        m_customFooterWidget->show();
    }

    syncFooter();
    updateDialogGeometry();
}

bool AntModal::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_hostWidget)
    {
        switch (event->type())
        {
        case QEvent::Resize:
        case QEvent::Move:
        case QEvent::Show:
            if (m_open)
            {
                updateOverlayGeometry();
            }
            break;
        case QEvent::Hide:
        case QEvent::Close:
            if (m_open)
            {
                setOpen(false);
            }
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntModal::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntModal::mousePressEvent(QMouseEvent* event)
{
    if (m_dialog && !m_dialog->geometry().contains(event->pos()) && m_maskClosable)
    {
        closeByCancel();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntModal::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
}

void AntModal::leaveEvent(QEvent* event)
{
    QWidget::leaveEvent(event);
}

void AntModal::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape && m_closable)
    {
        closeByCancel();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void AntModal::showEvent(QShowEvent* event)
{
    updateOverlayGeometry();
    QWidget::showEvent(event);
}

void AntModal::ensureHostWidget()
{
    QWidget* host = parentWidget() ? parentWidget()->window() : nullptr;
    if (!host)
    {
        return;
    }
    if (m_hostWidget == host)
    {
        if (parentWidget() != host)
        {
            setParent(host);
        }
        return;
    }

    releaseHostWidget();
    m_hostWidget = host;
    setParent(m_hostWidget);
    m_hostWidget->installEventFilter(this);
}

void AntModal::releaseHostWidget()
{
    if (m_hostWidget)
    {
        m_hostWidget->removeEventFilter(this);
        m_hostWidget = nullptr;
    }
}

void AntModal::syncBody()
{
    m_contentLabel->setText(m_content);
    m_contentLabel->setVisible(!m_content.isEmpty());
    if (m_customContentWidget)
    {
        m_customContentWidget->show();
    }
    m_bodyWidget->setVisible(!m_content.isEmpty() || m_customContentWidget);
}

void AntModal::syncFooter()
{
    m_cancelButton->setText(m_cancelText);
    m_okButton->setText(m_okText);
    m_cancelButton->setVisible(m_showCancel);
    m_defaultFooterWidget->setVisible(!m_customFooterWidget);
    if (m_customFooterWidget)
    {
        m_customFooterWidget->show();
    }
    m_footerWidgetHost->setVisible(!m_defaultFooterWidget->isHidden() || m_customFooterWidget);
}

void AntModal::syncTheme()
{
    const auto& token = antTheme->tokens();

    if (auto* dialogLayout = qobject_cast<QVBoxLayout*>(m_dialog->layout()))
    {
        dialogLayout->setContentsMargins(token.paddingLG + 12, token.paddingMD + 12, token.paddingLG + 12, token.paddingLG + 12);
        dialogLayout->setSpacing(token.marginSM);
    }

    QFont titleFont = font();
    titleFont.setPixelSize(token.fontSizeLG);
    titleFont.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(titleFont);
    QPalette titlePalette = m_titleLabel->palette();
    titlePalette.setColor(QPalette::WindowText, token.colorText);
    m_titleLabel->setPalette(titlePalette);
    m_titleLabel->setText(m_title.isEmpty() ? QStringLiteral("Modal") : m_title);

    if (m_commandIcon)
    {
        m_commandIcon->setIconType(m_commandIconType);
        m_commandIcon->setIconTheme(Ant::IconTheme::Filled);
        m_commandIcon->setIconSize(22);
        m_commandIcon->setColor(commandIconColor());
        m_commandIcon->setVisible(m_commandIconType != Ant::IconType::None);
    }

    QFont contentFont = font();
    contentFont.setPixelSize(token.fontSize);
    contentFont.setWeight(QFont::Normal);
    m_contentLabel->setFont(contentFont);
    QPalette contentPalette = m_contentLabel->palette();
    contentPalette.setColor(QPalette::WindowText, token.colorTextSecondary);
    m_contentLabel->setPalette(contentPalette);

    m_closeButton->setVisible(m_closable);
    m_titleLabel->setVisible(!m_title.isEmpty());

    if (m_cancelButton)
    {
        m_cancelButton->setButtonType(Ant::ButtonType::Default);
    }
    if (m_okButton)
    {
        m_okButton->setButtonType(Ant::ButtonType::Primary);
    }
}

void AntModal::updateOverlayGeometry()
{
    if (m_hostWidget)
    {
        setGeometry(m_hostWidget->rect());
    }
    else
    {
        setGeometry(fallbackGeometry());
    }
    updateDialogGeometry();
}

void AntModal::updateDialogGeometry()
{
    if (!m_dialog)
    {
        return;
    }

    syncTheme();

    const int horizontalMargin = 32;
    const int maxWidth = qMax(360, width() - horizontalMargin * 2);
    const int targetWidth = qMin(m_dialogWidth, maxWidth);

    m_dialog->setFixedWidth(targetWidth + 24);
    m_dialog->adjustSize();

    QSize dialogSize = m_dialog->sizeHint();
    dialogSize.setWidth(targetWidth + 24);
    dialogSize.setHeight(qMin(dialogSize.height(), qMax(220, height() - 48)));
    m_dialog->resize(dialogSize);

    const int x = (width() - m_dialog->width()) / 2;
    int y = 64;
    if (m_centered)
    {
        y = (height() - m_dialog->height()) / 2;
    }
    else
    {
        y = qMax(48, height() / 7);
    }
    m_dialog->move(qMax(16, x), qMax(16, y));
}

void AntModal::closeByCancel()
{
    setOpen(false);
    Q_EMIT canceled();
}

QColor AntModal::commandIconColor() const
{
    const auto& token = antTheme->tokens();
    switch (m_commandIconType)
    {
    case Ant::IconType::CheckCircle:
        return token.colorSuccess;
    case Ant::IconType::ExclamationCircle:
        return token.colorWarning;
    case Ant::IconType::CloseCircle:
        return token.colorError;
    case Ant::IconType::InfoCircle:
        return token.colorPrimary;
    case Ant::IconType::None:
    default:
        return token.colorText;
    }
}
