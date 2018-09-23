import os
import shutil
import patoolib
import urllib.request as req
import os.path as path
from string import Template

archparts = [
    {
        'app': 'x86_64',
        'remoteapp': '_x86_64',
        'pkg': 'amd64',
        'lib': 'x86_64',
        'apprun': 'x86_64'
    },
    {
        'app': 'x86',
        'remoteapp': '',
        'pkg': 'i386',
        'lib': 'i386',
        'apprun': 'i686'
    }
]

for part in archparts:
    appfile = Template(path.join('app-$app', 'vk_music_fs')).substitute(part)
    appfile_remote = \
        Template('https://github.com/vodka2/vk-music-fs/releases/download/1.0/vk_music_fs$remoteapp').substitute(part)

    desktop = 'vk_music_fs.desktop'
    icon = 'vk_music_fs-icon.svg'

    rootdir = Template('vk_music_fs-$app').substitute(part)
    tmpdir = path.join(rootdir, 'tmp')
    appdir = path.join(rootdir, 'vk_music_fs.AppDir')

    packages = map(lambda s: Template(s).substitute(part), [
        'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-filesystem1.67.0_1.67.0-7_$pkg.deb',
        'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-program-options1.67.0_1.67.0-7_$pkg.deb',
        'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-system1.67.0_1.67.0-7_$pkg.deb',
        'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-thread1.67.0_1.67.0-7_$pkg.deb',
        'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-chrono1.67.0_1.67.0-7_$pkg.deb',
        'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-atomic1.67.0_1.67.0-7_$pkg.deb',
        'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-date-time1.67.0_1.67.0-7_$pkg.deb',

        'http://deb.debian.org/debian/pool/main/z/zlib/zlib1g_1.2.11.dfsg-1_$pkg.deb',
        'http://deb.debian.org/debian/pool/main/g/gcc-8/libstdc++6_8.2.0-7_$pkg.deb',
        'http://deb.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.0h-4_$pkg.deb'
    ])

    deps = map(lambda s: Template(s).substitute(part), [
        'lib/$lib-linux-gnu/libz.so.1',
        'usr/lib/$lib-linux-gnu/libboost_filesystem.so.1.67.0',
        'usr/lib/$lib-linux-gnu/libboost_thread.so.1.67.0',
        'usr/lib/$lib-linux-gnu/libboost_program_options.so.1.67.0',
        'usr/lib/$lib-linux-gnu/libboost_system.so.1.67.0',
        'usr/lib/$lib-linux-gnu/libboost_chrono.so.1.67.0',
        'usr/lib/$lib-linux-gnu/libboost_date_time.so.1.67.0',
        'usr/lib/$lib-linux-gnu/libboost_atomic.so.1.67.0',
        'usr/lib/$lib-linux-gnu/libssl.so.1.1',
        'usr/lib/$lib-linux-gnu/libcrypto.so.1.1',
    ])

    extradeps = dict(map(lambda o: (Template(o[0]).substitute(part), o[1]), {
        'usr/lib/$lib-linux-gnu/libstdc++.so.6': 'usr/optional/libstdc++/libstdc++.so.6'
    }.items()))

    apprun = Template(
        'https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-$apprun'
    ).substitute(part)

    os.makedirs(appdir)
    os.makedirs(path.join(appdir, 'usr', 'bin'))
    os.makedirs(tmpdir)

    if path.exists(appfile):
        shutil.copy(appfile, path.join(appdir, 'usr', 'bin'))
    else:
        req.urlretrieve(appfile_remote, path.join(appdir, 'usr', 'bin', 'vk_music_fs'))

    shutil.copy(desktop, path.join(appdir, desktop))
    shutil.copy(icon, path.join(appdir, icon))
    req.urlretrieve(apprun, path.join(appdir, 'AppRun'))

    for package in packages:
        print('Unpacking package ' + package)
        fname_dir = path.join(tmpdir, path.basename(package))
        fname = path.join(fname_dir, path.basename(package))
        os.makedirs(fname_dir)
        req.urlretrieve(package, fname)
        patoolib.extract_archive(fname, outdir=fname_dir)
        patoolib.extract_archive(path.join(fname_dir, 'data.tar'), outdir=tmpdir)

    for dep in deps:
        print('Copying file ' + dep)
        if not path.exists(path.join(appdir, path.dirname(dep))):
            os.makedirs(path.join(appdir, path.dirname(dep)))

        shutil.copy(path.join(tmpdir, dep), path.join(appdir, dep), follow_symlinks=True)

    for src, dest in extradeps.items():
        print('Copying file ' + src)

        if not path.exists(path.join(appdir, path.dirname(dest))):
            os.makedirs(path.join(appdir, path.dirname(dest)))

        shutil.copy(path.join(tmpdir, src), path.join(appdir, dest), follow_symlinks=True)
