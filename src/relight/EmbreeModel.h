//
//  Filename:	EmbreeModel.h
//	Created:	3:6:2012   11:01

#ifndef		__DC_EmbreeModel_H__
#define		__DC_EmbreeModel_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"PathTracer.h"
#include	<embree2/rtcore.h>
#include    <embree2/rtcore_ray.h>

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** class cEmbryModel
class cEmbryModel : public IRayTracer {
private:

//	embree::BuildTriangle				*triangles;
	size_t								numTriangles;
//	embree::Ref<embree::Intersector>	triMesh;
	std::vector< sMesh >				meshes;
    RTCScene                            m_scene;

private:

	void				GetTriangleByIndex( int index, sMeshVertex& a, sMeshVertex& b, sMeshVertex& c ) const;

public:

						cEmbryModel( void );

	// ** IRayTracer
	virtual int			TraceRay( const sVector3& start, const sVector3& end, sRayTraceResult *result = 0 );
	virtual int			GetRayCount( void );
	virtual void		AddMesh( sMesh& mesh );
	virtual void		Create( void );
};

#endif	/*	!__DC_EmbreeModel_H__	*/