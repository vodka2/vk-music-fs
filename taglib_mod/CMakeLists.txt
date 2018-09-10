cmake_minimum_required(VERSION 3.7)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIR})
elseif(HAVE_ZLIB_SOURCE)
  include_directories(${ZLIB_SOURCE})
endif()

set(tag_HDRS
  tag.h
  fileref.h
  audioproperties.h
  taglib_export.h
  taglib_config.h
  toolkit/taglib.h
  toolkit/tstring.h
  toolkit/tlist.h
  toolkit/tlist.tcc
  toolkit/tstringlist.h
  toolkit/tbytevector.h
  toolkit/tbytevectorlist.h
  toolkit/tbytevectorstream.h
  toolkit/tiostream.h
  toolkit/tfile.h
  toolkit/tfilestream.h
  toolkit/tmap.h
  toolkit/tmap.tcc
  toolkit/tpropertymap.h
  toolkit/trefcounter.h
  toolkit/tdebuglistener.h
  mpeg/mpegfile.h
  mpeg/mpegproperties.h
  mpeg/mpegheader.h
  mpeg/xingheader.h
  mpeg/id3v1/id3v1genres.h
  mpeg/id3v2/id3v2extendedheader.h
  mpeg/id3v2/id3v2frame.h
  mpeg/id3v2/id3v2header.h
  mpeg/id3v2/id3v2synchdata.h
  mpeg/id3v2/id3v2footer.h
  mpeg/id3v2/id3v2framefactory.h
  mpeg/id3v2/id3v2tag.h
  mpeg/id3v2/frames/attachedpictureframe.h
  mpeg/id3v2/frames/commentsframe.h
  mpeg/id3v2/frames/eventtimingcodesframe.h
  mpeg/id3v2/frames/generalencapsulatedobjectframe.h
  mpeg/id3v2/frames/ownershipframe.h
  mpeg/id3v2/frames/popularimeterframe.h
  mpeg/id3v2/frames/privateframe.h
  mpeg/id3v2/frames/relativevolumeframe.h
  mpeg/id3v2/frames/synchronizedlyricsframe.h
  mpeg/id3v2/frames/textidentificationframe.h
  mpeg/id3v2/frames/uniquefileidentifierframe.h
  mpeg/id3v2/frames/unknownframe.h
  mpeg/id3v2/frames/unsynchronizedlyricsframe.h
  mpeg/id3v2/frames/urllinkframe.h
  mpeg/id3v2/frames/chapterframe.h
  mpeg/id3v2/frames/tableofcontentsframe.h
  mpeg/id3v2/frames/podcastframe.h
)

set(mpeg_SRCS
  mpeg/mpegfile.cpp
  mpeg/mpegproperties.cpp
  mpeg/mpegheader.cpp
  mpeg/xingheader.cpp
)

set(id3v1_SRCS
  mpeg/id3v1/id3v1genres.cpp
)

set(id3v2_SRCS
  mpeg/id3v2/id3v2framefactory.cpp
  mpeg/id3v2/id3v2synchdata.cpp
  mpeg/id3v2/id3v2tag.cpp
  mpeg/id3v2/id3v2header.cpp
  mpeg/id3v2/id3v2frame.cpp
  mpeg/id3v2/id3v2footer.cpp
  mpeg/id3v2/id3v2extendedheader.cpp
  )

set(frames_SRCS
  mpeg/id3v2/frames/attachedpictureframe.cpp
  mpeg/id3v2/frames/commentsframe.cpp
  mpeg/id3v2/frames/eventtimingcodesframe.cpp
  mpeg/id3v2/frames/generalencapsulatedobjectframe.cpp
  mpeg/id3v2/frames/ownershipframe.cpp
  mpeg/id3v2/frames/popularimeterframe.cpp
  mpeg/id3v2/frames/privateframe.cpp
  mpeg/id3v2/frames/relativevolumeframe.cpp
  mpeg/id3v2/frames/synchronizedlyricsframe.cpp
  mpeg/id3v2/frames/textidentificationframe.cpp
  mpeg/id3v2/frames/uniquefileidentifierframe.cpp
  mpeg/id3v2/frames/unknownframe.cpp
  mpeg/id3v2/frames/unsynchronizedlyricsframe.cpp
  mpeg/id3v2/frames/urllinkframe.cpp
  mpeg/id3v2/frames/chapterframe.cpp
  mpeg/id3v2/frames/tableofcontentsframe.cpp
  mpeg/id3v2/frames/podcastframe.cpp
)

set(toolkit_SRCS
  toolkit/tstring.cpp
  toolkit/tstringlist.cpp
  toolkit/tbytevector.cpp
  toolkit/tbytevectorlist.cpp
  toolkit/tbytevectorstream.cpp
  toolkit/tiostream.cpp
  toolkit/tfile.cpp
  toolkit/tfilestream.cpp
  toolkit/tdebug.cpp
  toolkit/tpropertymap.cpp
  toolkit/trefcounter.cpp
  toolkit/tdebuglistener.cpp
  toolkit/tzlib.cpp
)

if(HAVE_ZLIB_SOURCE)
  set(zlib_SRCS
    ${ZLIB_SOURCE}/adler32.c
    ${ZLIB_SOURCE}/crc32.c
    ${ZLIB_SOURCE}/inffast.c
    ${ZLIB_SOURCE}/inflate.c
    ${ZLIB_SOURCE}/inftrees.c
    ${ZLIB_SOURCE}/zutil.c
  )
endif()

set(tag_LIB_SRCS
  ${mpeg_SRCS} ${id3v1_SRCS} ${id3v2_SRCS} ${frames_SRCS} ${toolkit_SRCS}
  ${zlib_SRCS}
  tag.cpp
  tagunion.cpp
  fileref.cpp
  audioproperties.cpp
  tagutils.cpp
)

add_library(tag ${tag_LIB_SRCS} ${tag_HDRS})

target_include_directories(
        tag
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/toolkit
        ${CMAKE_CURRENT_SOURCE_DIR}/mpeg
        ${CMAKE_CURRENT_SOURCE_DIR}/mpeg/id3v2
        ${CMAKE_CURRENT_SOURCE_DIR}/mpeg/id3v2/frames
        ${CMAKE_CURRENT_SOURCE_DIR}/mpeg/id3v1
        ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
)

target_compile_definitions(tag PUBLIC HAVE_CONFIG_H TAGLIB_STATIC)

target_link_libraries(tag ${ZLIB_LIBRARIES})
