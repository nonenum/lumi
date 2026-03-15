// Copyright (c) nonenum 2026.

#include "core/Lumi.h"

int main(int argc, char *argv[]) {

#if defined(Q_OS_LINUX)
    qputenv("QT_QPA_PLATFORM", "xcb");
#endif

    QApplication application(argc, argv);

    Lumi lumi;
    lumi.show();

    return application.exec();
}