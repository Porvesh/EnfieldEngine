#include "headers/Camera.h"

void CameraBounds::calculateCameraPositions(float camera_offset_x, float camera_offset_y){

    float targetX = camera_offset_x;
    float targetY = camera_offset_y;

    // Apply easing to smoothly transition the camera towards the target positions.
    cam_x_pos += (targetX - user_camera_x_pos) * cam_ease_factor;
    cam_y_pos += (targetY - user_camera_y_pos) * cam_ease_factor;
}

void CameraBounds::updateZoomFactorFromDocument(const rapidjson::Document& doc) {
    if (doc.HasMember("zoom_factor") && doc["zoom_factor"].IsNumber()) {
        zoom_factor = doc["zoom_factor"].GetDouble();
    }
    if (doc.HasMember("cam_ease_factor") && doc["cam_ease_factor"].IsNumber()) {
        cam_ease_factor = doc["cam_ease_factor"].GetDouble();
    }
}

void CameraBounds::Intialize() {
    halfWidth = (CAMERA_WIDTH / 2);
    halfHeight = (CAMERA_HEIGHT / 2);
    zoom_factor = (1);
    cam_ease_factor = (1);
}
    
void CameraBounds::SetPosition(float x, float y) {
    CameraBounds::user_camera_x_pos = x;
    CameraBounds::user_camera_y_pos = y;
    CameraBounds::cam_x_pos = x;
    CameraBounds::cam_y_pos = y;
}

float CameraBounds::GetPositionX() {
    return CameraBounds::user_camera_x_pos;
}

float CameraBounds::GetPositionY() {
    return CameraBounds::user_camera_y_pos;
}

void CameraBounds::SetZoom(float zoom_factor) {
    CameraBounds::zoom_factor = zoom_factor;
}

float CameraBounds::GetZoom() {
    return CameraBounds::zoom_factor;
}

