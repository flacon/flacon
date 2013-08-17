/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Flacon - audio File Encoder
 * https://github.com/SokoloffA/flacon
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


#include "aboutdialog.h"
#include "translatorsinfo.h"
#include <QDate>
#include <QList>


/************************************************

 ************************************************/
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    setWindowTitle(tr("About Flacon"));

    logoLabel->setFixedSize(48, 48);
    logoLabel->setScaledContents(true);
    logoLabel->setPixmap(QPixmap(":logo"));

    authorsEdit->viewport()->setAutoFillBackground(false);
    thanksEdit->viewport()->setAutoFillBackground(false);
    translationsEdit->viewport()->setAutoFillBackground(false);
    programsEdit->viewport()->setAutoFillBackground(false);


    QString css="<style TYPE='text/css'> "
                    "body { font-family: sans-serif;} "
                    ".name { font-size: 16pt; } "
                    "a { white-space: nowrap ;} "
                    "h2 { font-size: 10pt;} "
                    "li { line-height: 120%;} "
                    ".techInfoKey { white-space: nowrap ; margin: 0 20px 0 16px; } "
                "</style>";

    titleLabel->setText(css + titleText());

    aboutLabel->setText(descriptionText() +
                        "<br><br><br>" +
                        copyrightText() +
                        "<hr>" +
                        tr("Homepage: %1").arg(homepageText()) +
                        "<p>" +
                        tr("License: %1").arg(licenseText()));


    authorsEdit->setHtml(css + authorsInfo().asString());
    thanksEdit->setHtml(css + tr("Special thanks to:") + thanksInfo().asString());
    translationsEdit->setHtml(css + translationsText());
    programsEdit->setHtml(css + tr("Flacon uses external programs. Many thanks to their authors!") + programsInfo().asString());
}


/************************************************

 ************************************************/
QString AboutDialog::titleText() const
{
    return QString("<div class=name>Flacon</div><div class=ver>%1</div>").arg(FLACON_VERSION);
}


/************************************************

 ************************************************/
QString AboutDialog::descriptionText() const
{
    return "Extracts individual tracks from one big audio file containing the entire album.";
}


/************************************************

 ************************************************/
QString AboutDialog::copyrightText() const
{
    return tr("Copyright: %1-%2 %3").arg("2012", QDate::currentDate().toString("yyyy"), "Alexander Sokolov");
}


/************************************************

 ************************************************/
QString AboutDialog::homepageText() const
{
    return "<a href='http://code.google.com/p/flacon'>code.google.com/p/flacon</a>";
}


/************************************************

 ************************************************/
QString AboutDialog::licenseText() const
{
    return "<a href='http://www.gnu.org/licenses/gpl-2.0.html'>GNU General Public License version 2</a>";
}


/************************************************

 ************************************************/
AboutInfo AboutDialog::authorsInfo() const
{
    AboutInfo result;
    result.add("Alexander Sokolov", "mailto:sokoloff.a@gmail.com");
    return result;
}


/************************************************

 ************************************************/
AboutInfo AboutDialog::thanksInfo() const
{
    AboutInfo result;

    result.add("Artem Aleksuk",
               "mailto:h31mail@yandex.ru",
               tr("WavPack support patch"));

    result.add("Charles Barcza",
               "mailto:kbarcza@blackpanther.hu",
               tr("Application icon, Packaging"));

    result.add("Kyrill Detinov",
               "mailto:lazy.kent@opensuse.org",
               tr("Packaging, testing"));

    result.add("Alain-Olivier Breysse",
               "mailto:yahoe.001@gmail.com",
               tr("Improvements in the UI"));

    return result;
}


/************************************************

 ************************************************/
QString AboutDialog::translationsText() const
{
    TranslatorsInfo translatorsInfo;
    return QString("%1<p><ul>%2</ul>").arg(
                tr("Flacon is translated into many languages thanks to the work of the translation teams all over the world."),
                translatorsInfo.asHtml()
                );
}


/************************************************

 ************************************************/
AboutInfo AboutDialog::programsInfo() const
{
    AboutInfo result;

    result.add("shntool",
               "http://etree.org/shnutils/shntool");

    result.add("flac and metaflac",
               "http://flac.sourceforge.net");

    result.add("mac",
               "http://etree.org/shnutils/shntool/support/formats/ape/unix http://www.monkeysaudio.com");

    result.add("oggenc",
               "http://www.xiph.org");

    result.add("LAME",
               "http://lame.sourceforge.net");

    result.add("WavPack",
               "http://www.wavpack.com");

    result.add("VorbisGain",
               "http://sjeng.org/vorbisgain.html");

    result.add("MP3Gain",
               "http://mp3gain.sourceforge.net");

    result.add("ttaenc",
               "http://tta.sourceforge.net");

    result.add("FAAC",
               "http://www.audiocoding.com");

    return result;
}


/************************************************

 ************************************************/
bool aboutItemLessThan(const AboutInfoItem &i1, const AboutInfoItem &i2)
{
     return i1.name.toLower() < i2.name.toLower();
 }


/************************************************

 ************************************************/
AboutInfo::AboutInfo():
    QList<AboutInfoItem>()
{
}


/************************************************

 ************************************************/
QString AboutInfo::asString() const
{
    AboutInfo list = *this;
    qSort(list.begin(), list.end(), aboutItemLessThan);
    QString result;

    result += "<ul>";
    foreach(AboutInfoItem item, list)
    {
        result += "<li>" + item.name;

        QStringList urls = item.url.split(" ", QString::SkipEmptyParts);
        foreach(QString url, urls)
        {
            QString text = QString(url).remove("mailto:").remove("http://");
            result += QString(" <a href='%1'>%2</a>").arg(url, text);
        }


        if (!item.description.isEmpty())
            result += "  - " + item.description;
        result += "</li>";
    }

    return result;
}


/************************************************

 ************************************************/
void AboutInfo::add(const QString &name, const QString &url, const QString &description)
{
    AboutInfoItem item;
    item.name = name;
    item.url = url;
    item.description = description;
    this->append(item);
}
