#ifndef QEXTSERIALPORT_UTILS_H
#define QEXTSERIALPORT_UTILS_H

#include "qextserialport.h"

#include <QString>
#include <QByteArray>

namespace utils
{
    // Konverteringsfunktioner portspecifikationer <->
    BaudRateType toBaudRateType(const QString& baud_rate);
    DataBitsType toDataBitsType(const QString& data_bits);
    ParityType toParityType(const QString& parity);
    StopBitsType toStopBitsType(const QString& stop_bits);

    QString toString(BaudRateType baud_rate);
    QString toString(DataBitsType data_bits);
    QString toString(ParityType parity);
    QString toString(StopBitsType stop_bits);
    QString toString(const bool logic);

    // Stycka upp en QByteArray i ett mer läsligt format
    QString readableByteArray(const QByteArray& ba);
} // namespace utils

#endif // UTILS_H
