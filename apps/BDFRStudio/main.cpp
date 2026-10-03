#include "StudioMainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFont>
#include <QTimer>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("BDFR");
    QCoreApplication::setApplicationName("BDFR Studio");
    QCoreApplication::setApplicationVersion("0.1.0");

    QFont font = app.font();
    font.setPointSize(10);
    app.setFont(font);

    const QStringList arguments = app.arguments();

    if (arguments.contains(QStringLiteral("--version"))) {
        return 0;
    }

    StudioMainWindow window;

    if (arguments.contains(QStringLiteral("--self-test"))) {
        QTimer::singleShot(250, &app, &QCoreApplication::quit);
        return app.exec();
    }

    window.show();
    return app.exec();
}
