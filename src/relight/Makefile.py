relight = StaticLibrary( 'relight', sources = [ '.', 'baker', 'scene', 'rt' ], defines = [ 'RELIGHT_BUILD_LIBRARY' ] )
relight.linkExternal( Library( 'embree2', True ) )