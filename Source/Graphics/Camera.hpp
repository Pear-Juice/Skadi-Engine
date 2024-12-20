//
// Created by blankitte on 12/16/24.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP

class Camera {
    UniformBufferObject ubo = {glm::mat4(1), glm::mat4(1)};
    float depthOfField = 0;
    float viewportWidth = 100;
    float viewportHeight = 100;
    float nearPlane = 0.1;
    float farPlane = 1000;

public:
    Camera(){};
    Camera(glm::mat4 transform) {
        depthOfField = 0;
        ubo.view = transform;

        regenerateUBO();
    }

    Camera(glm::mat4 transform, float depthOfField, float viewportWidth, float viewportHeight, float nearPlane = 0.1, float farPlane = 1000) {
        this->depthOfField = depthOfField;
        this->viewportWidth = viewportWidth;
        this->viewportHeight = viewportHeight;
        this->nearPlane = nearPlane;
        this->farPlane = farPlane;
        this->ubo.view = transform;

        regenerateUBO();
    }

    UniformBufferObject getUBO() {
        return ubo;
    }

    void regenerateUBO() {
        if (depthOfField == 0) {
            ubo.proj = glm::ortho(0.0f, viewportWidth, 0.0f, viewportHeight);
        }
        else {
            ubo.proj = glm::perspective(glm::radians(depthOfField), viewportWidth / viewportHeight, nearPlane, farPlane);
        }

	    ubo.proj[1][1] *= -1;
    }

    void setDepthOfField(float degrees) {
        depthOfField = degrees;
        regenerateUBO();
    }

    void setViewport(float width, float height) {
        viewportWidth = width;
        viewportHeight = height;
        regenerateUBO();
    }

    void setTransform(glm::mat4 transform) {
        ubo.view = transform;
    }

    void setNearFar(float near, float far) {
        nearPlane = near;
        farPlane = far;

        regenerateUBO();
    }
};

#endif //CAMERA_HPP
