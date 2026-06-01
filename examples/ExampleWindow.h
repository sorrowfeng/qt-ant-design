#pragma once

#include "widgets/AntWindow.h"

#include <QString>
#include <QStringList>

class AntNav;
class AntStackedWidget;
class QWidget;

class ExampleWindow : public AntWindow
{
    Q_OBJECT

public:
    explicit ExampleWindow(QWidget* parent = nullptr);
    int examplePageCount() const;
    QString examplePageName(int index) const;
    QWidget* examplePageWidget(int index) const;
    bool setExamplePageIndex(int index);

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
    QStringList m_pageNames;
};
