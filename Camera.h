#ifndef CAMERA_H
#define CAMERA_H

class Camera {
public:
    // hold the state for the whole engine
    static float x;
    static float y;
    static float zoom;

    // Function Headers
    static void SetPosition(float new_x, float new_y);
    static float GetPositionX();
    static float GetPositionY();
    static void SetZoom(float new_zoom);
    static float GetZoom();
};

#endif