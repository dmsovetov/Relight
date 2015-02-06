//-----------------------------------------------------------------------------
// Copyright (c) 2006-2008 dhpoware. All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#if !defined(CAMERA_H)
#define CAMERA_H

#include "mathlib.h"

//-----------------------------------------------------------------------------
// A general purpose 6DoF (six degrees of freedom) vector based camera.
//
// This camera class supports 2 different behaviors:
// first person mode, and flight mode.
//
// First person mode only allows 5DOF (x axis movement, y axis movement, z axis
// movement, yaw, and pitch) and movement is always parallel to the world x-z
// (ground) plane.
//
// Flight mode supports 6DoF. This is the camera class' default behavior.
//
// This camera class allows the camera to be moved in 2 ways: using fixed
// step world units, and using a supplied velocity and acceleration. The former
// simply moves the camera by the specified amount. To move the camera in this
// way call one of the move() methods. The other way to move the camera
// calculates the camera's displacement based on the supplied velocity,
// acceleration, and elapsed time. To move the camera in this way call the
// updatePosition() method.
//-----------------------------------------------------------------------------

class Camera
{
public:
    enum CameraBehavior
    {
        CAMERA_BEHAVIOR_FIRST_PERSON,
        CAMERA_BEHAVIOR_FLIGHT
    };
    
    Camera();
    ~Camera();

    void lookAt(const Vector3 &target);
    void lookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up);
    void move(float dx, float dy, float dz);
    void move(const Vector3 &direction, const Vector3 &amount);
    void perspective(float fovx, float aspect, float znear, float zfar);
    void rotate(float headingDegrees, float pitchDegrees, float rollDegrees);
    void updatePosition(const Vector3 &direction, float elapsedTimeSec);
    
    // Getter methods.

    const Vector3 &getAcceleration() const;
    CameraBehavior getBehavior() const;
    const Vector3 &getCurrentVelocity() const;
    const Vector3 &getPosition() const;
    const Matrix4 &getProjectionMatrix() const;
    const Vector3 &getVelocity() const;
    const Vector3 &getViewDirection() const;
    const Matrix4 &getViewMatrix() const;
    const Vector3 &getXAxis() const;
    const Vector3 &getYAxis() const;
    const Vector3 &getZAxis() const;
    
    // Setter methods.

    void setAcceleration(float x, float y, float z);
    void setAcceleration(const Vector3 &acceleration);
    void setBehavior(CameraBehavior newBehavior);
    void setCurrentVelocity(float x, float y, float z);
    void setCurrentVelocity(const Vector3 &currentVelocity);
    void setPosition(float x, float y, float z);
    void setPosition(const Vector3 &position);
    void setVelocity(float x, float y, float z);
    void setVelocity(const Vector3 &velocity);

private:
    void rotateFlight(float headingDegrees, float pitchDegrees, float rollDegrees);
    void rotateFirstPerson(float headingDegrees, float pitchDegrees);
    void updateVelocity(const Vector3 &direction, float elapsedTimeSec);
    void updateViewMatrix(bool orthogonalizeAxes);
    
    static const float DEFAULT_FOVX;
    static const float DEFAULT_ZFAR;
    static const float DEFAULT_ZNEAR;
    static const Vector3 WORLD_XAXIS;
    static const Vector3 WORLD_YAXIS;
    static const Vector3 WORLD_ZAXIS;

    CameraBehavior m_behavior;
    float m_fovx;
    float m_znear;
    float m_zfar;
    float m_aspectRatio;
    float m_accumPitchDegrees;
    Vector3 m_eye;
    Vector3 m_xAxis;
    Vector3 m_yAxis;
    Vector3 m_zAxis;
    Vector3 m_viewDir;
    Vector3 m_acceleration;
    Vector3 m_currentVelocity;
    Vector3 m_velocity;
    Matrix4 m_viewMatrix;
    Matrix4 m_projMatrix;
};

//-----------------------------------------------------------------------------

inline const Vector3 &Camera::getAcceleration() const
{ return m_acceleration; }

inline Camera::CameraBehavior Camera::getBehavior() const
{ return m_behavior; }

inline const Vector3 &Camera::getCurrentVelocity() const
{ return m_currentVelocity; }

inline const Vector3 &Camera::getPosition() const
{ return m_eye; }

inline const Matrix4 &Camera::getProjectionMatrix() const
{ return m_projMatrix; }

inline const Vector3 &Camera::getVelocity() const
{ return m_velocity; }

inline const Vector3 &Camera::getViewDirection() const
{ return m_viewDir; }

inline const Matrix4 &Camera::getViewMatrix() const
{ return m_viewMatrix; }

inline const Vector3 &Camera::getXAxis() const
{ return m_xAxis; }

inline const Vector3 &Camera::getYAxis() const
{ return m_yAxis; }

inline const Vector3 &Camera::getZAxis() const
{ return m_zAxis; }

#endif