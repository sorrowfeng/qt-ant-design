#include "Pages.h"

#include <QAction>
#include <QComboBox>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"
#include "widgets/AntCard.h"
#include "widgets/AntButton.h"
#include "widgets/AntDialog.h"
#include "widgets/AntDockManager.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntFileDialog.h"
#include "widgets/AntInputDialog.h"
#include "widgets/AntLog.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntNav.h"
#include "widgets/AntNavItem.h"
#include "widgets/AntPlainTextEdit.h"
#include "widgets/AntInputNumber.h"
#include "widgets/AntRibbon.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntSelect.h"
#include "widgets/AntStackedWidget.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntSwitch.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWidget.h"
#include "widgets/AntWindow.h"

namespace example::pages
{
namespace
{
QImage makeNavMediaImage(const QColor& color)
{
    QImage image(36, 36, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(3, 3, 30, 30), 7, 7);
    painter.setBrush(QColor(255, 255, 255, 215));
    painter.drawEllipse(QRectF(13, 13, 10, 10));
    return image;
}

class MasonryTile : public QWidget
{
public:
    MasonryTile(const QString& text, const QColor& color, QWidget* parent = nullptr)
        : QWidget(parent), m_text(text), m_color(color)
    {
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = antTheme->tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        QColor fill = m_color;
        fill.setAlphaF(antTheme->themeMode() == Ant::ThemeMode::Dark ? 0.28 : 0.16);
        painter.setPen(QPen(m_color, token.lineWidth));
        painter.setBrush(fill);
        painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5),
                                token.borderRadius, token.borderRadius);

        painter.setPen(token.colorText);
        painter.drawText(rect().adjusted(12, 10, -12, -10),
                         Qt::AlignLeft | Qt::AlignTop, m_text);
    }

private:
    QString m_text;
    QColor m_color;
};

class ThemeAwarePanel : public AntWidget
{
public:
    explicit ThemeAwarePanel(const QString& title, const QString& caption, QWidget* parent = nullptr)
        : AntWidget(parent), m_title(title), m_caption(caption)
    {
        setMinimumHeight(112);
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        const auto& token = tokens();
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

        const QRectF panel = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        painter.setPen(QPen(token.colorBorderSecondary, token.lineWidth));
        painter.setBrush(token.colorBgElevated);
        painter.drawRoundedRect(panel, token.borderRadiusLG, token.borderRadiusLG);

        painter.setPen(Qt::NoPen);
        painter.setBrush(token.colorPrimary);
        painter.drawRoundedRect(QRectF(16, 18, 5, height() - 36), 2, 2);

        painter.setPen(token.colorText);
        painter.drawText(QRect(32, 18, width() - 48, 24), Qt::AlignLeft | Qt::AlignVCenter, m_title);

        painter.setPen(token.colorTextSecondary);
        painter.drawText(QRect(32, 48, width() - 48, 24), Qt::AlignLeft | Qt::AlignVCenter, m_caption);

        const QString mode = currentTheme() == Ant::ThemeMode::Dark ? QStringLiteral("Dark") : QStringLiteral("Light");
        painter.setPen(token.colorTextTertiary);
        painter.drawText(QRect(32, 74, width() - 48, 22),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("%1 theme - %2 updates").arg(mode).arg(m_themeUpdates));
    }

    void onThemeChanged(Ant::ThemeMode mode) override
    {
        Q_UNUSED(mode)
        ++m_themeUpdates;
        update();
    }

private:
    QString m_title;
    QString m_caption;
    int m_themeUpdates = 0;
};
} // namespace

QWidget* createWidgetPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntWidget"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(QStringLiteral("AntWidget is the theme-aware base QWidget used by the example shell. "
                                                  "It exposes design tokens and receives global theme updates automatically."),
                                   page);
        cl->addWidget(desc);

        auto* row = new QHBoxLayout();
        row->setSpacing(12);
        row->addWidget(new ThemeAwarePanel(QStringLiteral("Token Surface"),
                                           QStringLiteral("Painted from AntThemeTokens"),
                                           page));
        row->addWidget(new ThemeAwarePanel(QStringLiteral("Theme Listener"),
                                           QStringLiteral("onThemeChanged refreshes this panel"),
                                           page));
        cl->addLayout(row);

        auto* toggle = new AntButton(QStringLiteral("Toggle Theme"));
        toggle->setButtonType(Ant::ButtonType::Primary);
        QObject::connect(toggle, &AntButton::clicked, antTheme, &AntTheme::toggleThemeMode);
        cl->addWidget(toggle, 0, Qt::AlignLeft);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createDialogPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntDialog"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(QStringLiteral("AntDialog is a frameless QDialog replacement with an Ant token title bar, theme-aware child palettes, Ant scroll bars, and a content widget for custom dialog bodies."),
                                   page);
        cl->addWidget(desc);

        auto* result = makeParagraph(QStringLiteral("Dialog result: not opened"), page);

        auto* openButton = new AntButton(QStringLiteral("Open Dialog"), page);
        openButton->setButtonType(Ant::ButtonType::Primary);
        QObject::connect(openButton, &AntButton::clicked, page, [page, result]() {
            AntDialog dialog(page);
            dialog.setWindowTitle(QStringLiteral("AntDialog"));

            auto* bodyLayout = new QVBoxLayout(dialog.contentWidget());
            bodyLayout->setContentsMargins(24, 20, 24, 20);
            bodyLayout->setSpacing(12);

            auto* title = new AntTypography(QStringLiteral("Theme-aware dialog surface"), dialog.contentWidget());
            title->setTitle(true);
            title->setTitleLevel(Ant::TypographyTitleLevel::H4);
            bodyLayout->addWidget(title);

            auto* text = new AntTypography(QStringLiteral("The custom title bar and body palette both react to AntTheme changes without stylesheets."), dialog.contentWidget());
            text->setParagraph(true);
            text->setWordWrap(true);
            bodyLayout->addWidget(text);

            auto* actionRow = new QHBoxLayout();
            actionRow->addStretch();
            auto* cancel = new AntButton(QStringLiteral("Cancel"), dialog.contentWidget());
            auto* ok = new AntButton(QStringLiteral("OK"), dialog.contentWidget());
            ok->setButtonType(Ant::ButtonType::Primary);
            QObject::connect(cancel, &AntButton::clicked, &dialog, &QDialog::reject);
            QObject::connect(ok, &AntButton::clicked, &dialog, &QDialog::accept);
            actionRow->addWidget(cancel);
            actionRow->addWidget(ok);
            bodyLayout->addLayout(actionRow);

            dialog.resize(460, 240);
            result->setText(dialog.exec() == QDialog::Accepted
                                ? QStringLiteral("Dialog result: accepted")
                                : QStringLiteral("Dialog result: rejected"));
        });

        cl->addWidget(openButton);
        cl->addWidget(result);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createInputDialogPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntInputDialog"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(QStringLiteral("AntInputDialog is the Ant Design replacement for QInputDialog. It reuses AntDialog chrome and provides text, integer, double, item selection, button text, options, and changed/selected signals without using the native dialog."),
                                   page);
        cl->addWidget(desc);

        auto* result = makeParagraph(QStringLiteral("Input result: not opened"), page);

        auto* actionRow = new QHBoxLayout();
        actionRow->setSpacing(8);

        auto* textButton = new AntButton(QStringLiteral("Text"), page);
        textButton->setButtonType(Ant::ButtonType::Primary);
        QObject::connect(textButton, &AntButton::clicked, page, [page, result]() {
            AntInputDialog dialog(page);
            dialog.setWindowTitle(QStringLiteral("Text input"));
            dialog.setLabelText(QStringLiteral("Project name"));
            dialog.setPlaceholderText(QStringLiteral("qt-ant-design"));
            dialog.setTextValue(QStringLiteral("qt-ant-design"));
            if (dialog.exec() == QDialog::Accepted)
            {
                result->setText(QStringLiteral("Text: %1").arg(dialog.textValue()));
            }
        });
        actionRow->addWidget(textButton);

        auto* intButton = new AntButton(QStringLiteral("Integer"), page);
        QObject::connect(intButton, &AntButton::clicked, page, [page, result]() {
            AntInputDialog dialog(page);
            dialog.setWindowTitle(QStringLiteral("Integer input"));
            dialog.setLabelText(QStringLiteral("Retry count"));
            dialog.setInputMode(AntInputDialog::IntInput);
            dialog.setIntRange(0, 10);
            dialog.setIntStep(1);
            dialog.setIntValue(3);
            if (dialog.exec() == QDialog::Accepted)
            {
                result->setText(QStringLiteral("Integer: %1").arg(dialog.intValue()));
            }
        });
        actionRow->addWidget(intButton);

        auto* doubleButton = new AntButton(QStringLiteral("Double"), page);
        QObject::connect(doubleButton, &AntButton::clicked, page, [page, result]() {
            AntInputDialog dialog(page);
            dialog.setWindowTitle(QStringLiteral("Double input"));
            dialog.setLabelText(QStringLiteral("Opacity"));
            dialog.setInputMode(AntInputDialog::DoubleInput);
            dialog.setDoubleRange(0.0, 1.0);
            dialog.setDoubleDecimals(2);
            dialog.setDoubleValue(0.72);
            if (dialog.exec() == QDialog::Accepted)
            {
                result->setText(QStringLiteral("Double: %1").arg(dialog.doubleValue(), 0, 'f', 2));
            }
        });
        actionRow->addWidget(doubleButton);

        auto* itemButton = new AntButton(QStringLiteral("Item"), page);
        QObject::connect(itemButton, &AntButton::clicked, page, [page, result]() {
            AntInputDialog dialog(page);
            dialog.setWindowTitle(QStringLiteral("Item input"));
            dialog.setLabelText(QStringLiteral("Theme mode"));
            dialog.setComboBoxEditable(false);
            dialog.setComboBoxItems({QStringLiteral("Default"), QStringLiteral("Dark"), QStringLiteral("Compact")});
            dialog.setTextValue(QStringLiteral("Dark"));
            if (dialog.exec() == QDialog::Accepted)
            {
                result->setText(QStringLiteral("Item: %1").arg(dialog.textValue()));
            }
        });
        actionRow->addWidget(itemButton);
        actionRow->addStretch();

        cl->addLayout(actionRow);
        cl->addWidget(result);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createStackedWidgetPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntStackedWidget"));
        auto* cl = card->bodyLayout();

        auto* stack = new AntStackedWidget(page);
        stack->setMinimumHeight(220);

        const QStringList titles = {
            QStringLiteral("Overview"),
            QStringLiteral("Details"),
            QStringLiteral("Activity"),
        };
        const QStringList captions = {
            QStringLiteral("A themed QStackedWidget replacement with Ant token background and border painting."),
            QStringLiteral("Pages keep native QStackedWidget ownership and current-index behavior."),
            QStringLiteral("Variant changes are handled by AntStackedWidgetStyle without stylesheets."),
        };
        for (int i = 0; i < titles.size(); ++i)
        {
            auto* panel = new AntWidget(stack);
            auto* panelLayout = new QVBoxLayout(panel);
            panelLayout->setContentsMargins(20, 18, 20, 18);
            panelLayout->setSpacing(8);

            auto* title = new AntTypography(titles.at(i), panel);
            title->setTitle(true);
            title->setTitleLevel(Ant::TypographyTitleLevel::H4);
            panelLayout->addWidget(title);

            auto* caption = new AntTypography(captions.at(i), panel);
            caption->setParagraph(true);
            caption->setWordWrap(true);
            panelLayout->addWidget(caption);
            panelLayout->addStretch();

            stack->addWidget(panel);
        }

        auto* actionRow = new QHBoxLayout();
        actionRow->setSpacing(8);
        for (int i = 0; i < titles.size(); ++i)
        {
            auto* button = new AntButton(titles.at(i), page);
            if (i == 0)
            {
                button->setButtonType(Ant::ButtonType::Primary);
            }
            QObject::connect(button, &AntButton::clicked, stack, [stack, i]() {
                stack->setCurrentIndex(i);
            });
            QObject::connect(stack, &QStackedWidget::currentChanged, button, [button, i](int current) {
                button->setButtonType(current == i ? Ant::ButtonType::Primary : Ant::ButtonType::Default);
            });
            actionRow->addWidget(button);
        }

        auto* variantSelect = new AntSelect(page);
        variantSelect->addOption(QStringLiteral("Outlined"), QStringLiteral("outlined"));
        variantSelect->addOption(QStringLiteral("Filled"), QStringLiteral("filled"));
        variantSelect->addOption(QStringLiteral("Borderless"), QStringLiteral("borderless"));
        variantSelect->setCurrentIndex(0);
        QObject::connect(variantSelect, &AntSelect::currentIndexChanged, stack, [stack](int index) {
            switch (index)
            {
            case 1:
                stack->setVariant(Ant::Variant::Filled);
                break;
            case 2:
                stack->setVariant(Ant::Variant::Borderless);
                break;
            default:
                stack->setVariant(Ant::Variant::Outlined);
                break;
            }
        });

        actionRow->addSpacing(8);
        actionRow->addWidget(variantSelect);
        actionRow->addStretch();

        cl->addLayout(actionRow);
        cl->addWidget(stack);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createNavItemPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntNavItem"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(QStringLiteral("AntNavItem is the self-painted sidebar item used by the example navigation. "
                                                  "Click the items below to switch the active state."),
                                   page);
        cl->addWidget(desc);

        auto* demoRow = new QHBoxLayout();
        demoRow->setSpacing(16);

        auto* navColumn = new AntWidget(page);
        navColumn->setFixedWidth(220);
        auto* navLayout = new QVBoxLayout(navColumn);
        navLayout->setContentsMargins(0, 8, 0, 8);
        navLayout->setSpacing(0);

        auto* overview = new AntNavItem(QStringLiteral("Overview"), navColumn);
        overview->setIcon(Ant::IconType::Home);
        auto* activity = new AntNavItem(QStringLiteral("Activity"), navColumn);
        activity->setIconName(QStringLiteral("GithubOutlined"));
        auto* settings = new AntNavItem(QStringLiteral("Settings"), navColumn);
        settings->setIconImage(makeNavMediaImage(QColor("#52c41a")));
        const QVector<AntNavItem*> items{overview, activity, settings};
        for (AntNavItem* item : items)
        {
            navLayout->addWidget(item);
        }
        navLayout->addStretch();

        auto* detail = makeParagraph(QStringLiteral("Overview selected"), page);
        overview->setActive(true);

        auto selectItem = [items, detail](int activeIndex) {
            for (int i = 0; i < items.size(); ++i)
            {
                items.at(i)->setActive(i == activeIndex);
            }
            detail->setText(QStringLiteral("%1 selected").arg(items.at(activeIndex)->text()));
        };
        QObject::connect(overview, &AntNavItem::clicked, overview, [selectItem]() { selectItem(0); });
        QObject::connect(activity, &AntNavItem::clicked, activity, [selectItem]() { selectItem(1); });
        QObject::connect(settings, &AntNavItem::clicked, settings, [selectItem]() { selectItem(2); });

        demoRow->addWidget(navColumn);
        demoRow->addWidget(detail, 1);
        cl->addLayout(demoRow);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createNavPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntNav"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(QStringLiteral("AntNav wraps AntNavItem into a reusable selection rail with grouped headers, current-index state, item data and Ant scroll bars."),
                                   page);
        cl->addWidget(desc);

        auto* demoRow = new QHBoxLayout();
        demoRow->setSpacing(16);

        auto* nav = new AntNav(page);
        nav->setFixedWidth(220);
        nav->addCategory(QStringLiteral("Workspace"));
        nav->addItem(QStringLiteral("Overview"), QStringLiteral("overview"));
        nav->setItemIcon(0, Ant::IconType::Home);
        nav->addItem(QStringLiteral("Activity"), QStringLiteral("activity"));
        nav->setItemIconName(1, QStringLiteral("GithubOutlined"));
        nav->addCategory(QStringLiteral("Manage"));
        nav->addItem(QStringLiteral("Members"), QStringLiteral("members"));
        nav->setItemIconImage(2, makeNavMediaImage(QColor("#1677ff")));
        nav->addItem(QStringLiteral("Settings"), QStringLiteral("settings"));
        nav->setItemIcon(3, Ant::IconType::Setting);

        auto* detail = makeParagraph(QStringLiteral("Overview selected"), page);
        QObject::connect(nav, &AntNav::currentIndexChanged, nav, [nav, detail](int) {
            detail->setText(QStringLiteral("%1 selected (%2)")
                                .arg(nav->currentText(), nav->currentData().toString()));
        });

        demoRow->addWidget(nav);
        demoRow->addWidget(detail, 1);
        cl->addLayout(demoRow);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createFileDialogPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntFileDialog"));
        auto* cl = card->bodyLayout();

        auto* desc = makeParagraph(QStringLiteral("AntFileDialog is a fully custom Ant Design file browser built from AntDialog, common places, a collapsed directory tree, QFileSystemModel/QTreeView file views, Ant inputs, Ant select, Ant buttons, token-painted panels, and a scoped QProxyStyle for file views."),
                                   page);
        cl->addWidget(desc);

        auto* selected = makeParagraph(QStringLiteral("No file selected"), page);

        auto* actionRow = new QHBoxLayout();
        actionRow->setSpacing(8);

        auto* openButton = new AntButton(QStringLiteral("Open File"), page);
        openButton->setButtonType(Ant::ButtonType::Primary);
        QObject::connect(openButton, &AntButton::clicked, page, [page, selected]() {
            AntFileDialog dialog(page,
                                 QStringLiteral("Open with AntFileDialog"),
                                 QDir::homePath(),
                                 QStringLiteral("All Files (*.*);;Images (*.png *.jpg *.jpeg);;Text Files (*.txt *.md)"));
            dialog.setAcceptMode(QFileDialog::AcceptOpen);
            dialog.setFileMode(QFileDialog::ExistingFile);
            if (dialog.exec() == QDialog::Accepted && !dialog.selectedFiles().isEmpty())
            {
                selected->setText(QStringLiteral("Selected: %1").arg(dialog.selectedFiles().constFirst()));
            }
        });
        actionRow->addWidget(openButton);

        auto* saveButton = new AntButton(QStringLiteral("Save As"), page);
        QObject::connect(saveButton, &AntButton::clicked, page, [page, selected]() {
            AntFileDialog dialog(page,
                                 QStringLiteral("Save with AntFileDialog"),
                                 QDir::homePath(),
                                 QStringLiteral("Text Files (*.txt);;All Files (*.*)"));
            dialog.setAcceptMode(QFileDialog::AcceptSave);
            dialog.setFileMode(QFileDialog::AnyFile);
            if (dialog.exec() == QDialog::Accepted && !dialog.selectedFiles().isEmpty())
            {
                selected->setText(QStringLiteral("Save target: %1").arg(dialog.selectedFiles().constFirst()));
            }
        });
        actionRow->addWidget(saveButton);
        actionRow->addStretch();

        cl->addLayout(actionRow);
        cl->addWidget(selected);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createDockWidgetPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntDockWidget"));
        auto* cl = card->bodyLayout();

        auto* infoLabel = makeParagraph(QStringLiteral("AntDockManager provides a themed docking workspace with a custom splitter/tab dock tree, draggable tabs, context menus, saved perspectives, translucent draggable AntDockWidget panels, and toggleable center/edge drop guide squares."), page);
        cl->addWidget(infoLabel);

        auto* manager = new AntDockManager(page);
        manager->setMinimumHeight(360);

        auto createDock = [](const QString& title, const QStringList& rows) {
            auto* dock = new AntDockWidget(title);
            auto* content = new QWidget();
            auto* dockLayout = new QVBoxLayout(content);
            dockLayout->setContentsMargins(12, 12, 12, 12);
            dockLayout->setSpacing(8);
            for (const QString& row : rows)
            {
                dockLayout->addWidget(makeText(row, content));
            }
            dockLayout->addStretch();
            dock->setWidget(content);
            return dock;
        };

        auto* filesDock = createDock(QStringLiteral("Explorer"),
                                     {QStringLiteral("src/widgets"),
                                      QStringLiteral("src/styles"),
                                      QStringLiteral("examples/pages")});
        auto* inspectorDock = createDock(QStringLiteral("Inspector"),
                                         {QStringLiteral("Selection: AntDockWidget"),
                                          QStringLiteral("Placement: Tabbed")});
        auto* outputDock = createDock(QStringLiteral("Output"),
                                      {QStringLiteral("Build ready"),
                                       QStringLiteral("Tests idle")});
        auto* previewDock = createDock(QStringLiteral("Preview"),
                                       {QStringLiteral("Dock panels can be split or tabified."),
                                        QStringLiteral("Drag a title bar to move a panel.")});

        manager->addDockWidget(Qt::LeftDockWidgetArea, filesDock);
        manager->splitDockWidget(filesDock, inspectorDock, Qt::Horizontal);
        manager->splitDockWidget(filesDock, outputDock, Qt::Vertical);
        manager->addDockWidget(previewDock, inspectorDock, AntDockManager::DockPlacement::Center);
        manager->savePerspective(QStringLiteral("Default"));

        auto* actionRow = new QHBoxLayout();
        auto* saveLayout = new AntButton(QStringLiteral("Save Layout"));
        saveLayout->setButtonType(Ant::ButtonType::Default);
        QObject::connect(saveLayout, &QPushButton::clicked, manager, [manager]() {
            manager->savePerspective(QStringLiteral("Workspace"));
        });
        actionRow->addWidget(saveLayout);

        auto* restoreLayout = new AntButton(QStringLiteral("Restore Layout"));
        restoreLayout->setButtonType(Ant::ButtonType::Default);
        QObject::connect(restoreLayout, &QPushButton::clicked, manager, [manager]() {
            if (!manager->restorePerspective(QStringLiteral("Workspace")))
            {
                manager->restorePerspective(QStringLiteral("Default"));
            }
        });
        actionRow->addWidget(restoreLayout);

        auto* guideToggle = new AntSwitch(page);
        guideToggle->setChecked(manager->isDropGuideEnabled());
        guideToggle->setCheckedText(QStringLiteral("Guides"));
        guideToggle->setUncheckedText(QStringLiteral("Guides"));
        QObject::connect(guideToggle, &AntSwitch::checkedChanged, manager, &AntDockManager::setDropGuideEnabled);
        QObject::connect(manager, &AntDockManager::dropGuideEnabledChanged, guideToggle, [guideToggle](bool enabled) {
            if (guideToggle->isChecked() != enabled)
            {
                guideToggle->setChecked(enabled);
            }
        });
        actionRow->addWidget(makeText(QStringLiteral("Drop guides"), page));
        actionRow->addWidget(guideToggle);
        actionRow->addStretch();
        cl->addLayout(actionRow);

        cl->addWidget(manager);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createLogPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntLog"));
        auto* cl = card->bodyLayout();

        auto* log = new AntLog(page);
        cl->addWidget(log);

        auto* btnRow = new QHBoxLayout();
        auto* addBtn = new AntButton(QStringLiteral("Add Sample Logs"));
        QObject::connect(addBtn, &QPushButton::clicked, log, [log]() {
            log->debug(QStringLiteral("This is a debug message"));
            log->info(QStringLiteral("Server started on port 8080"));
            log->success(QStringLiteral("Database connection established"));
            log->warning(QStringLiteral("Memory usage at 85%"));
            log->error(QStringLiteral("Failed to connect to upstream service"));
        });
        btnRow->addWidget(addBtn);

        auto* clearBtn = new AntButton(QStringLiteral("Clear"));
        clearBtn->setButtonType(Ant::ButtonType::Default);
        QObject::connect(clearBtn, &QPushButton::clicked, log, &AntLog::clear);
        btnRow->addWidget(clearBtn);
        btnRow->addStretch();
        cl->addLayout(btnRow);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createMasonryPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntMasonry"));
        auto* cl = card->bodyLayout();

        auto* masonry = new AntMasonry(page);
        masonry->setColumns(3);
        masonry->setSpacing(12);
        masonry->setMinimumHeight(420);

        const QList<int> heights = {90, 150, 120, 180, 110, 140};
        const QList<QString> colors = {
            QStringLiteral("#1677ff"), QStringLiteral("#13c2c2"), QStringLiteral("#52c41a"),
            QStringLiteral("#722ed1"), QStringLiteral("#eb2f96"), QStringLiteral("#fa8c16")
        };
        for (int i = 0; i < heights.size(); ++i)
        {
            auto* tile = new MasonryTile(QStringLiteral("Tile %1").arg(i + 1), QColor(colors[i]), masonry);
            tile->setMinimumHeight(heights[i]);
            masonry->addWidget(tile);
        }

        cl->addWidget(masonry);
        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createMenuBarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntMenuBar"));
        auto* cl = card->bodyLayout();

        auto* menuBar = new AntMenuBar(page);
        auto* fileMenu = menuBar->addMenu(QStringLiteral("File"));
        fileMenu->addAction(QStringLiteral("New"));
        fileMenu->addAction(QStringLiteral("Open"));
        fileMenu->addSeparator();
        fileMenu->addAction(QStringLiteral("Exit"));

        auto* editMenu = menuBar->addMenu(QStringLiteral("Edit"));
        editMenu->addAction(QStringLiteral("Undo"));
        editMenu->addAction(QStringLiteral("Redo"));
        editMenu->addSeparator();
        editMenu->addAction(QStringLiteral("Preferences"));

        menuBar->addMenu(QStringLiteral("View"));
        menuBar->addMenu(QStringLiteral("Help"));
        cl->addWidget(menuBar);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createPlainTextEditPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntPlainTextEdit"));
        auto* cl = card->bodyLayout();

        auto* te = new AntPlainTextEdit(page);
        te->setPlaceholderText(QStringLiteral("Type something..."));
        te->setMinimumHeight(150);
        cl->addWidget(te);

        auto* filled = new AntPlainTextEdit(page);
        filled->setVariant(Ant::Variant::Filled);
        filled->setPlaceholderText(QStringLiteral("Filled variant..."));
        filled->setMinimumHeight(100);
        cl->addWidget(filled);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createScrollAreaPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntScrollArea"));
        auto* cl = card->bodyLayout();

        auto* scroll = new AntScrollArea(page);
        scroll->setAutoHideScrollBar(false);
        scroll->setMinimumHeight(260);
        auto* content = new QWidget();
        auto* contentLayout = new QVBoxLayout(content);
        for (int i = 0; i < 30; ++i)
        {
            contentLayout->addWidget(makeText(QStringLiteral("Item %1 — Scroll to see the AntScrollBar").arg(i + 1), content));
        }
        contentLayout->addStretch();
        scroll->setWidget(content);
        cl->addWidget(scroll);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createScrollBarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntScrollBar"));
        auto* cl = card->bodyLayout();

        auto* desc = new AntTypography(QStringLiteral("AntScrollBar is a custom scrollbar with thin (8px) rounded handle, "
                                                      "auto-hide behavior, and transparent groove. No arrow buttons."));
        desc->setParagraph(true);
        cl->addWidget(desc);

        auto* scrollArea = new AntScrollArea(page);
        scrollArea->setAutoHideScrollBar(false);

        auto* scrollContent = new AntWidget();
        auto* scrollLayout = new QVBoxLayout(scrollContent);
        scrollLayout->setContentsMargins(8, 8, 8, 8);
        scrollLayout->setSpacing(8);
        for (int i = 0; i < 30; ++i)
        {
            auto* item = new AntTypography(QStringLiteral("Scroll item %1").arg(i + 1));
            item->setFixedHeight(32);
            scrollLayout->addWidget(item);
        }
        scrollArea->setWidget(scrollContent);
        cl->addWidget(scrollArea, 1);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createStatusBarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntStatusBar"));
        auto* cl = card->bodyLayout();

        auto* desc = new AntTypography(QStringLiteral("AntStatusBar displays status information at the bottom of a window. "
                                                      "Supports regular items, permanent items, message text, and size grip."));
        desc->setParagraph(true);
        cl->addWidget(desc);

        auto* statusBar = new AntStatusBar(page);
        statusBar->addItem(QStringLiteral("Ready"));
        statusBar->addItem(QStringLiteral("Line 1, Col 1"));
        statusBar->addPermanentItem(QStringLiteral("UTF-8"));
        statusBar->addPermanentItem(QStringLiteral("LF"));
        statusBar->setMessage(QStringLiteral("File saved successfully"));
        cl->addWidget(statusBar);

        auto* statusBar2 = new AntStatusBar(page);
        statusBar2->addItem(QStringLiteral("Connected"));
        statusBar2->addItem(QStringLiteral("3 warnings"));
        statusBar2->addPermanentItem(QStringLiteral("v1.0.0"));
        statusBar2->setSizeGrip(false);
        cl->addWidget(statusBar2);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createToolBarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntToolBar"));
        auto* cl = card->bodyLayout();

        auto* toolBar = new AntToolBar(QStringLiteral("Main Toolbar"), page);
        toolBar->addAction(QStringLiteral("New"));
        toolBar->addAction(QStringLiteral("Open"));
        toolBar->addAction(QStringLiteral("Save"));
        toolBar->addSeparator();
        toolBar->addAction(QStringLiteral("Undo"));
        toolBar->addAction(QStringLiteral("Redo"));
        cl->addWidget(toolBar);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

AntRibbon* createSampleRibbon(QWidget* parent)
{
    auto* ribbon = new AntRibbon(parent);

    auto* file = ribbon->addPage(QStringLiteral("File"), QStringLiteral("file"));
    auto* fileGroup = file->addGroup(QStringLiteral("File"));
    fileGroup->addLargeAction(new QAction(QStringLiteral("New"), ribbon));
    fileGroup->addLargeAction(new QAction(QStringLiteral("Open"), ribbon));
    fileGroup->addSmallAction(new QAction(QStringLiteral("Save"), ribbon));
    fileGroup->addSmallAction(new QAction(QStringLiteral("Export"), ribbon));
    fileGroup->addSmallAction(new QAction(QStringLiteral("Print"), ribbon));

    auto* edit = ribbon->addPage(QStringLiteral("Edit"), QStringLiteral("edit"));
    auto* clipboard = edit->addGroup(QStringLiteral("Clipboard"));
    clipboard->addLargeAction(new QAction(QStringLiteral("Paste"), ribbon));
    clipboard->addSmallAction(new QAction(QStringLiteral("Cut"), ribbon));
    clipboard->addSmallAction(new QAction(QStringLiteral("Copy"), ribbon));
    clipboard->addSmallAction(new QAction(QStringLiteral("Format"), ribbon));

    auto* controls = edit->addGroup(QStringLiteral("Ant Controls"));
    auto* modeSelect = new AntSelect();
    modeSelect->addOption(QStringLiteral("Normal"), QStringLiteral("normal"));
    modeSelect->addOption(QStringLiteral("Advanced"), QStringLiteral("advanced"));
    modeSelect->setCurrentIndex(0);
    controls->addWidget(modeSelect, Ant::RibbonItemSize::Small);

    auto* stepper = new AntInputNumber();
    stepper->setRange(0, 100);
    stepper->setValue(42);
    controls->addWidget(stepper, Ant::RibbonItemSize::Small);

    auto* applyButton = new AntButton(QStringLiteral("Apply"));
    applyButton->setButtonType(Ant::ButtonType::Primary);
    controls->addWidget(applyButton, Ant::RibbonItemSize::Large);

    auto* view = ribbon->addPage(QStringLiteral("View"), QStringLiteral("view"));
    auto* native = view->addGroup(QStringLiteral("Qt Widgets"));
    auto* combo = new QComboBox();
    combo->addItems({QStringLiteral("Compact"), QStringLiteral("Comfortable")});
    native->addWidget(combo, Ant::RibbonItemSize::Small);

    auto* search = new QLineEdit();
    search->setPlaceholderText(QStringLiteral("Search"));
    native->addWidget(search, Ant::RibbonItemSize::Small);

    auto* spin = new QSpinBox();
    spin->setRange(1, 12);
    spin->setValue(6);
    native->addWidget(spin, Ant::RibbonItemSize::Small);
    native->addSmallAction(new QAction(QStringLiteral("Refresh"), ribbon));
    native->addLargeAction(new QAction(QStringLiteral("Preview"), ribbon));

    return ribbon;
}

QWidget* createRibbonPage(QWidget* owner)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    auto* card = new AntCard(QStringLiteral("AntRibbon - Current Window"));
    auto* cl = card->bodyLayout();

    auto* desc = new AntTypography(QStringLiteral("Use this page to attach a Ribbon to the current AntWindow, "
                                                  "toggle its visibility, and inspect mixed Ant/Qt controls."));
    desc->setParagraph(true);
    cl->addWidget(desc);

    auto* targetWindow = qobject_cast<AntWindow*>(owner);
    auto* actionRow = new QHBoxLayout();
    actionRow->setSpacing(12);

    auto* addRibbon = new AntButton(QStringLiteral("Add Ribbon to Current Window"));
    addRibbon->setButtonType(Ant::ButtonType::Primary);
    auto* showRibbon = new AntButton(QStringLiteral("Show Ribbon"));
    auto* hideRibbon = new AntButton(QStringLiteral("Hide Ribbon"));
    auto* collapseRibbon = new AntButton(QStringLiteral("Collapse/Expand"));
    actionRow->addWidget(addRibbon);
    actionRow->addWidget(showRibbon);
    actionRow->addWidget(hideRibbon);
    actionRow->addWidget(collapseRibbon);
    actionRow->addStretch();
    cl->addLayout(actionRow);

    if (!targetWindow)
    {
        addRibbon->setEnabled(false);
        showRibbon->setEnabled(false);
        hideRibbon->setEnabled(false);
        collapseRibbon->setEnabled(false);
    }
    else
    {
        QObject::connect(addRibbon, &AntButton::clicked, targetWindow, [targetWindow]() {
            if (!targetWindow->ribbon())
            {
                targetWindow->setRibbon(createSampleRibbon(targetWindow));
            }
            targetWindow->setRibbonVisible(true);
        });
        QObject::connect(showRibbon, &AntButton::clicked, targetWindow, [targetWindow]() {
            if (!targetWindow->ribbon())
            {
                targetWindow->setRibbon(createSampleRibbon(targetWindow));
            }
            targetWindow->setRibbonVisible(true);
        });
        QObject::connect(hideRibbon, &AntButton::clicked, targetWindow, [targetWindow]() {
            targetWindow->setRibbonVisible(false);
        });
        QObject::connect(collapseRibbon, &AntButton::clicked, targetWindow, [targetWindow]() {
            if (!targetWindow->ribbon())
            {
                targetWindow->setRibbon(createSampleRibbon(targetWindow));
            }
            targetWindow->ribbon()->setCollapsed(!targetWindow->ribbon()->isCollapsed());
            targetWindow->setRibbonVisible(true);
        });
    }

    auto* previewCard = new AntCard(QStringLiteral("Standalone Preview"));
    previewCard->bodyLayout()->addWidget(createSampleRibbon(previewCard));

    layout->addWidget(card);
    layout->addWidget(previewCard);
    layout->addStretch();
    return page;
}

QWidget* createToolButtonPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntToolButton"));
        auto* cl = card->bodyLayout();

        auto* typeRow = new QHBoxLayout();
        typeRow->setSpacing(12);
        auto* btn1 = new AntToolButton(QStringLiteral("Default"));
        btn1->setButtonType(Ant::ButtonType::Default);
        typeRow->addWidget(btn1);
        auto* btn2 = new AntToolButton(QStringLiteral("Primary"));
        btn2->setButtonType(Ant::ButtonType::Primary);
        typeRow->addWidget(btn2);
        auto* btn3 = new AntToolButton(QStringLiteral("Text"));
        btn3->setButtonType(Ant::ButtonType::Text);
        typeRow->addWidget(btn3);
        typeRow->addStretch();
        cl->addLayout(typeRow);

        auto* sizeRow = new QHBoxLayout();
        sizeRow->setSpacing(12);
        auto* smallBtn = new AntToolButton(QStringLiteral("Small"));
        smallBtn->setButtonSize(Ant::Size::Small);
        sizeRow->addWidget(smallBtn);
        auto* midBtn = new AntToolButton(QStringLiteral("Middle"));
        midBtn->setButtonSize(Ant::Size::Middle);
        sizeRow->addWidget(midBtn);
        auto* largeBtn = new AntToolButton(QStringLiteral("Large"));
        largeBtn->setButtonSize(Ant::Size::Large);
        sizeRow->addWidget(largeBtn);
        sizeRow->addStretch();
        cl->addLayout(sizeRow);

        auto* stateRow = new QHBoxLayout();
        stateRow->setSpacing(12);
        auto* dangerBtn = new AntToolButton(QStringLiteral("Danger"));
        dangerBtn->setDanger(true);
        stateRow->addWidget(dangerBtn);
        auto* loadBtn = new AntToolButton(QStringLiteral("Loading"));
        loadBtn->setLoading(true);
        stateRow->addWidget(loadBtn);
        stateRow->addStretch();
        cl->addLayout(stateRow);

        auto* ddBtn = new AntToolButton(QStringLiteral("Actions"));
        ddBtn->setButtonType(Ant::ButtonType::Primary);
        auto* menu = new QMenu(ddBtn);
        menu->addAction(QStringLiteral("Edit"));
        menu->addAction(QStringLiteral("Delete"));
        menu->addSeparator();
        menu->addAction(QStringLiteral("Settings"));
        ddBtn->setMenu(menu);
        cl->addWidget(ddBtn);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}

QWidget* createWindowPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 24, 32, 24);
    layout->setSpacing(16);

    {
        auto* card = new AntCard(QStringLiteral("AntWindow - Frameless Window"));
        auto* cl = card->bodyLayout();

        auto* desc = new AntTypography(QStringLiteral("AntWindow is a frameless window with custom title bar, "
                                                      "draggable, minimize/maximize/close buttons with Ant Design styling. "
                                                      "Click the button below to open a demo window."));
        desc->setParagraph(true);
        cl->addWidget(desc);

        auto* openBtn = new AntButton(QStringLiteral("Open AntWindow"));
        openBtn->setButtonType(Ant::ButtonType::Primary);
        QObject::connect(openBtn, &AntButton::clicked, openBtn, []() {
            auto* window = new AntWindow();
            window->setWindowTitle(QStringLiteral("AntWindow Demo"));
            window->resize(600, 400);

            auto* central = new QWidget();
            auto* centralLayout = new QVBoxLayout(central);
            centralLayout->setContentsMargins(16, 16, 16, 16);
            auto* label = new AntTypography(QStringLiteral("This is an AntWindow with frameless design.\n"
                                                           "You can drag the title bar to move the window.\n"
                                                           "Double-click the title bar to maximize/restore."));
            label->setParagraph(true);
            centralLayout->addWidget(label);
            centralLayout->addStretch();
            window->setCentralWidget(central);

            window->setAttribute(Qt::WA_DeleteOnClose);
            window->show();
        });
        cl->addWidget(openBtn);

        layout->addWidget(card);
    }

    layout->addStretch();
    return page;
}
}
