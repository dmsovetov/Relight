//
//  Filename:	UVUnwrap.cpp
//	Created:	27:02:2012   17:41

/*
=========================================================================================

			HEADERS & DEFS

=========================================================================================
*/

#include	"UVUnwrap.h"


/*
=========================================================================================

			CODE

=========================================================================================
*/

// ** cLSCMMesh::cLSCMMesh
cLSCMMesh::cLSCMMesh( void ) : in_facet( false )
{
}

sVertex* cLSCMMesh::AddVertex( void )
{
	vertex.push_back( sVertex() );
	vertex.rbegin()->id = vertex.size() - 1;
	return &*( vertex.rbegin() );
}

sVertex* cLSCMMesh::AddVertex( const sVector3& p, const sVector2& t )
{
	vertex.push_back( sVertex( p, t ) );
	vertex.rbegin()->id = vertex.size() - 1;
	return &*( vertex.rbegin() );
}


void cLSCMMesh::BeginFacet( void )
{
	assert( !in_facet );
	facet.push_back( sFacet() );
	in_facet = true;
}

void cLSCMMesh::EndFacet( void )
{
	assert(in_facet);
	in_facet = false ;
}

void cLSCMMesh::AddVertexToFacet( unsigned int i )
{
	assert( in_facet );
	assert( i < vertex.size() ) ;
	facet.rbegin()->push_back( i );
}

void cLSCMMesh::Clear( void )
{
	vertex.clear();
	facet.clear();
}

void cLSCMMesh::Load( const std::string& file_name )
{
	std::ifstream input( file_name.c_str() );
	Clear();
	while( input ) {
		char line[1024] ;
		input.getline(line, 1024) ;
		std::stringstream line_input(line) ;
		std::string keyword ;
		line_input >> keyword ;
		if(keyword == "v") {
                sVector3 p ;
                line_input >> p ;
                AddVertex(p,sVector2(0,0)) ;
            } else if(keyword == "vt") {
                // Ignore tex vertices
            } else if(keyword == "f") {
                BeginFacet() ;
                while(line_input) {
                    std::string s ;
                    line_input >> s ;
                    if(s.length() > 0) {
                        std::stringstream v_input(s.c_str()) ;
                        int index ;
                        v_input >> index ;
                        AddVertexToFacet(index - 1) ;
                        char c ;
                        v_input >> c ;
                        if(c == '/') {
                            v_input >> index ;
                            // Ignore tex vertex index
                        }
                    }
                }
                EndFacet() ;
            }
        } 
}

void cLSCMMesh::Save( const std::string& file_name )
{
        unsigned int i,j ;
        std::ofstream out(file_name.c_str()) ;
        for(i=0; i<vertex.size(); i++) {
            out << "v " << vertex[i].pos << std::endl ;
        }
        for(i=0; i<vertex.size(); i++) {
           out << "vt " << vertex[i].uv << std::endl ;
        }
        for(i=0; i<facet.size(); i++) {
            out << "f " ;
            const sFacet& F = facet[i] ;
            for(j=0; j<F.size(); j++) {
                out << (F[j] + 1) << "/" << (F[j] + 1) << " " ;
            }
            out << std::endl ;
        }
        for(i=0; i<vertex.size(); i++) {
            if(vertex[i].locked) {
                out << "# anchor " << i+1 << std::endl ;
            }
        }
    }

// ------------------------------------ cUVUnwrap ------------------------------------ //

#ifdef HAVE_OPEN_NL

void cUVUnwrap::Unwrap( cLSCMMesh *_mesh )
{
	mesh = _mesh;

	int nb_vertices = mesh->vertex.size();
	Project();

	nlNewContext();
	nlSolverParameteri( NL_SOLVER, NL_CG );
	nlSolverParameteri( NL_PRECONDITIONER, NL_PRECOND_JACOBI );

	nlSolverParameteri( NL_NB_VARIABLES, 2 * nb_vertices );
	nlSolverParameteri( NL_LEAST_SQUARES, NL_TRUE );
	nlSolverParameteri( NL_MAX_ITERATIONS, 5 * nb_vertices );
	nlSolverParameterd( NL_THRESHOLD, 1e-10 );

	nlBegin( NL_SYSTEM );
	MeshToSolver();

	nlBegin( NL_MATRIX );
	SetupForLSCM();
	nlEnd( NL_MATRIX ) ;
	nlEnd( NL_SYSTEM ) ;
	nlSolve();
	SolverToMesh();

	nlDeleteContext( nlGetCurrent() );
}

// ** SetupForLSCM
void cUVUnwrap::SetupForLSCM( void )
{
	for( unsigned int i = 0; i < mesh->facet.size(); i++ ) {
		const sFacet& facet = mesh->facet[i] ;
		SetupFaceForLSCM( facet );
	}
}

// ** cUVUnwrap::SetupFaceForLSCM
void cUVUnwrap::SetupFaceForLSCM( const sFacet& face )
{
	for(unsigned int i = 1; i < face.size() - 1; i++ ) {
		SetupLSCMRelations( mesh->vertex[face[0]], mesh->vertex[face[i]], mesh->vertex[face[i + 1]] );
	}
}

// ** cUVUnwrap::ProjectTriangle
void cUVUnwrap::ProjectTriangle( const sVector3& p0, const sVector3& p1, const sVector3& p2, sVector2& z0, sVector2& z1, sVector2& z2 )
{
	sVector3 X = p1 - p0;
	X.normalize();
	sVector3 Z = X ^ (p2 - p0);
	Z.normalize() ;
	sVector3 Y = Z ^ X;

	const sVector3& O = p0;
        
	double x0 = 0 ;
	double y0 = 0 ;
	double x1 = (p1 - O).length() ;
	double y1 = 0 ;
	double x2 = (p2 - O) * X ;
	double y2 = (p2 - O) * Y ;        
            
	z0 = sVector2( x0, y0 );
	z1 = sVector2( x1, y1 );
	z2 = sVector2( x2, y2 ); 
}

// ** cUVUnwrap::SetupLSCMRelations
void cUVUnwrap::SetupLSCMRelations( const sVertex& v0, const sVertex& v1, const sVertex& v2 )
{
	int id0 = v0.id;
	int id1 = v1.id;
	int id2 = v2.id;
            
	const sVector3& p0 = v0.pos; 
	const sVector3& p1 = v1.pos;
	const sVector3& p2 = v2.pos;
            
	sVector2 z0,z1,z2 ;
	ProjectTriangle( p0, p1, p2, z0, z1, z2 );
	sVector2 z01 = z1 - z0 ;
	sVector2 z02 = z2 - z0 ;
	double a = z01.x ;
	double b = z01.y ;
	double c = z02.x ;
	double d = z02.y ;
	assert(b == 0.0) ;

	// Note  : 2*id + 0 --> u
	//         2*id + 1 --> v
	int u0_id = 2*id0     ;
	int v0_id = 2*id0 + 1 ;
	int u1_id = 2*id1     ;
	int v1_id = 2*id1 + 1 ;
	int u2_id = 2*id2     ;
	int v2_id = 2*id2 + 1 ;
        
	// Note : b = 0

	// Real part
	nlBegin( NL_ROW ) ;
	nlCoefficient( u0_id, -a+c )  ;
	nlCoefficient( v0_id,  b-d )  ;
	nlCoefficient( u1_id,   -c )  ;
	nlCoefficient( v1_id,    d )  ;
	nlCoefficient( u2_id,    a ) ;
	nlEnd( NL_ROW) ;

	// Imaginary part
	nlBegin( NL_ROW );
	nlCoefficient( u0_id, -b+d ) ;
	nlCoefficient( v0_id, -a+c ) ;
	nlCoefficient( u1_id,   -d ) ;
	nlCoefficient( v1_id,   -c ) ;
	nlCoefficient( v2_id,    a ) ;
	nlEnd( NL_ROW );
}

// ** cUVUnwrap::SolverToMesh
void cUVUnwrap::SolverToMesh( void )
{
	for( unsigned int i = 0; i < mesh->vertex.size(); i++ ) {
		sVertex& it = mesh->vertex[i];
		double u	= nlGetVariable( 2 * it.id     );
		double v	= nlGetVariable( 2 * it.id + 1 );
		it.uv		= sVector2( u * 0.5 + 0.5, v * 0.5 + 0.5 );
	}
}

// ** cUVUnwrap::MeshToSolver
void cUVUnwrap::MeshToSolver( void )
{
	for( unsigned int i = 0; i < mesh->vertex.size(); i++ ) {
		sVertex& it = mesh->vertex[i];
		double u	= it.uv.x;
		double v	= it.uv.y;
		nlSetVariable( 2 * it.id    , u );
		nlSetVariable( 2 * it.id + 1, v );
		if( it.locked ) {
			nlLockVariable( 2 * it.id     );
			nlLockVariable( 2 * it.id + 1 );
		} 
	}
}

// ** cUVUnwrap::Project
void cUVUnwrap::Project( void )
{
	unsigned int i ;

	lmTYPE xmin =  1e30 ;
	lmTYPE ymin =  1e30 ;
	lmTYPE zmin =  1e30 ;
	lmTYPE xmax = -1e30 ;
	lmTYPE ymax = -1e30 ;
	lmTYPE zmax = -1e30 ;

	for( i = 0; i < mesh->vertex.size(); i++ ) {
		const sVertex& v = mesh->vertex[i];

		xmin = nl_min( v.pos.x, xmin );
		ymin = nl_min( v.pos.y, xmin );
		zmin = nl_min( v.pos.z, xmin );

		xmax = nl_max( v.pos.x, xmin );
		ymax = nl_max( v.pos.y, xmin );
		zmax = nl_max( v.pos.z, xmin );
	}

	double dx = xmax - xmin;
	double dy = ymax - ymin;
	double dz = zmax - zmin;
        
	sVector3 V1, V2;

	// Find shortest bbox axis
	if( dx < dy && dx < dz ) {
		if( dy > dz ) {
			V1 = sVector3( 0, 1, 0 );
			V2 = sVector3( 0, 0, 1 );
		} else {
			V2 = sVector3(0,1,0) ;
			V1 = sVector3(0,0,1) ;
		}
	} else if(dy <= dx && dy <= dz) {
		if(dx > dz) {
			V1 = sVector3(1,0,0) ;
			V2 = sVector3(0,0,1) ;
		} else {
			V2 = sVector3(1,0,0) ;
			V1 = sVector3(0,0,1) ;
		}
	} else if(dz <= dx && dz <= dy) {
		if(dx > dy) {
			V1 = sVector3(1,0,0) ;
			V2 = sVector3(0,1,0) ;
		} else {
			V2 = sVector3(1,0,0) ;
			V1 = sVector3(0,1,0) ;
		}
	}

	// Project onto shortest bbox axis,
	// and lock extrema vertices

	sVertex* vxmin	= NULL ;
	double   umin	= 1e30 ;
	sVertex* vxmax	= NULL ;
	double   umax	= -1e30 ;

	for( i = 0; i < mesh->vertex.size(); i++ ) {
		sVertex& V = mesh->vertex[i];

		double u = V.pos * V1;
		double v = V.pos * V2;

		V.uv = sVector2( u, v );
		if(u < umin) {
			vxmin = &V;
			umin = u;
		} 
		if(u > umax) {
			vxmax = &V;
			umax = u;
		} 
	}

	vxmin->locked = true;
	vxmax->locked = true;
}

#endif

// ----------------------------------- cSimpleUnwrap ----------------------------------- //\

#if 0

// ** cSimpleUnwrap::Unwrap
void cSimpleUnwrap::Unwrap( sMesh *_mesh )
{
	mesh = _mesh;

	// ** Vertices
	for( int i = 0; i < mesh->totalVertices; i++ ) {
		mesh->vertices[i].uv[0] = FLT_MIN;
		mesh->vertices[i].uv[1] = FLT_MIN;

		vertices.push_back( triMesh.add_vertex( tTriangleMesh::Point( mesh->vertices[i].position[0], mesh->vertices[i].position[1], mesh->vertices[i].position[2] ) ) );
	}

	// ** Faces
	for( int i = 0; i < mesh->totalFaces; i++ ) {
		tVertexHandles vface;

		vface.push_back( vertices[ mesh->indices[i * 3 + 0] ] );
		vface.push_back( vertices[ mesh->indices[i * 3 + 1] ] );
		vface.push_back( vertices[ mesh->indices[i * 3 + 2] ] );

		faces.push_back( triMesh.add_face( vface ) );
	}

	// ** Unwrap faces
	for( tTriangleMesh::FaceIter i = triMesh.faces_begin(); i != triMesh.faces_end(); i++ ) {
		BeginIsland();
		UnwrapFace( mesh->vertices, i, NULL );
	}

	NormalizeUV();
}

// ** cSimpleUnwrap::BeginIsland
void cSimpleUnwrap::BeginIsland( void )
{
	r = rand() % 255 / 255.0;
	g = rand() % 255 / 255.0;
	b = rand() % 255 / 255.0;
}

// ** cSimpleUnwrap::UnwrapFace
void cSimpleUnwrap::UnwrapFace( sMeshVertex *vertices, const tFace& face, const int *adj )
{
	if( face.idx() < 0 || IsFaceProcessed( face ) ) {
		return;
	}

	int faceIndex = face.idx();
	int *idx	  = &mesh->indices[ faceIndex * 3 ];

	for( int i = 0; i < 3; i++ ) {
		vertices[ idx[i] ].color[0] = r;
		vertices[ idx[i] ].color[1] = g;
		vertices[ idx[i] ].color[2] = b;
	}

	if( adj ) {
		return;
	}

	int index[3];

	// ** Neighbors
	for( tTriangleMesh::FaceFaceIter i = triMesh.ff_begin( face ); i != triMesh.ff_end( face ); i++ ) {
		const tFace& f = i.handle();

		UnwrapFace( vertices, i, index );
	}
/*
	int faceIndex = face.idx();
	int *idx	  = &mesh->indices[ faceIndex * 3 ];

	sMeshVertex& A = vertices[ idx[0] ];
	sMeshVertex& B = vertices[ idx[1] ];
	sMeshVertex& C = vertices[ idx[2] ];

	A.color[0] = r, A.color[1] = g, A.color[2] = b;
	B.color[0] = r, B.color[1] = g, B.color[2] = b;
	C.color[0] = r, C.color[1] = g, C.color[2] = b;

	float AB = (sVector3( A.position ) - sVector3( B.position )).length();
	float AC = (sVector3( A.position ) - sVector3( C.position )).length();
	float BC = (sVector3( B.position ) - sVector3( C.position )).length();

	if( !adj ) {
		A.uv[0] = 0,  A.uv[1] = 0;
		B.uv[0] = AB, B.uv[1] = 0;
	}

	sIntersection intersection = CircleToCircleIntersection( A.uv, AC, B.uv, BC );
	if( intersection.solutions == 0 ) {
		return;
	}
	_ASSERTE( intersection.solutions == 2 );

	if( !IsPointInsideTriangles( vertices, intersection.a ) ) {
		if( adj ) {
			for( int i = 0; i < 3; i++ ) {
				sMeshVertex& v = vertices[ idx[i] ];

				if( v.uv[0] == FLT_MIN || v.uv[1] == FLT_MIN ) {
					v.uv[0] = intersection.a.x, v.uv[1] = intersection.a.y;
					break;
				}
			}
		} else {
			C.uv[0] = intersection.a.x, C.uv[1] = intersection.a.y;
		}
	}
	else if( !IsPointInsideTriangles( vertices, intersection.b ) ) {
		if( adj ) {
			for( int i = 0; i < 3; i++ ) {
				sMeshVertex& v = vertices[ idx[i] ];

				if( v.uv[0] == FLT_MIN || v.uv[1] == FLT_MIN ) {
					v.uv[0] = intersection.b.x, v.uv[1] = intersection.b.y;
					break;
				}
			}
		} else {
			C.uv[0] = intersection.b.x, C.uv[1] = intersection.b.y;
		}
	}
	else {
		GenerateColor();
	//	_ASSERTE( "Island" && NULL );
	}

	printf( "Triangle UV:\t(%2.2f %2.2f) (%2.2f %2.2f) (%2.2f %2.2f)\n", A.uv[0], A.uv[1], B.uv[0], B.uv[1], C.uv[0], C.uv[1] );
	printf( "\t\t\t%d %d %d\n", idx[0], idx[1], idx[2] );

	processed.push_back( face );

	// ** Neighbors
	for( tTriangleMesh::FaceFaceIter i = triMesh.ff_begin( face ); i != triMesh.ff_end( face ); i++ ) {
		const tFace& f = i.handle();

		UnwrapFace( vertices, i, idx );
	}*/
}

// ** cSimpleUnwrap::NormalizeUV
void cSimpleUnwrap::NormalizeUV( void )
{
	float minU = FLT_MAX, maxU = FLT_MIN;
	float minV = FLT_MAX, maxV = FLT_MIN;

	// ** Calculate min/max
	for( int i = 0; i < mesh->totalVertices; i++ ) {
		sMeshVertex& v = mesh->vertices[i];

		if( v.uv[0] < minU ) minU = v.uv[0];
		if( v.uv[0] > maxU ) maxU = v.uv[0];

		if( v.uv[1] < minV ) minV = v.uv[1];
		if( v.uv[1] > maxV ) maxV = v.uv[1];
	}

	// ** Normalize
	for( int i = 0; i < mesh->totalVertices; i++ ) {
		sMeshVertex& v = mesh->vertices[i];

		v.uv[0] = (v.uv[0] - minU) / (maxU - minU);
		v.uv[1] = (v.uv[1] - minV) / (maxV - minV);
	}
}

// ** cSimpleUnwrap::IsFaceProcessed
bool cSimpleUnwrap::IsFaceProcessed( const tFace& face ) const
{
	return std::find( processed.begin(), processed.end(), face ) != processed.end();
}

// ** cSimpleUnwrap::IsPointInsideTriangles
bool cSimpleUnwrap::IsPointInsideTriangles( const sMeshVertex *vertices, const sVector2& point ) const
{
	for( int i = 0, n = ( int )processed.size(); i < n; i++ ) {
		int *idx = &mesh->indices[ processed[i].idx() * 3 ];

		sVector2 a = vertices[ idx[0] ].uv;
		sVector2 b = vertices[ idx[1] ].uv;
		sVector2 c = vertices[ idx[2] ].uv;

		if( cLightmapper::IsInsideTriangle( a, b, c, point ) ) {
			return true;
		}
	}

	return false;
}

/*

1. берем треугольник (ABC) и измеряем длины ребер (AB, AC, BC) 
2. откладываем в текстурной плоскости первое ребро (AB): точки будут такие - At(0,0); Bt(длина(AB), 0) 
3. далее мысленно проводим 2 окружности с центрами в точках At и Bt радиусов длина(AC) и длина(BC) соответственно 
4. взяв с гугле функцию пересечения двух окружностей получаем 2 точки: Сt1, Ct2
остается высчитать точку Ct:

если Ct1 не попадает ни в какой треугольник, для которого уже расчитаны At, Bt, Ct, то {Ct = Ct1} 
иначе 
{ 
  если Ct2 не попадает ни в какой треугольник, для которого уже расчитаны At, Bt, Ct, то {Ct = Ct2} 
  иначе 
  { 
    // треугольник пересекается с уже расчитанными треугльниками 
    группируем все расчитанные треугольники в "остров" (прячем остров куданить в массив) и начинаем с пункта 1 и уже данного треугольника (он опять считается за первый) 
    goto Пункт_1; 
  } 
}

следующий треугольник берем смежный с текущим, первое ребро всегда будет совпадать - так что сразу goto Пункт_3; 
(смежный треугольник может быть не один поэтому делаем все рекурсивно)

в результате получаем один или несколько островов 
*/

// ** cSimpleUnwrap::CircleToCircleIntersection
cSimpleUnwrap::sIntersection cSimpleUnwrap::CircleToCircleIntersection( const sVector2& circlePosition1, float radius1, const sVector2& circlePosition2, float radius2 ) const
{
	sIntersection result;
	float a, distSq, dist, h;
	sVector2 d, r, v2;

	// d is the vertical and horizontal distances between the circle centers
	d = circlePosition1 - circlePosition2;

	//distance squared between the circles
	distSq = d.lengthSq();

	// Check for equality and infinite intersections exist
	if (distSq == 0 && radius1 == radius2)
	{
		return result;
	}

	//Check for solvability
	if (distSq > (radius1 + radius2) * (radius1 + radius2))
	{
		// no solution. circles do not intersect
		return result;
	}

	if (distSq < abs(radius1 - radius2) * abs(radius1 - radius2))
	{
		// no solution. one circle is contained in the other
		return result;
	}
		
	if (distSq == (radius1 + radius2) * (radius1 + radius2))
	{
		//one solution
		result.InsertSolution((circlePosition2 - circlePosition1) / (radius1 + radius2) * radius1 + circlePosition1);
		return result;
	}

	dist = sqrt(distSq);

	// 'point 2' is the point where the line through the circle
	// intersection points crosses the line between the circle
	// centers.

	// Determine the distance from point 0 to point 2
	a = ((radius1 * radius1) - (radius2 * radius2) + distSq) / (2.0f * dist);

	// Determine the coordinates of point 2
	v2 = circlePosition1 - d * (a / dist);

	// Determine the distance from point 2 to either of the intersection points
	h = sqrt((radius1 * radius1) - (a * a));

	d.normalize();
	sVector2 p( d.y, -d.x );

	result.InsertSolution( v2 + p * h );
	result.InsertSolution( v2 - p * h );

	return result;
}

#endif