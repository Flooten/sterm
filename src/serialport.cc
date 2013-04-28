#include "serialport.h"
#include "qextserialport_utils.h"

SerialPort::SerialPort(const QString& port_name,
                       const QString& baud_rate,
                       const QString& data_bits,
                       const QString& parity,
                       const QString& stop_bits,
                       QObject *parent)
    : QObject(parent)
    , port_(new QextSerialPort())
{
    setPortName(port_name);
    setBaudRate(utils::toBaudRateType(baud_rate));
    setDataBits(utils::toDataBitsType(data_bits));
    setParity(utils::toParityType(parity));
    setStopBits(utils::toStopBitsType(stop_bits));

    connect(port_, SIGNAL(readyRead()), this, SLOT(readyReadRelay()));

    port_->setTimeout(TIMEOUT);
}

SerialPort::~SerialPort()
{
    close();
    delete port_;
}

/*
 *  Public
 */

/* Öppna porten */
bool SerialPort::open()
{
    if (!port_->isOpen())
        return port_->open(QIODevice::ReadWrite);

    return false;
}

/* Stäng porten */
void SerialPort::close()
{
    if (port_->isOpen())
        port_->close();
}

void SerialPort::flush()
{
    port_->flush();
}

bool SerialPort::isOpen() const
{
    return port_->isOpen();
}

/* Getters */
QString SerialPort::portName() const
{
    return port_->portName();
}

BaudRateType SerialPort::baudRate() const
{
    return port_->baudRate();
}

DataBitsType SerialPort::dataBits()  const
{
    return port_->dataBits();
}

ParityType SerialPort::parity() const
{
    return port_->parity();
}

StopBitsType SerialPort::stopBits() const
{
    return port_->stopBits();
}

QString SerialPort::state() const
{
    if (port_->isOpen())
        return "OPEN";
    else
        return "CLOSED";
}

/* Setters */
void SerialPort::setPortName(const QString& port_name)
{
    port_->setPortName(port_name);
}

void SerialPort::setBaudRate(BaudRateType baud_rate)
{    
    if (baud_rate == BAUDINVALID)
        throw SerialPortException("Error: Invalid baud rate.");

    port_->setBaudRate(baud_rate);
}

void SerialPort::setDataBits(DataBitsType data_bits)
{
    if (data_bits == DATAINVALID)
        throw SerialPortException("Error: Invalid number of data bits.");

    port_->setDataBits(data_bits);
}

void SerialPort::setParity(ParityType parity)
{
    if (parity == PARINVALID)
        throw SerialPortException("Error: Invalid parity.");

    port_->setParity(parity);
}

void SerialPort::setStopBits(StopBitsType stop_bits)
{
    if (stop_bits == STOPINVALID)
        throw SerialPortException("Error: Invalid number of stop bits.");

    port_->setStopBits(stop_bits);
}

/* Skicka */
void SerialPort::transmit(const QString& msg)
{
    transmit(msg.toLocal8Bit());
}

void SerialPort::transmit(const QByteArray& msg)
{
    if (port_->isOpen())
    {
        qint64 bytes_written = port_->write(msg);

        if (bytes_written == 0)
            emit bytesWritten(msg.length());
        else
            emit bytesWritten(bytes_written);
    }
}

QByteArray SerialPort::readAll() const
{
    return port_->readAll();
}

qint64 SerialPort::bytesAvailable() const
{
    return port_->bytesAvailable();
}

/*
 *  Slots
 */
void SerialPort::readyReadRelay()
{
    emit readyRead();
}

void SerialPort::bytesWrittenRelay(qint64 bytes_written)
{
    emit bytesWritten(bytes_written);
}
