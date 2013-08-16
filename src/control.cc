#include "control.h"
#include "qextserialport_utils.h"

#include <QStringList>

Control::Control(QObject* parent)
    : QObject(parent)
{
    sterm_settings_ = new XmlControl(STERM_SETTINGS_);

    port_settings_ = new XmlControl(PORT_SETTINGS_);

    port_ = new SerialPort(port_settings_->attributeValue("port-name", "value"),
                           port_settings_->attributeValue("baud-rate", "value"),
                           port_settings_->attributeValue("data-bits", "value"),
                           port_settings_->attributeValue("parity", "value"),
                           port_settings_->attributeValue("stop-bits", "value"));
}

Control::~Control()
{
    delete port_;
    delete port_settings_;
}

void Control::parseInput(const UserInput& input)
{
    QString command = input.command();
    QStringList arguments = input.arguments();

    if (command == "help")
    {
        if (arguments.isEmpty())
            emit out(sterm_settings_->text("help_general"));
        else
            emit out(sterm_settings_->text("help_" + arguments.at(0)));
    }
    else if (command == "transmit" || command == "tmit")
    {

    }
    else if (command == "open")
    {
        if (port_->open())
            emit out("Port " + port_->portName() + " opened.");
        else
            emit out("Unable to open port " + port_->portName() + ".");
    }
    else if (command == "close")
    {
        port_->close();

        if (!port_->isOpen())
            emit out("Port " + port_->portName() + " closed.");
        else
            emit out("Unable to close port " + port_->portName() + ".");
    }
    else if (command == "status")
    {
        emit out("Port name: \t\t" + port_->portName() +
                 "\nPort status:\t" + port_->state() +
                 "\nBaud rate: \t\t" + utils::toString(port_->baudRate()) +
                 "\nData bits: \t\t" + utils::toString(port_->dataBits()) +
                 "\nParity: \t\t" + utils::toString(port_->parity()) +
                 "\nStop bits: \t\t" + utils::toString(port_->stopBits()));
    }
    else if (command == "set")
    {
        QString property = arguments.at(0);
        QString value = arguments.at(1);

        if (property == "port-name")
        {
            port_->setPortName(value);
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting port name to " + value + ".");
        }
        else if (property == "baud-rate")
        {
            port_->setBaudRate(utils::toBaudRateType(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting baud rate to " + value + ".");
        }
        else if (property == "data-bits")
        {
            port_->setDataBits(utils::toDataBitsType(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting number of data bits to " + value + ".");
        }
        else if (property == "parity")
        {
            port_->setParity(utils::toParityType(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting parity to " + value + ".");
        }
        else if (property == "stop-bits")
        {
            port_->setStopBits(utils::toStopBitsType(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting number of stop bits to " + value + ".");
        }
    }
}
