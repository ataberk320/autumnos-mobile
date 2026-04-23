#include <QApplication>
#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QMap>
#include <QPixmap>
#include <QImage>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <signal.h>
#include "taskm.h"

class MainWindow : public QWidget {
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void on_pushButton_clicked();
    void updateProcessList();
    void sendFrame();

private:
    Ui::Form *ui;
    QMap<QString, QString> friendlyNames;
    uchar* shm_ptr;
    int shm_fd;
};

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), ui(new Ui::Form), shm_ptr(nullptr), shm_fd(-1) {
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFixedSize(480, 768);

    shm_fd = shm_open("/autumn_app", O_RDWR | O_CREAT, 0666);
    if (shm_fd >= 0) {
        ftruncate(shm_fd, 480 * 768 * 4);
        shm_ptr = (uchar*)mmap(0, 480 * 768 * 4, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    } else {
        system("echo [AUTUMNOS] Opening SHM for AutixSurf failed! > /dev/kmsg");
    }

    friendlyNames = {
        {"sysui", "Autumn System User Interface"},
        {"watchdog.sh", "AutumnOS Software Watchdog"},
        {"AutumnCore", "AutumnOS Core"},
        {"AutixG2DService", "AutumnOS G2D Enabler"},
        {"AutixSurfDetector", "AutumnOS AutixSurf Display Server"},
        {"ASComposer", "AutixSurf Window Composer"}
    };

    QTimer *syncTimer = new QTimer(this);
    connect(syncTimer, &QTimer::timeout, this, &MainWindow::sendFrame);
    syncTimer->start(33);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateProcessList);
    timer->start(2000);

    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_clicked);

    updateProcessList();
}

void MainWindow::sendFrame() {
    if (!shm_ptr || shm_ptr == MAP_FAILED) return;

    QPixmap pixmap = this->grab(this->rect());
    QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);

    if (!img.isNull()) {
        memcpy(shm_ptr, img.bits(), 480 * 768 * 4);
    }
}

void MainWindow::on_pushButton_clicked() {
    int currentRow = ui->tableWidget->currentRow();
    if (currentRow < 0) return;

    QString pidStr = ui->tableWidget->item(currentRow, 1)->text();
    pid_t pid = pidStr.toInt();

    if (kill(pid, SIGKILL) == 0) {
        ui->tableWidget->removeRow(currentRow);
    }
}

void MainWindow::updateProcessList() {
	ui->tableWidget->setRowCount(0);
	QDir procDir("/proc");
	procDir.setNameFilters(QStringList() << "[0-9]*");
	procDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

	for (const QString &pidStr : procDir.entryList()) {
		QFile commFile(QString("/proc/%1/comm").arg(pidStr));
		if (commFile.open(QIODevice::ReadOnly)) {
			QString rawBinary = commFile.readAll().trimmed();
			commFile.close();
			QString displayName = friendlyNames.value(rawBinary, rawBinary);
			
			int row = ui->tableWidget->rowCount();
			ui->tableWidget->insertRow(row);
			
			ui->tableWidget->setItem(row, 0, new QTableWidgetItem(rawBinary));
			ui->tableWidget->setItem(row, 1, new QTableWidgetItem(pidStr));
			ui->tableWidget->setItem(row, 2, new QTableWidgetItem(displayName));
		}
	}
}

MainWindow::~MainWindow() {
    if (shm_ptr && shm_ptr != MAP_FAILED) {
        munmap(shm_ptr, 480 * 768 * 4);
    }
    if (shm_fd >= 0) {
        ::close(shm_fd);
    }
    delete ui;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
