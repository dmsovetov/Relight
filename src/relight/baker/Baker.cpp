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

#include "../BuildCheck.h"

#include "Baker.h"
#include "../scene/Mesh.h"
#include "../Lightmap.h"
#include "../scene/Scene.h"

#define RELIGHT_BAKE_FACES  (1)

namespace relight {

namespace bake {

// ** Baker::Baker
Baker::Baker( const Scene* scene, Progress* progress ) : m_scene( scene ), m_progress( progress )
{

}

Baker::~Baker( void )
{

}

// ** Baker::bakeMesh
RelightStatus Baker::bakeMesh( const Mesh* mesh )
{
    Lightmap* lightmap = mesh->lightmap();
    if( !lightmap ) {
        return RelightInvalidCall;
    }

    int     width    = lightmap->width();
    int     height   = lightmap->height();
    Lumel*  lumels   = lightmap->lumels();
    int     progress = 0;

#if RELIGHT_BAKE_FACES
    for( int i = 0, n = mesh->faceCount(); i < n; i++ ) {
        bakeFace( mesh, i );
        if( m_progress ) {
            m_progress->notify( i + 1, mesh->faceCount() );
        }
    }
#else
    for( int j = 0; j < height; j++ ) {
        for( int i = 0; i < width; i++ ) {
            Lumel& lumel = lumels[j*width + i];

            if( lumel ) {
                bakeLumel( lumel );
            }
        }

        if( m_progress ) {
            m_progress->notify( j + 1, height );
        }
    }
#endif

    return RelightSuccess;
}

// ** Baker::bakeFace
void Baker::bakeFace( const Mesh* mesh, Index index )
{
    Face      face      = mesh->face( index );
    Lightmap* lightmap  = mesh->lightmap();

    // ** Calculate UV bounds
    Uv min, max;
    face.uvRect( min, max );

    int uStart = min.u * lightmap->width();
    int uEnd   = max.u * lightmap->width();
    int vStart = min.v * lightmap->height();
    int vEnd   = max.v * lightmap->height();

    // ** Initialize lumels
    for( int v = vStart; v <= vEnd; v++ ) {
        for( int u = uStart; u <= uEnd; u++ ) {
            Lumel& lumel = lightmap->lumel( u, v );
            if( !lumel || lumel.m_faceIdx != index ) {
                continue;
            }

            Uv uv( (u + 0.5f) / float( lightmap->width() ), (v + 0.5f) / float( lightmap->height() ) );
            Barycentric barycentric;

            if( !face.isUvInside( uv, barycentric, Vertex::Lightmap ) ) {
                continue;
            }

            bakeLumel( lumel );
        }
    }
}

// ** Baker::bake
RelightStatus Baker::bake( void )
{
    for( int i = 0, n = m_scene->meshCount(); i < n; i++ ) {
        bakeMesh( m_scene->mesh( i ) );
    }

    return RelightSuccess;
}

// ** Baker::bakeLumel
void Baker::bakeLumel( Lumel& lumel )
{

}

} // namespace bake

} // namespace relight
