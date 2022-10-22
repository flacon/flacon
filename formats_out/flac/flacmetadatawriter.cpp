#include "flacmetadatawriter.h"
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "FlacMetadataWriter")
}

/************************************************

 ************************************************/
FlacMetadataWriter::FlacMetadataWriter(const QString &filePath) :
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
void FlacMetadataWriter::save()
{
    if (!mFile.save()) {
        throw FlaconError("Can't save file");
    }
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setTags(const Track &track)
{
    TagLib::Ogg::XiphComment *tags = mFile.xiphComment(true);
    setXiphTags(tags, track);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setEmbeddedCue(const QString &cue)
{
    TagLib::Ogg::XiphComment *tags = mFile.xiphComment(true);
    setXiphEmbeddedCue(tags, cue);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setCoverImage(const CoverImage &image)
{
    if (!image.isEmpty()) {
        TagLib::ByteVector dt(image.data().data(), image.data().size());

        TagLib::FLAC::Picture *pic = new TagLib::FLAC::Picture();
        pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
        pic->setData(dt);
        pic->setMimeType(image.mimeType().toStdString());
        pic->setWidth(image.size().width());
        pic->setHeight(image.size().height());
        pic->setColorDepth(image.depth());

        mFile.addPicture(pic);
    }
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setTrackReplayGain(float gain, float peak)
{
    TagLib::Ogg::XiphComment *tags = mFile.xiphComment(true);
    setXiphTrackReplayGain(tags, gain, peak);
}

/************************************************
  The comments is still owned by the TagLib::File and should not be deleted by the user.
  It will be deleted when the file (object) is destroyed.
 ************************************************/
void FlacMetadataWriter::setAlbumReplayGain(float gain, float peak)
{
    TagLib::Ogg::XiphComment *tags = mFile.xiphComment(true);
    setXiphAlbumReplayGain(tags, gain, peak);
}
