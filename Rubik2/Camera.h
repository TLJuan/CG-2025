#pragma once
#include "MatrixOperations.h"
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Camera values by Default
const float YAW         = 90.0f;//-90.0f; // Yaw, aka: horizontal rotation
const float PITCH       =  0.0f;  // Pitch. aka: vertical rotation
const float SPEED       =  2.5f;  
const float SENSITIVITY =  0.1f;  //For mouse based input
const float ZOOM        =  60.0f; // FOV

class Camera {
public:
    vec3 Position;
    vec3 Front;
    vec3 Up;
    vec3 Right;
    vec3 WorldUp; //(0,1,0)
    
	float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom; //FOV

    Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Get the view matrix using LookAt
    matrix4 GetViewMatrix() {
        matrix4 view;
        view.LookAt(Position, Position + Front, Up); //eye, target, up
        return view;
    }
    
    // Orbit around a fixed point
    void Orbit(float angleY, float angleX, float radius, const vec3& target) {
        // Clamp vertical angle to prevent flipping(Gimbal lock workaround)
        float clampedAngleX = std::fmax(-PI/2.0f + 0.01f, std::fmin(PI/2.0f - 0.01f, angleX));
        // Calculate new spherical position
        float camX = radius * std::cos(angleY) * std::cos(clampedAngleX);
        float camY = radius * std::sin(clampedAngleX);
        float camZ = radius * std::sin(angleY) * std::cos(clampedAngleX);
        
        Position = vec3(camX + target.x, camY + target.y, camZ + target.z);
        
        // Recalculate Front vector (keep it always pointing at the target)
        Front = (target - Position).normalize();
        updateCameraVectors();
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors() {
        // Calculate the new Front vector
        vec3 front;
        //sample code for first-person (free-look) camera, 
        Right = Front.cross(WorldUp).normalize();
		
		Up = Right.cross(Front).normalize();
        
		// front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        // front.y = sin(glm::radians(Pitch));
        // front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        // Front = front.normalize();
        // Right = glm::normalize(glm::cross(Front, WorldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        // Up    = glm::normalize(glm::cross(Right, Front));
    }
};