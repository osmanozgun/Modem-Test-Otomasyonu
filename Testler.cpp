#include "Testler.h"
#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>

TabTestBaslat::TabTestBaslat(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    // PORT COMBO + TARA BUTONU
    QHBoxLayout *portLayout = new QHBoxLayout();
    QLabel *portLabel = new QLabel("Seri Port:");
    portCombo = new QComboBox();
    portCombo->setFixedWidth(200);
    QPushButton *btnPortTara = new QPushButton("Port Tara");
    btnPortTara->setFixedWidth(100);

    auto portlariListele = [this]() {
        QStringList portList;
        QDir devDir("/dev");
        QStringList filters = {"ttyUSB*", "ttyACM*"};
        for (const QString &filter : filters) {
            QStringList found = devDir.entryList(QStringList(filter), QDir::System | QDir::NoSymLinks);
            for (const QString &dev : found)
                portList << "/dev/" + dev;
        }
        portCombo->clear();
        portCombo->addItems(portList);
    };
    portlariListele();
    connect(btnPortTara, &QPushButton::clicked, this, portlariListele);

    portLayout->addStretch();
    portLayout->addWidget(portLabel);
    portLayout->addWidget(portCombo);
    portLayout->addWidget(btnPortTara);
    portLayout->addStretch();
    layout->addLayout(portLayout);

    // SERİ NO GİRİŞİ
    QHBoxLayout *seriKaydetLayout = new QHBoxLayout();
    QLabel *seriLabel = new QLabel("Seri No:");
    seriNoEdit = new QLineEdit();
    seriNoEdit->setFixedWidth(200);
    seriKaydetLayout->addStretch();
    seriKaydetLayout->addWidget(seriLabel);
    seriKaydetLayout->addWidget(seriNoEdit);
    seriKaydetLayout->addStretch();
    layout->addLayout(seriKaydetLayout);

    // RSSI Aralık Ayarı
    QHBoxLayout *rssiRangeLayout = new QHBoxLayout();
    QLabel *maxLabel = new QLabel("RSSI Max:");
    maxRssiCombo = new QComboBox();
    QLabel *minLabel = new QLabel("RSSI Min:");
    minRssiCombo = new QComboBox();
    

    for (int i = -120; i <= -80; i += 5)
    maxRssiCombo->addItem(QString::number(i));
    for (int i = -70; i <= 0; i += 5)
    minRssiCombo->addItem(QString::number(i));
    


    minRssiCombo->setCurrentText("-80");
    maxRssiCombo->setCurrentText("-50");

    rssiRangeLayout->addStretch();
    rssiRangeLayout->addWidget(minLabel);
    rssiRangeLayout->addWidget(minRssiCombo);
    rssiRangeLayout->addSpacing(20);
    rssiRangeLayout->addWidget(maxLabel);
    rssiRangeLayout->addWidget(maxRssiCombo);
    rssiRangeLayout->addStretch();
    layout->addLayout(rssiRangeLayout);

    // BUTONLAR
    QHBoxLayout *testBtnLayout = new QHBoxLayout();
    btnBaslat = new QPushButton("Testleri Başlat");
    btnDurdur = new QPushButton("Testleri Durdur");
    testBtnLayout->addStretch();
    testBtnLayout->addWidget(btnBaslat);
    testBtnLayout->addSpacing(20);
    testBtnLayout->addWidget(btnDurdur);
    testBtnLayout->addStretch();
    layout->addLayout(testBtnLayout);

    connect(btnBaslat, &QPushButton::clicked, this, &TabTestBaslat::baslatTest);
    connect(btnDurdur, &QPushButton::clicked, this, &TabTestBaslat::durdurTest);

    // DURUM ETİKETLERİ
    statusLabel = new QLabel("Durum: Bekleniyor...");
    statusLabel->setFont(QFont("Arial", 12));
    statusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel);

    eepromLabel = new QLabel("EEPROM Testi: ❌");
    rfLabel = new QLabel("RF Testi: ❌");
    networkLabel = new QLabel("Ağ Kalitesi Testi: ❌");
    rssiLabel = new QLabel("RSSI: -");
    genelSonucLabel = new QLabel("GENEL SONUÇ: FAIL");

    QFont labelFont("Arial", 13);
    for (QLabel *lbl : {eepromLabel, rfLabel, networkLabel, rssiLabel}) {
        lbl->setFont(labelFont);
        lbl->setAlignment(Qt::AlignCenter);
    }

    genelSonucLabel->setFont(QFont("Segoe UI Emoji", 14, QFont::Bold));
    genelSonucLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(eepromLabel);
    layout->addWidget(rfLabel);
    layout->addWidget(networkLabel);
    layout->addWidget(rssiLabel);
    layout->addWidget(genelSonucLabel);

    logEdit = new QPlainTextEdit();
    logEdit->setFont(QFont("Arial", 10));
    logEdit->setReadOnly(true);
    layout->addWidget(logEdit);

    port = new QSerialPort(this);
    connect(port, &QSerialPort::readyRead, this, &TabTestBaslat::seriVeriOku);
}

void TabTestBaslat::baslatTest() {
    QString seri_no = seriNoEdit->text().trimmed();
    QString selectedPort = portCombo->currentText();

    if (selectedPort.isEmpty()) {
        QMessageBox::warning(this, "Uyarı", "Lütfen bir seri port seçin!");
        return;
    }
    if (seri_no.isEmpty()) {
        QMessageBox::warning(this, "Uyarı", "Seri numarası giriniz!");
        return;
    }

    statusLabel->setText("Durum: Port açılıyor...");

    port->setPortName(selectedPort);
    port->setBaudRate(921600);
    port->setDataBits(QSerialPort::Data8);
    port->setParity(QSerialPort::NoParity);
    port->setStopBits(QSerialPort::OneStop);
    port->setFlowControl(QSerialPort::NoFlowControl);

    if (!port->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Hata", "Seri port açılamadı:\n" + port->errorString());
        return;
    }

    statusLabel->setText("Durum: Dinleniyor...");
    logEdit->appendPlainText(">> Port açıldı, veri bekleniyor...");

    eeprom_ok = rf_ok = network_ok = false;
    rssiLabel->setText("RSSI: -");
    rssiLabel->setStyleSheet("");
    logLines.clear();

    durBlink();
    blinkCount = 0;
    guncelleGorunum();

    QSqlQuery testQuery;
    testQuery.prepare("INSERT INTO modem_testleri (seri_no, eeprom, rf, network, result, aciklama) "
                      "VALUES (:seri_no, 0, 0, 0, :result, '') "
                      "ON CONFLICT(seri_no) DO UPDATE SET "
                      "eeprom = 0, rf = 0, network = 0, result = :result2, aciklama = ''");
    testQuery.bindValue(":seri_no", seri_no);
    testQuery.bindValue(":result", "FAIL");
    testQuery.bindValue(":result2", "FAIL");
    testQuery.exec();
}

void TabTestBaslat::durdurTest() {
    if (port && port->isOpen()) {
        port->close();
        logEdit->appendPlainText(">> Port kapatıldı.");
        statusLabel->setText("Durum: Test durduruldu.");
    }
}

void TabTestBaslat::seriVeriOku() {
    QString seri_no = seriNoEdit->text().trimmed();
    if (seri_no.isEmpty()) return;

    QByteArray data = port->readAll();
    QString text = QString::fromUtf8(data).trimmed();
    logEdit->appendPlainText(">> " + text);
    logLines.append(text);

    QSqlQuery update;
    if (text.contains("Flash open success", Qt::CaseInsensitive)) {
        eeprom_ok = true;
        update.prepare("UPDATE modem_testleri SET eeprom = 1 WHERE seri_no = :seri_no");
    } else if (text.contains("Flash open fail", Qt::CaseInsensitive)) {
        eeprom_ok = false;
        update.prepare("UPDATE modem_testleri SET eeprom = 0 WHERE seri_no = :seri_no");
    }

    if (!update.lastQuery().isEmpty()) {
        update.bindValue(":seri_no", seri_no);
        update.exec();
    }

    if (text.contains("rssip", Qt::CaseInsensitive)) {
        QRegularExpression re("rssip\\s*(-?\\d+)");
        QRegularExpressionMatch match = re.match(text);
        if (match.hasMatch()) {
            int rssi = match.captured(1).toInt();
            guncelleRssi(rssi);

            int minRssi = minRssiCombo->currentText().toInt();
            int maxRssi = maxRssiCombo->currentText().toInt();

            if (rssi >= maxRssi && rssi <= minRssi) {
                network_ok = true;
                QSqlQuery q;
                q.prepare("UPDATE modem_testleri SET network = 1 WHERE seri_no = :seri_no");
                q.bindValue(":seri_no", seri_no);
                q.exec();
            }
        }
    }


    guncelleGorunum();

    QSqlQuery q;
    q.prepare("UPDATE modem_testleri SET result = :res WHERE seri_no = :seri_no");
    q.bindValue(":res", (eeprom_ok && rf_ok && network_ok) ? "PASS" : "FAIL");
    q.bindValue(":seri_no", seri_no);
    q.exec();
}

void TabTestBaslat::guncelleRssi(int rssi) {
    rssiLabel->setText(QString("RSSI: %1 dBm").arg(rssi));
    rssiLabel->setStyleSheet(rssi >= -50
        ? "QLabel { background-color: lightgreen; font-weight: bold; }"
        : "QLabel { background-color: orange; font-weight: bold; }");
}

void TabTestBaslat::guncelleGorunum() {
    QFont tickFont("Segoe UI Emoji", 13);
    rf_ok = RFTesti(logLines);

    eepromLabel->setText(QString("EEPROM Testi: %1").arg(eeprom_ok ? "✅" : "❌"));
    rfLabel->setText(QString("RF Testi: %1").arg(rf_ok ? "✅" : "❌"));
    networkLabel->setText(QString("Ağ Kalitesi Testi: %1").arg(network_ok ? "✅" : "❌"));

    for (QLabel *lbl : {eepromLabel, rfLabel, networkLabel})
        lbl->setFont(tickFont);

    bool pass = eeprom_ok && rf_ok && network_ok;
    genelSonucLabel->setText(pass ? "GENEL SONUÇ: PASS" : "GENEL SONUÇ: FAIL");
    genelSonucLabel->setStyleSheet(pass
        ? "QLabel { color: green; font-weight: bold; }"
        : "QLabel { color: red; font-weight: bold; }");

    baslatBlink();
}

void TabTestBaslat::baslatBlink() {
    durBlink();
    blinkTimer = new QTimer(this);
    blinkCount = 0;
    blinkVisible = true;

    connect(blinkTimer, &QTimer::timeout, this, [=]() mutable {
        blinkCount++;
        if (blinkCount >= 6) {
            durBlink();
            eepromLabel->setStyleSheet(eeprom_ok ? "background-color: lightgreen;" : "background-color: red; color: white;");
            rfLabel->setStyleSheet(rf_ok ? "background-color: lightgreen;" : "background-color: red; color: white;");
            networkLabel->setStyleSheet(network_ok ? "background-color: lightgreen;" : "background-color: red; color: white;");
            return;
        }

        QString yesil = "background-color: lightgreen; font-weight: bold;";
        QString kirmizi = "background-color: red; color: white; font-weight: bold;";
        eepromLabel->setStyleSheet(blinkVisible ? (eeprom_ok ? yesil : kirmizi) : "");
        rfLabel->setStyleSheet(blinkVisible ? (rf_ok ? yesil : kirmizi) : "");
        networkLabel->setStyleSheet(blinkVisible ? (network_ok ? yesil : kirmizi) : "");
        blinkVisible = !blinkVisible;
    });

    blinkTimer->start(300);
}

void TabTestBaslat::durBlink() {
    if (blinkTimer) {
        blinkTimer->stop();
        delete blinkTimer;
        blinkTimer = nullptr;
    }
}

bool TabTestBaslat::RFTesti(const QStringList &logs) {
    int txOkCount = 0, macCreatedCount = 0, rssiCount = 0;
    for (const QString &line : logs) {
        QString lower = line.toLower();
        if (lower.contains("tx ok")) txOkCount++;
        if (lower.contains("4emac:ka is created")) macCreatedCount++;
        if (lower.contains("rssip")) rssiCount++;
    }
    return txOkCount >= 3 && macCreatedCount >= 2 && rssiCount >= 2;
}
