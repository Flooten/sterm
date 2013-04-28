#include "terminal.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Terminal w;

    //QApplication::setStyle("fusion");

    w.show();
    
    return a.exec();
}
