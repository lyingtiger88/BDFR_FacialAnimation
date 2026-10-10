#pragma once

#include "bdfr/metahuman/MetaHumanRig.h"

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPoint>
#include <QMatrix4x4>
#include <QVector3D>

#include <memory>
#include <unordered_map>
#include <vector>

class QImage;
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
    void setMeshes(const std::vector<bdfr::metahuman::MetaHumanMeshData>& meshes);
    void clearMesh();

    bool loadBaseColorTexture(const QString& path, QString* error = nullptr);
    bool setBaseColorImage(const QImage& image, QString* error = nullptr);
    bool loadNormalTexture(const QString& path, QString* error = nullptr);
    bool loadRoughnessTexture(const QString& path, QString* error = nullptr);
    bool loadSpecularTexture(const QString& path, QString* error = nullptr);
    void clearTextures();

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
        float tx;
        float ty;
        float tz;
        float materialClass;
    };

    bool loadTextureInto(
        const QString& path,
        std::unique_ptr<QOpenGLTexture>& target,
        QString* error);
    void rebuildGpuVertices();
    void recalculateTangents();
    void buildNeutralJointGlobals();
    void applyJointSkinning(const bdfr::metahuman::MetaHumanRigOutput& output);
    void uploadMeshIfReady();
    void updateCameraBounds();

    bdfr::metahuman::MetaHumanMeshData mesh_;
    std::vector<GpuVertex> gpuVertices_;

    QOpenGLBuffer vertexBuffer_{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer indexBuffer_{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject vao_;
    QOpenGLShaderProgram shader_;
    std::unique_ptr<QOpenGLTexture> baseColorTexture_;
    std::unique_ptr<QOpenGLTexture> normalTexture_;
    std::unique_ptr<QOpenGLTexture> roughnessTexture_;
    std::unique_ptr<QOpenGLTexture> specularTexture_;

    std::vector<QMatrix4x4> neutralJointGlobals_;
    std::vector<QMatrix4x4> inverseNeutralJointGlobals_;

    bool glReady_ = false;
    float yawDegrees_ = 0.0F;
    float pitchDegrees_ = 0.0F;
    float zoom_ = 1.0F;
    float meshRadius_ = 1.0F;
    QVector3D meshCenter_{0.0F, 0.0F, 0.0F};
    QPoint lastMousePosition_;
};
