#pragma once

#include "bdfr/core/FacialTypes.h"
#include "bdfr/mocap/ExternalMocap.h"
#include "bdfr/runtime/LiveSessionReceiver.h"
#include "bdfr/metahuman/MetaHumanRig.h"

#include <QElapsedTimer>
#include <QMainWindow>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class FacePreviewWidget;
class MetaHumanViewportWidget;
class QCloseEvent;
class QComboBox;
class QLabel;
class QPushButton;
class QProgressBar;
class QSpinBox;
class QTableWidget;
class QTimer;
class QTreeWidget;
class QTabWidget;

class StudioMainWindow final : public QMainWindow {
public:
    StudioMainWindow();
    ~StudioMainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    struct SharedSnapshot {
        bool receiverOpen = false;
        bool hasFrame = false;
        std::string error;
        bdfr::FacialFrame latestFrame;
        bdfr::runtime::LiveReceiverStats stats;
        std::uint64_t deliveredFrames = 0;
    };

    void buildUi();
    QWidget* buildConnectionPanel();
    QWidget* buildMonitorPanel();
    QWidget* buildCurvesPanel();
    QWidget* buildProjectPanel();

    void applyTheme();
    void refreshLocalAddresses();
    void refreshUi();

    void startReceiver();
    void stopReceiver();
    void receiverLoop(std::uint16_t port, std::string bindAddress);
    void sendLocalTestPacket();

    void loadMetaHumanDna();
    void loadMetaHumanBaseColor();
    void updateMetaHumanEvaluation(const bdfr::FacialFrame& frame);

    void toggleRecording();
    void saveRecording();

    void updateGauge(QProgressBar* gauge, float value);
    void updateCurveTable(const bdfr::FacialFrame& frame);
    void setStatus(const QString& text, bool good);

    std::atomic<bool> receiverRunning_{false};
    std::atomic<bool> recording_{false};
    std::thread receiverThread_;

    mutable std::mutex sharedMutex_;
    SharedSnapshot shared_;
    std::vector<bdfr::mocap::MocapPacket> recordedPackets_;
    std::uint64_t recordSequence_ = 0;

    QTimer* refreshTimer_ = nullptr;
    QElapsedTimer fpsTimer_;
    std::uint64_t lastDeliveredFrames_ = 0;
    double displayedFps_ = 0.0;

    QComboBox* addressCombo_ = nullptr;
    QSpinBox* portSpin_ = nullptr;
    QPushButton* receiverButton_ = nullptr;
    QPushButton* localTestButton_ = nullptr;
    QPushButton* recordButton_ = nullptr;
    QPushButton* loadDnaButton_ = nullptr;
    QPushButton* loadTextureButton_ = nullptr;

    QLabel* connectionStateLabel_ = nullptr;
    QLabel* sourceLabel_ = nullptr;
    QLabel* localAddressLabel_ = nullptr;
    QLabel* fpsLabel_ = nullptr;
    QLabel* confidenceLabel_ = nullptr;
    QLabel* packetsLabel_ = nullptr;
    QLabel* lossLabel_ = nullptr;
    QLabel* clockLabel_ = nullptr;
    QLabel* recordingLabel_ = nullptr;
    QLabel* headLabel_ = nullptr;
    QLabel* gazeLabel_ = nullptr;
    QLabel* metaHumanStatusLabel_ = nullptr;
    QLabel* metaHumanStatsLabel_ = nullptr;
    QLabel* metaHumanEvalLabel_ = nullptr;

    FacePreviewWidget* facePreview_ = nullptr;
    MetaHumanViewportWidget* metaHumanViewport_ = nullptr;
    QTabWidget* previewTabs_ = nullptr;

    QProgressBar* jawGauge_ = nullptr;
    QProgressBar* blinkLeftGauge_ = nullptr;
    QProgressBar* blinkRightGauge_ = nullptr;
    QProgressBar* smileLeftGauge_ = nullptr;
    QProgressBar* smileRightGauge_ = nullptr;
    QProgressBar* browGauge_ = nullptr;

    QTableWidget* curvesTable_ = nullptr;
    QTreeWidget* projectTree_ = nullptr;

    bdfr::metahuman::MetaHumanRigRuntime metaHumanRig_;
};
