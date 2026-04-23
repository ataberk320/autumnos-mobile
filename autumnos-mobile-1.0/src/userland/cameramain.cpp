#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QCamera>
#include <QVideoSink>
#include <QVideoFrame>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <QImageWriter>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include "camera.h"

class CameraApp : public QWidget {
    Q_OBJECT
public:
    CameraApp(QWidget *parent = nullptr);
    ~CameraApp();

private slots:
    void processFrame(const QVideoFrame &frame);
    void takePhoto();
    void updateSharedMemory();

private:
    Ui::Form *ui;
    QCamera *camera;
    QMediaCaptureSession captureSession;
    QVideoSink* videoSink;
    uchar* shm_ptr;
    int shm_fd;
    QTimer *shmTimer;
    QImage lastFrame;
};

CameraApp::CameraApp(QWidget *parent) : QWidget(parent), ui(new Ui::Form), shm_ptr(nullptr), shm_fd(-1) {
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    
    this->setFixedSize(480, 768); 

    shm_fd = shm_open("/autumn_app", O_RDWR | O_CREAT, 0666);
    if (shm_fd >= 0) {
        ftruncate(shm_fd, 480 * 768 * 4);
        shm_ptr = (uchar*)mmap(0, 480 * 768 * 4, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    }

    camera = new QCamera(QMediaDevices::defaultVideoInput(), this);
    videoSink = new QVideoSink(this);
    captureSession.setCamera(camera);
    captureSession.setVideoSink(videoSink);

    connect(videoSink, &QVideoSink::videoFrameChanged, this, &CameraApp::processFrame);
    connect(ui->pushButton, &QPushButton::clicked, this, &CameraApp::takePhoto);

    shmTimer = new QTimer(this);
    connect(shmTimer, &QTimer::timeout, this, &CameraApp::updateSharedMemory);
    shmTimer->start(200); 

    ui->label->setScaledContents(true);
    camera->start();
}

void CameraApp::updateSharedMemory() {
    if (shm_ptr && shm_ptr != MAP_FAILED) {
        QImage screenGrab = this->grab().toImage().convertToFormat(QImage::Format_ARGB32);

        if (!screenGrab.isNull()) {
            uint sizeToCopy = qMin((uint)screenGrab.sizeInBytes(), (uint)(480 * 768 * 4));
            memcpy(shm_ptr, screenGrab.bits(), sizeToCopy);
        }
    }
}

void CameraApp::processFrame(const QVideoFrame &frame) {
    if (!frame.isValid()) return;

    QVideoFrame cloneFrame = frame;
    if (cloneFrame.map(QVideoFrame::ReadOnly)) {
        QImage img = cloneFrame.toImage().convertToFormat(QImage::Format_ARGB32);
        if (!img.isNull()) {
            lastFrame = img;
            ui->label->setPixmap(QPixmap::fromImage(lastFrame).scaled(
                ui->label->size(), 
                Qt::KeepAspectRatioByExpanding, 
                Qt::FastTransformation
            ));
        }
        cloneFrame.unmap();
    }
}

void CameraApp::takePhoto() {
    QString path = "photos";
    QDir dir(path);
    if (!dir.exists()) dir.mkpath(".");

    QString fileName = QString("%1/Autumn_%2.jpg")
                       .arg(path)
                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    if (lastFrame.isNull()) return;

    QImageWriter writer(fileName);
    writer.setFormat("jpg");
    writer.setQuality(95);

    if (writer.write(lastFrame)) {
        this->setStyleSheet("background-color: white;");
        QTimer::singleShot(100, [this](){ this->setStyleSheet("background-color: transparent;"); });
    }
}

CameraApp::~CameraApp() {
    shmTimer->stop();
    if (shm_ptr && shm_ptr != MAP_FAILED) munmap(shm_ptr, 480 * 768 * 4);
    if (shm_fd >= 0) ::close(shm_fd);
    delete ui;
}

#include "cameramain.moc"
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    CameraApp w;
    QPixmap CaptureIcon("/usr/share/camera.png");
    QPixmap GalleryShortcut("/usr/share/gallery.png");
    QPushButton *CaptureButton = w.findChild<QPushButton*>("pushButton");
    if (CaptureButton) {
	CaptureButton->setIcon(QIcon(CaptureIcon));
    }
    w.show();
    return a.exec();
}
