#ifndef MESSAGEFILTER_H
#define MESSAGEFILTER_H

#include <QByteArray>

class MessageFilter
{
public:
    enum FilterType
    {
        INVALID,
        SOMLENGTH,
        SOMEOM
    };

    MessageFilter();

    bool isValid() const;
    int message_length() const;
    FilterType type() const;
    QByteArray startOfMessage() const;
    QByteArray endOfMessage() const;

    void setMessageLength(int length);
    void setStartOfMessage(const QByteArray& ba);
    void setEndOfMessage(const QByteArray& ba);

private:
    int message_length_ = -1;
    FilterType type_;
    QByteArray start_of_message_;
    QByteArray end_of_message_;

    void validate();
};

#endif // MESSAGEFILTER_H
