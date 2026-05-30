#pragma once

#include "widgets/AntWindow.h"

class AntNav;
class AntStackedWidget;
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

    QWidget* m_central = nullptr;
    QWidget* m_sidebar = nullptr;
    QWidget* m_content = nullptr;
    AntNav* m_nav = nullptr;
    AntStackedWidget* m_stack = nullptr;
};
