#include "serialport_utils.h"

namespace utils
{
    QSerialPort::BaudRate toBaudRate(const QString& baud_rate)
    {
        if (baud_rate == "1200")
            return QSerialPort::Baud1200;
        else if (baud_rate == "2400")
            return QSerialPort::Baud2400;
        else if (baud_rate == "4800")
            return QSerialPort::Baud4800;
        else if (baud_rate == "9600")
            return QSerialPort::Baud9600;
        else if (baud_rate == "19200")
            return QSerialPort::Baud19200;
        else if (baud_rate == "38400")
            return QSerialPort::Baud38400;
        else if (baud_rate == "57600")
            return QSerialPort::Baud57600;
        else if (baud_rate == "115200")
            return QSerialPort::Baud115200;
        else
            return QSerialPort::UnknownBaud;
    }

    QSerialPort::DataBits toDataBits(const QString &data_bits)
    {
        if (data_bits == "5")
            return QSerialPort::Data5;
        else if (data_bits == "6")
            return QSerialPort::Data6;
        else if (data_bits == "7")
            return QSerialPort::Data7;
        else if (data_bits == "8")
            return QSerialPort::Data8;
        else
            return QSerialPort::UnknownDataBits;
    }

    QSerialPort::Parity toParity(const QString &parity)
    {
        QString tmp = parity.toLower();

        if (tmp == "even")
            return QSerialPort::EvenParity;
        else if (tmp == "none")
            return QSerialPort::NoParity;
        else if (tmp == "odd")
            return QSerialPort::OddParity;
        else if (tmp == "space")
            return QSerialPort::SpaceParity;
        else
            return QSerialPort::UnknownParity;
    }

    QSerialPort::StopBits toStopBits(const QString &stop_bits)
    {
        if (stop_bits == "1")
            return QSerialPort::OneStop;
        else if (stop_bits == "2")
            return QSerialPort::TwoStop;
        else
            return QSerialPort::UnknownStopBits;
    }

    QString toString(QSerialPort::BaudRate baud_rate)
    {
        switch (baud_rate)
        {
        case QSerialPort::Baud1200:
            return "1200";
        case QSerialPort::Baud2400:
            return "2400";
        case QSerialPort::Baud9600:
            return "9600";
        case QSerialPort::Baud19200:
            return "19200";
        case QSerialPort::Baud38400:
            return "38400";
        case QSerialPort::Baud57600:
            return "57600";
        case QSerialPort::Baud115200:
            return "115200";
        default:
            return "Error";
        }
    }

    QString toString(QSerialPort::DataBits data_bits)
    {
        switch (data_bits)
        {
        case QSerialPort::Data5:
            return "5";
        case QSerialPort::Data6:
            return "6";
        case QSerialPort::Data7:
            return "7";
        case QSerialPort::Data8:
            return "8";
        default:
            return "Error";
        }
    }

    QString toString(QSerialPort::Parity parity)
    {
        switch (parity)
        {
        case QSerialPort::EvenParity:
            return "Even";
        case QSerialPort::NoParity:
            return "None";
        case QSerialPort::OddParity:
            return "Odd";
        case QSerialPort::SpaceParity:
            return "Space";
        default:
            return "Error";
        }
    }

    QString toString(QSerialPort::StopBits stop_bits)
    {
        switch (stop_bits)
        {
        case QSerialPort::OneStop:
            return "1";
        case QSerialPort::TwoStop:
            return "2";
        default:
            return "Error";
        }
    }


    QString toString(const bool logic)
    {
        if (logic)
            return "On";
        else
            return "Off";
    }


    QString readableByteArray(const QByteArray& ba)
    {
        QString str(ba);

        for (int i = 2; i < str.length(); i += 3)
            // Dela upp i grupper om 2
            str.insert(i, " ");

        return str;
    }

    /* Hexadecimal data in ASCII interpreted to QByteArray */
    QByteArray asciiToHex(const QString& str)
    {
        QString str_tmp = str;

        if (str.startsWith("0x"))
        {
            str_tmp.remove("0x");
        }

        QByteArray ba;

        if (str_tmp.length() % 2 == 0)
        {
            // Jämnt antal tecken.
            for (int i = 0; i < str_tmp.length(); i += 2)
            {
                bool partial_ok;
                QString partial_string = str_tmp.mid(i, 2);

                int partial_hex = partial_string.toInt(&partial_ok, 16);

                if (partial_ok)
                {
                    ba.append(partial_hex);
                }
                else
                {
                    return QByteArray();
                }
            }
        }
        else
        {
            return QByteArray();
        }

        return ba;
    }

}
