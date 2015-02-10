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

#ifndef __Relight_Scene_H__
#define __Relight_Scene_H__

#include "Relight.h"

namespace relight {

    /*!
     A bake mask bits.
     */
    enum BakeMask {
        BakeDirect   = 0x1,                         //!< Bake direct light.
        BakeIndirect = 0x2,                         //!< Bake indirect (reflected) light (aka GlobalIllumination).
        BakeAll      = BakeDirect | BakeIndirect    //!< Bakes all light data.
    };

    /*!
     Instance of a Scene class holds all lights, mesh instances
     and rendering settings.
     */
    class Scene {
    public:

        //! Returns a tracer.
        rt::ITracer*            tracer( void ) const;

        //! Creates a new Lightmap instance.
        Lightmap*               createLightmap( int width, int height ) const;

        //! Creates a new PhotonMap instance.
        Photonmap*              createPhotonmap( int width, int height ) const;

        //! Begins a scene creation.
        /*!
         This method sould be called before adding any scene objects.
         */
        RelightStatus           begin( void );

        //! Ens a scene creation.
        /*!
         This method should be called after the scene objects are added and BEFORE
         the Scene::bake call.
         */
        RelightStatus           end( void );

        /*!
         Creates a new Instance and places it on scene.
         \param mesh A mesh data to create an Instance from.
         \param transform mesh instance transform.
         */
        Instance*               addMesh( const Mesh* mesh, const Matrix4& transform );

        /*!
         Adds a Light instance to a scene.
         \param light A Light instance to be added.
         */
        RelightStatus           addLight( const Light* light );

        //! Returns a total amount of lights.
        int                     lightCount( void ) const;

        //! Returns a Light instance by index.
        const Light*            light( int index ) const;

        //! Returns a total amount of mesh instances.
        int                     instanceCount( void ) const;

        //! Returns a Instance by index.
        const Instance*         instance( int index ) const;

        /*!
         Backes a lightmap to a texture.
         \param bake A bake mask.
         */
        RelightStatus           bake( int mask = BakeAll );

        //! Creates a new scene.
        static Scene*           create( void );

    private:

                                //! Constructs a new Scene instance.
                                Scene( void );

    private:

        //! Relight scene state.
        enum State {
            StateInitial,
            StateAddingObjects,
            StateReadyToBake
        };

        //! Scene mesh instances.
        Array<const Instance*>  m_meshInstances;

        //! Scene lights.
        Array<const Light*>     m_lights;

        //! Scene state.
        State                   m_state;

        //! Ray tracer instance.
        rt::ITracer*            m_tracer;
    };

} // namespace relight

#endif  /*  !defined( __Relight_Scene_H__ ) */
