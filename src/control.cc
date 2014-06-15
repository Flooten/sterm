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
    , call_response_map_(new QMap<QString, QString>)
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
    delete call_response_map_;
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
                                throw ControlException("Error: Not a valid hex input.\n");
                            }
                        }
                    }
                    else
                    {
                        throw ControlException("Not an even number of characters.\n");
                    }
                }

                emit out("Transmitting...\nRaw data: " + utils::readableByteArray(message.toHex()) + "\n");
                port_->transmit(message);
            }
            else
            {
                // Tolka som ASCII
                QString message = arguments.join(' ');

                emit out("Transmitting...\nRaw data: " + message + "\n");

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
        // Argument 0 describes the repetition frequency in Hertz (or stop)

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
    else if (command == "respond")
    {
        // Look for the message in argument 0.
        // If found, respond with argument 1.
        QString call = arguments.at(0);
        QString response = arguments.at(1);

        if (call_response_map_->contains(call))
            throw ControlException("A response for that call is already defined.");

        call_response_map_->insert(call, response);

        emit out("Response added. Responding to " + call + " with " + response + ".\n");
    }
    else if (command == "lr")
    {
        // List calls and responses
        emit out(QString("No.\tCall:\tResponse:\n") +
                 QString("---\t-----\t---------"));

        if (call_response_map_->isEmpty())
            throw ControlException("No responses defined.\n");


        QMapIterator<QString, QString> i(*call_response_map_);

        int index = 1;

        while (i.hasNext())
        {
            i.next();
            emit out(QString::number(index) + ".\t" + i.key() + "\t" + i.value());
            ++index;
        }

        emit out("");
    }
    else if (command == "remove")
    {
        QString property = arguments.at(0);

        if (property == "response")
        {
            bool ok;
            int target_index = arguments.at(1).toInt(&ok, 10);

            if (!ok)
                throw ControlException("Error converting the index to an integer.\n");

            QMapIterator<QString, QString> i(*call_response_map_);

            int index = 1;
            bool found = false;

            while (i.hasNext())
            {
                i.next();

                if (index == target_index)
                {
                    emit out("Removed response " + QString::number(target_index) + ". '" + i.key() + "' -> '" + i.value() + "'\n");
                    call_response_map_->remove(i.key());
                    found = true;
                    break;
                }
                else
                {
                    ++index;
                }
            }

            if (!found)
                throw ControlException("Index out of range.");
        }
    }
}


void Control::printWelcomeMessage()
{
    emit out(sterm_settings_->text("welcome_message") + '\n');
}

/* Prints the contents of data */
void Control::printData(const QByteArray& data)
{
    emit out("Received " + QString::number(data.length()) + " bytes." + "\n" +
             "ASCII:\t" + data + "\n" +
             "Hex:\t" + utils::readableByteArray(data.toHex()));
}

/* Function to analyze the received data */
void Control::parseData(const QByteArray& data)
{
    // Call and response
    QMapIterator<QString, QString> i(*call_response_map_);

    while (i.hasNext())
    {
        i.next();

        QString call = i.key();

        if (call.startsWith("0x"))
        {
            // Interpret call as hexadecimal
            call.remove("0x");

            QByteArray call_ba;

            if (call.length() % 2 == 0)
            {
                // Jämnt antal tecken.
                for (int i = 0; i < call.length(); i += 2)
                {
                    bool partial_ok;
                    QString partial_string = call.mid(i, 2);

                    int partial_hex = partial_string.toInt(&partial_ok, 16);

                    if (partial_ok)
                    {
                        call_ba.append(partial_hex);
                    }
                    else
                    {
                        throw ControlException("Error: Not a valid hex input.\n");
                    }
                }
            }
            else
            {
                throw ControlException("Error: Not an even number of characters.\n");
            }

            if (data.contains(call_ba))
            {
                // Complete call sequence found -> respond
                parseInput(UserInput("transmit " + i.value()));
            }
        }
        else if (call.startsWith("0b"))
        {
            // Interpret call as binary
            call.remove("0b");
            // TO BE IMPLEMENTED
        }
        else
        {
            // Interpret call as ASCII

            if (data.contains(i.key().toLocal8Bit()))
            {
                // Complete call sequence found -> respond
                parseInput(UserInput("transmit " + i.value()));
            }
        }
    }

    // Custom parse code here


    // Print the received data
    printData(data_);
    data_.clear();
}

/*
 *  Private slots
 */

/* Reads data from the port buffer and prints */
void Control::readData()
{
    if (port_->isOpen() && port_->bytesAvailable() != 0)
    {
        data_.append(port_->readAll());

        parseData(data_);
    }
}

/* Repeats the input in repeated_input_ as long as repeat_timer_ is running */
void Control::repeatInput()
{
    try { parseInput(*repeated_input_); }
    catch (std::logic_error& e) { emit out(e.what()); }
}
