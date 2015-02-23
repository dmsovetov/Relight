#include <GLUT/GLUT.h>

#include "TestRadiosity.h"
#include "TestLightmapper.h"

cTest* test = NULL;

void keyPressed (unsigned char key, int x, int y)
{
	if( test ) {
		test->KeyPressed( key );
	}
}  

void windowReshapeFunc( GLint width, GLint height )
{
	glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, width, height);
	GLfloat aspect = (GLfloat)width / height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(60, aspect, 0.25, 100);
    glMatrixMode(GL_MODELVIEW);
}

void display()
{
	if( test ) {
		test->Update();
	}

//	glClearColor( 0.1, 0.2, 0.3, 1.0 );

    glClearColor( 0.52734375f, 0.8046875f, 0.91796875f, 1.0f ); // Sky blue
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

//    float fogColor[] = { 0.05, 0.1, 0.15, 1.0 };
//    float fogStart = 5;

    float fogColor[] = { 0.52734375f, 0.8046875f, 0.91796875f, 1.0f };
    float fogStart = 0.05f;

//    glEnable( GL_FOG );
    glFogi( GL_FOG_MODE, GL_LINEAR);
    glFogfv( GL_FOG_COLOR, fogColor );
    glFogf( GL_FOG_DENSITY, 0.2f );
    glFogf( GL_FOG_START, fogStart );
    glFogf( GL_FOG_END, 8 );

	glEnable( GL_TEXTURE_2D );
	glPushMatrix();

	// ** Test rendering
	if( test ) {
        test->Render();
	}

	glPopMatrix();

    glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    int window_width = 800;
    int window_height = 600;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

    glutInitWindowPosition(300, 20);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow(argv[0]);
	glutSetWindowTitle( "Lightmapper" );
	glutKeyboardFunc(keyPressed);
	glutReshapeFunc(windowReshapeFunc);

    glMatrixMode(GL_PROJECTION);
	glViewport(0, 0, window_width, window_height);
	GLfloat aspect = (GLfloat) window_width / window_height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(60, aspect, 0.25, 100);
    glMatrixMode(GL_MODELVIEW);
    glShadeModel( GL_SMOOTH );
    glClearColor( 0.0f, 0.1f, 0.0f, 0.5f );
    glClearDepth( 1.0f );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
	glDisable( GL_CULL_FACE );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	// ** Create tests
//	test = new cTestRadiosity( lights, sizeof( lights ) / sizeof( sLight ) );
	test = new cTestLightmapper;
	test->Create();

    glutDisplayFunc(display);
    glutMainLoop();
    return 1;
}