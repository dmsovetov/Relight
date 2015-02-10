//
//  Filename:	TestLightmapper.h
//	Created:	2:6:2012   20:07

#ifndef		__DC_TestLightmapper_H__
#define		__DC_TestLightmapper_H__

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"Test.h"
#include    <Relight.h>
#include    <pthread.h>

#define		MAX_LIGHTMAPS	(16)

/*
=========================================================================================

			CODE

=========================================================================================
*/

class cLightmapper;
class cLightmap;
class cPhotonMap;
class Model_OBJ;
struct sLight;

struct WorkerData {
    relight::Scene*     m_scene;
    relight::Progress*  m_progress;
};

class BakingProgress : public relight::Progress {
public:

                                BakingProgress( const relight::Lightmap* lightmap, unsigned int* textureId )
                                    : m_lightmap( lightmap ), m_textureId( textureId ), m_hasUpdates( false ) {}

    virtual void                notify( int step, int stepCount );

public:

    unsigned int*               m_textureId;
    const relight::Lightmap*    m_lightmap;
    bool                        m_hasUpdates;
};

// ** class cTestLightmapper
class cTestLightmapper : public cTest {
private:

	cLightmapper	*lightmapper;

	cLightmap		*direct[MAX_LIGHTMAPS];
	cLightmap		*indirect[MAX_LIGHTMAPS];
	cPhotonMap		*photonMap[MAX_LIGHTMAPS];

	IRayTracer		*model;

	// ** GPU textures
	unsigned int	texIndirect[MAX_LIGHTMAPS], texDirect[MAX_LIGHTMAPS];

	// ** Settings
	bool			useLinearFiltration;
	bool			renderDirect, renderIndirect;

	// ** Lights
	const sLight	*lights;
	int				totalLights;

    // ** Rotation
    float           rotation;

private:

	void			CalculateDirectLight( void );
	void			CalculateIndirectLight( void );

	void			UpdatePixels( const cLightmap *lightmap, float *pixels );

	void			SaveLightmaps( void );
    void            LoadLightmaps( void );

    void            createScene( const char* fileName );

    static void*    worker( void* userData );

public:

					cTestLightmapper( const sLight *_lights, int _totalLights )
						: lights( _lights ), totalLights( _totalLights ) {}

	// ** cTest
	virtual void	KeyPressed( int key );
	virtual void	Create( IRayTracer *model, const Model_OBJ& mesh );
	virtual void	Render( Model_OBJ& mesh, Model_OBJ& light );

private:

    unsigned int    createTextureFromLightmap( const relight::Lightmap* lightmap ) const;
    void            renderInstance( const relight::Mesh* mesh ) const;

private:

    pthread_t           m_thread;
    WorkerData          m_data;

    BakingProgress*     m_progress;
    relight::Scene*     m_scene;
    relight::Lightmap*  m_diffuse;
    relight::Photonmap* m_photons;

    unsigned int        m_diffuseGl;
};

#endif	/*	!__DC_TestLightmapper_H__	*/