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
#include "../scene/Mesh.h"

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

        const Mesh* mesh    = m_meshes[rays.geomID[i]];
        Face        face    = mesh->face( rays.primID[i] );
        Barycentric coord   = Barycentric( rays.u[i], rays.v[i] );

        segments[i].m_hit.m_normal  = face.normalAt( coord );
        segments[i].m_hit.m_color   = face.colorAt( coord );
        segments[i].m_hit.m_uv      = face.uvAt( coord, Vertex::Lightmap );
        segments[i].m_hit.m_mesh    = mesh;
    }
}

// ** Embree::initializeRay
float Embree::initializeRay( RTCRay& ray, Vec3& direction, const Vec3& start, const Vec3& end ) const
{
    direction = end - start;
    float len = direction.normalize();

    ray.org[0] = start.x;
    ray.org[1] = start.y;
    ray.org[2] = start.z;

    ray.dir[0] = direction.x;
    ray.dir[1] = direction.y;
    ray.dir[2] = direction.z;

    ray.tnear = 0.01f;
    ray.tfar  = len;

    ray.geomID = RTC_INVALID_GEOMETRY_ID;
    ray.primID = RTC_INVALID_GEOMETRY_ID;
    ray.instID = RTC_INVALID_GEOMETRY_ID;
    ray.mask   = 0xFFFFFFFF;
    ray.time   = 0.0f;

    return len;
}

// ** Embree::test
bool Embree::test( const Vec3& start, const Vec3& end )
{
    Vec3   direction;
    RTCRay ray;
    initializeRay( ray, direction, start, end );

    rtcOccluded( m_scene, ray );

    return ray.geomID != -1;
}

// ** Embree::traceSegment
Hit Embree::traceSegment( const Vec3& start, const Vec3& end, int flags, int step )
{
    Vec3   direction;
    RTCRay ray;
    initializeRay( ray, direction, start, end );

    rtcIntersect( m_scene, ray );

    if( ray.primID == -1 ) {
        return Hit();
    }

    Vec3        point   = start + direction * ray.tfar;
    const Mesh* mesh    = m_meshes[ray.geomID];
    Face        face    = mesh->face( ray.primID );
    Barycentric coord   = Barycentric( ray.u, ray.v );
    Rgba        color   = face.colorAt( coord );

    if( flags & HitUseAlpha && color.a <= 0.1f && step < 25 ) {
        return traceSegment( point + direction, end, flags, step + 1 );
    }

    Hit result;

    if( flags & HitNormal ) result.m_normal = face.normalAt( coord );
    if( flags & HitColor )  result.m_color  = face.colorAt( coord );
    if( flags & HitUv )     result.m_uv     = face.uvAt( coord, Vertex::Lightmap );

	result.m_point = point;
    result.m_mesh  = mesh;

    return result;
}

// ** Embree::addMesh
void Embree::addMesh( const Mesh* mesh )
{
    // ** Create a new Embree geomentry
    unsigned geom = rtcNewTriangleMesh( m_scene, RTC_GEOMETRY_STATIC, mesh->faceCount(), mesh->vertexCount() );

    // ** Upload vertex buffer
    EmVertex* vertices = ( EmVertex* )rtcMapBuffer( m_scene, geom, RTC_VERTEX_BUFFER );

    for( int j = 0, n = mesh->vertexCount(); j < n; j++ ) {
        EmVertex&   dst = vertices[j];
        Vec3        pos = mesh->vertex( j ).position;

        dst.x = pos.x;
        dst.y = pos.y;
        dst.z = pos.z;
        dst.a = 1.0f;
    }

    rtcUnmapBuffer( m_scene, geom, RTC_VERTEX_BUFFER );

    // ** Upload index buffer
    EmFace* triangles = ( EmFace* )rtcMapBuffer( m_scene, geom, RTC_INDEX_BUFFER );
    int     idx       = 0;

    for( int j = 0, n = mesh->indexCount() / 3; j < n; j++ ) {
        EmFace&     tri  = triangles[idx++];
        const Face& face = mesh->face( j );

        tri.v0 = mesh->index( j * 3 + 0 );
        tri.v1 = mesh->index( j * 3 + 1 );
        tri.v2 = mesh->index( j * 3 + 2 );
    }

    rtcUnmapBuffer( m_scene, geom, RTC_INDEX_BUFFER );

    // ** Push instance to a registry
    m_meshes.push_back( mesh );
}

} // namespace rt

} // namespace relight
