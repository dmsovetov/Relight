Project( externals = '../externals', paths = [ '../externals/src' ] )

Include( 'demo' )
Include( 'relight' )

Module( url = 'https://github.com/dmsovetov/foo.git',        folder = '../externals/src' )
Module( url = 'https://github.com/dmsovetov/dreemchest.git', folder = '../externals/src', makefile = 'src' )
Module( url = 'https://github.com/dmsovetov/uscene.git',     folder = '../externals/src', makefile = 'src' )