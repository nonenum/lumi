// Copyright (c) nonenum 2026.

#include <QApplication>
#include "core/Lumi.h"

int main(int argc, char *argv[]) {

// =================================================
// Telling Wayland to fuck off, in case of Linux
#if defined(Q_OS_LINUX)
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif
// =================================================

    QApplication application(argc, argv);

    Lumi lumi;
    lumi.show();

    return application.exec();
}