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

#include "replaygain.h"
#include "types.h"
#include <QDebug>
#include <cmath>
#include <QBuffer>
#include "converter/wavheader.h"

static void registerQtMetaTypes()
{
    static bool done = false;

    if (!done) {
        qRegisterMetaType<ReplayGain::Result>();
        done = true;
    }
}

using namespace ReplayGain;

// These are the filters used to calculate perceived loudness. The table data was copied
// from the Foobar2000 source code.

static constexpr int YULE_ORDER   = 10;
static constexpr int BUTTER_ORDER = 2;
static constexpr int NAX_CHAN_NUM = 2;

#define DEBUG_LEVELS 0
#define DEBUG_FLOATS 0

enum class WavType {
    Int8 = 1,
    Int16,
    Int24,
    Int32,
};

struct RgFreqInfo
{
    uint32_t rate;
    double   BYule[YULE_ORDER + 1];
    double   AYule[YULE_ORDER + 1];
    double   BButter[BUTTER_ORDER + 1];
    double   AButter[BUTTER_ORDER + 1];
};

/************************************************
 *
 * **********************************************/
static constexpr RgFreqInfo FREQ_INFOS[] = {
    {
            48000,
            { 0.03857599435200, -0.02160367184185, -0.00123395316851, -0.00009291677959, -0.01655260341619, 0.02161526843274, -0.02074045215285, 0.00594298065125, 0.00306428023191, 0.00012025322027, 0.00288463683916 },
            { 1., -3.84664617118067, 7.81501653005538, -11.34170355132042, 13.05504219327545, -12.28759895145294, 9.48293806319790, -5.87257861775999, 2.75465861874613, -0.86984376593551, 0.13919314567432 },
            { 0.98621192462708, -1.97242384925416, 0.98621192462708 },
            { 1., -1.97223372919527, 0.97261396931306 },
    },

    {
            44100,
            { 0.05418656406430, -0.02911007808948, -0.00848709379851, -0.00851165645469, -0.00834990904936, 0.02245293253339, -0.02596338512915, 0.01624864962975, -0.00240879051584, 0.00674613682247, -0.00187763777362 },
            { 1., -3.47845948550071, 6.36317777566148, -8.54751527471874, 9.47693607801280, -8.81498681370155, 6.85401540936998, -4.39470996079559, 2.19611684890774, -0.75104302451432, 0.13149317958808 },
            { 0.98500175787242, -1.97000351574484, 0.98500175787242 },
            { 1., -1.96977855582618, 0.97022847566350 },
    },

    {
            32000,
            { 0.15457299681924, -0.09331049056315, -0.06247880153653, 0.02163541888798, -0.05588393329856, 0.04781476674921, 0.00222312597743, 0.03174092540049, -0.01390589421898, 0.00651420667831, -0.00881362733839 },
            { 1., -2.37898834973084, 2.84868151156327, -2.64577170229825, 2.23697657451713, -1.67148153367602, 1.00595954808547, -0.45953458054983, 0.16378164858596, -0.05032077717131, 0.02347897407020 },
            { 0.97938932735214, -1.95877865470428, 0.97938932735214 },
            { 1., -1.95835380975398, 0.95920349965459 },
    },

    {
            24000,
            { 0.30296907319327, -0.22613988682123, -0.08587323730772, 0.03282930172664, -0.00915702933434, -0.02364141202522, -0.00584456039913, 0.06276101321749, -0.00000828086748, 0.00205861885564, -0.02950134983287 },
            { 1., -1.61273165137247, 1.07977492259970, -0.25656257754070, -0.16276719120440, -0.22638893773906, 0.39120800788284, -0.22138138954925, 0.04500235387352, 0.02005851806501, 0.00302439095741 },
            { 0.97531843204928, -1.95063686409857, 0.97531843204928 },
            { 1., -1.95002759149878, 0.95124613669835 },
    },

    {
            22050,
            { 0.33642304856132, -0.25572241425570, -0.11828570177555, 0.11921148675203, -0.07834489609479, -0.00469977914380, -0.00589500224440, 0.05724228140351, 0.00832043980773, -0.01635381384540, -0.01760176568150 },
            { 1., -1.49858979367799, 0.87350271418188, 0.12205022308084, -0.80774944671438, 0.47854794562326, -0.12453458140019, -0.04067510197014, 0.08333755284107, -0.04237348025746, 0.02977207319925 },
            { 0.97316523498161, -1.94633046996323, 0.97316523498161 },
            { 1., -1.94561023566527, 0.94705070426118 },
    },

    {
            16000,
            { 0.44915256608450, -0.14351757464547, -0.22784394429749, -0.01419140100551, 0.04078262797139, -0.12398163381748, 0.04097565135648, 0.10478503600251, -0.01863887810927, -0.03193428438915, 0.00541907748707 },
            { 1., -0.62820619233671, 0.29661783706366, -0.37256372942400, 0.00213767857124, -0.42029820170918, 0.22199650564824, 0.00613424350682, 0.06747620744683, 0.05784820375801, 0.03222754072173 },
            { 0.96454515552826, -1.92909031105652, 0.96454515552826 },
            { 1., -1.92783286977036, 0.93034775234268 },
    },

    {
            12000,
            { 0.56619470757641, -0.75464456939302, 0.16242137742230, 0.16744243493672, -0.18901604199609, 0.30931782841830, -0.27562961986224, 0.00647310677246, 0.08647503780351, -0.03788984554840, -0.00588215443421 },
            { 1., -1.04800335126349, 0.29156311971249, -0.26806001042947, 0.00819999645858, 0.45054734505008, -0.33032403314006, 0.06739368333110, -0.04784254229033, 0.01639907836189, 0.01807364323573 },
            { 0.96009142950541, -1.92018285901082, 0.96009142950541 },
            { 1., -1.91858953033784, 0.92177618768381 },
    },

    {
            11025,
            { 0.58100494960553, -0.53174909058578, -0.14289799034253, 0.17520704835522, 0.02377945217615, 0.15558449135573, -0.25344790059353, 0.01628462406333, 0.06920467763959, -0.03721611395801, -0.00749618797172 },
            { 1., -0.51035327095184, -0.31863563325245, -0.20256413484477, 0.14728154134330, 0.38952639978999, -0.23313271880868, -0.05246019024463, -0.02505961724053, 0.02442357316099, 0.01818801111503 },
            { 0.95856916599601, -1.91713833199203, 0.95856916599601 },
            { 1., -1.91542108074780, 0.91885558323625 },
    },

    {
            8000,
            { 0.53648789255105, -0.42163034350696, -0.00275953611929, 0.04267842219415, -0.10214864179676, 0.14590772289388, -0.02459864859345, -0.11202315195388, -0.04060034127000, 0.04788665548180, -0.02217936801134 },
            { 1., -0.25049871956020, -0.43193942311114, -0.03424681017675, -0.04678328784242, 0.26408300200955, 0.15113130533216, -0.17556493366449, -0.18823009262115, 0.05477720428674, 0.04704409688120 },
            { 0.94597685600279, -1.89195371200558, 0.94597685600279 },
            { 1., -1.88903307939452, 0.89487434461664 },
    },

    {
            18900,
            { 0.38524531015142, -0.27682212062067, -0.09980181488805, 0.09951486755646, -0.08934020156622, -0.00322369330199, -0.00110329090689, 0.03784509844682, 0.01683906213303, -0.01147039862572, -0.01941767987192 },
            { 1.00000000000000, -1.29708918404534, 0.90399339674203, -0.29613799017877, -0.42326645916207, 0.37934887402200, -0.37919795944938, 0.23410283284785, -0.03892971758879, 0.00403009552351, 0.03640166626278 },
            { 0.96535326815829, -1.93070653631658, 0.96535326815829 },
            { 1.00000000000000, -1.92950577983524, 0.93190729279793 },
    },

    {
            37800,
            { 0.08717879977844, -0.01000374016172, -0.06265852122368, -0.01119328800950, -0.00114279372960, 0.02081333954769, -0.01603261863207, 0.01936763028546, 0.00760044736442, -0.00303979112271, -0.00075088605788 },
            { 1.00000000000000, -2.62816311472146, 3.53734535817992, -3.81003448678921, 3.91291636730132, -3.53518605896288, 2.71356866157873, -1.86723311846592, 1.12075382367659, -0.48574086886890, 0.11330544663849 },
            { 0.98252400815195, -1.96504801630391, 0.98252400815195 },
            { 1.00000000000000, -1.96474258269041, 0.96535344991740 },
    },

    {
            56000,
            { 0.03144914734085, -0.06151729206963, 0.08066788708145, -0.09737939921516, 0.08943210803999, -0.06989984672010, 0.04926972841044, -0.03161257848451, 0.01456837493506, -0.00316015108496, 0.00132807215875 },
            { 1.00000000000000, -4.87377313090032, 12.03922160140209, -20.10151118381395, 25.10388534415171, -24.29065560815903, 18.27158469090663, -10.45249552560593, 4.30319491872003, -1.13716992070185, 0.14510733527035 },
            { 0.98816995007392, -1.97633990014784, 0.98816995007392 },
            { 1.00000000000000, -1.97619994516973, 0.97647985512594 },
    },

    {
            64000,
            { 0.02613056568174, -0.08128786488109, 0.14937282347325, -0.21695711675126, 0.25010286673402, -0.23162283619278, 0.17424041833052, -0.10299599216680, 0.04258696481981, -0.00977952936493, 0.00105325558889 },
            { 1.00000000000000, -5.73625477092119, 16.15249794355035, -29.68654912464508, 39.55706155674083, -39.82524556246253, 30.50605345013009, -17.43051772821245, 7.05154573908017, -1.80783839720514, 0.22127840210813 },
            { 0.98964101933472, -1.97928203866944, 0.98964101933472 },
            { 1.00000000000000, -1.97917472731009, 0.97938935002880 },
    },

    {
            88200,
            { 0.02667482047416, -0.11377479336097, 0.23063167910965, -0.30726477945593, 0.33188520686529, -0.33862680249063, 0.31807161531340, -0.23730796929880, 0.12273894790371, -0.03840017967282, 0.00549673387936 },
            { 1.00000000000000, -6.31836451657302, 18.31351310801799, -31.88210014815921, 36.53792146976740, -28.23393036467559, 14.24725258227189, -4.04670980012854, 0.18865757280515, 0.25420333563908, -0.06012333531065 },
            { 0.99247255046129, -1.98494510092259, 0.99247255046129 },
            { 1.00000000000000, -1.98488843762335, 0.98500176422183 },
    },

    {
            96000,
            { 0.00588138296683, -0.01613559730421, 0.02184798954216, -0.01742490405317, 0.00464635643780, 0.01117772513205, -0.02123865824368, 0.01959354413350, -0.01079720643523, 0.00352183686289, -0.00063124341421 },
            { 1.00000000000000, -5.97808823642008, 16.21362507964068, -25.72923730652599, 25.40470663139513, -14.66166287771134, 2.81597484359752, 2.51447125969733, -2.23575306985286, 0.75788151036791, -0.10078025199029 },
            { 0.99308203517541, -1.98616407035082, 0.99308203517541 },
            { 1.00000000000000, -1.98611621154089, 0.98621192916075 },
    },

    {
            112000,
            { 0.00528778718259, -0.01893240907245, 0.03185982561867, -0.02926260297838, 0.00715743034072, 0.01985743355827, -0.03222614850941, 0.02565681978192, -0.01210662313473, 0.00325436284541, -0.00044173593001 },
            { 1.00000000000000, -6.24932108456288, 17.42344320538476, -27.86819709054896, 26.79087344681326, -13.43711081485123, -0.66023612948173, 6.03658091814935, -4.24926577030310, 1.40829268709186, -0.19480852628112 },
            { 0.99406737810867, -1.98813475621734, 0.99406737810867 },
            { 1.00000000000000, -1.98809955990514, 0.98816995252954 },
    },

    {
            128000,
            { 0.00553120584305, -0.02112620545016, 0.03549076243117, -0.03362498312306, 0.01425867248183, 0.01344686928787, -0.03392770787836, 0.03464136459530, -0.02039116051549, 0.00667420794705, -0.00093763762995 },
            { 1.00000000000000, -6.14581710839925, 16.04785903675838, -22.19089131407749, 15.24756471580286, -0.52001440400238, -8.00488641699940, 6.60916094768855, -2.37856022810923, 0.33106947986101, 0.00459820832036 },
            { 0.99480702681278, -1.98961405362557, 0.99480702681278 },
            { 1.00000000000000, -1.98958708647324, 0.98964102077790 },
    },

    {
            144000,
            { 0.00639682359450, -0.02556437970955, 0.04230854400938, -0.03722462201267, 0.01718514827295, 0.00610592243009, -0.03065965747365, 0.04345745003539, -0.03298592681309, 0.01320937236809, -0.00220304127757 },
            { 1.00000000000000, -6.14814623523425, 15.80002457141566, -20.78487587686937, 11.98848552310315, 3.36462015062606, -10.22419868359470, 6.65599702146473, -1.67141861110485, -0.05417956536718, 0.07374767867406 },
            { 0.99538268958706, -1.99076537917413, 0.99538268958706 },
            { 1.00000000000000, -1.99074405950505, 0.99078669884321 },
    },

    {
            176400,
            { 0.00268568524529, -0.00852379426080, 0.00852704191347, 0.00146116310295, -0.00950855828762, 0.00625449515499, 0.00116183868722, -0.00362461417136, 0.00203961000134, -0.00050664587933, 0.00004327455427 },
            { 1.00000000000000, -5.57512782763045, 12.44291056065794, -12.87462799681221, 3.08554846961576, 6.62493459880692, -7.07662766313248, 2.51175542736441, 0.06731510802735, -0.24567753819213, 0.03961404162376 },
            { 0.99622916581118, -1.99245833162236, 0.99622916581118 },
            { 1.00000000000000, -1.99244411238133, 0.99247255086339 },
    },

    {
            192000,
            { 0.01184742123123, -0.04631092400086, 0.06584226961238, -0.02165588522478, -0.05656260778952, 0.08607493592760, -0.03375544339786, -0.04216579932754, 0.06416711490648, -0.03444708260844, 0.00697275872241 },
            { 1.00000000000000, -5.24727318348167, 10.60821585192244, -8.74127665810413, -1.33906071371683, 8.07972882096606, -5.46179918950847, 0.54318070652536, 0.87450969224280, -0.34656083539754, 0.03034796843589 },
            { 0.99653501465135, -1.99307002930271, 0.99653501465135 },
            { 1.00000000000000, -1.99305802314321, 0.99308203546221 },
    }
};

static constexpr auto FREQ_INFOS_SIZE = sizeof(FREQ_INFOS) / sizeof(FREQ_INFOS[0]);

static constexpr int32_t FILTER[] = {
    50, 464, 968, 711, -1203, -5028, -9818, -13376,
    -12870, -6021, 7526, 25238, 41688, 49778, 43050, 18447,
    -21428, -67553, -105876, -120890, -100640, -41752, 47201, 145510,
    224022, 252377, 208224, 86014, -97312, -301919, -470919, -541796,
    -461126, -199113, 239795, 813326, 1446343, 2043793, 2509064, 2763659,
    2763659, 2509064, 2043793, 1446343, 813326, 239795, -199113, -461126,
    -541796, -470919, -301919, -97312, 86014, 208224, 252377, 224022,
    145510, 47201, -41752, -100640, -120890, -105876, -67553, -21428,
    18447, 43050, 49778, 41688, 25238, 7526, -6021, -12870,
    -13376, -9818, -5028, -1203, 711, 968, 464, 50
};

static constexpr int NUM_TERMS = ((int)(sizeof(FILTER) / sizeof(FILTER[0])));

class Decimator
{
public:
    Decimator(int numChannels, int ratio);

    bool run(int32_t sample, int32_t *res);

private:
    struct ChanState
    {
        int N                = 0;
        int delay[NUM_TERMS] = { 0 };
        int index            = 0;
    };

    ChanState mChanState[NAX_CHAN_NUM];
    int       mChan        = 0;
    const int mNumChannels = 0;
    const int mRatio       = 0;
};

/************************************************
 *
 ************************************************/
Decimator::Decimator(int numChannels, int ratio) :
    mNumChannels(numChannels),
    mRatio(ratio)
{
    for (int i = 0; i < mNumChannels; ++i) {
        mChanState[i].N     = i;
        mChanState[i].index = NUM_TERMS - ratio;
    }
}

/************************************************
 *
 ************************************************/
bool Decimator::run(int32_t sample, int32_t *res)
{
    ChanState &sp = mChanState[mChan];
    ++mChan;
    if (mChan == mNumChannels) {
        mChan = 0;
    }
    sp.delay[sp.index] = sample;
    ++sp.index;

    if (sp.index == NUM_TERMS) {
        int64_t sum = 0;

        for (int i = 0; i < NUM_TERMS; ++i) {
            sum += (int64_t)FILTER[i] * sp.delay[i];
        }

        *res = (int32_t)(sum >> 24);
        memmove(sp.delay, sp.delay + mRatio, sizeof(sp.delay[0]) * (NUM_TERMS - mRatio));
        sp.index = NUM_TERMS - mRatio;
        return true;
    }

    return false;
}

/************************************************
 *
 ************************************************/
class TrackGain::Engine
{
public:
    Engine(Result &result);
    ~Engine();

    WavType type = WavType(0);

    size_t loadHeader(const char *data, size_t size);

    uint addBytes(const char *data, size_t size);
    void add_int8(const char *data, size_t size);
    void add_int16(const char *data, size_t size);
    void add_int24(const char *data, size_t size);
    void add_int32(const char *data, size_t size);
    void addFloatSample(uint32_t sample);

    void   calc(uint32_t count);
    void   calcStereoPeak(uint32_t count);
    void   yuleFilterStereoSamples(float *samples, uint32_t size);
    void   butterFilterStereoSamples(float *samples, uint32_t size);
    double calcStereoRms(float *samples, uint32_t size) const;

public:
    Result &mResult;

    bool       mHeaderReady = false;
    QByteArray mHeaderData;

    int      mNumChannels   = 0;
    uint32_t mSampleRate    = 0;
    uint16_t mBitsPerSample = 0;
    size_t   mRemains       = 0;

    uint     mIntSampleIndex = 0;
    uint32_t mIntSample      = 0;

    Decimator *mDecimator = nullptr;

    float   *mFloatSamples        = nullptr;
    uint32_t mFloatSamplesMaxSize = 0;
    uint32_t mFloatSamplesIndex   = 0;

    double const *mYuleCoeffA   = nullptr;
    double const *mYuleCoeffB   = nullptr;
    double const *mButterCoeffA = nullptr;
    double const *mButterCoeffB = nullptr;

    float mYuleHistA[256]    = { 0 };
    float mYuleHistB[256]    = { 0 };
    float butter_hist_a[256] = { 0 };
    float butter_hist_b[256] = { 0 };

    int mYuleHistI   = 20;
    int mButterHistI = 4;
};

/************************************************
 *
 ************************************************/
TrackGain::Engine::Engine(Result &result) :
    mResult(result)
{
}

/************************************************
 *
 ************************************************/
size_t TrackGain::Engine::loadHeader(const char *data, size_t size)
{
    size_t prev = mHeaderData.size();

    Conv::WavHeader header;

    try {
        // Read ............................
        mHeaderData.append(data, size);

        QBuffer buf(&mHeaderData);
        buf.open(QBuffer::ReadOnly);
        header = Conv::WavHeader(&buf);

        mHeaderData.clear();
        mHeaderReady = true;
    }
    catch (FlaconError &err) {
        return size;
    }

    // Initialize ......................
    mNumChannels   = header.numChannels();
    mSampleRate    = header.sampleRate();
    mBitsPerSample = header.bitsPerSample();
    mRemains       = header.dataSize();

    if (mNumChannels > 2) {
        throw FlaconError("can't handle multichannel files yet!");
    }

    uint32_t sampleRate = mSampleRate;
    if (sampleRate >= 256000) {
        mDecimator = new Decimator(mNumChannels, 4);
        sampleRate /= 4;
    }

    mFloatSamplesMaxSize = sampleRate / 20 * NAX_CHAN_NUM; // 2 - is always stereo
    mFloatSamples        = new float[mFloatSamplesMaxSize];

    // Initialize filters;
    for (uint i = 0; i < FREQ_INFOS_SIZE; ++i) {
        if (FREQ_INFOS[i].rate == sampleRate) {
            mYuleCoeffA   = FREQ_INFOS[i].AYule;
            mYuleCoeffB   = FREQ_INFOS[i].BYule;
            mButterCoeffA = FREQ_INFOS[i].AButter;
            mButterCoeffB = FREQ_INFOS[i].BButter;
            break;
        }
    }

    if (!mYuleCoeffA) {
        throw FlaconError(QString("sample rate of %1 is not supported!").arg(header.sampleRate()));
    }

    // clang-format off
        switch (mBitsPerSample) {
            case 8:  type = WavType::Int8;  break;
            case 16: type = WavType::Int16; break;
            case 24: type = WavType::Int24; break;
            case 32: type = WavType::Int32; break;
        }
    // clang-format on

    return header.dataStartPos() - prev;
}

/************************************************
 *
 ************************************************/
TrackGain::Engine::~Engine()
{
    delete[] mFloatSamples;
}

/************************************************
 *
 ************************************************/
inline void TrackGain::Engine::addFloatSample(uint32_t in)
{
    int32_t sample = 0;
    float   factor = 0;
    switch (type) {
        case WavType::Int8:
            sample = int8_t(in) ^ (0xFFFFFF00 + 0b10000000);
            factor = 1.0 / 128.0;
            break;

        case WavType::Int16:
            sample = int16_t(in);
            factor = 1.0 / 32768.0;
            break;

        case WavType::Int24:
            if (in & (1 << 23)) {
                sample = int32_t(in | 0xFF000000);
            }
            else {
                sample = int32_t(in & 0x00FFFFFF);
            }
            factor = 1.0 / 8388608.0;
            break;

        case WavType::Int32:
            sample = int32_t(in);
            factor = 1.0 / 2147483648.0;
            break;
    }

    if (mDecimator) {
        int32_t n = 0;
        if (!mDecimator->run(sample, &n)) {
            return;
        }
        sample = n;
    }

    mFloatSamples[mFloatSamplesIndex] = sample * factor;
    mFloatSamplesIndex++;

#if DEBUG_FLOATS
    printf("%08x -> %0.16f\n", sample, mFloatSamples[mFloatSamplesIndex - 1]);
#endif

    if (mNumChannels == 1) {
        mFloatSamples[mFloatSamplesIndex] = mFloatSamples[mFloatSamplesIndex - 1];
        mFloatSamplesIndex++;
    }

    if (mFloatSamplesIndex == mFloatSamplesMaxSize) {
        calc(mFloatSamplesMaxSize);
        mFloatSamplesIndex = 0;
    }
}

/************************************************
 *
 ************************************************/
uint TrackGain::Engine::addBytes(const char *data, size_t size)
{
    const uint intSampleSize = mBitsPerSample / 8;

    size_t cnt = std::min(size, size_t(intSampleSize - mIntSampleIndex)) % intSampleSize;
    if (cnt == 0) {
        return 0;
    }

    for (size_t i = 0; i < cnt; ++i) {
        mIntSample      = (mIntSample & ~(0xFF << (mIntSampleIndex * 8))) | (data[i] << mIntSampleIndex * 8);
        mIntSampleIndex = (mIntSampleIndex + 1) % (intSampleSize);
    }

    if (mIntSampleIndex == 0) {
        addFloatSample(mIntSample);
        mIntSample = 0;
    }

    return cnt;
}

/************************************************
 *
 ************************************************/
void TrackGain::Engine::add_int8(const char *data, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        addFloatSample(uint8_t(data[i]));
    }

    mRemains -= size;

    if (mRemains == 0 && mFloatSamplesIndex != 0) {
        calc(mFloatSamplesIndex);
    }
}

/************************************************
 *
 ************************************************/
void TrackGain::Engine::add_int16(const char *data, size_t size)
{
    {
        size_t cnt = addBytes(data, size);
        data += cnt;
        size -= cnt;
        mRemains -= cnt;
    }

    {
        using T    = uint16_t;
        size_t cnt = size / sizeof(T);

        const T *d = (const T *)(data);
        for (size_t i = 0; i < cnt; ++i) {
            addFloatSample(d[i]);
        }

        data += cnt * sizeof(T);
        size -= cnt * sizeof(T);
        mRemains -= cnt * sizeof(T);
    }

    if (size) {
        size_t cnt = addBytes(data, size);
        mRemains -= cnt;
    }

    if (mRemains == 0 && mFloatSamplesIndex != 0) {
        calc(mFloatSamplesIndex);
    }
}

/************************************************
 *
 ************************************************/
void TrackGain::Engine::add_int24(const char *data, size_t size)
{
    {
        size_t cnt = addBytes(data, size);
        data += cnt;
        size -= cnt;
        mRemains -= cnt;
    }

    {
        size_t cnt = size / 3;

        for (size_t i = 0; i < cnt; ++i) {
            uint32_t sample = *(const uint32_t *)(data + i * 3);
            addFloatSample(sample);
        }

        data += cnt * 3;
        size -= cnt * 3;
        mRemains -= cnt * 3;
    }

    if (size) {
        size_t cnt = addBytes(data, size);
        mRemains -= cnt;
    }

    if (mRemains == 0 && mFloatSamplesIndex != 0) {
        calc(mFloatSamplesIndex);
    }
}

/************************************************
 *
 ************************************************/
void TrackGain::Engine::add_int32(const char *data, size_t size)
{
    {
        size_t cnt = addBytes(data, size);
        data += cnt;
        size -= cnt;
        mRemains -= cnt;
    }

    {
        using T    = uint32_t;
        size_t cnt = size / sizeof(T);

        const T *d = (const T *)(data);
        for (size_t i = 0; i < cnt; ++i) {
            addFloatSample(d[i]);
        }

        data += cnt * sizeof(T);
        size -= cnt * sizeof(T);
        mRemains -= cnt * sizeof(T);
    }

    if (size) {
        size_t cnt = addBytes(data, size);
        mRemains -= cnt;
    }

    if (mRemains == 0 && mFloatSamplesIndex != 0) {
        calc(mFloatSamplesIndex);
    }
}

/************************************************
 *
 ************************************************/
void TrackGain::Engine::calc(uint32_t count)
{
    calcStereoPeak(count);
    yuleFilterStereoSamples(mFloatSamples, count);
    butterFilterStereoSamples(mFloatSamples, count);

    int32_t level = (int32_t)floor(100 * calcStereoRms(mFloatSamples, count));
#if DEBUG_LEVELS
    printf("LEVEL: %d  PEAK: %f  cnt:%d\n", level, mResult.mPeak, count);
#endif

    if (level < 0) {
        mResult.mHistogram[0]++;
    }
    else if (size_t(level) >= mResult.mHistogram.size()) {
        mResult.mHistogram[mResult.mHistogram.size() - 1]++;
    }
    else {
        mResult.mHistogram[level]++;
    }
}

/************************************************
 * Update largest absolute sample value
 ************************************************/
void TrackGain::Engine::calcStereoPeak(uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i) {
        mResult.mPeak = std::max(mResult.mPeak, std::abs(mFloatSamples[i]));
    }
}

/************************************************
 * Optimized mplementation of 10th-order IIR stereo filter
 ************************************************/
void TrackGain::Engine::yuleFilterStereoSamples(float *samples, uint32_t size)
{
    size = size / 2;
    double left, right;
    int    i, j;

    i = mYuleHistI;

    // If filter history is very small magnitude, clear it completely to prevent denormals
    // from rattling around in there forever (slowing us down).

    for (j = -20; j < 0; ++j)
        if (fabs(mYuleHistA[i + j]) > 1e-10 || fabs(mYuleHistB[i + j]) > 1e-10)
            break;

    if (!j) {
        memset(mYuleHistA, 0, sizeof(mYuleHistA));
        memset(mYuleHistB, 0, sizeof(mYuleHistB));
    }

    while (size--) {
        left  = (mYuleHistB[i] = samples[0]) * mYuleCoeffB[0];
        right = (mYuleHistB[i + 1] = samples[1]) * mYuleCoeffB[0];
        left += mYuleHistB[i - 2] * mYuleCoeffB[1] - mYuleHistA[i - 2] * mYuleCoeffA[1];
        right += mYuleHistB[i - 1] * mYuleCoeffB[1] - mYuleHistA[i - 1] * mYuleCoeffA[1];
        left += mYuleHistB[i - 4] * mYuleCoeffB[2] - mYuleHistA[i - 4] * mYuleCoeffA[2];
        right += mYuleHistB[i - 3] * mYuleCoeffB[2] - mYuleHistA[i - 3] * mYuleCoeffA[2];
        left += mYuleHistB[i - 6] * mYuleCoeffB[3] - mYuleHistA[i - 6] * mYuleCoeffA[3];
        right += mYuleHistB[i - 5] * mYuleCoeffB[3] - mYuleHistA[i - 5] * mYuleCoeffA[3];
        left += mYuleHistB[i - 8] * mYuleCoeffB[4] - mYuleHistA[i - 8] * mYuleCoeffA[4];
        right += mYuleHistB[i - 7] * mYuleCoeffB[4] - mYuleHistA[i - 7] * mYuleCoeffA[4];
        left += mYuleHistB[i - 10] * mYuleCoeffB[5] - mYuleHistA[i - 10] * mYuleCoeffA[5];
        right += mYuleHistB[i - 9] * mYuleCoeffB[5] - mYuleHistA[i - 9] * mYuleCoeffA[5];
        left += mYuleHistB[i - 12] * mYuleCoeffB[6] - mYuleHistA[i - 12] * mYuleCoeffA[6];
        right += mYuleHistB[i - 11] * mYuleCoeffB[6] - mYuleHistA[i - 11] * mYuleCoeffA[6];
        left += mYuleHistB[i - 14] * mYuleCoeffB[7] - mYuleHistA[i - 14] * mYuleCoeffA[7];
        right += mYuleHistB[i - 13] * mYuleCoeffB[7] - mYuleHistA[i - 13] * mYuleCoeffA[7];
        left += mYuleHistB[i - 16] * mYuleCoeffB[8] - mYuleHistA[i - 16] * mYuleCoeffA[8];
        right += mYuleHistB[i - 15] * mYuleCoeffB[8] - mYuleHistA[i - 15] * mYuleCoeffA[8];
        left += mYuleHistB[i - 18] * mYuleCoeffB[9] - mYuleHistA[i - 18] * mYuleCoeffA[9];
        right += mYuleHistB[i - 17] * mYuleCoeffB[9] - mYuleHistA[i - 17] * mYuleCoeffA[9];
        left += mYuleHistB[i - 20] * mYuleCoeffB[10] - mYuleHistA[i - 20] * mYuleCoeffA[10];
        right += mYuleHistB[i - 19] * mYuleCoeffB[10] - mYuleHistA[i - 19] * mYuleCoeffA[10];
        samples[0] = mYuleHistA[i] = (float)left;
        samples[1] = mYuleHistA[i + 1] = (float)right;
        samples += 2;

        if ((i += 2) == 256) {
            memcpy(mYuleHistA, mYuleHistA + 236, sizeof(mYuleHistA[0]) * 20);
            memcpy(mYuleHistB, mYuleHistB + 236, sizeof(mYuleHistB[0]) * 20);
            i = 20;
        }
    }

    mYuleHistI = i;
}

/************************************************
 * Optimized mplementation of 2nd-order IIR stereo filter
 ************************************************/
void TrackGain::Engine::butterFilterStereoSamples(float *samples, uint32_t size)
{
    size = size / 2;
    double left, right;
    int    i, j;

    i = mButterHistI;

    // If filter history is very small magnitude, clear it completely to prevent denormals
    // from rattling around in there forever (slowing us down).

    for (j = -4; j < 0; ++j)
        if (fabs(butter_hist_a[i + j]) > 1e-10 || fabs(butter_hist_b[i + j]) > 1e-10)
            break;

    if (!j) {
        memset(butter_hist_a, 0, sizeof(butter_hist_a));
        memset(butter_hist_b, 0, sizeof(butter_hist_b));
    }

    while (size--) {
        left  = (butter_hist_b[i] = samples[0]) * mButterCoeffB[0];
        right = (butter_hist_b[i + 1] = samples[1]) * mButterCoeffB[0];
        left += butter_hist_b[i - 2] * mButterCoeffB[1] - butter_hist_a[i - 2] * mButterCoeffA[1];
        right += butter_hist_b[i - 1] * mButterCoeffB[1] - butter_hist_a[i - 1] * mButterCoeffA[1];
        left += butter_hist_b[i - 4] * mButterCoeffB[2] - butter_hist_a[i - 4] * mButterCoeffA[2];
        right += butter_hist_b[i - 3] * mButterCoeffB[2] - butter_hist_a[i - 3] * mButterCoeffA[2];
        samples[0] = butter_hist_a[i] = (float)left;
        samples[1] = butter_hist_a[i + 1] = (float)right;
        samples += 2;

        if ((i += 2) == 256) {
            memcpy(butter_hist_a, butter_hist_a + 252, sizeof(butter_hist_a[0]) * 4);
            memcpy(butter_hist_b, butter_hist_b + 252, sizeof(butter_hist_b[0]) * 4);
            i = 4;
        }
    }

    mButterHistI = i;
}

/************************************************
 * Calculate stereo rms level. Minimum value is about -100 dB for digital silence. The 90 dB
 * offset is to compensate for the normalized float range and 3 dB is for stereo samples.
 ************************************************/
double TrackGain::Engine::calcStereoRms(float *samples, uint32_t size) const
{
    double sum = 1e-16;

    for (uint32_t i = 0; i < size; ++i) {
        sum += samples[i] * samples[i];
    }

    return 10 * log10(sum / (size / 2)) + 90.0 - 3.0;
}

/************************************************
 *
 ************************************************/
TrackGain::TrackGain() :
    mEngine(new Engine(mResult))
{
}

/************************************************
 *
 ************************************************/
TrackGain::~TrackGain()
{
    delete mEngine;
}

/************************************************
 *
 ************************************************/
void TrackGain::add(const char *data, size_t size)
{
    if (!mEngine->mHeaderReady) {
        size_t pos = mEngine->loadHeader(data, size);

        if (pos >= size) {
            return;
        }

        size -= pos;
        data += pos;
    }

    size = std::min(size, mEngine->mRemains);

    if (size) {
        // clang-format off
        switch (mEngine->type) {
            case WavType::Int8:  return mEngine->add_int8(data, size);
            case WavType::Int16: return mEngine->add_int16(data, size);
            case WavType::Int24: return mEngine->add_int24(data, size);
            case WavType::Int32: return mEngine->add_int32(data, size);
        }
        // clang-format on
    }
}

/************************************************
 *
 ************************************************/
void AlbumGain::add(const Result &trackGain)
{
    Result::Histogram       &albumHistogram = mResult.mHistogram;
    const Result::Histogram &trackHistogram = trackGain.histogram();

    for (size_t i = 0; i < albumHistogram.size(); ++i) {
        albumHistogram[i] += trackHistogram[i];
    }

    mResult.mPeak = std::max(mResult.mPeak, trackGain.peak());
}

/************************************************
 *
 ************************************************/
Result::Result()
{
    registerQtMetaTypes();
}

/************************************************
 *
 ************************************************/
Result::Result(const Result &other) :
    mHistogram(other.mHistogram),
    mPeak(other.mPeak)
{
    registerQtMetaTypes();
}

/************************************************
 * Calculate the ReplayGain value from the specified loudness histogram; clip to -24 / +64 dB
 ************************************************/
float Result::gain() const
{
    uint32_t loud_count    = 0;
    uint32_t total_windows = 0;
    float    unclipped_gain;
    size_t   i;

    for (i = 0; i < mHistogram.size(); i++) {
        total_windows += mHistogram[i];
    }

    while (i--) {
        if ((loud_count += mHistogram[i]) * 20 >= total_windows) {
            break;
        }
    }

    unclipped_gain = (float)(64.54 - i / 100.0);

    if (unclipped_gain > 64.0) {
        return 64.0;
    }

    if (unclipped_gain < -24.0) {
        return -24.0;
    }

    return unclipped_gain;
}
