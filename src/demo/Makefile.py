demo = Executable( 'demo', sources = [ '.' ], paths = [ '../relight', '../../modules/dreemchest/src/dreemchest', '../../modules/uscene/src' ], link = [ 'relight', 'Dreemchest', 'uScene' ] )
demo.linkExternal( Library( 'OpenGL', True ), Library( 'fbx', True ), Library( 'yaml', True ) )

if platform == 'MacOS':
	demo.linkExternal( Library( 'QuartzCore', True ) )