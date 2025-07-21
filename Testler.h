#ifndef TESTLER_H
#define TESTLER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QMessageBox>
#include <QSerialPort>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QStringList>
#include <QDir> // Port listeleme için
#include <QComboBox>
#include <QCheckBox>

class TabTestBaslat : public QWidget {
    Q_OBJECT

public:
    explicit TabTestBaslat(QWidget *parent = nullptr);

private slots:
    void baslatTest();
    void durdurTest();
    void seriVeriOku();

private:
    void guncelleRssi(int rssi);
    void guncelleGorunum();
    void baslatBlink();
    void durBlink();
    bool RFTesti(const QStringList &logs);

    QSerialPort *port;
    QLabel *statusLabel;
    QLabel *eepromLabel, *rfLabel, *networkLabel, *genelSonucLabel, *rssiLabel;
    QPlainTextEdit *logEdit;
    QLineEdit *seriNoEdit;
    QPushButton *btnBaslat, *btnDurdur;
    QComboBox *portCombo;
    QComboBox *minRssiCombo;
    QComboBox *maxRssiCombo;
    QList<QComboBox*> portCombos;
    QList<QCheckBox*> portChecks;

    QTimer *blinkTimer = nullptr;
    int blinkCount = 0;
    bool blinkVisible = true;
    QStringList logLines;
    bool eeprom_ok = false;
    bool rf_ok = false;
    bool network_ok = false;
};

#endif // TESTLER_H
