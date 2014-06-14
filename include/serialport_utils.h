#ifndef SERIALPORT_UTILS_H
#define SERIALPORT_UTILS_H

#include <QString>
#include <QByteArray>
#include <QSerialPort>

namespace utils
{
    // Konverteringsfunktioner portspecifikationer <->
    QSerialPort::BaudRate toBaudRate(const QString& baud_rate);
    QSerialPort::DataBits toDataBits(const QString& data_bits);
    QSerialPort::Parity toParity(const QString& parity);
    QSerialPort::StopBits toStopBits(const QString& stop_bits);

    QString toString(QSerialPort::BaudRate baud_rate);
    QString toString(QSerialPort::DataBits data_bits);
    QString toString(QSerialPort::Parity parity);
    QString toString(QSerialPort::StopBits stop_bits);
    QString toString(const bool logic);

    // Stycka upp en QByteArray i ett mer läsligt format
    QString readableByteArray(const QByteArray& ba);
}

#endif // SERIALPORT_UTILS_H
