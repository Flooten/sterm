#include "control.h"
#include "serialport_utils.h"

#include <QStringList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

Control::Control(QObject* parent)
    : QObject(parent)
    , mfilter_(new MessageFilter())
    , repeat_timer_(new QTimer())
    , report_lock_timer_(new QTimer())
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

    connect(port_, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(report_lock_timer_, SIGNAL(timeout()), this, SLOT(reportUnlock()));
    connect(repeat_timer_, SIGNAL(timeout()), this, SLOT(repeatInput()));

    report_lock_timer_->start(round(1000 / report_frequency_));
}

Control::~Control()
{
    delete port_settings_;
    delete mfilter_;
    delete repeat_timer_;
    delete report_lock_timer_;
    delete repeated_input_;
    delete call_response_map_;
    delete port_;
}

/* Function to parse new user input*/
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
        if (!arguments.isEmpty())
        {
            parseInput(UserInput("set port-name " + arguments.at(0)));
        }

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
    else if (command == "status" || command == "stat")
    {
        QStringList lst;

        lst << "Property:" << "Value:\n" <<
               "---------" << "------\n" <<
               "Port name" << port_->portName() + "\n" <<
               "Port status" << port_->state() + "\n" <<
               "Baud rate" << utils::toString(port_->baudRate()) + "\n" <<
               "Data bits" << utils::toString(port_->dataBits()) + "\n" <<
               "Parity" << utils::toString(port_->parity()) + "\n" <<
               "Stop bits" << utils::toString(port_->stopBits()) + "\n";

        emit out(lst);
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
    else if (command == "tset")
    {
        QString property = arguments.at(0);
        QString value = arguments.at(1);

        if (property == "report-frequency")
        {
            // Update report_frequency_
            // Conversion will always be successful because of RegExp verified user input
            report_frequency_ = value.toInt();
            report_lock_timer_->start(round(1000 / report_frequency_));
        }
    }
    else if (command == "lp")
    {
        QListIterator<QSerialPortInfo> i(QSerialPortInfo::availablePorts());

        char index = 1;

        QStringList lst;

        lst << "No." << "Name:" << "Status:\n" <<
               "---" << "-----" << "-------\n";

        while (i.hasNext())
        {
            QSerialPortInfo info = i.next();
            QString s;

            if (info.isBusy())
                s = QString("In Use");
            else
                s = QString("Available");

            lst << QString::number(index) + "." << info.portName() << s + "\n";

            ++index;
        }

        // No ports found
        if (index == 1)
            emit out("No ports available.");

        emit out(lst);
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
    else if (command == "respond" || command == "re")
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
        if (call_response_map_->isEmpty())
            throw ControlException("No responses defined.\n");


        QMapIterator<QString, QString> i(*call_response_map_);

        int index = 1;

        QStringList lst;

        lst << "No." << "Call:" << "Response:\n" <<
               "---" << "-----" << "---------\n";

        while (i.hasNext())
        {
            i.next();
            lst << QString::number(index) + "." << i.key() << i.value() + "\n";
            ++index;
        }

        emit out(lst);
    }
    else if (command == "remove" || command == "rm")
    {
        QString property = arguments.at(0);

        if (property == "response" || property == "re")
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
    else if (command == "filter")
    {
        QString type = arguments.at(0);

        if (type == "sl")
        {
            // Setup SOM + length message filter

            QString som = arguments.at(1);

            if (som.startsWith("0x"))
                mfilter_->setStartOfMessage(utils::asciiToHex(som));
            else
                mfilter_->setStartOfMessage(som.toLocal8Bit());

            mfilter_->setMessageLength(arguments.at(2).toInt());

            if (mfilter_->isValid())
                emit out(QString("Successfully set up message filter:\n") +
                         QString("Start-of-message:\t") + QString(mfilter_->startOfMessage()) + "\n" +
                         QString("Message length:\t") + QString::number(mfilter_->message_length()) + "\n");
        }
        else if (type == "se")
        {
            // Setup SOM + EOM message filter
            QString som = arguments.at(1);

            if (som.startsWith("0x"))
                mfilter_->setStartOfMessage(utils::asciiToHex(som));
            else
                mfilter_->setStartOfMessage(som.toLocal8Bit());

            QString eom = arguments.at(2);

            if (eom.startsWith("0x"))
                mfilter_->setEndOfMessage(utils::asciiToHex(eom));
            else
                mfilter_->setEndOfMessage(eom.toLocal8Bit());
        }
        else if (type == "remove")
        {
            mfilter_->clear();
        }
    }
    else if (command == "lf")
    {
        QStringList lst;
        lst << "Type:" << "SoM:" << "Length/EoM:\n"
            << "-----" << "----" << "-----------\n";

        if (mfilter_->isValid())
        {
            QString type;
            switch (mfilter_->type())
            {
            case MessageFilter::FilterType::SOMLENGTH:
                lst << "SoM + length" << "0x" + mfilter_->startOfMessage().toHex() << QString::number(mfilter_->message_length()) + "\n";
                break;

            case MessageFilter::FilterType::SOMEOM:
                lst << "SoM + EoM" << "0x" + mfilter_->startOfMessage().toHex() << "0x" +  mfilter_->endOfMessage().toHex() + "\n";
                break;

            default:
                type = "Invalid";
                break;
            }

            emit out(lst);
        }
        else
        {
            emit out("No filter defined.\n");
        }
    }
}

/* Function to print the welcome message */
void Control::printWelcomeMessage()
{
    emit out(sterm_settings_->text("welcome_message") + '\n');
}

/* If defined, applies the filter mfilter_ to the
 * received data and forwards the result to parseData() */
void Control::filterData(const QByteArray& data)
{
    // Construct messages if filter is active, otherwise print the data
    if (mfilter_->isValid())
    {
        QByteArray buffer = data;

        switch(mfilter_->type())
        {
        case MessageFilter::FilterType::SOMLENGTH:
            // Construct message of predefined length
            while (buffer.length() > 0)
            {
                if (constructing_)
                {
                    // # Remaining bytes
                    int remainder = mfilter_->message_length() - data_.length();

                    // Currently constructing a message
                    if (buffer.length() >= remainder)
                    {
                        // Complete remainder found
                        data_.append(buffer.mid(0, remainder));
                        buffer.remove(0, remainder);
                        constructing_ = false;

                        // Forward to parsing
                        parseData(data_);
                        data_.clear();
                    }
                    else
                    {
                        // Everything in buffer belongs to the message under construction
                        data_.append(buffer);
                        buffer.clear();
                    }
                }
                else
                {
                    // Look for start of message, discard other data
                    int index = buffer.indexOf(mfilter_->startOfMessage());

                    if (index != -1)
                    {
                        // SOM found, discard prepended data unless index = 0

                        if (index != 0)
                        {
                            // Discard data
                            buffer.remove(0, index);
                        }

                        // SOM is now @ index 0
                        int message_length = mfilter_->message_length();

                        if (buffer.length() >= message_length)
                        {
                            // Complete message in buffer, forward to parsing
                            parseData(buffer.mid(0, message_length));
                            buffer.remove(0, message_length);
                        }
                        else
                        {
                            // Incomplete message in buffer, start construction
                            data_.append(buffer);
                            buffer.clear();
                            constructing_ = true;
                        }
                    }
                    else
                    {
                        // Buffer filled with undesired data, discard
                        buffer.clear();
                    }
                }
            }
            break;

        case MessageFilter::FilterType::SOMEOM:
            while (buffer.length() > 0)
            {
                if (constructing_)
                {
                    // # Remaining bytes
                    int eom_index = buffer.indexOf(mfilter_->endOfMessage());

                    // Currently constructing a message
                    if (eom_index != -1)
                    {
                        // End-of-message found, complete current message.

                        // Adjust for EoM of arbitrary length
                        eom_index += mfilter_->endOfMessage().length();
                        data_.append(buffer.left(eom_index));
                        buffer.remove(0, eom_index);
                        constructing_ = false;

                        // Forward to parsing
                        parseData(data_);
                        data_.clear();
                    }
                    else
                    {
                        // Everything in buffer belongs to the message under construction
                        data_.append(buffer);
                        buffer.clear();
                    }
                }
                else
                {
                    // Look for start of message, discard other data
                    int index = buffer.indexOf(mfilter_->startOfMessage());

                    if (index != -1)
                    {
                        // SOM found, discard prepended data unless index = 0

                        if (index != 0)
                        {
                            // Discard data
                            buffer.remove(0, index);
                        }

                        // SOM is now @ index 0
                        int eom_index = buffer.indexOf(mfilter_->endOfMessage());

                        if (eom_index != -1)
                        {
                            // Complete message in buffer, forward to parsing

                            // Adjust for EoM of arbitrary length
                            eom_index += mfilter_->endOfMessage().length();
                            parseData(buffer.left(eom_index));
                            buffer.remove(0, eom_index);
                        }
                        else
                        {
                            // Incomplete message in buffer, start construction
                            data_.append(buffer);
                            buffer.clear();
                            constructing_ = true;
                        }
                    }
                    else
                    {
                        // Buffer filled with undesired data, discard
                        buffer.clear();
                    }
                }
            }
            break;

        default:
            break;
        }

    }
    else
    {
        // Call and response
        parseCallResponse(data);

        // Print the received data
        printData(data);
    }
}

/* Prints the contents of data */
void Control::printData(const QByteArray& data)
{
    if (!report_locked_)
    {
        emit out("Received " + QString::number(data.length()) + " bytes." + "\n" +
                 "ASCII:\t" + data + "\n" +
                 "Hex:\t" + utils::readableByteArray(data.toHex()) + "\n");

        // Lock terminal update
        report_locked_ = true;
    }
}

/* Function to analyze the received data */
void Control::parseData(const QByteArray& data)
{
    // CUSTOM PARSE CODE HERE


    // Call and response
    parseCallResponse(data);

    // Print the received data
    printData(data);
}

/* Function to respond to user defined calls in data */
void Control::parseCallResponse(const QByteArray& data)
{
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

            // Search through the message for registred calls
            int index = data.indexOf(call_ba);
            while (index != -1)
            {
                // Complete call sequence found -> respond
                parseInput(UserInput("transmit " + i.value()));
                index = data.indexOf(call_ba, index + call_ba.length());
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
}

/*
 *  Private slots
 */

/* Reads data from the port buffer and prints */
void Control::readData()
{
    filterData(port_->readAll());
}

/* Repeats the input in repeated_input_ as long as repeat_timer_ is running */
void Control::repeatInput()
{
    try { parseInput(*repeated_input_); }
    catch (std::logic_error& e) { emit out(e.what()); }
}

/* Unlocks the ability of printData() to write to the terminal */
void Control::reportUnlock()
{
    report_locked_ = false;
}
