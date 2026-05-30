#pragma once

#include "core/QtAntDesignExport.h"

#include <QDir>
#include <QFileDialog>
#include <QStringList>

#include "AntDialog.h"

class AntButton;
class AntInput;
class AntSelect;
class QFileSystemModel;
class QModelIndex;
class QAbstractButton;
class QAbstractItemView;
class QAbstractScrollArea;
class QHeaderView;
class QLineEdit;
class QTreeView;
class QWidget;

class QT_ANT_DESIGN_EXPORT AntFileDialog : public AntDialog
{
    Q_OBJECT

public:
    using AcceptMode = QFileDialog::AcceptMode;
    using FileMode = QFileDialog::FileMode;
    using Option = QFileDialog::Option;
    using Options = QFileDialog::Options;

    explicit AntFileDialog(QWidget* parent = nullptr,
                           const QString& caption = QString(),
                           const QString& directory = QString(),
                           const QString& filter = QString());

    static QString getOpenFileName(QWidget* parent = nullptr,
                                   const QString& caption = QString(),
                                   const QString& dir = QString(),
                                   const QString& filter = QString(),
                                   QString* selectedFilter = nullptr,
                                   Options options = Options());
    static QStringList getOpenFileNames(QWidget* parent = nullptr,
                                        const QString& caption = QString(),
                                        const QString& dir = QString(),
                                        const QString& filter = QString(),
                                        QString* selectedFilter = nullptr,
                                        Options options = Options());
    static QString getSaveFileName(QWidget* parent = nullptr,
                                   const QString& caption = QString(),
                                   const QString& dir = QString(),
                                   const QString& filter = QString(),
                                   QString* selectedFilter = nullptr,
                                   Options options = Options());
    static QString getExistingDirectory(QWidget* parent = nullptr,
                                        const QString& caption = QString(),
                                        const QString& dir = QString(),
                                        Options options = Options());

    void setAcceptMode(AcceptMode mode);
    AcceptMode acceptMode() const;

    void setFileMode(FileMode mode);
    FileMode fileMode() const;

    void setNameFilter(const QString& filter);
    void setNameFilters(const QStringList& filters);
    QStringList nameFilters() const;
    void selectNameFilter(const QString& filter);
    QString selectedNameFilter() const;

    void setDirectory(const QString& directory);
    void setDirectory(const QDir& directory);
    QDir directory() const;

    void selectFile(const QString& filename);
    QStringList selectedFiles() const;

    void setOption(Option option, bool on = true);
    bool testOption(Option option) const;
    void setOptions(Options options);
    Options options() const;

    QString defaultSuffix() const;
    void setDefaultSuffix(const QString& suffix);

    void refreshAntStyle() override;

protected:
    bool event(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void initializeAntStyle();
    void buildUi();
    void wireUi();
    void updateHeaderText();
    void updateAcceptButton();
    void updateModelFilters();
    void updateFilterOptions();
    void updateSelectionFromView();
    void acceptSelection();
    void setCurrentDirectory(const QDir& directory, bool addToHistory = true);
    void navigateToIndex(const QModelIndex& index);
    QStringList selectedFilesFromView() const;
    QString normalizeInputPath(const QString& text) const;
    QString appendDefaultSuffixIfNeeded(const QString& path) const;
    QStringList patternsForFilter(const QString& filter) const;
    QString defaultTitleForMode() const;
    QString acceptTextForMode() const;
    bool acceptsDirectoriesOnly() const;
    void scheduleChildSync();
    void syncChildControls();
    void applyDialogPalette(QWidget* widget);

    QFileSystemModel* m_model = nullptr;
    QTreeView* m_view = nullptr;
    AntInput* m_pathInput = nullptr;
    AntInput* m_fileNameInput = nullptr;
    AntSelect* m_filterSelect = nullptr;
    AntButton* m_acceptButton = nullptr;
    AntButton* m_cancelButton = nullptr;
    QWidget* m_sidebar = nullptr;

    AcceptMode m_acceptMode = QFileDialog::AcceptOpen;
    FileMode m_fileMode = QFileDialog::ExistingFile;
    Options m_options = QFileDialog::DontUseNativeDialog;
    QStringList m_nameFilters;
    QString m_selectedNameFilter;
    QStringList m_selectedFiles;
    QDir m_currentDir;
    QStringList m_directoryHistory;
    QString m_defaultSuffix;
    int m_directoryHistoryIndex = -1;
    bool m_hasExplicitCaption = false;
    bool m_childSyncQueued = false;
    int m_childSyncCount = 0;
};
