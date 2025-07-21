#ifndef VERI_EKLE_H
#define VERI_EKLE_H

#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QTextEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

class TabVeriEkle : public QWidget {
    Q_OBJECT

public:
    explicit TabVeriEkle(QWidget *parent = nullptr);

private slots:
    void updateResultField();
    void veriKaydet();

private:
    QLineEdit *seriEdit;
    QLabel *resultLabel;
    QCheckBox *eepromCheck, *rfCheck, *networkCheck;
    QTextEdit *aciklamaEdit;
    QTextEdit *logEdit;
};

#endif // VERI_EKLE_H
