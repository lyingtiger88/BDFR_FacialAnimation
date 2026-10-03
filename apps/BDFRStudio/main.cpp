#include "StudioMainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFont>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("BDFR");
    QCoreApplication::setApplicationName("BDFR Studio");
    QCoreApplication::setApplicationVersion("0.1.0");

    QFont font = app.font();
    font.setPointSize(10);
    app.setFont(font);

    StudioMainWindow window;
    window.show();

    return app.exec();
}
