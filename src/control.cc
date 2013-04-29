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
            out(sterm_settings_->text("help_general"));
        else
            out(sterm_settings_->text("help_" + arguments.at(0)));
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
