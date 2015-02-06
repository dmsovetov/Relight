//
//  Filename:	EmbreeModel.cpp
//	Created:	3:6:2012   11:02

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"EmbreeModel.h"
//#include	"Mesh.h"
#include	"Lightmapper.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** cEmbryModel::cEmbryModel
cEmbryModel::cEmbryModel( void )
{
    rtcInit( NULL );
	numTriangles = 0;


//	triangles = new embree::BuildTriangle[65536];
}

// ** cEmbryModel::GetRayCount
int cEmbryModel::GetRayCount( void )
{
	return 0;
}

// ** cEmbryModel::TraceRay
int cEmbryModel::TraceRay( const sVector3& start, const sVector3& end, sRayTraceResult *result )
{
    sVector3 dir = end - start;
    float len = dir.normalize();
    
    RTCRay ray;
    ray.org[0] = start.x;
    ray.org[1] = start.y;
    ray.org[2] = start.z;

    ray.dir[0] = dir.x;
    ray.dir[1] = dir.y;
    ray.dir[2] = dir.z;

    ray.tnear = 0.01f;
    ray.tfar  = len /*- 0.02f*/;

    ray.geomID = RTC_INVALID_GEOMETRY_ID;
    ray.primID = RTC_INVALID_GEOMETRY_ID;
    ray.instID = RTC_INVALID_GEOMETRY_ID;
    ray.mask   = 0xFFFFFFFF;
    ray.time   = 0.0f;

    rtcIntersect( m_scene, ray );

    result->point = start + dir * ray.time;

    if( ray.primID == -1 ) {
        return -1;
    }

    sMeshVertex a, b, c;
    GetTriangleByIndex( ray.primID, a, b, c );

    result->normal	 = cLightmapper::TriangleInterpolate( ( sVector3 )a.normal, b.normal, c.normal, ray.u, ray.v );
    result->color	 = cLightmapper::TriangleInterpolate( ( sColor )a.color, b.color, c.color, ray.u, ray.v );
    result->lightmap = 0;
    sVector2 uv		 = cLightmapper::TriangleInterpolate( ( sVector2 )a.uv, b.uv, c.uv, ray.u, ray.v );

    result->u		 = uv.x;
    result->v		 = uv.y;

    return 0;

/*
	using namespace embree;

	sVector3 dir	= end - start;
	float	 r		= dir.normalize();
	Ray ray( Vec3f( start.x, start.y, start.z ), Vec3f( dir.x, dir.y, dir.z ), 0.01f, r - 0.01f ); 
	Hit hit;

	triMesh->intersect( ray, hit );
	result->point = start + dir * hit.t;

	if( !hit ) {
		return -1;
	}

	assert( hit.id0 != 65536 );
	assert( hit.id1 != 65536 );

	sMeshVertex a, b, c;
	GetTriangleByIndex( hit.id1, a, b, c );

	result->normal	 = cLightmapper::TriangleInterpolate( ( sVector3 )a.normal, b.normal, c.normal, hit.u, hit.v );
	result->color	 = cLightmapper::TriangleInterpolate( ( sColor )a.color, b.color, c.color, hit.u, hit.v );
	result->lightmap = hit.id0;
	sVector2 uv		 = cLightmapper::TriangleInterpolate( ( sVector2 )a.uv, b.uv, c.uv, hit.u, hit.v );

	result->u		 = uv.x;
	result->v		 = uv.y;

	return 0;
*/
}

// ** cEmbryModel::GetTriangleByIndex
void cEmbryModel::GetTriangleByIndex( int index, sMeshVertex& a, sMeshVertex& b, sMeshVertex& c ) const
{
	const sMesh& m  = meshes[0];
    assert( index >= 0 && index < m.totalFaces );

	const int *face = &m.indices[index * 3];

	a = *( const sMeshVertex* )(m.vertices + face[0]);
	b = *( const sMeshVertex* )(m.vertices + face[1]);
	c = *( const sMeshVertex* )(m.vertices + face[2]);
}

// ** cEmbryModel::Create
void cEmbryModel::Create( void )
{
    m_scene = rtcNewScene( RTC_SCENE_STATIC, RTC_INTERSECT1 );
//	triMesh = embree::rtcCreateAccel( "default", triangles, numTriangles );
}

// ** cEmbryModel::AddMesh
void cEmbryModel::AddMesh( sMesh& mesh )
{
	if( numTriangles ) {
		return;
	}

    unsigned geom = rtcNewTriangleMesh( m_scene, RTC_GEOMETRY_STATIC, mesh.totalFaces, mesh.totalVertices );

    struct Vertex {
        float x, y, z, a;
    };
    struct Triangle {
        int v0, v1, v2;
    };

    Vertex* vertices = ( Vertex* )rtcMapBuffer( m_scene, geom, RTC_VERTEX_BUFFER );

    for( int i = 0; i < mesh.totalVertices; i++ ) {
        Vertex&      dst = vertices[i];
        sMeshVertex& src = mesh.vertices[i];

        dst.x = src.position[0];
        dst.y = src.position[1];
        dst.z = src.position[2];
        dst.a = 1.0f;
    }

    rtcUnmapBuffer( m_scene, geom, RTC_VERTEX_BUFFER );

    Triangle* triangles = ( Triangle* )rtcMapBuffer( m_scene, geom, RTC_INDEX_BUFFER );

    for( int i = 0; i < mesh.totalFaces; i++ ) {
        Triangle& tri  = triangles[i];
        int*      face = &mesh.indices[i * 3 ];

        tri.v0 = face[0];
        tri.v1 = face[1];
        tri.v2 = face[2];
    }

    rtcUnmapBuffer( m_scene, geom, RTC_INDEX_BUFFER );

    rtcCommit( m_scene );
/*
	for( int i = 0; i < mesh.totalFaces; i++ ) {
	//	embree::BuildTriangle& tri = triangles[numTriangles++];

		int *face = &mesh.indices[i * 3];

		tri.x0 = mesh.vertices[ face[0] ].position[0];
		tri.y0 = mesh.vertices[ face[0] ].position[1];
		tri.z0 = mesh.vertices[ face[0] ].position[2];

		tri.x1 = mesh.vertices[ face[1] ].position[0];
		tri.y1 = mesh.vertices[ face[1] ].position[1];
		tri.z1 = mesh.vertices[ face[1] ].position[2];

		tri.x2 = mesh.vertices[ face[2] ].position[0];
		tri.y2 = mesh.vertices[ face[2] ].position[1];
		tri.z2 = mesh.vertices[ face[2] ].position[2];

		tri.id0 = mesh.vertices[ face[0] ].lightmapIndex;
		tri.id1 = i;
		tri.id2 = 65536;
	}
*/
	meshes.push_back( mesh );
}