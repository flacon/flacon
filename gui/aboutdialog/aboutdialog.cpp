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

#include "aboutdialog.h"
#include "types.h"
#include <QDate>
#include <QList>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>

/************************************************

 ************************************************/
AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    setWindowTitle(tr("About Flacon"));

    this->layout()->setSpacing(10);
    titleLabel->setStyleSheet("#titleLabel { color: #FFFFFF; }");

    authorsEdit->viewport()->setAutoFillBackground(false);
    thanksEdit->viewport()->setAutoFillBackground(false);
    translationsEdit->viewport()->setAutoFillBackground(false);
    programsEdit->viewport()->setAutoFillBackground(false);

    QString css = "<style TYPE='text/css'> "
                  "body { font-family: sans-serif;} "
                  ".name { font-size: 16pt; } "
                  "a { white-space: nowrap ;} "
                  "h2 { font-size: 10pt;} "
                  "li { line-height: 120%;} "
                  ".techInfoKey { white-space: nowrap ; margin: 0 20px 0 16px; } "
                  "</style>";

    titleLabel->setText(css + titleText());

    aboutLabel->setText(descriptionText() + "<br><br><br>" + copyrightText() + "<hr>" +

                        tr("Homepage: %1").arg(homepageText()) + "<p>" +

                        tr("Bug tracker %1", "About dialog, About tab").arg(bugTrackerText()) + "<p>" +

                        tr("License: %1").arg(licenseText()));

    authorsEdit->setHtml(css + authorsInfo().asString());
    thanksEdit->setHtml(css + tr("Special thanks to:") + thanksInfo().asString());
    translationsEdit->setHtml(css + translationsText());
    programsEdit->setHtml(css + tr("Flacon uses external programs. Many thanks to their authors!") + programsInfo().asString());
}

/************************************************
 *
 ************************************************/
void AboutDialog::paintEvent(QPaintEvent *)
{
    QRect    rect(0, 0, this->width(), titleLabel->pos().y() + titleLabel->height());
    QPainter painter(this);
    painter.fillRect(rect, QColor::fromRgb(0x404040));
}

/************************************************

 ************************************************/
QString AboutDialog::titleText() const
{
    return "<table style='width:100%' border=0><tr>"
           "<td style='padding:8px 8px;'><img src=':/48/mainicon' style='margin:8 px;'></td>"
           "<td style='padding:8px 8px;'>"
            +
#ifdef GIT_BRANCH
            QString("<div class=name>Flacon</div> developer version."
                    "<div class=ver>%1 + git %2</b> "
                    "<a href='https://github.com/flacon/flacon/commit/%3'>%3</a></div>")
                    .arg(FLACON_VERSION)
                    .arg(GIT_BRANCH)
                    .arg(GIT_COMMIT_HASH)
            +
#else
            QString("<div class=name>Flacon</div><div class=ver>Version %1</div>").arg(FLACON_VERSION) +
#endif
            "</td></tr></table>";
}

/************************************************

 ************************************************/
QString AboutDialog::descriptionText() const
{
    return tr("Extracts individual tracks from one big audio file containing the entire album.");
}

/************************************************

 ************************************************/
QString AboutDialog::copyrightText() const
{
    return tr("Copyright: %1-%2 %3").arg("2012", QDate::currentDate().toString("yyyy"), "Alexander Sokolov");
}

/************************************************

 ************************************************/
QString AboutDialog::bugTrackerText() const
{
    return "<a href='https://github.com/flacon/flacon/issues'>https://github.com/flacon/flacon/issues</a>";
}

/************************************************

 ************************************************/
QString AboutDialog::homepageText() const
{
    return "<a href='http://flacon.github.io/'>flacon.github.io</a>";
}

/************************************************

 ************************************************/
QString AboutDialog::licenseText() const
{
    return "<a href='http://www.gnu.org/licenses/lgpl-2.1.html'>GNU Lesser General Public License version 2.1 or later</a>";
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
               tr("WavPack support patch", "Thanks on the about page"));

    result.add("Kyrill Detinov",
               "mailto:lazy.kent@opensuse.org",
               tr("Packaging, testing", "Thanks on the about page"));

    result.add("",
               "mailto:ao.french.l10n@rbox.me",
               tr("Improvements in the UI", "Thanks on the about page"));

    result.add("Taras Sokol",
               "mailto:tsokolp@gmail.com",
               tr("Flacon account on github.com", "Thanks on the about page"));

    result.add("FlatIcon",
               "https://www.flaticon.com",
               tr("Icon for application", "Thanks on the about page"));

    result.add("Icons8",
               "https://icons8.com",
               tr("Icons for application", "Thanks on the about page"));

    return result;
}

/************************************************

 ************************************************/
QString AboutDialog::translationsText() const
{
    return tr("Flacon is translated into many languages thanks to the work of the Flacon translation teams on <a href='%1'>Transifex</a>.")
            .arg("https://www.transifex.com/sokoloff/flacon/");
}

/************************************************

 ************************************************/
AboutInfo AboutDialog::programsInfo() const
{
    AboutInfo result;

    result.add("alacenc",
               "https://github.com/flacon/alacenc");

    result.add("flac",
               "http://flac.sourceforge.net");

    result.add("FAAC",
               "http://www.audiocoding.com");

    result.add("LAME",
               "http://lame.sourceforge.net");

    result.add("mac",
               "http://www.monkeysaudio.com");

    result.add("oggenc",
               "http://www.xiph.org");

    result.add("opusenc",
               "http://www.xiph.org");

    result.add("sox",
               "https://sox.sourceforge.net");

    result.add("ttaenc",
               "http://tta.sourceforge.net");

    result.add("WavPack",
               "http://www.wavpack.com");

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
AboutInfo::AboutInfo() :
    QList<AboutInfoItem>()
{
}

/************************************************

 ************************************************/
QString AboutInfo::asString() const
{
    AboutInfo list = *this;
    std::sort(list.begin(), list.end(), aboutItemLessThan);
    QString result;

    result += "<ul>";
    foreach (AboutInfoItem item, list) {
        result += "<li>" + item.name;

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QStringList urls = item.url.split(" ", QString::SkipEmptyParts);
#else
        QStringList urls = item.url.split(" ", Qt::SkipEmptyParts);
#endif
        foreach (QString url, urls) {
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
    item.name        = name;
    item.url         = url;
    item.description = description;
    this->append(item);
}
