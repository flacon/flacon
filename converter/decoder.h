#ifndef DECODER_H
#define DECODER_H

#include "wav.h"
#include <QObject>
#include <QString>
#include "../cue.h"

class QIODevice;


class Decoder : public QObject
{
    Q_OBJECT
public:
    explicit Decoder(QObject *parent = 0);

    bool open(QIODevice *inputDevice);
    bool open(const QString fileName);
    void close();

    bool extract(const CueTime &start, const CueTime &end, QIODevice *outDevice);
    bool extract(const CueTime &start, const CueTime &end, const QString &fileName);

    QString errorString() const { return mErrorString; }

signals:

public slots:

private:
    QIODevice *mInputDevice;
    WavHeader mWavHeader;
    QString   mErrorString;
};



#endif // DECODER_H
