#include "terminal.h"
#include "ui_terminal.h"
#include "userinput.h"

Terminal::Terminal(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Terminal)
{
    ui->setupUi(this);

    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(parseCommand()));
}

Terminal::~Terminal()
{
    delete ui;    
}

/* Ta hand om ny userinput */
void Terminal::parseCommand()
{
    UserInput input(ui->lineEdit->text());

    if (input.isValid())
        out("Valid input.");
    else
        out("Invalid input.");

    ui->lineEdit->clear();
}

/* Skriver till terminalfönstret */
void Terminal::out(const QString &str)
{
    ui->textEdit->append(str);
}
