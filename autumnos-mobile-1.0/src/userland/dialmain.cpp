#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QSerialPort>
#include <QDebug>
#include "dialer.h"

class DialerWindow: public QWidget, private Ui::Form
{
    Q_OBJECT
    QSerialPort *sim800;
    QTimer *colorTimer;
    int grayVal = 80;
    bool increasing = true;

public:
    DialerWindow(QString phoneNumber, QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUi(this);
        sim800 = new QSerialPort(this);
        sim800->setPortName("/dev/ttyV1");
        sim800->setBaudRate(QSerialPort::Baud9600);
        label_2->setText(phoneNumber);
        colorTimer = new QTimer(this);
        connect(colorTimer, &QTimer::timeout, [this]() {
            if (increasing) grayVal += 5; else grayVal -= 5;
            if (grayVal >= 200) increasing = false;
            if (grayVal <= 80) increasing = true;

            QString style = QString("color: rgb(%1, %1, %1); font-weight: bold;").arg(grayVal);
            label_2->setStyleSheet(style);
        });
        colorTimer->start(50);
        connect(sim800, &QSerialPort::readyRead, [this]() {
            QByteArray response = sim800->readAll();
            qDebug() << "Response:" << response;

            if (response.contains("NO CARRIER") || response.contains("BUSY") || response.contains("NO ANSWER")) {
                terminateDialer();
            }
        });

        connect(pushButton, &QPushButton::clicked, [this]() {
            terminateDialer();
        });
    }
private:
    void terminateDialer() {
        if(colorTimer) colorTimer->stop();
        if (sim800->isOpen()) sim800->close();
        qApp->quit();
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString targetNumber = (argc > 1) ? argv[1] : "...";
    DialerWindow w(targetNumber);
    w.show();
    return app.exec();
}

#include "dialmain.moc"
