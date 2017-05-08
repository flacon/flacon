/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2012-2013
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


#include "formats/format.h"
#include "settings.h"
#include "inputaudiofile.h"
#include "outformat.h"
#include <QDir>
#include <QDebug>
#include <QProcessEnvironment>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    #include <QDesktopServices>
#else
    #include <QStandardPaths>
#endif



#ifdef Q_OS_WIN
    #define PATH_ENV_SEPARATOR ';'
    #define BINARY_EXT ".exe"

#elif defined(Q_OS_OS2)
    #define PATH_ENV_SEPARATOR ';'
    #define BINARY_EXT ".exe"

#else
    #define PATH_ENV_SEPARATOR ':'
    #define BINARY_EXT ""

#endif

QString Settings::mFileName;
static Settings *inst = NULL;

/************************************************

 ************************************************/
Settings *Settings::instance()
{
    if (!inst)
    {
        if (mFileName.isEmpty())
            inst = new Settings("flacon", "flacon");
        else
            inst = new Settings(mFileName);
    }

    return inst;
}


/************************************************

 ************************************************/
void Settings::setFileName(const QString &fileName)
{
    mFileName = fileName;
    delete inst;
    inst = 0;
}


/************************************************

 ************************************************/
Settings::Settings(const QString &organization, const QString &application) :
    QSettings(organization, application)
{
    init();
}


/************************************************

 ************************************************/
Settings::Settings(const QString &fileName):
    QSettings(fileName, QSettings::IniFormat)
{
    init();
}


/************************************************

 ************************************************/
void Settings::init()
{

    setDefaultValue(Tags_DefaultCodepage,   "AUTODETECT");

    // Globals **********************************
    setDefaultValue(Encoder_ThreadCount,    8);
    setDefaultValue(Encoder_TmpDir,         "");

    // Out Files ********************************
    setDefaultValue(OutFiles_Pattern,       "%a/{%y - }%A/%n - %t");


#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
    QString outDir = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
#else
    QString outDir = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
#endif
    outDir.replace(QDir::homePath(), "~");
    setDefaultValue(OutFiles_Directory,     outDir);
    setDefaultValue(OutFiles_Format,        "FLAC");

    // Internet *********************************
    setDefaultValue(Inet_CDDBHost,          "freedb.freedb.org");

    // Misc *************************************
    setDefaultValue(Misc_LastDir,           QDir::homePath());

    // PerTrackCue **************************
    setDefaultValue(PerTrackCue_Create,     false);
    setDefaultValue(PerTrackCue_Pregap,     OutFormat::preGapTypeToString(OutFormat::PreGapExtractToFile));
    setDefaultValue(PerTrackCue_FlaconTags, true);

    // ConfigureDialog **********************
    setDefaultValue(ConfigureDialog_Width,  645);
    setDefaultValue(ConfigureDialog_Height, 425);


    foreach(OutFormat *format, OutFormat::allFormats())
    {
        QHashIterator<QString, QVariant> i(format->defaultParameters());
        while (i.hasNext())
        {
            i.next();
            setDefaultValue(i.key(), i.value());
        }

        mPrograms << format->encoderProgramName();
        mPrograms << format->gainProgramName();
    }


    foreach (const AudioFormat *format, AudioFormat::inputFormats())
    {
        mPrograms << format->decoderProgramName();
    }

    mPrograms.remove("");

    foreach(QString program, mPrograms)
    {
        if (value("Programs/" + program).toString().isEmpty())
            setValue("Programs/" + program, findProgram(program));
    }
}


/************************************************

 ************************************************/
QString Settings::keyToString(Settings::Key key) const
{
    switch (key)
    {
    case Tags_DefaultCodepage:  return "Tags/DefaultCodepage";

    // MainWindow **************************
    case MainWindow_Width:      return "MainWindow/Width";
    case MainWindow_Height:     return "MainWindow/Height";

    // Globals *****************************
    case Encoder_ThreadCount:   return "Encoder/ThreadCount";
    case Encoder_TmpDir:        return "Encoder/TmpDir";

    // Out Files ***************************
    case OutFiles_Pattern:      return "OutFiles/Pattern";
    case OutFiles_Directory:    return "OutFiles/Directory";
    case OutFiles_Format:       return "OutFiles/Format";
    case OutFiles_PatternHistory: return "OutFiles/PatternHistory";

    // Internet ****************************
    case Inet_CDDBHost:         return "Inet/CDDBHost";


    // Misc *********************************
    case Misc_LastDir:          return "Misc/LastDirectory";


    // PerTrackCue **************************
    case PerTrackCue_Create:    return "PerTrackCue/Create";
    case PerTrackCue_Pregap:    return "PerTrackCue/Pregap";
    case PerTrackCue_FlaconTags:return "PerTrackCue/InsertCreatorTag";

    // ConfigureDialog **********************
    case ConfigureDialog_Width:     return "ConfigureDialog/Width";
    case ConfigureDialog_Height:    return "ConfigureDialog/Height";
    }

    return "";
}

/************************************************

 ************************************************/
Settings::~Settings()
{
}



/************************************************

 ************************************************/
QVariant Settings::value(Key key, const QVariant &defaultValue) const
{
    return value(keyToString(key), defaultValue);
}


/************************************************

 ************************************************/
QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
{
    return QSettings::value(key, defaultValue);
}


/************************************************

 ************************************************/
bool Settings::checkProgram(const QString &program) const
{
    QString val = programName(program);

    if (val.isEmpty())
        return false;

    QFileInfo fi(val);
    return fi.exists() && fi.isExecutable();
}


/************************************************

 ************************************************/
QString Settings::programName(const QString &program) const
{
    return value("Programs/" + program).toString();
}


/************************************************

 ************************************************/
QString Settings::findProgram(const QString &program) const
{
    QStringList paths = QProcessEnvironment::systemEnvironment().value("PATH").split(PATH_ENV_SEPARATOR);
    foreach(QString path, paths)
    {
        QFileInfo fi(path + QDir::separator() + program + BINARY_EXT);
        if (fi.exists() && fi.isExecutable())
            return fi.absoluteFilePath();
    }
    return "";
}


/************************************************

 ************************************************/
void Settings::setValue(Settings::Key key, const QVariant &value)
{
    setValue(keyToString(key), value);
    emit changed();
}


/************************************************

 ************************************************/
void Settings::setValue(const QString &key, const QVariant &value)
{
    QSettings::setValue(key, value);
    emit changed();
}

/************************************************

 ************************************************/
void Settings::setDefaultValue(Key key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}


/************************************************

 ************************************************/
void Settings::setDefaultValue(const QString &key, const QVariant &defaultValue)
{
    setValue(key, value(key, defaultValue));
}




