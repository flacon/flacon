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


#include "translatorsinfo.h"
#include <QDebug>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtGui/QTextDocument>

void fillLangguages(QMap<QString, QString> *languages)
{
    languages->insert("ach"   ,"Acoli");
    languages->insert("af"    ,"Afrikaans");
    languages->insert("af_ZA" ,"Afrikaans (South Africa)");
    languages->insert("ak"    ,"Akan");
    languages->insert("sq"    ,"Albanian");
    languages->insert("sq_AL" ,"Albanian (Albania)");
    languages->insert("aln"   ,"Albanian Gheg");
    languages->insert("am"    ,"Amharic");
    languages->insert("am_ET" ,"Amharic (Ethiopia)");
    languages->insert("ar"    ,"Arabic");
    languages->insert("ar_SA" ,"Arabic (Saudi Arabia)");
    languages->insert("ar_AA" ,"Arabic (Unitag)");
    languages->insert("an"    ,"Aragonese");
    languages->insert("hy"    ,"Armenian");
    languages->insert("hy_AM" ,"Armenian (Armenia)");
    languages->insert("as"    ,"Assamese");
    languages->insert("as_IN" ,"Assamese (India)");
    languages->insert("ast"   ,"Asturian");
    languages->insert("az"    ,"Azerbaijani");
    languages->insert("az_AZ" ,"Azerbaijani (Azerbaijan)");
    languages->insert("bal"   ,"Balochi");
    languages->insert("eu"    ,"Basque");
    languages->insert("eu_ES" ,"Basque (Spain)");
    languages->insert("be"    ,"Belarusian");
    languages->insert("be_BY" ,"Belarusian (Belarus)");
    languages->insert("be@tarask"     ,"Belarusian (Tarask)");
    languages->insert("bn"    ,"Bengali");
    languages->insert("bn_BD" ,"Bengali (Bangladesh)");
    languages->insert("bn_IN" ,"Bengali (India)");
    languages->insert("brx"   ,"Bodo");
    languages->insert("bs"    ,"Bosnian");
    languages->insert("bs_BA" ,"Bosnian (Bosnia and Herzegovina)");
    languages->insert("br"    ,"Breton");
    languages->insert("bg"    ,"Bulgarian");
    languages->insert("bg_BG" ,"Bulgarian (Bulgaria)");
    languages->insert("my"    ,"Burmese");
    languages->insert("my_MM" ,"Burmese (Myanmar)");
    languages->insert("ca"    ,"Catalan");
    languages->insert("ca_ES" ,"Catalan (Spain)");
    languages->insert("ca@valencia"   ,"Catalan (Valencian)");
    languages->insert("hne"   ,"Chhattisgarhi");
    languages->insert("cgg"   ,"Chiga");
    languages->insert("zh"    ,"Chinese");
    languages->insert("zh_CN" ,"Chinese (China)");
    languages->insert("zh_CN.GB2312"  ,"Chinese (China) (GB2312)");
    languages->insert("zh_HK" ,"Chinese (Hong Kong)");
    languages->insert("zh_TW" ,"Chinese (Taiwan)");
    languages->insert("zh_TW.Big5"    ,"Chinese (Taiwan) (Big5) ");
    languages->insert("kw"    ,"Cornish");
    languages->insert("co"    ,"Corsican");
    languages->insert("crh"   ,"Crimean Turkish");
    languages->insert("hr"    ,"Croatian");
    languages->insert("hr_HR" ,"Croatian (Croatia)");
    languages->insert("cs"    ,"Czech");
    languages->insert("cs_CZ" ,"Czech (Czech Republic)");
    languages->insert("da"    ,"Danish");
    languages->insert("da_DK" ,"Danish (Denmark)");
    languages->insert("nl"    ,"Dutch");
    languages->insert("nl_BE" ,"Dutch (Belgium)");
    languages->insert("nl_NL" ,"Dutch (Netherlands)");
    languages->insert("dz"    ,"Dzongkha");
    languages->insert("dz_BT" ,"Dzongkha (Bhutan)");
    languages->insert("en"    ,"English");
    languages->insert("en_AU" ,"English (Australia)");
    languages->insert("en_CA" ,"English (Canada)");
    languages->insert("en_IE" ,"English (Ireland)");
    languages->insert("en_ZA" ,"English (South Africa)");
    languages->insert("en_GB" ,"English (United Kingdom)");
    languages->insert("en_US" ,"English (United States)");
    languages->insert("eo"    ,"Esperanto");
    languages->insert("et"    ,"Estonian");
    languages->insert("et_EE" ,"Estonian (Estonia)");
    languages->insert("fo"    ,"Faroese");
    languages->insert("fo_FO" ,"Faroese (Faroe Islands)");
    languages->insert("fil"   ,"Filipino");
    languages->insert("fi"    ,"Finnish");
    languages->insert("fi_FI" ,"Finnish (Finland)");
    languages->insert("frp"   ,"Franco-Provençal (Arpitan)");
    languages->insert("fr"    ,"French");
    languages->insert("fr_CA" ,"French (Canada)");
    languages->insert("fr_FR" ,"French (France)");
    languages->insert("fr_CH" ,"French (Switzerland)");
    languages->insert("fur"   ,"Friulian");
    languages->insert("ff"    ,"Fulah");
    languages->insert("gd"    ,"Gaelic, Scottish");
    languages->insert("gl"    ,"Galician");
    languages->insert("gl_ES" ,"Galician (Spain)");
    languages->insert("lg"    ,"Ganda");
    languages->insert("ka"    ,"Georgian");
    languages->insert("ka_GE" ,"Georgian (Georgia)");
    languages->insert("de"    ,"German");
    languages->insert("de_DE" ,"German (Germany)");
    languages->insert("de_CH" ,"German (Switzerland)");
    languages->insert("el"    ,"Greek");
    languages->insert("el_GR" ,"Greek (Greece)");
    languages->insert("gu"    ,"Gujarati");
    languages->insert("gu_IN" ,"Gujarati (India)");
    languages->insert("gun"   ,"Gun");
    languages->insert("ht"    ,"Haitian (Haitian Creole)");
    languages->insert("ht_HT" ,"Haitian (Haitian Creole) (Haiti)");
    languages->insert("ha"    ,"Hausa");
    languages->insert("he"    ,"Hebrew");
    languages->insert("he_IL" ,"Hebrew (Israel)");
    languages->insert("hi"    ,"Hindi");
    languages->insert("hi_IN" ,"Hindi (India)");
    languages->insert("hu"    ,"Hungarian");
    languages->insert("hu_HU" ,"Hungarian (Hungary)");
    languages->insert("is"    ,"Icelandic");
    languages->insert("is_IS" ,"Icelandic (Iceland)");
    languages->insert("ig"    ,"Igbo");
    languages->insert("ilo"   ,"Iloko");
    languages->insert("id"    ,"Indonesian");
    languages->insert("id_ID" ,"Indonesian (Indonesia)");
    languages->insert("ia"    ,"Interlingua");
    languages->insert("ga"    ,"Irish");
    languages->insert("ga_IE" ,"Irish (Ireland)");
    languages->insert("it"    ,"Italian");
    languages->insert("it_IT" ,"Italian (Italy)");
    languages->insert("ja"    ,"Japanese");
    languages->insert("ja_JP" ,"Japanese (Japan)");
    languages->insert("jv"    ,"Javanese");
    languages->insert("kn"    ,"Kannada");
    languages->insert("kn_IN" ,"Kannada (India)");
    languages->insert("ks"    ,"Kashmiri");
    languages->insert("ks_IN" ,"Kashmiri (India)");
    languages->insert("csb"   ,"Kashubian");
    languages->insert("kk"    ,"Kazakh");
    languages->insert("kk_KZ" ,"Kazakh (Kazakhstan)");
    languages->insert("km"    ,"Khmer");
    languages->insert("km_KH" ,"Khmer (Cambodia)");
    languages->insert("rw"    ,"Kinyarwanda");
    languages->insert("ky"    ,"Kirgyz");
    languages->insert("tlh"   ,"Klingon");
    languages->insert("ko"    ,"Korean");
    languages->insert("ko_KR" ,"Korean (Korea)");
    languages->insert("ku"    ,"Kurdish");
    languages->insert("ku_IQ" ,"Kurdish (Iraq)");
    languages->insert("lo"    ,"Lao");
    languages->insert("lo_LA" ,"Lao (Laos)");
    languages->insert("la"    ,"Latin");
    languages->insert("lv"    ,"Latvian");
    languages->insert("lv_LV" ,"Latvian (Latvia)");
    languages->insert("li"    ,"Limburgian");
    languages->insert("ln"    ,"Lingala");
    languages->insert("lt"    ,"Lithuanian");
    languages->insert("lt_LT" ,"Lithuanian (Lithuania)");
    languages->insert("nds"   ,"Low German");
    languages->insert("lb"    ,"Luxembourgish");
    languages->insert("mk"    ,"Macedonian");
    languages->insert("mk_MK" ,"Macedonian (Macedonia)");
    languages->insert("mai"   ,"Maithili");
    languages->insert("mg"    ,"Malagasy");
    languages->insert("ms"    ,"Malay");
    languages->insert("ml"    ,"Malayalam");
    languages->insert("ml_IN" ,"Malayalam (India)");
    languages->insert("ms_MY" ,"Malay (Malaysia)");
    languages->insert("mt"    ,"Maltese");
    languages->insert("mt_MT" ,"Maltese (Malta)");
    languages->insert("mi"    ,"Maori");
    languages->insert("arn"   ,"Mapudungun");
    languages->insert("mr"    ,"Marathi");
    languages->insert("mr_IN" ,"Marathi (India)");
    languages->insert("mn"    ,"Mongolian");
    languages->insert("mn_MN" ,"Mongolian (Mongolia)");
    languages->insert("nah"   ,"Nahuatl");
    languages->insert("nr"    ,"Ndebele, South");
    languages->insert("nap"   ,"Neapolitan");
    languages->insert("ne"    ,"Nepali");
    languages->insert("ne_NP" ,"Nepali (Nepal)");
    languages->insert("se"    ,"Northern Sami");
    languages->insert("nso"   ,"Northern Sotho");
    languages->insert("no"    ,"Norwegian");
    languages->insert("nb"    ,"Norwegian Bokmål");
    languages->insert("nb_NO" ,"Norwegian Bokmål (Norway)");
    languages->insert("no_NO" ,"Norwegian (Norway)");
    languages->insert("nn"    ,"Norwegian Nynorsk");
    languages->insert("nn_NO" ,"Norwegian Nynorsk (Norway)");
    languages->insert("ny"    ,"Nyanja");
    languages->insert("oc"    ,"Occitan (post 1500)");
    languages->insert("or"    ,"Oriya");
    languages->insert("or_IN" ,"Oriya (India)");
    languages->insert("pa"    ,"Panjabi (Punjabi)");
    languages->insert("pa_IN" ,"Panjabi (Punjabi) (India)");
    languages->insert("pap"   ,"Papiamento");
    languages->insert("fa"    ,"Persian");
    languages->insert("fa_IR" ,"Persian (Iran)");
    languages->insert("pms"   ,"Piemontese");
    languages->insert("pl"    ,"Polish");
    languages->insert("pl_PL" ,"Polish (Poland)");
    languages->insert("pt"    ,"Portuguese");
    languages->insert("pt_BR" ,"Portuguese (Brazil)");
    languages->insert("pt_PT" ,"Portuguese (Portugal)");
    languages->insert("ps"    ,"Pushto");
    languages->insert("ro"    ,"Romanian");
    languages->insert("ro_RO" ,"Romanian (Romania)");
    languages->insert("rm"    ,"Romansh");
    languages->insert("ru"    ,"Russian");
    languages->insert("ru_RU" ,"Russian (Russia)");
    languages->insert("sm"    ,"Samoan");
    languages->insert("sc"    ,"Sardinian");
    languages->insert("sco"   ,"Scots");
    languages->insert("sr"    ,"Serbian");
    languages->insert("sr@latin"      ,"Serbian (Latin)");
    languages->insert("sr_RS@latin"   ,"Serbian (Latin) (Serbia)");
    languages->insert("sr_RS" ,"Serbian (Serbia)");
    languages->insert("sn"    ,"Shona");
    languages->insert("sd"    ,"Sindhi");
    languages->insert("si"    ,"Sinhala");
    languages->insert("si_LK" ,"Sinhala (Sri Lanka)");
    languages->insert("sk"    ,"Slovak");
    languages->insert("sk_SK" ,"Slovak (Slovakia)");
    languages->insert("sl"    ,"Slovenian");
    languages->insert("sl_SI" ,"Slovenian (Slovenia)");
    languages->insert("so"    ,"Somali");
    languages->insert("son"   ,"Songhay");
    languages->insert("st"    ,"Sotho, Southern");
    languages->insert("st_ZA" ,"Sotho, Southern (South Africa)");
    languages->insert("es"    ,"Spanish");
    languages->insert("es_AR" ,"Spanish (Argentina)");
    languages->insert("es_BO" ,"Spanish (Bolivia)");
    languages->insert("es_CL" ,"Spanish (Chile)");
    languages->insert("es_CO" ,"Spanish (Colombia)");
    languages->insert("es_CR" ,"Spanish (Costa Rica)");
    languages->insert("es_DO" ,"Spanish (Dominican Republic)");
    languages->insert("es_EC" ,"Spanish (Ecuador)");
    languages->insert("es_SV" ,"Spanish (El Salvador)");
    languages->insert("es_MX" ,"Spanish (Mexico)");
    languages->insert("es_NI" ,"Spanish (Nicaragua)");
    languages->insert("es_PA" ,"Spanish (Panama)");
    languages->insert("es_PY" ,"Spanish (Paraguay)");
    languages->insert("es_PE" ,"Spanish (Peru)");
    languages->insert("es_PR" ,"Spanish (Puerto Rico)");
    languages->insert("es_ES" ,"Spanish (Spain)");
    languages->insert("es_UY" ,"Spanish (Uruguay)");
    languages->insert("es_VE" ,"Spanish (Venezuela)");
    languages->insert("su"    ,"Sundanese");
    languages->insert("sw"    ,"Swahili");
    languages->insert("sw_KE" ,"Swahili (Kenya)");
    languages->insert("sv"    ,"Swedish");
    languages->insert("sv_FI" ,"Swedish (Finland)");
    languages->insert("sv_SE" ,"Swedish (Sweden)");
    languages->insert("tl"    ,"Tagalog");
    languages->insert("tl_PH" ,"Tagalog (Philippines)");
    languages->insert("tg"    ,"Tajik");
    languages->insert("tg_TJ" ,"Tajik (Tajikistan)");
    languages->insert("ta"    ,"Tamil");
    languages->insert("ta_IN" ,"Tamil (India)");
    languages->insert("ta_LK" ,"Tamil (Sri-Lanka)");
    languages->insert("tt"    ,"Tatar");
    languages->insert("te"    ,"Telugu");
    languages->insert("te_IN" ,"Telugu (India)");
    languages->insert("th"    ,"Thai");
    languages->insert("th_TH" ,"Thai (Thailand)");
    languages->insert("bo"    ,"Tibetan");
    languages->insert("bo_CN" ,"Tibetan (China)");
    languages->insert("ti"    ,"Tigrinya");
    languages->insert("to"    ,"Tongan");
    languages->insert("tr"    ,"Turkish");
    languages->insert("tr_TR" ,"Turkish (Turkey)");
    languages->insert("tk"    ,"Turkmen");
    languages->insert("ug"    ,"Uighur");
    languages->insert("uk"    ,"Ukrainian");
    languages->insert("uk_UA" ,"Ukrainian (Ukraine)");
    languages->insert("hsb"   ,"Upper Sorbian");
    languages->insert("ur"    ,"Urdu");
    languages->insert("ur_PK" ,"Urdu (Pakistan)");
    languages->insert("uz"    ,"Uzbek");
    languages->insert("ve"    ,"Venda");
    languages->insert("vi"    ,"Vietnamese");
    languages->insert("vi_VN" ,"Vietnamese (Vietnam)");
    languages->insert("vls"   ,"Vlaams");
    languages->insert("wa"    ,"Walloon");
    languages->insert("cy"    ,"Welsh");
    languages->insert("cy_GB" ,"Welsh (United Kingdom)");
    languages->insert("fy"    ,"Western Frisian");
    languages->insert("fy_NL" ,"Western Frisian (Netherlands)");
    languages->insert("wo"    ,"Wolof");
    languages->insert("wo_SN" ,"Wolof (Senegal)");
    languages->insert("xh"    ,"Xhosa");
    languages->insert("yi"    ,"Yiddish");
    languages->insert("yo"    ,"Yoruba");
    languages->insert("zu"    ,"Zulu");
    languages->insert("zu_ZA" ,"Zulu (South Africa)");
}


QString getValue(const QSettings &src, const QString &key)
{
    QString ret = src.value(key).toString().trimmed();
    if (ret == "-")
        return "";

    return ret;
}



TranslatorsInfo::TranslatorsInfo()
{
    //fillLangguages(&mLanguagesList);

    QSettings src(":/translatorsInfo", QSettings::IniFormat);
    src.setIniCodec("UTF-8");

    foreach(QString group, src.childGroups())
    {
        QString lang = group.section("_", 1).remove(".info");
        src.beginGroup(group);
        int cnt = src.allKeys().count();
        for (int i=0; i<cnt; i++)
        {
            QString nameEnglish = getValue(src, QString("translator_%1_nameEnglish").arg(i));
            QString nameNative = getValue(src, QString("translator_%1_nameNative").arg(i));
            QString contact = getValue(src, QString("translator_%1_contact").arg(i));

            if (nameEnglish.startsWith(QString("Translator %1. ").arg(i)))
                nameEnglish = "";

            if (nameNative.startsWith(QString("Translator %1. ").arg(i)))
                nameNative = "";

            if (contact.startsWith(QString("Translator %1. ").arg(i)))
                contact = "";

            if (nameEnglish.isEmpty())
                nameEnglish = nameNative;

            if (!nameEnglish.isEmpty())
            {
                process(lang, nameEnglish, nameNative, contact);
            }

        }
        src.endGroup();
    }

}

TranslatorsInfo::~TranslatorsInfo()
{
    qDeleteAll(mItems);
}

QString TranslatorsInfo::asHtml() const
{
    QString ret;
    foreach(Translator *translator, mItems)
    {
        ret += "<li>" + translator->asHtml() + "</li>";
    }

    return ret;
}



void TranslatorsInfo::process(const QString &lang, const QString &englishName, const QString &nativeName, const QString &contact)
{
    QString key = QString("%1:%2:%3").arg(englishName, nativeName, contact);
    Translator *translator = mItems.value(key);

    if (!translator)
    {
        translator = new Translator(englishName, nativeName, contact);
        mItems.insert(key, translator);
    }

    translator->addLanguage(lang);
}


Translator::Translator(const QString &englishName, const QString &nativeName, const QString &contact)
{
    mEnglishName = englishName;

    if (nativeName != englishName)
        mNativeName = nativeName;

    mContact = contact;

    if (mNativeName.isEmpty())
        mInfo = QString("%1").arg(mEnglishName);
    else
        mInfo = QString("%1 (%2)").arg(mEnglishName, mNativeName);

    if (!mContact.isEmpty())
    {
        if (mContact.contains(QRegExp("^(https?|mailto):")))
            mInfo = QString(" <a href='%1'>%2</a>").arg(contact, mInfo.toHtmlEscaped());
        else if (contact.contains("@") || contact.contains("<"))
            mInfo = QString(" <a href='mailto:%1'>%2</a>").arg(contact, mInfo.toHtmlEscaped());
        else
            mInfo = QString(" <a href='http://%1'>%2</a>").arg(contact, mInfo.toHtmlEscaped());
    }
}


void Translator::addLanguage(QString langId)
{
    static QMap<QString, QString> mLanguagesList;
    if (mLanguagesList.isEmpty())
    {
        fillLangguages(&mLanguagesList);
    }

    if (mLanguagesList.contains(langId))
        mLanguages << mLanguagesList.value(langId);
    else
        mLanguages << langId;
}


QString Translator::asHtml()
{
    QString ret(mInfo);
    ret += " - " + mLanguages.join(", ");
    return ret;
}

