demo = Executable( 'demo', sources = [ '.' ], paths = [ '../relight' ], link = [ 'relight' ] )
demo.linkExternal( Library( 'OpenGL', True ), Library( 'GLUT', True ) )