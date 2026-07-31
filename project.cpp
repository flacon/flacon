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

#include "project.h"
#include "settings.h"
#include "cue.h"
#include "inputaudiofile.h"
#include "audiofilematcher.h"
#include "appconfig.h"

#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QDir>

/************************************************

 ************************************************/
void Project::clear()
{
    QList<Disc *> discs;
    for (int i = 0; i < count(); ++i)
        discs << disc(i);

    removeDisc(discs);
}

/************************************************

 ************************************************/
Project *Project::instance()
{
    static Project *inst = nullptr;
    if (!inst)
        inst = new Project();

    return inst;
}

/************************************************

 ************************************************/
Project::Project(QObject *parent) :
    QObject(parent)
{
}

/************************************************

 ************************************************/
Disc *Project::disc(int index) const
{
    return mDiscs.at(index);
}

/************************************************

 ************************************************/
int Project::count() const
{
    return mDiscs.count();
}

/************************************************

 ************************************************/
int Project::insertDisc(Disc *disc, int index)
{
    if (index < 0)
        index = mDiscs.count();

    mDiscs.insert(index, disc);
    mValidator.insertDisk(disc, index);

    emit layoutChanged();
    return index;
}

/************************************************

 ************************************************/
void Project::removeDisc(const QList<Disc *> &discs)
{
    mValidator.removeDisk(discs);

    for (Disk *disc : discs) {
        emit beforeRemoveDisc(disc);
        if (mDiscs.removeAll(disc)) {
            disc->deleteLater();
        }

        emit afterRemoveDisc();
    }
}

/************************************************

 ************************************************/
int Project::indexOf(const Disc *disc) const
{
    return mDiscs.indexOf(const_cast<Disc *>(disc));
}

/************************************************
 *
 ************************************************/
bool Project::discExists(const QString &cueUri)
{
    foreach (const Disc *d, mDiscs) {
        if (d->cueFilePath() == cueUri)
            return true;
    }
    return false;
}

/************************************************

 ************************************************/
Disc *Project::addFile(const QString &fileName, bool isOptional) noexcept(false)
{
    return addFile(QFileInfo(fileName), isOptional);
}

/************************************************

 ************************************************/
Disc *Project::addFile(const QFileInfo &file, bool isOptional) noexcept(false)
{
    return (file.size() > 102400) ? addAudioFile(file, isOptional) : addCueFile(file, isOptional);
}

/************************************************

 ************************************************/
Disc *Project::addAudioFile(const QString &fileName, bool isOptional) noexcept(false)
{
    return addAudioFile(QFileInfo(fileName), isOptional);
}

/************************************************

 ************************************************/
Disc *Project::addAudioFile(const QFileInfo &file, bool isOptional) noexcept(false)
{
    for (int i = 0; i < count(); ++i) {
        if (disc(i)->audioFilePaths().contains(file.canonicalFilePath())) {
            return nullptr;
        }
    }

    InputAudioFile audio(file.absoluteFilePath());
    if (!audio.isValid()) {
        throw FlaconError(audio.errorString());
    }

    AudioFileMatcher matcher;
    matcher.matchForAudio(file.filePath());
    InputAudioFileList audioFiles = matcher.audioFiles();

    if (!isOptional && !matcher.audioFiles().contains(audio)) {
        audioFiles.insert(0, audio);
    }

    return addDisc(matcher.cue(), audioFiles);
}

/************************************************

 ************************************************/
Disc *Project::addCueFile(const QString &fileName, bool isOptional)
{
    return addCueFile(QFileInfo(fileName), isOptional);
}

/************************************************

 ************************************************/
Disc *Project::addCueFile(const QFileInfo &file, bool isOptional)
{
    try {
        Cue cue(file.absoluteFilePath());

        if (discExists(cue.filePath())) {
            return nullptr;
        }

        AudioFileMatcher matcher;
        matcher.matchForCue(cue);

        if (!isOptional && matcher.cue().isEmpty()) {
            return addDisc(cue, matcher.audioFiles());
        }

        return addDisc(matcher.cue(), matcher.audioFiles());
    }
    catch (FlaconError &err) {
        emit layoutChanged();
        qWarning() << err.what();
        throw err;
    }
}

/************************************************

 ************************************************/
Disc *Project::addDisc(const Cue cue, const InputAudioFileList audioFiles)
{
    Disc *disc = new Disc();

    if (!cue.isEmpty()) {
        disc->setCue(cue);
    }

    if (!audioFiles.isEmpty()) {
        disc->setAudioFiles(audioFiles);
    }

    if (disc->isEmpty()) {
        delete disc;
        return nullptr;
    }

    disc->searchCoverImage();
    addDisc(disc);
    emit layoutChanged();
    return disc;
}

/************************************************

 ************************************************/
bool Project::selectProfile(const QString &profileId)
{
    Profile *p = mProfiles.find(profileId);
    if (p) {
        mProfile = p;
    }
    else {
        if (!mProfiles.isEmpty()) {
            mProfile = &mProfiles.first();
        }
        else {
            static Profile nullProfile;
            mProfile = &nullProfile;
        }
    }

    mValidator.setProfile(profile());

    return p != nullptr;
}

/************************************************

 ************************************************/
void Project::setProfiles(Profiles profiles)
{
    QString id = profile()->id();
    mProfiles  = profiles;
    selectProfile(id);
}

/************************************************
 *
 ************************************************/
void Project::load(Settings *settings)
{
#if !BUNDLED_PROGRAMS
    settings->readExtPrograms();
#endif
    mProfiles = settings->readProfiles();
    selectProfile(settings->readCurrentProfileId());
}

/************************************************
 *
 ************************************************/
void Project::save(Settings *settings)
{
#if !BUNDLED_PROGRAMS
    settings->writeExtPrograms();
#endif
    settings->writeProfiles(mProfiles);
    settings->writeCurrentProfileId(profile()->id());
    settings->sync();
}

/************************************************

 ************************************************/
void Project::emitDiscChanged(Disc *disc)
{
    emit discChanged(disc);
}

/************************************************

 ************************************************/
void Project::emitLayoutChanged()
{
    emit layoutChanged();
    if (mValidator.isValid()) {
        mValidator.revalidate();
    }
}
