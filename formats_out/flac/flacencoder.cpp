#include "flacencoder.h"

QStringList FlacEncoder::programArgs() const
{
    QStringList args;
    args << programPath();

    args << "--force";  // Force overwriting of output files.
    args << "--silent"; // Suppress progress indicator

    // Settings .................................................
    // Compression parametr really looks like --compression-level-N
    args << QString("--compression-level-%1").arg(profile().value("Compression").toString());

    // Tags .....................................................
    if (!track().artist().isEmpty())
        args << "--tag" << QString("artist=%1").arg(track().artist());

    if (!track().album().isEmpty())
        args << "--tag" << QString("album=%1").arg(track().album());

    if (!track().genre().isEmpty())
        args << "--tag" << QString("genre=%1").arg(track().genre());

    if (!track().date().isEmpty())
        args << "--tag" << QString("date=%1").arg(track().date());

    if (!track().title().isEmpty())
        args << "--tag" << QString("title=%1").arg(track().title());

    if (!track().tag(TagId::AlbumArtist).isEmpty())
        args << "--tag" << QString("albumartist=%1").arg(track().tag(TagId::AlbumArtist));

    if (!track().comment().isEmpty())
        args << "--tag" << QString("comment=%1").arg(track().comment());

    if (!track().discId().isEmpty())
        args << "--tag" << QString("discId=%1").arg(track().discId());

    args << "--tag" << QString("tracknumber=%1").arg(track().trackNum());
    args << "--tag" << QString("totaltracks=%1").arg(track().trackCount());
    args << "--tag" << QString("tracktotal=%1").arg(track().trackCount());

    args << "--tag" << QString("disc=%1").arg(track().discNum());
    args << "--tag" << QString("discnumber=%1").arg(track().discNum());
    args << "--tag" << QString("disctotal=%1").arg(track().discCount());

    if (!coverFile().isEmpty()) {
        args << QString("--picture=%1").arg(coverFile());
    }

    if (profile().isEmbedCue()) {
        args << "--tag" << QString("cuesheet=%1").arg(embeddedCue());
    }

    args << "-";
    args << "-o" << outFile();
    return args;
}
