//
//  Filename:	LightmapCalculator.h
//	Created:	22:03:2012   16:09

#ifndef		__LightmapCalculator_H__
#define		__LightmapCalculator_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"Lightmapper.h"

/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** class cLightmapCalculator
class cLightmapCalculator {
protected:

	const cLightmapper	*lightmapper;
	IRayTracer		*raytracer;
	int				startRow, startFace;
	int				rowStride;
	mutable int		row;
	int				faceStride;
	mutable int		face;

protected:

	float			GetShadowTerm( const sVector3& position, const sVector3& light, float lightRadius, int jitter = 1 ) const;

	int				GetFirstRow( void ) const;
	int				GetNextRow( void ) const;
	int				GetFirstFace( void ) const;
	int				GetNextFace( void ) const;

public:

					cLightmapCalculator( const cLightmapper *_lightmapper, IRayTracer *_raytracer )
						: lightmapper( _lightmapper ), raytracer( _raytracer ), startRow( 0 ), startFace( 0 ), faceStride( 0 ), rowStride( 1 ) {}

	virtual void	Calculate( cLightmap *lm, const sLight *light, int *rowsProcessed = NULL ) {}
	void			SetStartRow( int row ) { startRow = row; }
	int				GetStartRow( void ) const { return startRow; }
	void			SetRowStride( int stride ) { rowStride = stride; }
	int				GetRowStride( void ) const { return rowStride; }

	void			SetStartFace( int face ) { startFace = face; }
	int				GetStartFace( void ) const { return startFace; }
	void			SetFaceStride( int stride ) { faceStride = stride; }
	int				GetFaceStride( void ) const { return faceStride; }
};

#endif	/*	!__LightmapCalculator_H__	*/