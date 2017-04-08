#include "decoder.h"
#include "../cue.h"
#include <QIODevice>
#include <QByteArray>
#include <QFile>

#define BUF_SIZE            4096

int timeToBytes(CueTime time, WavHeader wav)
{
    if (wav.isCdQuality())
    {
        return (int)((((double)time.frames() * (double)wav.byteRate()) / 75.0) + 0.5);
    }
    else
    {
        return (int)((((double)time.milliseconds() * (double)wav.byteRate()) / 1000.0) + 0.5);
    }
}

bool skip(QIODevice *stream, quint64 bytes)
{
    quint64 pos = stream->pos();
    stream->read(bytes);
    return (stream->pos() - pos == bytes);
}


Decoder::Decoder(QObject *parent) :
    QObject(parent),
    mInputDevice(NULL)
{

}

bool Decoder::open(QIODevice *inputDevice)
{
    mErrorString = "";

    mInputDevice = inputDevice;

    if (!mInputDevice->isOpen())
    {
        if (mInputDevice->open(QIODevice::ReadOnly))
        {
            mErrorString = mInputDevice->errorString();
            return false;
        }
    }

    try
    {
        mWavHeader.load(mInputDevice);
    }
    catch (char const *err)
    {
        mErrorString = err;
        return false;
    }

    return true;
}

bool Decoder::open(const QString fileName)
{
    mInputDevice = new QFile(fileName, this);
    if (!mInputDevice->open(QFile::ReadOnly))
    {
        mErrorString = mInputDevice->errorString();
        return false;
    }

    return open(mInputDevice);
}

void Decoder::close()
{
    if (mInputDevice)
        mInputDevice->close();
}

bool Decoder::extract(const CueTime &start, const CueTime &end, QIODevice *outDevice)
{
    try
    {
        mErrorString = "";
        quint32 bs = timeToBytes(start, mWavHeader);
        quint32 be = timeToBytes(end,   mWavHeader);

        outDevice->write(StdWavHeader(be - bs, mWavHeader).toByteArray());

        skip(mInputDevice, mWavHeader.dataStartPos() + bs - mInputDevice->pos());


        QByteArray buf;
        int remain = be - bs;
        if (remain < 0)
        {
            mErrorString = "[Decoder] Incorrect start or end time.";
            return false;
        }

        while (remain > 0)
        {
            int n = qMin(BUF_SIZE, remain);
            buf = mInputDevice->read(n);
            if (buf.length() != n)
            {
                mErrorString = "[Decoder] Unexpected end of input file.";
                return false;
            }

            remain -= n;
            if (outDevice->write(buf) != n)
            {
                mErrorString = "[Decoder] " + outDevice->errorString();
                return false;
            }


        }
        return true;

    }
    catch (char const *err)
    {
        mErrorString = "[Decoder] " + QString(err);
        return false;
    }
}

bool Decoder::extract(const CueTime &start, const CueTime &end, const QString &fileName)
{
    QFile file(fileName);
    if (! file.open(QFile::WriteOnly | QFile::Truncate))
    {
        mErrorString = file.errorString();
        return false;
    }

    bool res = extract(start, end, &file);
    file.close();

    return res;
}
