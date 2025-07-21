#ifndef PROGRAMLAMA_H
#define PROGRAMLAMA_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QProcess>

class TabModemProgramla : public QWidget {
    Q_OBJECT

public:
    explicit TabModemProgramla(QWidget *parent = nullptr);

private slots:
    void portlariTara();
    void modemiProgramla();
    

private:
    QList<QCheckBox*> portChecks;
    QList<QComboBox*> portCombos;
    QPlainTextEdit *outputEdit;
    QProcess *process;
};

#endif // PROGRAMLAMA_H
