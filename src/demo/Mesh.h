#pragma once

//#include "Lightmapper.h"

#include <unistd.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <fstream>
#include <iostream>


/*************************************************************************** 
  OBJ Loading 
 ***************************************************************************/
 /*
class Model_OBJ
{
	typedef std::map< sMeshVertex, int >	tVertexMap;
	typedef std::vector< int >				tIndexArray;
	typedef std::vector< float >			tFloatArray;

  public: 
				Model_OBJ();			
    int			Load(char *filename);	// Loads the model
	void		Draw( int meshIndex );	// Draws the model on the screen
	void		Release();				// Release the model
	int			PushVertex( const sMeshVertex& v );
	void		CommitMesh( bool hasUVs, bool hasNormals );
 
//	float*	normals;							// Stores the normals
//	float*	uv;
//	float*	vertices;					// Stores the points which make the object
	int		totalFaces, currentMeshFaces;

	tFloatArray		normals, uv, vertices;
	tIndexArray		xyzIndices, uvIndices, normIndices;
//	int		*xyzIndices, *uvIndices, *normIndices;

	std::vector< sMesh >			meshes;
};
 
//#define WHITE	1.0f, 0.9f, 0.8f
#define WHITE	0.9f, 0.9f, 0.9f
#define RED		0.65f, 0.25f, 0.15f
//#define RED		0.8f, 0.3f, 0.3f
#define GREEN	0.15f, 0.65f, 0.25f
//#define GREEN	0.45f, 0.49f, 0.36f
//#define GREEN	0.3f, 0.3f, 0.8f
 
#define POINTS_PER_VERTEX 3
#define TOTAL_FLOATS_IN_TRIANGLE 9
using namespace std;
 
inline Model_OBJ::Model_OBJ()
{
//	this->uiIndex	 = 0;
	this->totalFaces = 0;
	currentMeshFaces = 0;
}

inline void Model_OBJ::CommitMesh( bool hasUVs, bool hasNormals )
{
	sMesh mesh;

	mesh.totalFaces = currentMeshFaces;

	std::vector< int >				vertexIndices;
	std::vector< sMeshVertex >		vertexArray;
	tVertexMap						vertexMap;
	int								uiIndex = 0;

	for( int i = totalFaces - currentMeshFaces; i < totalFaces; i++ ) {
		int idxXyz[]	= { xyzIndices[i * 3 + 0],	xyzIndices[i * 3 + 1],	xyzIndices[i * 3 + 2]	};
		int idxUv[]		= { uvIndices[i * 3 + 0],	uvIndices[i * 3 + 1],	uvIndices[i * 3 + 2]	};
		int idxNorm[]	= { normIndices[i * 3 + 0],	normIndices[i * 3 + 1],	normIndices[i * 3 + 2]	};

		// ** XYZ
		for( int j = 0; j < 3; j++ ) {
			int xyzIndex = idxXyz[j];
			int nrmIndex = idxNorm[j];
			int uvIndex  = idxUv[j];

			sMeshVertex v;

			v.position[0]	= vertices[ xyzIndex * 3 + 0 ];
			v.position[1]	= vertices[ xyzIndex * 3 + 1 ];
			v.position[2]	= vertices[ xyzIndex * 3 + 2 ];

			v.normal[0]		= hasNormals ? normals[ nrmIndex * 3 + 0 ] : 0.0f;
			v.normal[1]		= hasNormals ? normals[ nrmIndex * 3 + 1 ] : 1.0f;
			v.normal[2]		= hasNormals ? normals[ nrmIndex * 3 + 2 ] : 0.0f;

		//	v.color[0]		= hasNormals ? (normals[ nrmIndex * 3 + 0 ] * 0.5f + 0.5f) : 1.0f;
		//	v.color[1]		= hasNormals ? (normals[ nrmIndex * 3 + 1 ] * 0.5f + 0.5f) : 1.0f;
		//	v.color[2]		= hasNormals ? (normals[ nrmIndex * 3 + 2 ] * 0.5f + 0.5f) : 1.0f;

			static const float colors[12][3] = { WHITE, RED, GREEN };

			const int k = 33;
			const int l = 29;
			if( i > k && i <= k + 2 ) {
				v.color[0]		= colors[1][0];
				v.color[1]		= colors[1][1];
				v.color[2]		= colors[1][2];
			}
			else if( i > l && i <= l + 2 ) {
				v.color[0]		= colors[2][0];
				v.color[1]		= colors[2][1];
				v.color[2]		= colors[2][2];
			} else {
				v.color[0]		= colors[0][0];
				v.color[1]		= colors[0][1];
				v.color[2]		= colors[0][2];
			}

			if( hasUVs ) {
				v.uv[0]  = uv[ uvIndex * 2 + 0 ];
				v.uv[1]  = uv[ uvIndex * 2 + 1 ];

				if( v.uv[0] > 1.0f ) v.uv[0] = 1.0f;
				if( v.uv[1] > 1.0f ) v.uv[1] = 1.0f;
				if( v.uv[0] < 0.0f ) v.uv[0] = 0.0f;
				if( v.uv[1] < 0.0f ) v.uv[1] = 0.0f;
			}

			tVertexMap::iterator vi = vertexMap.find( v );
			unsigned int index = -1;

			if( vi == vertexMap.end() ) {
				index		 = uiIndex++;
				vertexMap[v] = index;
			} else {
				index = vi->second;
			}

			vertexIndices.push_back( index );
		}
	}

	// ** Copy vertices
	vertexArray.resize( vertexMap.size() );
	for( tVertexMap::const_iterator i = vertexMap.begin(); i != vertexMap.end(); ++i ) {
		vertexArray[i->second] = i->first;
	}

	mesh.totalVertices	= vertexArray.size();
	mesh.vertices		= new sMeshVertex[vertexArray.size()];
	mesh.indices		= new int[vertexIndices.size()];
	memcpy( mesh.indices, &vertexIndices[0], sizeof( int ) * vertexIndices.size() );
	memcpy( mesh.vertices, &vertexArray[0], sizeof( sMeshVertex ) * vertexArray.size() );

	meshes.push_back( mesh );
	currentMeshFaces = 0;
}
 
inline int Model_OBJ::Load(char* filename)
{
	string line;
	ifstream objFile (filename);	
	if (objFile.is_open())													// If obj file is open, continue
	{
		bool hasNormals  = false;
		bool hasUVs		 = false;
		bool hasMesh	 = false;
 
		while (! objFile.eof() )											// Start reading file data
		{		
			getline (objFile,line);											// Get line from file

			char c1 = line.c_str()[0];
			char c2 = line.c_str()[1];
 
			if( c1 == 'v' && c2 == 't' )
			{
				float x, y;

				sscanf( line.c_str() + 2, "%f %f ", &x, &y );
				uv.push_back( x ), uv.push_back( y );

				hasUVs = true;

				if( currentMeshFaces ) {
					CommitMesh( hasUVs, hasNormals );
					hasMesh = false;
					hasUVs = false;
					hasNormals = false;
				}
			}
			else if(c1 == 'v' && c2 == 'n' )
			{
				float x, y, z;

				sscanf( line.c_str() + 2,"%f %f %f ", &x, &y, &z );
				normals.push_back( x ), normals.push_back( y ), normals.push_back( z );

				hasNormals = true;

				if( currentMeshFaces ) {
					CommitMesh( hasUVs, hasNormals );
					hasMesh = false;
					hasUVs = false;
					hasNormals = false;
				}
			}
			else if (c1 == 'v')
			{
				float x, y, z;
				sscanf( line.c_str() + 1, "%f %f %f ", &x, &y, &z );
				vertices.push_back( x ), vertices.push_back( y ), vertices.push_back( z );

				if( currentMeshFaces ) {
					CommitMesh( hasUVs, hasNormals );
					hasMesh = false;
					hasUVs = false;
					hasNormals = false;
				}
			}

			if( c1 == 'f' )
			{
				int vertexNumber[4] = { 0, 0, 0 };
				int uvNumber[4] = { 0, 0, 0 };
				int normNumber[4] = { 0, 0, 0 };

				if( hasNormals && hasUVs ) {
					sscanf( line.c_str() + 1, "%i/%i/%i %i/%i/%i %i/%i/%i",	&vertexNumber[0], &uvNumber[0], &normNumber[0],
																			&vertexNumber[1], &uvNumber[1], &normNumber[1],
																			&vertexNumber[2], &uvNumber[2], &normNumber[2] );
				}
				else if( hasNormals ) {
					sscanf( line.c_str() + 1, "%i/%i %i/%i %i/%i",	&vertexNumber[0], &normNumber[0],
																	&vertexNumber[1], &normNumber[1],
																	&vertexNumber[2], &normNumber[2] );
				}
				else if( hasUVs ) {
					sscanf( line.c_str() + 1, "%i/%i %i/%i %i/%i",	&vertexNumber[0], &uvNumber[0],
																	&vertexNumber[1], &uvNumber[1],
																	&vertexNumber[2], &uvNumber[2] );
				}
				else {
					sscanf( line.c_str() + 1, "%i %i %i", &vertexNumber[0], &vertexNumber[1], &vertexNumber[2] );
				}

				for( int i = 0; i < 3; i++ ) {
					uvIndices.push_back( uvNumber[i] - 1 );
					xyzIndices.push_back( vertexNumber[i] - 1 );
					normIndices.push_back( normNumber[i] - 1 );
				}

				totalFaces++;
				currentMeshFaces++;
			}
		}
		objFile.close();													// Close OBJ file

		if( currentMeshFaces ) {
			CommitMesh( hasUVs, hasNormals );
		}
	}
	else 
	{
        char buf[512];
		cout << "Unable to open file " << filename << " (cwd " << getcwd( buf, 512 ) << ")\n";
	}
	return 0;
}

inline int Model_OBJ::PushVertex( const sMeshVertex& v )
{
	return -1;
}
 
inline void Model_OBJ::Release()
{
//	free(this->normals);
//	free(this->vertices);
}

#ifdef WIN32
    extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
#endif

inline void Model_OBJ::Draw( int meshIndex )
{
 	glEnableClientState(GL_VERTEX_ARRAY);						// Enable vertex arrays
 	glEnableClientState(GL_NORMAL_ARRAY);						// Enable normal arrays
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);				// Enable texture coord arrays
	glEnableClientState(GL_COLOR_ARRAY);				// Enable texture coord arrays

//	for( int i = 0; i < meshes.size(); i++ ) {
		const sMesh& mesh = meshes[meshIndex];

		glVertexPointer( 3, GL_FLOAT, sizeof( sMeshVertex ), mesh.vertices->position );				// Vertex Pointer to triangle array
		glNormalPointer( GL_FLOAT, sizeof( sMeshVertex ), mesh.vertices->normal );						// Normal pointer to normal array
		glColorPointer( 3, GL_FLOAT, sizeof( sMeshVertex ), mesh.vertices->color );						// Normal pointer to normal array

		glClientActiveTextureARB( GL_TEXTURE1_ARB );
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer( 2, GL_FLOAT, sizeof( sMeshVertex ), mesh.vertices->uv );

		glClientActiveTextureARB( GL_TEXTURE0_ARB );
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer( 2, GL_FLOAT, sizeof( sMeshVertex ), mesh.vertices->uv );

		glDrawElements( GL_TRIANGLES, mesh.totalFaces * 3, GL_UNSIGNED_INT, mesh.indices );
//	}

	glDisableClientState(GL_VERTEX_ARRAY);						// Disable vertex arrays
	glDisableClientState(GL_NORMAL_ARRAY);						// Disable normal arrays
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);				// Disable texture coord arrays
	glDisableClientState(GL_COLOR_ARRAY);				// Enable texture coord arrays
}*/
 