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

#ifndef TRANSLATORSINFO_H
#define TRANSLATORSINFO_H
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QStringList>

class Translator
{
public:
    Translator(const QString &englishName, const QString &nativeName, const QString &contact);

    QString englishName() const { return mEnglishName; }
    QString nativeName() const { return mNativeName; }
    QString contact() const { return mContact; }

    QString info() const { return mInfo; }

    void    addLanguage(QString langId);
    QString asHtml();

private:
    QString mEnglishName;
    QString mNativeName;
    QString mContact;

    QString     mInfo;
    QStringList mLanguages;
};

class TranslatorsInfo
{
public:
    TranslatorsInfo();
    ~TranslatorsInfo();
    QString asHtml() const;

private:
    QMap<QString, Translator *> mItems;
    void                        process(const QString &lang, const QString &englishName, const QString &nativeName, const QString &contact);
};

#endif // TRANSLATORSINFO_H
