#include "Programlama.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QFileInfo>
#include <QApplication>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QFont>

TabModemProgramla::TabModemProgramla(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *multiPortLayout = new QHBoxLayout();
    QLabel *portLabel = new QLabel("Seri Portlar:");
    multiPortLayout->addWidget(portLabel);

    for (int i = 0; i < 4; ++i) {
        QCheckBox *check = new QCheckBox(QString("P%1").arg(i+1));
        QComboBox *combo = new QComboBox();
        combo->setFixedWidth(130);
        combo->setEnabled(false);

        connect(check, &QCheckBox::toggled, combo, &QComboBox::setEnabled);
        portChecks.append(check);
        portCombos.append(combo);

        QVBoxLayout *v = new QVBoxLayout();
        v->addWidget(check);
        v->addWidget(combo);
        multiPortLayout->addLayout(v);
    }

    QPushButton *taraBtn = new QPushButton("Port Tara");
    multiPortLayout->addWidget(taraBtn);
    mainLayout->addLayout(multiPortLayout);

    QPushButton *btnProgramla = new QPushButton("Seçilenleri Programla");
    mainLayout->addWidget(btnProgramla, 0, Qt::AlignCenter);

    outputEdit = new QPlainTextEdit();
    outputEdit->setReadOnly(true);
    outputEdit->setFont(QFont("Courier New", 10));
    mainLayout->addWidget(outputEdit);

    connect(taraBtn, &QPushButton::clicked, this, &TabModemProgramla::portlariTara);
    connect(btnProgramla, &QPushButton::clicked, this, &TabModemProgramla::modemiProgramla);

    portlariTara();
}

void TabModemProgramla::portlariTara() {
    QDir devDir("/dev");
    QStringList filters = {"ttyUSB*", "ttyACM*"};
    QStringList portList;
    for (const QString &filter : filters) {
        QStringList found = devDir.entryList(QStringList(filter), QDir::System | QDir::NoSymLinks);
        for (const QString &dev : found)
            portList << "/dev/" + dev;
    }

    for (QComboBox *combo : portCombos) {
        combo->clear();
        combo->addItems(portList);
    }
}

void TabModemProgramla::modemiProgramla() {
    QString baseDir = QCoreApplication::applicationDirPath();
    QString bslDir = baseDir + "/cc2538-bsl";
    QString scriptPath = bslDir + "/cc2538-bsl.py";
    QString firmwarePath = bslDir + "/meter-reader-v2.bin";

    if (!QFileInfo::exists(scriptPath) || !QFileInfo::exists(firmwarePath)) {
        outputEdit->appendPlainText("HATA: cc2538-bsl.py veya meter-reader-v2.bin bulunamadı!\nKlasör: " + bslDir);
        return;
    }

    outputEdit->clear();
    int xOffset = 100;
    int yOffset = 100;
    int xStep = 620;
    int pencereSayaci = 0;

    for (int i = 0; i < portCombos.size(); ++i) {
        if (!portChecks[i]->isChecked()) continue;

        QString seciliPort = portCombos[i]->currentText().trimmed();
        if (seciliPort.isEmpty()) {
            outputEdit->appendPlainText(QString("P%1: Seri port seçilmemiş!").arg(i + 1));
            continue;
        }

        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle(QString("P%1 - Programlama Çıktısı").arg(i + 1));
        dialog->resize(600, 400);
        dialog->move(xOffset + pencereSayaci * xStep, yOffset);

        QVBoxLayout *layout = new QVBoxLayout(dialog);
        QPlainTextEdit *terminalOutput = new QPlainTextEdit();
        terminalOutput->setReadOnly(true);
        terminalOutput->setFont(QFont("Courier New", 10));
        layout->addWidget(terminalOutput);
        dialog->show();

        pencereSayaci++;

        QProcess *p = new QProcess(dialog);
        QString *buffer = new QString();
        bool *doneFlag = new bool(false);
        bool *timeoutFlag = new bool(false);

        auto kontrolEt = [=]() mutable {
            QString b = buffer->toLower();

            if (!*doneFlag && b.contains("write done")) {
                
                *doneFlag = true;
                terminalOutput->appendPlainText(QString("P%1 - Modem programlama başarılı!").arg(i + 1));
                QMessageBox::information(dialog, "Başarılı", QString("P%1: Programlama tamamlandı.").arg(i + 1));
                outputEdit->appendPlainText(QString("P%1: Programlama başarılı!").arg(i + 1));
            }

            if (!*timeoutFlag && b.contains("error: timeout waiting for ack/nack")) {
                
                *timeoutFlag = true;
                terminalOutput->appendPlainText("Bağlantı hatası: ACK/NACK alınamadı!");
                QMessageBox::critical(dialog, "HATA", QString("P%1: Boot modu ya da bağlantı hatası.").arg(i + 1));
                outputEdit->appendPlainText(QString("P%1: Programlama başarısız!").arg(i + 1));
            }
        };

        connect(p, &QProcess::readyReadStandardOutput, [=]() mutable {
            QString out = QString::fromUtf8(p->readAllStandardOutput());
            terminalOutput->appendPlainText(out);
            *buffer += out;
            kontrolEt();
        });

        connect(p, &QProcess::readyReadStandardError, [=]() mutable {
            QString err = QString::fromUtf8(p->readAllStandardError());
            terminalOutput->appendPlainText(err);
            *buffer += err;
            kontrolEt();
        });

        QString program = "sudo";
        QStringList args;
        args << "python3" << scriptPath << "-e" << "-w" << "-v" << "-p" << seciliPort << firmwarePath;
        terminalOutput->appendPlainText("Komut:\n" + program + " " + args.join(" ") + "\n");

        p->setWorkingDirectory(bslDir);
        p->start(program, args);
    }
}
