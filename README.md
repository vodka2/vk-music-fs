# VK Music FS

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

Currently only compilation from sources is available

1. Install the following packages: `cmake g++ libssl-dev libfuse-dev libboost-system1.67-dev libboost-thread1.67-dev libboost-filesystem1.67-dev libboost-program-options1.67-dev zlib1g-dev` GCC compiler must support C++17
2. Execute in the sources directory: `mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --target vk_music_fs`
3. Copy `vk_music_fs` to the preferred location

## Usage

### General

First run `vk_music_fs --get_token vk_login vk_password`. The program will print the token and the user agent, copy these two lines to the `VkMusicFs.ini` file either in the same directory or in the configuration folder. (`~/.config/VkMusicFs/VkMusicFs.ini` or `C:\Users\<Username>\AppData\Roaming\VkMusicFs\VkMusicFs.ini`). If you are using Linux create a mount point: `mkdir ~/VkMusicFs`. Then launch the program again: `vk_music_fs ~/VkMusicFs` or just `vk_music_fs` on Windows and the file system will be mounted. On Windows the default mount point is `Z:`.

It is also possible to specify `--token` and `--user_agent` options when launching the program instead of using a configuration file.

On Windows it may be necessary to add `-o uid=-1,gid=-1` option.

Please disable thumbnails for Mp3 files in the file system directory. **On Windows select "Optimize this folder for: Documents" in folder properties, or it may be very slow.**

On Windows only mounting as a disk is supported.

### Commands

In `My audios` and `Search` directories:

1. Create a directory with numeric name, and the parent directory will contain the specified number of Mp3 files
2. Create a directory with name like `10-30` to load 30 files starting from offset 10

In `Search` directory:

1. Create a directory with name, say, `Justin Bieber` to search for the songs by this artist
2. Create a directory with name `Baby` inside the `Justin Bieber` directory and the search query will be `Justin Bieber Baby`.

It is possible to delete directories and files.
