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

#include "../Relight.h"

namespace relight {

    /*!
     Light cuttoff is used for configuring the light source influence direction (omni, directional, spotlight).
     LightCutoff class is a base for all cutoff models and represents an omni-directional light.
     */
    class LightCutoff {
    public:

                            //! Constructs a LightCutoff instance.
                            LightCutoff( const Light* light );
        virtual             ~LightCutoff( void ) {}

        //! Calculates a light influence to a given point.
        virtual float       calculate( const Vec3& point ) const;

        //! Calculates a light cutoff for direction.
        virtual float       cutoffForDirection( const Vec3& direction ) const;

    protected:

        //! Parent light instance.
        const Light*        m_light;
    };

    /*!
     LightSpotCutoff is used for spot lights.
     */
    class LightSpotCutoff : public LightCutoff {
    public:

                            //! Constructs a LightSpotCutoff instance
                            /*!
                             \param light Parent light source.
                             \param direction Spot light direction.
                             \param cutoff The cutoff value represents the maximum angle between the light direction and the light to pixel vector.
                             \param exponent The cutoff exponent.
                             */
                            LightSpotCutoff( const Light* light, const Vec3& direction, float cutoff, float exponent );
        virtual             ~LightSpotCutoff( void ) {}

        //! Calculates a light influence for a spot light.
        virtual float       calculate( const Vec3& point ) const;

        //! Calculates a light cutoff for direction.
        virtual float       cutoffForDirection( const Vec3& direction ) const;

    private:

        //! Light direction.
        Vec3                m_direction;

        //! Light cutoff.
        float               m_cutoff;

        //! Light spot exponent.
        float               m_exponent;
    };

    /*!
     Light distance attenuation model.
     */
    class LightAttenuation {
    public:

                            //! Constructs a LightAttenuation instance.
                            LightAttenuation( const Light* light );
        virtual             ~LightAttenuation( void ) {}

        //! Calculates light attenuation by distance.
        /*!
         An attenuation factor is a value in range [0;1], where 0 means
         that light is attenuated to zero, and 1 means no attenuation at all.
         */
        virtual float       calculate( float distance ) const { return 0.0f; }

    protected:

        //! Parent light source.
        const Light*        m_light;
    };

    /*!
     Linear light attenuation model.
     */
    class LinearLightAttenuation : public LightAttenuation {
    public:

                            //! Constructs a LinearLightAttenuation instance.
                            /*!
                             \param light Parent light source.
                             \param radius Light influence radius.
                             */
                            LinearLightAttenuation( const Light* light, float radius );

        // ** LinearLightAttenuation
        virtual float       calculate( float distance ) const;

    private:

        //! Light influence radius.
        float               m_radius;
    };

    /*!
     Light to point influence model.
     */
    class LightInfluence {
    public:

                            //! Constructs a LightInfluence instance.
                            LightInfluence( const Light* light );
        virtual             ~LightInfluence( void ) {}

        //! Calculates omni light influence to a given point.
        virtual float       calculate( rt::ITracer* tracer, const Vec3& light, const Vec3& point, const Vec3& normal, float& distance ) const;

        //! Calculates a light influence by a Lambert's cosine law.
        static float        lambert( const Vec3& direction, const Vec3& normal );

    protected:

        //! Parent light instance.
        const Light*        m_light;
    };

    /*!
     Directional light influence model.
     */
    class DirectionalLightInfluence : public LightInfluence {
    public:

                            //! Constructs a DirectionalLightInfluence instance.
                            DirectionalLightInfluence( const Light* light, const Vec3& direction );

        //! Calculates a directional light influence.
        virtual float       calculate( rt::ITracer* tracer, const Vec3& light, const Vec3& point, const Vec3& normal, float& distance ) const;

    private:

        //! Light source direction.
        Vec3                m_direction;
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
        virtual void        emit( const Scene* scene, Vec3& position, Vec3& direction ) const;

    protected:

        //! Parent light source.
        const Light*        m_light;
    };

    /*!
     Directional photon emitter emits photons with a given direction.
     */
    class DirectionalPhotonEmitter : public PhotonEmitter {
    public:

                            //! Constructs a new DirectionalPhotonEmitter instance.
                            /*!
                             \param light Parent light source.
                             \param direction Emission direction.
                             */
                            DirectionalPhotonEmitter( const Light* light, const Vec3& direction );

        //! Emits a new photon.
        virtual void        emit( const Scene* scene, Vec3& position, Vec3& direction ) const;

    private:

        //! Emission direction.
        Vec3                m_direction;

        //! Emission plane.
        Plane               m_plane;
    };

    /*!
     A base class for all light types in Relight.
     */
    class Light {
    public:

        virtual             ~Light( void );

        //! Returns an light influence model.
        LightInfluence*     influence( void ) const;

        //! Sets an light influence model.
        void                setInfluence( LightInfluence* value );

        //! Returns an attenuation model.
        LightAttenuation*   attenuation( void ) const;

        //! Sets an attenuation model.
        void                setAttenuation( LightAttenuation* value );

        //! Returns a light vertex generator.
        LightVertexGenerator* vertexGenerator( void ) const;

        //! Sets an light vertex generator.
        void                setVertexGenerator( LightVertexGenerator* value );

        //! Returns an cutoff model.
        LightCutoff*        cutoff( void ) const;

        //! Sets an cutoff model.
        void                setCutoff( LightCutoff* value );

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

        //! Creates a point light instance.
        static Light*       createPointLight( const Vec3& position, float radius, const Color& color = Color( 1.0f, 1.0f, 1.0f ), float intensity = 1.0f, bool castsShadow = true );

        //! Creates a spot light instance.
        static Light*       createSpotLight( const Vec3& position, const Vec3& direction, float cutoff, float radius, const Color& color = Color( 1.0f, 1.0f, 1.0f ), float intensity = 1.0f, bool castsShadow = true );

        //! Creates a directional light instance.
        static Light*       createDirectionalLight( const Vec3& direction, const Color& color = Color( 1.0f, 1.0f, 1.0f ), float intensity = 1.0f, bool castsShadow = true );

        //! Creates an area light instance.
        static Light*       createAreaLight( const Mesh* mesh, const Vec3& position, const Color& color = Color( 1.0f, 1.0f, 1.0f ), float intensity = 1.0f, bool castsShadow = true );

    protected:

                            //! Constructs a Light instance.
                            Light( void );

    private:

        //! Light position.
        Vec3                m_position;

        //! Light color.
        Color               m_color;

        //! Light intensity.
        float               m_intensity;

        //! Casts shadow flag.
        bool                m_castsShadow;

        //! Light cutoff model.
        LightCutoff*        m_cutoff;

        //! Light attenuation model.
        LightAttenuation*   m_attenuation;

        //! Light influence model.
        LightInfluence*     m_influence;

        //! Light vertex sampler.
        LightVertexGenerator*   m_vertexGenerator;

        //! Light source photon emitter.
        PhotonEmitter*      m_photonEmitter;
    };

    //! A light vertex is a single light point used with mesh light sources.
    struct LightVertex {
        Vec3                m_position; //!< Point position.
        Vec3                m_normal;   //!< Point normal.
    };

    typedef Array<LightVertex> LightVertexBuffer;

    /*!
     A LightVertexGenerator used to generate a set of light points for mesh light sources.
     */
    class LightVertexGenerator {
    public:

                                    //! Constructs a LightVertexGenerator instance.
                                    LightVertexGenerator( const Mesh* mesh );
        virtual                     ~LightVertexGenerator( void ) {}

        //! Returns an array of light vertices.
        const LightVertexBuffer&    vertices( void ) const;

        //! Generates a set of light vertices based on mesh vertices.
        virtual void                generate( void );

        //! Clears a previously generated data.
        void                        clear( void );

        //! Returns a total number of light vertices.
        int                         vertexCount( void ) const;

    protected:

        //! Adds a new light vertex.
        void                        push( const Vertex& vertex );

    protected:

        //! Source mesh.
        const Mesh*                 m_mesh;

        //! A set of generated light vertices.
        LightVertexBuffer           m_vertices;
    };

    /*!
     A FaceLightVertexGenerator generates a set of vertices based on mesh vertices & face centroids.
     Face based generator can also perform a mesh tesselation.
     */
    class FaceLightVertexGenerator : public LightVertexGenerator {
    public:

                                    //! Constructs a FaceLightVertexGenerator instance.
                                    /*!
                                     \param mesh Source light mesh.
                                     \param excludeVertices The flag indicating that we should skip mesh vertices.
                                     \param maxSubdivisions Maximum subdivision steps per mesh face (0 means no tesselation).
                                     */
                                    FaceLightVertexGenerator( const Mesh* mesh, bool excludeVertices, int maxSubdivisions );

        //! Generates a set of light vertices.
        virtual void                generate( void );

    private:

        //! Generates a set of light vertices from triangle.
        virtual void                generateFromTriangle( const Triangle& triangle, int subdivision );

    private:

        //! The flag indicating that we should skip mesh vertices.
        bool                        m_excludeVertices;

        //! Max subdivision depth.
        int                         m_maxSubdivisions;
    };

} // namespace relight

#endif  /*  !defined( __Relight_Light_H__ ) */
