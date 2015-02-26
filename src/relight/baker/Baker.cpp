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

namespace relight {

namespace bake {

// --------------------------------------------- Baker --------------------------------------------- //

// ** Baker::Baker
Baker::Baker( const Scene* scene, Progress* progress, BakeIterator* iterator ) : m_scene( scene ), m_progress( progress ), m_iterator( iterator )
{

}

Baker::~Baker( void )
{

}

// ** Baker::bakeMesh
RelightStatus Baker::bakeMesh( const Mesh* mesh )
{
    if( mesh->faceCount() == 0 ) {
        return RelightInvalidCall;
    }

    if( !m_iterator ) {
        return RelightInvalidCall;
    }

    Lightmap* lightmap = mesh->lightmap();
    if( !lightmap ) {
        return RelightInvalidCall;
    }

    int progress = 0;

    m_iterator->begin( this, lightmap, mesh );
    
    while( m_iterator->next() ) {
        if( m_progress ) m_progress->notify( mesh, ++progress, m_iterator->itemCount() );
    }

    return RelightSuccess;
}

// ** Baker::bakeFace
void Baker::bakeFace( const Mesh* mesh, Index index )
{
    const Face& face      = mesh->face( index );
    Lightmap*   lightmap  = mesh->lightmap();

    // ** Calculate UV bounds
    Uv min, max;
    face.uvRect( min, max );

    int uStart = min.x * lightmap->width();
    int uEnd   = max.x * lightmap->width();
    int vStart = min.y * lightmap->height();
    int vEnd   = max.y * lightmap->height();

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

// ---------------------------------------------- BakeIterator ---------------------------------------------- //

// ** BakeIterator::BakeIterator
BakeIterator::BakeIterator( int first, int step ) : m_baker( NULL ), m_lightmap( NULL ), m_index( 0 ), m_firstIndex( first ), m_step( step )
{

}

// ** BakeIterator::begin
void BakeIterator::begin( Baker* baker, Lightmap* lightmap, const Mesh* mesh )
{
    m_baker     = baker;
    m_lightmap  = lightmap;
    m_mesh      = mesh;
    m_index     = m_firstIndex;
}

// ** BakeIterator::next
bool BakeIterator::next( void )
{
    m_index += m_step;
    return m_index < itemCount();
}

// ** BakeIterator::itemCount
int BakeIterator::itemCount( void ) const
{
    return 0;
}

// ** BakeIterator::bake
void BakeIterator::bake( Lumel& lumel )
{
    m_baker->bakeLumel( lumel );
}

// --------------------------------------------- LumelBakeIterator --------------------------------------------- //

// ** LumelBakeIterator::LumelBakeIterator
LumelBakeIterator::LumelBakeIterator( int first, int step ) : BakeIterator( first, step )
{

}

// ** LumelBakeIterator::itemCount
int LumelBakeIterator::itemCount( void ) const
{
    return m_lightmap->width() * m_lightmap->height();
}

// ** LumelBakeIterator::next
bool LumelBakeIterator::next( void )
{
    Lumel& lumel = m_lightmap->lumels()[m_index];

    if( lumel ) {
        bake( lumel );
    }

    return BakeIterator::next();
}

// --------------------------------------------- FaceBakeIterator --------------------------------------------- //

// ** FaceBakeIterator::FaceBakeIterator
FaceBakeIterator::FaceBakeIterator( int first, int step ) : BakeIterator( first, step )
{

}

// ** FaceBakeIterator::itemCount
int FaceBakeIterator::itemCount( void ) const
{
    return m_mesh->faceCount();
}

// ** FaceBakeIterator::next
bool FaceBakeIterator::next( void )
{
    if( m_index >= m_mesh->faceCount() ) {
        return false;
    }
    
    const Face& face = m_mesh->face( m_index );

    // ** Calculate UV bounds
    Uv min, max;
    face.uvRect( min, max );

    int uStart = min.x * m_lightmap->width();
    int uEnd   = max.x * m_lightmap->width();
    int vStart = min.y * m_lightmap->height();
    int vEnd   = max.y * m_lightmap->height();

    // ** Process face lumels
    for( int v = vStart; v <= vEnd; v++ ) {
        for( int u = uStart; u <= uEnd; u++ ) {
            Lumel& lumel = m_lightmap->lumel( u, v );
            if( !lumel || lumel.m_faceIdx != m_index ) {
                continue;
            }

            Uv uv( (u + 0.5f) / float( m_lightmap->width() ), (v + 0.5f) / float( m_lightmap->height() ) );
            Barycentric barycentric;

            if( !face.isUvInside( uv, barycentric, Vertex::Lightmap ) ) {
                continue;
            }

            bake( lumel );
        }
    }
    
    return BakeIterator::next();
}

} // namespace bake

} // namespace relight
