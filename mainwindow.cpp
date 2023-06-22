#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QDir>
#include <QListWidgetItem>
#include <QSql>
#include <QTableWidget>
#include <QTreeWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QFileSystemModel>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QCheckBox>


void MainWindow::on_tarananDosyalarListWidget_itemClicked(QListWidgetItem *item)
{

    QString dosyaAdi = item->text();
    for (int i = 0; i < m_tarananDosyalar.count(); ++i) {
        QFileInfo dosya = m_tarananDosyalar.at(i);
        if (dosya.fileName() == dosyaAdi) {
            m_secilenDosya = dosya;
            break;
        }
    }
}

void MainWindow::listeleTarananDosyalar(const QStringList &dosyaListesi)
{

    ui->listWidget->clear();
    for ( auto i : dosyaListesi)
    {
        ui->listWidget->addItem(i);
    }
}


void MainWindow::dosyaSec()
{

    QFileDialog* _f_dlg = new QFileDialog(this);
      _f_dlg->setFileMode(QFileDialog::Directory);
      _f_dlg->setOption(QFileDialog::DontUseNativeDialog, true);


      QListView *l = _f_dlg->findChild<QListView*>("listView");
      if (l) {
        l->setSelectionMode(QAbstractItemView::MultiSelection);
       }

      QTreeView *t = _f_dlg->findChild<QTreeView*>();
       if (t) {
         t->setSelectionMode(QAbstractItemView::MultiSelection);
        }

      int nMode = _f_dlg->exec();
      QString a;
      QStringList dosyaYollari = _f_dlg->selectedFiles();
        static int i = 0;
        QStringList dosyaBilgileri;
      for (const QString& dosyaYolu : dosyaYollari) {
          QFileInfo fileInfo(dosyaYolu);
          QString dosyaAdi = fileInfo.fileName();
          QFileInfo dosya(fileInfo);
          if (i)
          {
              a.append(fileInfo.absoluteFilePath());
              qDebug() << "Dosya : " << fileInfo.fileName();
              dosyaBilgileri.append(fileInfo.absoluteFilePath());

          }
            i++;


          m_tarananDosyalar.append(dosya);

      }
      qDebug() << "Dosya kaydedildi: " << dosyaBilgileri;
      listeleTarananDosyalar(dosyaBilgileri);

}
void MainWindow::on_itemSelected(QString item)
{
    ui->listWidget->addItem(item);
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    connect(ui->dosyaSecButton, &QPushButton::clicked, this, &MainWindow::dosyaSec);
    //connect(w2, SIGNAL(itemSelected(QString)), this, SLOT(on_itemSelected(QString)));
    //connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::on_itemSelectButton_clicked);
    connect(ui->silButton_2, &QPushButton::clicked, this, &MainWindow::on_silButton_2_clicked);
    connect(ui->taraButton, &QPushButton::clicked, this, &MainWindow::on_taraButton_clicked);
    connect(ui->detayGosterButton, &QPushButton::clicked, this, &MainWindow::on_detayGosterButton_clicked);
    ui->stackedWidget->setCurrentIndex(0);

    QTimer* timer;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(autoScan()));

    timer->start(60000);

}

void MainWindow::on_kaydetButton_clicked()
{
//listWidget öğesine eklenen dosyalardan seçilen dosyanın sık kullanılanlara kaydedilmesi
    QString dosyaAdi = ui->listWidget->currentItem()->text();
    QString dosyaBoyutu = QString::number(m_secilenDosya.size());


    QSqlQuery query;
    query.prepare("INSERT INTO saved_files (file_name, time,file_size) VALUES (:file_name, :time, :file_size)");
    query.bindValue(":file_name", dosyaAdi);
    query.bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":file_size", dosyaBoyutu);
    if (query.exec()) {
        qDebug() << "Dosya kaydedildi: " << dosyaAdi;
    } else {
        qDebug() << "Dosya kaydedilemedi: " << dosyaAdi << query.lastError().text();
    }

}



void MainWindow::on_listeleButton_clicked()
{

   ui->stackedWidget->setCurrentIndex(2);
   listeleKaydedilenDosyalar();
}

void MainWindow::on_taramaButton_clicked() {
//listwidget öğesine eklen her dosya ve klasör taranır

    QString messageText = "Tüm dosyaların malware taraması tamamlandı.\n";
       bool malwareFound = false;

       QListWidgetItem *item;
       for (int i = 0; i < ui->listWidget->count(); i++) {
           item = ui->listWidget->item(i);
           QString filename = item->text();

           QFileInfo fileInfo(filename);

           if (fileInfo.isDir()) {
               QDirIterator dirIterator(filename, QStringList() << "*.*", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
               while (dirIterator.hasNext()) {
                   QString filename = dirIterator.next();
                   QFile file(filename);

                   if (!file.open(QIODevice::ReadOnly)) {
                       qDebug() << "Dosya açılamadı " << filename;
                       messageText += "Dosya " + filename + " açılamadı.\n";
                       continue;
                   }

                   QByteArray data = file.readAll();

                   if (data.contains("Malware")) {
                       qDebug() << "Malware var... ";
                       messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulundu\n";
                       malwareFound = true;

                   } else {
                       qDebug() << "Malware yok... ";
                       messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulunamadı\n";
                   }

                   file.close();
               }
           } else {
               QFile file(filename);
               if (!file.open(QIODevice::ReadOnly)) {
                   qDebug() << "Dosya açılamadı " << filename;
               }
               QByteArray data = file.readAll();

               if (data.contains("Malware")) {
                   qDebug() << "Malware var... ";
                   messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulundu\n";
                   malwareFound = true;
               } else {
                   qDebug() << "Malware yok... ";
                   messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulunamadı\n";
               }

               file.close();
           }
       }

       if (malwareFound) {
           QMessageBox::warning(this, "Malware Taraması Tamamlandı", messageText);
       } else {
           QMessageBox::information(this, "Malware Taraması Tamamlandı", messageText);
       }
}
void MainWindow::on_silButon_clicked(){

//ilk sayfada seçilen dosyaları listwidget öğesinden silme fonksiyonu
    QListWidget *listWidget = ui->listWidget;
    QListWidgetItem *item = listWidget->currentItem();
    int row = listWidget->row(item);
    listWidget->takeItem(row);
    delete item;

}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_sablonOlustur_clicked()
{
 //giriş sayfasına dosyalar eklendikten sonra seçilen dosyalardan şablon oluşturma fonksiyonu.

    auto item_list = ui->listWidget->selectedItems();
    if (item_list.count() == 0) {
        QMessageBox::information(this, tr("Dosya yok"), tr("Lütfen dosya seçin."));
        return;
    }


    bool ok;
    QString templateName = QInputDialog::getText(this, tr("Şablon ismi girin"), tr("Şablon İsmi:"), QLineEdit::Normal, "", &ok);
    if (!ok || templateName.isEmpty()) {
        return;
    }

    bool isWeekly = false;
    bool isMonthly = false;

    QDialog dialog(this);
    dialog.setWindowTitle("Otomatik Tarama İçin Zaman Tipi Seçin");
    dialog.resize(300, 150);
    QVBoxLayout layout(&dialog);

    QCheckBox weeklyCheckBox("Haftalık Tarama");
    QCheckBox monthlyCheckBox("Aylık Tarama");



    layout.addWidget(&weeklyCheckBox);
    layout.addWidget(&monthlyCheckBox);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    layout.addWidget(&buttonBox);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(&weeklyCheckBox, &QCheckBox::stateChanged, [&monthlyCheckBox](int state) {
        if (state == Qt::Checked) {
            monthlyCheckBox.setChecked(false);
        }
    });

    QObject::connect(&monthlyCheckBox, &QCheckBox::stateChanged, [&weeklyCheckBox](int state) {
        if (state == Qt::Checked) {
            weeklyCheckBox.setChecked(false);
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        isWeekly = weeklyCheckBox.isChecked();
        isMonthly = monthlyCheckBox.isChecked();
    }
    int weeklyOrMonthly=0;
    int day=0;
    QString selectedHour="0";
    QString selectedMinute="0";
    QMap<QString, int> daysOfWeek {
        {"Her Pazartesi", 1},
        {"Her Salı", 2},
        {"Her Çarşamba", 3},
        {"Her Perşembe", 4},
        {"Her Cuma", 5},
        {"Her Cumartesi", 6},
        {"Her Pazar", 7}
    };
    if (isWeekly) {
        weeklyOrMonthly=1;
        QComboBox* dayOfWeekComboBox = new QComboBox;
        for (auto it = daysOfWeek.begin(); it != daysOfWeek.end(); it++) {
            dayOfWeekComboBox->addItem(it.key());
        }

         QComboBox* timeComboBox = new QComboBox;
                   for(int i=1;i<24;i++){
                       QString time = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                       timeComboBox->addItem(time);
                   }

         QComboBox* minuteComboBox = new QComboBox;
                   for(int i=0;i<60;i++){
                       QString minute = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                         minuteComboBox->addItem(minute);
                   }

                   QDialog dialog(this);
                   dialog.setWindowTitle("Haftalık Otomatik Tarama");
                   dialog.resize(300, 150);
                   QVBoxLayout* layout = new QVBoxLayout(&dialog);

                   QLabel* label = new QLabel("Gün seçin:");
                   layout->addWidget(label);
                   layout->addWidget(dayOfWeekComboBox);

                   QLabel* label2 = new QLabel("Saat ve dakika seçin:");
                   QHBoxLayout* hbox = new QHBoxLayout;
                   hbox->addWidget(timeComboBox);

                   hbox->addWidget(minuteComboBox);
                   layout->addWidget(label2);
                   layout->addLayout(hbox);

                   QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
                   layout->addWidget(buttonBox);

                   QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                   QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                   if (dialog.exec() == QDialog::Accepted) {
                       QString selectedDay = dayOfWeekComboBox->currentText();
                       QString selectedTime = timeComboBox->currentText() + ":" + minuteComboBox->currentText();
                       selectedHour=timeComboBox->currentText();
                       selectedMinute=minuteComboBox->currentText();
                       qDebug() << selectedDay;
                       qDebug() << selectedTime;
                       day = daysOfWeek[dayOfWeekComboBox->currentText()];
                   }


                   delete label2;
                   delete label;
                   delete timeComboBox;
                   delete minuteComboBox;
                   delete dayOfWeekComboBox;
                   delete buttonBox;
                   delete hbox;
                   delete layout;


    } else if (isMonthly) {
         weeklyOrMonthly=2;

        QComboBox* dayOfMonthComboBox = new QComboBox;

        for(int i=1;i<32;i++){
            dayOfMonthComboBox->addItem(QString::number(i));
        }

         QComboBox* timeComboBox = new QComboBox;
                   for(int i=1;i<24;i++){
                       QString time = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                       timeComboBox->addItem(time);

                   }

         QComboBox* minuteComboBox = new QComboBox;
                   for(int i=0;i<60;i++){
                       QString minute = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                         minuteComboBox->addItem(minute);
                   }

                   QDialog dialog(this);
                   dialog.setWindowTitle("Aylık Otomatik Tarama");
                   dialog.resize(300, 150);
                   QVBoxLayout* layout = new QVBoxLayout(&dialog);

                   QLabel* label = new QLabel("Gün seçin:");
                   layout->addWidget(label);
                   layout->addWidget(dayOfMonthComboBox);
                   QLabel* messageLabel = new QLabel("＊Eğer ay içinde seçtiğiniz gün yoksa, görev ayın son günü çalıştırılır.");
                   messageLabel->setStyleSheet("color: grey");
                   messageLabel->setFixedWidth(300);
                   messageLabel->setWordWrap(true);
                   layout->addWidget(messageLabel);

                   QLabel* label2 = new QLabel("Saat ve dakika seçin:");
                   QHBoxLayout* hbox = new QHBoxLayout;
                   hbox->addWidget(timeComboBox);

                   hbox->addWidget(minuteComboBox);
                   layout->addWidget(label2);
                   layout->addLayout(hbox);

                   QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
                   layout->addWidget(buttonBox);

                   QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                   QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                   if (dialog.exec() == QDialog::Accepted) {
                       QString selectedDay = dayOfMonthComboBox->currentText();
                       QString selectedTime = timeComboBox->currentText() + ":" + minuteComboBox->currentText();
                       selectedHour=timeComboBox->currentText();
                       selectedMinute=minuteComboBox->currentText();
                       qDebug() << selectedDay;
                       qDebug() << selectedTime;
                       day=selectedDay.toInt();
                   }


                   delete label2;
                   delete label;
                   delete messageLabel;
                   delete timeComboBox;
                   delete minuteComboBox;
                   delete dayOfMonthComboBox;
                   delete buttonBox;
                   delete hbox;
                   delete layout;
    } else {
        qDebug() << "Otomatik tarama tercih edilmedi";

    }
    qDebug() << day;
    qDebug() << weeklyOrMonthly;
    qDebug() << selectedHour;
    qDebug() << selectedMinute;

    QSqlQuery query2;
    for (auto a : item_list) {
        QFileInfo dosya(a->text());
        query2.prepare("INSERT INTO file_groups (file_string) VALUES (:file_string)");
        query2.bindValue(":file_string", dosya.fileName());
        if (query2.exec()) {
            qDebug() << "Dosya kaydedildi: " << dosya.fileName();
        } else {
            qDebug() << "Dosya kaydedilemedi: " << dosya.fileName() << query2.lastError().text();
        }
    }


    QSqlDatabase templates_db = QSqlDatabase::database("templates_db");
    QSqlQuery query(templates_db);
    QString timeZoneId = "Europe/Istanbul";
    QTimeZone timeZone = QTimeZone(timeZoneId.toUtf8());


    QLocale::setDefault(QLocale::c());
    qputenv("TZ", timeZone.id());


    #ifdef Q_OS_WIN
        _tzset();
    #else
        tzset();
    #endif


    QDateTime utcDateTime = QDateTime::currentDateTimeUtc();
    QDateTime localDateTime = utcDateTime.toTimeZone(timeZone);



    query.prepare("INSERT INTO saved_templates (template_name, time, file_paths, scan_type, scan_hour, scan_minute, weekly_day, monthly_day) "
                  "VALUES (:template_name, :time, :file_paths, :scan_type, :scan_hour, :scan_minute, :weekly_day, :monthly_day)");
    query.bindValue(":template_name", templateName);
    query.bindValue(":time", localDateTime.toString());
    QStringList file_paths;
    for (auto a : item_list) {
        file_paths.append(a->text());
    }
    query.bindValue(":file_paths", file_paths.join(";"));
    if (query.exec()) {
        qDebug() << "Şablon kaydedildi ";
    } else {
        qDebug() << "Şablon kaydedilemedi:" << query.lastError().text();
    }

    query.bindValue(":scan_type", weeklyOrMonthly);
    query.bindValue(":scan_hour", selectedHour);
    query.bindValue(":scan_minute", selectedMinute);
    query.bindValue(":weekly_day", day);
    query.bindValue(":monthly_day", day);

    if (query.exec()) {
        qDebug() << "Template saved to database";
    } else {
        qDebug() << "Failed to save template to database";
        qDebug() << query.lastError();
        return;
    }

    ui->secilen_gruplar_tableWidget->clearContents();
    ui->secilen_gruplar_tableWidget->setRowCount(0);
    ui->secilen_gruplar_tableWidget->setColumnCount(2);
    QStringList headers;
    headers << "Şablon İsmi" << "Kayıt Zamanı";
    ui->secilen_gruplar_tableWidget->setHorizontalHeaderLabels(headers);

    int row = 0;
    while (query.next()) {
        QString template_name = query.value("template_name").toString();
        QStringList file_paths = query.value("file_paths").toString().split(";");
        ui->secilen_gruplar_tableWidget->insertRow(row);
         ui->secilen_gruplar_tableWidget->setItem(row, 0, new QTableWidgetItem(template_name));
        ui->secilen_gruplar_tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("time").toString()));
       // ui->secilen_gruplar_tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1))); // Satır numaralarını ekleyin

        row++;
    }
    QHeaderView* headerView = ui->secilen_gruplar_tableWidget->horizontalHeader();
      headerView->setSectionResizeMode(QHeaderView::Stretch);

}
void MainWindow::on_kayitliSablonlarButton_clicked() {
//kayıtlı şablonların listelenmesi

    QSqlQuery query(QSqlDatabase::database("templates_db"));
    query.prepare("SELECT template_name, file_paths, time FROM saved_templates");

    if (!query.exec()) {
        qDebug() << "Error retrieving template names:" << query.lastError().text();
        return;
    }

    ui->secilen_gruplar_tableWidget->clearContents();
    ui->secilen_gruplar_tableWidget->setRowCount(0);
    ui->secilen_gruplar_tableWidget->setColumnCount(2);
    QStringList headers;
    headers << "Şablon İsmi" << "Kayıt Zamanı";
    ui->secilen_gruplar_tableWidget->setHorizontalHeaderLabels(headers);

    int row = 0;
    while (query.next()) {
        QString template_name = query.value("template_name").toString();
        QStringList file_paths = query.value("file_paths").toString().split(";");
        ui->secilen_gruplar_tableWidget->insertRow(row);
         ui->secilen_gruplar_tableWidget->setItem(row, 0, new QTableWidgetItem(template_name));
        ui->secilen_gruplar_tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("time").toString()));
       // ui->secilen_gruplar_tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
        row++;
    }
    QHeaderView* headerView = ui->secilen_gruplar_tableWidget->horizontalHeader();
      headerView->setSectionResizeMode(QHeaderView::Stretch);
      ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_geriButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}


void MainWindow::on_detayGosterButton_clicked()
{
 //kayıtlı şablonun içerisinde kayıtlı olan dosya ve klasör yollarının görüntülenmesi
    int selectedRow = ui->secilen_gruplar_tableWidget->currentRow();
        if (selectedRow < 0) {
            return;
        }

        QString template_name = ui->secilen_gruplar_tableWidget->item(selectedRow, 0)->text();
        QSqlQuery query(QSqlDatabase::database("templates_db"));
        query.prepare("SELECT file_paths FROM saved_templates WHERE template_name = :template_name");
        query.bindValue(":template_name", template_name);

        if (!query.exec() || !query.first()) {
            qDebug() << "Error retrieving template details:" << query.lastError().text();
            return;
        }

        QStringList file_paths = query.value("file_paths").toString().split(";");

        QDialog dialog(this);
        QVBoxLayout layout(&dialog);

        QLabel label("Dosya Yolları:");
        layout.addWidget(&label);

        QListWidget *listWidget = new QListWidget(&dialog);
        for (const QString &file_path : file_paths) {
            listWidget->addItem(file_path);
        }
        layout.addWidget(listWidget);

        dialog.setWindowTitle("Şablon Detayları");
        dialog.exec();

}

void MainWindow::on_silButton_3_clicked()
{
    //kayıtlı şablonlar sayfasındaki seçilen şablonun silinmesi
    int selectedRow = ui->secilen_gruplar_tableWidget->currentRow();
        if (selectedRow != -1) {
            QString selectedTemplateName = ui->secilen_gruplar_tableWidget->item(selectedRow, 0)->text();
            QSqlDatabase templates_db = QSqlDatabase::database("templates_db");
            QSqlQuery query(templates_db);
            query.prepare("DELETE FROM saved_templates WHERE template_name = ?");
            query.addBindValue(selectedTemplateName);
            if(query.exec())
            {
                qDebug() << "Template deleted from the database";
                ui->secilen_gruplar_tableWidget->removeRow(selectedRow);
            }
            else
            {
                qDebug() << "Failed to delete the template from the database";
            }
        }
        else {
            qDebug() << "No template selected";
        }


}

//stackwidget (2)


void MainWindow::on_itemSelectButton_clicked()
{
    QString selected = ui->tableWidget->currentItem()->text();
    qDebug() << selected;
}


void MainWindow::listeleKaydedilenDosyalar()
{

        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(3);

        QStringList headers;
        headers << tr("Dosya Yolu") << tr("Dosya Adı") << tr("Kayıt Zamanı");
        ui->tableWidget->setHorizontalHeaderLabels(headers);
        QSqlQuery query1("SELECT group_id, file_string FROM file_groups");
        while (query1.next()) {
            QString group_id = query1.value(0).toString();
            QString file_string = query1.value(1).toString();
            qDebug() << "Group id" << group_id << "File_string " << file_string;


        }
        QSqlQuery query("SELECT file_name, time, file_size FROM saved_files");


        while (query.next()) {

            QString fileName = query.value(0).toString();
            QString time = query.value(1).toString();
            QString ifPathPathName;
            bool isPath = false;
            if (fileName.contains('/'))
            {
                QFileInfo fileInfo(fileName);
                ifPathPathName = fileInfo.fileName();

                isPath = true;

            }


            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);

            QHeaderView *header = ui->tableWidget->horizontalHeader();
            header->setSectionResizeMode(QHeaderView::Fixed);
            for (int i = 0; i < ui->tableWidget->columnCount(); ++i) {
                header->setSectionResizeMode(i, QHeaderView::Fixed);
                header->resizeSection(i, 194);
            }
            if (isPath)
            {

                QTableWidgetItem *pathItem = new QTableWidgetItem(fileName);
                pathItem->setFlags(pathItem->flags() ^ Qt::ItemIsEditable);
                QTableWidgetItem *pathNameItem = new QTableWidgetItem(ifPathPathName);
                pathNameItem->setFlags(pathNameItem->flags() ^ Qt::ItemIsEditable);
                QTableWidgetItem *timeItem = new QTableWidgetItem(time);
                timeItem->setFlags(timeItem->flags() ^ Qt::ItemIsEditable);


                ui->tableWidget->setItem(row, 0, pathItem);
                ui->tableWidget->setItem(row, 1, pathNameItem);
                ui->tableWidget->setItem(row, 2, timeItem);
            }
            else
            {

                QTableWidgetItem *fileNameItem = new QTableWidgetItem(fileName);
                fileNameItem->setFlags(fileNameItem->flags() ^ Qt::ItemIsEditable);
                QTableWidgetItem *timeItem = new QTableWidgetItem(time);
                timeItem->setFlags(timeItem->flags() ^ Qt::ItemIsEditable);

                ui->tableWidget->setItem(row, 0, fileNameItem);
                ui->tableWidget->setItem(row, 1, timeItem);
            }

        }
}

void MainWindow::on_taraButton_clicked() {
    QString messageText = "Tüm dosyaların malware taraması tamamlandı.\n";
    bool malwareFound = false;

    QTableWidgetItem *item;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        item = ui->tableWidget->item(i, 0);
        QString filename = item->text();


        QFileInfo fileInfo(filename);

        if (fileInfo.isDir()) {
            QDirIterator dirIterator(filename, QStringList() << "*.*", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
            while (dirIterator.hasNext()) {
                QString filename = dirIterator.next();
                QFile file(filename);

                if (!file.open(QIODevice::ReadOnly)) {
                    qDebug() << "Dosya açılamadı " << filename;
                    messageText += "Dosya " + filename + " açılamadı.\n";
                    continue;
                }

                QByteArray data = file.readAll();

                if (data.contains("Malware")) {
                    qDebug() << "Malware var... ";
                    messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulundu\n";
                    malwareFound = true;
                } else {
                    qDebug() << "Malware yok... ";
                    messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulunamadı\n";
                }

                file.close();
            }
        } else {
            QFile file(filename);
            if (!file.open(QIODevice::ReadOnly)) {
                qDebug() << "Dosya açılamadı " << filename;
            }
            QByteArray data = file.readAll();

            if (data.contains("Malware")) {
                qDebug() << "Malware var... ";
                messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulundu\n";
                malwareFound = true;
            } else {
                qDebug() << "Malware yok... ";
                messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulunamadı\n";
            }

            file.close();
        }
    }
if (malwareFound) {
    QMessageBox::warning(this, "Malware Taraması Tamamlandı", messageText);
} else {
    QMessageBox::information(this, "Malware Taraması Tamamlandı", messageText);
}
    }




void MainWindow::on_silButton_2_clicked()
{
    int row = ui->tableWidget->currentRow();
        if (row < 0) {
            return;
        }
        QTableWidgetItem *pathNameItem = ui->tableWidget->item(row, 0);
        QTableWidgetItem *fileNameItem = ui->tableWidget->item(row, 1);
        QTableWidgetItem *timeItem = ui->tableWidget->item(row, 2);


        QString fileName = pathNameItem ? pathNameItem->text() : fileNameItem->text();
        QString time = timeItem->text();

        ui->tableWidget->removeRow(row);

        QSqlQuery query;
        query.prepare("SELECT file_id FROM saved_files WHERE file_name = ? AND time = ?");
        query.addBindValue(fileName);
        query.addBindValue(time);
        if (query.exec() && query.next()) {
            int fileId = query.value(0).toInt();

            query.prepare("DELETE FROM saved_files WHERE file_id = ?");
            query.addBindValue(fileId);
            if (query.exec()) {
                qDebug() << "File deleted from database.";
            } else {
                qDebug() << "Error deleting file:" << query.lastError().text();
            }
        } else {
            qDebug() << "Error getting file ID:" << query.lastError().text();
        }
}

void MainWindow::on_geriButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_yeniSablon_clicked()
{

       QFileDialog* _f_dlg = new QFileDialog(this);
       _f_dlg->setFileMode(QFileDialog::Directory);
       _f_dlg->setOption(QFileDialog::DontUseNativeDialog, true);

       QListView *l = _f_dlg->findChild<QListView*>("listView");
       if (l) {
           l->setSelectionMode(QAbstractItemView::MultiSelection);
       }

       QTreeView *t = _f_dlg->findChild<QTreeView*>();
       if (t) {
           t->setSelectionMode(QAbstractItemView::MultiSelection);
       }

       int nMode = _f_dlg->exec();
       QStringList dosyaYollari = _f_dlg->selectedFiles();


       bool ok;
       QString templateName = QInputDialog::getText(this, tr("Şablon ismi girin"), tr("Şablon İsmi:"), QLineEdit::Normal, "", &ok);
       if (!ok || templateName.isEmpty()) {
           return;
       }

       bool isWeekly = false;
       bool isMonthly = false;

       QDialog dialog(this);
       dialog.setWindowTitle("Otomatik Tarama İçin Zaman Tipi Seçin");
       dialog.resize(300, 150);
       QVBoxLayout layout(&dialog);

       QCheckBox weeklyCheckBox("Haftalık Tarama");
       QCheckBox monthlyCheckBox("Aylık Tarama");



       layout.addWidget(&weeklyCheckBox);
       layout.addWidget(&monthlyCheckBox);

       QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
       layout.addWidget(&buttonBox);

       QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
       QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
       QObject::connect(&weeklyCheckBox, &QCheckBox::stateChanged, [&monthlyCheckBox](int state) {
           if (state == Qt::Checked) {
               monthlyCheckBox.setChecked(false);
           }
       });

       QObject::connect(&monthlyCheckBox, &QCheckBox::stateChanged, [&weeklyCheckBox](int state) {
           if (state == Qt::Checked) {
               weeklyCheckBox.setChecked(false);
           }
       });

       if (dialog.exec() == QDialog::Accepted) {
           isWeekly = weeklyCheckBox.isChecked();
           isMonthly = monthlyCheckBox.isChecked();
       }
       int weeklyOrMonthly=0;
       int day=0;
       QString selectedHour="0";
       QString selectedMinute="0";
       QMap<QString, int> daysOfWeek {
           {"Her Pazartesi", 1},
           {"Her Salı", 2},
           {"Her Çarşamba", 3},
           {"Her Perşembe", 4},
           {"Her Cuma", 5},
           {"Her Cumartesi", 6},
           {"Her Pazar", 7}
       };
       if (isWeekly) {
           weeklyOrMonthly=1;
           QComboBox* dayOfWeekComboBox = new QComboBox;
           for (auto it = daysOfWeek.begin(); it != daysOfWeek.end(); it++) {
               dayOfWeekComboBox->addItem(it.key());
           }

            QComboBox* timeComboBox = new QComboBox;
                      for(int i=1;i<24;i++){
                          QString time = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                          timeComboBox->addItem(time);
                      }

            QComboBox* minuteComboBox = new QComboBox;
                      for(int i=0;i<60;i++){
                          QString minute = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                            minuteComboBox->addItem(minute);
                      }

                      QDialog dialog(this);
                      dialog.setWindowTitle("Haftalık Otomatik Tarama");
                      dialog.resize(300, 150);
                      QVBoxLayout* layout = new QVBoxLayout(&dialog);

                      QLabel* label = new QLabel("Gün seçin:");
                      layout->addWidget(label);
                      layout->addWidget(dayOfWeekComboBox);

                      QLabel* label2 = new QLabel("Saat ve dakika seçin:");
                      QHBoxLayout* hbox = new QHBoxLayout;
                      hbox->addWidget(timeComboBox);

                      hbox->addWidget(minuteComboBox);
                      layout->addWidget(label2);
                      layout->addLayout(hbox);

                      QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
                      layout->addWidget(buttonBox);

                      QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                      QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                      if (dialog.exec() == QDialog::Accepted) {
                          QString selectedDay = dayOfWeekComboBox->currentText();
                          QString selectedTime = timeComboBox->currentText() + ":" + minuteComboBox->currentText();
                          selectedHour=timeComboBox->currentText();
                          selectedMinute=minuteComboBox->currentText();
                          qDebug() << selectedDay;
                          qDebug() << selectedTime;
                          day = daysOfWeek[dayOfWeekComboBox->currentText()];
                      }


                      delete label2;
                      delete label;
                      delete timeComboBox;
                      delete minuteComboBox;
                      delete dayOfWeekComboBox;
                      delete buttonBox;
                      delete hbox;
                      delete layout;


       } else if (isMonthly) {
            weeklyOrMonthly=2;

           QComboBox* dayOfMonthComboBox = new QComboBox;

           for(int i=1;i<32;i++){
               dayOfMonthComboBox->addItem(QString::number(i));
           }

            QComboBox* timeComboBox = new QComboBox;
                      for(int i=1;i<24;i++){
                          QString time = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                          timeComboBox->addItem(time);

                      }

            QComboBox* minuteComboBox = new QComboBox;
                      for(int i=0;i<60;i++){
                          QString minute = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                            minuteComboBox->addItem(minute);
                      }

                      QDialog dialog(this);
                      dialog.setWindowTitle("Aylık Otomatik Tarama");
                      dialog.resize(300, 150);
                      QVBoxLayout* layout = new QVBoxLayout(&dialog);

                      QLabel* label = new QLabel("Gün seçin:");
                      layout->addWidget(label);
                      layout->addWidget(dayOfMonthComboBox);
                      QLabel* messageLabel = new QLabel("＊Eğer ay içinde seçtiğiniz gün yoksa, görev ayın son günü çalıştırılır.");
                      messageLabel->setStyleSheet("color: grey");
                      messageLabel->setFixedWidth(300);
                      messageLabel->setWordWrap(true);
                      layout->addWidget(messageLabel);

                      QLabel* label2 = new QLabel("Saat ve dakika seçin:");
                      QHBoxLayout* hbox = new QHBoxLayout;
                      hbox->addWidget(timeComboBox);

                      hbox->addWidget(minuteComboBox);
                      layout->addWidget(label2);
                      layout->addLayout(hbox);

                      QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
                      layout->addWidget(buttonBox);

                      QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                      QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                      if (dialog.exec() == QDialog::Accepted) {
                          QString selectedDay = dayOfMonthComboBox->currentText();
                          QString selectedTime = timeComboBox->currentText() + ":" + minuteComboBox->currentText();
                          selectedHour=timeComboBox->currentText();
                          selectedMinute=minuteComboBox->currentText();
                          qDebug() << selectedDay;
                          qDebug() << selectedTime;
                          day=selectedDay.toInt();
                      }


                      delete label2;
                      delete label;
                      delete messageLabel;
                      delete timeComboBox;
                      delete minuteComboBox;
                      delete dayOfMonthComboBox;
                      delete buttonBox;
                      delete hbox;
                      delete layout;
       } else {
           qDebug() << "Otomatik tarama tercih edilmedi";

       }
       qDebug() << day;
       qDebug() << weeklyOrMonthly;
       qDebug() << selectedHour;
       qDebug() << selectedMinute;
       QSqlDatabase templates_db = QSqlDatabase::database("templates_db");
       QSqlQuery query(templates_db);
       QString fileString = dosyaYollari.join(";");
       QString timeZoneId = "Europe/Istanbul";
       QTimeZone timeZone = QTimeZone(timeZoneId.toUtf8());


       QLocale::setDefault(QLocale::c());
       qputenv("TZ", timeZone.id());


       #ifdef Q_OS_WIN
           _tzset();
       #else
           tzset();
       #endif


       QDateTime utcDateTime = QDateTime::currentDateTimeUtc();
       QDateTime localDateTime = utcDateTime.toTimeZone(timeZone);

       query.prepare("INSERT INTO saved_templates (template_name, time, file_paths, scan_type, scan_hour, scan_minute, weekly_day, monthly_day) "
                     "VALUES (:template_name, :time, :file_paths, :scan_type, :scan_hour, :scan_minute, :weekly_day, :monthly_day)");
       query.bindValue(":template_name", templateName);
       query.bindValue(":time", localDateTime.toString());
       query.bindValue(":file_paths", fileString);
       query.bindValue(":scan_type", weeklyOrMonthly);
       query.bindValue(":scan_hour", selectedHour);
       query.bindValue(":scan_minute", selectedMinute);
       query.bindValue(":weekly_day", day);
       query.bindValue(":monthly_day", day);

       if (query.exec()) {
           qDebug() << "Template saved to database";
       } else {
           qDebug() << "Failed to save template to database";
           qDebug() << query.lastError();
           return;
       }

       ui->secilen_gruplar_tableWidget->clearContents();
       ui->secilen_gruplar_tableWidget->setRowCount(0);
       ui->secilen_gruplar_tableWidget->setColumnCount(2);
       QStringList headers;
       headers << "Şablon İsmi" << "Kayıt Zamanı";
       ui->secilen_gruplar_tableWidget->setHorizontalHeaderLabels(headers);

       int row = 0;
       while (query.next()) {
           QString template_name = query.value("template_name").toString();
           QStringList file_paths = query.value("file_paths").toString().split(";");
           ui->secilen_gruplar_tableWidget->insertRow(row);
            ui->secilen_gruplar_tableWidget->setItem(row, 0, new QTableWidgetItem(template_name));
           ui->secilen_gruplar_tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("time").toString()));
          // ui->secilen_gruplar_tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1))); // Satır numaralarını ekleyin

           row++;
       }
       QHeaderView* headerView = ui->secilen_gruplar_tableWidget->horizontalHeader();
         headerView->setSectionResizeMode(QHeaderView::Stretch);
         on_kayitliSablonlarButton_clicked();

}

void MainWindow::on_sablonDuzenle_clicked()
{
    if (!ui->secilen_gruplar_tableWidget->selectionModel()->hasSelection()) {
            QMessageBox::warning(this, tr("Şablon Seçilmedi"), tr("Lütfen düzenlemek için bir şablon seçin"), QMessageBox::Ok);
            return;
        }
        int selectedRow = ui->secilen_gruplar_tableWidget->selectionModel()->selectedRows().first().row();

        QString templateName = ui->secilen_gruplar_tableWidget->item(selectedRow, 0)->text();
        QString fileString;
        QStringList filePaths;

        QSqlDatabase templates_db = QSqlDatabase::database("templates_db");
        QSqlQuery query(templates_db);

        query.prepare("SELECT file_paths FROM saved_templates WHERE template_name = :template_name");
        query.bindValue(":template_name", templateName);
        if (query.exec()) {
            if (query.next()) {
                fileString = query.value(0).toString();
                filePaths = fileString.split(";", Qt::SkipEmptyParts);
            } else {
                qDebug() << "Failed to retrieve template from database";
                return;
            }
        } else {
            qDebug() << "Failed to query database";
            qDebug() << query.lastError();
            return;
        }

        QDialog dialog(this);
        dialog.setWindowTitle(tr("Şablon Düzenle"));
        QVBoxLayout layout(&dialog);
        QListWidget* fileListWidget = new QListWidget(&dialog);
        fileListWidget->addItems(filePaths);
        layout.addWidget(fileListWidget);
        QHBoxLayout buttonLayout;
        QPushButton otoButton(tr("Otomatik Tarama"), &dialog);
        QPushButton addButton(tr("Dosya ekle"), &dialog);
        QPushButton deleteButton(tr("Sil"), &dialog);
        QPushButton saveButton(tr("Kaydet"), &dialog);

        buttonLayout.addWidget(&otoButton);
        buttonLayout.addWidget(&addButton);
        buttonLayout.addWidget(&deleteButton);
        buttonLayout.addWidget(&saveButton);
        layout.addLayout(&buttonLayout);
        int day=0;
        QString selectedHour="0";
        QString selectedMinute="0";
         int weeklyOrMonthly=0;
        connect(&otoButton, &QPushButton::clicked,[&](){

            QDialog autoScanDialog(this);
            autoScanDialog.setWindowTitle(tr("Otomatik Tarama"));
            autoScanDialog.resize(300, 150);

            QVBoxLayout layout(&autoScanDialog);

            QCheckBox* weeklyCheckBox = new QCheckBox(tr("Haftalık"), &autoScanDialog);
            QCheckBox* monthlyCheckBox = new QCheckBox(tr("Aylık"), &autoScanDialog);
            QLabel* dayLabel = new QLabel(tr("Gün:"), &autoScanDialog);
            QLabel* hourLabel = new QLabel(tr("Saat:"), &autoScanDialog);
            QLabel* minuteLabel = new QLabel(tr("Dakika:"), &autoScanDialog);
            QComboBox* dayComboBox = new QComboBox(&autoScanDialog);
            QComboBox* hourComboBox = new QComboBox(&autoScanDialog);
            QComboBox* minuteComboBox = new QComboBox(&autoScanDialog);
            QPushButton* okButton = new QPushButton(tr("Tamam"), &autoScanDialog);
            connect(okButton, &QPushButton::clicked, &autoScanDialog, &QDialog::accept);

            QMap<QString, int> daysOfWeek {
                {"Her Pazartesi", 1},
                {"Her Salı", 2},
                {"Her Çarşamba", 3},
                {"Her Perşembe", 4},
                {"Her Cuma", 5},
                {"Her Cumartesi", 6},
                {"Her Pazar", 7}
            };
            connect(weeklyCheckBox, &QCheckBox::clicked, [dayComboBox,monthlyCheckBox, weeklyCheckBox, daysOfWeek, &weeklyOrMonthly]() {
                if (weeklyCheckBox->isChecked()) {
                    monthlyCheckBox->setChecked(false);
                    weeklyOrMonthly = 1;
                    dayComboBox->clear();
                    for (auto it = daysOfWeek.begin(); it != daysOfWeek.end(); it++) {
                        dayComboBox->addItem(it.key());
                    }
                }
            });

            connect(monthlyCheckBox, &QCheckBox::clicked, [dayComboBox,weeklyCheckBox, monthlyCheckBox, &weeklyOrMonthly]() {
                if (monthlyCheckBox->isChecked()) {
                    weeklyCheckBox->setChecked(false);
                    weeklyOrMonthly = 2;
                    dayComboBox->clear();
                    for (int i = 1; i <= 31; i++) {
                        dayComboBox->addItem(QString::number(i));
                    }
                }
            });

            if (weeklyCheckBox->isChecked()) {
                monthlyCheckBox->setChecked(false);
                weeklyOrMonthly = 1;
            }
            else if (monthlyCheckBox->isChecked()) {
                weeklyCheckBox->setChecked(false);
                weeklyOrMonthly = 2;
            }
            else {
                weeklyOrMonthly = 0;
            }


            for (int i = 0; i < 24; i++) {

                    QString time = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                    hourComboBox->addItem(time);

            }
            for (int i = 0; i < 60; i++) {
                QString minute = QString("%1").arg(i, 2, 10, QLatin1Char('0'));
                  minuteComboBox->addItem(minute);

            }
            QHBoxLayout* timeLayout = new QHBoxLayout();
            timeLayout->addWidget(hourComboBox);
            timeLayout->addWidget(minuteComboBox);

            QHBoxLayout* timeLabel = new QHBoxLayout();
            timeLabel->addWidget(hourLabel);
            timeLabel->addWidget(minuteLabel);

            QWidget* timeLabelWidget = new QWidget(&autoScanDialog);
            timeLabelWidget->setLayout(timeLabel);

            layout.addWidget(weeklyCheckBox);
            layout.addWidget(monthlyCheckBox);
            layout.addWidget(dayLabel);
            layout.addWidget(dayComboBox);
            layout.addWidget(timeLabelWidget);
            layout.addLayout(timeLayout);
            layout.addWidget(okButton);
            if (autoScanDialog.exec()) {
                QString selectedDay = dayComboBox->currentText();
                QString selectedTime = hourComboBox->currentText() + ":" + minuteComboBox->currentText();
                selectedHour = hourComboBox->currentText();
                selectedMinute = minuteComboBox->currentText();
                qDebug() << selectedDay;
                qDebug() << selectedTime;
                day = daysOfWeek[selectedDay];
                qDebug() << day;
                qDebug() << weeklyOrMonthly;
            }

         });
        connect(&addButton, &QPushButton::clicked, [&](){
            QFileDialog* fileDialog = new QFileDialog(this);
                   fileDialog->setFileMode(QFileDialog::Directory);
                   fileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
                   QListView *l = fileDialog->findChild<QListView*>("listView");
                   if (l) {
                       l->setSelectionMode(QAbstractItemView::MultiSelection);
                   }
                   QTreeView *t = fileDialog->findChild<QTreeView*>();
                   if (t) {
                       t->setSelectionMode(QAbstractItemView::MultiSelection);
                   }

                   if (fileDialog->exec()) {
                       QStringList newFilePaths = fileDialog->selectedFiles();
                       foreach (const QString& newFilePath, newFilePaths) {
                           fileListWidget->addItem(newFilePath);
                           filePaths.append(newFilePath);
                       }
                   }
        });
        QList<QString> deletedFilePaths;
       connect(&deleteButton, &QPushButton::clicked, [&](){

           QList<QListWidgetItem*> selectedItems = fileListWidget->selectedItems();
             if (selectedItems.isEmpty()) {
                 QMessageBox::warning(this, tr("Dosya Seçilmedi"), tr("Lütfen silmek için bir dosya seçin"), QMessageBox::Ok);
                 return;
             }
             QString selectedFilePath = selectedItems.first()->text();

             delete selectedItems.first();
             filePaths.removeAll(selectedFilePath);
             fileString = filePaths.join(";");
       });
       connect(&saveButton, &QPushButton::clicked, [&](){
           QString newFileString = filePaths.join(";");
              QSqlQuery query(templates_db);
              query.prepare("UPDATE saved_templates SET file_paths = :file_paths, scan_type = :scan_type, scan_hour = :scan_hour, scan_minute = :scan_minute, weekly_day = :weekly_day, monthly_day = :monthly_day WHERE template_name = :template_name");
                query.bindValue(":file_paths", newFileString);
                query.bindValue(":scan_type", weeklyOrMonthly);
                query.bindValue(":scan_hour", selectedHour);
                query.bindValue(":scan_minute", selectedMinute);
                query.bindValue(":weekly_day", day);
                query.bindValue(":monthly_day", day);
                query.bindValue(":template_name", templateName);

              foreach (const QString& deletedFilePath, deletedFilePaths) {
                     QSqlQuery deleteQuery(templates_db);
                     deleteQuery.prepare("UPDATE saved_templates SET file_paths = REPLACE(file_paths, :deleted_file_path, '') WHERE template_name = :template_name");
                     deleteQuery.bindValue(":deleted_file_path", deletedFilePath);
                     deleteQuery.bindValue(":template_name", templateName);
                     if (!deleteQuery.exec()) {
                         qDebug() << "Failed to delete file path from database";
                         qDebug() << deleteQuery.lastError();
                     }
                 }


              if (query.exec()) {
                  QMessageBox::information(this, tr("Başarılı"), tr("Şablon başarıyla güncellendi"), QMessageBox::Ok);
                  dialog.close();
              } else {
                  qDebug() << "Failed to update database";
                  qDebug() << query.lastError();
                  return;
              }
        });
       connect(&dialog, &QDialog::rejected, [&](){
           foreach (const QString& newFilePath, filePaths) {
               QList<QListWidgetItem*> items = fileListWidget->findItems(newFilePath, Qt::MatchExactly);
               if (!items.isEmpty()) {
                   QListWidgetItem* item = items.at(0);
                   int row = fileListWidget->row(item);
                   fileListWidget->takeItem(row);
                   filePaths.removeOne(newFilePath);
               }
           }
       });
       dialog.exec();

}


void MainWindow::on_sablonTara_clicked()
{
    //seçilen şablonun taranması
    int selectedRow = ui->secilen_gruplar_tableWidget->selectionModel()->selectedRows().first().row();
       QString templateName = ui->secilen_gruplar_tableWidget->item(selectedRow, 0)->text();

       QSqlDatabase templates_db = QSqlDatabase::database("templates_db");
       QSqlQuery query(templates_db);
       query.prepare("SELECT file_paths FROM saved_templates WHERE template_name = :template_name");
       query.bindValue(":template_name", templateName);
       if (!query.exec()) {
           qDebug() << "Failed to query database";
           qDebug() << query.lastError();
           return;
       }

       QString messageText;
       bool malwareFound = false;

       QDialog dialog(this);
       dialog.setWindowTitle("Tarama İşlemi");
       dialog.setFixedSize(300, 100);
       QPushButton stopButton("Taramayı Durdur", &dialog);
       stopButton.setGeometry(10, 40, 120, 30);
       connect(&stopButton, &QPushButton::clicked, &dialog, &QDialog::accept);


       dialog.show();

       while (query.next()) {
           QString fileString = query.value(0).toString();
           QStringList filePaths = fileString.split(";", Qt::SkipEmptyParts);

           for (QString& filePath : filePaths) {
               QFileInfo fileInfo(filePath);

               if (fileInfo.isDir()) {
                   QDirIterator dirIterator(filePath, QStringList() << "*.*", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                   while (dirIterator.hasNext()) {
                       QString filename = dirIterator.next();

                       QApplication::processEvents();

                       QFile file(filename);

                       if (!file.open(QIODevice::ReadOnly)) {
                           qDebug() << "Failed to open file " << filename;
                           messageText += "Dosya " + filename + " açılamadı.\n";
                           continue;
                       }

                       QByteArray data = file.readAll();

                       if (data.contains("Malware")) {
                           qDebug() << "Malware detected in file " << filename;
                           messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulundu\n";
                           malwareFound = true;

                       } else {
                           qDebug() << "No malware found in file " << filename;
                           messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulunamadı\n";
                       }

                       file.close();
                       if (dialog.result() == QDialog::Accepted) {
                           QMessageBox::information(this, "Tarama Durduruldu", "Tarama durduruldu. Henüz tarama tamamlanmadı.");
                           return;
                       }
                   }
               } else {
                   QFile file(filePath);

                   if (!file.open(QIODevice::ReadOnly)) {
                       qDebug() << "Failed to open file " << filePath;
                   }

                    QByteArray data = file.readAll();

                    if (data.contains("Malware")) {
                        qDebug() << "Malware detected in file " << filePath;
                        messageText += "Dosya " + filePath + " için malware taraması tamamlandı. \nSonuç: Malware bulundu\n";
                        malwareFound = true;
                    } else {
                        qDebug() << "No malware found in file " << filePath;
                        messageText += "Dosya " + filePath + " için malware taraması tamamlandı. \nSonuç: Malware bulunamadı\n";
                    }

                    file.close();
                }

            }
        }
       if (malwareFound) {
           QMessageBox::warning(this, "Malware Taraması Tamamlandı", messageText);
       } else {
           QMessageBox::information(this, "Malware Taraması Tamamlandı", messageText);
       }
}

void MainWindow::autoScan() {

    QSqlDatabase templates_db = QSqlDatabase::database("templates_db");
       QSqlQuery query(templates_db);
       if (!templates_db.isOpen())
       {
           if (templates_db.driver())
           {
               qDebug() << "Driver yok"  << templates_db.driverName();
           }
           qDebug() << "Failed to templates_db database";
       }

       query.prepare("SELECT file_paths, scan_type, weekly_day, monthly_day, scan_hour, scan_minute FROM saved_templates WHERE scan_type != 0");
       if (!query.exec()) {
           qDebug() << "Failed to query database";
           qDebug() << query.lastError();
           return;
       }
       while (query.next()) {
           QString fileString = query.value(0).toString();
           int scanType = query.value(1).toInt();
           int weeklyDay = query.value(2).toInt();
           int monthlyDay = query.value(3).toInt();
           int scanHour = query.value(4).toInt();
           int scanMinute = query.value(5).toInt();

           QTime currentTime = QTime::currentTime();

           QDate currentDate = QDate::currentDate();
           int currentMonth = currentDate.month();
           int currentYear = currentDate.year();

           QDate selectedDate = QDate(currentYear, currentMonth, monthlyDay);
           if (selectedDate.daysInMonth() < monthlyDay) {
               monthlyDay = selectedDate.daysInMonth();;
           }

           if (scanType == 1 && weeklyDay != QDate::currentDate().dayOfWeek()) {
               continue;
           } else if (scanType == 1 && weeklyDay == QDate::currentDate().dayOfWeek() && (currentTime.hour() != scanHour || currentTime.minute() != scanMinute)) {
               continue;
           }

           if (scanType == 2 && monthlyDay != QDate::currentDate().day()) {
               continue;
           } else if (scanType == 2 && monthlyDay == QDate::currentDate().day() && (currentTime.hour() != scanHour || currentTime.minute() != scanMinute)) {
               continue;
           }

        QStringList filePaths = fileString.split(";", Qt::SkipEmptyParts);
        bool malwareFound = false;
        QString messageText;
        for (QString& filePath : filePaths) {
            QFileInfo fileInfo(filePath);



            if (fileInfo.isDir()) {
                QDirIterator dirIterator(filePath, QStringList() << "*.*", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                while (dirIterator.hasNext()) {
                    QString filename = dirIterator.next();

                    QApplication::processEvents();

                    QFile file(filename);

                    if (!file.open(QIODevice::ReadOnly)) {
                        qDebug() << "Failed to open file " << filename;
                        continue;
                    }

                    QByteArray data = file.readAll();

                    if (data.contains("Malware")) {
                        qDebug() << "Malware detected in file " << filename;
                         messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulundu\n";
                         malwareFound = true;
                    } else {
                        qDebug() << "No malware found in file " << filename;
                         messageText += "Dosya " + filename + " için malware taraması tamamlandı. \nSonuç: Malware bulunamadı\n";
                    }

                    file.close();
                }
            } else {
                QFile file(filePath);

                if (!file.open(QIODevice::ReadOnly)) {
                    qDebug() << "Failed to open file " << filePath;
                }

                QByteArray data = file.readAll();

                if (data.contains("Malware")) {
                    qDebug() << "Malware detected in file " << filePath;
                     messageText += "Dosya " + filePath + " için malware taraması tamamlandı. \nSonuç: Malware bulundu\n";
                     malwareFound = true;
                } else {
                    qDebug() << "No malware found in file " << filePath;
                     messageText += "Dosya " + filePath + " için malware taraması tamamlandı. \nSonuç: Malware bulunamadı\n";
                }
                file.close();
            }
        }
        if (malwareFound) {
            QMessageBox::warning(this, "Malware Taraması Tamamlandı", messageText);
        } else {
            QMessageBox::information(this, "Malware Taraması Tamamlandı", messageText);
        }
    }
}
