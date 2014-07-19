#ifndef TERMINAL_H
#define TERMINAL_H

#include <QDialog>
#include <QString>
#include <QStringList>

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
    QStringList history_;    
    bool history_reset_ = true;
    int current_line_ = 0;

    const int TABLE_SEP_ = 3;

    bool eventFilter(QObject* object, QEvent* event);

private slots:
    void parseInput();
    void out(const QString& str);
    void out(const QStringList& lst);
    void resetCurrentLine();
    void clear();
};

#endif // TERMINAL_H
