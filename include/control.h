#ifndef CONTROL_H
#define CONTROL_H

#include "serialport.h"
#include <QString>

class Control
{
public:
    Control();

private:
    SerialPort* port_;

signals:
    void out(const QString& str);
};

#endif // CONTROL_H
