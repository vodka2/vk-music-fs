# VK Music FS

[![Build Status](https://travis-ci.com/vodka2/vk-music-fs.svg?branch=master)](https://travis-ci.com/vodka2/vk-music-fs)

![VK Music FS screenshot](https://i.imgur.com/OoiJfaW.gif)

FUSE virtual system for VK (Vkontakte) audios. VK account is required.

## Description

The program makes it possible to listen to the music from VK using your favorite player and copy songs to your PC. Searching and obtaining Mp3 files from "My audios" is supported. Missing ID3v2 tags are automatically added.

## Installation

### Windows

Windows 7 or later is supported.

1. Download and install [WinFsp driver](https://github.com/billziss-gh/winfsp/releases).
2. Download `vk_music_fs.exe` from releases
3. Copy WinFsp DLL from `C:\Program Files\WinFsp\bin` to the `vk_music_fs.exe` directory

### Linux

Download appimage from releases: `vk_music_fs-i386.AppImage` or `vk_music_fs-x86_64.AppImage`. Appimages are executable files with bundled dependencies.

Installation from source:

1. Install the following packages: `cmake g++ libssl-dev libfuse-dev libboost-system1.67-dev libboost-thread1.67-dev libboost-filesystem1.67-dev libboost-program-options1.67-dev zlib1g-dev` GCC compiler must support C++17
2. Execute in the sources directory: `mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --target vk_music_fs`
3. Copy `vk_music_fs` to the preferred location

### OS X

Installation from source:

1. Install the following packages `cmake gcc openssl osxfuse boost`
2. Execute in the sources directory:
```
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-8 -DCMAKE_C_COMPILER=/usr/local/bin/gcc-8 && cmake --build . --target vk_music_fs
```

## Usage

### General

#### Linux

Run `vk_music_fs --get_token vk_login vk_password`. The program will print the token and the user agent, copy these two lines to the `VkMusicFs.ini` file either in the same directory or in the configuration folder `~/.config/VkMusicFs/VkMusicFs.ini`. Create a mount point: `mkdir ~/VkMusicFs`. Then launch the program again: `vk_music_fs ~/VkMusicFs` and the file system will be mounted.

Please disable thumbnails for Mp3 files in the file system directory.

#### Windows

Run `vk_music_fs --get_token vk_login vk_password`. The program will print the token and the user agent, copy these two lines to the `VkMusicFs.ini` file either in the same directory or in the configuration folder `C:\Users\<Username>\AppData\Roaming\VkMusicFs\VkMusicFs.ini`. Then launch the program again: `vk_music_fs` and the file system will be mounted. The default mount point is `Z:`.

**Please select "Optimize this folder for: Documents" in folder properties, or it may be very slow.**

Only mounting as a disk is supported. It may be necessary to add `-o uid=-1,gid=-1` option when mounting fails.

#### OS X

Run `vk_music_fs --get_token vk_login vk_password`. The program will print the token and the user agent, copy these two lines to the `VkMusicFs.ini` file either in the same directory or in the configuration folder `~/Library/Application Support/VkMusicFs/VkMusicFs.ini`. Create a mount point: `mkdir ~/VkMusicFs`. Then launch the program again: `vk_music_fs -o modules=iconv,from_code=UTF-8,to_code=UTF-8-MAC ~/VkMusicFs` and the file system will be mounted.

Please disable thumbnails for the file system directory: in Finder press Command+J, then uncheck "Show icon preview" and click on "Use as defaults".


### Commands

In `My audios` and `Search` directories:

1. Create a directory with numeric name, and the parent directory will contain the specified number of Mp3 files
2. Create a directory with name like `10-30` to load 30 files starting from offset 10

In `Search` directory:

1. Create a directory with name, say, `Justin Bieber` to search for the songs by this artist
2. Create a directory with name `Baby` inside the `Justin Bieber` directory and the search query will be `Justin Bieber Baby`.

It is possible to delete directories and files.

### Command-line options

Default values may differ for your OS and extra FUSE options are not shown.

```
Usage ./vk_music_fs mountpoint [options]

General options:
  --token arg                           set token
  --user_agent arg                      set user agent
  --sizes_cache_size arg (=10000)       set max number of remote file sizes in
                                        cache
  --files_cache_size arg (=300)         set max number of remote files in cache
  --mp3_ext arg (=.mp3)                 set mp3 files extension
  --num_search_files arg (=10)          set initial number of files in the
                                        search directory
  --cache_dir arg (=/home/vodka2/.cache/VkMusicFs/)
                                        set cache dir
  --create_dummy_dirs arg (=0)          create dummy dirs
  --num_size_retries arg (=3)           set max number of HEAD requests when
                                        retrieving size
  --err_log arg (=/home/vodka2/.config/VkMusicFs/ErrorLog.txt)
                                        set error log file name
  --log_err_to_file arg (=0)            log errors to file
  --http_timeout arg (=12000)           set HTTP requests timeout in
                                        milliseconds
  --help                                produce help message
  --clear_cache                         clear remote files and remote file
                                        sizes cache
Token options:
  --get_token arg       Obtain token by login and password

```
