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

#include "../Relight.h"

namespace relight {

    /*!
     Instance of a Scene class holds all lights, mesh instances
     and rendering settings.
     */
    class Scene {
    friend class Relight;
    public:

        //! Returns a tracer.
        rt::ITracer*            tracer( void ) const;

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
         Creates a new Mesh and places it on scene.
         \param mesh A mesh data to create an instance from.
         \param transform mesh instance transform.
         */
        Mesh*                   addMesh( const Mesh* mesh, const Matrix4& transform, const Material* material = NULL );

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
        int                     meshCount( void ) const;

        //! Returns a mesh by index.
        const Mesh*             mesh( int index ) const;

        //! Returns a scene bounding box.
        const Bounds&           bounds( void ) const;

    private:

                                //! Constructs a new Scene instance.
                                Scene( void );

        //! Updates scene bounds.
        void                    updateBounds( void );

    private:

        //! Relight scene state.
        enum State {
            StateInitial,
            StateAddingObjects,
            StateReadyToBake
        };

        //! Scene mesh instances.
        Array<const Mesh*>      m_meshes;

        //! Scene lights.
        Array<const Light*>     m_lights;

        //! Scene state.
        State                   m_state;

        //! Ray tracer instance.
        rt::ITracer*            m_tracer;

        //! Scene bounds.
        Bounds                  m_bounds;
    };

} // namespace relight

#endif  /*  !defined( __Relight_Scene_H__ ) */
