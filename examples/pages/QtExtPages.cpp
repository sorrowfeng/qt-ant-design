#include "Pages.h"

#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "PageCommon.h"
#include "core/AntTypes.h"
#include "widgets/AntButton.h"
#include "widgets/AntDockWidget.h"
#include "widgets/AntLog.h"
#include "widgets/AntMasonry.h"
#include "widgets/AntMenuBar.h"
#include "widgets/AntPlainTextEdit.h"
#include "widgets/AntScrollArea.h"
#include "widgets/AntScrollBar.h"
#include "widgets/AntStatusBar.h"
#include "widgets/AntToolBar.h"
#include "widgets/AntToolButton.h"
#include "widgets/AntTypography.h"
#include "widgets/AntWindow.h"

namespace example::pages
{
QWidget* createDockWidgetPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntDockWidget"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto* infoLabel = new QLabel(QStringLiteral("Dock widgets can be created from QMainWindow.\nThis page shows the AntDockWidget API."));
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    auto* dock = new AntDockWidget(QStringLiteral("My Dock"), page);
    auto* dockContent = new QWidget();
    auto* dockLayout = new QVBoxLayout(dockContent);
    dockLayout->addWidget(new QLabel(QStringLiteral("Dock content area")));
    dockLayout->addStretch();
    dock->setWidget(dockContent);
    dock->setMinimumWidth(200);

    layout->addWidget(dock);

    layout->addStretch();
    return page;
}

QWidget* createLogPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntLog"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

    auto* log = new AntLog(page);
    layout->addWidget(log);

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
    layout->addLayout(btnRow);

    return page;
}

QWidget* createMasonryPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntMasonry"));
    QFont f = title->font();
    f.setPixelSize(20);
    f.setBold(true);
    title->setFont(f);
    layout->addWidget(title);

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
        auto* card = new QLabel(QStringLiteral("Tile %1").arg(i + 1), masonry);
        card->setAlignment(Qt::AlignCenter);
        card->setMinimumHeight(heights[i]);
        card->setStyleSheet(QStringLiteral("background:%1; color:white; border-radius:10px;").arg(colors[i % colors.size()]));
        masonry->addWidget(card);
    }

    layout->addWidget(masonry);
    layout->addStretch();
    return page;
}

QWidget* createMenuBarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntMenuBar"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

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
    layout->addWidget(menuBar);

    layout->addStretch();
    return page;
}

QWidget* createPlainTextEditPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntPlainTextEdit"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto* te = new AntPlainTextEdit(page);
    te->setPlaceholderText(QStringLiteral("Type something..."));
    te->setMinimumHeight(150);
    layout->addWidget(te);

    auto* filled = new AntPlainTextEdit(page);
    filled->setVariant(Ant::Variant::Filled);
    filled->setPlaceholderText(QStringLiteral("Filled variant..."));
    filled->setMinimumHeight(100);
    layout->addWidget(filled);

    layout->addStretch();
    return page;
}

QWidget* createScrollAreaPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntScrollArea"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto* scroll = new AntScrollArea(page);
    auto* content = new QWidget();
    auto* contentLayout = new QVBoxLayout(content);
    for (int i = 0; i < 30; ++i)
    {
        contentLayout->addWidget(new QLabel(QStringLiteral("Item %1 — Scroll to see the AntScrollBar").arg(i + 1)));
    }
    contentLayout->addStretch();
    scroll->setWidget(content);
    layout->addWidget(scroll);

    return page;
}

QWidget* createScrollBarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("AntScrollBar")));

    auto* desc = new AntTypography(QStringLiteral("AntScrollBar is a custom scrollbar with thin (8px) rounded handle, "
                                                  "auto-hide behavior, and transparent groove. No arrow buttons."));
    desc->setParagraph(true);
    layout->addWidget(desc);

    auto* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBar(new AntScrollBar(Qt::Vertical));
    scrollArea->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal));

    auto* scrollContent = new QWidget();
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
    layout->addWidget(scrollArea, 1);

    return page;
}

QWidget* createStatusBarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("AntStatusBar")));

    auto* desc = new AntTypography(QStringLiteral("AntStatusBar displays status information at the bottom of a window. "
                                                  "Supports regular items, permanent items, message text, and size grip."));
    desc->setParagraph(true);
    layout->addWidget(desc);

    auto* statusBar = new AntStatusBar(page);
    statusBar->addItem(QStringLiteral("Ready"));
    statusBar->addItem(QStringLiteral("Line 1, Col 1"));
    statusBar->addPermanentItem(QStringLiteral("UTF-8"));
    statusBar->addPermanentItem(QStringLiteral("LF"));
    statusBar->setMessage(QStringLiteral("File saved successfully"));
    layout->addWidget(statusBar);

    auto* statusBar2 = new AntStatusBar(page);
    statusBar2->addItem(QStringLiteral("Connected"));
    statusBar2->addItem(QStringLiteral("3 warnings"));
    statusBar2->addPermanentItem(QStringLiteral("v1.0.0"));
    statusBar2->setSizeGrip(false);
    layout->addWidget(statusBar2);

    layout->addStretch();
    return page;
}

QWidget* createToolBarPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel(QStringLiteral("AntToolBar"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    auto* toolBar = new AntToolBar(QStringLiteral("Main Toolbar"), page);
    toolBar->addAction(QStringLiteral("New"));
    toolBar->addAction(QStringLiteral("Open"));
    toolBar->addAction(QStringLiteral("Save"));
    toolBar->addSeparator();
    toolBar->addAction(QStringLiteral("Undo"));
    toolBar->addAction(QStringLiteral("Redo"));
    layout->addWidget(toolBar);

    layout->addStretch();
    return page;
}

QWidget* createToolButtonPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setSpacing(16);

    auto* title = new QLabel(QStringLiteral("AntToolButton"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

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
    layout->addLayout(typeRow);

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
    layout->addLayout(sizeRow);

    auto* stateRow = new QHBoxLayout();
    stateRow->setSpacing(12);
    auto* dangerBtn = new AntToolButton(QStringLiteral("Danger"));
    dangerBtn->setDanger(true);
    stateRow->addWidget(dangerBtn);
    auto* loadBtn = new AntToolButton(QStringLiteral("Loading"));
    loadBtn->setLoading(true);
    stateRow->addWidget(loadBtn);
    stateRow->addStretch();
    layout->addLayout(stateRow);

    auto* ddBtn = new AntToolButton(QStringLiteral("Actions"));
    ddBtn->setButtonType(Ant::ButtonType::Primary);
    auto* menu = new QMenu(ddBtn);
    menu->addAction(QStringLiteral("Edit"));
    menu->addAction(QStringLiteral("Delete"));
    menu->addSeparator();
    menu->addAction(QStringLiteral("Settings"));
    ddBtn->setMenu(menu);
    layout->addWidget(ddBtn);

    layout->addStretch();
    return page;
}

QWidget* createWindowPage(QWidget* /*owner*/)
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 16, 24, 16);
    layout->setSpacing(16);

    layout->addWidget(createSectionTitle(QStringLiteral("AntWindow - Frameless Window")));

    auto* desc = new AntTypography(QStringLiteral("AntWindow is a frameless window with custom title bar, "
                                                  "draggable, minimize/maximize/close buttons with Ant Design styling. "
                                                  "Click the button below to open a demo window."));
    desc->setParagraph(true);
    layout->addWidget(desc);

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
    layout->addWidget(openBtn);

    layout->addStretch();
    return page;
}
}
