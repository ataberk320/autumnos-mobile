#include <QApplication>
#include <QMainWindow>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QSerialPort>
#include <QIcon>
#include <QTextEdit>
#include "messages.h"
#include <QFontDatabase>


class SerialManager;
class DatabaseManager;

class MainWindow : public QMainWindow {

public:
    MainWindow(QWidget *parent = nullptr);

    void on_pushButton_3_clicked();
    void on_pushButton_2_clicked();
    void onNewMessageArrived(QString num, QString msg);

private:
    Ui::Form *ui;
    QSerialPort *serialPort;
    SerialManager *serialManager;
    DatabaseManager *dbManager;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Form)
{
    ui->setupUi(this);

    serialPort = new QSerialPort(this);
    serialPort->setBaudRate(QSerialPort::Baud115200);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    serialPort->setPortName("/dev/ttyS1");

    ui->frame_3->hide();
    ui->frame_3->setEnabled(false);


    connect(ui->pushButton_3, &QPushButton::clicked, [this]() {
        on_pushButton_3_clicked();
    });

    connect(ui->pushButton_2, &QPushButton::clicked, [this]() {
        on_pushButton_2_clicked();
    });
}

void MainWindow::on_pushButton_3_clicked() {
    ui->frame_3->show();
    ui->frame_3->setEnabled(true);
    ui->frame_3->raise();
    ui->textEdit->setFocus();
}

void MainWindow::on_pushButton_2_clicked() {
    QString num = ui->textEdit->toPlainText();
    QString msg = ui->textEdit_2->toPlainText();

    if(num.isEmpty() || msg.isEmpty()) return;

    QTreeWidgetItem *root1 = ui->treeWidget->topLevelItem(0);

    if (root1) {
        QTreeWidgetItem *sentFolder = root1->child(4);
        if (sentFolder) {
            QTreeWidgetItem *senderItem = new QTreeWidgetItem(sentFolder);
            senderItem->setText(0, "Ben (" + num + ")");
            senderItem->setIcon(0, QIcon("/usr/share/pixmap/avatar.png"));

            QTreeWidgetItem *messageItem = new QTreeWidgetItem(senderItem);
            messageItem->setText(0, msg);

            root1->setExpanded(true);
            sentFolder->setExpanded(true);
            senderItem->setExpanded(true);
            ui->treeWidget->setCurrentItem(messageItem);
        }
    }

    ui->textEdit_2->clear();
    ui->frame_3->hide();
    ui->frame_3->setEnabled(false);
}

void MainWindow::onNewMessageArrived(QString num, QString msg) {
    QTreeWidgetItem *root1 = ui->treeWidget->topLevelItem(0);

    if (root1) {
        QTreeWidgetItem *unreadFolder = root1->child(0);
        if (unreadFolder) {
            QTreeWidgetItem *senderItem = new QTreeWidgetItem(unreadFolder);
            senderItem->setText(0, num);
            senderItem->setIcon(0, QIcon("/usr/share/pixmap/avatar.png"));

            QTreeWidgetItem *messageItem = new QTreeWidgetItem(senderItem);
            messageItem->setText(0, msg);

            root1->setExpanded(true);
            unreadFolder->setExpanded(true);
            senderItem->setExpanded(true);
            ui->treeWidget->setCurrentItem(messageItem);
        }
    }
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    int fontId = QFontDatabase::addApplicationFont("/usr/share/fonts/DroidSans-Bold.ttf");

    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont autumnFont(family);
        a.setFont(autumnFont);
    }
    MainWindow w;
    w.show();
    return a.exec();
}