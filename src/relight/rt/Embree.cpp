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

#include "Embree.h"
#include "../Mesh.h"

namespace relight {

namespace rt {

// ** Embree::Embree
Embree::Embree( void )
{
    rtcInit( NULL );
}

Embree::~Embree( void )
{
    rtcDeleteScene( m_scene );
    rtcExit();
}

// ** Embree::begin
void Embree::begin( void )
{
    m_scene = rtcNewScene( RTC_SCENE_STATIC, RTC_INTERSECT1 | RTC_INTERSECT4 );
}

// ** Embree::end
void Embree::end( void )
{
    rtcCommit( m_scene );
}

// ** Embree::traceSegments
void Embree::traceSegments( Segment segments[4] )
{
    int     valid4[4] = { -1,-1,-1,-1 };
    RTCRay4 rays;

    for( int i = 0; i < 4; i++ ) {
        Vec3 dir  = segments[i].m_end - segments[i].m_start;
        float len = dir.normalize();

        rays.orgx[i] = segments[i].m_start.x;
        rays.orgy[i] = segments[i].m_start.y;
        rays.orgz[i] = segments[i].m_start.z;

        rays.dirx[i] = dir.x;
        rays.diry[i] = dir.y;
        rays.dirz[i] = dir.z;

        rays.tnear[i] = 0.01f;
        rays.tfar[i]  = len;

        rays.geomID[i] = RTC_INVALID_GEOMETRY_ID;
        rays.primID[i] = RTC_INVALID_GEOMETRY_ID;
        rays.instID[i] = RTC_INVALID_GEOMETRY_ID;
        rays.mask[i]   = 0xFFFFFFFF;
        rays.time[i]   = 0.0f;
    }

    rtcIntersect4( valid4, m_scene, rays );

    for( int i = 0; i < 4; i++ ) {
        if( rays.geomID[i] == RTC_INVALID_GEOMETRY_ID ) {
            continue;
        }

        const Instance* instance = m_instances[rays.geomID[i]];
        Face            face     = instance->mesh()->face( rays.primID[i] );
        Barycentric     coord    = Barycentric( rays.u[i], rays.v[i] );

        segments[i].m_hit.m_normal    = face.normalAt( coord );
        segments[i].m_hit.m_color     = face.colorAt( coord );
        segments[i].m_hit.m_uv        = face.uvAt( coord, Vertex::Lightmap );
        segments[i].m_hit.m_instance  = instance;
    }
}

// ** Embree::traceSegment
bool Embree::traceSegment( const Vec3& start, const Vec3& end, Hit* result )
{
    Vec3 dir = end - start;
    float len = dir.normalize();

    RTCRay ray;
    ray.org[0] = start.x;
    ray.org[1] = start.y;
    ray.org[2] = start.z;

    ray.dir[0] = dir.x;
    ray.dir[1] = dir.y;
    ray.dir[2] = dir.z;

    ray.tnear = 0.01f;
    ray.tfar  = len;

    ray.geomID = RTC_INVALID_GEOMETRY_ID;
    ray.primID = RTC_INVALID_GEOMETRY_ID;
    ray.instID = RTC_INVALID_GEOMETRY_ID;
    ray.mask   = 0xFFFFFFFF;
    ray.time   = 0.0f;

    rtcIntersect( m_scene, ray );

    if( result ) {
        result->m_point = start + dir * ray.time;
    }

    if( ray.primID == -1 ) {
        return false;
    }

    if( !result ) {
        return true;
    }

    const Instance* instance = m_instances[ray.geomID];
    Face            face     = instance->mesh()->face( ray.primID );
    Barycentric     coord    = Barycentric( ray.u, ray.v );

    result->m_normal    = face.normalAt( coord );
    result->m_color     = face.colorAt( coord );
    result->m_uv        = face.uvAt( coord, Vertex::Lightmap );
    result->m_instance  = instance;

    return true;
}

// ** Embree::addInstance
void Embree::addInstance( const Instance* instance )
{
    // ** Get a mesh data from instance
    const Mesh* mesh = instance->mesh();
    assert( mesh->submeshCount() == 1 );

    // ** Create a new Embree geomentry
    unsigned geom = rtcNewTriangleMesh( m_scene, RTC_GEOMETRY_STATIC, mesh->faceCount(), mesh->vertexCount() );

    // ** Upload vertex buffer
    EmVertex* vertices = ( EmVertex* )rtcMapBuffer( m_scene, geom, RTC_VERTEX_BUFFER );

    for( int i = 0, n = mesh->submeshCount(); i < n; i++ ) {
        const SubMesh& sub = mesh->submesh( i );

        for( int j = 0; j < sub.m_vertices.size(); j++ ) {
            EmVertex&   dst = vertices[j];
            Vec3        pos = instance->transform() * sub.m_vertices[j].m_position;

            dst.x = pos.x;
            dst.y = pos.y;
            dst.z = pos.z;
            dst.a = 1.0f;
        }
    }

    rtcUnmapBuffer( m_scene, geom, RTC_VERTEX_BUFFER );

    // ** Upload index buffer
    EmFace* triangles = ( EmFace* )rtcMapBuffer( m_scene, geom, RTC_INDEX_BUFFER );
    int     idx       = 0;

    for( int i = 0, n = mesh->submeshCount(); i < n; i++ ) {
        const SubMesh& sub = mesh->submesh( i );

        for( int j = 0; j < sub.m_totalFaces; j++ ) {
            EmFace& tri  = triangles[idx++];

            tri.v0 = sub.m_indices[j * 3 + 0];
            tri.v1 = sub.m_indices[j * 3 + 1];
            tri.v2 = sub.m_indices[j * 3 + 2];
        }
    }

    rtcUnmapBuffer( m_scene, geom, RTC_INDEX_BUFFER );

    // ** Push instance to a registry
    m_instances.push_back( instance );
}

} // namespace rt

} // namespace relight
