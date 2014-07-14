#include "messagefilter.h"

MessageFilter::MessageFilter()
    : start_of_message_(QByteArray())
    , end_of_message_(QByteArray())
{
    validate();
}

bool MessageFilter::isValid() const
{
    return (type_ != FilterType::INVALID);
}

int MessageFilter::message_length() const
{
    return message_length_;
}

MessageFilter::FilterType MessageFilter::type() const
{
    return type_;
}

QByteArray MessageFilter::startOfMessage() const
{
    return start_of_message_;
}

QByteArray MessageFilter::endOfMessage() const
{
    return end_of_message_;
}

void MessageFilter::setMessageLength(int length)
{
    message_length_ = length;
    validate();
}

void MessageFilter::setStartOfMessage(const QByteArray &ba)
{
    start_of_message_ = ba;
    validate();
}

void MessageFilter::setEndOfMessage(const QByteArray &ba)
{
    end_of_message_ = ba;
    validate();
}

void MessageFilter::clear()
{
    start_of_message_.clear();
    end_of_message_.clear();
    message_length_ = -1;
    type_ = FilterType::INVALID;
}

void MessageFilter::validate()
{
    if (start_of_message_.isEmpty())
    {
        // SOM not defined
        type_ = FilterType::INVALID;
    }
    else if (message_length_ > 0 && end_of_message_.isEmpty())
    {
        // Message length defined but not EOM -> SOMLENGTH
        type_ = FilterType::SOMLENGTH;
    }
    else if (message_length_ <= 0 && !end_of_message_.isEmpty())
    {
        // EOM defined but not message length -> SOMEOM
        type_ = FilterType::SOMEOM;
    }
    else
    {
        // Invalid
        type_ = FilterType::INVALID;
    }

}
