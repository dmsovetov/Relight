Project( externals = '../externals', paths = '../modules' )

Include( 'demo' )
Include( 'relight' )

Module( url = 'https://github.com/dmsovetov/math.git',       folder = '../modules' )
Module( url = 'https://github.com/dmsovetov/dreemchest.git', folder = '../modules', makefile = 'src' )
Module( url = 'https://github.com/dmsovetov/uscene.git',     folder = '../modules', makefile = 'src' )