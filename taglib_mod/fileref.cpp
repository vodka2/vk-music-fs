/***************************************************************************
    copyright            : (C) 2002 - 2008 by Scott Wheeler
    email                : wheeler@kde.org

    copyright            : (C) 2010 by Alex Novichkov
    email                : novichko@atnet.ru
                           (added APE file support)
 ***************************************************************************/

/***************************************************************************
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License version   *
 *   2.1 as published by the Free Software Foundation.                     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA         *
 *   02110-1301  USA                                                       *
 *                                                                         *
 *   Alternatively, this file is available under the Mozilla Public        *
 *   License Version 1.1.  You may obtain a copy of the License at         *
 *   http://www.mozilla.org/MPL/                                           *
 ***************************************************************************/

#include <tfile.h>
#include <tfilestream.h>
#include <tstring.h>
#include <tdebug.h>
#include <trefcounter.h>
#include <id3v2framefactory.h>

#include "fileref.h"
#include "mpegfile.h"

using namespace TagLib;

namespace
{
  typedef List<const FileRef::FileTypeResolver *> ResolverList;
  ResolverList fileTypeResolvers;

  // Detect the file type by user-defined resolvers.

  File *detectByResolvers(FileName fileName, bool readAudioProperties,
                          AudioProperties::ReadStyle audioPropertiesStyle)
  {
    ResolverList::ConstIterator it = fileTypeResolvers.begin();
    for(; it != fileTypeResolvers.end(); ++it) {
      File *file = (*it)->createFile(fileName, readAudioProperties, audioPropertiesStyle);
      if(file)
        return file;
    }

    return 0;
  }

  // Detect the file type based on the file extension.

  File* detectByExtension(IOStream *stream, bool readAudioProperties,
                          AudioProperties::ReadStyle audioPropertiesStyle)
  {
#ifdef _WIN32
    const String s = stream->name().toString();
#else
    const String s(stream->name());
#endif

    String ext;
    const int pos = s.rfind(".");
    if(pos != -1)
      ext = s.substr(pos + 1).upper();

    // If this list is updated, the method defaultFileExtensions() should also be
    // updated.  However at some point that list should be created at the same time
    // that a default file type resolver is created.

    if(ext.isEmpty())
      return 0;

    // .oga can be any audio in the Ogg container. So leave it to content-based detection.

    if(ext == "MP3")
      return new MPEG::File(stream, ID3v2::FrameFactory::instance(), readAudioProperties, audioPropertiesStyle);

    return 0;
  }

  // Detect the file type based on the actual content of the stream.

  File *detectByContent(IOStream *stream, bool readAudioProperties,
                        AudioProperties::ReadStyle audioPropertiesStyle)
  {
    File *file = 0;

    if(MPEG::File::isSupported(stream))
      file = new MPEG::File(stream, ID3v2::FrameFactory::instance(), readAudioProperties, audioPropertiesStyle);

    // isSupported() only does a quick check, so double check the file here.

    if(file) {
      if(file->isValid())
        return file;
      else
        delete file;
    }

    return 0;
  }

  // Internal function that supports FileRef::create().
  // This looks redundant, but necessary in order not to change the previous
  // behavior of FileRef::create().

  File* createInternal(FileName fileName, bool readAudioProperties,
                       AudioProperties::ReadStyle audioPropertiesStyle)
  {
    File *file = detectByResolvers(fileName, readAudioProperties, audioPropertiesStyle);
    if(file)
      return file;

#ifdef _WIN32
    const String s = fileName.toString();
#else
    const String s(fileName);
#endif

    String ext;
    const int pos = s.rfind(".");
    if(pos != -1)
      ext = s.substr(pos + 1).upper();

    if(ext.isEmpty())
      return 0;

    if(ext == "MP3")
      return new MPEG::File(fileName, ID3v2::FrameFactory::instance(), readAudioProperties, audioPropertiesStyle);

    return 0;
  }
}

class FileRef::FileRefPrivate : public RefCounter
{
public:
  FileRefPrivate() :
    RefCounter(),
    file(0),
    stream(0) {}

  ~FileRefPrivate() {
    delete file;
    delete stream;
  }

  File     *file;
  IOStream *stream;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

FileRef::FileRef() :
  d(new FileRefPrivate())
{
}

FileRef::FileRef(FileName fileName, bool readAudioProperties,
                 AudioProperties::ReadStyle audioPropertiesStyle) :
  d(new FileRefPrivate())
{
  parse(fileName, readAudioProperties, audioPropertiesStyle);
}

FileRef::FileRef(IOStream* stream, bool readAudioProperties, AudioProperties::ReadStyle audioPropertiesStyle) :
  d(new FileRefPrivate())
{
  parse(stream, readAudioProperties, audioPropertiesStyle);
}

FileRef::FileRef(File *file) :
  d(new FileRefPrivate())
{
  d->file = file;
}

FileRef::FileRef(const FileRef &ref) :
  d(ref.d)
{
  d->ref();
}

FileRef::~FileRef()
{
  if(d->deref())
    delete d;
}

Tag *FileRef::tag() const
{
  if(isNull()) {
    debug("FileRef::tag() - Called without a valid file.");
    return 0;
  }
  return d->file->tag();
}

AudioProperties *FileRef::audioProperties() const
{
  if(isNull()) {
    debug("FileRef::audioProperties() - Called without a valid file.");
    return 0;
  }
  return d->file->audioProperties();
}

File *FileRef::file() const
{
  return d->file;
}

bool FileRef::save()
{
  if(isNull()) {
    debug("FileRef::save() - Called without a valid file.");
    return false;
  }
  return d->file->save();
}

const FileRef::FileTypeResolver *FileRef::addFileTypeResolver(const FileRef::FileTypeResolver *resolver) // static
{
  fileTypeResolvers.prepend(resolver);
  return resolver;
}

StringList FileRef::defaultFileExtensions()
{
  StringList l;

  l.append("ogg");
  l.append("flac");
  l.append("oga");
  l.append("mp3");
  l.append("mpc");
  l.append("wv");
  l.append("spx");
  l.append("tta");
  l.append("m4a");
  l.append("m4r");
  l.append("m4b");
  l.append("m4p");
  l.append("3g2");
  l.append("mp4");
  l.append("m4v");
  l.append("wma");
  l.append("asf");
  l.append("aif");
  l.append("aiff");
  l.append("wav");
  l.append("ape");
  l.append("mod");
  l.append("module"); // alias for "mod"
  l.append("nst"); // alias for "mod"
  l.append("wow"); // alias for "mod"
  l.append("s3m");
  l.append("it");
  l.append("xm");

  return l;
}

bool FileRef::isNull() const
{
  return (!d->file || !d->file->isValid());
}

FileRef &FileRef::operator=(const FileRef &ref)
{
  FileRef(ref).swap(*this);
  return *this;
}

void FileRef::swap(FileRef &ref)
{
  using std::swap;

  swap(d, ref.d);
}

bool FileRef::operator==(const FileRef &ref) const
{
  return (ref.d->file == d->file);
}

bool FileRef::operator!=(const FileRef &ref) const
{
  return (ref.d->file != d->file);
}

File *FileRef::create(FileName fileName, bool readAudioProperties,
                      AudioProperties::ReadStyle audioPropertiesStyle) // static
{
  return createInternal(fileName, readAudioProperties, audioPropertiesStyle);
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////

void FileRef::parse(FileName fileName, bool readAudioProperties,
                    AudioProperties::ReadStyle audioPropertiesStyle)
{
  // Try user-defined resolvers.

  d->file = detectByResolvers(fileName, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // Try to resolve file types based on the file extension.

  d->stream = new FileStream(fileName);
  d->file = detectByExtension(d->stream, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // At last, try to resolve file types based on the actual content.

  d->file = detectByContent(d->stream, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // Stream have to be closed here if failed to resolve file types.

  delete d->stream;
  d->stream = 0;
}

void FileRef::parse(IOStream *stream, bool readAudioProperties,
                    AudioProperties::ReadStyle audioPropertiesStyle)
{
  // User-defined resolvers won't work with a stream.

  // Try to resolve file types based on the file extension.

  d->file = detectByExtension(stream, readAudioProperties, audioPropertiesStyle);
  if(d->file)
    return;

  // At last, try to resolve file types based on the actual content of the file.

  d->file = detectByContent(stream, readAudioProperties, audioPropertiesStyle);
}
