#include "copya.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Copya w;
    w.show();
    return a.exec();
}
