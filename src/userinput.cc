#include "userinput.h"

UserInput::UserInput(const QString& input)
{
    addAllowedInput();

    QString validation_copy = input;
    validateInput(validation_copy);

    arguments_ = input.split(" ");

    // Ta bort eventuella ws
    arguments_.removeAll("");

    command_ = arguments_.front();
    arguments_.pop_front();
}

bool UserInput::isValid() const
{
    return valid_;
}

QString UserInput::command() const
{
    return command_;
}

QStringList UserInput::arguments() const
{
    return arguments_;
}

/*
 *  Private
 */

void UserInput::addAllowedInput()
{
    allowed_input_.push_back(QRegExp("help(\\s+)?(transmit|open|close|exit|status|set)?"));
    allowed_input_.push_back(QRegExp("(transmit|tmit)(\\s+\\S+)+"));
    allowed_input_.push_back(QRegExp("open(\\s+)?"));
    allowed_input_.push_back(QRegExp("close(\\s+)?"));
    allowed_input_.push_back(QRegExp("exit(\\s+)?"));
    allowed_input_.push_back(QRegExp("status(\\s+)?"));
    allowed_input_.push_back(QRegExp("set(\\s+)?(port-name|baud-rate|data-bits|parity|stop-bits)\\s+\\S+"));
    allowed_input_.push_back(QRegExp("clear(\\s+)?"));
}

void UserInput::validateInput(QString& input)
{
    QVectorIterator<QRegExp> itr(allowed_input_);
    QRegExpValidator::State state;

    while (itr.hasNext())
    {
        validator.setRegExp(itr.next());

        int pos = 0;
        state = validator.validate(input, pos);

        if (state == QRegExpValidator::Acceptable)
        {
            valid_ = true;
            break;
        }
    }
}
