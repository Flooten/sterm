#ifndef TERMINAL_H
#define TERMINAL_H

#include <QDialog>
#include <QString>

#include "control.h"

namespace Ui
{
    class Terminal;
}

class Terminal : public QDialog
{
    Q_OBJECT
    
public:
    explicit Terminal(QWidget *parent = 0);
    ~Terminal();
    
private:
    Ui::Terminal *ui;
    Control* control_;

private slots:
    void parseInput();
    void out(const QString& str);
};

#endif // TERMINAL_H
