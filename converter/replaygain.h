/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)BSD
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright (c) 1998 - 2020 David Bryant.
 * Copyright: 2022 Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This code is initially based on the wavpack project https://github.com/dbry/WavPack
 * This code distributed under the BSD Software License.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Conifer Software nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef REPLAYGAIN_H
#define REPLAYGAIN_H

#include <array>
#include <QMetaType>

namespace ReplayGain {

class Result
{
    friend class TrackGain;
    friend class AlbumGain;

public:
    Result();
    Result(const Result &other);

    float gain() const;
    float peak() const { return mPeak; }

    using Histogram = std::array<uint32_t, 12000>;
    const Histogram &histogram() const { return mHistogram; }

    bool isNull() const { return mPeak == 0.0; }

protected:
    Result(const Histogram &histogram, float peak) :
        mHistogram(histogram),
        mPeak(peak)
    {
    }

    Histogram mHistogram = { 0 };
    float     mPeak      = 0.0;
};

class TrackGain
{
    friend class AlbumGain;

public:
    TrackGain();
    virtual ~TrackGain();

    /// Adds the first length chars of data to the replaygain.
    void add(const char *data, size_t size);

    Result result() const { return mResult; }

private:
    class Engine;
    Engine *mEngine = nullptr;
    Result  mResult;
};

class AlbumGain
{
public:
    void   add(const Result &trackGain);
    Result result() const { return mResult; }

private:
    Result mResult;
};

} // namespace

Q_DECLARE_METATYPE(ReplayGain::Result);

#endif // REPLAYGAIN_H
