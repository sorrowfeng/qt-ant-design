#include "AntInput.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>

#include "../styles/AntInputStyle.h"
#include "core/AntTheme.h"

AntInput::AntInput(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_Hover, true);
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

    m_clearButton = new QToolButton(this);
    m_clearButton->setText(QStringLiteral("x"));
    m_clearButton->setAutoRaise(true);
    m_clearButton->setFocusPolicy(Qt::NoFocus);
    connect(m_clearButton, &QToolButton::clicked, m_lineEdit, &QLineEdit::clear);

    m_passwordButton = new QToolButton(this);
    m_passwordButton->setText(QStringLiteral("..."));
    m_passwordButton->setAutoRaise(true);
    m_passwordButton->setFocusPolicy(Qt::NoFocus);
    m_passwordButton->hide();
    connect(m_passwordButton, &QToolButton::clicked, this, [this]() {
        m_passwordVisible = !m_passwordVisible;
        m_lineEdit->setEchoMode(m_passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
        m_passwordButton->setText(m_passwordVisible ? QStringLiteral("hide") : QStringLiteral("..."));
    });

    m_searchButton = new QToolButton(this);
    m_searchButton->setText(QStringLiteral("⌕"));
    m_searchButton->setAutoRaise(true);
    m_searchButton->setFocusPolicy(Qt::NoFocus);
    m_searchButton->hide();
    connect(m_searchButton, &QToolButton::clicked, this, [this]() {
        Q_EMIT searchTriggered(m_lineEdit->text());
    });
    connect(m_lineEdit, &QLineEdit::returnPressed, this, [this]() {
        if (m_searchMode) Q_EMIT searchTriggered(m_lineEdit->text());
    });

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
void AntInput::setPlaceholderText(const QString& text) { m_lineEdit->setPlaceholderText(text); }
Ant::InputSize AntInput::inputSize() const { return m_inputSize; }
Ant::InputStatus AntInput::status() const { return m_status; }
Ant::InputVariant AntInput::variant() const { return m_variant; }
bool AntInput::allowClear() const { return m_allowClear; }
bool AntInput::isPasswordMode() const { return m_passwordMode; }

void AntInput::setInputSize(Ant::InputSize size)
{
    if (m_inputSize == size)
        return;
    m_inputSize = size;
    rebuildLayout();
    updateGeometry();
    update();
    Q_EMIT inputSizeChanged(m_inputSize);
}

void AntInput::setStatus(Ant::InputStatus status)
{
    if (m_status == status)
        return;
    m_status = status;
    update();
    Q_EMIT statusChanged(m_status);
}

void AntInput::setError(bool error)
{
    setStatus(error ? Ant::InputStatus::Error : Ant::InputStatus::Normal);
}

void AntInput::setVariant(Ant::InputVariant variant)
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
    updateButtonVisibility();
    Q_EMIT passwordModeChanged(m_passwordMode);
}

bool AntInput::isSearchMode() const { return m_searchMode; }
void AntInput::setSearchMode(bool searchMode)
{
    if (m_searchMode == searchMode) return;
    m_searchMode = searchMode;
    updateButtonVisibility();
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
    case Ant::InputSize::Large:
        m.height = token.controlHeightLG;
        m.fontSize = token.fontSizeLG;
        m.paddingX = token.paddingSM;
        m.radius = token.borderRadiusLG;
        m.iconSize = 18;
        break;
    case Ant::InputSize::Small:
        m.height = token.controlHeightSM;
        m.fontSize = token.fontSize;
        m.paddingX = token.paddingXS;
        m.radius = token.borderRadiusSM;
        m.iconSize = 14;
        break;
    case Ant::InputSize::Middle:
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
    m_layout->addSpacing(m.paddingX);
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

    const QString textColor = isEnabled() ? token.colorText.name(QColor::HexArgb) : token.colorTextDisabled.name(QColor::HexArgb);
    m_lineEdit->setStyleSheet(QStringLiteral("QLineEdit { background: transparent; border: none; color: %1; selection-background-color: %2; }")
                                  .arg(textColor, token.colorPrimary.name(QColor::HexArgb)));

    const QString toolStyle = QStringLiteral("QToolButton { border: none; background: transparent; color: %1; padding: 0 4px; }")
                                  .arg(token.colorTextTertiary.name(QColor::HexArgb));
    m_clearButton->setStyleSheet(toolStyle);
    m_passwordButton->setStyleSheet(toolStyle);
    const QString labelStyle = QStringLiteral("QLabel { color: %1; background: transparent; }").arg(token.colorText.name(QColor::HexArgb));
    if (m_addonBefore)
        m_addonBefore->setStyleSheet(labelStyle);
    if (m_addonAfter)
        m_addonAfter->setStyleSheet(labelStyle);
}

QColor AntInput::borderColor() const
{
    const auto& token = antTheme->tokens();
    if (!isEnabled())
        return token.colorBorderDisabled;
    if (m_status == Ant::InputStatus::Error)
        return m_hovered || m_focused ? token.colorErrorHover : token.colorError;
    if (m_status == Ant::InputStatus::Warning)
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
