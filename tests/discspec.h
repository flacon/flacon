/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2021
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef DISCSPEC_H
#define DISCSPEC_H
#include <QSettings>

class Disc;

namespace Tests {

class DiscSpec
{
    static constexpr const char *KEY_TRACK_GROUP = "TRACKS/%1";

    static constexpr const char *KEY_CUE_FILE_PATH = "CUE FILE";
    static constexpr const char *KEY_COVER_FILE    = "COVER FILE";

    static constexpr const char *KEY_TRACK_DATE       = "DATE";
    static constexpr const char *KEY_TRACK_DISCID     = "DISCID";
    static constexpr const char *KEY_TRACK_COMMENT    = "COMMENT";
    static constexpr const char *KEY_TRACK_FILE       = "FILE";
    static constexpr const char *KEY_TRACK_PERFORMER  = "PERFORMER";
    static constexpr const char *KEY_TRACK_TITLE      = "TITLE";
    static constexpr const char *KEY_TRACK_INDEX_0    = "INDEX 00";
    static constexpr const char *KEY_TRACK_INDEX_1    = "INDEX 01";
    static constexpr const char *KEY_TRACK_AUDIO_FILE = "AUDIO FILE";

public:
    DiscSpec(const QString &fileName);
    void verify(const Disc &disc) const;

    QString fileName() const { return mData.fileName(); }

    QString cueFilePath() const;
    int     tracksCount() const;

    QString trackTitle(int index) const { return trackValue(index, KEY_TRACK_TITLE); }
    QString trackDate(int index) const { return trackValue(index, KEY_TRACK_DATE); }
    QString trackDiscId(int index) const { return trackValue(index, KEY_TRACK_DISCID); }
    QString trackComment(int index) const { return trackValue(index, KEY_TRACK_COMMENT); }
    QString trackFile(int index) const { return trackValue(index, KEY_TRACK_FILE); }
    QString trackPerformer(int index) const { return trackValue(index, KEY_TRACK_PERFORMER); }
    QString trackIndex0(int index) const { return trackValue(index, KEY_TRACK_INDEX_0); }
    QString trackIndex1(int index) const { return trackValue(index, KEY_TRACK_INDEX_1); }
    QString trackAudioFile(int index) const { return trackValue(index, KEY_TRACK_AUDIO_FILE); }
    QString trackAudioFilePath(int index) const;

    static void write(const Disc &disc, const QString &fileName);

private:
    mutable QSettings mData;
    const QString     mDir;

    QString trackKey(int track, const QString tag) const;
    QString trackValue(int track, const QString &key) const;
};

}

#endif // DISCSPEC_H
