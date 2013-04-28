#include "serialport.h"
#include "qextserialport_utils.h"

SerialPort::SerialPort(const QString& port_name,
                       const QString& baud_rate,
                       const QString& data_bits,
                       const QString& parity,
                       const QString& stop_bits,
                       QObject *parent)
    : QObject(parent)
{
    // Specifikationer
    parsePortSettings(baud_rate, data_bits, parity, stop_bits);

    // Skapa en port med ovanstående specifikationer
    port_ = new QextSerialPort(port_name, port_settings_, QextSerialPort::EventDriven);

    setupConnections();
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
    port_->setBaudRate(baud_rate);
}

void SerialPort::setDataBits(DataBitsType data_bits)
{
    port_->setDataBits(data_bits);
}

void SerialPort::setParity(ParityType parity)
{
    port_->setParity(parity);
}

void SerialPort::setStopBits(StopBitsType stop_bits)
{
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
     *  Private
     */
void SerialPort::setupConnections()
{
    connect(port_, SIGNAL(readyRead()), this, SLOT(readyReadRelay()));
}

void SerialPort::parsePortSettings(const QString& baud_rate, const QString& data_bits, const QString& parity, const QString& stop_bits)
{
    // LÄGG TILL ERRORHANTERING

    // Sätt baudrate
    port_settings_.BaudRate = utils::toBaudRateType(baud_rate);

    // Sätt antal databitar
    port_settings_.DataBits = utils::toDataBitsType(data_bits);

    // Sätt paritet
    port_settings_.Parity = utils::toParityType(parity);

    // Sätt antal stoppbitar
    port_settings_.StopBits = utils::toStopBitsType(stop_bits);
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
