import os
import shutil
import patoolib
import urllib.request as req
import os.path as path

appfile = path.join('app', 'vk_music_fs')
desktop = 'vk_music_fs.desktop'
icon = 'vk_music_fs-icon.svg'

rootdir = 'vk_music_fs'
tmpdir = path.join('vk_music_fs', 'tmp')
appdir = path.join(rootdir, 'vk_music_fs.AppDir')

packages = [
    'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-filesystem1.67.0_1.67.0-7_i386.deb',
    'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-program-options1.67.0_1.67.0-7_i386.deb',
    'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-system1.67.0_1.67.0-7_i386.deb',
    'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-thread1.67.0_1.67.0-7_i386.deb',
    'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-chrono1.67.0_1.67.0-7_i386.deb',
    'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-atomic1.67.0_1.67.0-7_i386.deb',
    'http://deb.debian.org/debian/pool/main/b/boost1.67/libboost-date-time1.67.0_1.67.0-7_i386.deb',

    'http://deb.debian.org/debian/pool/main/z/zlib/zlib1g_1.2.11.dfsg-1_i386.deb',
    'http://deb.debian.org/debian/pool/main/g/gcc-8/libstdc++6_8.2.0-7_i386.deb',
    'http://deb.debian.org/debian/pool/main/o/openssl/libssl1.1_1.1.0h-4_i386.deb'
]

deps = [
    'lib/i386-linux-gnu/libz.so.1',
    'usr/lib/i386-linux-gnu/libboost_filesystem.so.1.67.0',
    'usr/lib/i386-linux-gnu/libboost_thread.so.1.67.0',
    'usr/lib/i386-linux-gnu/libboost_program_options.so.1.67.0',
    'usr/lib/i386-linux-gnu/libboost_system.so.1.67.0',
    'usr/lib/i386-linux-gnu/libboost_chrono.so.1.67.0',
    'usr/lib/i386-linux-gnu/libboost_date_time.so.1.67.0',
    'usr/lib/i386-linux-gnu/libboost_atomic.so.1.67.0',
    'usr/lib/i386-linux-gnu/libssl.so.1.1',
    'usr/lib/i386-linux-gnu/libcrypto.so.1.1',
]

extradeps = {
    'usr/lib/i386-linux-gnu/libstdc++.so.6': 'usr/optional/libstdc++/libstdc++.so.6'
}

apprun = 'https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-i686'

os.makedirs(appdir)
os.makedirs(path.join(appdir, 'usr', 'bin'))
os.makedirs(tmpdir)

shutil.copy(appfile, os.path.join(appdir, 'usr', 'bin'))
shutil.copy(desktop, os.path.join(appdir, desktop))
shutil.copy(icon, os.path.join(appdir, icon))
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
