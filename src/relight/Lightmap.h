//
//  Filename:	Lightmap.h
//	Created:	22:03:2012   15:35

#ifndef		__Lightmap_H__
#define		__Lightmap_H__

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
/*
// ** enum eExtraLightmap
enum eExtraLightmap {
	LightmapAO		= 1,
	LightmapPhotons,
	LightmapIndirect,
	LightmapTotal
};*/

// ** enum eLumelFlag
enum eLumelFlag {
	LumelValid		= 1,
	LumelExpanded	= 2,
};

// ** struct sLumel
struct sLumel {
	sVector3		normal;
	sVector3		position;

	sColor			color;
	unsigned char	flags;
//	bool			isValid;

	int				faceIndex;

					sLumel( void ) : flags( 0 ), color( 1.0f, 0.0f, 1.0f ) {}

	bool			IsValid( void ) const { return flags & LumelValid ? true : false; }
	void			SetValid( void ) { flags |= LumelValid; }

	bool			IsExpanded( void ) const { return flags & LumelExpanded ? true : false; }
	void			SetExpanded( void ) { flags |= LumelExpanded; }
};

// ** class cLightmap
class cLightmap {
friend class cLightCalculator;
private:

	cLightmapper	*lightmapper;
	int				index;
//	cLightmap		*extra[LightmapTotal];
//	void			*userData;
	int				width;
	int				height;
	sLumel			*lumels;

private:

	void			CreateLumels( int faceIndex, const sLMFace& face );
	void			CalculateLumel( int faceIndex, const sLMFace& face, sLumel *lumel, double u, double v, double bu, double bv );
	const sColor&	GetNearestColor( int x, int y, int radius ) const;

public:

					cLightmap( cLightmapper *_lightmapper ) : lightmapper( _lightmapper ), width( 0 ), height( 0 ), lumels( NULL ) {}
					~cLightmap( void ) { delete[]lumels; }

	void			Create( int index, int width, int height );
	void			Clear( void );
	void			Blur( void );
	void			Expand( void );
	void			Save( const char *fileName ) const;
    void            Load( const char *fileName );
	int				GetIndex( void ) const { return index; }
	int				GetWidth( void ) const { return width; }
	int				GetHeight( void ) const { return height; }
	sLumel*			GetLumels( void ) { return lumels; }
	const sLumel*	GetLumels( void ) const { return lumels; }
//	void*			GetUserData( void ) { return userData; }
//	void			SetUserData( void *_userData ) { userData = _userData; }
//	void			SetExtraLightmap( eExtraLightmap type, cLightmap *lightmap ) { extra[type] = lightmap; }
//	cLightmap*		GetExtraLightmap( eExtraLightmap type ) { return extra[type]; }

	void			SaveLumels( const char *fileName );
	void			LoadLumels( const char *fileName );
};

#endif	/*	!__Lightmap_H__	*/