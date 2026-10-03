#include "FacePreviewWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>

#include <algorithm>
#include <cmath>

namespace {

float curve(const bdfr::FacialFrame& frame, const char* name) {
    const auto it = frame.curves.find(name);
    return it == frame.curves.end()
        ? 0.0F
        : std::clamp(it->second, 0.0F, 1.0F);
}

} // namespace

FacePreviewWidget::FacePreviewWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(360, 360);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void FacePreviewWidget::setFrame(const bdfr::FacialFrame& frame) {
    frame_ = frame;
    hasFrame_ = true;
    update();
}

void FacePreviewWidget::clearFrame() {
    hasFrame_ = false;
    update();
}

QSize FacePreviewWidget::minimumSizeHint() const {
    return QSize(360, 360);
}

void FacePreviewWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF bounds = rect().adjusted(12, 12, -12, -12);
    p.fillRect(rect(), QColor("#0c1016"));

    const QPointF center = bounds.center();
    const double scale = std::min(bounds.width(), bounds.height()) / 420.0;

    p.save();
    p.translate(center);

    const double yaw = hasFrame_ ? frame_.head.yaw : 0.0;
    const double pitch = hasFrame_ ? frame_.head.pitch : 0.0;
    p.translate(yaw * 0.9 * scale, pitch * 0.55 * scale);

    const float jaw = hasFrame_ ? curve(frame_, "jawOpen") : 0.05F;
    const float blinkL = hasFrame_ ? curve(frame_, "eyeBlinkLeft") : 0.0F;
    const float blinkR = hasFrame_ ? curve(frame_, "eyeBlinkRight") : 0.0F;
    const float smileL = hasFrame_ ? curve(frame_, "mouthSmileLeft") : 0.0F;
    const float smileR = hasFrame_ ? curve(frame_, "mouthSmileRight") : 0.0F;
    const float brow = hasFrame_ ? curve(frame_, "browInnerUp") : 0.0F;
    const float gazeX = hasFrame_ ? frame_.gaze.x : 0.0F;
    const float gazeY = hasFrame_ ? frame_.gaze.y : 0.0F;

    QRadialGradient faceGradient(
        QPointF(-45 * scale, -70 * scale),
        240 * scale);
    faceGradient.setColorAt(0.0, QColor("#3b4657"));
    faceGradient.setColorAt(0.65, QColor("#252d39"));
    faceGradient.setColorAt(1.0, QColor("#151a22"));

    QPainterPath head;
    head.moveTo(0, -178 * scale);
    head.cubicTo(
        -112 * scale, -174 * scale,
        -142 * scale, -88 * scale,
        -132 * scale, 18 * scale);
    head.cubicTo(
        -124 * scale, 112 * scale,
        -72 * scale, 168 * scale,
        0, 188 * scale);
    head.cubicTo(
        72 * scale, 168 * scale,
        124 * scale, 112 * scale,
        132 * scale, 18 * scale);
    head.cubicTo(
        142 * scale, -88 * scale,
        112 * scale, -174 * scale,
        0, -178 * scale);

    p.setPen(QPen(QColor("#536173"), 2.0 * scale));
    p.setBrush(faceGradient);
    p.drawPath(head);

    // ears
    p.setBrush(QColor("#242c37"));
    p.drawEllipse(QPointF(-137 * scale, -2 * scale), 18 * scale, 36 * scale);
    p.drawEllipse(QPointF(137 * scale, -2 * scale), 18 * scale, 36 * scale);

    // brows
    p.setPen(QPen(QColor("#9aa8ba"), 7.0 * scale, Qt::SolidLine, Qt::RoundCap));
    const double browLift = brow * 18.0 * scale;
    p.drawLine(
        QPointF(-83 * scale, (-72 * scale) - browLift),
        QPointF(-25 * scale, (-65 * scale) - browLift));
    p.drawLine(
        QPointF(25 * scale, (-65 * scale) - browLift),
        QPointF(83 * scale, (-72 * scale) - browLift));

    auto drawEye = [&](double x, float blink, bool left) {
        const double open = std::max(2.0, (18.0 * (1.0 - blink)) * scale);
        QRectF eyeRect(
            (x - 35 * scale),
            (-43 * scale) - open * 0.5,
            70 * scale,
            open);

        p.setPen(QPen(QColor("#78879a"), 2.0 * scale));
        p.setBrush(QColor("#d5dbe4"));
        p.drawEllipse(eyeRect);

        if (blink < 0.88F) {
            const double gx = std::clamp<double>(gazeX, -1.0, 1.0) * 10.0 * scale;
            const double gy = std::clamp<double>(gazeY, -1.0, 1.0) * 5.0 * scale;
            const QPointF pupil(
                x + gx,
                -43 * scale + gy);

            p.setBrush(QColor("#4d86b9"));
            p.setPen(Qt::NoPen);
            p.drawEllipse(pupil, 7.5 * scale, 7.5 * scale);
            p.setBrush(QColor("#0c1117"));
            p.drawEllipse(pupil, 3.4 * scale, 3.4 * scale);
        }

        if (left) {
            p.setPen(QPen(QColor("#202833"), 1.5 * scale));
        }
    };

    drawEye(-58 * scale, blinkL, true);
    drawEye(58 * scale, blinkR, false);

    // nose
    p.setPen(QPen(QColor("#657286"), 2.0 * scale));
    p.setBrush(Qt::NoBrush);
    QPainterPath nose;
    nose.moveTo(0, -25 * scale);
    nose.cubicTo(
        -7 * scale, 5 * scale,
        -11 * scale, 26 * scale,
        -20 * scale, 41 * scale);
    nose.cubicTo(
        -6 * scale, 49 * scale,
        7 * scale, 49 * scale,
        20 * scale, 41 * scale);
    p.drawPath(nose);

    // mouth
    const double smile =
        (static_cast<double>(smileL) + static_cast<double>(smileR)) * 0.5;
    const double mouthWidth = (78.0 + smile * 22.0) * scale;
    const double mouthOpen = (5.0 + jaw * 46.0) * scale;
    const double cornerLift = smile * 18.0 * scale;
    const double mouthY = 89.0 * scale;

    QPainterPath mouth;
    mouth.moveTo(-mouthWidth, mouthY - cornerLift);
    mouth.cubicTo(
        -40 * scale, mouthY + mouthOpen * 0.45,
        40 * scale, mouthY + mouthOpen * 0.45,
        mouthWidth, mouthY - cornerLift);
    mouth.cubicTo(
        42 * scale, mouthY + mouthOpen,
        -42 * scale, mouthY + mouthOpen,
        -mouthWidth, mouthY - cornerLift);

    p.setPen(QPen(QColor("#7f4855"), 2.0 * scale));
    p.setBrush(QColor("#2a151c"));
    p.drawPath(mouth);

    if (jaw > 0.18F) {
        QRectF teeth(
            -52 * scale,
            mouthY + 2 * scale,
            104 * scale,
            std::min(12.0 * scale, mouthOpen * 0.35));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#d8d8d1"));
        p.drawRoundedRect(teeth, 3 * scale, 3 * scale);
    }

    // chin/jaw accent responds to jawOpen.
    p.setPen(QPen(QColor("#405065"), 2.0 * scale));
    p.drawArc(
        QRectF(
            -66 * scale,
            (118 + jaw * 16) * scale,
            132 * scale,
            42 * scale),
        200 * 16,
        140 * 16);

    p.restore();

    p.setPen(QColor("#75849a"));
    p.drawText(
        QRectF(18, 16, width() - 36, 30),
        Qt::AlignLeft | Qt::AlignVCenter,
        hasFrame_
            ? QStringLiteral("LIVE FACE RIG PREVIEW")
            : QStringLiteral("FACE RIG PREVIEW — WAITING FOR FRAME"));
}
