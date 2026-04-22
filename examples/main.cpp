#include <QApplication>

#include "ExampleWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("qt-ant-design-example"));

    ExampleWindow window;
    window.resize(1120, 760);
    window.show();

    return app.exec();
}
