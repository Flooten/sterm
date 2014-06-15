#ifndef CONTROL_H
#define CONTROL_H

#include "serialport.h"
#include "xmlcontrol.h"
#include "userinput.h"

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QTimer>
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
    void printWelcomeMessage();

private:
    SerialPort* port_;
    XmlControl* port_settings_;
    XmlControl* sterm_settings_;
    QByteArray data_;
    QTimer* timer_;
    QTimer* repeat_timer_;
    UserInput* repeated_input_;
    QMap<QString, QString>* call_response_map_;

    bool autoclear_ = false;

    const QString PORT_SETTINGS_ = "port_settings.xml";
    const QString STERM_SETTINGS_ = ":/data/resources/sterm.xml";
    const int TIMER_VALUE = 500;

    void printData(const QByteArray& data);


signals:
    void out(const QString& str);
    void clear();

private slots:
    void readData();
    void repeatInput();
};

#endif // CONTROL_H
