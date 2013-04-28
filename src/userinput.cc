#include "userinput.h"

UserInput::UserInput(const QString& input)
{
    addAllowedInput();

    QString tmp = input;
    validateInput(tmp);

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
