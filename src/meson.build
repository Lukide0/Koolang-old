main_sources += files('main.cpp')

#-- config.h --#
config = configuration_data()
config.set('VERSION', '"@0@"'.format(meson.project_version()))

configure_file(
    output : 'config.h',
    configuration : config
)


subdir('logger')
subdir('terminal')
subdir('ast')
subdir('kir')
subdir('air')
