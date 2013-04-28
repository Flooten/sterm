#include "control.h"
#include "exception.h"
#include "qextserialport_utils.h"

#include <QStringList>

Control::Control(QObject* parent)
    : QObject(parent)
{
    XmlControl port_settings(PORT_SETTINGS_);
    port_ = new SerialPort(port_settings.attributeValue("port-name", "value"),
                           port_settings.attributeValue("baud-rate", "value"),
                           port_settings.attributeValue("data-bits", "value"),
                           port_settings.attributeValue("parity", "value"),
                           port_settings.attributeValue("stop-bits", "value"));
}

void Control::parseInput(const UserInput& input)
{
    QString command = input.command();
    QStringList arguments = input.arguments();

    if (command == "help")
    {

    }
    else if (command == "transmit" || command == "tmit")
    {

    }
    else if (command == "open")
    {

    }
    else if (command == "close")
    {

    }
    else if (command == "status")
    {

    }
    else if (command == "set")
    {
        QString property = arguments.at(0);
        QString value = arguments.at(1);

        if (property == "port-name")
        {
            port_->setPortName(value);
            emit out("Setting port name to " + value + ".");
        }
        else if (property == "baud-rate")
        {
            port_->setBaudRate(utils::toBaudRateType(value));
            emit out("Setting baud rate to " + value + ".");
        }
        else if (property == "data-bits")
        {
            port_->setDataBits(utils::toDataBitsType(value));
            emit out("Setting number of data bits to " + value + ".");
        }
        else if (property == "parity")
        {
            port_->setParity(utils::toParityType(value));
            emit out("Setting parity to " + value + ".");
        }
        else if (property == "stop-bits")
        {
            port_->setStopBits(utils::toStopBitsType(value));
            emit out("Setting number of stop bits to " + value + ".");
        }
    }
}
