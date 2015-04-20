demo = Executable( 'demo', sources = [ '.' ], paths = [ '../relight', '../../modules/dreemchest/src/dreemchest', '../../modules/uscene/src' ], link = [ 'relight', 'dRenderer', 'dThreads', 'dPlatform', 'uScene' ] )
demo.linkExternal( Library( 'OpenGL', True ), Library( 'fbx', True ), Library( 'yaml', True ) )

if MacOS:
	demo.linkExternal( Library( 'QuartzCore', True ) )