#pragma once

#include "bdfr/core/FacialTypes.h"

#include <QWidget>

class FacePreviewWidget final : public QWidget {
public:
    explicit FacePreviewWidget(QWidget* parent = nullptr);

    void setFrame(const bdfr::FacialFrame& frame);
    void clearFrame();

    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bdfr::FacialFrame frame_;
    bool hasFrame_ = false;
};
