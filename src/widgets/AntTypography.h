#pragma once

#include "core/QtAntDesignExport.h"

#include <QPointer>
#include <QWidget>

#include "core/AntTypes.h"

class QEvent;
class QMouseEvent;

class QT_ANT_DESIGN_EXPORT AntTypography : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(Ant::TypographyType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(Ant::TypographyTitleLevel titleLevel READ titleLevel WRITE setTitleLevel NOTIFY titleLevelChanged)
    Q_PROPERTY(bool title READ isTitle WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool paragraph READ isParagraph WRITE setParagraph NOTIFY paragraphChanged)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap NOTIFY paragraphChanged)
    Q_PROPERTY(bool disabled READ isDisabled WRITE setDisabled NOTIFY disabledChanged)
    Q_PROPERTY(bool strong READ isStrong WRITE setStrong NOTIFY strongChanged)
    Q_PROPERTY(bool underline READ isUnderline WRITE setUnderline NOTIFY underlineChanged)
    Q_PROPERTY(bool delete_ READ isDelete WRITE setDelete NOTIFY deleteChanged)
    Q_PROPERTY(bool code READ isCode WRITE setCode NOTIFY codeChanged)
    Q_PROPERTY(bool mark READ isMark WRITE setMark NOTIFY markChanged)
    Q_PROPERTY(bool italic READ isItalic WRITE setItalic NOTIFY italicChanged)
    Q_PROPERTY(bool copyable READ isCopyable WRITE setCopyable NOTIFY copyableChanged)
    Q_PROPERTY(bool ellipsis READ isEllipsis WRITE setEllipsis NOTIFY ellipsisChanged)
    Q_PROPERTY(int ellipsisRows READ ellipsisRows WRITE setEllipsisRows NOTIFY ellipsisRowsChanged)
    Q_PROPERTY(QString href READ href WRITE setHref NOTIFY hrefChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

public:
    explicit AntTypography(QWidget* parent = nullptr);
    explicit AntTypography(const QString& text, QWidget* parent = nullptr);

    QString text() const;
    void setText(const QString& text);
    void clear();

    Ant::TypographyType type() const;
    void setType(Ant::TypographyType type);

    bool isTitle() const;
    void setTitle(bool title);
    Ant::TypographyTitleLevel titleLevel() const;
    void setTitleLevel(Ant::TypographyTitleLevel level);

    bool isParagraph() const;
    void setParagraph(bool paragraph);
    bool wordWrap() const;
    void setWordWrap(bool wordWrap);

    bool isDisabled() const;
    void setDisabled(bool disabled);
    bool isStrong() const;
    void setStrong(bool strong);
    bool isUnderline() const;
    void setUnderline(bool underline);
    bool isDelete() const;
    void setDelete(bool del);
    bool isCode() const;
    void setCode(bool code);
    bool isMark() const;
    void setMark(bool mark);
    bool isItalic() const;
    void setItalic(bool italic);

    bool isCopyable() const;
    void setCopyable(bool copyable);
    bool isEllipsis() const;
    void setEllipsis(bool ellipsis);
    int ellipsisRows() const;
    void setEllipsisRows(int rows);

    QString href() const;
    void setHref(const QString& href);
    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    bool isPressed() const;
    bool isCopyHovered() const;
    bool isCopyPressed() const;
    bool isCopied() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    void textChanged(const QString& text);
    void typeChanged(Ant::TypographyType type);
    void titleChanged(bool title);
    void titleLevelChanged(Ant::TypographyTitleLevel level);
    void paragraphChanged(bool paragraph);
    void disabledChanged(bool disabled);
    void strongChanged(bool strong);
    void underlineChanged(bool underline);
    void deleteChanged(bool del);
    void codeChanged(bool code);
    void markChanged(bool mark);
    void italicChanged(bool italic);
    void copyableChanged(bool copyable);
    void ellipsisChanged(bool ellipsis);
    void ellipsisRowsChanged(int rows);
    void hrefChanged(const QString& href);
    void alignmentChanged(Qt::Alignment alignment);
    void copied(const QString& text);
    void linkActivated(const QString& href);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    int fontSizeForLevel() const;
    QFont createFont() const;
    QColor textColor() const;
    QRect textDrawRect() const;
    QRect copyButtonRect() const;

    QString m_text;
    Ant::TypographyType m_type = Ant::TypographyType::Default;
    Ant::TypographyTitleLevel m_titleLevel = Ant::TypographyTitleLevel::H1;
    bool m_title = false;
    bool m_paragraph = false;
    bool m_disabled = false;
    bool m_strong = false;
    bool m_underline = false;
    bool m_delete = false;
    bool m_code = false;
    bool m_mark = false;
    bool m_italic = false;
    bool m_copyable = false;
    bool m_ellipsis = false;
    int m_ellipsisRows = 1;
    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
    bool m_copyHovered = false;
    bool m_copyPressed = false;
    bool m_copied = false;
    bool m_pressed = false;
    QString m_href;
};
