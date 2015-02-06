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

#include <cmath>
#include "camera.h"

const float Camera::DEFAULT_FOVX = 90.0f;
const float Camera::DEFAULT_ZFAR = 1000.0f;
const float Camera::DEFAULT_ZNEAR = 0.1f;
const Vector3 Camera::WORLD_XAXIS(1.0f, 0.0f, 0.0f);
const Vector3 Camera::WORLD_YAXIS(0.0f, 1.0f, 0.0f);
const Vector3 Camera::WORLD_ZAXIS(0.0f, 0.0f, 1.0f);

Camera::Camera()
{
    m_behavior = CAMERA_BEHAVIOR_FLIGHT;
    
    m_fovx = DEFAULT_FOVX;
    m_znear = DEFAULT_ZNEAR;
    m_zfar = DEFAULT_ZFAR;
    m_aspectRatio = 0.0f;
    
    m_accumPitchDegrees = 0.0f;
    
    m_eye.set(0.0f, 0.0f, 0.0f);
    m_xAxis.set(1.0f, 0.0f, 0.0f);
    m_yAxis.set(0.0f, 1.0f, 0.0f);
    m_zAxis.set(0.0f, 0.0f, 1.0f);
    m_viewDir.set(0.0f, 0.0f, -1.0f);
    
    m_acceleration.set(0.0f, 0.0f, 0.0f);
    m_currentVelocity.set(0.0f, 0.0f, 0.0f);
    m_velocity.set(0.0f, 0.0f, 0.0f);
    
    m_viewMatrix.identity();
    m_projMatrix.identity();
}

Camera::~Camera()
{
}

void Camera::lookAt(const Vector3 &target)
{
    lookAt(m_eye, target, m_yAxis);
}

void Camera::lookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up)
{
    m_eye = eye;

    m_zAxis = eye - target;
    m_zAxis.normalize();

    m_viewDir = -m_zAxis;

    m_xAxis = Vector3::cross(up, m_zAxis);
    m_xAxis.normalize();

    m_yAxis = Vector3::cross(m_zAxis, m_xAxis);
    m_yAxis.normalize();
    m_xAxis.normalize();

    m_viewMatrix[0][0] = m_xAxis.x;
    m_viewMatrix[1][0] = m_xAxis.y;
    m_viewMatrix[2][0] = m_xAxis.z;
    m_viewMatrix[3][0] = -Vector3::dot(m_xAxis, eye);

    m_viewMatrix[0][1] = m_yAxis.x;
    m_viewMatrix[1][1] = m_yAxis.y;
    m_viewMatrix[2][1] = m_yAxis.z;
    m_viewMatrix[3][1] = -Vector3::dot(m_yAxis, eye);

    m_viewMatrix[0][2] = m_zAxis.x;
    m_viewMatrix[1][2] = m_zAxis.y;
    m_viewMatrix[2][2] = m_zAxis.z;    
    m_viewMatrix[3][2] = -Vector3::dot(m_zAxis, eye);

    // Extract the pitch angle from the view matrix.
    m_accumPitchDegrees = Math::radiansToDegrees(-asinf(m_viewMatrix[1][2]));
}

void Camera::move(float dx, float dy, float dz)
{
    // Moves the camera by dx world units to the left or right; dy
    // world units upwards or downwards; and dz world units forwards
    // or backwards.

    Vector3 eye = m_eye;
    Vector3 forwards;

    if (m_behavior == CAMERA_BEHAVIOR_FIRST_PERSON)
    {
        // Calculate the forwards direction. Can't just use the camera's local
        // z axis as doing so will cause the camera to move more slowly as the
        // camera's view approaches 90 degrees straight up and down.

        forwards = Vector3::cross(WORLD_YAXIS, m_xAxis);
        forwards.normalize();
    }
    else
    {
        forwards = m_viewDir;
    }
    
    eye += m_xAxis * dx;
    eye += WORLD_YAXIS * dy;
    eye += forwards * dz;
    
    setPosition(eye);
}

void Camera::move(const Vector3 &direction, const Vector3 &amount)
{
    // Moves the camera by the specified amount of world units in the specified
    // direction in world space.

    m_eye.x += direction.x * amount.x;
    m_eye.y += direction.y * amount.y;
    m_eye.z += direction.z * amount.z;

    updateViewMatrix(false);
}

void Camera::perspective(float fovx, float aspect, float znear, float zfar)
{
    // Construct a projection matrix based on the horizontal field of view
    // 'fovx' rather than the more traditional vertical field of view 'fovy'.

    float e = 1.0f / tanf(Math::degreesToRadians(fovx) / 2.0f);
    float aspectInv = 1.0f / aspect;
    float fovy = 2.0f * atanf(aspectInv / e);
    float xScale = 1.0f / tanf(0.5f * fovy);
    float yScale = xScale / aspectInv;

    m_projMatrix[0][0] = xScale;
    m_projMatrix[0][1] = 0.0f;
    m_projMatrix[0][2] = 0.0f;
    m_projMatrix[0][3] = 0.0f;

    m_projMatrix[1][0] = 0.0f;
    m_projMatrix[1][1] = yScale;
    m_projMatrix[1][2] = 0.0f;
    m_projMatrix[1][3] = 0.0f;

    m_projMatrix[2][0] = 0.0f;
    m_projMatrix[2][1] = 0.0f;
    m_projMatrix[2][2] = (zfar + znear) / (znear - zfar);
    m_projMatrix[2][3] = -1.0f;

    m_projMatrix[3][0] = 0.0f;
    m_projMatrix[3][1] = 0.0f;
    m_projMatrix[3][2] = (2.0f * zfar * znear) / (znear - zfar);
    m_projMatrix[3][3] = 0.0f;

    m_fovx = fovx;
    m_aspectRatio = aspect;
    m_znear = znear;
    m_zfar = zfar;
}

void Camera::rotate(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    // Rotates the camera based on its current behavior.
    // Note that not all behaviors support rolling.

    switch (m_behavior)
    {
    case CAMERA_BEHAVIOR_FIRST_PERSON:
        rotateFirstPerson(headingDegrees, pitchDegrees);
        break;

    case CAMERA_BEHAVIOR_FLIGHT:
        rotateFlight(headingDegrees, pitchDegrees, rollDegrees);
        break;
    }

    updateViewMatrix(true);
}

void Camera::rotateFlight(float headingDegrees, float pitchDegrees, float rollDegrees)
{
    Matrix4 rotMtx;

    // Rotate camera's existing x and z axes about its existing y axis.
    if (headingDegrees != 0.0f)
    {
        rotMtx.rotate(m_yAxis, headingDegrees);
        m_xAxis = m_xAxis * rotMtx;
        m_zAxis = m_zAxis * rotMtx;
    }

    // Rotate camera's existing y and z axes about its existing x axis.
    if (pitchDegrees != 0.0f)
    {
        rotMtx.rotate(m_xAxis, pitchDegrees);
        m_yAxis = m_yAxis * rotMtx;
        m_zAxis = m_zAxis * rotMtx;
    }

    // Rotate camera's existing x and y axes about its existing z axis.
    if (rollDegrees != 0.0f)
    {
        rotMtx.rotate(m_zAxis, rollDegrees);
        m_xAxis = m_xAxis * rotMtx;
        m_yAxis = m_yAxis * rotMtx;
    }
}

void Camera::rotateFirstPerson(float headingDegrees, float pitchDegrees)
{
    m_accumPitchDegrees += pitchDegrees;

    if (m_accumPitchDegrees > 90.0f)
    {
        pitchDegrees = 90.0f - (m_accumPitchDegrees - pitchDegrees);
        m_accumPitchDegrees = 90.0f;
    }

    if (m_accumPitchDegrees < -90.0f)
    {
        pitchDegrees = -90.0f - (m_accumPitchDegrees - pitchDegrees);
        m_accumPitchDegrees = -90.0f;
    }

    Matrix4 rotMtx;

    // Rotate camera's existing x and z axes about the world y axis.
    if (headingDegrees != 0.0f)
    {
        rotMtx.rotate(WORLD_YAXIS, headingDegrees);
        m_xAxis = m_xAxis * rotMtx;
        m_zAxis = m_zAxis * rotMtx;
    }

    // Rotate camera's existing y and z axes about its existing x axis.
    if (pitchDegrees != 0.0f)
    {
        rotMtx.rotate(m_xAxis, pitchDegrees);
        m_yAxis = m_yAxis * rotMtx;
        m_zAxis = m_zAxis * rotMtx;
    }
}

void Camera::setAcceleration(float x, float y, float z)
{
    m_acceleration.set(x, y, z);
}

void Camera::setAcceleration(const Vector3 &acceleration)
{
    m_acceleration = acceleration;
}

void Camera::setBehavior(CameraBehavior newBehavior)
{
    if (m_behavior == CAMERA_BEHAVIOR_FLIGHT && newBehavior == CAMERA_BEHAVIOR_FIRST_PERSON)
    {
        // Moving from flight-simulator mode to first-person.
        // Need to ignore camera roll, but retain existing pitch and heading.

        lookAt(m_eye, m_eye + m_zAxis.inverse(), WORLD_YAXIS);
    }

    m_behavior = newBehavior;
}

void Camera::setCurrentVelocity(float x, float y, float z)
{
    m_currentVelocity.set(x, y, z);
}

void Camera::setCurrentVelocity(const Vector3 &currentVelocity)
{
    m_currentVelocity = currentVelocity;
}

void Camera::setPosition(float x, float y, float z)
{
    m_eye.set(x, y, z);
    updateViewMatrix(false);
}

void Camera::setPosition(const Vector3 &position)
{
    m_eye = position;
    updateViewMatrix(false);
}

void Camera::setVelocity(float x, float y, float z)
{
    m_velocity.set(x, y, z);
}

void Camera::setVelocity(const Vector3 &velocity)
{
    m_velocity = velocity;
}

void Camera::updatePosition(const Vector3 &direction, float elapsedTimeSec)
{
    // Moves the camera using Newton's second law of motion. Unit mass is
    // assumed here to somewhat simplify the calculations. The direction vector
    // is in the range [-1,1].

    if (m_currentVelocity.magnitudeSq() != 0.0f)
    {
        // Only move the camera if the velocity vector is not of zero length.
        // Doing this guards against the camera slowly creeping around due to
        // floating point rounding errors.

        Vector3 displacement = (m_currentVelocity * elapsedTimeSec) +
            (0.5f * m_acceleration * elapsedTimeSec * elapsedTimeSec);

        // Floating point rounding errors will slowly accumulate and cause the
        // camera to move along each axis. To prevent any unintended movement
        // the displacement vector is clamped to zero for each direction that
        // the camera isn't moving in. Note that the updateVelocity() method
        // will slowly decelerate the camera's velocity back to a stationary
        // state when the camera is no longer moving along that direction. To
        // account for this the camera's current velocity is also checked.

        if (direction.x == 0.0f && Math::closeEnough(m_currentVelocity.x, 0.0f))
            displacement.x = 0.0f;

        if (direction.y == 0.0f && Math::closeEnough(m_currentVelocity.y, 0.0f))
            displacement.y = 0.0f;

        if (direction.z == 0.0f && Math::closeEnough(m_currentVelocity.z, 0.0f))
            displacement.z = 0.0f;

        move(displacement.x, displacement.y, displacement.z);
    }

    // Continuously update the camera's velocity vector even if the camera
    // hasn't moved during this call. When the camera is no longer being moved
    // the camera is decelerating back to its stationary state.

    updateVelocity(direction, elapsedTimeSec);
}

void Camera::updateVelocity(const Vector3 &direction, float elapsedTimeSec)
{
    // Updates the camera's velocity based on the supplied movement direction
    // and the elapsed time (since this method was last called). The movement
    // direction is in the range [-1,1].

    if (direction.x != 0.0f)
    {
        // Camera is moving along the x axis.
        // Linearly accelerate up to the camera's max speed.

        m_currentVelocity.x += direction.x * m_acceleration.x * elapsedTimeSec;

        if (m_currentVelocity.x > m_velocity.x)
            m_currentVelocity.x = m_velocity.x;
        else if (m_currentVelocity.x < -m_velocity.x)
            m_currentVelocity.x = -m_velocity.x;
    }
    else
    {
        // Camera is no longer moving along the x axis.
        // Linearly decelerate back to stationary state.

        if (m_currentVelocity.x > 0.0f)
        {
            if ((m_currentVelocity.x -= m_acceleration.x * elapsedTimeSec) < 0.0f)
                m_currentVelocity.x = 0.0f;
        }
        else
        {
            if ((m_currentVelocity.x += m_acceleration.x * elapsedTimeSec) > 0.0f)
                m_currentVelocity.x = 0.0f;
        }
    }

    if (direction.y != 0.0f)
    {
        // Camera is moving along the y axis.
        // Linearly accelerate up to the camera's max speed.

        m_currentVelocity.y += direction.y * m_acceleration.y * elapsedTimeSec;

        if (m_currentVelocity.y > m_velocity.y)
            m_currentVelocity.y = m_velocity.y;
        else if (m_currentVelocity.y < -m_velocity.y)
            m_currentVelocity.y = -m_velocity.y;
    }
    else
    {
        // Camera is no longer moving along the y axis.
        // Linearly decelerate back to stationary state.

        if (m_currentVelocity.y > 0.0f)
        {
            if ((m_currentVelocity.y -= m_acceleration.y * elapsedTimeSec) < 0.0f)
                m_currentVelocity.y = 0.0f;
        }
        else
        {
            if ((m_currentVelocity.y += m_acceleration.y * elapsedTimeSec) > 0.0f)
                m_currentVelocity.y = 0.0f;
        }
    }

    if (direction.z != 0.0f)
    {
        // Camera is moving along the z axis.
        // Linearly accelerate up to the camera's max speed.

        m_currentVelocity.z += direction.z * m_acceleration.z * elapsedTimeSec;

        if (m_currentVelocity.z > m_velocity.z)
            m_currentVelocity.z = m_velocity.z;
        else if (m_currentVelocity.z < -m_velocity.z)
            m_currentVelocity.z = -m_velocity.z;
    }
    else
    {
        // Camera is no longer moving along the z axis.
        // Linearly decelerate back to stationary state.

        if (m_currentVelocity.z > 0.0f)
        {
            if ((m_currentVelocity.z -= m_acceleration.z * elapsedTimeSec) < 0.0f)
                m_currentVelocity.z = 0.0f;
        }
        else
        {
            if ((m_currentVelocity.z += m_acceleration.z * elapsedTimeSec) > 0.0f)
                m_currentVelocity.z = 0.0f;
        }
    }
}

void Camera::updateViewMatrix(bool orthogonalizeAxes)
{
    if (orthogonalizeAxes)
    {
        // Regenerate the camera's local axes to orthogonalize them.
        
        m_zAxis.normalize();
        
        m_yAxis = Vector3::cross(m_zAxis, m_xAxis);
        m_yAxis.normalize();
        
        m_xAxis = Vector3::cross(m_yAxis, m_zAxis);
        m_xAxis.normalize();

        m_viewDir = -m_zAxis;
    }

    // Reconstruct the view matrix.

    m_viewMatrix[0][0] = m_xAxis.x;
    m_viewMatrix[1][0] = m_xAxis.y;
    m_viewMatrix[2][0] = m_xAxis.z;
    m_viewMatrix[3][0] = -Vector3::dot(m_xAxis, m_eye);

    m_viewMatrix[0][1] = m_yAxis.x;
    m_viewMatrix[1][1] = m_yAxis.y;
    m_viewMatrix[2][1] = m_yAxis.z;
    m_viewMatrix[3][1] = -Vector3::dot(m_yAxis, m_eye);

    m_viewMatrix[0][2] = m_zAxis.x;    
    m_viewMatrix[1][2] = m_zAxis.y;
    m_viewMatrix[2][2] = m_zAxis.z;   
    m_viewMatrix[3][2] = -Vector3::dot(m_zAxis, m_eye);

    m_viewMatrix[0][3] = 0.0f;
    m_viewMatrix[1][3] = 0.0f;
    m_viewMatrix[2][3] = 0.0f;
    m_viewMatrix[3][3] = 1.0f;
}