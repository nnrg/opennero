from OpenNero import *

# use these constants to set startup properties
# if you want to know what these mean, look at C++ docs for AppConfig

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
app_config.start_mod_dir = "hub:common"
app_config.start_mod_name = "hub"
app_config.seeds = "55555"

# use these constants to play with physics
# make sure NERO_BUILD_PHYSICS is set to 1 during compile (in Preprocessor.h)
# if you want to change a default, copy-uncomment-change
# if you want to know what these mean, look at the C++ docs of PhysicsConfig
#
# physics_config = PhysicsConfig()
# physics_config.tracking_radius = 1000
# physics_config.gravity = 9.8
# physics_config.CFM = 1e-5
# physics_config.ERP = 0.10
# physics_config.damping = 0.01
# physics_config.damping_torque = 0.01
# physics_config.default_mass = 10
# physics_config.step_size = 0.005
# physics_config.ground_offset = -20
# app_config.physics = physics_config

print str(app_config)
