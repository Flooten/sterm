#include "control.h"
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
            BaudRateType type = utils::toBaudRateType(value);

            if (type == BAUDINVALID)
                throw ControlException("Error: Invalid baud rate '" + value + "'.");

            port_->setBaudRate(type);
            emit out("Setting baud rate to " + value + ".");
        }
        else if (property == "data-bits")
        {
            DataBitsType type = utils::toDataBitsType(value);

            if (type == DATAINVALID)
                throw ControlException("Error: Invalid number of data bits '" + value + "'.");

            port_->setDataBits(type);
            emit out("Setting number of data bits to " + value + ".");
        }
        else if (property == "parity")
        {
            ParityType type = utils::toParityType(value);

            if (type == PARINVALID)
                throw ControlException("Error: Invalid parity '" + value + "'.");

            port_->setParity(type);
            emit out("Setting parity to " + value + ".");
        }
        else if (property == "stop-bits")
        {
            StopBitsType type = utils::toStopBitsType(value);

            if (type == STOPINVALID)
                throw ControlException("Error: Invalid number of stop bits '" + value + "'.");

            port_->setStopBits(type);
            emit out("Setting number of stop bits to " + value + ".");
        }
    }
    else if (command == "clear")
    {

    }
}
