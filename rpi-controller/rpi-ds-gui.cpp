#include "daisyComsManager.h"
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QString>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Daisy Seed Controller");
    window.resize(800, 480);
    window.setStyleSheet("background-color: #1e1e1e; color: #ffffff;");

    QVBoxLayout *mainLayout = new QVBoxLayout(&window);
    mainLayout->setContentsMargins(20, 15, 20, 15);
    mainLayout->setSpacing(12);

    // Top Bar with Title and Window Controls
    QHBoxLayout *topBarLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("Daisy Seed I2C Controller", &window);
    QFont titleFont("Arial", 18, QFont::Bold);
    titleLabel->setFont(titleFont);

    QPushButton *fullscreenBtn = new QPushButton("Fullscreen", &window);
    fullscreenBtn->setStyleSheet(
        "QPushButton { background-color: #333333; color: #ffffff; border: 1px solid #555555; "
        "border-radius: 6px; padding: 6px 14px; font-size: 13px; } "
        "QPushButton:hover { background-color: #444444; }"
    );
    QObject::connect(fullscreenBtn, &QPushButton::clicked, [&window, fullscreenBtn]() {
        if (window.isFullScreen()) {
            window.showNormal();
            fullscreenBtn->setText("Fullscreen");
        } else {
            window.showFullScreen();
            fullscreenBtn->setText("Windowed");
        }
    });

    QPushButton *exitBtn = new QPushButton("Exit", &window);
    exitBtn->setStyleSheet(
        "QPushButton { background-color: #7b1fa2; color: #ffffff; border-radius: 6px; "
        "padding: 6px 14px; font-size: 13px; font-weight: bold; } "
        "QPushButton:hover { background-color: #9c27b0; }"
    );
    QObject::connect(exitBtn, &QPushButton::clicked, &app, &QApplication::quit);

    topBarLayout->addWidget(titleLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(fullscreenBtn);
    topBarLayout->addWidget(exitBtn);
    mainLayout->addLayout(topBarLayout);

    // Slider Value Display
    QLabel *valueLabel = new QLabel("Slider Value: 0", &window);
    QFont valFont("Arial", 18, QFont::DemiBold);
    valueLabel->setFont(valFont);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet("color: #4fc3f7;");
    mainLayout->addWidget(valueLabel);

    // Byte Breakdown Display
    QLabel *bytesLabel = new QLabel("Bytes: [Byte 0 (Val):   0 (0x00)]  [Byte 1 (Effect): 1 (0x01)]  [Byte 2 (Param): 0 (0x00)]", &window);
    QFont byteFont("Monospace", 13);
    bytesLabel->setFont(byteFont);
    bytesLabel->setAlignment(Qt::AlignCenter);
    bytesLabel->setStyleSheet("background-color: #2a2a2a; border-radius: 6px; padding: 8px; color: #e0e0e0;");
    mainLayout->addWidget(bytesLabel);

    // I2C Status Display
    QLabel *statusLabel = new QLabel("I2C Status: Ready", &window);
    QFont statusFont("Arial", 13);
    statusLabel->setFont(statusFont);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: #81c784;");
    mainLayout->addWidget(statusLabel);

    // Touch-friendly Slider sized to comfortably fit on 800x480
    QSlider *distortionSlider = new QSlider(Qt::Horizontal, &window);
    distortionSlider->setRange(0, 100);
    distortionSlider->setValue(0);
    distortionSlider->setMinimumHeight(70);
    distortionSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    distortionSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "    height: 22px;"
        "    background: #333333;"
        "    border-radius: 11px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: #0288d1;"
        "    border-radius: 11px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: #ffffff;"
        "    border: 3px solid #0288d1;"
        "    width: 52px;"
        "    margin-top: -15px;"
        "    margin-bottom: -15px;"
        "    border-radius: 26px;"
        "}"
    );
    mainLayout->addWidget(distortionSlider);
    mainLayout->addStretch();

    // Initialize Communications Manager
    daisyComsManager comsManager(1, 0x40);
    bool i2cInitOk = comsManager.initialize();
    if (!i2cInitOk) {
        statusLabel->setText("I2C Init FAILED! (Check /dev/i2c-1 permission or wiring)");
        statusLabel->setStyleSheet("color: #e57373; font-weight: bold;");
    }

    // --- Throttled I2C Transmitter (Prevents GUI Freezing) ---
    uint8_t pendingVal = 0;
    bool isDirty = false;

    auto transmitI2C = [&]() {
        if (!isDirty) return;
        isDirty = false;

        uint8_t effectId = 0x01;
        uint8_t paramId  = 0x00;
        bool ok = comsManager.sendValueToDaisy(effectId, paramId, pendingVal);
        if (ok) {
            statusLabel->setText(QString("I2C Status: OK (Sent 3 bytes) -> Expected LED: %1")
                .arg(pendingVal < 33 ? "ON" : (pendingVal < 66 ? "OFF" : "ON")));
            statusLabel->setStyleSheet("color: #81c784; font-weight: bold;");
        } else {
            statusLabel->setText("I2C Status: WRITE FAILED! (NACK or Bus Error)");
            statusLabel->setStyleSheet("color: #e57373; font-weight: bold;");
        }
    };

    // Timer fires every 35ms (~28 FPS), keeping UI responsive even during bus timeouts
    QTimer *throttleTimer = new QTimer(&window);
    QObject::connect(throttleTimer, &QTimer::timeout, transmitI2C);
    throttleTimer->start(35);

    // Update UI immediately on touch movement, and queue I2C write
    QObject::connect(distortionSlider, &QSlider::valueChanged, 
        [&](int rawValue) {
            uint8_t val = static_cast<uint8_t>(rawValue);
            pendingVal = val;
            isDirty = true;

            // Instant UI feedback
            valueLabel->setText(QString("Slider Value: %1").arg(val));
            bytesLabel->setText(QString(
                "Bytes: [Byte 0 (Val): %1 (0x%2)]  [Byte 1 (Effect): 1 (0x01)]  [Byte 2 (Param): 0 (0x00)]"
            ).arg(val, 3)
             .arg(val, 2, 16, QChar('0')).toUpper());
        });

    // Send immediately when user lifts finger
    QObject::connect(distortionSlider, &QSlider::sliderReleased, transmitI2C);

    window.show();
    return app.exec();
}