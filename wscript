#! /usr/bin/env python
# encoding: utf-8

# Notes
# - host_speed reports almost always zero for anything else than ref
#   so the renderer greatly dominates frame time
# - renderer frame seems to take less when using cl_maxfps, which makes
#   sense as some frames fit on a timeslice uninterrupted by the scheduler
#   because it uses usleep() often (and then is where it pauses)
# - there are some commands not in menu: {hi,medium,low}_spec
# - cl_maxfps works fine at 60, and uses usleep() to wait
#

VERSION = '1.26.8'
APPNAME = 'quake2xp'
top = '.'
out = 'build'

# Sources
sources_glob = {
    'game' : [
        'game/*.c'
        ],
    'xatrix' : [
        'xsrc/*.c'
        ],
    'rogue' : [
        'roguesrc/**/*.c'
        ],
    'client' : [
        'game/q_shared.c',
        'client/*.c',
        'qcommon/*.c',
        'server/*.c',
        'ref_gl/*.c',
        'linux/*.c',
        'game/m_flash.c'
        ]
}

def dist(ctx):
    ctx.excl  = '**/.svn **/.waf* **/.lock-w* Libs/* build/* test/* **/*.vcproj* **/*.vcxproj*'

def options(opt):
    opt.load('compiler_c')

def configure(conf):
    conf.load('compiler_c')
    for lib in ['sdl', 'ogg', 'vorbis', 'vorbisfile', 'IL', 'ILU', 'ILUT', 'openal', 'x11']:
        conf.check_cfg(package=lib, args=['--cflags', '--libs'])

def build(bld):
    src_dir = bld.srcnode
    #src_dir = bld.path.find_dir('src')

    bld.env.append_value('CFLAGS', ['-Wno-unused-result'])
    # bld.env.append_value('CFLAGS', ['-O3', '-march=native'])
    bld.env.append_value('CFLAGS', ['-O3'])
    #bld.env.append_value('CFLAGS', ['-g', '-Wall'])
    #bld.env.append_value('CFLAGS', ['-pg', '-O3', '-march=native'])
    #bld.env.append_value('LINKFLAGS', ['-pg'])

    # Expand source files
    sources = {}
    for k, v in sources_glob.items():
        sources[k] = []
        for pat in v:
            sources[k] += src_dir.ant_glob(pat)

    bld.env.DATADIR = bld.env.PREFIX + '/share/quake2xp'

    # Game shared library environment
    genv = bld.env.derive()
    genv.cshlib_PATTERN = genv.cshlib_PATTERN.replace('lib', '')

    bld.shlib(
        source = sources['game'],
        target = 'baseq2/gamexp',
        install_path = '${DATADIR}/baseq2',
        env = genv
        )

    bld.shlib(
        source = sources['xatrix'],
        target = 'xatrix/gamexp',
        install_path = '${DATADIR}/xatrix',
        env = genv
        )

    bld.shlib(
        source = sources['rogue'],
        target = 'rogue/game',
        install_path = '${DATADIR}/rogue',
        env = genv
        )

    bld.program(
        cflags = '-DSYSTEMWIDE="' + bld.env.DATADIR + '"',
        source = sources['client'],
        target = 'quake2xp',
        lib = ['z', 'm', 'dl'],
        use = ['IL', 'ILU', 'ILUT', 'OPENAL', 'X11', 'SDL', 'OGG', 'VORBIS', 'VORBISFILE']
    )
