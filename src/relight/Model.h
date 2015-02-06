//
//  Filename:	Model.h
//	Created:	27:02:2012   11:14

#ifndef		__Model_H__
#define		__Model_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"PathTracer.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

class btVector3;
class btCollisionWorld;
class btCollisionObject;
class btDefaultCollisionConfiguration;
class btAxisSweep3;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btTriangleMeshShape;

struct sMesh;
struct sMeshVertex;
struct sVector3;

// ** class cBulletModel
class cBulletModel : public IRayTracer {
private:

	btCollisionWorld				*collisionWorld;
	btDefaultCollisionConfiguration	*collisionConfiguration;
	btCollisionDispatcher			*dispatcher;
	btAxisSweep3					*overlappingPairCache;
	int								rayCount;

private:

	bool				Raytest( const btVector3& rayFrom, const btVector3& rayTo, sRayTraceResult *result );
	void				CalculateBarycentric( sRayTraceResult *result ) const;
	void				GetTriangleByIndex( btTriangleMeshShape *shape, int index, sMeshVertex& a, sMeshVertex& b, sMeshVertex& c ) const;

public:

						cBulletModel( void ) : collisionWorld( NULL ) {}

	// ** IRayTracer
	virtual int			TraceRay( const sVector3& start, const sVector3& end, sRayTraceResult *result = 0 );
	virtual int			GetRayCount( void );
	virtual void		AddMesh( sMesh& mesh );
	virtual void		Create( void ) {}
};

#endif	/*	!__Model_H__	*/