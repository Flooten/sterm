#ifndef CONTROL_H
#define CONTROL_H

#include "serialport.h"
#include "xmlcontrol.h"
#include "userinput.h"

#include <QObject>
#include <QString>
#include <stdexcept>

class ControlException : public std::logic_error
{
public:
    ControlException(const QString& str) : std::logic_error(str.toStdString()) { }
};

class Control : public QObject
{
    Q_OBJECT

public:
    Control(QObject* parent = nullptr);
    ~Control();

    void parseInput(const UserInput& input);

private:
    SerialPort* port_;
    XmlControl* port_settings_;
    XmlControl* sterm_settings_;

    const QString PORT_SETTINGS_ = "port_settings.xml";
    const QString STERM_SETTINGS_ = ":/data/resources/sterm.xml";

signals:
    void out(const QString& str);
};

#endif // CONTROL_H
