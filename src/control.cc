#include "control.h"
#include "serialport_utils.h"

#include <QStringList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

Control::Control(QObject* parent)
    : QObject(parent)
    , timer_(new QTimer())
    , repeat_timer_(new QTimer())
    , repeated_input_(new UserInput(""))
{
    sterm_settings_ = new XmlControl(STERM_SETTINGS_);

    port_settings_ = new XmlControl(PORT_SETTINGS_);

    port_ = new SerialPort(port_settings_->attributeValue("port-name", "value"),
                           port_settings_->attributeValue("baud-rate", "value"),
                           port_settings_->attributeValue("data-bits", "value"),
                           port_settings_->attributeValue("parity", "value"),
                           port_settings_->attributeValue("stop-bits", "value"));

    connect(timer_, SIGNAL(timeout()), this, SLOT(readData()));
    connect(repeat_timer_, SIGNAL(timeout()), this, SLOT(repeatInput()));

    timer_->start(TIMER_VALUE);
}

Control::~Control()
{
    delete port_settings_;
    delete timer_;
    delete repeat_timer_;
    delete repeated_input_;
    delete port_;
}

void Control::parseInput(const UserInput& input)
{
    QString command = input.command();
    QStringList arguments = input.arguments();

    if (autoclear_)
        emit clear();

    if (command == "help")
    {
        if (arguments.isEmpty())
            emit out(sterm_settings_->text("help_general") + '\n');
        else
            emit out(sterm_settings_->text("help_" + arguments.at(0)) + '\n');
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
                                throw ControlException("Error: Not a valid hex input.");
                            }
                        }
                    }
                    else
                    {
                        throw ControlException("Not an even number of characters.");
                    }
                }

                emit out("Transmitting...\nRaw data: " + utils::readableByteArray(message.toHex()) + "\n");
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
            emit out("Port " + port_->portName() + " opened.\n");
        else
            emit out("Unable to open port " + port_->portName() + ".\n");
    }
    else if (command == "close")
    {
        if (!port_->isOpen())
            throw ControlException("Port " + port_->portName() + " is already closed.\n");

        port_->close();

        if (!port_->isOpen())
            emit out("Port " + port_->portName() + " closed.\n");
        else
            emit out("Unable to close port " + port_->portName() + ".\n");
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
        if (port_->isOpen())
            throw ControlException("Cannot set port properties while the port is open.\n");

        QString property = arguments.at(0);
        QString value = arguments.at(1);

        if (property == "port-name")
        {
            // Verify that the port exists
            QListIterator<QSerialPortInfo> i(QSerialPortInfo::availablePorts());

            bool exists = false;

            while (i.hasNext())
                if (value == i.next().portName())
                    exists = true;

            if (!exists)
                throw ControlException("The port '" + value + "' does not exist. Use 'lp' to list available serial ports.\n");

            port_->setPortName(value);
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting port name to " + value + ".\n");
        }
        else if (property == "baud-rate")
        {
            port_->setBaudRate(utils::toBaudRate(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting baud rate to " + value + ".\n");
        }
        else if (property == "data-bits")
        {
            port_->setDataBits(utils::toDataBits(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting number of data bits to " + value + ".\n");
        }
        else if (property == "parity")
        {
            port_->setParity(utils::toParity(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting parity to " + value + ".\n");
        }
        else if (property == "stop-bits")
        {
            port_->setStopBits(utils::toStopBits(value));
            port_settings_->setAttributeValue(property, "value", value);

            emit out("Setting number of stop bits to " + value + ".\n");
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

        // No ports found
        if (index == 1)
            emit out("No ports available.");

        emit out("");
    }
    else if (command == "autoclear")
    {
        if (autoclear_)
        {
            autoclear_ = false;
            emit out("Disabling autoclearing of the terminal.\n");
        }
        else
        {
            autoclear_ = true;
            emit out("Enabling autoclearing of the terminal.\n");
        }
    }
    else if (command == "repeat")
    {
        // Repeat the command that is input as argument 1
        // Argument 0 describes the repetition frequency in Hertz

        QString primary_arg = arguments.at(0);

        if (primary_arg == "stop")
        {
            repeat_timer_->stop();
            emit out("Input repetition halted.\n");
            return;
        }
        else
        {
            if (repeat_timer_->isActive())
                throw ControlException("Unable to begin repetition. The command '" + repeated_input_->command() + "' is currently being repeated.\n");

            bool ok;
            int value = primary_arg.toInt(&ok, 10);

            if (!ok)
                throw ControlException("Not a valid repetition frequency.\n");

            arguments.removeFirst();

            repeated_input_ = new UserInput(arguments.join(" "));

            if (!repeated_input_->isValid())
                throw ControlException("Invalid or incomplete command '" + repeated_input_->command() + "'.\n");

            repeat_timer_->start(1000 / value);

            emit out("Repeating '" + arguments.join(" ") + "' with frequency " + QString::number(value) + " Hz.\n");
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

/* Repeats the input in repeated_input_ as long as repeat_timer_ is running */
void Control::repeatInput()
{
    try { parseInput(*repeated_input_); }
    catch (std::logic_error& e) { emit out(e.what()); }
}
