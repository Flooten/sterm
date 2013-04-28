#ifndef USERINPUT_H
#define USERINPUT_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QRegExp>
#include <QRegExpValidator>

class UserInput
{
public:
    UserInput(const QString &input);

    bool isValid() const;
    QString command() const;
    QStringList arguments() const;

private:

    bool valid_ = false;
    QString command_;
    QStringList arguments_;
    QVector<QRegExp> allowed_input_;
    QRegExpValidator validator;

    void addAllowedInput();
    void validateInput(QString& input);
};

#endif // USERINPUT_H
