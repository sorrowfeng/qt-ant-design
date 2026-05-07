#include "AntInput.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>

#include "../styles/AntInputStyle.h"
#include "core/AntTheme.h"

namespace
{
class AntInputIconButton : public QToolButton
{
public:
    enum class Kind
    {
        Clear,
        Password,
        Search
    };

    explicit AntInputIconButton(Kind kind, QWidget* parent = nullptr)
        : QToolButton(parent),
          m_kind(kind)
    {
        setAutoRaise(true);
        setFocusPolicy(Qt::NoFocus);
        setFixedSize(22, 22);
    }

    void setActive(bool active)
    {
        if (m_active == active)
        {
            return;
        }
        m_active = active;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);

        QColor iconColor = palette().color(QPalette::ButtonText);
        if (!isEnabled())
        {
            iconColor.setAlphaF(0.35);
        }
        else if (isDown())
        {
            iconColor = antTheme->tokens().colorTextSecondary;
        }

        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        painter.setPen(QPen(iconColor, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        QRectF iconRect(0, 0, 16, 16);
        iconRect.moveCenter(rect().center());

        if (m_kind == Kind::Clear)
        {
            const QPointF center = iconRect.center();
            painter.setPen(Qt::NoPen);
            painter.setBrush(iconColor);
            painter.drawEllipse(center, 5.0, 5.0);

            painter.setPen(QPen(antTheme->tokens().colorBgContainer, 1.3, Qt::SolidLine, Qt::RoundCap));
            painter.drawLine(center + QPointF(-2.2, -2.2), center + QPointF(2.2, 2.2));
            painter.drawLine(center + QPointF(2.2, -2.2), center + QPointF(-2.2, 2.2));
            return;
        }

        if (m_kind == Kind::Search)
        {
            painter.setPen(QPen(antTheme->tokens().colorBorder, 1));
            painter.drawLine(QPointF(0.5, 0), QPointF(0.5, height()));

            const QPointF center = iconRect.center() - QPointF(1.2, 1.2);
            painter.setPen(QPen(iconColor, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawEllipse(center, 4.8, 4.8);
            painter.drawLine(center + QPointF(3.6, 3.6), center + QPointF(7.0, 7.0));
            return;
        }

        QPainterPath eye;
        const QPointF center = iconRect.center();
        eye.moveTo(iconRect.left() + 1.2, center.y());
        eye.cubicTo(iconRect.left() + 4.0, iconRect.top() + 3.6,
                    iconRect.right() - 4.0, iconRect.top() + 3.6,
                    iconRect.right() - 1.2, center.y());
        eye.cubicTo(iconRect.right() - 4.0, iconRect.bottom() - 3.6,
                    iconRect.left() + 4.0, iconRect.bottom() - 3.6,
                    iconRect.left() + 1.2, center.y());
        painter.drawPath(eye);
        painter.drawEllipse(center, 2.0, 2.0);

        if (!m_active)
        {
            painter.drawLine(iconRect.topRight() + QPointF(-1.5, 1.5),
                             iconRect.bottomLeft() + QPointF(1.5, -1.5));
        }
    }

private:
    Kind m_kind;
    bool m_active = false;
};
}

AntInput::AntInput(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    auto* inputStyle = new AntInputStyle(style());
    inputStyle->setParent(this);
    setStyle(inputStyle);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setFrame(false);
    m_lineEdit->setClearButtonEnabled(false);
    m_lineEdit->installEventFilter(this);

    m_clearButton = new AntInputIconButton(AntInputIconButton::Kind::Clear, this);
    connect(m_clearButton, &QToolButton::clicked, m_lineEdit, &QLineEdit::clear);

    m_passwordButton = new AntInputIconButton(AntInputIconButton::Kind::Password, this);
    m_passwordButton->hide();
    connect(m_passwordButton, &QToolButton::clicked, this, [this]() {
        m_passwordVisible = !m_passwordVisible;
        m_lineEdit->setEchoMode(m_passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
        static_cast<AntInputIconButton*>(m_passwordButton)->setActive(m_passwordVisible);
    });

    m_searchButton = new AntInputIconButton(AntInputIconButton::Kind::Search, this);
    m_searchButton->hide();
    connect(m_searchButton, &QToolButton::clicked, this, [this]() {
        Q_EMIT searchTriggered(m_lineEdit->text());
    });
    connect(m_lineEdit, &QLineEdit::returnPressed, this, [this]() {
        if (m_searchMode) Q_EMIT searchTriggered(m_lineEdit->text());
    });
    connect(m_lineEdit, &QLineEdit::textEdited, this, &AntInput::textEdited);
    connect(m_lineEdit, &QLineEdit::returnPressed, this, &AntInput::returnPressed);
    connect(m_lineEdit, &QLineEdit::editingFinished, this, &AntInput::editingFinished);
    connect(m_lineEdit, &QLineEdit::selectionChanged, this, &AntInput::selectionChanged);
    connect(m_lineEdit, &QLineEdit::inputRejected, this, &AntInput::inputRejected);

    connect(m_lineEdit, &QLineEdit::textChanged, this, [this](const QString& value) {
        updateButtonVisibility();
        Q_EMIT textChanged(value);
    });
    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        updateVisualState();
        update();
    });

    rebuildLayout();
    updateVisualState();
}

QLineEdit* AntInput::lineEdit() const { return m_lineEdit; }
QString AntInput::text() const { return m_lineEdit->text(); }
void AntInput::setText(const QString& text) { m_lineEdit->setText(text); }
void AntInput::clear() { m_lineEdit->clear(); }
QString AntInput::placeholderText() const { return m_lineEdit->placeholderText(); }
void AntInput::setPlaceholderText(const QString& text) { m_lineEdit->setPlaceholderText(text); }
bool AntInput::isReadOnly() const { return m_lineEdit->isReadOnly(); }
void AntInput::setReadOnly(bool readOnly) { m_lineEdit->setReadOnly(readOnly); }
int AntInput::maxLength() const { return m_lineEdit->maxLength(); }
void AntInput::setMaxLength(int length) { m_lineEdit->setMaxLength(length); }
QLineEdit::EchoMode AntInput::echoMode() const { return m_lineEdit->echoMode(); }
void AntInput::setEchoMode(QLineEdit::EchoMode mode) { m_lineEdit->setEchoMode(mode); }
Qt::Alignment AntInput::alignment() const { return m_lineEdit->alignment(); }
void AntInput::setAlignment(Qt::Alignment alignment) { m_lineEdit->setAlignment(alignment); }
int AntInput::cursorPosition() const { return m_lineEdit->cursorPosition(); }
void AntInput::setCursorPosition(int position) { m_lineEdit->setCursorPosition(position); }
void AntInput::setSelection(int start, int length) { m_lineEdit->setSelection(start, length); }
void AntInput::selectAll() { m_lineEdit->selectAll(); }
void AntInput::deselect() { m_lineEdit->deselect(); }
QString AntInput::selectedText() const { return m_lineEdit->selectedText(); }
bool AntInput::hasSelectedText() const { return m_lineEdit->hasSelectedText(); }
void AntInput::copy() const { m_lineEdit->copy(); }
void AntInput::cut() { m_lineEdit->cut(); }
void AntInput::paste() { m_lineEdit->paste(); }
void AntInput::undo() { m_lineEdit->undo(); }
void AntInput::redo() { m_lineEdit->redo(); }
Ant::Size AntInput::inputSize() const { return m_inputSize; }
Ant::Status AntInput::status() const { return m_status; }
Ant::Variant AntInput::variant() const { return m_variant; }
bool AntInput::allowClear() const { return m_allowClear; }
bool AntInput::isPasswordMode() const { return m_passwordMode; }

void AntInput::setInputSize(Ant::Size size)
{
    if (m_inputSize == size)
        return;
    m_inputSize = size;
    rebuildLayout();
    updateGeometry();
    update();
    Q_EMIT inputSizeChanged(m_inputSize);
}

void AntInput::setStatus(Ant::Status status)
{
    if (m_status == status)
        return;
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

void AntInput::setError(bool error)
{
    setStatus(error ? Ant::Status::Error : Ant::Status::Normal);
}

void AntInput::setVariant(Ant::Variant variant)
{
    if (m_variant == variant)
        return;
    m_variant = variant;
    update();
    Q_EMIT variantChanged(m_variant);
}

void AntInput::setAllowClear(bool allowClear)
{
    if (m_allowClear == allowClear)
        return;
    m_allowClear = allowClear;
    updateButtonVisibility();
    Q_EMIT allowClearChanged(m_allowClear);
}

void AntInput::setPasswordMode(bool passwordMode)
{
    if (m_passwordMode == passwordMode)
        return;
    m_passwordMode = passwordMode;
    m_passwordVisible = false;
    m_lineEdit->setEchoMode(passwordMode ? QLineEdit::Password : QLineEdit::Normal);
    static_cast<AntInputIconButton*>(m_passwordButton)->setActive(false);
    updateButtonVisibility();
    Q_EMIT passwordModeChanged(m_passwordMode);
}

bool AntInput::isSearchMode() const { return m_searchMode; }
void AntInput::setSearchMode(bool searchMode)
{
    if (m_searchMode == searchMode) return;
    m_searchMode = searchMode;
    rebuildLayout();
    updateGeometry();
    update();
}

void AntInput::setAddonBefore(const QString& text)
{
    if (!m_addonBefore)
    {
        m_addonBefore = new QLabel(this);
        m_addonBefore->setAlignment(Qt::AlignCenter);
    }
    m_addonBefore->setText(text);
    m_addonBefore->setVisible(!text.isEmpty());
    rebuildLayout();
}

void AntInput::setAddonAfter(const QString& text)
{
    if (!m_addonAfter)
    {
        m_addonAfter = new QLabel(this);
        m_addonAfter->setAlignment(Qt::AlignCenter);
    }
    m_addonAfter->setText(text);
    m_addonAfter->setVisible(!text.isEmpty());
    rebuildLayout();
}

void AntInput::setPrefixIcon(const QIcon& icon)
{
    if (!m_prefixIconLabel)
    {
        m_prefixIconLabel = new QLabel(this);
        m_prefixIconLabel->setAlignment(Qt::AlignCenter);
    }
    const Metrics m = metrics();
    m_prefixIconLabel->setPixmap(icon.pixmap(m.iconSize, m.iconSize));
    m_prefixIconLabel->setVisible(!icon.isNull());
    rebuildLayout();
}

void AntInput::setSuffixIcon(const QIcon& icon)
{
    if (!m_suffixIconLabel)
    {
        m_suffixIconLabel = new QLabel(this);
        m_suffixIconLabel->setAlignment(Qt::AlignCenter);
    }
    const Metrics m = metrics();
    m_suffixIconLabel->setPixmap(icon.pixmap(m.iconSize, m.iconSize));
    m_suffixIconLabel->setVisible(!icon.isNull());
    rebuildLayout();
}

void AntInput::setPrefixWidget(QWidget* widget)
{
    if (m_prefixWidget && m_prefixWidget->parent() == this)
        m_prefixWidget->deleteLater();
    m_prefixWidget = widget;
    if (m_prefixWidget)
        m_prefixWidget->setParent(this);
    rebuildLayout();
}

void AntInput::setSuffixWidget(QWidget* widget)
{
    if (m_suffixWidget && m_suffixWidget->parent() == this)
        m_suffixWidget->deleteLater();
    m_suffixWidget = widget;
    if (m_suffixWidget)
        m_suffixWidget->setParent(this);
    rebuildLayout();
}

QSize AntInput::sizeHint() const
{
    return QSize(220, metrics().height);
}

QSize AntInput::minimumSizeHint() const
{
    return QSize(80, metrics().height);
}

void AntInput::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void AntInput::leaveEvent(QEvent* event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}

bool AntInput::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_lineEdit)
    {
        if (event->type() == QEvent::FocusIn)
        {
            m_focused = true;
            update();
            Q_EMIT focusIn();
        }
        else if (event->type() == QEvent::FocusOut)
        {
            m_focused = false;
            update();
            Q_EMIT focusOut();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void AntInput::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        m_lineEdit->setEnabled(isEnabled());
        updateButtonVisibility();
        updateVisualState();
        update();
    }
    QWidget::changeEvent(event);
}

AntInput::Metrics AntInput::metrics() const
{
    const auto& token = antTheme->tokens();
    Metrics m;
    switch (m_inputSize)
    {
    case Ant::Size::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.paddingX = token.paddingSM;
        m.radius = token.borderRadiusLG;
        m.iconSize = 18;
        break;
    case Ant::Size::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS;
        m.radius = token.borderRadiusSM;
        m.iconSize = 14;
        break;
    case Ant::Size::Middle:
        m.height = token.controlHeight;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingSM - token.lineWidth;
        m.radius = token.borderRadius;
        m.iconSize = 16;
        break;
    }
    return m;
}

void AntInput::rebuildLayout()
{
    while (QLayoutItem* item = m_layout->takeAt(0))
        delete item;

    const Metrics m = metrics();
    auto setupLabel = [&](QLabel* label) {
        if (!label)
            return;
        label->setMinimumHeight(m.height);
        label->setContentsMargins(m.paddingX, 0, m.paddingX, 0);
    };
    setupLabel(m_addonBefore);
    setupLabel(m_addonAfter);
    m_searchButton->setFixedSize(m.height, m.height);

    if (m_addonBefore)
        m_layout->addWidget(m_addonBefore);
    m_layout->addSpacing(m.paddingX);
    if (m_prefixIconLabel)
    {
        m_layout->addWidget(m_prefixIconLabel);
        m_layout->addSpacing(6);
    }
    if (m_prefixWidget)
    {
        m_layout->addWidget(m_prefixWidget);
        m_layout->addSpacing(6);
    }
    m_layout->addWidget(m_lineEdit, 1);
    if (m_suffixWidget)
    {
        m_layout->addSpacing(6);
        m_layout->addWidget(m_suffixWidget);
    }
    if (m_suffixIconLabel)
    {
        m_layout->addSpacing(6);
        m_layout->addWidget(m_suffixIconLabel);
    }
    m_layout->addWidget(m_clearButton);
    m_layout->addWidget(m_passwordButton);
    m_layout->addWidget(m_searchButton);
    if (!m_searchMode)
    {
        m_layout->addSpacing(m.paddingX);
    }
    if (m_addonAfter)
        m_layout->addWidget(m_addonAfter);

    updateButtonVisibility();
    updateVisualState();
}

void AntInput::updateButtonVisibility()
{
    m_clearButton->setVisible(m_allowClear && !m_lineEdit->text().isEmpty() && isEnabled());
    m_passwordButton->setVisible(m_passwordMode);
    m_searchButton->setVisible(m_searchMode);
}

void AntInput::updateVisualState()
{
    const auto& token = antTheme->tokens();
    const Metrics m = metrics();
    QFont f = m_lineEdit->font();
    f.setPixelSize(m.fontSize);
    m_lineEdit->setFont(f);
    m_lineEdit->setMinimumHeight(m.height);
    setMinimumHeight(m.height);
    setMaximumHeight(m.height);

    // LineEdit colors via QPalette
    QPalette lePalette = m_lineEdit->palette();
    lePalette.setColor(QPalette::Base, Qt::transparent);
    lePalette.setColor(QPalette::Text, isEnabled() ? token.colorText : token.colorTextDisabled);
    lePalette.setColor(QPalette::PlaceholderText, isEnabled() ? token.colorTextPlaceholder : token.colorTextDisabled);
    lePalette.setColor(QPalette::Highlight, token.colorPrimary);
    lePalette.setColor(QPalette::HighlightedText, Qt::white);
    m_lineEdit->setPalette(lePalette);
    m_lineEdit->setAttribute(Qt::WA_TranslucentBackground, true);
    m_lineEdit->setStyleSheet(QStringLiteral("QLineEdit { background: transparent; border: none; padding: 0; }"));

    // ToolButton colors via QPalette
    for (auto* btn : {m_clearButton, m_passwordButton, m_searchButton})
    {
        QPalette tbPalette = btn->palette();
        tbPalette.setColor(QPalette::ButtonText, token.colorTextTertiary);
        btn->setPalette(tbPalette);
    }

    // Addon label colors via QPalette
    if (m_addonBefore)
    {
        QPalette abPalette = m_addonBefore->palette();
        abPalette.setColor(QPalette::WindowText, token.colorText);
        m_addonBefore->setPalette(abPalette);
    }
    if (m_addonAfter)
    {
        QPalette aaPalette = m_addonAfter->palette();
        aaPalette.setColor(QPalette::WindowText, token.colorText);
        m_addonAfter->setPalette(aaPalette);
    }
}

QColor AntInput::borderColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
        return token.colorBorderDisabled;
    if (m_status == Ant::Status::Error)
        return m_hovered || m_focused ? token.colorErrorHover : token.colorError;
    if (m_status == Ant::Status::Warning)
        return m_hovered || m_focused ? token.colorWarningHover : token.colorWarning;
    if (m_focused)
        return token.colorPrimary;
    if (m_hovered)
        return token.colorPrimaryHover;
    return token.colorBorder;
}

bool AntInput::isInputFocused() const
{
    return m_focused;
}

bool AntInput::isInputHovered() const
{
    return m_hovered;
}

QRect AntInput::addonBeforeRect() const
{
    return m_addonBefore && m_addonBefore->isVisible() ? m_addonBefore->geometry() : QRect();
}

QRect AntInput::addonAfterRect() const
{
    return m_addonAfter && m_addonAfter->isVisible() ? m_addonAfter->geometry() : QRect();
}

QRect AntInput::searchButtonRect() const
{
    return m_searchMode && m_searchButton && m_searchButton->isVisible() ? m_searchButton->geometry() : QRect();
}
