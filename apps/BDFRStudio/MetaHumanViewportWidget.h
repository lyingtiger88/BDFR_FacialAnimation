#pragma once

#include "bdfr/metahuman/MetaHumanRig.h"

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPoint>
#include <QVector3D>

#include <memory>
#include <unordered_map>
#include <vector>

class QMouseEvent;
class QOpenGLTexture;
class QWheelEvent;

class MetaHumanViewportWidget final
    : public QOpenGLWidget,
      protected QOpenGLFunctions {
public:
    explicit MetaHumanViewportWidget(QWidget* parent = nullptr);
    ~MetaHumanViewportWidget() override;

    void setMesh(const bdfr::metahuman::MetaHumanMeshData& mesh);
    void clearMesh();

    bool loadBaseColorTexture(const QString& path, QString* error = nullptr);
    void clearTexture();

    void setRigOutput(const bdfr::metahuman::MetaHumanRigOutput& output);

    bool hasMesh() const noexcept;
    QString meshName() const;

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct GpuVertex {
        float px;
        float py;
        float pz;
        float nx;
        float ny;
        float nz;
        float u;
        float v;
    };

    void rebuildGpuVertices();
    void uploadMeshIfReady();
    void updateCameraBounds();

    bdfr::metahuman::MetaHumanMeshData mesh_;
    std::vector<GpuVertex> gpuVertices_;

    QOpenGLBuffer vertexBuffer_{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer indexBuffer_{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject vao_;
    QOpenGLShaderProgram shader_;
    std::unique_ptr<QOpenGLTexture> texture_;

    bool glReady_ = false;
    float yawDegrees_ = 0.0F;
    float pitchDegrees_ = 0.0F;
    float zoom_ = 1.0F;
    float meshRadius_ = 1.0F;
    QVector3D meshCenter_{0.0F, 0.0F, 0.0F};
    QPoint lastMousePosition_;
};
