#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace gps {

    struct BoundingBox {
        glm::vec3 min;
        glm::vec3 max;
    };

    enum MOVE_DIRECTION {
        MOVE_FORWARD,
        MOVE_BACKWARD,
        MOVE_LEFT,
        MOVE_RIGHT
    };

    class Camera {
    public:
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);

        glm::mat4 getViewMatrix();

        void move(MOVE_DIRECTION direction, float speed, const std::vector<BoundingBox>& obstacles);

        void rotate(float pitch, float yaw);

        glm::vec3 getCameraPosition() const;
        glm::vec3 getCameraFront() const;
        glm::vec3 getCameraUp() const;

        void setCameraPosition(const glm::vec3& position);
        void setCameraTarget(const glm::vec3& target);

    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
    };
}

#endif // CAMERA_HPP
