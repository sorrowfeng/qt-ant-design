#pragma once

#include <QMainWindow>

class AntButton;
class QHBoxLayout;
class QLabel;
class QMouseEvent;
class QStackedWidget;
class QVBoxLayout;

class ExampleWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ExampleWindow(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QWidget* createAlertPage();
    QWidget* createButtonPage();
    QWidget* createAvatarPage();
    QWidget* createBadgePage();
    QWidget* createBreadcrumbPage();
    QWidget* createCheckboxPage();
    QWidget* createDatePickerPage();
    QWidget* createDropdownPage();
    QWidget* createDividerPage();
    QWidget* createFormPage();
    QWidget* createIconPage();
    QWidget* createInputPage();
    QWidget* createInputNumberPage();
    QWidget* createMessagePage();
    QWidget* createMenuPage();
    QWidget* createModalPage();
    QWidget* createNotificationPage();
    QWidget* createPopoverPage();
    QWidget* createPopconfirmPage();
    QWidget* createPaginationPage();
    QWidget* createProgressPage();
    QWidget* createRadioPage();
    QWidget* createSelectPage();
    QWidget* createSkeletonPage();
    QWidget* createSliderPage();
    QWidget* createSpinPage();
    QWidget* createSwitchPage();
    QWidget* createTabsPage();
    QWidget* createTagPage();
    QWidget* createTooltipPage();
    QWidget* createTimePickerPage();
    QWidget* createCardPage();
    QWidget* wrapPage(QWidget* page);
    QLabel* createSectionTitle(const QString& title);
    void addNavButton(const QString& text, int pageIndex);
    void applyTheme();

    QWidget* m_central = nullptr;
    QWidget* m_sidebar = nullptr;
    QWidget* m_titleBar = nullptr;
    QWidget* m_content = nullptr;
    QVBoxLayout* m_navLayout = nullptr;
    QStackedWidget* m_stack = nullptr;
    AntButton* m_themeButton = nullptr;
    bool m_dragging = false;
    QPoint m_dragOffset;
};
