#include "StudioMainWindow.h"

#include "bdfr/runtime/SessionStream.h"

#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <map>
#include <utility>

namespace {

double steadySeconds() {
    using Clock = std::chrono::steady_clock;
    return std::chrono::duration<double>(
        Clock::now().time_since_epoch()).count();
}

float curveValue(const bdfr::FacialFrame& frame, const char* name) {
    const auto it = frame.curves.find(name);
    return it == frame.curves.end() ? 0.0F : it->second;
}

QLabel* makeMetricLabel(const QString& caption, const QString& value) {
    auto* label = new QLabel(
        QStringLiteral("<span style='color:#8f9bad;'>%1</span><br>"
                       "<span style='font-size:18pt;font-weight:600;'>%2</span>")
            .arg(caption, value));
    label->setTextFormat(Qt::RichText);
    label->setMinimumWidth(110);
    return label;
}

QProgressBar* makeGauge() {
    auto* gauge = new QProgressBar();
    gauge->setRange(0, 1000);
    gauge->setValue(0);
    gauge->setTextVisible(true);
    gauge->setFormat(QStringLiteral("0.000"));
    gauge->setMinimumHeight(24);
    return gauge;
}

} // namespace

StudioMainWindow::StudioMainWindow() {
    setWindowTitle(QStringLiteral("BDFR Studio v0.1 — Live Face Monitor"));
    resize(1440, 880);
    setMinimumSize(1080, 680);

    buildUi();
    applyTheme();
    refreshLocalAddresses();

    refreshTimer_ = new QTimer(this);
    refreshTimer_->setInterval(50);
    connect(refreshTimer_, &QTimer::timeout, this, [this] {
        refreshUi();
    });
    refreshTimer_->start();

    fpsTimer_.start();
    setStatus(QStringLiteral("Ready — receiver stopped"), false);
}

StudioMainWindow::~StudioMainWindow() {
    stopReceiver();
}

void StudioMainWindow::closeEvent(QCloseEvent* event) {
    stopReceiver();
    event->accept();
}

void StudioMainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(12, 12, 12, 12);
    centralLayout->setSpacing(10);

    centralLayout->addWidget(buildConnectionPanel());
    centralLayout->addWidget(buildMonitorPanel(), 1);

    setCentralWidget(central);

    auto* projectDock = new QDockWidget(QStringLiteral("Project / Session"), this);
    projectDock->setObjectName(QStringLiteral("ProjectDock"));
    projectDock->setWidget(buildProjectPanel());
    addDockWidget(Qt::LeftDockWidgetArea, projectDock);

    auto* curvesDock = new QDockWidget(QStringLiteral("Live Curves"), this);
    curvesDock->setObjectName(QStringLiteral("CurvesDock"));
    curvesDock->setWidget(buildCurvesPanel());
    curvesDock->setMinimumWidth(330);
    addDockWidget(Qt::RightDockWidgetArea, curvesDock);

    statusBar()->showMessage(QStringLiteral("BDFR Studio initialized"));
}

QWidget* StudioMainWindow::buildConnectionPanel() {
    auto* box = new QGroupBox(QStringLiteral("Live Receiver"));
    auto* layout = new QHBoxLayout(box);

    addressEdit_ = new QLineEdit(QStringLiteral("0.0.0.0"));
    addressEdit_->setReadOnly(true);
    addressEdit_->setMaximumWidth(120);

    portSpin_ = new QSpinBox();
    portSpin_->setRange(1, 65535);
    portSpin_->setValue(5000);
    portSpin_->setMaximumWidth(100);

    receiverButton_ = new QPushButton(QStringLiteral("Start Receiver"));
    receiverButton_->setMinimumWidth(140);
    connect(receiverButton_, &QPushButton::clicked, this, [this] {
        if (receiverRunning_) {
            stopReceiver();
        } else {
            startReceiver();
        }
    });

    recordButton_ = new QPushButton(QStringLiteral("● Record"));
    recordButton_->setEnabled(false);
    recordButton_->setMinimumWidth(110);
    connect(recordButton_, &QPushButton::clicked, this, [this] {
        toggleRecording();
    });

    connectionStateLabel_ = new QLabel(QStringLiteral("● OFFLINE"));
    connectionStateLabel_->setMinimumWidth(110);

    localAddressLabel_ = new QLabel();
    localAddressLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    layout->addWidget(new QLabel(QStringLiteral("Bind")));
    layout->addWidget(addressEdit_);
    layout->addWidget(new QLabel(QStringLiteral("UDP Port")));
    layout->addWidget(portSpin_);
    layout->addWidget(receiverButton_);
    layout->addWidget(recordButton_);
    layout->addWidget(connectionStateLabel_);
    layout->addSpacing(10);
    layout->addWidget(new QLabel(QStringLiteral("PC IPv4:")));
    layout->addWidget(localAddressLabel_, 1);

    return box;
}

QWidget* StudioMainWindow::buildMonitorPanel() {
    auto* widget = new QWidget();
    auto* root = new QVBoxLayout(widget);
    root->setContentsMargins(0, 0, 0, 0);

    auto* metrics = new QFrame();
    auto* metricsLayout = new QHBoxLayout(metrics);

    fpsLabel_ = makeMetricLabel(QStringLiteral("FPS"), QStringLiteral("0.0"));
    confidenceLabel_ = makeMetricLabel(QStringLiteral("Confidence"), QStringLiteral("0.000"));
    packetsLabel_ = makeMetricLabel(QStringLiteral("Packets"), QStringLiteral("0"));
    lossLabel_ = makeMetricLabel(QStringLiteral("Lost"), QStringLiteral("0"));
    clockLabel_ = makeMetricLabel(QStringLiteral("Clock offset"), QStringLiteral("0 ms"));
    recordingLabel_ = makeMetricLabel(QStringLiteral("Recording"), QStringLiteral("OFF"));

    metricsLayout->addWidget(fpsLabel_);
    metricsLayout->addWidget(confidenceLabel_);
    metricsLayout->addWidget(packetsLabel_);
    metricsLayout->addWidget(lossLabel_);
    metricsLayout->addWidget(clockLabel_);
    metricsLayout->addWidget(recordingLabel_);
    metricsLayout->addStretch(1);

    root->addWidget(metrics);

    auto* expressions = new QGroupBox(QStringLiteral("Live Performance"));
    auto* grid = new QGridLayout(expressions);

    jawGauge_ = makeGauge();
    blinkLeftGauge_ = makeGauge();
    blinkRightGauge_ = makeGauge();
    smileLeftGauge_ = makeGauge();
    smileRightGauge_ = makeGauge();
    browGauge_ = makeGauge();

    const std::pair<const char*, QProgressBar*> gauges[] = {
        {"Jaw Open", jawGauge_},
        {"Blink Left", blinkLeftGauge_},
        {"Blink Right", blinkRightGauge_},
        {"Smile Left", smileLeftGauge_},
        {"Smile Right", smileRightGauge_},
        {"Brow Inner Up", browGauge_}
    };

    for (int i = 0; i < 6; ++i) {
        const int row = i % 3;
        const int column = (i / 3) * 2;
        grid->addWidget(new QLabel(QString::fromUtf8(gauges[i].first)), row, column);
        grid->addWidget(gauges[i].second, row, column + 1);
    }

    headLabel_ = new QLabel(QStringLiteral("Head  P 0.0°   Y 0.0°   R 0.0°"));
    gazeLabel_ = new QLabel(QStringLiteral("Gaze  X 0.000   Y 0.000   conf 0.000"));

    grid->addWidget(headLabel_, 3, 0, 1, 2);
    grid->addWidget(gazeLabel_, 3, 2, 1, 2);

    root->addWidget(expressions);

    auto* help = new QFrame();
    auto* helpLayout = new QVBoxLayout(help);

    auto* title = new QLabel(QStringLiteral("Android → BDFR Studio"));
    title->setStyleSheet(QStringLiteral("font-size:18pt;font-weight:600;"));

    auto* instructions = new QLabel(
        QStringLiteral(
            "1. Start Receiver  •  2. Put PC IPv4 + port 5000 in Android  •  "
            "3. Tap Test PC  •  4. Get Model → Live for real face curves\n"
            "Frames arriving here are decoded through the same BDFP/clock/jitter runtime used by the CLI."));
    instructions->setWordWrap(true);

    helpLayout->addWidget(title);
    helpLayout->addWidget(instructions);

    root->addWidget(help);
    root->addStretch(1);

    return widget;
}

QWidget* StudioMainWindow::buildCurvesPanel() {
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(4, 4, 4, 4);

    curvesTable_ = new QTableWidget(0, 2);
    curvesTable_->setHorizontalHeaderLabels(
        {QStringLiteral("Curve"), QStringLiteral("Value")});
    curvesTable_->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch);
    curvesTable_->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    curvesTable_->verticalHeader()->setVisible(false);
    curvesTable_->setAlternatingRowColors(true);
    curvesTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    curvesTable_->setSelectionBehavior(QAbstractItemView::SelectRows);

    layout->addWidget(curvesTable_);
    return widget;
}

QWidget* StudioMainWindow::buildProjectPanel() {
    auto* widget = new QWidget();
    auto* layout = new QVBoxLayout(widget);

    projectTree_ = new QTreeWidget();
    projectTree_->setHeaderHidden(true);

    auto* project = new QTreeWidgetItem(
        projectTree_, {QStringLiteral("BDFR Live Project")});

    auto* actor = new QTreeWidgetItem(
        project, {QStringLiteral("Actor — Android")});

    auto* session = new QTreeWidgetItem(
        actor, {QStringLiteral("Live Session")});

    new QTreeWidgetItem(
        session, {QStringLiteral("Take 001 — waiting")});

    projectTree_->expandAll();

    auto* info = new QLabel(
        QStringLiteral(
            "v0.1 live workspace\n"
            "• UDP/BDFP receiver\n"
            "• 52-curve monitor\n"
            "• packet diagnostics\n"
            "• BDFS recording"));
    info->setWordWrap(true);

    layout->addWidget(projectTree_, 1);
    layout->addWidget(info);

    return widget;
}

void StudioMainWindow::applyTheme() {
    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QWidget {
            background: #101318;
            color: #e9edf4;
        }
        QGroupBox {
            border: 1px solid #2a303a;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 10px;
            font-weight: 600;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
            color: #b9c4d4;
        }
        QLineEdit, QSpinBox, QTableWidget, QTreeWidget {
            background: #171b22;
            border: 1px solid #303744;
            border-radius: 5px;
            padding: 5px;
            selection-background-color: #315b8d;
        }
        QPushButton {
            background: #242c37;
            border: 1px solid #3a4657;
            border-radius: 6px;
            padding: 7px 12px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: #2f3a49;
        }
        QPushButton:disabled {
            color: #68717e;
            background: #191d23;
        }
        QProgressBar {
            background: #171b22;
            border: 1px solid #303744;
            border-radius: 5px;
            text-align: center;
        }
        QProgressBar::chunk {
            background: #3c7bb7;
            border-radius: 4px;
        }
        QHeaderView::section {
            background: #1e242d;
            color: #aeb8c7;
            padding: 6px;
            border: 0;
        }
        QDockWidget::title {
            background: #171b22;
            padding: 8px;
            font-weight: 600;
        }
        QStatusBar {
            background: #0c0f13;
            color: #94a2b5;
        }
    )"));
}

void StudioMainWindow::refreshLocalAddresses() {
    QStringList addresses;

    const auto allAddresses = QNetworkInterface::allAddresses();
    for (const QHostAddress& address : allAddresses) {
        if (address.protocol() != QAbstractSocket::IPv4Protocol ||
            address.isLoopback()) {
            continue;
        }

        addresses.push_back(address.toString());
    }

    if (addresses.isEmpty()) {
        localAddressLabel_->setText(QStringLiteral("No LAN IPv4 detected"));
    } else {
        addresses.removeDuplicates();
        localAddressLabel_->setText(addresses.join(QStringLiteral("   |   ")));
    }
}

void StudioMainWindow::startReceiver() {
    if (receiverRunning_) {
        return;
    }

    const int port = portSpin_->value();

    {
        std::lock_guard<std::mutex> lock(sharedMutex_);
        shared_ = {};
        recordedPackets_.clear();
        recordSequence_ = 0;
    }

    receiverRunning_ = true;
    receiverButton_->setText(QStringLiteral("Stop Receiver"));
    portSpin_->setEnabled(false);
    recordButton_->setEnabled(true);

    fpsTimer_.restart();
    lastDeliveredFrames_ = 0;
    displayedFps_ = 0.0;

    receiverThread_ = std::thread(
        &StudioMainWindow::receiverLoop,
        this,
        static_cast<std::uint16_t>(port));

    setStatus(
        QStringLiteral("Starting UDP receiver on port %1…").arg(port),
        true);
}

void StudioMainWindow::stopReceiver() {
    if (!receiverRunning_ && !receiverThread_.joinable()) {
        return;
    }

    receiverRunning_ = false;

    if (receiverThread_.joinable()) {
        receiverThread_.join();
    }

    if (recording_) {
        recording_ = false;
        saveRecording();
    }

    receiverButton_->setText(QStringLiteral("Start Receiver"));
    portSpin_->setEnabled(true);
    recordButton_->setEnabled(false);
    recordButton_->setText(QStringLiteral("● Record"));

    setStatus(QStringLiteral("Receiver stopped"), false);
}

void StudioMainWindow::receiverLoop(std::uint16_t port) {
    bdfr::runtime::LiveSessionReceiver receiver(0.035);
    std::string error;

    if (!receiver.open(port, "0.0.0.0", &error)) {
        std::lock_guard<std::mutex> lock(sharedMutex_);
        shared_.error = error;
        shared_.receiverOpen = false;
        receiverRunning_ = false;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(sharedMutex_);
        shared_.receiverOpen = true;
        shared_.error.clear();
    }

    while (receiverRunning_) {
        const double now = steadySeconds();

        error.clear();
        receiver.poll(50, now, &error);

        bdfr::FacialFrame frame;
        while (receiver.popReady(now, frame)) {
            std::lock_guard<std::mutex> lock(sharedMutex_);

            shared_.latestFrame = frame;
            shared_.hasFrame = true;
            ++shared_.deliveredFrames;

            if (recording_) {
                bdfr::mocap::MocapPacket packet;
                packet.sourceId = "bdfr-studio";
                packet.sequenceNumber = recordSequence_++;
                packet.frame = frame;
                recordedPackets_.push_back(std::move(packet));
            }
        }

        {
            std::lock_guard<std::mutex> lock(sharedMutex_);
            shared_.stats = receiver.stats();
            if (!error.empty()) {
                shared_.error = error;
            }
        }
    }

    receiver.close();

    std::lock_guard<std::mutex> lock(sharedMutex_);
    shared_.receiverOpen = false;
}

void StudioMainWindow::refreshUi() {
    SharedSnapshot snapshot;

    {
        std::lock_guard<std::mutex> lock(sharedMutex_);
        snapshot = shared_;
    }

    if (!receiverRunning_ && receiverThread_.joinable()) {
        receiverThread_.join();

        receiverButton_->setText(QStringLiteral("Start Receiver"));
        portSpin_->setEnabled(true);
        recordButton_->setEnabled(false);

        if (!snapshot.error.empty()) {
            setStatus(
                QStringLiteral("Receiver error: %1")
                    .arg(QString::fromStdString(snapshot.error)),
                false);
        }
    }

    if (snapshot.receiverOpen) {
        if (snapshot.stats.packetsReceived > 0) {
            connectionStateLabel_->setText(QStringLiteral("● CONNECTED"));
            connectionStateLabel_->setStyleSheet(
                QStringLiteral("color:#63d297;font-weight:700;"));
        } else {
            connectionStateLabel_->setText(QStringLiteral("● LISTENING"));
            connectionStateLabel_->setStyleSheet(
                QStringLiteral("color:#e0b75b;font-weight:700;"));
        }
    } else {
        connectionStateLabel_->setText(QStringLiteral("● OFFLINE"));
        connectionStateLabel_->setStyleSheet(
            QStringLiteral("color:#8c96a5;font-weight:700;"));
    }

    const qint64 elapsedMs = fpsTimer_.elapsed();
    if (elapsedMs >= 500) {
        const std::uint64_t delta =
            snapshot.deliveredFrames - lastDeliveredFrames_;

        displayedFps_ =
            static_cast<double>(delta) * 1000.0 /
            static_cast<double>(elapsedMs);

        lastDeliveredFrames_ = snapshot.deliveredFrames;
        fpsTimer_.restart();
    }

    fpsLabel_->setText(
        QStringLiteral("<span style='color:#8f9bad;'>FPS</span><br>"
                       "<span style='font-size:18pt;font-weight:600;'>%1</span>")
            .arg(displayedFps_, 0, 'f', 1));

    packetsLabel_->setText(
        QStringLiteral("<span style='color:#8f9bad;'>Packets</span><br>"
                       "<span style='font-size:18pt;font-weight:600;'>%1</span>")
            .arg(snapshot.stats.packetsReceived));

    lossLabel_->setText(
        QStringLiteral("<span style='color:#8f9bad;'>Lost</span><br>"
                       "<span style='font-size:18pt;font-weight:600;'>%1</span>")
            .arg(snapshot.stats.packetsLost));

    clockLabel_->setText(
        QStringLiteral("<span style='color:#8f9bad;'>Clock offset</span><br>"
                       "<span style='font-size:18pt;font-weight:600;'>%1 ms</span>")
            .arg(snapshot.stats.clockOffsetSeconds * 1000.0, 0, 'f', 1));

    recordingLabel_->setText(
        QStringLiteral("<span style='color:#8f9bad;'>Recording</span><br>"
                       "<span style='font-size:18pt;font-weight:600;'>%1</span>")
            .arg(recording_ ? QStringLiteral("REC") : QStringLiteral("OFF")));

    if (!snapshot.hasFrame) {
        return;
    }

    const auto& frame = snapshot.latestFrame;

    confidenceLabel_->setText(
        QStringLiteral("<span style='color:#8f9bad;'>Confidence</span><br>"
                       "<span style='font-size:18pt;font-weight:600;'>%1</span>")
            .arg(frame.confidence, 0, 'f', 3));

    updateGauge(jawGauge_, curveValue(frame, "jawOpen"));
    updateGauge(blinkLeftGauge_, curveValue(frame, "eyeBlinkLeft"));
    updateGauge(blinkRightGauge_, curveValue(frame, "eyeBlinkRight"));
    updateGauge(smileLeftGauge_, curveValue(frame, "mouthSmileLeft"));
    updateGauge(smileRightGauge_, curveValue(frame, "mouthSmileRight"));
    updateGauge(browGauge_, curveValue(frame, "browInnerUp"));

    headLabel_->setText(
        QStringLiteral("Head  P %1°   Y %2°   R %3°")
            .arg(frame.head.pitch, 0, 'f', 1)
            .arg(frame.head.yaw, 0, 'f', 1)
            .arg(frame.head.roll, 0, 'f', 1));

    gazeLabel_->setText(
        QStringLiteral("Gaze  X %1   Y %2   conf %3")
            .arg(frame.gaze.x, 0, 'f', 3)
            .arg(frame.gaze.y, 0, 'f', 3)
            .arg(frame.gaze.confidence, 0, 'f', 3));

    updateCurveTable(frame);
}

void StudioMainWindow::toggleRecording() {
    if (!receiverRunning_) {
        return;
    }

    if (!recording_) {
        {
            std::lock_guard<std::mutex> lock(sharedMutex_);
            recordedPackets_.clear();
            recordSequence_ = 0;
        }

        recording_ = true;
        recordButton_->setText(QStringLiteral("■ Stop & Save"));
        statusBar()->showMessage(QStringLiteral("Recording live BDFR frames…"));
        return;
    }

    recording_ = false;
    recordButton_->setText(QStringLiteral("● Record"));
    saveRecording();
}

void StudioMainWindow::saveRecording() {
    std::vector<bdfr::mocap::MocapPacket> packets;

    {
        std::lock_guard<std::mutex> lock(sharedMutex_);
        packets = recordedPackets_;
    }

    if (packets.empty()) {
        QMessageBox::information(
            this,
            QStringLiteral("BDFR Studio"),
            QStringLiteral("No facial frames were recorded."));
        return;
    }

    const QString fileName = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Save BDFR Take"),
        QStringLiteral("BDFR_Take_001.bdfs"),
        QStringLiteral("BDFR Session (*.bdfs)"));

    if (fileName.isEmpty()) {
        return;
    }

    std::string error;

    if (!bdfr::runtime::SessionStream::save(
            std::filesystem::path(fileName.toStdWString()),
            packets,
            &error)) {
        QMessageBox::critical(
            this,
            QStringLiteral("Save failed"),
            QString::fromStdString(error));
        return;
    }

    statusBar()->showMessage(
        QStringLiteral("Saved %1 frames → %2")
            .arg(packets.size())
            .arg(fileName),
        8000);
}

void StudioMainWindow::updateGauge(QProgressBar* gauge, float value) {
    const float clamped = std::clamp(value, 0.0F, 1.0F);
    gauge->setValue(static_cast<int>(std::lround(clamped * 1000.0F)));
    gauge->setFormat(QString::number(clamped, 'f', 3));
}

void StudioMainWindow::updateCurveTable(const bdfr::FacialFrame& frame) {
    std::map<std::string, float> sorted(
        frame.curves.begin(),
        frame.curves.end());

    curvesTable_->setRowCount(static_cast<int>(sorted.size()));

    int row = 0;
    for (const auto& [name, value] : sorted) {
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(name));
        auto* valueItem = new QTableWidgetItem(
            QString::number(value, 'f', 4));
        valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        curvesTable_->setItem(row, 0, nameItem);
        curvesTable_->setItem(row, 1, valueItem);
        ++row;
    }
}

void StudioMainWindow::setStatus(const QString& text, bool good) {
    statusBar()->showMessage(text);

    if (good) {
        connectionStateLabel_->setStyleSheet(
            QStringLiteral("color:#63d297;font-weight:700;"));
    }
}
