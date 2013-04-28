#include "qextserialport_utils.h"

#include <QStringList>

namespace utils
{
    BaudRateType toBaudRateType(const QString& baud_rate)
    {
        if (baud_rate == "1200")
            return BAUD1200;
        else if (baud_rate == "2400")
            return BAUD2400;
        else if (baud_rate == "4800")
            return BAUD4800;
        else if (baud_rate == "9600")
            return BAUD9600;
        else if (baud_rate == "19200")
            return BAUD19200;
        else if (baud_rate == "38400")
            return BAUD38400;
        else if (baud_rate == "57600")
            return BAUD57600;
        else if (baud_rate == "115200")
            return BAUD115200;
        else
            return BAUDINVALID;
    }

    DataBitsType toDataBitsType(const QString& data_bits)
    {
        if (data_bits == "5")
            return DATA_5;
        else if (data_bits == "6")
            return DATA_6;
        else if (data_bits == "7")
            return DATA_7;
        else if (data_bits == "8")
            return DATA_8;
        else
            return DATAINVALID;
    }

    ParityType toParityType(const QString& parity)
    {
        QString tmp = parity.toLower();

        if (tmp == "even")
            return PAR_EVEN;
        else if (tmp == "none")
            return PAR_NONE;
        else if (tmp == "odd")
            return PAR_ODD;
        else if (tmp == "space")
            return PAR_SPACE;
        else
            return PARINVALID;
    }

    StopBitsType toStopBitsType(const QString& stop_bits)
    {
        if (stop_bits == "1")
            return STOP_1;
        else if (stop_bits == "2")
            return STOP_2;
        else
            return STOPINVALID;
    }

    QString toString(BaudRateType baud_rate)
    {
        switch (baud_rate)
        {
        case BAUD1200:
            return "1200";
        case BAUD2400:
            return "2400";
        case BAUD9600:
            return "9600";
        case BAUD19200:
            return "19200";
        case BAUD38400:
            return "38400";
        case BAUD57600:
            return "57600";
        case BAUD115200:
            return "115200";
        default:
            return "Error";
        }
    }

    QString toString(DataBitsType data_bits)
    {
        switch (data_bits)
        {
        case DATA_5:
            return "5";
        case DATA_6:
            return "6";
        case DATA_7:
            return "7";
        case DATA_8:
            return "8";
        default:
            return "Error";
        }
    }

    QString toString(ParityType parity)
    {
        switch (parity)
        {
        case PAR_EVEN:
            return "Even";
        case PAR_NONE:
            return "None";
        case PAR_ODD:
            return "Odd";
        case PAR_SPACE:
            return "Space";
        default:
            return "Error";
        }
    }

    QString toString(StopBitsType stop_bits)
    {
        switch (stop_bits)
        {
        case STOP_1:
            return "1";
        case STOP_2:
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
}
