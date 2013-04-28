#include "terminal.h"
#include "ui_terminal.h"
#include "userinput.h"
#include "xmlcontrol.h"

#include <QDebug>

Terminal::Terminal(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Terminal)
{
    ui->setupUi(this);

    try
    {
        control_ = new Control(this);
    }
    catch (XmlException &e)
    {
        qDebug() << e.what();
        exit(-1);
    }

    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(parseInput()));
    connect(control_, SIGNAL(out(QString)), this, SLOT(out(QString)));
}

Terminal::~Terminal()
{
    delete ui;    
}

/* Ta hand om ny userinput */
void Terminal::parseInput()
{
    UserInput input(ui->lineEdit->text());

    if (input.isValid())
    {
        try { control_->parseInput(input); }
        catch (ControlException& e) { out(e.what()); }
    }
    else
    {
        out("Invalid or incomplete command '" + input.command() + "'.");
    }

    ui->lineEdit->clear();
}

/* Skriver till terminalfönstret */
void Terminal::out(const QString& str)
{
    ui->textEdit->append(str);
}
