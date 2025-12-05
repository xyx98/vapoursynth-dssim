project('vapoursynth-dssim', 'c',
    default_options: ['buildtype=release', 'b_ndebug=if-release', 'c_std=c23','warning_level=2','b_lto=false'],
    meson_version: '>=0.51.0',
    version: '1.0.0'
)

add_project_arguments('-Wno-unused-parameter','-Wno-discarded-qualifiers', language : 'c')

sources = [ 
    'dssim.c',
    ]

libs = []

deps = [
    dependency('vapoursynth').partial_dependency(compile_args : true, includes : true),
    dependency('dssim',static : true,method : 'pkg-config' ,version: '>=3.0.0'),
]


shared_module('vs_dssim', sources,
    dependencies: deps,
    name_prefix: '',
    install: true,
    install_dir: join_paths(deps[0].get_variable(pkgconfig: 'libdir'), 'vapoursynth'),
    gnu_symbol_visibility : 'hidden'
)