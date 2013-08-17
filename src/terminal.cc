#include "terminal.h"
#include "ui_terminal.h"
#include "userinput.h"
#include "xmlcontrol.h"

#include <QDebug>
#include <QKeyEvent>
#include <QFile>

Terminal::Terminal(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Terminal)
{
    ui->setupUi(this);

    try
    {
        control_ = new Control(this);
    }
    catch (std::logic_error& e)
    {
        qDebug() << e.what();
        exit(-1);
    }

    connect(ui->lineEdit_command, SIGNAL(returnPressed()), this, SLOT(parseInput()));
    connect(ui->lineEdit_command, SIGNAL(editingFinished()), this, SLOT(resetCurrentLine()));
    connect(control_, SIGNAL(out(QString)), this, SLOT(out(QString)));

    ui->lineEdit_command->installEventFilter(this);

    control_->printWelcomeMessage();
}

Terminal::~Terminal()
{
    delete control_;
    delete ui;    
}

/* Ta hand om ny userinput */
void Terminal::parseInput()
{
    QString input_str = ui->lineEdit_command->text();

    if (input_str.isEmpty())
        return;

    UserInput input(input_str);

    if (input.isValid())
    {
        QString command = input.command();

        if (command == "clear")
        {
            ui->textEdit->clear();
        }
        else if (command == "exit")
        {
            exit(0);
        }
        else
        {
            try { control_->parseInput(input); }
            catch (std::logic_error& e) { out(e.what()); }
        }
    }
    else
    {
        out("Invalid or incomplete command '" + input.command() + "'.");
    }

    history_.prepend(input_str);
    ui->lineEdit_command->clear();
}

/* Skriver till terminalfönstret */
void Terminal::out(const QString& str)
{
    ui->textEdit->append(str + '\n');
}

/* Filtrera knapptryckningar */
bool Terminal::eventFilter(QObject* object, QEvent* event)
{
    if (object == ui->lineEdit_command)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent* key_event = static_cast<QKeyEvent*>(event);

            switch (key_event->key())
            {
            case Qt::Key_Up:
            {
                if (history_.isEmpty())
                    return false;

                // Mot äldre kommandon
                if (history_reset_)
                {
                    history_reset_ = false;
                }
                else
                {
                    if (current_line_ != history_.size() - 1)
                    // Öka om ej sista elementet
                        ++current_line_;
                }

                QString line = history_[current_line_].trimmed();
                ui->lineEdit_command->setText(line);

                return true;
            }

            case Qt::Key_Down:
            {
                if (history_.isEmpty())
                    return false;

                // Mot nyare kommandon
                if (current_line_ != 0)
                    // Minska om ej sista elementet
                    --current_line_;
                else
                {
                    // Rensa om sista raden
                    ui->lineEdit_command->clear();
                    return true;
                }

                QString line = history_[current_line_].trimmed();
                ui->lineEdit_command->setText(line);

                return true;
            }

            default:
                return false;
            }
        }
        else
            // Hantera inte annat än knapptryckningar
            return false;
    }
    else
        return QDialog::eventFilter(object, event);
}

void Terminal::resetCurrentLine()
{
    history_reset_ = true;
    current_line_ = 0;
}
