#include "flacmetadata.h"
#include <taglib/flacfile.h>
#include <taglib/xiphcomment.h>
#include <QLoggingCategory>

namespace {
Q_LOGGING_CATEGORY(LOG, "FlacEncoder")
}

void FlacMetadata::writeTags() const
{
    TagLib::FLAC::File file(filePath().toLocal8Bit(), false);

    if (!file.isValid()) {
        qCWarning(LOG) << Q_FUNC_INFO << "file is invalid";
        throw FlaconError("Can't open file");
    }

    // The comments is still owned by the FLAC::File and should not be deleted by the user.
    // It will be deleted when the file (object) is destroyed.
    TagLib::Ogg::XiphComment *comments = file.xiphComment(true);
    this->writeXiphComments(comments);

    if (!coverImage().isEmpty()) {
        const CoverImage  &img = coverImage();
        TagLib::ByteVector dt(img.data().data(), img.data().size());

        TagLib::FLAC::Picture *pic = new TagLib::FLAC::Picture();
        pic->setType(TagLib::FLAC::Picture::Type::FrontCover);
        pic->setData(dt);
        pic->setMimeType(img.mimeType().toStdString());
        pic->setWidth(img.size().width());
        pic->setHeight(img.size().height());
        pic->setColorDepth(img.depth());

        file.addPicture(pic);
    }

    if (!file.save()) {
        throw FlaconError("Can't save file");
    }
}
