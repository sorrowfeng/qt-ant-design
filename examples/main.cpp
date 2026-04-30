#include <QApplication>

#include "ExampleWindow.h"
#include "core/AntFont.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("qt-ant-design-example"));
    AntFont::applyToApplication(&app);

    ExampleWindow window;
    window.setMinimumSize(960, 640);
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
