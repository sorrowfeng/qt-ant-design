#include "AntTypography.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QTimer>
#include <QUrl>

#include "core/AntTheme.h"
#include "styles/AntTypographyStyle.h"

AntTypography::AntTypography(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntTypographyStyle(style()));
    setMouseTracking(true);
}

AntTypography::AntTypography(const QString& text, QWidget* parent)
    : AntTypography(parent)
{
    m_text = text;
}

QString AntTypography::text() const { return m_text; }

void AntTypography::setText(const QString& text)
{
    if (m_text == text)
    {
        return;
    }
    m_text = text;
    updateGeometry();
    update();
    Q_EMIT textChanged(m_text);
}

Ant::TypographyType AntTypography::type() const { return m_type; }

void AntTypography::setType(Ant::TypographyType type)
{
    if (m_type == type)
    {
        return;
    }
    m_type = type;
    setCursor(m_disabled ? Qt::ForbiddenCursor : (m_type == Ant::TypographyType::Link ? Qt::PointingHandCursor : Qt::ArrowCursor));
    update();
    Q_EMIT typeChanged(m_type);
}

bool AntTypography::isTitle() const { return m_title; }

void AntTypography::setTitle(bool title)
{
    if (m_title == title)
    {
        return;
    }
    m_title = title;
    updateGeometry();
    update();
    Q_EMIT titleChanged(m_title);
}

Ant::TypographyTitleLevel AntTypography::titleLevel() const { return m_titleLevel; }

void AntTypography::setTitleLevel(Ant::TypographyTitleLevel level)
{
    if (m_titleLevel == level)
    {
        return;
    }
    m_titleLevel = level;
    if (m_title)
    {
        updateGeometry();
    }
    update();
    Q_EMIT titleLevelChanged(m_titleLevel);
}

bool AntTypography::isParagraph() const { return m_paragraph; }

void AntTypography::setParagraph(bool paragraph)
{
    if (m_paragraph == paragraph)
    {
        return;
    }
    m_paragraph = paragraph;
    updateGeometry();
    update();
    Q_EMIT paragraphChanged(m_paragraph);
}

bool AntTypography::isDisabled() const { return m_disabled; }

void AntTypography::setDisabled(bool disabled)
{
    if (m_disabled == disabled)
    {
        return;
    }
    m_disabled = disabled;
    setCursor(m_disabled ? Qt::ForbiddenCursor : (m_type == Ant::TypographyType::Link ? Qt::PointingHandCursor : Qt::ArrowCursor));
    update();
    Q_EMIT disabledChanged(m_disabled);
}

bool AntTypography::isStrong() const { return m_strong; }

void AntTypography::setStrong(bool strong)
{
    if (m_strong == strong)
    {
        return;
    }
    m_strong = strong;
    updateGeometry();
    update();
    Q_EMIT strongChanged(m_strong);
}

bool AntTypography::isUnderline() const { return m_underline; }

void AntTypography::setUnderline(bool underline)
{
    if (m_underline == underline)
    {
        return;
    }
    m_underline = underline;
    update();
    Q_EMIT underlineChanged(m_underline);
}

bool AntTypography::isDelete() const { return m_delete; }

void AntTypography::setDelete(bool del)
{
    if (m_delete == del)
    {
        return;
    }
    m_delete = del;
    update();
    Q_EMIT deleteChanged(m_delete);
}

bool AntTypography::isCode() const { return m_code; }

void AntTypography::setCode(bool code)
{
    if (m_code == code)
    {
        return;
    }
    m_code = code;
    updateGeometry();
    update();
    Q_EMIT codeChanged(m_code);
}

bool AntTypography::isMark() const { return m_mark; }

void AntTypography::setMark(bool mark)
{
    if (m_mark == mark)
    {
        return;
    }
    m_mark = mark;
    update();
    Q_EMIT markChanged(m_mark);
}

bool AntTypography::isItalic() const { return m_italic; }

void AntTypography::setItalic(bool italic)
{
    if (m_italic == italic)
    {
        return;
    }
    m_italic = italic;
    update();
    Q_EMIT italicChanged(m_italic);
}

bool AntTypography::isCopyable() const { return m_copyable; }

void AntTypography::setCopyable(bool copyable)
{
    if (m_copyable == copyable)
    {
        return;
    }
    m_copyable = copyable;
    updateGeometry();
    update();
    Q_EMIT copyableChanged(m_copyable);
}

bool AntTypography::isEllipsis() const { return m_ellipsis; }

void AntTypography::setEllipsis(bool ellipsis)
{
    if (m_ellipsis == ellipsis)
    {
        return;
    }
    m_ellipsis = ellipsis;
    updateGeometry();
    update();
    Q_EMIT ellipsisChanged(m_ellipsis);
}

int AntTypography::ellipsisRows() const { return m_ellipsisRows; }

void AntTypography::setEllipsisRows(int rows)
{
    const int clamped = qMax(1, rows);
    if (m_ellipsisRows == clamped)
    {
        return;
    }
    m_ellipsisRows = clamped;
    if (m_ellipsis)
    {
        updateGeometry();
        update();
    }
    Q_EMIT ellipsisRowsChanged(m_ellipsisRows);
}

QString AntTypography::href() const { return m_href; }

void AntTypography::setHref(const QString& href)
{
    if (m_href == href)
    {
        return;
    }
    m_href = href;
    Q_EMIT hrefChanged(m_href);
}

bool AntTypography::isPressed() const { return m_pressed; }

bool AntTypography::isCopyHovered() const { return m_copyHovered; }

bool AntTypography::isCopyPressed() const { return m_copyPressed; }

bool AntTypography::isCopied() const { return m_copied; }

QSize AntTypography::sizeHint() const
{
    const auto& token = antTheme->tokens();
    QFont f = createFont();
    QFontMetrics fm(f);

    const int copyBtnWidth = m_copyable ? token.fontSize + token.paddingXXS * 2 : 0;

    if (m_code)
    {
        const int codePadX = qMax(1, qRound(f.pixelSize() * 0.4));
        const int codePadTop = qMax(1, qRound(f.pixelSize() * 0.2));
        const int codePadBottom = qMax(1, qRound(f.pixelSize() * 0.1));
        const int textW = fm.horizontalAdvance(m_text) + codePadX * 2 + copyBtnWidth;
        const int textH = fm.height() + codePadTop + codePadBottom + token.lineWidth * 2;
        return QSize(qMax(60, textW), qMax(fm.height(), textH));
    }

    if (m_paragraph || m_ellipsis)
    {
        const int availWidth = qMax(200, width() > 0 ? width() : 400);
        QRect textRect(0, 0, availWidth - copyBtnWidth, QWIDGETSIZE_MAX);
        int flags = Qt::TextWordWrap;
        if (m_ellipsis)
        {
            textRect.setHeight(fm.height() * m_ellipsisRows);
        }
        QRect br = fm.boundingRect(textRect, flags, m_text);
        return QSize(qMax(120, br.width() + copyBtnWidth + token.paddingXS), br.height() + token.paddingXS);
    }

    const int textW = fm.horizontalAdvance(m_text) + copyBtnWidth;
    const int textH = fm.height() + 4;
    return QSize(qMax(60, textW), qMax(24, textH));
}

QSize AntTypography::minimumSizeHint() const
{
    QFont f = createFont();
    QFontMetrics fm(f);
    return QSize(40, fm.height() + 4);
}

void AntTypography::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}

void AntTypography::mousePressEvent(QMouseEvent* event)
{
    if (m_disabled)
    {
        QWidget::mousePressEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton && m_copyable && copyButtonRect().contains(event->pos()))
    {
        m_copyPressed = true;
        update();
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton && m_type == Ant::TypographyType::Link && !m_href.isEmpty())
    {
        m_pressed = true;
        update();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void AntTypography::mouseMoveEvent(QMouseEvent* event)
{
    const bool copyHovered = m_copyable && copyButtonRect().contains(event->pos());
    if (m_copyHovered != copyHovered)
    {
        m_copyHovered = copyHovered;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void AntTypography::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_copyPressed)
    {
        const bool triggerCopy = !m_disabled && copyButtonRect().contains(event->pos());
        m_copyPressed = false;
        if (triggerCopy)
        {
            QApplication::clipboard()->setText(m_text);
            m_copied = true;
            QTimer::singleShot(2000, this, [this]() {
                if (m_copied)
                {
                    m_copied = false;
                    update();
                }
            });
            Q_EMIT copied(m_text);
        }
        update();
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && m_pressed)
    {
        const bool triggerLink = !m_disabled && rect().contains(event->pos()) && !m_href.isEmpty();
        m_pressed = false;
        if (triggerLink)
        {
            Q_EMIT linkActivated(m_href);
            QDesktopServices::openUrl(QUrl(m_href));
        }
        update();
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void AntTypography::leaveEvent(QEvent* event)
{
    if (m_copyHovered)
    {
        m_copyHovered = false;
        update();
    }
    QWidget::leaveEvent(event);
}

void AntTypography::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

int AntTypography::fontSizeForLevel() const
{
    const auto& token = antTheme->tokens();
    switch (m_titleLevel)
    {
    case Ant::TypographyTitleLevel::H1:
        return 38;
    case Ant::TypographyTitleLevel::H2:
        return 30;
    case Ant::TypographyTitleLevel::H3:
        return 24;
    case Ant::TypographyTitleLevel::H4:
        return token.fontSizeXL;
    case Ant::TypographyTitleLevel::H5:
        return token.fontSizeLG;
    default:
        return token.fontSize;
    }
}

QFont AntTypography::createFont() const
{
    const auto& token = antTheme->tokens();
    QFont f = font();

    if (m_title)
    {
        f.setPixelSize(fontSizeForLevel());
        f.setWeight(QFont::DemiBold);
    }
    else if (m_code)
    {
        f.setFamily(QStringLiteral("Consolas, Courier New, monospace"));
        f.setPixelSize(qMax(1, qRound(token.fontSize * 0.85)));
    }
    else
    {
        f.setPixelSize(token.fontSize);
    }

    if (m_strong)
    {
        f.setWeight(QFont::DemiBold);
    }
    if (m_italic)
    {
        f.setItalic(true);
    }
    if (m_underline)
    {
        f.setUnderline(true);
    }

    return f;
}

QColor AntTypography::textColor() const
{
    const auto& token = antTheme->tokens();
    if (m_disabled)
    {
        return token.colorTextDisabled;
    }
    switch (m_type)
    {
    case Ant::TypographyType::Secondary:
        return token.colorTextTertiary;
    case Ant::TypographyType::Success:
        return token.colorSuccess;
    case Ant::TypographyType::Warning:
        return token.colorWarning;
    case Ant::TypographyType::Danger:
        return token.colorError;
    case Ant::TypographyType::LightSolid:
        return token.colorTextLightSolid;
    case Ant::TypographyType::Link:
        return token.colorLink;
    case Ant::TypographyType::Default:
    default:
        return token.colorText;
    }
}

QRect AntTypography::textDrawRect() const
{
    const auto& token = antTheme->tokens();
    const int copyBtnW = m_copyable ? antTheme->tokens().fontSize + antTheme->tokens().paddingXXS * 2 : 0;
    const int pad = m_code ? token.paddingXXS : 0;
    return QRect(pad, pad, width() - pad * 2 - copyBtnW, height() - pad * 2);
}

QRect AntTypography::copyButtonRect() const
{
    if (!m_copyable)
    {
        return QRect();
    }
    const auto& token = antTheme->tokens();
    QFont f = createFont();
    QFontMetrics fm(f);
    const int iconSize = token.fontSize;
    const int gap = token.paddingXXS;
    const int btnW = iconSize + gap * 2;
    const int textW = qMin(fm.horizontalAdvance(m_text), qMax(0, width() - btnW));
    const int x = qMin(textW + gap, qMax(0, width() - btnW));
    const int y = qMax(0, (fm.height() - iconSize) / 2);
    return QRect(x, y, btnW, iconSize + gap);
}
