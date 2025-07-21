#include "Kayit_Listele.h"

TabKayitListele::TabKayitListele(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel("Seri No:");
    searchEdit = new QLineEdit();
    searchEdit->setFixedWidth(200);
    searchLayout->addStretch();
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);
    searchLayout->addStretch();
    mainLayout->addLayout(searchLayout);

    QHBoxLayout *upperLayout = new QHBoxLayout();

    table = new QTableWidget();
    table->setFont(QFont("Arial", 11));
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"Seri No", "EEPROM", "RF", "Network", "Sonuç"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    for (int i = 0; i < 4; ++i)
        table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    upperLayout->addWidget(table, 4);

    QVBoxLayout *aciklamaLayout = new QVBoxLayout();
    QLabel *aciklamaLabel = new QLabel("Açıklama:");
    aciklamaViewer = new QTextEdit();
    aciklamaViewer->setReadOnly(true);
    aciklamaViewer->setMinimumHeight(100);
    aciklamaViewer->setFont(QFont("Arial", 10));
    aciklamaLayout->addWidget(aciklamaLabel);
    aciklamaLayout->addWidget(aciklamaViewer);
    upperLayout->addLayout(aciklamaLayout, 2);

    mainLayout->addLayout(upperLayout);

    QLabel *logLabel = new QLabel("Log:");
    logViewer = new QPlainTextEdit();
    logViewer->setReadOnly(true);
    logViewer->setFont(QFont("Courier New", 10));
    logViewer->setPlaceholderText("Loglar burada gösterilecek...");
    logViewer->setMinimumHeight(150);
    mainLayout->addWidget(logLabel);
    mainLayout->addWidget(logViewer);

    QPushButton *deleteBtn = new QPushButton("Seçilen Kaydı Sil");
    QPushButton *deleteLogBtn = new QPushButton("Seçilen Log Kaydını Sil");
    mainLayout->addWidget(deleteBtn);
    mainLayout->addWidget(deleteLogBtn);

    connect(searchEdit, &QLineEdit::textChanged, this, &TabKayitListele::ara);
    connect(deleteBtn, &QPushButton::clicked, this, &TabKayitListele::seciliKaydiSil);
    connect(deleteLogBtn, &QPushButton::clicked, this, &TabKayitListele::seciliLoguSil);
    connect(table, &QTableWidget::itemSelectionChanged, this, &TabKayitListele::verileriGoster);

    tabloyuDoldur();
}

void TabKayitListele::tabloyuDoldur(const QString &filter) {
    QSqlQuery query;
    if (filter.isEmpty()) {
        query.exec("SELECT seri_no, eeprom, rf, network, result FROM modem_testleri");
    } else {
        query.prepare("SELECT seri_no, eeprom, rf, network, result FROM modem_testleri WHERE seri_no LIKE :filter");
        query.bindValue(":filter", "%" + filter + "%");
        query.exec();
    }

    table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        for (int col = 0; col < 5; ++col) {
            QString text;
            QTableWidgetItem *item;

            if (col >= 1 && col <= 3) {
                bool val = query.value(col).toBool();
                text = val ? QString::fromUtf8("✅") : QString::fromUtf8("❌");
                item = new QTableWidgetItem(text);
                item->setFont(QFont("Segoe UI Emoji", 14));
            } else {
                text = query.value(col).toString();
                item = new QTableWidgetItem(text);
                item->setFont(QFont("Arial", 11));
            }

            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);

            if (col == 4) {
                if (text == "PASS")
                    item->setForeground(QBrush(Qt::darkGreen));
                else if (text == "FAIL")
                    item->setForeground(QBrush(Qt::red));
            }

            table->setItem(row, col, item);
        }
        row++;
    }
}

void TabKayitListele::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    tabloyuDoldur(searchEdit->text());
    logViewer->clear();
    aciklamaViewer->clear();
}

void TabKayitListele::ara(const QString &filter) {
    tabloyuDoldur(filter);
    logViewer->clear();
    aciklamaViewer->clear();
}

void TabKayitListele::seciliKaydiSil() {
    int row = table->currentRow();
    if (row < 0) return;

    QString seriNo = table->item(row, 0)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Kayıt Sil", "Seçilen kaydı silmek istediğinize emin misiniz?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM modem_testleri WHERE seri_no = :seri_no");
        query.bindValue(":seri_no", seriNo);
        if (query.exec()) {
            tabloyuDoldur(searchEdit->text());
            logViewer->clear();
            aciklamaViewer->clear();
        }
    }
}

void TabKayitListele::seciliLoguSil() {
    int row = table->currentRow();
    if (row < 0) return;

    QString seriNo = table->item(row, 0)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Log Sil", "Seçilen kaydın log bilgisini silmek istediğinize emin misiniz?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM modem_loglari WHERE seri_no = :seri_no");
        query.bindValue(":seri_no", seriNo);
        if (query.exec()) {
            logViewer->clear();
        }
    }
}

void TabKayitListele::verileriGoster() {
    int row = table->currentRow();
    if (row < 0) return;

    QString seriNo = table->item(row, 0)->text();

    QSqlQuery query1;
    query1.prepare("SELECT aciklama FROM modem_testleri WHERE seri_no = :seri_no");
    query1.bindValue(":seri_no", seriNo);
    if (query1.exec() && query1.next()) {
        aciklamaViewer->setPlainText(query1.value(0).toString());
    } else {
        aciklamaViewer->setPlainText("Açıklama bulunamadı.");
    }

    QSqlQuery query2;
    query2.prepare("SELECT log FROM modem_loglari WHERE seri_no = :seri_no");
    query2.bindValue(":seri_no", seriNo);
    if (query2.exec() && query2.next()) {
        logViewer->setPlainText(query2.value(0).toString());
    } else {
        logViewer->setPlainText("Log bulunamadı.");
    }
}
