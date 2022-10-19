#include "wvmetadatawriter.h"
#include <taglib/apetag.h>

#include <QLoggingCategory>
namespace {
Q_LOGGING_CATEGORY(LOG, "WvMetadataWriter")
}

/************************************************

 ************************************************/
WvMetadataWriter::WvMetadataWriter(const QString &filePath) :
    MetadataWriter(filePath),
    mFile(filePath.toLocal8Bit(), false)
{
    if (!mFile.isValid()) {
        qCWarning(LOG) << Q_FUNC_INFO << "file is invalid";
        throw FlaconError("Can't open file");
    }
}

/************************************************

 ************************************************/
void WvMetadataWriter::save()
{
    if (!mFile.save()) {
        throw FlaconError("Can't save file");
    }
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void WvMetadataWriter::setTags(const Track &track)
{
    TagLib::APE::Tag *tags = mFile.APETag(true);
    setApeTags(tags, track);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void WvMetadataWriter::setEmbeddedCue(const QString &)
{
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void WvMetadataWriter::setCoverImage(const CoverImage &image)
{
    TagLib::APE::Tag *tags = mFile.APETag(true);
    setApeCoverImage(tags, image);
}
