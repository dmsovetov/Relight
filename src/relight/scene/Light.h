/**************************************************************************

 The MIT License (MIT)

 Copyright (c) 2015 Dmitry Sovetov

 https://github.com/dmsovetov

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 **************************************************************************/

#ifndef __Relight_Light_H__
#define __Relight_Light_H__

#include "Relight.h"
#include "Vector.h"

namespace relight {

    /*!
     A supported light type enumeration.
     */
    enum LightType {
        LightPoint,
        AreaLight,

        TotalLightTypes
    };

    /*!
     Light distance attenuation model.
     */
    class LightAttenuation {
    public:

        virtual             ~LightAttenuation( void ) {}

        //! Calculates light attenuation by distance.
        /*!
         An attenuation factor is a value in range [0;1], where 0 means
         that light is attenuated to zero, and 1 means no attenuation at all.
         */
        virtual float       calculate( const Light* light, float distance ) const { return 0.0f; }
    };

    /*!
     Linear light attenuation model.
     */
    class LinearLightAttenuation : public LightAttenuation {
    public:

        // ** LinearLightAttenuation
        virtual float       calculate( const Light* light, float distance ) const;
    };

    /*!
     Photon emitter is used to determine an amount of photons to be emitted by a given light.
     */
    class PhotonEmitter {
    public:

                            //! Constructs a new PhotonEmitter instance.
                            PhotonEmitter( const Light* light );
        virtual             ~PhotonEmitter( void ) {}

        //! Calculates an amount of photons to be emitted.
        virtual int         photonCount( void ) const;

        //! Emits a new photon.
        virtual Vec3        emit( void ) const;

    private:

        //! Parent light source.
        const Light*        m_light;
    };

    /*!
     A base class for all light types in Relight.
     */
    class Light {
    public:

        virtual             ~Light( void );

        //! Returns a light type.
        virtual LightType   type( void ) const;

        //! Returns an attenuation model.
        LightAttenuation*   attenuation( void ) const;

        //! Sets an attenuation model.
        void                setAttenuation( LightAttenuation* value );

        //! Returns a photon emitter.
        PhotonEmitter*      photonEmitter( void ) const;

        //! Sets a photon emitter.
        /*!
         By default each light has a photon emitter, light sources with no photon emitter
         set won't make any influence to a global illumination.
         */
        void                setPhotonEmitter( PhotonEmitter* value );

        //! Returns a light position.
        const Vec3&         position( void ) const;

        //! Sets a light position.
        void                setPosition( const Vec3& value );

        //! Returns a light color.
        const Color&        color( void ) const;

        //! Sets a light color.
        void                setColor( const Color& value );

        //! Returns a light intensity.
        float               intensity( void ) const;

        //! Sets a light intensity.
        void                setIntensity( float value );

        //! Returns true if this light casts shadow otherwise false.
        bool                castsShadow( void ) const;

        //! Sets a castShadow flag.
        void                setCastsShadow( bool value );

    protected:

                            //! Constructs a Light instance.
                            Light( LightType type = TotalLightTypes );

    private:

        //! Light type.
        LightType           m_type;

        //! Light position.
        Vec3                m_position;

        //! Light color.
        Color               m_color;

        //! Light intensity.
        float               m_intensity;

        //! Casts shadow flag.
        bool                m_castsShadow;

        //! Light attenuation model.
        LightAttenuation*   m_attenuation;

        //! Light source photon emitter.
        PhotonEmitter*      m_photonEmitter;
    };

    /*!
     A point light (omni-directional).
     */
    class PointLight : public Light {
    public:

                            //! Constructs a new PointLight instance.
                            PointLight( void );

        //! Returns a light radius.
        float               radius( void ) const;

        //! Sets a light radius.
        void                setRadius( float value );

        //! Creates a new PointLight instance.
        static PointLight*  create( const Vec3& position, float radius, const Color& color = Color( 1.0f, 1.0f, 1.0f ), float intensity = 1.0f, bool castsShadow = true );

    private:

        //! A light radius.
        float               m_radius;
    };

    /*!
     A spherical area light.
     */
    class SphericalAreaLight : public PointLight {
    public:
    };

} // namespace relight

#endif  /*  !defined( __Relight_Light_H__ ) */
