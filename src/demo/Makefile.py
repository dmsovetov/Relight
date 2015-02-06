#embree = findPackage( 'embree'  )
demo = Executable( 'demo', sources = [ '.' ], include = [ '../relight', '/usr/local/include' ], libs = [ 'relight' ] )
demo.frameworks( 'OpenGL', 'GLUT' )