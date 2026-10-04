#include "MetaHumanViewportWidget.h"

#include <QImage>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QVector3D>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <limits>

MetaHumanViewportWidget::MetaHumanViewportWidget(QWidget* parent)
    : QOpenGLWidget(parent) {
    setMinimumSize(420, 420);
    setFocusPolicy(Qt::StrongFocus);
}

MetaHumanViewportWidget::~MetaHumanViewportWidget() {
    makeCurrent();

    texture_.reset();

    if (vertexBuffer_.isCreated()) {
        vertexBuffer_.destroy();
    }

    if (indexBuffer_.isCreated()) {
        indexBuffer_.destroy();
    }

    if (vao_.isCreated()) {
        vao_.destroy();
    }

    doneCurrent();
}

bool MetaHumanViewportWidget::hasMesh() const noexcept {
    return !mesh_.vertices.empty() && !mesh_.indices.empty();
}

QString MetaHumanViewportWidget::meshName() const {
    return QString::fromStdString(mesh_.name);
}

void MetaHumanViewportWidget::setMesh(
    const bdfr::metahuman::MetaHumanMeshData& mesh) {
    mesh_ = mesh;
    rebuildGpuVertices();
    updateCameraBounds();

    if (glReady_) {
        makeCurrent();
        uploadMeshIfReady();
        doneCurrent();
    }

    update();
}

void MetaHumanViewportWidget::clearMesh() {
    mesh_ = {};
    gpuVertices_.clear();
    update();
}

bool MetaHumanViewportWidget::loadBaseColorTexture(
    const QString& path,
    QString* error) {

    QImage image(path);

    if (image.isNull()) {
        if (error) {
            *error = QStringLiteral("Unable to load texture image.");
        }
        return false;
    }

    image = image.convertToFormat(QImage::Format_RGBA8888);

    makeCurrent();

    texture_.reset();

    texture_ = std::make_unique<QOpenGLTexture>(
        image.mirrored(false, true));

    texture_->setMinificationFilter(
        QOpenGLTexture::LinearMipMapLinear);

    texture_->setMagnificationFilter(
        QOpenGLTexture::Linear);

    texture_->setWrapMode(
        QOpenGLTexture::Repeat);

    doneCurrent();

    if (error) {
        error->clear();
    }

    update();
    return true;
}

void MetaHumanViewportWidget::clearTexture() {
    makeCurrent();
    texture_.reset();
    doneCurrent();
    update();
}

void MetaHumanViewportWidget::setRigOutput(
    const bdfr::metahuman::MetaHumanRigOutput& output) {

    if (!hasMesh()) {
        return;
    }

    rebuildGpuVertices();

    for (const auto& target : mesh_.morphTargets) {
        const auto weightIt =
            output.blendShapes.find(
                target.channelName);

        if (weightIt == output.blendShapes.end()) {
            continue;
        }

        const float weight = weightIt->second;

        if (std::fabs(weight) < 0.00001F) {
            continue;
        }

        for (const auto& delta : target.deltas) {
            if (delta.vertexIndex >= gpuVertices_.size()) {
                continue;
            }

            auto& vertex =
                gpuVertices_[delta.vertexIndex];

            vertex.px += delta.dx * weight;
            vertex.py += delta.dy * weight;
            vertex.pz += delta.dz * weight;
        }
    }

    if (glReady_ && vertexBuffer_.isCreated()) {
        makeCurrent();
        vertexBuffer_.bind();
        vertexBuffer_.write(
            0,
            gpuVertices_.data(),
            static_cast<int>(
                gpuVertices_.size() *
                sizeof(GpuVertex)));
        vertexBuffer_.release();
        doneCurrent();
    }

    update();
}

void MetaHumanViewportWidget::initializeGL() {
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    static const char* vertexShader = R"(
        #version 330 core

        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aUv;

        uniform mat4 uMvp;
        uniform mat4 uModel;

        out vec3 vNormal;
        out vec2 vUv;

        void main() {
            gl_Position = uMvp * vec4(aPosition, 1.0);
            vNormal = mat3(uModel) * aNormal;
            vUv = aUv;
        }
    )";

    static const char* fragmentShader = R"(
        #version 330 core

        in vec3 vNormal;
        in vec2 vUv;

        uniform sampler2D uBaseColor;
        uniform bool uHasTexture;

        out vec4 fragColor;

        void main() {
            vec3 baseColor =
                uHasTexture
                    ? texture(uBaseColor, vUv).rgb
                    : vec3(0.55, 0.47, 0.43);

            vec3 normal = normalize(vNormal);
            vec3 lightDir = normalize(vec3(0.3, 0.65, 0.7));

            float diffuse =
                max(dot(normal, lightDir), 0.0);

            float lighting =
                0.28 + diffuse * 0.72;

            fragColor =
                vec4(baseColor * lighting, 1.0);
        }
    )";

    shader_.addShaderFromSourceCode(
        QOpenGLShader::Vertex,
        vertexShader);

    shader_.addShaderFromSourceCode(
        QOpenGLShader::Fragment,
        fragmentShader);

    shader_.link();

    vao_.create();
    vertexBuffer_.create();
    indexBuffer_.create();

    glReady_ = true;

    uploadMeshIfReady();
}

void MetaHumanViewportWidget::resizeGL(
    int,
    int) {
}

void MetaHumanViewportWidget::paintGL() {
    glClearColor(
        0.035F,
        0.045F,
        0.06F,
        1.0F);

    glClear(
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT);

    if (!hasMesh() ||
        !shader_.isLinked() ||
        !vao_.isCreated()) {
        return;
    }

    const float aspect =
        height() > 0
            ? static_cast<float>(width()) /
              static_cast<float>(height())
            : 1.0F;

    QMatrix4x4 projection;
    projection.perspective(
        36.0F,
        aspect,
        0.01F,
        10000.0F);

    const float distance =
        std::max(
            meshRadius_ * 3.2F / zoom_,
            0.1F);

    QMatrix4x4 view;
    view.lookAt(
        QVector3D(0.0F, 0.0F, distance),
        QVector3D(0.0F, 0.0F, 0.0F),
        QVector3D(0.0F, 1.0F, 0.0F));

    QMatrix4x4 model;
    model.rotate(
        pitchDegrees_,
        1.0F,
        0.0F,
        0.0F);

    model.rotate(
        yawDegrees_,
        0.0F,
        1.0F,
        0.0F);

    model.translate(
        -meshCenter_);

    const QMatrix4x4 mvp =
        projection * view * model;

    shader_.bind();
    shader_.setUniformValue(
        "uMvp",
        mvp);

    shader_.setUniformValue(
        "uModel",
        model);

    shader_.setUniformValue(
        "uHasTexture",
        texture_ != nullptr);

    shader_.setUniformValue(
        "uBaseColor",
        0);

    if (texture_) {
        texture_->bind(0);
    }

    vao_.bind();

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(
            mesh_.indices.size()),
        GL_UNSIGNED_INT,
        nullptr);

    vao_.release();

    if (texture_) {
        texture_->release();
    }

    shader_.release();
}

void MetaHumanViewportWidget::mousePressEvent(
    QMouseEvent* event) {
    lastMousePosition_ =
        event->position().toPoint();
}

void MetaHumanViewportWidget::mouseMoveEvent(
    QMouseEvent* event) {
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }

    const QPoint position =
        event->position().toPoint();

    const QPoint delta =
        position - lastMousePosition_;

    lastMousePosition_ =
        position;

    yawDegrees_ +=
        static_cast<float>(delta.x()) * 0.45F;

    pitchDegrees_ +=
        static_cast<float>(delta.y()) * 0.35F;

    pitchDegrees_ =
        std::clamp(
            pitchDegrees_,
            -89.0F,
            89.0F);

    update();
}

void MetaHumanViewportWidget::wheelEvent(
    QWheelEvent* event) {
    const float step =
        static_cast<float>(
            event->angleDelta().y()) /
        1200.0F;

    zoom_ =
        std::clamp(
            zoom_ + step,
            0.25F,
            5.0F);

    update();
}

void MetaHumanViewportWidget::rebuildGpuVertices() {
    gpuVertices_.clear();
    gpuVertices_.reserve(
        mesh_.vertices.size());

    for (const auto& vertex : mesh_.vertices) {
        gpuVertices_.push_back(
            {
                vertex.px,
                vertex.py,
                vertex.pz,
                vertex.nx,
                vertex.ny,
                vertex.nz,
                vertex.u,
                vertex.v
            });
    }
}

void MetaHumanViewportWidget::uploadMeshIfReady() {
    if (!glReady_ ||
        !hasMesh() ||
        !shader_.isLinked()) {
        return;
    }

    vao_.bind();

    vertexBuffer_.bind();
    vertexBuffer_.setUsagePattern(
        QOpenGLBuffer::DynamicDraw);

    vertexBuffer_.allocate(
        gpuVertices_.data(),
        static_cast<int>(
            gpuVertices_.size() *
            sizeof(GpuVertex)));

    indexBuffer_.bind();
    indexBuffer_.setUsagePattern(
        QOpenGLBuffer::StaticDraw);

    indexBuffer_.allocate(
        mesh_.indices.data(),
        static_cast<int>(
            mesh_.indices.size() *
            sizeof(std::uint32_t)));

    shader_.bind();

    shader_.enableAttributeArray(0);
    shader_.setAttributeBuffer(
        0,
        GL_FLOAT,
        offsetof(GpuVertex, px),
        3,
        sizeof(GpuVertex));

    shader_.enableAttributeArray(1);
    shader_.setAttributeBuffer(
        1,
        GL_FLOAT,
        offsetof(GpuVertex, nx),
        3,
        sizeof(GpuVertex));

    shader_.enableAttributeArray(2);
    shader_.setAttributeBuffer(
        2,
        GL_FLOAT,
        offsetof(GpuVertex, u),
        2,
        sizeof(GpuVertex));

    shader_.release();

    indexBuffer_.release();
    vertexBuffer_.release();
    vao_.release();
}

void MetaHumanViewportWidget::updateCameraBounds() {
    if (mesh_.vertices.empty()) {
        meshCenter_ =
            QVector3D(0.0F, 0.0F, 0.0F);

        meshRadius_ = 1.0F;
        return;
    }

    QVector3D minPoint(
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max());

    QVector3D maxPoint(
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest());

    for (const auto& vertex : mesh_.vertices) {
        minPoint.setX(
            std::min(minPoint.x(), vertex.px));
        minPoint.setY(
            std::min(minPoint.y(), vertex.py));
        minPoint.setZ(
            std::min(minPoint.z(), vertex.pz));

        maxPoint.setX(
            std::max(maxPoint.x(), vertex.px));
        maxPoint.setY(
            std::max(maxPoint.y(), vertex.py));
        maxPoint.setZ(
            std::max(maxPoint.z(), vertex.pz));
    }

    meshCenter_ =
        (minPoint + maxPoint) * 0.5F;

    meshRadius_ =
        std::max(
            (maxPoint - minPoint).length() * 0.5F,
            0.001F);
}
