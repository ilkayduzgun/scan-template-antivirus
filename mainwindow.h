#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QListWidgetItem>
#include <QtSql>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QThread>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QFileInfo m_secilenDosya; // m_secilenDosya değişkeni burada tanımlandı
    QList<QFileInfo> m_tarananDosyalar;

public slots:
    void dosyaSec();
    void autoScan();
    void listeleTarananDosyalar(const QStringList &dosyaListesi);
    void listeleKaydedilenDosyalar();
    void on_tarananDosyalarListWidget_itemClicked(QListWidgetItem *item);
    void on_kaydetButton_clicked();    
    void on_taramaButton_clicked();
    void on_itemSelected(QString item);
    void on_silButon_clicked();


private slots:
    void on_sablonOlustur_clicked();
    void on_geriButton_clicked();
    void on_kayitliSablonlarButton_clicked();
    void on_listeleButton_clicked();
    void on_taraButton_clicked();
    void on_silButton_2_clicked();
    void on_itemSelectButton_clicked();
    void on_geriButton_2_clicked();

    void on_silButton_3_clicked();
    void on_detayGosterButton_clicked();

    void on_yeniSablon_clicked();
    void on_sablonDuzenle_clicked();
    void on_sablonTara_clicked();

};






#endif // MAINWINDOW_H
