# VK Music FS

[![Build Status](https://travis-ci.com/vodka2/vk-music-fs.svg?branch=master)](https://travis-ci.com/vodka2/vk-music-fs)

![скриншот VK Music FS](https://i.imgur.com/OoiJfaW.gif)

Виртуальная файловая система для аудиозаписей ВК. Для её использования необходим аккаунт ВКонтакте.

1. [Описание](#описание)
2. [Установка](#установка)
3. [Первый запуск](#первый_запуск)
4. [Основное](#основное)
5. [Команды](#команды)
6. [Опции командной строки](#опции_командной_строки)

<a name="описание"></a>

## Описание

Программа позволяет слушать музыку из ВК в любом плеере и загружать песни на свой компьютер. Поддерживаются сохранённые аудиозаписи и плейлисты, поиск по названию песни и исполнителю. Автоматически, используя название песни и исполнителя, добавляются отсутствующие ID3 теги.

<a name="установка"></a>

## Установка

### Linux

Загрузите AppImage из [релизов](https://github.com/vodka2/vk-music-fs/releases), `vk_music_fs-i386.AppImage` или `vk_music_fs-x86_64.AppImage`. AppImage — исполняемый файл, в котором уже есть все зависимости. Их можно запускать, как «обычные» исполняемые файлы: `./vk_music_fs-x86_64.AppImage ...` Более подробная информация в [документации к AppImage](https://docs.appimage.org/user-guide/run-appimages.html#mount-or-extract-appimages).

Установка из исходных кодов:

1. Установите зависимости: `cmake g++ libssl-dev libfuse-dev libboost-system1.67-dev libboost-thread1.67-dev libboost-filesystem1.67-dev libboost-program-options1.67-dev zlib1g-dev` GCC должен поддерживать C++17
2. Выполните команду в каталоге с исходниками: `mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --target vk_music_fs`
3. Скопируйте исполняемый файл `vk_music_fs`

### Windows

Поддерживается система Windows 7 и более поздние.

1. Загрузите и установите [драйвер WinFsp](https://github.com/billziss-gh/winfsp/releases).
2. Загрузите `vk_music_fs.exe` из [релизов](https://github.com/vodka2/vk-music-fs/releases)
3. Скопируйте DLL WinFsp из `C:\Program Files\WinFsp\bin` в каталог, в который вы поместили `vk_music_fs.exe`

### OS X

Установка из исходных кодов:

1. Установите зависимости: `cmake gcc openssl osxfuse boost`
2. Выполните команду в каталоге с исходниками:
```
mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release DCMAKE_CXX_COMPILER=/usr/local/bin/g++-8 -DCMAKE_C_COMPILER=/usr/local/bin/gcc-8 && cmake --build . --target vk_music_fs
```

## Как использовать

<a name="первый_запуск"></a>

### Первый запуск

#### Linux

Запустите программу в терминале `vk_music_fs --get_token vk_login vk_password`. (`vk_login` и `vk_password` — логин и пароль VK). Программа выведет токен и user-agent, эти две строки нужно скопировать либо в файл `VkMusicFs.ini` в том же каталоге, либо в `~/.config/VkMusicFs/VkMusicFs.ini`. Создайте директорию для монтирования: `mkdir ~/VkMusicFs`. Запустите программу снова `vk_music_fs ~/VkMusicFs`.

#### Windows

Запустите программу `vk_music_fs --get_token vk_login vk_password`. (`vk_login` и `vk_password` — логин и пароль VK). Программа выведет токен и user-agent, эти две строки нужно скопировать либо в файл `VkMusicFs.ini` в том же каталоге, либо в `C:\Users\<Username>\AppData\Roaming\VkMusicFs\VkMusicFs.ini`. Запустите программу снова `vk_music_fs` и система будет примонтирована, по умолчанию в `Z:`. Поддерживается монтирование только в диск, не в директорию. Может потребоваться добавить опции `vk_music_fs -o uid=-1,gid=-1` если примонтировать не удастся.

#### OS X

Запустите программу `vk_music_fs --get_token vk_login vk_password`. (`vk_login` и `vk_password` — логин и пароль VK). Программа выведет токен и user-agent, эти две строки нужно скопировать либо в файл `VkMusicFs.ini` в том же каталоге, либо в  `~/Library/Application Support/VkMusicFs/VkMusicFs.ini`. Создайте точку монтирования: `mkdir ~/VkMusicFs`. Запустите программу снова: `vk_music_fs -o modules=iconv,from_code=UTF-8,to_code=UTF-8-MAC ~/VkMusicFs` и файловая система будет примонтирована.

<a name="основное"></a>

### Основное

#### Dummy dirs

VK Music FS работает в файловом менеджере, взаимодействие с ним, в основном, происходит через создание каталогов. Например, чтобы создать искать по названию песни «Justin Bieber - Baby», нужно создать каталог `Justin Bieber - Baby` в `Search`.

Однако создание нового каталога в различных файловых менеджерах может отличаться. В SpaceFM открывается отдельное окно для ввода имени директории и сразу создаётся «правильный» каталог. А, например, в стандартном проводнике Windows сначала создаётся каталог `Новая папка`, а потом уже он переименовывается в то, что нужно. Чтобы это исправить и не делать лишние запросы к VK, нужно включить опцию `--create_dummy_dirs 1`. Тогда поиск будет идти только при переименовании. Если же используется, например, Total Commander или терминал, то эту опцию наоборот следует отключить: `--create_dummy_dirs 0`.

#### Предварительное чтение файла

Файловые менеджеры любят открывать файлы, чтобы, к примеру, сделать изображение для предпросмотра. Эту опцию лучше отключить для каталога с Vk Music FS, так как содержимое незакэшированных файлов получается по сети, а не с диска.

В проводнике Windows нужно установить опцию Свойства диска -> Настройка -> Оптимизировать эту папку -> Документы и отметить чекбокс «применять этот же шаблон ко всем подпапкам». В Finder OS X нужно для каталога с файловой системой нажать Command+J, убрать чекбокс «Показывать вид значков» и нажать «Использовать как стандартные».

<a name="команды"></a>

### Команды

*После выполнения команд может потребоваться обновление каталога — нужно нажать F5.*

1. Создайте каталог с именем, например, 2 чтобы родительский каталог содержал два последних элемента (аудиозаписи).

   <img src="https://i.imgur.com/aRoP14i.png" width="400"/>

   После этого можно создать каталог 10, тогда будет загружено уже 10 аудиозаписей и так далее.

   Команда работает в `My audios`, `My playlists`, в поддиректориях `Search` и `Search by artist`.

   У «верхних» аудиозаписей в интерфейсе ВК меньшая дата изменения, то есть, можно упорядочить аудиозаписи по дате изменения по возрастанию, чтобы сверху были последние аудиозаписи.

2. Создайте каталог с именем, например, 1-2, чтобы загрузить две аудиозаписи, начиная со второй (первая цифра — номер смещения, вторая — число аудиозаписей).

   <img src="https://i.imgur.com/wKghdc2.png" width="400"/>

   Команда работает в `My audios`, `My playlists`, в поддиректориях `Search` и `Search by artist`.

3. Создайте каталог с именем `Justin Bieber` и в нём будут результаты поиска по этому тексту.

    <img src="https://i.imgur.com/fER2Hhj.png" width="450"/>

   Если в каталоге создать подкаталог `Baby`, то запрос будет уточнён — `Justin Bieber Baby`

   <img src="https://i.imgur.com/SAp2WDB.png" width="450"/>

   Команда работает в `Search` и `Search by artist`, в поддиректориях `Search` и `Search by artist`.

4. Переименуйте файл, добавив к нему `_a`, тогда аудиозапись будет добавлена в `My audios`.

    <img src="https://i.imgur.com/RM5CyvM.png" width="450"/>

   Команда работает в поддиректориях `Search` и `Search by artist`.

6. Создайте каталог `r` или `r<ЦИФРA>` или `refresh<ЦИФРА>` (`r12`, `refresh678`), чтобы обновить элементы (аудиозаписи) в каталоге.

   <img src="https://i.imgur.com/qinednf.png" width="450"/>

   Команда работает в `My audios` и `My playlists`.

5. Удалите файл, чтобы удалить его из `My audios`. Команда работает только в этом каталоге.

6. Удалите отдельную аудиозапись или каталог в `Search` и `Search by artist`, тогда они просто не будут показываться.

<a name="опции_командной_строки"></a>

### Опции командной строки

Значения по умолчанию могут быть другими и дополнительные опции FUSE не показываются. Все опции можно указать и в конфигурационном файле.

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
