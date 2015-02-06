//
//  Filename:	Model.cpp
//	Created:	27:02:2012   11:29

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"Model.h"
//#include	"Mesh.h"

#ifdef HAVE_BULLET
    #include    <btBulletCollisionCommon.h>
    #include    <btBulletDynamicsCommon.h>
#endif

/*
=========================================================================================

			CODE

=========================================================================================
*/

#ifdef HAVE_BULLET

// ** cBulletModel::GetRayCount
int cBulletModel::GetRayCount( void )
{
	int r = rayCount;
	rayCount = 0;
	return r;
}

// ** cBulletModel::Raytest
bool cBulletModel::Raytest( const btVector3& rayFrom, const btVector3& rayTo, sRayTraceResult *result )
{
	rayCount++;
	
	// ** sRayTraceCallback
	struct sRayTraceCallback : public btCollisionWorld::ClosestRayResultCallback {
		const btCollisionShape		*m_hitTriangleShape;
		int							m_hitTriangleIndex;
		int							m_hitShapePart;


		// ** sRayTraceCallback
		sRayTraceCallback( const btVector3 & rayFrom,const btVector3 & rayTo )
			: btCollisionWorld::ClosestRayResultCallback( rayFrom, rayTo ),
			  m_hitTriangleShape( NULL ),
			  m_hitTriangleIndex( 0 ),
			  m_hitShapePart( 0 ) {}

		// ** addSingleResult
		virtual btScalar addSingleResult( btCollisionWorld::LocalRayResult & rayResult, bool normalInWorldSpace ) {
			if (rayResult.m_localShapeInfo)
			{
				m_hitTriangleShape = rayResult.m_collisionObject->getCollisionShape();
				m_hitTriangleIndex = rayResult.m_localShapeInfo->m_triangleIndex;
				m_hitShapePart = rayResult.m_localShapeInfo->m_shapePart;
			} else {
				m_hitTriangleShape = NULL;
				m_hitTriangleIndex = 0;
				m_hitShapePart = 0;
			}
			return ClosestRayResultCallback::addSingleResult(rayResult,normalInWorldSpace);
		}
	};

	sRayTraceCallback cb( rayFrom, rayTo );

	collisionWorld->rayTest( rayFrom, rayTo, cb );
	if( cb.hasHit() ) {
		if( result ) {
			result->normal.x = cb.m_hitNormalWorld.x();
			result->normal.y = cb.m_hitNormalWorld.y();
			result->normal.z = cb.m_hitNormalWorld.z();

			result->point.x = cb.m_hitPointWorld.x();
			result->point.y = cb.m_hitPointWorld.y();
			result->point.z = cb.m_hitPointWorld.z();

			result->triangle = cb.m_hitTriangleIndex;
			result->object	 = ( void* )cb.m_hitTriangleShape;
			result->mesh	 = ( sMesh* )cb.m_hitTriangleShape->getUserPointer();

			CalculateBarycentric( result );
		}
		return true;
	}

	return false;
}

// ** cBulletModel::CalculateBarycentric
void cBulletModel::CalculateBarycentric( sRayTraceResult *result ) const
{
	sVector3 p  = result->point;
	sMeshVertex a, b, c;

	GetTriangleByIndex( ( btTriangleMeshShape* )(( btCollisionShape* )result->object), result->triangle, a, b, c );
	sVector2 barycentric = cLightmapper::CalculateBarycentric( a.position, b.position, c.position, p );

	sVector2 uv = cLightmapper::TriangleInterpolate( ( sVector2 )a.uv, b.uv, c.uv, barycentric.x, barycentric.y );
	result->color = cLightmapper::TriangleInterpolate( ( sColor )a.color, b.color, c.color, barycentric.x, barycentric.y );

	result->u           = uv.x;
	result->v           = uv.y;
    result->lightmap    = a.lightmapIndex;

	float s = result->u + result->v + (1.0f - result->u - result->v);
	assert( s >= 0.0f && s <= 1.0f );
    assert( a.lightmapIndex == b.lightmapIndex && a.lightmapIndex == c.lightmapIndex );
}

// ** cBulletModel::TraceRay
int cBulletModel::TraceRay( const sVector3& start, const sVector3& end, sRayTraceResult *result )
{
	sVector3 dir = end - start; dir.normalize();
	sVector3 s	 = start + dir * 0.01f;

	btVector3			from( s.x, s.y, s.z );
	btVector3			to( end.x, end.y, end.z );
	btVector3			worldNormal, worldPoint;
	btCollisionObject	*object;

	if( !Raytest( from, to, result ) ) {
		return -1;
	}

	return 0;
}

// ** cBulletModel::GetTriangleByIndex
void cBulletModel::GetTriangleByIndex( btTriangleMeshShape *shape, int index, sMeshVertex& a, sMeshVertex& b, sMeshVertex& c ) const
{
	const unsigned char	*vertices, *indices;
	int					numVertices, numFaces;
	int					vertexStride, indexStride;
	PHY_ScalarType		vertexType, indexType;

	btTriangleIndexVertexArray *mesh = ( btTriangleIndexVertexArray* )shape->getMeshInterface();
	mesh->getLockedReadOnlyVertexIndexBase( &vertices, numVertices, vertexType, vertexStride, &indices, indexStride, numFaces, indexType );

	const int *face = ( const int* )(indices + index * indexStride);

	a = *( const sMeshVertex* )(vertices + face[0] * vertexStride);
	b = *( const sMeshVertex* )(vertices + face[1] * vertexStride);
	c = *( const sMeshVertex* )(vertices + face[2] * vertexStride);
}

// ** cBulletModel::AddMesh
void cBulletModel::AddMesh( sMesh& mesh )
{
//	static int baseLightmapFace = 0;

	rayCount = 0;

	if( !collisionWorld ) {
		collisionConfiguration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher( collisionConfiguration );
		btVector3 worldMin( -1000, -1000, -1000 );
		btVector3 worldMax(  1000,  1000,  1000 );
		overlappingPairCache = new btAxisSweep3( worldMin, worldMax );

		collisionWorld = new btCollisionWorld( dispatcher, overlappingPairCache, collisionConfiguration );
	}

	// ** Upload mesh to Bullet
	btTriangleIndexVertexArray *indexVertexArrays = new btTriangleIndexVertexArray( mesh.totalFaces, mesh.indices, sizeof( int ) * 3,
																					mesh.totalVertices, ( btScalar* )&mesh.vertices->position, sizeof( sMeshVertex ) );

	btTriangleMeshShape	*shape	= new btBvhTriangleMeshShape( indexVertexArrays, true );
	btCollisionObject	*obj	= new btCollisionObject;

	shape->setUserPointer( const_cast< sMesh* >( &mesh ) );
	obj->setCollisionShape( shape );
	collisionWorld->addCollisionObject( obj );

//	mesh.baseLightmapFace = baseLightmapFace;
//	baseLightmapFace += mesh.totalFaces;
}

#endif