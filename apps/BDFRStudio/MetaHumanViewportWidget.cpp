#include "MetaHumanViewportWidget.h"

#include <QImage>
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>
#include <QWheelEvent>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace {

QMatrix4x4 makeLocalMatrix(
    const bdfr::metahuman::MetaHumanJoint& joint) {

    QMatrix4x4 result;
    result.translate(joint.tx, joint.ty, joint.tz);

    const QQuaternion rotation(
        joint.qw,
        joint.qx,
        joint.qy,
        joint.qz);

    result.rotate(rotation);
    result.scale(joint.sx, joint.sy, joint.sz);
    return result;
}

float materialClassForName(const std::string& name) {
    std::string lower = name;
    std::transform(
        lower.begin(),
        lower.end(),
        lower.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });

    if (lower.find("eye") != std::string::npos ||
        lower.find("iris") != std::string::npos ||
        lower.find("cornea") != std::string::npos) {
        return 1.0F;
    }

    if (lower.find("teeth") != std::string::npos ||
        lower.find("tooth") != std::string::npos ||
        lower.find("gum") != std::string::npos) {
        return 2.0F;
    }

    return 0.0F;
}

QQuaternion normalizedDeltaQuaternion(
    float x,
    float y,
    float z,
    float w) {

    QQuaternion q(w, x, y, z);

    if (q.lengthSquared() < 0.000001F) {
        return QQuaternion();
    }

    q.normalize();
    return q;
}

} // namespace

MetaHumanViewportWidget::MetaHumanViewportWidget(QWidget* parent)
    : QOpenGLWidget(parent) {
    setMinimumSize(420, 420);
    setFocusPolicy(Qt::StrongFocus);
}

MetaHumanViewportWidget::~MetaHumanViewportWidget() {
    makeCurrent();

    baseColorTexture_.reset();
    normalTexture_.reset();
    roughnessTexture_.reset();
    specularTexture_.reset();

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
    return !mesh_.vertices.empty() &&
           !mesh_.indices.empty();
}

QString MetaHumanViewportWidget::meshName() const {
    return QString::fromStdString(mesh_.name);
}

void MetaHumanViewportWidget::setMesh(
    const bdfr::metahuman::MetaHumanMeshData& mesh) {

    mesh_ = mesh;

    const float meshMaterialClass =
        materialClassForName(mesh_.name);

    if (meshMaterialClass > 0.0F) {
        for (auto& vertex : mesh_.vertices) {
            vertex.materialClass =
                meshMaterialClass;
        }
    }

    rebuildGpuVertices();
    recalculateTangents();
    buildNeutralJointGlobals();
    updateCameraBounds();

    if (glReady_) {
        makeCurrent();
        uploadMeshIfReady();
        doneCurrent();
    }

    update();
}

void MetaHumanViewportWidget::setMeshes(
    const std::vector<bdfr::metahuman::MetaHumanMeshData>& meshes) {

    bdfr::metahuman::MetaHumanMeshData combined;
    combined.name = "MetaHuman LOD0 Scene";

    std::unordered_map<std::string, std::size_t>
        morphTargetLookup;

    bool copiedSkeleton = false;

    for (const auto& source : meshes) {
        const std::uint32_t vertexOffset =
            static_cast<std::uint32_t>(
                combined.vertices.size());

        const float materialClass =
            materialClassForName(source.name);

        for (auto vertex : source.vertices) {
            vertex.materialClass =
                materialClass;
            combined.vertices.push_back(
                vertex);
        }

        for (const auto index :
             source.indices) {
            combined.indices.push_back(
                vertexOffset + index);
        }

        combined.skinInfluences.insert(
            combined.skinInfluences.end(),
            source.skinInfluences.begin(),
            source.skinInfluences.end());

        if (!copiedSkeleton &&
            !source.joints.empty()) {
            combined.joints =
                source.joints;

            combined.jointAttributeCountPerJoint =
                source.jointAttributeCountPerJoint;

            copiedSkeleton = true;
        }

        for (const auto& sourceTarget :
             source.morphTargets) {

            std::size_t targetIndex = 0;

            const auto existing =
                morphTargetLookup.find(
                    sourceTarget.channelName);

            if (existing ==
                morphTargetLookup.end()) {

                targetIndex =
                    combined.morphTargets.size();

                bdfr::metahuman::MetaHumanMorphTarget
                    target;

                target.channelName =
                    sourceTarget.channelName;

                combined.morphTargets.push_back(
                    std::move(target));

                morphTargetLookup.emplace(
                    sourceTarget.channelName,
                    targetIndex);

            } else {
                targetIndex =
                    existing->second;
            }

            auto& destinationTarget =
                combined.morphTargets[
                    targetIndex];

            destinationTarget.deltas.reserve(
                destinationTarget.deltas.size() +
                sourceTarget.deltas.size());

            for (auto delta :
                 sourceTarget.deltas) {

                delta.vertexIndex +=
                    vertexOffset;

                destinationTarget.deltas.push_back(
                    delta);
            }
        }
    }

    setMesh(combined);
}

void MetaHumanViewportWidget::clearMesh() {
    mesh_ = {};
    gpuVertices_.clear();
    neutralJointGlobals_.clear();
    inverseNeutralJointGlobals_.clear();
    update();
}

bool MetaHumanViewportWidget::loadTextureInto(
    const QString& path,
    std::unique_ptr<QOpenGLTexture>& target,
    QString* error) {

    QImage image(path);

    if (image.isNull()) {
        if (error) {
            *error =
                QStringLiteral(
                    "Unable to load texture image.");
        }

        return false;
    }

    image =
        image.convertToFormat(
            QImage::Format_RGBA8888);

    makeCurrent();

    target.reset();

    target =
        std::make_unique<QOpenGLTexture>(
            image.mirrored(false, true));

    target->setMinificationFilter(
        QOpenGLTexture::LinearMipMapLinear);

    target->setMagnificationFilter(
        QOpenGLTexture::Linear);

    target->setWrapMode(
        QOpenGLTexture::Repeat);

    doneCurrent();

    if (error) {
        error->clear();
    }

    update();
    return true;
}

bool MetaHumanViewportWidget::loadBaseColorTexture(
    const QString& path,
    QString* error) {
    return loadTextureInto(
        path,
        baseColorTexture_,
        error);
}

bool MetaHumanViewportWidget::setBaseColorImage(
    const QImage& sourceImage,
    QString* error) {

    if (sourceImage.isNull()) {
        if (error) {
            *error =
                QStringLiteral("Generated FaceBuilder texture image is empty.");
        }
        return false;
    }

    QImage image =
        sourceImage.convertToFormat(
            QImage::Format_RGBA8888);

    makeCurrent();

    baseColorTexture_.reset();

    baseColorTexture_ =
        std::make_unique<QOpenGLTexture>(
            image.mirrored(false, true));

    baseColorTexture_->setMinificationFilter(
        QOpenGLTexture::LinearMipMapLinear);

    baseColorTexture_->setMagnificationFilter(
        QOpenGLTexture::Linear);

    baseColorTexture_->setWrapMode(
        QOpenGLTexture::Repeat);

    doneCurrent();

    if (error) {
        error->clear();
    }

    update();
    return true;
}

bool MetaHumanViewportWidget::loadNormalTexture(
    const QString& path,
    QString* error) {
    return loadTextureInto(
        path,
        normalTexture_,
        error);
}

bool MetaHumanViewportWidget::loadRoughnessTexture(
    const QString& path,
    QString* error) {
    return loadTextureInto(
        path,
        roughnessTexture_,
        error);
}

bool MetaHumanViewportWidget::loadSpecularTexture(
    const QString& path,
    QString* error) {
    return loadTextureInto(
        path,
        specularTexture_,
        error);
}

void MetaHumanViewportWidget::clearTextures() {
    makeCurrent();

    baseColorTexture_.reset();
    normalTexture_.reset();
    roughnessTexture_.reset();
    specularTexture_.reset();

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

        if (weightIt ==
            output.blendShapes.end()) {
            continue;
        }

        const float weight =
            weightIt->second;

        if (std::fabs(weight) < 0.00001F) {
            continue;
        }

        for (const auto& delta :
             target.deltas) {

            if (delta.vertexIndex >=
                gpuVertices_.size()) {
                continue;
            }

            auto& vertex =
                gpuVertices_[delta.vertexIndex];

            vertex.px += delta.dx * weight;
            vertex.py += delta.dy * weight;
            vertex.pz += delta.dz * weight;
        }
    }

    applyJointSkinning(output);
    recalculateTangents();

    if (glReady_ &&
        vertexBuffer_.isCreated()) {

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
        layout(location = 3) in vec3 aTangent;
        layout(location = 4) in float aMaterialClass;

        uniform mat4 uMvp;
        uniform mat4 uModel;

        out vec3 vWorldPosition;
        out vec3 vNormal;
        out vec3 vTangent;
        out vec2 vUv;
        flat out float vMaterialClass;

        void main() {
            vec4 worldPosition =
                uModel * vec4(aPosition, 1.0);

            gl_Position =
                uMvp * vec4(aPosition, 1.0);

            mat3 normalMatrix =
                transpose(inverse(mat3(uModel)));

            vWorldPosition =
                worldPosition.xyz;

            vNormal =
                normalize(
                    normalMatrix * aNormal);

            vTangent =
                normalize(
                    normalMatrix * aTangent);

            vUv = aUv;
            vMaterialClass = aMaterialClass;
        }
    )";

    static const char* fragmentShader = R"(
        #version 330 core

        in vec3 vWorldPosition;
        in vec3 vNormal;
        in vec3 vTangent;
        in vec2 vUv;
        flat in float vMaterialClass;

        uniform sampler2D uBaseColor;
        uniform sampler2D uNormalMap;
        uniform sampler2D uRoughnessMap;
        uniform sampler2D uSpecularMap;

        uniform bool uHasBaseColor;
        uniform bool uHasNormal;
        uniform bool uHasRoughness;
        uniform bool uHasSpecular;
        uniform vec3 uCameraPosition;

        out vec4 fragColor;

        vec3 getNormal() {
            vec3 N = normalize(vNormal);

            if (!uHasNormal ||
                vMaterialClass > 0.5) {
                return N;
            }

            vec3 T = normalize(
                vTangent -
                dot(vTangent, N) * N);

            vec3 B =
                normalize(cross(N, T));

            mat3 TBN = mat3(T, B, N);

            vec3 tangentNormal =
                texture(
                    uNormalMap,
                    vUv).xyz * 2.0 - 1.0;

            return normalize(
                TBN * tangentNormal);
        }

        void main() {
            bool isEye =
                vMaterialClass > 0.5 &&
                vMaterialClass < 1.5;

            bool isTeeth =
                vMaterialClass >= 1.5;

            vec3 baseColor;

            if (isEye) {
                baseColor =
                    vec3(
                        0.72,
                        0.76,
                        0.79);
            } else if (isTeeth) {
                baseColor =
                    vec3(
                        0.88,
                        0.84,
                        0.72);
            } else {
                baseColor =
                    uHasBaseColor
                        ? pow(
                            texture(
                                uBaseColor,
                                vUv).rgb,
                            vec3(2.2))
                        : vec3(
                            0.55,
                            0.43,
                            0.38);
            }

            float roughness =
                isEye
                    ? 0.08
                    : (
                        isTeeth
                            ? 0.28
                            : (
                                uHasRoughness
                                    ? clamp(
                                        texture(
                                            uRoughnessMap,
                                            vUv).r,
                                        0.04,
                                        1.0)
                                    : 0.58));

            float specular =
                isEye
                    ? 0.92
                    : (
                        isTeeth
                            ? 0.55
                            : (
                                uHasSpecular
                                    ? clamp(
                                        texture(
                                            uSpecularMap,
                                            vUv).r,
                                        0.0,
                                        1.0)
                                    : 0.32));

            vec3 N = getNormal();

            vec3 L =
                normalize(
                    vec3(
                        0.35,
                        0.65,
                        0.70));

            vec3 V =
                normalize(
                    uCameraPosition -
                    vWorldPosition);

            vec3 H =
                normalize(L + V);

            float NoL =
                max(dot(N, L), 0.0);

            float NoH =
                max(dot(N, H), 0.0);

            float shininess =
                mix(
                    120.0,
                    8.0,
                    roughness);

            float specularTerm =
                pow(NoH, shininess) *
                specular;

            // Soft wrap-lighting approximation gives skin a
            // subtle subsurface-like response without requiring
            // a full screen-space SSS pass.
            float wrappedDiffuse =
                clamp(
                    (dot(N, L) + 0.35) /
                    1.35,
                    0.0,
                    1.0);

            vec3 warmScatter =
                vMaterialClass < 0.5
                    ? vec3(
                        1.0,
                        0.24,
                        0.15) *
                      pow(
                        1.0 - NoL,
                        2.0) *
                      0.07
                    : vec3(0.0);

            vec3 ambient =
                baseColor * 0.18;

            vec3 diffuse =
                baseColor *
                wrappedDiffuse *
                0.78;

            vec3 highlight =
                vec3(1.0) *
                specularTerm *
                0.55;

            vec3 linearColor =
                ambient +
                diffuse +
                highlight +
                warmScatter *
                baseColor;

            vec3 srgb =
                pow(
                    max(
                        linearColor,
                        vec3(0.0)),
                    vec3(1.0 / 2.2));

            fragColor =
                vec4(srgb, 1.0);
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
        0.022F,
        0.028F,
        0.038F,
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
            meshRadius_ *
                3.2F /
                zoom_,
            0.1F);

    QMatrix4x4 view;

    view.lookAt(
        QVector3D(
            0.0F,
            0.0F,
            distance),
        QVector3D(
            0.0F,
            0.0F,
            0.0F),
        QVector3D(
            0.0F,
            1.0F,
            0.0F));

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
        projection *
        view *
        model;

    shader_.bind();

    shader_.setUniformValue(
        "uMvp",
        mvp);

    shader_.setUniformValue(
        "uModel",
        model);

    shader_.setUniformValue(
        "uCameraPosition",
        QVector3D(
            0.0F,
            0.0F,
            distance));

    shader_.setUniformValue(
        "uHasBaseColor",
        baseColorTexture_ != nullptr);

    shader_.setUniformValue(
        "uHasNormal",
        normalTexture_ != nullptr);

    shader_.setUniformValue(
        "uHasRoughness",
        roughnessTexture_ != nullptr);

    shader_.setUniformValue(
        "uHasSpecular",
        specularTexture_ != nullptr);

    shader_.setUniformValue(
        "uBaseColor",
        0);

    shader_.setUniformValue(
        "uNormalMap",
        1);

    shader_.setUniformValue(
        "uRoughnessMap",
        2);

    shader_.setUniformValue(
        "uSpecularMap",
        3);

    if (baseColorTexture_) {
        baseColorTexture_->bind(0);
    }

    if (normalTexture_) {
        normalTexture_->bind(1);
    }

    if (roughnessTexture_) {
        roughnessTexture_->bind(2);
    }

    if (specularTexture_) {
        specularTexture_->bind(3);
    }

    vao_.bind();

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(
            mesh_.indices.size()),
        GL_UNSIGNED_INT,
        nullptr);

    vao_.release();

    shader_.release();
}

void MetaHumanViewportWidget::mousePressEvent(
    QMouseEvent* event) {

    lastMousePosition_ =
        event
            ->position()
            .toPoint();
}

void MetaHumanViewportWidget::mouseMoveEvent(
    QMouseEvent* event) {

    if (!(event->buttons() &
          Qt::LeftButton)) {
        return;
    }

    const QPoint position =
        event
            ->position()
            .toPoint();

    const QPoint delta =
        position -
        lastMousePosition_;

    lastMousePosition_ =
        position;

    yawDegrees_ +=
        static_cast<float>(
            delta.x()) *
        0.45F;

    pitchDegrees_ +=
        static_cast<float>(
            delta.y()) *
        0.35F;

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
            event
                ->angleDelta()
                .y()) /
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

    for (const auto& vertex :
         mesh_.vertices) {

        gpuVertices_.push_back(
            {
                vertex.px,
                vertex.py,
                vertex.pz,
                vertex.nx,
                vertex.ny,
                vertex.nz,
                vertex.u,
                vertex.v,
                1.0F,
                0.0F,
                0.0F,
                vertex.materialClass
            });
    }
}

void MetaHumanViewportWidget::recalculateTangents() {
    for (auto& vertex :
         gpuVertices_) {
        vertex.tx = 0.0F;
        vertex.ty = 0.0F;
        vertex.tz = 0.0F;
    }

    for (std::size_t i = 0;
         i + 2 < mesh_.indices.size();
         i += 3) {

        const auto i0 =
            mesh_.indices[i + 0];

        const auto i1 =
            mesh_.indices[i + 1];

        const auto i2 =
            mesh_.indices[i + 2];

        if (i0 >= gpuVertices_.size() ||
            i1 >= gpuVertices_.size() ||
            i2 >= gpuVertices_.size()) {
            continue;
        }

        auto& v0 = gpuVertices_[i0];
        auto& v1 = gpuVertices_[i1];
        auto& v2 = gpuVertices_[i2];

        const QVector3D p0(
            v0.px,
            v0.py,
            v0.pz);

        const QVector3D p1(
            v1.px,
            v1.py,
            v1.pz);

        const QVector3D p2(
            v2.px,
            v2.py,
            v2.pz);

        const QVector2D uv0(
            v0.u,
            v0.v);

        const QVector2D uv1(
            v1.u,
            v1.v);

        const QVector2D uv2(
            v2.u,
            v2.v);

        const QVector3D edge1 =
            p1 - p0;

        const QVector3D edge2 =
            p2 - p0;

        const QVector2D duv1 =
            uv1 - uv0;

        const QVector2D duv2 =
            uv2 - uv0;

        const float denominator =
            duv1.x() * duv2.y() -
            duv1.y() * duv2.x();

        if (std::fabs(denominator) <
            0.0000001F) {
            continue;
        }

        const float inverse =
            1.0F /
            denominator;

        const QVector3D tangent =
            (
                edge1 * duv2.y() -
                edge2 * duv1.y()
            ) * inverse;

        for (const auto index :
             {i0, i1, i2}) {
            auto& vertex =
                gpuVertices_[index];

            vertex.tx += tangent.x();
            vertex.ty += tangent.y();
            vertex.tz += tangent.z();
        }
    }

    for (auto& vertex :
         gpuVertices_) {

        QVector3D tangent(
            vertex.tx,
            vertex.ty,
            vertex.tz);

        if (tangent.lengthSquared() <
            0.000001F) {
            tangent =
                QVector3D(
                    1.0F,
                    0.0F,
                    0.0F);
        } else {
            tangent.normalize();
        }

        vertex.tx = tangent.x();
        vertex.ty = tangent.y();
        vertex.tz = tangent.z();
    }
}

void MetaHumanViewportWidget::buildNeutralJointGlobals() {
    neutralJointGlobals_.clear();
    inverseNeutralJointGlobals_.clear();

    neutralJointGlobals_.resize(
        mesh_.joints.size());

    inverseNeutralJointGlobals_.resize(
        mesh_.joints.size());

    for (std::size_t i = 0;
         i < mesh_.joints.size();
         ++i) {

        const auto& joint =
            mesh_.joints[i];

        const QMatrix4x4 local =
            makeLocalMatrix(joint);

        const std::uint16_t parent =
            joint.parentIndex;

        if (parent < i &&
            parent <
                neutralJointGlobals_.size()) {

            neutralJointGlobals_[i] =
                neutralJointGlobals_[parent] *
                local;

        } else {
            neutralJointGlobals_[i] =
                local;
        }

        bool invertible = false;

        inverseNeutralJointGlobals_[i] =
            neutralJointGlobals_[i]
                .inverted(&invertible);

        if (!invertible) {
            inverseNeutralJointGlobals_[i] =
                QMatrix4x4();
        }
    }
}

void MetaHumanViewportWidget::applyJointSkinning(
    const bdfr::metahuman::MetaHumanRigOutput& output) {

    constexpr std::size_t stride = 10;

    if (mesh_.joints.empty() ||
        neutralJointGlobals_.size() !=
            mesh_.joints.size() ||
        inverseNeutralJointGlobals_.size() !=
            mesh_.joints.size() ||
        output.jointValues.size() <
            mesh_.joints.size() *
                stride ||
        mesh_.skinInfluences.size() !=
            gpuVertices_.size()) {
        return;
    }

    std::vector<QMatrix4x4>
        animatedGlobals(
            mesh_.joints.size());

    for (std::size_t i = 0;
         i < mesh_.joints.size();
         ++i) {

        const auto& bind =
            mesh_.joints[i];

        const std::size_t base =
            i * stride;

        QMatrix4x4 local;

        local.translate(
            bind.tx +
                output.jointValues[
                    base + 0],
            bind.ty +
                output.jointValues[
                    base + 1],
            bind.tz +
                output.jointValues[
                    base + 2]);

        const QQuaternion bindRotation(
            bind.qw,
            bind.qx,
            bind.qy,
            bind.qz);

        const QQuaternion deltaRotation =
            normalizedDeltaQuaternion(
                output.jointValues[
                    base + 3],
                output.jointValues[
                    base + 4],
                output.jointValues[
                    base + 5],
                output.jointValues[
                    base + 6]);

        local.rotate(
            bindRotation *
            deltaRotation);

        local.scale(
            bind.sx +
                output.jointValues[
                    base + 7],
            bind.sy +
                output.jointValues[
                    base + 8],
            bind.sz +
                output.jointValues[
                    base + 9]);

        const std::uint16_t parent =
            bind.parentIndex;

        if (parent < i &&
            parent <
                animatedGlobals.size()) {

            animatedGlobals[i] =
                animatedGlobals[parent] *
                local;

        } else {
            animatedGlobals[i] =
                local;
        }
    }

    for (std::size_t vertexIndex = 0;
         vertexIndex < gpuVertices_.size();
         ++vertexIndex) {

        const auto& influences =
            mesh_.skinInfluences[
                vertexIndex];

        if (influences.empty()) {
            continue;
        }

        const auto original =
            gpuVertices_[vertexIndex];

        const QVector3D originalPosition(
            original.px,
            original.py,
            original.pz);

        const QVector3D originalNormal(
            original.nx,
            original.ny,
            original.nz);

        QVector3D skinnedPosition(
            0.0F,
            0.0F,
            0.0F);

        QVector3D skinnedNormal(
            0.0F,
            0.0F,
            0.0F);

        float totalWeight = 0.0F;

        for (const auto& influence :
             influences) {

            if (influence.jointIndex >=
                animatedGlobals.size()) {
                continue;
            }

            const QMatrix4x4 skinMatrix =
                animatedGlobals[
                    influence.jointIndex] *
                inverseNeutralJointGlobals_[
                    influence.jointIndex];

            skinnedPosition +=
                (
                    skinMatrix *
                    originalPosition
                ) *
                influence.weight;

            skinnedNormal +=
                skinMatrix
                    .mapVector(
                        originalNormal) *
                influence.weight;

            totalWeight +=
                influence.weight;
        }

        if (totalWeight <= 0.00001F) {
            continue;
        }

        skinnedPosition /=
            totalWeight;

        if (skinnedNormal.lengthSquared() >
            0.000001F) {
            skinnedNormal.normalize();
        } else {
            skinnedNormal =
                originalNormal;
        }

        auto& vertex =
            gpuVertices_[vertexIndex];

        vertex.px =
            skinnedPosition.x();

        vertex.py =
            skinnedPosition.y();

        vertex.pz =
            skinnedPosition.z();

        vertex.nx =
            skinnedNormal.x();

        vertex.ny =
            skinnedNormal.y();

        vertex.nz =
            skinnedNormal.z();
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

    shader_.enableAttributeArray(3);

    shader_.setAttributeBuffer(
        3,
        GL_FLOAT,
        offsetof(GpuVertex, tx),
        3,
        sizeof(GpuVertex));

    shader_.enableAttributeArray(4);

    shader_.setAttributeBuffer(
        4,
        GL_FLOAT,
        offsetof(GpuVertex, materialClass),
        1,
        sizeof(GpuVertex));

    shader_.release();

    indexBuffer_.release();
    vertexBuffer_.release();
    vao_.release();
}

void MetaHumanViewportWidget::updateCameraBounds() {
    if (mesh_.vertices.empty()) {
        meshCenter_ =
            QVector3D(
                0.0F,
                0.0F,
                0.0F);

        meshRadius_ =
            1.0F;

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

    for (const auto& vertex :
         mesh_.vertices) {

        minPoint.setX(
            std::min(
                minPoint.x(),
                vertex.px));

        minPoint.setY(
            std::min(
                minPoint.y(),
                vertex.py));

        minPoint.setZ(
            std::min(
                minPoint.z(),
                vertex.pz));

        maxPoint.setX(
            std::max(
                maxPoint.x(),
                vertex.px));

        maxPoint.setY(
            std::max(
                maxPoint.y(),
                vertex.py));

        maxPoint.setZ(
            std::max(
                maxPoint.z(),
                vertex.pz));
    }

    meshCenter_ =
        (minPoint + maxPoint) *
        0.5F;

    meshRadius_ =
        std::max(
            (
                maxPoint -
                minPoint
            ).length() *
                0.5F,
            0.001F);
}
