#include "Camera.hpp"
#include <iostream>

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
        this->cameraUpDirection = glm::cross(cameraFrontDirection, cameraRightDirection);
    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }

    bool isInsideBox(const glm::vec3& position, const BoundingBox& box) {
        return (position.x > box.min.x && position.x < box.max.x &&
                position.y > box.min.y && position.y < box.max.y &&
                position.z > box.min.z && position.z < box.max.z);
    }

    void Camera::move(MOVE_DIRECTION direction, float speed, const std::vector<BoundingBox>& obstacles) {
        glm::vec3 newPosition = cameraPosition;

        if (direction == gps::MOVE_FORWARD)
            newPosition += speed * cameraFrontDirection;
        else if (direction == gps::MOVE_BACKWARD)
            newPosition -= speed * cameraFrontDirection;
        else if (direction == gps::MOVE_LEFT)
            newPosition -= speed * cameraRightDirection;
        else if (direction == gps::MOVE_RIGHT)
            newPosition += speed * cameraRightDirection;

        for (const auto& box : obstacles) {
            if (isInsideBox(newPosition, box)) {
                std::cout << "Collision detected! Movement canceled.\n";
                return;
            }
        }

        cameraPosition = newPosition;
        cameraTarget = cameraPosition + cameraFrontDirection;
    }

    void Camera::rotate(float pitch, float yaw) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(front);

        glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, globalUp));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    glm::vec3 Camera::getCameraPosition() const {
        return cameraPosition;
    }

    glm::vec3 Camera::getCameraFront() const {
        return cameraFrontDirection;
    }

    glm::vec3 Camera::getCameraUp() const {
        return cameraUpDirection;
    }

    void Camera::setCameraPosition(const glm::vec3& position) {
        this->cameraPosition = position;
        this->cameraTarget = cameraPosition + cameraFrontDirection;
    }

    void Camera::setCameraTarget(const glm::vec3& target) {
        this->cameraTarget = target;
        this->cameraFrontDirection = glm::normalize(target - cameraPosition);

        glm::vec3 globalUp = glm::vec3(0.0f, 1.0f, 0.0f);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, globalUp));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

}
