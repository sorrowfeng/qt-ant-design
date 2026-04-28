#include "AntDescriptions.h"

#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QResizeEvent>
#include <utility>
#include <QVBoxLayout>

#include "../styles/AntDescriptionsStyle.h"
#include "core/AntTheme.h"

namespace
{

class BorderedCell : public QWidget
{
public:
    BorderedCell(QWidget* parent, const QColor& bg, const QColor& border)
        : QWidget(parent), m_bg(bg), m_border(border) {}

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(m_border, 1));
        p.setBrush(m_bg);
        p.drawRect(rect().adjusted(0, 0, -1, -1));
    }

private:
    QColor m_bg;
    QColor m_border;
};

} // namespace

AntDescriptionsItem::AntDescriptionsItem(QWidget* parent)
    : QWidget(parent)
{
    hide();
}

AntDescriptionsItem::AntDescriptionsItem(const QString& label, const QString& content, QWidget* parent)
    : AntDescriptionsItem(parent)
{
    m_label = label;
    m_content = content;
}

QString AntDescriptionsItem::label() const { return m_label; }
void AntDescriptionsItem::setLabel(const QString& label)
{
    if (m_label == label) return;
    m_label = label;
    Q_EMIT labelChanged(m_label);
}

QString AntDescriptionsItem::content() const { return m_content; }
void AntDescriptionsItem::setContent(const QString& content)
{
    if (m_content == content) return;
    m_content = content;
    Q_EMIT contentChanged(m_content);
}

int AntDescriptionsItem::span() const { return m_span; }
void AntDescriptionsItem::setSpan(int span)
{
    span = qMax(1, span);
    if (m_span == span) return;
    m_span = span;
    Q_EMIT spanChanged(m_span);
}

QWidget* AntDescriptionsItem::contentWidget() const { return m_contentWidget.data(); }
void AntDescriptionsItem::setContentWidget(QWidget* widget)
{
    if (m_contentWidget == widget) return;
    if (m_contentWidget) m_contentWidget->setParent(nullptr);
    m_contentWidget = widget;
    Q_EMIT contentChanged(m_content);
}

AntDescriptions::AntDescriptions(QWidget* parent)
    : QWidget(parent)
{
    setStyle(new AntDescriptionsStyle(style()));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_root = new QVBoxLayout(this);
    m_root->setContentsMargins(0, 0, 0, 0);
    m_root->setSpacing(16);

    m_header = new QWidget(this);
    auto* headerLayout = new QHBoxLayout(m_header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(12);
    m_titleLabel = new QLabel(m_header);
    m_extraLabel = new QLabel(m_header);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    headerLayout->addWidget(m_titleLabel, 1);
    headerLayout->addWidget(m_extraLabel);

    m_body = new QWidget(this);
    m_body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_grid = new QGridLayout(m_body);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setHorizontalSpacing(0);
    m_grid->setVerticalSpacing(0);

    m_root->addWidget(m_header);
    m_root->addWidget(m_body);

    rebuildLayout();
    updateTheme();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        rebuildGrid();
        updateTheme();
    });
}

QString AntDescriptions::title() const { return m_title; }
void AntDescriptions::setTitle(const QString& title)
{
    if (m_title == title) return;
    m_title = title;
    rebuildLayout();
    updateTheme();
    Q_EMIT titleChanged(m_title);
}

QString AntDescriptions::extra() const { return m_extra; }
void AntDescriptions::setExtra(const QString& extra)
{
    if (m_extra == extra) return;
    m_extra = extra;
    rebuildLayout();
    updateTheme();
    Q_EMIT extraChanged(m_extra);
}

int AntDescriptions::columnCount() const { return m_columnCount; }
void AntDescriptions::setColumnCount(int count)
{
    count = qMax(1, count);
    if (m_columnCount == count) return;
    m_columnCount = count;
    rebuildGrid();
    Q_EMIT columnCountChanged(m_columnCount);
}

bool AntDescriptions::isBordered() const { return m_bordered; }
void AntDescriptions::setBordered(bool bordered)
{
    if (m_bordered == bordered) return;
    m_bordered = bordered;
    rebuildGrid();
    update();
    Q_EMIT borderedChanged(m_bordered);
}

bool AntDescriptions::isVertical() const { return m_vertical; }
void AntDescriptions::setVertical(bool vertical)
{
    if (m_vertical == vertical) return;
    m_vertical = vertical;
    rebuildGrid();
    Q_EMIT verticalChanged(m_vertical);
}

QList<AntDescriptionsItem*> AntDescriptions::items() const { return m_items; }

void AntDescriptions::addItem(AntDescriptionsItem* item)
{
    if (!item || m_items.contains(item)) return;
    item->setParent(this);
    item->hide();
    m_items.append(item);
    connect(item, &AntDescriptionsItem::labelChanged, this, [this]() { rebuildGrid(); });
    connect(item, &AntDescriptionsItem::contentChanged, this, [this]() { rebuildGrid(); });
    connect(item, &AntDescriptionsItem::spanChanged, this, [this]() { rebuildGrid(); });
    rebuildGrid();
}

AntDescriptionsItem* AntDescriptions::addItem(const QString& label, const QString& content, int span)
{
    auto* item = new AntDescriptionsItem(label, content, this);
    item->setSpan(span);
    addItem(item);
    return item;
}

void AntDescriptions::clearItems()
{
    for (AntDescriptionsItem* item : std::as_const(m_items))
    {
        if (item) item->deleteLater();
    }
    m_items.clear();
    rebuildGrid();
}

QSize AntDescriptions::sizeHint() const
{
    const QSize bodyHint = m_body ? m_body->sizeHint() : QSize();
    const QSize headerHint = (m_header && m_header->isVisible()) ? m_header->sizeHint() : QSize();
    const int spacing = headerHint.isValid() && bodyHint.isValid() ? m_root->spacing() : 0;
    return QSize(qMax(bodyHint.width(), headerHint.width()), bodyHint.height() + headerHint.height() + spacing);
}

QSize AntDescriptions::minimumSizeHint() const
{
    return sizeHint();
}

void AntDescriptions::paintEvent(QPaintEvent* event) { Q_UNUSED(event) }

void AntDescriptions::resizeEvent(QResizeEvent* event) { QWidget::resizeEvent(event); }

void AntDescriptions::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        updateTheme();
    }
    QWidget::changeEvent(event);
}

void AntDescriptions::rebuildLayout()
{
    m_header->setVisible(!m_title.isEmpty() || !m_extra.isEmpty());
    m_titleLabel->setText(m_title);
    m_extraLabel->setText(m_extra);
    rebuildGrid();
}

void AntDescriptions::rebuildGrid()
{
    while (QLayoutItem* item = m_grid->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }

    const int columns = qMax(1, m_columnCount);
    int row = 0;
    int col = 0;

    for (AntDescriptionsItem* item : std::as_const(m_items))
    {
        if (!item) continue;
        int span = qMin(columns, qMax(1, item->span()));
        if (col + span > columns)
        {
            ++row;
            col = 0;
        }

        if (m_vertical)
        {
            QWidget* labelCell = buildLabelCell(item->label());
            QWidget* contentCell = buildContentCell(item);
            m_grid->addWidget(labelCell, row * 2, col, 1, span);
            m_grid->addWidget(contentCell, row * 2 + 1, col, 1, span);
        }
        else
        {
            QWidget* labelCell = buildLabelCell(item->label());
            QWidget* contentCell = buildContentCell(item);
            m_grid->addWidget(labelCell, row, col * 2, 1, 1);
            m_grid->addWidget(contentCell, row, col * 2 + 1, 1, span * 2 - 1);
        }

        col += span;
        if (col >= columns)
        {
            ++row;
            col = 0;
        }
    }

    updateTheme();
}

void AntDescriptions::updateTheme()
{
    const auto& token = antTheme->tokens();

    QFont titleFont = font();
    titleFont.setPixelSize(token.fontSizeLG);
    titleFont.setWeight(QFont::DemiBold);
    m_titleLabel->setFont(titleFont);
    m_extraLabel->setFont(font());

    QPalette p1 = m_titleLabel->palette();
    p1.setColor(QPalette::WindowText, token.colorText);
    m_titleLabel->setPalette(p1);

    QPalette p2 = m_extraLabel->palette();
    p2.setColor(QPalette::WindowText, token.colorTextSecondary);
    m_extraLabel->setPalette(p2);
}

QWidget* AntDescriptions::buildLabelCell(const QString& text)
{
    const auto& token = antTheme->tokens();
    QWidget* cell = m_bordered
        ? new BorderedCell(m_body, token.colorFillQuaternary, token.colorSplit)
        : new QWidget(m_body);
    auto* layout = new QVBoxLayout(cell);
    layout->setContentsMargins(token.paddingLG, token.padding, token.paddingLG, token.padding);
    layout->setSpacing(0);
    auto* label = new QLabel(text, cell);
    label->setWordWrap(true);
    QFont f = label->font();
    f.setPixelSize(token.fontSize);
    label->setFont(f);
    QPalette p = label->palette();
    p.setColor(QPalette::WindowText, token.colorTextSecondary);
    label->setPalette(p);
    layout->addWidget(label, 0, Qt::AlignVCenter);
    return cell;
}

QWidget* AntDescriptions::buildContentCell(AntDescriptionsItem* item)
{
    const auto& token = antTheme->tokens();
    QWidget* cell = m_bordered
        ? new BorderedCell(m_body, token.colorBgContainer, token.colorSplit)
        : new QWidget(m_body);
    auto* layout = new QVBoxLayout(cell);
    layout->setContentsMargins(token.paddingLG, token.padding, token.paddingLG, token.padding);
    layout->setSpacing(0);
    if (item->contentWidget())
    {
        item->contentWidget()->setParent(cell);
        layout->addWidget(item->contentWidget(), 0, Qt::AlignVCenter);
        item->contentWidget()->show();
    }
    else
    {
        auto* label = new QLabel(item->content(), cell);
        label->setWordWrap(true);
        QFont f = label->font();
        f.setPixelSize(token.fontSize);
        label->setFont(f);
        QPalette p = label->palette();
        p.setColor(QPalette::WindowText, token.colorText);
        label->setPalette(p);
        layout->addWidget(label, 0, Qt::AlignVCenter);
    }
    return cell;
}
