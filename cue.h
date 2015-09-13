#ifndef CUE_H
#define CUE_H

#include <QString>
#include <QList>
#include "tagset.h"

class QFile;

class CueIndex
{
public:
    CueIndex(const QString &str = "");

    bool isNull() const { return mNull; }
    QString toString(bool cdQuality = true) const;

    CueIndex operator-(const CueIndex &other) const;
    bool operator==(const CueIndex &other) const;
    bool operator!=(const CueIndex &other) const;

private:
    bool mNull;
    int mCdValue;
    int mHiValue;

    bool parse(const QString &str);
};


class CueReader
{
public:
    explicit CueReader(const QString &fileName);

    void load();
    QString fileName() const { return mFileName; }
    int diskCount() const { return mTagSetList.count(); }
    CueIndex cueIndex(int diskNum, int trackNum, int indexNum) const;
    TagSet tags(int diskNum) const { return mTagSetList.at(diskNum); }

private:
    QString mFileName;
    QList<TagSet> mTagSetList;
    QHash<quint64, CueIndex> mIndexes;

    QByteArray mPerformer;
    QByteArray mAlbum;
    QByteArray mGenre;
    QByteArray mDate;
    QByteArray mComment;
    QByteArray mSongwriter;
    QByteArray mDiskId;
    QByteArray mCatalog;
    QByteArray mCdTextFile;

    void parse(QFile &file);
    TagSet parseOneDiskTags(QFile &file, QByteArray fileTag);
};

#endif // CUE_H
