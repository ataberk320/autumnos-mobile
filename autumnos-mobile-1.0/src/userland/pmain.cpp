#include <QApplication>
#include <QSerialPort>
#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "form.h"

class MainWindow : public QWidget, private Ui::Form
{
    QSerialPort *sim800;
    uchar* shm_ptr;
    QTimer *syncTimer;
    int shm_fd;

public:
    MainWindow(QWidget *parent = nullptr) : QWidget(parent)
    {
        setupUi(this);
	this->setWindowFlags(Qt::FramelessWindowHint);
        this->setFixedSize(480, 768);
	shm_fd = shm_open("/autumn_app", O_RDWR | O_CREAT, 0666);
	if (shm_fd < 0) {
		system("echo [AUTUMNOS] Opening SHM for AutixSurf is failed!  > /dev/kmsg");
	}
	else {
		shm_ptr = (uchar*)mmap(0, 480 * 768 * 4, PROT_WRITE, MAP_SHARED, shm_fd, 0);
	}

	syncTimer = new QTimer(this);
        connect(syncTimer, &QTimer::timeout, this, &MainWindow::sendFrame);
        syncTimer->start(33);
        sim800 = new QSerialPort(this);
        sim800->setPortName("/tmp/ttyS1");
        sim800->setBaudRate(QSerialPort::Baud9600);
        if (!sim800->open(QIODevice::ReadWrite)) {
            qDebug() << "Serial port is not actived!";
        }
        connect(pushButton_14, &QPushButton::clicked, [this]() {
            QString text = plainTextEdit->toPlainText();
            if (!text.isEmpty()) {
                text.chop(1);
                plainTextEdit->setPlainText(text);
                QTextCursor cursor = plainTextEdit->textCursor();
                cursor.movePosition(QTextCursor::End);
                plainTextEdit->setTextCursor(cursor);
            }
        });

        connect(pushButton, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("1");});
        connect(pushButton_2, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("2");});
        connect(pushButton_3, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("3");});
        connect(pushButton_4, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("4");});
        connect(pushButton_5, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("5");});
        connect(pushButton_6, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("6");});
        connect(pushButton_7, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("7");});
        connect(pushButton_8, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("8");});
        connect(pushButton_9, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("9");});
        connect(pushButton_12, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("*");});
        connect(pushButton_10, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("0");});
        connect(pushButton_11, &QPushButton::clicked, [this]() {plainTextEdit->insertPlainText("#");});

        connect(pushButton_13, &QPushButton::clicked, [this]() {
            if(sim800->isOpen()) {
                QProcess *dialer = new QProcess(this);
                QString number = plainTextEdit->toPlainText().trimmed();
                QString atCommand = "ATD"+number+";\r\n";
                sim800->write(atCommand.toUtf8());
                dialer->start("./com.autumnos.dialer.atm");
            }
        });
    }
        void sendFrame() {
	if (shm_ptr == MAP_FAILED || !shm_ptr) return;
	QPixmap pixmap = this->grab(this->rect());
	QImage img = pixmap.toImage().scaled(480, 768, Qt::IgnoreAspectRatio).convertToFormat(QImage::Format_ARGB32);

        if (!img.isNull()) {
           	 memcpy(shm_ptr, img.bits(), 480 * 768 * 4);
       	   }
    	}

   	~MainWindow() {
        if (shm_ptr && shm_ptr != MAP_FAILED) { 
		munmap(shm_ptr, 480 * 768 * 4);
	}
        if (shm_fd >= 0) ::close(shm_fd);
     };
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
