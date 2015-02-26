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

#ifndef __Relight_Baker_H__
#define __Relight_Baker_H__

#include "../Relight.h"

namespace relight {

namespace bake {

    class BakeIterator;

    //! Baker class is a base class for all light bakers (direct, indirect, ambient occlusion, etc)
    class Baker {
    friend class BakeIterator;
    public:

                                //! Constructs a new Baker instance.
                                Baker( const Scene* scene, Progress* progress, BakeIterator* iterator );

        virtual                 ~Baker( void );

        //! Bakes a light data to an instance lightmap.
        virtual RelightStatus   bakeMesh( const Mesh* mesh );

        //! Bakes all scene light data to a set of textures.
        virtual RelightStatus   bake( void );

    protected:

        //! Bakes a data to a given lumel.
        virtual void            bakeLumel( Lumel& lumel );

        //! Bakes a data to lumels corresponding to this face.
        void                    bakeFace( const Mesh* mesh, Index index );

    protected:

        //! Baking progress
        Progress*               m_progress;

        //! Scene to bake.
        const Scene*            m_scene;

        //! Bake iterator.
        BakeIterator*           m_iterator;
    };

    //! Bake iterator is used to iterate over lightmap lumels and bake data into them.
    class BakeIterator {
    public:

                                //! Constructs a BakeIterator instance.
                                /*!
                                 \param baker Parent baker.
                                 \param first First element index.
                                 \param step Iteration step.
                                 */
                                BakeIterator( int first, int step );

        //! Begins iteration process.
        /*!
         \param baker Parent baker.
         \param lightmap A target lightmap to bake.
         \param mesh Current mesh being processed.
         */
        virtual void            begin( Baker* baker, Lightmap* lightmap, const Mesh* mesh );

        //! Processes a next iterator item.
        /*!
         \return True if there are any items left for processing, otherwise false.
         */
        virtual bool            next( void );

        //! Returns a total amount of items to process.
        virtual int             itemCount( void ) const;

    protected:

        //! Processes a single lumel.
        void                    bake( Lumel& lumel );

    protected:

        //! Parent baker.
        Baker*                  m_baker;

        //! Current target lightmap.
        Lightmap*               m_lightmap;

        //! Mesh being processed.
        const Mesh*             m_mesh;

        //! Current element index.
        int                     m_index;

        //! First element index.
        int                     m_firstIndex;

        //! Iteration elements step.
        int                     m_step;
    };

    //! LumelBakeIterator is used to bake lightmap lumels one by one.
    class LumelBakeIterator : public BakeIterator {
    public:

                                //! Constructs a LumelBakeIterator instance.
                                LumelBakeIterator( int first, int step );

        //! Processes a next lightmap lumel.
        virtual bool            next( void );

        //! Returns a total amount of items to process.
        virtual int             itemCount( void ) const;
    };

    //! FaceBakeIterator is used to bake mesh faces on by one.
    class FaceBakeIterator : public BakeIterator {
    public:

                                //! Constructs a FaceBakeIterator instance.
                                FaceBakeIterator( int first, int step );

        //! Processes a next lightmap lumel.
        virtual bool            next( void );

        //! Returns a total amount of items to process.
        virtual int             itemCount( void ) const;
    };

} // namespace bake

} // namespacce relight

#endif  /*  !defined( __Relight_Baker_H__ ) */
