#include "opusmetadatawriter.h"

#include <QLoggingCategory>
namespace {
Q_LOGGING_CATEGORY(LOG, "OpusMetadataWriter")
}

/************************************************

 ************************************************/
OpusMetadataWriter::OpusMetadataWriter(const QString &filePath) :
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
void OpusMetadataWriter::save()
{
    if (!mFile.save()) {
        throw FlaconError("Can't save file");
    }
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void OpusMetadataWriter::setTags(const Track &track)
{
    TagLib::Ogg::XiphComment *tags = mFile.tag();
    setXiphTags(tags, track);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void OpusMetadataWriter::setEmbeddedCue(const QString &cue)
{
    TagLib::Ogg::XiphComment *tags = mFile.tag();
    setXiphEmbeddedCue(tags, cue);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void OpusMetadataWriter::setCoverImage(const CoverImage &image)
{
    TagLib::Ogg::XiphComment *tags = mFile.tag();
    setXiphCoverImage(tags, image);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void OpusMetadataWriter::setTrackReplayGain(float gain, float peak)
{
    TagLib::Ogg::XiphComment *tags = mFile.tag();
    setXiphTrackReplayGain(tags, gain, peak);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void OpusMetadataWriter::setAlbumReplayGain(float gain, float peak)
{
    TagLib::Ogg::XiphComment *tags = mFile.tag();
    setXiphAlbumReplayGain(tags, gain, peak);
}
