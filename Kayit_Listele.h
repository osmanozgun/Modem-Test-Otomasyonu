#ifndef KAYIT_LISTELE_H
#define KAYIT_LISTELE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QBrush>
#include <QFont>

class TabKayitListele : public QWidget {
    Q_OBJECT

public:
    explicit TabKayitListele(QWidget *parent = nullptr);
    void tabloyuDoldur(const QString &filter = "");

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    void ara(const QString &filter);
    void seciliKaydiSil();
    void seciliLoguSil();
    void verileriGoster();

private:
    QTableWidget *table;
    QLineEdit *searchEdit;
    QPlainTextEdit *logViewer;
    QTextEdit *aciklamaViewer;

    
};

#endif // KAYIT_LISTELE_H
