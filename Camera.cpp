#include "Camera.h"

// Initialize the static variables
float Camera::x = 0.0f;
float Camera::y = 0.0f;
float Camera::zoom = 1.0f;

void Camera::SetPosition(float new_x, float new_y) {
    x = new_x;
    y = new_y;
}

float Camera::GetPositionX() {
    return x;
}

float Camera::GetPositionY() {
    return y;
}

void Camera::SetZoom(float new_zoom) {
    zoom = new_zoom;
}

float Camera::GetZoom() {
    return zoom;
}