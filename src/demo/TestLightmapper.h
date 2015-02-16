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
struct Instance;

class BakingProgress : public relight::Progress {
public:

                            BakingProgress( Instance* instance )
                                : m_instance( instance ), m_hasUpdates( false ) {}

    virtual void            notify( int step, int stepCount );

public:

    Instance*               m_instance;
    bool                    m_hasUpdates;
};

struct Instance {
    BakingProgress*         m_progress;
    std::string             m_name;
    relight::Mesh*          m_mesh;
    relight::Lightmap*      m_lightmap;
    relight::Lightmap*      m_photons;
    unsigned int            m_diffuseId;
    unsigned int            m_lightmapId;
};

struct SceneSettings {
    float                   m_fogIntensity;
    relight::Color          m_fogColor;
};

typedef std::vector<Instance*> InstanceArray;

struct WorkerData {
    relight::Scene*                     m_scene;
    relight::IndirectLightSettings      m_indirectLightSettings;
    relight::AmbientOcclusionSettings   m_aoSettings;
    Instance*                           m_instance;
    InstanceArray*                      m_instances;
    int                                 m_startIndex;
    int                                 m_step;
};

enum MeshList {
    Mesh_Light = 0,
    Mesh_Ground,
    
    Mesh_Tomb05c,
    Mesh_Gravestone01,
    TotalMeshes
};

struct Prefab {
    relight::Mesh*      m_mesh;
    unsigned int        m_diffuse;
};

// ** class cTestLightmapper
class cTestLightmapper : public cTest {
private:

	void			CalculateDirectLight( void );
	void			CalculateIndirectLight( void );

	void			UpdatePixels( const cLightmap *lightmap, float *pixels );

	void			SaveLightmaps( void );
    void            LoadLightmaps( void );

    void            createScene( void );

    static void*    bakeWorker( void* userData );
    static void*    lightmapWorker( void* userData );

public:

	// ** cTest
	virtual void	KeyPressed( int key );
	virtual void	Create( void );
	virtual void	Render( void );

private:

    unsigned int    createTextureFromLightmap( const relight::Lightmap* lightmap ) const;
    unsigned int    createTextureFromFile( const char* fileName ) const;
    void            renderInstance( const Instance* instance ) const;
    Instance*       placeInstance( const std::string& name, const Prefab& prefab, const relight::Matrix4& T, int lightmapSize );
    relight::Mesh*  loadMesh( const char* fileName, const char* diffuse, const relight::Color& color = relight::Color( 1, 1, 1 ) ) const;
    Prefab          createGroundPlane( const char* diffuse, int size, const relight::Color& color = relight::Color( 1, 1, 1 ) ) const;
    Prefab          loadPrefab( const char* fileName, const char* diffuse, const relight::Color& color = relight::Color( 1, 1, 1 ) );

    void            startLightmapsThread( const relight::IndirectLightSettings& indirectLight, const relight::AmbientOcclusionSettings& ambientOcclusion );
    void            startBakingThread( Instance* instance, int index, int threadCount, const relight::IndirectLightSettings& indirectLight, const relight::AmbientOcclusionSettings& ambientOcclusion );

private:

    pthread_t           m_thread;
    WorkerData          m_data;

    relight::Scene*     m_scene;
    float               m_rotationY;
    float               m_rotationX;
    bool                m_useLightmaps;

    Prefab              m_meshes[TotalMeshes];
    InstanceArray       m_instances;
};

#endif	/*	!__DC_TestLightmapper_H__	*/