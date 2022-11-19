project('Koolang', 'cpp',
    version : '1.0',
    license : 'Apache-2.0',
    default_options : [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++23',
        'b_sanitize=address',
        'b_lundef=false',
    ],
    meson_version : '>= 0.56',
)

compiler = meson.get_compiler('cpp')
if compiler.get_id() != 'msvc'
    add_project_arguments(
        '-Wshadow',
        '-Wconversion',
        '-Wold-style-cast',
        language : 'cpp'
    )
endif


if get_option('buildtype') == 'debug'
    add_project_arguments('-DKOOLANG_DEBUG_MODE', language: 'cpp')
endif

main_sources = []
libs = []

inc = include_directories('src')

subdir('src')

if get_option('enable-tests')
    subdir('test')
endif

main_executable = executable('koolang', main_sources,
    include_directories : inc,
    link_with : libs
)



test('Main executable test', main_executable)
