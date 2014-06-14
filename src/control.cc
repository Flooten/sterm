#include "control.h"
#include "serialport_utils.h"

#include <QStringList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

Control::Control(QObject* parent)
    : QObject(parent)
    , timer_(new QTimer())
{
    sterm_settings_ = new XmlControl(STERM_SETTINGS_);

    port_settings_ = new XmlControl(PORT_SETTINGS_);

    port_ = new SerialPort(port_settings_->attributeValue("port-name", "value"),
                           port_settings_->attributeValue("baud-rate", "value"),
                           port_settings_->attributeValue("data-bits", "value"),
                           port_settings_->attributeValue("parity", "value"),
                           port_settings_->attributeValue("stop-bits", "value"));

    connect(timer_, SIGNAL(timeout()), this, SLOT(readData()));

    timer_->start(TIMER_VALUE);
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

    if (static_mode_)
        emit clear();

    if (command == "help")
    {
        if (arguments.isEmpty())
            emit out(sterm_settings_->text("help_general"));
        else
            emit out(sterm_settings_->text("help_" + arguments.at(0)));
    }
    else if (command == "transmit" || command == "tmit")
    {
        if (port_->isOpen())
        {
            if (arguments.at(0).startsWith("0x"))
            {
                // Tolka som hexadecimalt
                QByteArray message;

                for (int i = 0; i < arguments.count(); ++i)
                {
                    // Gå igenom alla angivna argument.
                    QString arg = arguments.at(i);
                    arg.remove("0x");

                    // Stöd för ej byteseparerad input.
                    if (arg.length() % 2 == 0)
                    {
                        // Jämnt antal tecken.
                        for (int i = 0; i < arg.length(); i += 2)
                        {
                            bool partial_ok;
                            QString partial_string = arg.mid(i, 2);

                            int partial_hex = partial_string.toInt(&partial_ok, 16);

                            if (partial_ok)
                            {
                                message.append(partial_hex);
                            }
                            else
                            {
                                emit out("Error: Not a valid hex input.");
                                return;
                            }
                        }
                    }
                    else
                    {
                        emit out("Not an even number of characters.");
                        return;
                    }
                }

                emit out("Transmitting...\nRaw data: " + utils::readableByteArray(message.toHex()));
                port_->transmit(message);
            }
            else
            {
                // Tolka som ASCII
                QString message = arguments.join(' ');

                emit out("Transmitting...\nRaw data: " + message);

                port_->transmit(message);
            }
        }
        else
        {
            emit out("Unable to transmit: The port is closed.");
        }
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
        emit out(QString("Port Status:\n") +
                 QString("------------"));
        emit out("Port name: \t\t" + port_->portName() +
                 "\nPort status:\t" + port_->state() +
                 "\nBaud rate: \t\t" + utils::toString(port_->baudRate()) +
                 "\nData bits: \t\t" + utils::toString(port_->dataBits()) +
                 "\nParity: \t\t" + utils::toString(port_->parity()) +
                 "\nStop bits: \t\t" + utils::toString(port_->stopBits()) + "\n");
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
            port_->setBaudRate(utils::toBaudRate(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting baud rate to " + value + ".");
        }
        else if (property == "data-bits")
        {
            port_->setDataBits(utils::toDataBits(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting number of data bits to " + value + ".");
        }
        else if (property == "parity")
        {
            port_->setParity(utils::toParity(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting parity to " + value + ".");
        }
        else if (property == "stop-bits")
        {
            port_->setStopBits(utils::toStopBits(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting number of stop bits to " + value + ".");
        }
    }
    else if (command == "lp")
    {
        QListIterator<QSerialPortInfo> i(QSerialPortInfo::availablePorts());

        emit out(QString("No.\tName:\tStatus:\n") +
                 QString("---\t-----\t-------"));

        char index = 1;

        while (i.hasNext())
        {
            QSerialPortInfo info = i.next();
            QString s;

            if (info.isBusy())
                s = QString("In Use");
            else
                s = QString("Available");

            emit out(QString::number(index) + QString(".\t") + info.portName() + "\t" + s);

            ++index;
        }

        emit out("");
    }
    else if (command == "autoclear")
    {
        if (static_mode_)
        {
            static_mode_ = false;
            emit out("Disabling autoclearing of the terminal.\n");
        }
        else
        {
            static_mode_ = true;
            emit out("Enabling autoclearing of the terminal.\n");
        }
    }
}

void Control::printWelcomeMessage()
{
    emit out(sterm_settings_->text("welcome_message") + '\n');
}

/* Skriver ut innehållet i data */
void Control::printData(const QByteArray& data)
{
    emit out("Received " + QString::number(data.length()) + " bytes." + "\n" +
             "ASCII:\t" + data + "\n" +
             "Hex:\t" + utils::readableByteArray(data.toHex()));
}

/*
 *  Private slots
 */

/* Läser data från portens buffer och skriver ut */
void Control::readData()
{
    if (port_->isOpen() && port_->bytesAvailable() != 0)
    {
        data_.append(port_->readAll());

        printData(data_);

        data_.clear();
    }
}
