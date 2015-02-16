demo = Executable( 'demo', sources = [ '.' ], paths = [ '../relight' ], link = [ 'relight' ] )
demo.frameworks( 'OpenGL', 'GLUT' )