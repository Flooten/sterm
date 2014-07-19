#include "terminal.h"
#include "ui_terminal.h"
#include "userinput.h"
#include "xmlcontrol.h"

#include <QDebug>
#include <QKeyEvent>
#include <QFile>
#include <QStringListIterator>
#include <vector>

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
    connect(control_, SIGNAL(out(QStringList)), this, SLOT(out(QStringList)));
    connect(control_, SIGNAL(clear()), this, SLOT(clear()));

    ui->lineEdit_command->installEventFilter(this);

    control_->printWelcomeMessage();
}

Terminal::~Terminal()
{
    delete control_;
    delete ui;    
}

/* Parse new user input */
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

/* Output to the terminal window */
void Terminal::out(const QString& str)
{
    ui->textEdit->append(str);
}

/* Support for table-style printing */
void Terminal::out(const QStringList& lst)
{
    // Local copy
    QStringList list(lst);

    QStringListIterator i(list);

    // Determine # columns
    int columns = 1;

    std::vector<int> column_sizes;

    while (i.hasNext())
    {
        column_sizes.push_back(0);

        if (i.next().contains("\n"))
            break;
        else
            ++columns;
    }

    // Determine column max width
    for (int i = 0; i < list.size(); ++i)
    {
        if (list.at(i).length() > column_sizes[i % columns])
            column_sizes[i % columns] = list.at(i).length();
    }

    // Add maxlength - cell length + separator spaces to all cells
    for (int i = 0; i < list.length(); ++i)
    {
        // Generate spaces
        QString fill = "";

        for (int j = 0; j < column_sizes[i % columns] + TABLE_SEP_ - list.at(i).length(); ++j)
            fill.append(" ");

        QString str = list.at(i);

        if (str.contains("\n"))
        {
            list.replace(i, str.remove("\n") + fill + "\n");
        }
        else
        {
            list.replace(i, str + fill);
        }
    }

    ui->textEdit->append(list.join(""));
}

/* Filter button presses */
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

/* Clears the terminal */
void Terminal::clear()
{
    ui->textEdit->clear();
}
