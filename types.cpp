/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/flacon/flacon
 *
 * Copyright: 2017
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


#include "types.h"
#include <QVector>


/************************************************
 *
 ************************************************/
QString preGapTypeToString(PreGapType type)
{
    switch(type)
    {
    case PreGapType::ExtractToFile:   return "Extract";
    case PreGapType::AddToFirstTrack: return "AddToFirst";
    default:                          return "Disable";
    }
}


/************************************************

 ************************************************/
PreGapType strToPreGapType(const QString &str)
{
    QString s = str.toUpper();

    if (s == "EXTRACT")     return PreGapType::ExtractToFile;
    if (s == "ADDTOFIRST")  return PreGapType::AddToFirstTrack;

    return PreGapType::AddToFirstTrack;
}



/************************************************

 ************************************************/
QString gainTypeToString(GainType type)
{
    switch(type)
    {
    case GainType::Disable: return "Disable";
    case GainType::Track:   return "Track";
    case GainType::Album:   return "Album";
    }

    return "Disable";
}


/************************************************

 ************************************************/
GainType strToGainType(const QString &str)
{
    QString s = str.toUpper();

    if (s == "TRACK")   return GainType::Track;
    if (s == "ALBUM")   return GainType::Album;

    return GainType::Disable;
}


/************************************************

 ************************************************/
QString coverModeToString(CoverMode mode)
{
    switch(mode)
    {
    case CoverMode::Disable:  return "Disable";
    case CoverMode::OrigSize: return "OrigSize";
    case CoverMode::Scale:    return "Scale";
    }

    return "Disable";

}


/************************************************

 ************************************************/
CoverMode strToCoverMode(const QString &str)
{
    QString s = str.toUpper();

    if (s == "ORIGSIZE") return CoverMode::OrigSize;
    if (s == "SCALE")    return CoverMode::Scale;

    return CoverMode::Disable;
}


/************************************************

 ************************************************/
unsigned int levenshteinDistance(const QString &s1, const QString & s2)
{
    const size_t len1 = s1.size(), len2 = s2.size();
    QVector<unsigned int> col(len2+1), prevCol(len2+1);

    for (int i = 0; i < prevCol.size(); i++)
        prevCol[i] = i;

    for (unsigned int i = 0; i < len1; i++)
    {
        col[0] = i+1;
        for (unsigned int j = 0; j < len2; j++)
            col[j+1] = qMin( qMin( 1 + col[j], 1 + prevCol[1 + j]),
                            prevCol[j] + (s1[i]==s2[j] ? 0 : 1) );
        col.swap(prevCol);
    }
    return prevCol[len2];
}


/************************************************
 *
 ************************************************/
QIcon loadIcon(const QString &iconName, bool loadDisable)
{
    QVector<int> sizes;
    sizes << 16 << 22 << 32 << 48 << 64 << 128 << 256 << 512;

    QIcon res;
    foreach (int size, sizes)
        res.addFile(QString(":%2/%1").arg(iconName).arg(size), QSize(size, size), QIcon::Normal);


    if (loadDisable)
    {
        foreach (int size, sizes)
            res.addFile(QString(":%2/%1_disable").arg(iconName).arg(size), QSize(size, size), QIcon::Disabled);
    }

    return res;
}
