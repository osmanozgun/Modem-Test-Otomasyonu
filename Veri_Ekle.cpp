#include "Veri_Ekle.h"

TabVeriEkle::TabVeriEkle(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *veriBox = new QGroupBox("Modem Test Girişi");
    QVBoxLayout *veriBoxLayout = new QVBoxLayout();

    QWidget *centerWidget = new QWidget();
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setAlignment(Qt::AlignCenter);

    QHBoxLayout *seriLayout = new QHBoxLayout();
    QLabel *seriLabel = new QLabel("Seri No:");
    seriEdit = new QLineEdit();
    seriEdit->setFixedWidth(200);
    seriLayout->addStretch();
    seriLayout->addWidget(seriLabel);
    seriLayout->addWidget(seriEdit);
    seriLayout->addStretch();
    centerLayout->addLayout(seriLayout);

    QHBoxLayout *checkLayout = new QHBoxLayout();
    eepromCheck = new QCheckBox("EEPROM Testi");
    rfCheck = new QCheckBox("RF Bağlantı Testi");
    networkCheck = new QCheckBox("Ağ Kalitesi Testi");
    checkLayout->addWidget(eepromCheck);
    checkLayout->addWidget(rfCheck);
    checkLayout->addWidget(networkCheck);
    checkLayout->setAlignment(Qt::AlignCenter);
    centerLayout->addLayout(checkLayout);

    QHBoxLayout *resultLayout = new QHBoxLayout();
    QLabel *resultTextLabel = new QLabel("Sonuç:");
    resultLabel = new QLabel("FAIL");
    resultLabel->setFont(QFont("Segoe UI Emoji", 11, QFont::Bold));
    resultLabel->setStyleSheet("QLabel { color: red; }");
    resultLayout->addWidget(resultTextLabel);
    resultLayout->addSpacing(10);
    resultLayout->addWidget(resultLabel);
    resultLayout->setAlignment(Qt::AlignCenter);
    centerLayout->addLayout(resultLayout);

    veriBoxLayout->addWidget(centerWidget);
    veriBox->setLayout(veriBoxLayout);
    mainLayout->addWidget(veriBox);

    QGroupBox *logBox = new QGroupBox("");
    QVBoxLayout *logLayout = new QVBoxLayout();

    aciklamaEdit = new QTextEdit();
    aciklamaEdit->setPlaceholderText("Test açıklamasını girin...");
    logLayout->addWidget(new QLabel("Açıklama:"));
    logLayout->addWidget(aciklamaEdit);

    logEdit = new QTextEdit();
    logEdit->setPlaceholderText("Logları buraya girin...");
    logLayout->addWidget(new QLabel("Log Ekle:"));
    logLayout->addWidget(logEdit);

    QPushButton *btnKaydet = new QPushButton("Veritabanına Kaydet");
    logLayout->addWidget(btnKaydet, 0, Qt::AlignCenter);

    logBox->setLayout(logLayout);
    mainLayout->addWidget(logBox);

    connect(btnKaydet, &QPushButton::clicked, this, &TabVeriEkle::veriKaydet);
    connect(eepromCheck, &QCheckBox::stateChanged, this, &TabVeriEkle::updateResultField);
    connect(rfCheck, &QCheckBox::stateChanged, this, &TabVeriEkle::updateResultField);
    connect(networkCheck, &QCheckBox::stateChanged, this, &TabVeriEkle::updateResultField);
}

void TabVeriEkle::updateResultField() {
    bool allChecked = eepromCheck->isChecked() && rfCheck->isChecked() && networkCheck->isChecked();
    resultLabel->setText(allChecked ? "PASS" : "FAIL");
    resultLabel->setFont(QFont("Segoe UI Emoji", 11, QFont::Bold));
    resultLabel->setStyleSheet(allChecked
        ? "QLabel { color: green; }"
        : "QLabel { color: red; }");
}

void TabVeriEkle::veriKaydet() {
    QString seri_no = seriEdit->text().trimmed();
    QString result = resultLabel->text().trimmed();
    QString aciklama = aciklamaEdit->toPlainText().trimmed();
    QString logText = logEdit->toPlainText().trimmed();

    if (seri_no.isEmpty()) {
        QMessageBox::warning(this, "Uyarı", "Seri numarası boş olamaz!");
        return;
    }

    int eeprom = eepromCheck->isChecked() ? 1 : 0;
    int rf = rfCheck->isChecked() ? 1 : 0;
    int network = networkCheck->isChecked() ? 1 : 0;

    QSqlQuery query;
    query.prepare("INSERT INTO modem_testleri (seri_no, eeprom, rf, network, result, aciklama) "
                  "VALUES (:seri_no, :eeprom, :rf, :network, :result, :aciklama) "
                  "ON CONFLICT(seri_no) DO UPDATE SET "
                  "eeprom = :eeprom2, rf = :rf2, network = :network2, result = :result2, aciklama = :aciklama2");

    query.bindValue(":seri_no", seri_no);
    query.bindValue(":eeprom", eeprom);
    query.bindValue(":rf", rf);
    query.bindValue(":network", network);
    query.bindValue(":result", result);
    query.bindValue(":aciklama", aciklama);
    query.bindValue(":eeprom2", eeprom);
    query.bindValue(":rf2", rf);
    query.bindValue(":network2", network);
    query.bindValue(":result2", result);
    query.bindValue(":aciklama2", aciklama);

    if (!query.exec()) {
        QMessageBox::critical(this, "Hata", "Veri kaydedilemedi!\n\n" + query.lastError().text());
        return;
    }

    QSqlQuery logQuery;
    logQuery.prepare("INSERT OR REPLACE INTO modem_loglari (seri_no, log) VALUES (:seri_no, :log)");
    logQuery.bindValue(":seri_no", seri_no);
    logQuery.bindValue(":log", logText);
    logQuery.exec();

    QMessageBox::information(this, "Başarılı", "Kayıt başarıyla kaydedildi.");
}
