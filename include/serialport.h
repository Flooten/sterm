#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "qextserialport.h"

#include <QString>
#include <QObject>
#include <stdexcept>

class SerialPortException : public std::logic_error
{
public:
    SerialPortException(const QString& str) : std::logic_error(str.toStdString()) {}
};

class SerialPort : public QObject
{
    Q_OBJECT
public:
    SerialPort(const QString& port_name,
               const QString& baud_rate,
               const QString& data_bits,
               const QString& parity,
               const QString& stop_bits,
               QObject *parent = nullptr);

    ~SerialPort();

    QString portName() const;
    BaudRateType baudRate() const;
    DataBitsType dataBits() const;
    ParityType parity() const;
    StopBitsType stopBits() const;
    QString state() const;

    void setPortName(const QString& port_name);
    void setBaudRate(BaudRateType baud_rate);
    void setDataBits(DataBitsType data_bits);
    void setParity(ParityType parity);
    void setStopBits(StopBitsType stop_bits);

    bool open();
    void close();
    void flush();
    bool isOpen() const;

    void transmit(const QString& msg);
    void transmit(const QByteArray& msg);

    QByteArray readAll() const;

    qint64 bytesAvailable() const;

private:
    QextSerialPort* port_;
    PortSettings port_settings_;
    const int TIMEOUT = 500;

signals:
    void readyRead();
    void bytesWritten(qint64 bytes_written);
    void out(const QString& str);

private slots:
    void readyReadRelay();
    void bytesWrittenRelay(qint64 byte_count);
};

#endif // SERIALPORT_H
