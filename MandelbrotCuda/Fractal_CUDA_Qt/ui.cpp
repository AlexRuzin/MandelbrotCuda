#pragma once

#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include "ui.h"
#include "types.h"

using namespace ui;

error_t window::init_window(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QString::fromStdString(windowResourceName)));
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}