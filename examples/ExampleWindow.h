#pragma once

#include "widgets/AntWindow.h"

class AntButton;
class QStackedWidget;
class QVBoxLayout;
class QWidget;

class ExampleWindow : public AntWindow
{
    Q_OBJECT

public:
    explicit ExampleWindow(QWidget* parent = nullptr);

protected:
    QSize sizeHint() const override;

private:
    void buildSidebar();
    void buildPages();
    void addCategoryHeader(const QString& title);
    void addNavButton(const QString& text, int pageIndex);
    void applyTheme();

    QWidget* m_central = nullptr;
    QWidget* m_sidebar = nullptr;
    QWidget* m_content = nullptr;
    QVBoxLayout* m_navLayout = nullptr;
    QStackedWidget* m_stack = nullptr;
    AntButton* m_themeButton = nullptr;
};
