#include "AntFileDialog.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QPainter>
#include <QPalette>
#include <QSet>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QTimer>
#include <QToolButton>
#include <QTreeView>
#include <QVector>
#include <QVBoxLayout>

#include "../styles/AntFileDialogStyle.h"
#include "AntButton.h"
#include "AntInput.h"
#include "AntScrollBar.h"
#include "AntSelect.h"
#include "AntToolTip.h"
#include "AntTypography.h"
#include "core/AntTheme.h"
#include "core/AntTypes.h"

namespace
{
class AntFileDialogPanel : public QWidget
{
public:
    enum class Role
    {
        Sidebar,
        Content,
        Footer
    };

    explicit AntFileDialogPanel(Role role, QWidget* parent = nullptr)
        : QWidget(parent),
          m_role(role)
    {
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)
        const auto& token = antTheme->tokens();

        QColor fill = token.colorBgContainer;
        if (m_role == Role::Sidebar)
        {
            fill = token.colorFillQuaternary;
        }
        else if (m_role == Role::Footer)
        {
            fill = token.colorBgElevated;
        }

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const QRect panelRect = rect().adjusted(0, 0, -1, -1);
        AntStyleBase::drawCrispRoundedRect(&painter, panelRect,
                                           QPen(token.colorBorderSecondary, token.lineWidth),
                                           fill, token.borderRadiusLG, token.borderRadiusLG);
    }

private:
    Role m_role;
};

QStringList splitNameFilterList(const QString& filter)
{
    if (filter.trimmed().isEmpty())
    {
        return {QStringLiteral("All Files (*)")};
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return filter.split(QStringLiteral(";;"), Qt::SkipEmptyParts);
#else
    return filter.split(QStringLiteral(";;"), QString::SkipEmptyParts);
#endif
}

QFileDialog::Options antDialogOptions(QFileDialog::Options options)
{
    return options | QFileDialog::DontUseNativeDialog;
}

void setAntScrollBars(QAbstractScrollArea* area)
{
    if (!area)
    {
        return;
    }
    if (!qobject_cast<AntScrollBar*>(area->verticalScrollBar()))
    {
        area->setVerticalScrollBar(new AntScrollBar(Qt::Vertical, area));
    }
    if (!qobject_cast<AntScrollBar*>(area->horizontalScrollBar()))
    {
        area->setHorizontalScrollBar(new AntScrollBar(Qt::Horizontal, area));
    }
}

AntButton* createIconButton(Ant::IconType icon, const QString& toolTip, QWidget* parent)
{
    auto* button = new AntButton(parent);
    button->setButtonIconType(icon);
    button->setToolTip(toolTip);
    button->setFixedWidth(36);
    button->setFocusPolicy(Qt::StrongFocus);
    return button;
}

struct CommonPlace
{
    QString label;
    QString path;
    Ant::IconType icon = Ant::IconType::Home;
};

QString existingDirectoryPath(const QString& path)
{
    if (path.trimmed().isEmpty())
    {
        return QString();
    }

    const QFileInfo info(path);
    if (!info.exists() || !info.isDir())
    {
        return QString();
    }
    return QDir(info.absoluteFilePath()).absolutePath();
}

void appendCommonPlace(QVector<CommonPlace>& places,
                       QSet<QString>& seen,
                       const QString& label,
                       const QString& path,
                       Ant::IconType icon)
{
    const QString absolutePath = existingDirectoryPath(path);
    if (absolutePath.isEmpty() || seen.contains(absolutePath))
    {
        return;
    }

    seen.insert(absolutePath);
    places.append({label, absolutePath, icon});
}

QVector<CommonPlace> commonPlaces()
{
    QVector<CommonPlace> places;
    QSet<QString> seen;

    appendCommonPlace(places, seen, QStringLiteral("Home"), QDir::homePath(), Ant::IconType::Home);
    appendCommonPlace(places,
                      seen,
                      QStringLiteral("Desktop"),
                      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                      Ant::IconType::Star);
    appendCommonPlace(places,
                      seen,
                      QStringLiteral("Documents"),
                      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                      Ant::IconType::Edit);
    appendCommonPlace(places,
                      seen,
                      QStringLiteral("Downloads"),
                      QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
                      Ant::IconType::CloudUpload);
    appendCommonPlace(places,
                      seen,
                      QStringLiteral("Pictures"),
                      QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                      Ant::IconType::Star);
    appendCommonPlace(places,
                      seen,
                      QStringLiteral("Music"),
                      QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
                      Ant::IconType::Bell);
    appendCommonPlace(places,
                      seen,
                      QStringLiteral("Videos"),
                      QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
                      Ant::IconType::Setting);
    return places;
}
} // namespace

AntFileDialog::AntFileDialog(QWidget* parent, const QString& caption,
                             const QString& directory, const QString& filter)
    : AntDialog(parent),
      m_hasExplicitCaption(!caption.isEmpty())
{
    setWindowTitle(caption);
    initializeAntStyle();
    buildUi();
    wireUi();
    setNameFilters(splitNameFilterList(filter));
    setCurrentDirectory(QDir(directory.isEmpty() ? QDir::homePath() : directory));
    updateHeaderText();
    updateAcceptButton();
    syncChildControls();

    connect(antTheme, &AntTheme::themeChanged, this, [this]() {
        syncChildControls();
        update();
    });
}

QString AntFileDialog::getOpenFileName(QWidget* parent, const QString& caption, const QString& dir,
                                       const QString& filter, QString* selectedFilter, Options options)
{
    AntFileDialog dialog(parent, caption, dir, filter);
    dialog.setOptions(antDialogOptions(options));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (selectedFilter && !selectedFilter->isEmpty())
    {
        dialog.selectNameFilter(*selectedFilter);
    }
    const QString result = dialog.exec() == QDialog::Accepted && !dialog.selectedFiles().isEmpty()
        ? dialog.selectedFiles().constFirst()
        : QString();
    if (selectedFilter)
    {
        *selectedFilter = dialog.selectedNameFilter();
    }
    return result;
}

QStringList AntFileDialog::getOpenFileNames(QWidget* parent, const QString& caption, const QString& dir,
                                            const QString& filter, QString* selectedFilter, Options options)
{
    AntFileDialog dialog(parent, caption, dir, filter);
    dialog.setOptions(antDialogOptions(options));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (selectedFilter && !selectedFilter->isEmpty())
    {
        dialog.selectNameFilter(*selectedFilter);
    }
    const QStringList result = dialog.exec() == QDialog::Accepted ? dialog.selectedFiles() : QStringList();
    if (selectedFilter)
    {
        *selectedFilter = dialog.selectedNameFilter();
    }
    return result;
}

QString AntFileDialog::getSaveFileName(QWidget* parent, const QString& caption, const QString& dir,
                                       const QString& filter, QString* selectedFilter, Options options)
{
    AntFileDialog dialog(parent, caption, dir, filter);
    dialog.setOptions(antDialogOptions(options));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    if (selectedFilter && !selectedFilter->isEmpty())
    {
        dialog.selectNameFilter(*selectedFilter);
    }
    const QString result = dialog.exec() == QDialog::Accepted && !dialog.selectedFiles().isEmpty()
        ? dialog.selectedFiles().constFirst()
        : QString();
    if (selectedFilter)
    {
        *selectedFilter = dialog.selectedNameFilter();
    }
    return result;
}

QString AntFileDialog::getExistingDirectory(QWidget* parent, const QString& caption,
                                            const QString& dir, Options options)
{
    AntFileDialog dialog(parent, caption, dir);
    dialog.setOptions(antDialogOptions(options | QFileDialog::ShowDirsOnly));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    const QStringList result = dialog.exec() == QDialog::Accepted ? dialog.selectedFiles() : QStringList();
    return result.isEmpty() ? QString() : result.constFirst();
}

void AntFileDialog::setAcceptMode(AcceptMode mode)
{
    if (m_acceptMode == mode)
    {
        return;
    }
    m_acceptMode = mode;
    updateHeaderText();
    updateAcceptButton();
}

AntFileDialog::AcceptMode AntFileDialog::acceptMode() const
{
    return m_acceptMode;
}

void AntFileDialog::setFileMode(FileMode mode)
{
    if (m_fileMode == mode)
    {
        return;
    }
    m_fileMode = mode;
    if (m_view)
    {
        m_view->setSelectionMode(m_fileMode == QFileDialog::ExistingFiles
                                     ? QAbstractItemView::ExtendedSelection
                                     : QAbstractItemView::SingleSelection);
    }
    updateModelFilters();
    updateHeaderText();
    updateAcceptButton();
}

AntFileDialog::FileMode AntFileDialog::fileMode() const
{
    return m_fileMode;
}

void AntFileDialog::setNameFilter(const QString& filter)
{
    setNameFilters(splitNameFilterList(filter));
}

void AntFileDialog::setNameFilters(const QStringList& filters)
{
    m_nameFilters = filters;
    m_nameFilters.removeAll(QString());
    if (m_nameFilters.isEmpty())
    {
        m_nameFilters.append(QStringLiteral("All Files (*)"));
    }
    if (!m_nameFilters.contains(m_selectedNameFilter))
    {
        m_selectedNameFilter = m_nameFilters.constFirst();
    }
    updateFilterOptions();
    updateModelFilters();
}

QStringList AntFileDialog::nameFilters() const
{
    return m_nameFilters;
}

void AntFileDialog::selectNameFilter(const QString& filter)
{
    if (filter.isEmpty())
    {
        return;
    }
    if (!m_nameFilters.contains(filter))
    {
        m_nameFilters.prepend(filter);
        updateFilterOptions();
    }
    m_selectedNameFilter = filter;
    updateFilterOptions();
    updateModelFilters();
}

QString AntFileDialog::selectedNameFilter() const
{
    return m_selectedNameFilter;
}

void AntFileDialog::setDirectory(const QString& directory)
{
    setCurrentDirectory(QDir(directory));
}

void AntFileDialog::setDirectory(const QDir& directory)
{
    setCurrentDirectory(directory);
}

QDir AntFileDialog::directory() const
{
    return m_currentDir;
}

void AntFileDialog::selectFile(const QString& filename)
{
    if (filename.isEmpty())
    {
        return;
    }

    const QString path = normalizeInputPath(filename);
    const QFileInfo info(path);
    if (m_fileNameInput)
    {
        m_fileNameInput->setText(info.isDir() ? info.absoluteFilePath() : info.fileName());
    }
    if (m_model && m_view)
    {
        const QModelIndex index = m_model->index(path);
        if (index.isValid())
        {
            m_view->setCurrentIndex(index);
            m_view->scrollTo(index, QAbstractItemView::PositionAtCenter);
        }
    }
    m_selectedFiles = QStringList{path};
    updateAcceptButton();
}

QStringList AntFileDialog::selectedFiles() const
{
    if (!m_selectedFiles.isEmpty())
    {
        return m_selectedFiles;
    }
    return selectedFilesFromView();
}

void AntFileDialog::setOption(Option option, bool on)
{
    if (option == QFileDialog::DontUseNativeDialog)
    {
        m_options |= QFileDialog::DontUseNativeDialog;
        return;
    }

    if (on)
    {
        m_options |= option;
    }
    else
    {
        m_options &= ~option;
    }
    m_options |= QFileDialog::DontUseNativeDialog;
    updateModelFilters();
    syncChildControls();
}

bool AntFileDialog::testOption(Option option) const
{
    if (option == QFileDialog::DontUseNativeDialog)
    {
        return true;
    }
    return m_options.testFlag(option);
}

void AntFileDialog::setOptions(Options options)
{
    m_options = options | QFileDialog::DontUseNativeDialog;
    updateModelFilters();
    syncChildControls();
}

AntFileDialog::Options AntFileDialog::options() const
{
    return m_options | QFileDialog::DontUseNativeDialog;
}

QString AntFileDialog::defaultSuffix() const
{
    return m_defaultSuffix;
}

void AntFileDialog::setDefaultSuffix(const QString& suffix)
{
    m_defaultSuffix = suffix;
    if (m_defaultSuffix.startsWith(QLatin1Char('.')))
    {
        m_defaultSuffix.remove(0, 1);
    }
}

void AntFileDialog::refreshAntStyle()
{
    AntDialog::refreshAntStyle();
    syncChildControls();
    update();
}

bool AntFileDialog::event(QEvent* event)
{
    const bool handled = AntDialog::event(event);
    switch (event->type())
    {
    case QEvent::ChildAdded:
    case QEvent::LayoutRequest:
    case QEvent::PaletteChange:
    case QEvent::Polish:
    case QEvent::StyleChange:
        scheduleChildSync();
        break;
    default:
        break;
    }
    return handled;
}

void AntFileDialog::showEvent(QShowEvent* event)
{
    AntDialog::showEvent(event);
    scheduleChildSync();
}

void AntFileDialog::initializeAntStyle()
{
    m_options |= QFileDialog::DontUseNativeDialog;
    installAntStyle<AntFileDialogStyle>(this);
    setAttribute(Qt::WA_Hover, true);
    setAutoFillBackground(false);
    setMinimumSize(760, 500);
    resize(880, 560);
    setSizeGripEnabled(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void AntFileDialog::buildUi()
{
    const auto& token = antTheme->tokens();

    auto* rootLayout = new QVBoxLayout(contentWidget());
    rootLayout->setContentsMargins(token.paddingLG, token.paddingLG, token.paddingLG, token.paddingLG);
    rootLayout->setSpacing(token.margin);

    auto* toolbarLayout = new QHBoxLayout;
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(token.marginXS);

    auto* backButton = createIconButton(Ant::IconType::Left, QStringLiteral("Back"), this);
    auto* upButton = createIconButton(Ant::IconType::Up, QStringLiteral("Up"), this);
    auto* homeButton = createIconButton(Ant::IconType::Home, QStringLiteral("Home"), this);
    auto* refreshButton = createIconButton(Ant::IconType::Search, QStringLiteral("Refresh"), this);
    toolbarLayout->addWidget(backButton);
    toolbarLayout->addWidget(upButton);
    toolbarLayout->addWidget(homeButton);
    toolbarLayout->addWidget(refreshButton);

    m_pathInput = new AntInput(this);
    m_pathInput->setPlaceholderText(QStringLiteral("Current folder"));
    m_pathInput->setAllowClear(false);
    toolbarLayout->addWidget(m_pathInput, 1);
    rootLayout->addLayout(toolbarLayout);

    auto* bodyLayout = new QHBoxLayout;
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(token.margin);

    m_sidebar = new AntFileDialogPanel(AntFileDialogPanel::Role::Sidebar, this);
    m_sidebar->setMinimumWidth(220);
    m_sidebar->setMaximumWidth(260);
    auto* sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setContentsMargins(token.paddingXS, token.paddingXS, token.paddingXS, token.paddingXS);
    sidebarLayout->setSpacing(token.paddingXXS);

    buildCommonPlaces(sidebarLayout);

    m_directoryModel = new QFileSystemModel(this);
    m_directoryModel->setReadOnly(true);
    m_directoryModel->setNameFilterDisables(false);
    m_directoryModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Drives);
    m_directoryModel->setRootPath(QString());

    m_directoryTree = new QTreeView(m_sidebar);
    m_directoryTree->setObjectName(QStringLiteral("antFileDialogDirectoryTree"));
    m_directoryTree->setModel(m_directoryModel);
    m_directoryTree->setFrameShape(QFrame::NoFrame);
    m_directoryTree->setHeaderHidden(true);
    m_directoryTree->setRootIsDecorated(true);
    m_directoryTree->setItemsExpandable(true);
    m_directoryTree->setUniformRowHeights(true);
    m_directoryTree->setAlternatingRowColors(false);
    m_directoryTree->setMouseTracking(true);
    m_directoryTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_directoryTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_directoryTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_directoryTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_directoryTree->setMinimumHeight(180);
    m_directoryTree->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    for (int column = 1; column < m_directoryModel->columnCount(); ++column)
    {
        m_directoryTree->setColumnHidden(column, true);
    }
    sidebarLayout->addWidget(m_directoryTree, 1);
    bodyLayout->addWidget(m_sidebar);

    auto* contentPanel = new AntFileDialogPanel(AntFileDialogPanel::Role::Content, this);
    auto* contentLayout = new QVBoxLayout(contentPanel);
    contentLayout->setContentsMargins(token.paddingSM, token.paddingSM, token.paddingSM, token.paddingSM);
    contentLayout->setSpacing(0);

    m_model = new QFileSystemModel(this);
    m_model->setReadOnly(true);
    m_model->setNameFilterDisables(false);

    m_view = new QTreeView(contentPanel);
    m_view->setObjectName(QStringLiteral("antFileDialogFileView"));
    m_view->setModel(m_model);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setRootIsDecorated(false);
    m_view->setItemsExpandable(false);
    m_view->setUniformRowHeights(true);
    m_view->setAlternatingRowColors(false);
    m_view->setMouseTracking(true);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_view->setSortingEnabled(true);
    m_view->sortByColumn(0, Qt::AscendingOrder);
    if (m_view->header())
    {
        m_view->header()->setHighlightSections(false);
        m_view->header()->setStretchLastSection(true);
    }
    contentLayout->addWidget(m_view);
    bodyLayout->addWidget(contentPanel, 1);
    rootLayout->addLayout(bodyLayout, 1);

    auto* footerPanel = new AntFileDialogPanel(AntFileDialogPanel::Role::Footer, this);
    auto* footerLayout = new QHBoxLayout(footerPanel);
    footerLayout->setContentsMargins(token.paddingSM, token.paddingSM, token.paddingSM, token.paddingSM);
    footerLayout->setSpacing(token.marginSM);

    auto* fileLabel = new AntTypography(QStringLiteral("File"), footerPanel);
    fileLabel->setType(Ant::TypographyType::Secondary);
    footerLayout->addWidget(fileLabel);

    m_fileNameInput = new AntInput(footerPanel);
    m_fileNameInput->setPlaceholderText(QStringLiteral("File name"));
    m_fileNameInput->setAllowClear(true);
    footerLayout->addWidget(m_fileNameInput, 1);

    m_filterSelect = new AntSelect(footerPanel);
    m_filterSelect->setMinimumWidth(190);
    m_filterSelect->setMaxVisibleItems(6);
    footerLayout->addWidget(m_filterSelect);

    m_cancelButton = new AntButton(QStringLiteral("Cancel"), footerPanel);
    footerLayout->addWidget(m_cancelButton);

    m_acceptButton = new AntButton(footerPanel);
    m_acceptButton->setButtonType(Ant::ButtonType::Primary);
    m_acceptButton->setDefault(true);
    footerLayout->addWidget(m_acceptButton);
    rootLayout->addWidget(footerPanel);

    connect(backButton, &QAbstractButton::clicked, this, [this]() {
        if (m_directoryHistoryIndex > 0)
        {
            --m_directoryHistoryIndex;
            setCurrentDirectory(QDir(m_directoryHistory.at(m_directoryHistoryIndex)), false);
        }
    });
    connect(upButton, &QAbstractButton::clicked, this, [this]() {
        QDir parentDir = m_currentDir;
        parentDir.cdUp();
        setCurrentDirectory(parentDir);
    });
    connect(homeButton, &QAbstractButton::clicked, this, [this]() {
        setCurrentDirectory(QDir(QDir::homePath()));
    });
    connect(refreshButton, &QAbstractButton::clicked, this, [this]() {
        if (m_model)
        {
            m_model->setRootPath(m_currentDir.absolutePath());
        }
        updateModelFilters();
    });
}

void AntFileDialog::wireUi()
{
    connect(m_pathInput, &AntInput::returnPressed, this, [this]() {
        setCurrentDirectory(QDir(QDir::fromNativeSeparators(m_pathInput->text())));
    });
    connect(m_fileNameInput, &AntInput::returnPressed, this, &AntFileDialog::acceptSelection);
    connect(m_fileNameInput, &AntInput::textChanged, this, [this]() {
        m_selectedFiles.clear();
        updateAcceptButton();
    });
    connect(m_filterSelect, &AntSelect::currentIndexChanged, this, [this](int index) {
        if (index >= 0 && index < m_nameFilters.size())
        {
            m_selectedNameFilter = m_nameFilters.at(index);
            updateModelFilters();
        }
    });
    connect(m_cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(m_acceptButton, &QAbstractButton::clicked, this, &AntFileDialog::acceptSelection);
    connect(m_view, &QTreeView::doubleClicked, this, &AntFileDialog::navigateToIndex);
    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this]() { updateSelectionFromView(); });
    connect(m_directoryTree->selectionModel(), &QItemSelectionModel::currentChanged,
            this, [this](const QModelIndex& current) { navigateToDirectoryTreeIndex(current); });
    connect(m_directoryTree, &QTreeView::activated,
            this, &AntFileDialog::navigateToDirectoryTreeIndex);
}

void AntFileDialog::updateHeaderText()
{
    const QString title = m_hasExplicitCaption && !windowTitle().isEmpty()
        ? windowTitle()
        : defaultTitleForMode();
    if (!m_hasExplicitCaption)
    {
        setWindowTitle(title);
    }
}

void AntFileDialog::updateAcceptButton()
{
    if (!m_acceptButton)
    {
        return;
    }

    m_acceptButton->setText(acceptTextForMode());

    bool canAccept = false;
    const QString textPath = normalizeInputPath(m_fileNameInput ? m_fileNameInput->text() : QString());
    const QFileInfo textInfo(textPath);
    const QStringList selected = selectedFilesFromView();

    if (m_fileMode == QFileDialog::AnyFile)
    {
        canAccept = !textPath.isEmpty() || !selected.isEmpty();
    }
    else if (acceptsDirectoriesOnly())
    {
        canAccept = textInfo.isDir() || !selected.isEmpty();
    }
    else if (m_fileMode == QFileDialog::ExistingFiles)
    {
        canAccept = !selected.isEmpty();
    }
    else
    {
        canAccept = textInfo.isFile() || !selected.isEmpty();
    }
    m_acceptButton->setEnabled(canAccept);
}

void AntFileDialog::updateModelFilters()
{
    if (!m_model)
    {
        return;
    }

    QDir::Filters filters = QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Drives;
    if (!acceptsDirectoriesOnly())
    {
        filters |= QDir::Files;
    }
    m_model->setFilter(filters);

    if (acceptsDirectoriesOnly())
    {
        m_model->setNameFilters(QStringList());
    }
    else
    {
        m_model->setNameFilters(patternsForFilter(m_selectedNameFilter));
    }
}

void AntFileDialog::updateFilterOptions()
{
    if (!m_filterSelect)
    {
        return;
    }

    m_filterSelect->clearOptions();
    int selectedIndex = 0;
    for (int i = 0; i < m_nameFilters.size(); ++i)
    {
        const QString filter = m_nameFilters.at(i);
        m_filterSelect->addOption(filter, filter);
        if (filter == m_selectedNameFilter)
        {
            selectedIndex = i;
        }
    }
    m_filterSelect->setEnabled(!acceptsDirectoriesOnly() && m_nameFilters.size() > 1);
    m_filterSelect->setCurrentIndex(selectedIndex);
}

void AntFileDialog::updateSelectionFromView()
{
    const QStringList files = selectedFilesFromView();
    m_selectedFiles = files;
    if (m_fileNameInput)
    {
        QSignalBlocker blocker(m_fileNameInput);
        if (files.size() == 1)
        {
            const QFileInfo info(files.constFirst());
            m_fileNameInput->setText(info.isDir() ? info.absoluteFilePath() : info.fileName());
        }
        else if (files.size() > 1)
        {
            QStringList names;
            for (const QString& file : files)
            {
                names.append(QFileInfo(file).fileName());
            }
            m_fileNameInput->setText(names.join(QStringLiteral("; ")));
        }
    }
    updateAcceptButton();
}

void AntFileDialog::acceptSelection()
{
    QStringList files;
    const QString text = m_fileNameInput ? m_fileNameInput->text().trimmed() : QString();

    if (m_fileMode == QFileDialog::ExistingFiles)
    {
        files = selectedFilesFromView();
    }
    else if (!text.isEmpty())
    {
        QString path = normalizeInputPath(text);
        if (m_fileMode == QFileDialog::AnyFile)
        {
            path = appendDefaultSuffixIfNeeded(path);
            files = QStringList{QFileInfo(path).absoluteFilePath()};
        }
        else
        {
            const QFileInfo info(path);
            if (acceptsDirectoriesOnly() && info.isDir())
            {
                files = QStringList{info.absoluteFilePath()};
            }
            else if (!acceptsDirectoriesOnly() && info.isFile())
            {
                files = QStringList{info.absoluteFilePath()};
            }
        }
    }

    if (files.isEmpty())
    {
        files = selectedFilesFromView();
    }
    if (files.isEmpty())
    {
        updateAcceptButton();
        return;
    }

    m_selectedFiles = files;
    QDialog::accept();
}

void AntFileDialog::setCurrentDirectory(const QDir& directory, bool addToHistory)
{
    QString path = directory.path().isEmpty() ? QDir::homePath() : directory.path();
    QFileInfo info(path);
    if (info.isFile())
    {
        path = info.absolutePath();
    }
    if (!QDir(path).exists())
    {
        path = QDir::homePath();
    }

    m_currentDir = QDir(QDir(path).absolutePath());
    const QString absolutePath = m_currentDir.absolutePath();
    if (addToHistory)
    {
        const QString currentHistory = m_directoryHistoryIndex >= 0 &&
                                       m_directoryHistoryIndex < m_directoryHistory.size()
            ? m_directoryHistory.at(m_directoryHistoryIndex)
            : QString();
        if (currentHistory != absolutePath)
        {
            while (m_directoryHistory.size() > m_directoryHistoryIndex + 1)
            {
                m_directoryHistory.removeLast();
            }
            m_directoryHistory.append(absolutePath);
            m_directoryHistoryIndex = m_directoryHistory.size() - 1;
        }
    }

    if (m_pathInput)
    {
        m_pathInput->setText(QDir::toNativeSeparators(absolutePath));
    }
    if (m_model && m_view)
    {
        const QModelIndex rootIndex = m_model->setRootPath(absolutePath);
        m_view->setRootIndex(rootIndex.isValid() ? rootIndex : m_model->index(absolutePath));
        m_view->setColumnWidth(0, 280);
        m_view->clearSelection();
    }
    syncDirectoryTreeToCurrent();
    if (m_fileNameInput)
    {
        m_fileNameInput->clear();
    }
    m_selectedFiles.clear();
    updateAcceptButton();
}

void AntFileDialog::navigateToIndex(const QModelIndex& index)
{
    if (!index.isValid() || !m_model)
    {
        return;
    }

    const QFileInfo info(m_model->filePath(index));
    if (info.isDir())
    {
        setCurrentDirectory(QDir(info.absoluteFilePath()));
        return;
    }

    if (!acceptsDirectoriesOnly())
    {
        selectFile(info.absoluteFilePath());
        acceptSelection();
    }
}

void AntFileDialog::navigateToDirectoryTreeIndex(const QModelIndex& index)
{
    if (!index.isValid() || !m_directoryModel)
    {
        return;
    }

    const QModelIndex directoryIndex = index.sibling(index.row(), 0);
    const QFileInfo info(m_directoryModel->filePath(directoryIndex));
    if (!info.isDir())
    {
        return;
    }

    const QDir directory(info.absoluteFilePath());
    const QString absolutePath = directory.absolutePath();
    if (QDir(m_currentDir.absolutePath()).absolutePath() == absolutePath)
    {
        return;
    }
    setCurrentDirectory(directory);
}

void AntFileDialog::syncDirectoryTreeToCurrent()
{
    if (!m_directoryModel || !m_directoryTree)
    {
        return;
    }

    const QModelIndex index = m_directoryModel->index(m_currentDir.absolutePath());
    if (!index.isValid())
    {
        return;
    }

    if (QItemSelectionModel* selection = m_directoryTree->selectionModel())
    {
        QSignalBlocker blocker(selection);
        m_directoryTree->setCurrentIndex(index);
    }
    else
    {
        m_directoryTree->setCurrentIndex(index);
    }
    m_directoryTree->setProperty("antFileDialogDirectoryTreeAutoExpandsCurrent", false);
}

QStringList AntFileDialog::selectedFilesFromView() const
{
    QStringList files;
    if (!m_view || !m_model || !m_view->selectionModel())
    {
        return files;
    }

    QModelIndexList rows = m_view->selectionModel()->selectedRows(0);
    if (rows.isEmpty() && m_view->currentIndex().isValid())
    {
        rows.append(m_view->currentIndex().sibling(m_view->currentIndex().row(), 0));
    }

    QSet<QString> seen;
    for (const QModelIndex& index : rows)
    {
        if (!index.isValid())
        {
            continue;
        }
        const QFileInfo info(m_model->filePath(index));
        if ((acceptsDirectoriesOnly() && !info.isDir()) ||
            (!acceptsDirectoriesOnly() && info.isDir()))
        {
            continue;
        }
        const QString path = info.absoluteFilePath();
        if (!seen.contains(path))
        {
            seen.insert(path);
            files.append(path);
        }
    }
    return files;
}

QString AntFileDialog::normalizeInputPath(const QString& text) const
{
    QString path = QDir::fromNativeSeparators(text.trimmed());
    if (path.isEmpty())
    {
        return QString();
    }
    if ((path.startsWith(QLatin1Char('"')) && path.endsWith(QLatin1Char('"'))) ||
        (path.startsWith(QLatin1Char('\'')) && path.endsWith(QLatin1Char('\''))))
    {
        path = path.mid(1, path.size() - 2);
    }
    const QFileInfo info(path);
    if (info.isAbsolute())
    {
        return info.absoluteFilePath();
    }
    return QFileInfo(m_currentDir, path).absoluteFilePath();
}

QString AntFileDialog::appendDefaultSuffixIfNeeded(const QString& path) const
{
    QFileInfo info(path);
    if (!info.suffix().isEmpty())
    {
        return path;
    }

    QString suffix = m_defaultSuffix;
    if (suffix.isEmpty())
    {
        const QStringList patterns = patternsForFilter(m_selectedNameFilter);
        for (const QString& pattern : patterns)
        {
            if (pattern.startsWith(QStringLiteral("*.")) && pattern.indexOf(QLatin1Char('*'), 1) < 0)
            {
                suffix = pattern.mid(2);
                break;
            }
        }
    }
    if (suffix.isEmpty() || suffix.contains(QLatin1Char('*')) || suffix.contains(QLatin1Char('?')))
    {
        return path;
    }
    return path + QLatin1Char('.') + suffix;
}

QStringList AntFileDialog::patternsForFilter(const QString& filter) const
{
    QString patternText = filter.trimmed();
    const int left = patternText.indexOf(QLatin1Char('('));
    const int right = patternText.lastIndexOf(QLatin1Char(')'));
    if (left >= 0 && right > left)
    {
        patternText = patternText.mid(left + 1, right - left - 1);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QStringList patterns = patternText.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#else
    QStringList patterns = patternText.split(QLatin1Char(' '), QString::SkipEmptyParts);
#endif
    if (patterns.isEmpty())
    {
        patterns.append(QStringLiteral("*"));
    }
    if (patterns.contains(QStringLiteral("*.*")) && !patterns.contains(QStringLiteral("*")))
    {
        patterns.append(QStringLiteral("*"));
    }
    patterns.removeDuplicates();
    return patterns;
}

QString AntFileDialog::defaultTitleForMode() const
{
    if (acceptsDirectoriesOnly())
    {
        return QStringLiteral("Select Folder");
    }
    return m_acceptMode == QFileDialog::AcceptSave
        ? QStringLiteral("Save File")
        : QStringLiteral("Open File");
}

QString AntFileDialog::acceptTextForMode() const
{
    if (acceptsDirectoriesOnly())
    {
        return QStringLiteral("Select");
    }
    return m_acceptMode == QFileDialog::AcceptSave
        ? QStringLiteral("Save")
        : QStringLiteral("Open");
}

bool AntFileDialog::acceptsDirectoriesOnly() const
{
    return m_fileMode == QFileDialog::Directory || m_options.testFlag(QFileDialog::ShowDirsOnly);
}

void AntFileDialog::buildCommonPlaces(QVBoxLayout* sidebarLayout)
{
    if (!sidebarLayout || !m_sidebar)
    {
        return;
    }

    const auto& token = antTheme->tokens();

    auto* placesTitle = new AntTypography(QStringLiteral("Quick access"), m_sidebar);
    placesTitle->setObjectName(QStringLiteral("antFileDialogPlacesTitle"));
    placesTitle->setType(Ant::TypographyType::Secondary);
    placesTitle->setPixelSize(token.fontSizeSM);
    sidebarLayout->addWidget(placesTitle);

    const QVector<CommonPlace> places = commonPlaces();
    const int quickAccessColumns = qMax(1, places.size());
    const int quickAccessButtonSide = qMax(token.controlHeightSM, 24);
    const int quickAccessSpacing = qMax(1, token.paddingXXS / 2);
    auto* placesPanel = new QWidget(m_sidebar);
    placesPanel->setObjectName(QStringLiteral("antFileDialogPlacesPanel"));
    placesPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto* placesLayout = new QGridLayout(placesPanel);
    placesLayout->setContentsMargins(0, 0, 0, 0);
    placesLayout->setHorizontalSpacing(quickAccessSpacing);
    placesLayout->setVerticalSpacing(quickAccessSpacing);

    int placeIndex = 0;
    for (const CommonPlace& place : places)
    {
        auto* button = new AntButton(placesPanel);
        button->setObjectName(QStringLiteral("antFileDialogPlaceButton"));
        button->setProperty("antFileDialogPlacePath", place.path);
        button->setProperty("antFileDialogPlaceLabel", place.label);
        button->setButtonType(Ant::ButtonType::Text);
        button->setButtonSize(Ant::Size::Small);
        button->setButtonIconType(place.icon);
        button->setAccessibleName(place.label);
        button->setFixedSize(quickAccessButtonSide, quickAccessButtonSide);
        const QString toolTipText = QStringLiteral("%1\n%2").arg(place.label, QDir::toNativeSeparators(place.path));
        button->setProperty("antFileDialogPlaceToolTipText", toolTipText);
        auto* toolTip = new AntToolTip(placesPanel);
        toolTip->setObjectName(QStringLiteral("antFileDialogPlaceTooltip"));
        toolTip->setTitle(toolTipText);
        toolTip->setPlacement(Ant::TooltipPlacement::Top);
        toolTip->setTarget(button);
        const int row = placeIndex / quickAccessColumns;
        const int column = placeIndex % quickAccessColumns;
        placesLayout->addWidget(button, row, column, Qt::AlignLeft);
        connect(button, &QAbstractButton::clicked, this, [this, path = place.path]() {
            navigateToCommonPlace(path);
        });
        ++placeIndex;
    }
    const int quickAccessRows = qMax(1, (placeIndex + quickAccessColumns - 1) / quickAccessColumns);
    placesPanel->setMaximumHeight(quickAccessRows * quickAccessButtonSide
                                  + qMax(0, quickAccessRows - 1) * quickAccessSpacing);
    sidebarLayout->addWidget(placesPanel);

    auto* treeTitle = new AntTypography(QStringLiteral("Folders"), m_sidebar);
    treeTitle->setObjectName(QStringLiteral("antFileDialogDirectoryTreeTitle"));
    treeTitle->setType(Ant::TypographyType::Secondary);
    treeTitle->setPixelSize(token.fontSizeSM);
    sidebarLayout->addSpacing(token.paddingXXS);
    sidebarLayout->addWidget(treeTitle);
}

void AntFileDialog::navigateToCommonPlace(const QString& path)
{
    const QString absolutePath = existingDirectoryPath(path);
    if (absolutePath.isEmpty())
    {
        return;
    }
    setCurrentDirectory(QDir(absolutePath));
}

void AntFileDialog::scheduleChildSync()
{
    if (m_childSyncQueued)
    {
        return;
    }
    m_childSyncQueued = true;
    QTimer::singleShot(0, this, [this]() {
        m_childSyncQueued = false;
        syncChildControls();
    });
}

void AntFileDialog::syncChildControls()
{
    applyDialogPalette(this);

    const auto scrollAreas = findChildren<QAbstractScrollArea*>();
    for (QAbstractScrollArea* area : scrollAreas)
    {
        area->setFrameShape(QFrame::NoFrame);
        applyDialogPalette(area);
        if (area->viewport())
        {
            applyDialogPalette(area->viewport());
            area->viewport()->setAutoFillBackground(true);
        }
        setAntScrollBars(area);
    }

    const auto itemViews = findChildren<QAbstractItemView*>();
    for (QAbstractItemView* view : itemViews)
    {
        view->setAlternatingRowColors(false);
        view->setMouseTracking(true);
        applyDialogPalette(view);
        if (view->viewport())
        {
            applyDialogPalette(view->viewport());
        }
    }

    const auto headers = findChildren<QHeaderView*>();
    for (QHeaderView* header : headers)
    {
        if (header->style() != style())
        {
            header->setStyle(style());
        }
        header->setHighlightSections(false);
        applyDialogPalette(header);
    }

    const auto lineEdits = findChildren<QLineEdit*>();
    for (QLineEdit* lineEdit : lineEdits)
    {
        lineEdit->setFrame(false);
        lineEdit->setAttribute(Qt::WA_TranslucentBackground, true);
        lineEdit->setAutoFillBackground(false);
        applyDialogPalette(lineEdit);
    }

    const auto toolButtons = findChildren<QToolButton*>();
    for (QToolButton* button : toolButtons)
    {
        button->setAutoRaise(true);
        button->setMouseTracking(true);
        applyDialogPalette(button);
    }

    const auto buttons = findChildren<QAbstractButton*>();
    for (QAbstractButton* button : buttons)
    {
        button->setMouseTracking(true);
        applyDialogPalette(button);
    }

    ++m_childSyncCount;
    setProperty("antFileDialogChildSyncCount", m_childSyncCount);
    setProperty("antFileDialogUsesNativeDialog", false);
}

void AntFileDialog::applyDialogPalette(QWidget* widget)
{
    if (!widget)
    {
        return;
    }

    const auto& token = antTheme->tokens();
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Window, token.colorBgContainer);
    pal.setColor(QPalette::Base, token.colorBgContainer);
    pal.setColor(QPalette::AlternateBase, token.colorFillQuaternary);
    pal.setColor(QPalette::Button, token.colorBgContainer);
    pal.setColor(QPalette::Text, token.colorText);
    pal.setColor(QPalette::WindowText, token.colorText);
    pal.setColor(QPalette::ButtonText, token.colorText);
    pal.setColor(QPalette::PlaceholderText, token.colorTextPlaceholder);
    pal.setColor(QPalette::Highlight, token.colorPrimaryBg);
    pal.setColor(QPalette::HighlightedText, token.colorText);
    pal.setColor(QPalette::Disabled, QPalette::Text, token.colorTextDisabled);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, token.colorTextDisabled);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, token.colorTextDisabled);
    pal.setColor(QPalette::Disabled, QPalette::Base, token.colorBgContainerDisabled);
    pal.setColor(QPalette::Disabled, QPalette::Button, token.colorBgContainerDisabled);
    widget->setPalette(pal);
}
