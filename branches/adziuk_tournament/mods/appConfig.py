from OpenNero import *
import sys
import getopt

# use these constants to set startup properties
# if you want to know what these mean, look at C++ docs for AppConfig

def usage():
    print 'USAGE:'
    print '   OpenNERO [options]'
    print ' OPTIONS:'
    print '   --window-title=...'
    print '     Set the window title'
    print '   --log-config-file=...'
    print '     Specify the log config file to use'
    print '   --no-window'
    print '     Do not use graphics - headless mode'
    print '   --width=...'
    print '     Window width in pixels'
    print '   --height=...'
    print '     Window height in pixels'
    print '   --depth=...'
    print '     Color depth in bits'
    print '   --fullscreen'
    print '     Use the full screen'
    print '   --mod-path=...'
    print '     Startup mod path to use'
    print '   --mod=...'
    print '     Startup mod to use'
    print '   --command=...'
    print '     Python command to run immediately after loading the mod'
    print '   --seeds=...'
    print '     Random seeds to use'

# list of options
config_opts = [ \
               'window-title=', \
               'log-config-file=', \
               'no-window', \
               'width=', \
               'height=', \
               'depth=', \
               'fullscreen', \
               'mod-path=', \
               'mod=', \
               'command=', \
               'seeds=' \
              ]

def default_app_config():
    """get the default app_config"""
    app_config = AppConfig()
    app_config.window_title = 'OpenNero'
    app_config.log_config_file = 'logConfig.py'
    app_config.renderer = 'OpenGL' 
    app_config.window_width = 800
    app_config.window_height = 600
    app_config.bpp = 16
    app_config.fullscreen = False
    app_config.use_stencil_buffer = True
    app_config.use_vsync = True
    app_config.start_mod_dir = "Blocksworld:common"
    app_config.start_mod_name = "Blocksworld"
    app_config.start_command = ""
    app_config.seeds = "55555"
    return app_config

def process_config_opts(app_config, opts):
    output = None
    verbose = False
    for o, a in opts:
        if o == '--window-title':
            app_config.window_title = a
        elif o == '--log-config-file':
            app_config.log_config_file = a
        elif o == '--no-window':
            app_config.renderer = 'null'
        elif o == '--width':
            app_config.window_width = int(a)
        elif o == '--height':
            app_config.window_height = int(a)
        elif o == '--depth':
            app_config.bpp = int(a)
        elif o == '--fullscreen':
            app_config.fullscreen = True
        elif o == '--mod-path':
            app_config.start_mod_dir = a
        elif o == '--mod':
            app_config.start_mod_name = a
        elif o == '--command':
            app_config.start_command = a
        elif o == '--seeds':
            app_config.seeds
        else:
            assert False, 'Unhandled option: %s with value %s' % (o,a) 

app_config = default_app_config()
try:
    opts, args = getopt.getopt(sys.argv[1:], '', config_opts)
    process_config_opts(app_config, opts)
except getopt.GetoptError, err:
    # print help information and exit:
    print str(err) # will print something like "option -a not recognized"
    usage()
    sys.exit(2)

print str(app_config)
