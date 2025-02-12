#!/usr/bin/env python3

import os
import subprocess
import xml.dom.minidom as minidom
import hashlib

class Error(Exception):
    pass

##################################
#
def XML_Element_text(element):
    if type(element.firstChild) is minidom.xml.dom.minidom.Text:
        return element.firstChild.nodeValue
    else:
        return ""
minidom.xml.dom.minidom.Element.text = XML_Element_text

def XML_NodeList_text(nodeList):
    if len(nodeList) < 1:
        return ""

    return nodeList[0].text()

minidom.xml.dom.minicompat.NodeList.text = XML_NodeList_text


##################################
#
class TsMessage:
    ##############################
    #
    def __init__(self):
        self.source = ""
        self.context = ""
        self.comment = ""
        self.extracomment = ""
        self.translations = []
        self.numerus = False

    ##############################
    #
    @property
    def id(self):
        b = f"{self.source} ({self.comment}) @{self.context}".encode("UTF-8")
        return hashlib.sha256(b).hexdigest()


    ##############################
    #
    def load(self, xmlMessage):
        self.context = xmlMessage.parentNode.getElementsByTagName("name").text()
        self.numerus = xmlMessage.getAttribute("numerus") == "yes"
        self.source  = xmlMessage.getElementsByTagName("source").text()
        self.comment = xmlMessage.getElementsByTagName("comment").text()
        self.extracomment = xmlMessage.getElementsByTagName("extracomment").text()

        xmlTranslation = xmlMessage.getElementsByTagName("translation")
        if xmlTranslation:
            if self.numerus:
                for xmlNumerusform in xmlTranslation[0].getElementsByTagName("numerusform"):
                    if xmlNumerusform.text():
                        self.translations.append(xmlNumerusform.text())
            else:
                self.translations.append(xmlTranslation.text())


    ##############################
    #
    def write(self, xmlContext):
        xmlDoc = xmlContext.ownerDocument

        xmlMessage = xmlDoc.createElement("message")

        xmlSource = xmlDoc.createElement("source")
        xmlSource.appendChild(xmlDoc.createTextNode(self.source))
        xmlMessage.appendChild(xmlSource)

        if self.comment:
            xmlComment = xmlDoc.createElement("comment")
            xmlComment.appendChild(xmlDoc.createTextNode(self.comment))
            xmlMessage.appendChild(xmlComment)

        if self.extracomment:
            xmlExtraComment = xmlDoc.createElement("extracomment")
            xmlExtraComment.appendChild(xmlDoc.createTextNode(self.extracomment))
            xmlMessage.appendChild(xmlExtraComment)

        xmlTranslation = xmlDoc.createElement("translation")
        #xmlTranslation.setAttribute("type", "unfinished")
        xmlMessage.appendChild(xmlTranslation)

        if not self.numerus:
            xmlTranslation.appendChild(xmlDoc.createTextNode(self.translations[0]))
        else:
            xmlMessage.setAttribute("numerus", "yes")

            for s in self.translations[:3]:
                xmlNumerusform = xmlDoc.createElement("numerusform")
                xmlNumerusform.appendChild(xmlDoc.createTextNode(s))
                xmlTranslation.appendChild(xmlNumerusform)


        xmlContext.appendChild(xmlMessage)
        return xmlMessage


    ##############################
    #
    def json_comment(self):
        if not self.extracomment:
            return self.comment

        if not self.comment:
            return self.extracomment

        if self.comment == self.extracomment:
            return self.comment

        return f"{self.comment}\n----------\n{self.extracomment}"

    ##################################
    #
    def json_contex(self):
        res = self.context
        if self.comment:
            res += "\n----------\n" + self.comment

        return res

    ##################################
    #
    def _translation_forms(self):
        res = {"one": "", "few": "", "many": "", "other": ""}

        if len(self.translations) == 0:
            return res

        if len(self.translations) == 1:
            res["one"]   = self.translations[0]
            res["few"]   = self.translations[0]
            res["many"]  = self.translations[0]
            res["other"] = self.translations[0]
            return res

        if len(self.translations) == 2:
            res["one"]   = self.translations[0]
            res["few"]   = self.translations[1]
            res["many"]  = self.translations[1]
            res["other"] = self.translations[1]
            return res

        if len(self.translations) == 3:
            res["one"]   = self.translations[0]
            res["few"]   = self.translations[1]
            res["many"]  = self.translations[2]
            res["other"] = self.translations[2]
            return res

        if len(self.translations) == 4:
            res["one"]   = self.translations[0]
            res["few"]   = self.translations[1]
            res["many"]  = self.translations[2]
            res["other"] = self.translations[3]
            return res

        raise Error(f"Unsupported plural forms: {self.translations}")

    ##################################
    #
    def json_translation(self, lang):
        if not self.numerus:
            return self.translations[0]

        forms = self._translation_forms()

        res = ""
        for key in LANGS[lang]:
            res += " %s {%s}" % (key, forms[key])

        return "{n, plural," + res + "}"


    ##################################
    #
    def set_json_translation(self, str):
        self.translations = []

        if (", plural," in str) != self.numerus:
            if self.numerus:
                raise Error(f"plurulal json string for non numerus message: {self.source} @ {self.comment} translation = '{str}'")
            else:
                raise Error(f"non plurulal json string for numerus message: {self.source} @ {self.comment} translation = '{str}'")

        if not self.numerus:
            self.translations.append(str)

        else:
            forms = self._parse_icu_plurals(str)

            one = forms["one"]
            other = forms["other"]
            few   = forms.get("few", other)
            many  = forms.get("many", other)

            self.translations.append(one)
            self.translations.append(few)
            self.translations.append(many)
            self.translations.append(other)


    ##############################
    #
    def _parse_icu_plurals(self, str):
        res = {}
        level = 0
        key = ""

        b = str.index("plural,") + len("plural,")
        for i in range(b, len(str)):
            c= str[i]

            if c == '{':
                level += 1
                if level == 1:
                    key = str[b:i].strip()
                    b = i + 1

            if c == '}':
                level -= 1
                if level == 0:
                    res[key] = str[b:i]
                    b = i + 1

        return res

##################################
#
class TsFile:
    ##############################
    #
    def __init__(self):
        self.messages = []

    ##############################
    #
    def load(self, ts_file, is_source = True):
        xml = minidom.parse(ts_file)
        ts = xml.getElementsByTagName("TS")[0]
        for xmlContext in ts.getElementsByTagName("context"):
            for xmlMessage in xmlContext.getElementsByTagName("message"):

                message = TsMessage()
                message.load(xmlMessage)

                if is_source:
                    message.translations = [message.source]

                self.messages.append(message)


    ##############################
    #
    def write(self, ts_file):
        doc = minidom.Document()
        xmlTS = doc.createElement("TS")
        xmlTS.setAttribute("version", "2.1")
        doc.appendChild(xmlTS)

        lang = self.get_lang(ts_file)
        if lang:
            xmlTS.setAttribute("language", self.get_lang(ts_file))


        context_name = None
        xmlContext = None
        for message in self.messages:
            if context_name != message.context:
                context_name = message.context

                xmlContext = doc.createElement("context")
                xmlTS.appendChild(xmlContext)
                xmlName = doc.createElement("name")
                xmlName.appendChild(doc.createTextNode(message.context))
                xmlContext.appendChild(xmlName)

            message.write(xmlContext)

        text = doc.toprettyxml(indent="    ")
        text = "\n".join(text.split("\n")[1:])

        with open(ts_file, 'w') as w:
            w.write('<?xml version="1.0" ?>\n<!DOCTYPE TS>\n')
            w.write(text)


    ##################################
    #
    @staticmethod
    def get_lang(file_path):
        file_name = os.path.basename(file_path)
        start = file_name.find("_") + 1
        end = file_name.find(".")
        return file_name[start:end]



##################################
#
def push_source():
    args = [
        "tx",
        "push",
        "--source",
    ]

    subprocess.run(args)


##################################
#
def push_lang(lang):
    args = [
        "tx",
        "push",
        "--translation",
        "--languages",
        lang,
    ]

    subprocess.run(args)


##################################
#
def pull_source():
    args = [
        "tx",
        "pull",
        "--source",
    ]

    subprocess.run(args)

##################################
#
def pull_translations():
    args = [
        "tx",
        "pull",
        "--all",
        "--force",
    ]

    subprocess.run(args)

##################################
# imported from https://rest.api.transifex.com/languages
#   doc: https://developers.transifex.com/reference/get_languages
LANGS = {
    'aa_DJ':             ['one', 'other'],
    'ab':                ['one', 'other'],
    'ace':               ['other'],
    'ach':               ['one', 'other'],
    'ady':               ['one', 'other'],
    'aeb':               ['zero', 'one', 'two', 'many', 'few', 'other'],
    'af':                ['one', 'other'],
    'af_ZA':             ['one', 'other'],
    'ak':                ['one', 'other'],
    'aln':               ['one', 'other'],
    'am':                ['one', 'other'],
    'am_ET':             ['one', 'other'],
    'an':                ['one', 'other'],
    'ang':               ['one', 'other'],
    'ar':                ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_AA':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_AE':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'arb':               ['zero', 'one', 'two', 'many', 'few', 'other'],
    'arc':               ['one', 'other'],
    'ar_DZ':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_EG':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_IQ':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_JO':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_LB':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'arn':               ['one', 'other'],
    'ar_QA':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_SA':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_SD':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ar_SY':             ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ary':               ['zero', 'one', 'two', 'many', 'few', 'other'],
    'as':                ['one', 'other'],
    'as_IN':             ['one', 'other'],
    'ast':               ['one', 'other'],
    'ast_ES':            ['one', 'other'],
    'ay':                ['one', 'other'],
    'ayl':               ['zero', 'one', 'two', 'many', 'few', 'other'],
    'az':                ['one', 'other'],
    'az@Arab':           ['one', 'other'],
    'az_AZ':             ['one', 'other'],
    'az_IR':             ['one', 'other'],
    'az@latin':          ['one', 'other'],
    'ba':                ['other'],
    'bal':               ['one', 'other'],
    'bar':               ['one', 'other'],
    'bcq':               ['one', 'other'],
    'be':                ['one', 'many', 'few', 'other'],
    'be_BY':             ['one', 'many', 'few', 'other'],
    'bem':               ['one', 'other'],
    'bem_ZM':            ['one', 'other'],
    'ber':               ['one', 'other'],
    'be@tarask':         ['one', 'many', 'few', 'other'],
    'bg':                ['one', 'other'],
    'bg_BG':             ['one', 'other'],
    'bg@Cyrl':           ['one', 'other'],
    'bg_US':             ['one', 'other'],
    'bho':               ['one', 'other'],
    'bi':                ['one', 'other'],
    'bm':                ['other'],
    'bn':                ['one', 'other'],
    'bn_BD':             ['one', 'other'],
    'bn_IN':             ['one', 'other'],
    'bo':                ['other'],
    'bo_CN':             ['other'],
    'bqi':               ['one', 'other'],
    'bqi_IR':            ['one', 'other'],
    'br':                ['one', 'two', 'many', 'few', 'other'],
    'br_FR':             ['one', 'two', 'many', 'few', 'other'],
    'brx':               ['one', 'other'],
    'bs':                ['one', 'few', 'other'],
    'bs_BA':             ['one', 'few', 'other'],
    'bs_BA-SRP':         ['one', 'few', 'other'],
    'bua':               ['one', 'other'],
    'ca':                ['one', 'other'],
    'ca_ES':             ['one', 'other'],
    'cak':               ['one', 'other'],
    'ca@valencia':       ['one', 'other'],
    'cdo':               ['other'],
    'ce':                ['one', 'other'],
    'ceb':               ['one', 'other'],
    'cgg':               ['one', 'other'],
    'ch':                ['one', 'other'],
    'chk':               ['one', 'other'],
    'chr':               ['one', 'other'],
    'cjy':               ['other'],
    'ckb':               ['one', 'other'],
    'cmn':               ['other'],
    'cnr':               ['one', 'few', 'other'],
    'cnx':               ['zero', 'one', 'two', 'many', 'few', 'other'],
    'co':                ['one', 'other'],
    'cpx':               ['other'],
    'crh':               ['one', 'other'],
    'cs':                ['one', 'many', 'few', 'other'],
    'csb':               ['one', 'few', 'other'],
    'cs_CZ':             ['one', 'many', 'few', 'other'],
    'cs@qtfiletype':     ['one', 'few', 'other'],
    'cv':                ['one', 'other'],
    'cy':                ['one', 'two', 'many', 'other'],
    'cy_GB':             ['one', 'two', 'many', 'other'],
    'czh':               ['other'],
    'czo':               ['other'],
    'da':                ['one', 'other'],
    'da_DK':             ['one', 'other'],
    'de':                ['one', 'other'],
    'de_AT':             ['one', 'other'],
    'de_BE':             ['one', 'other'],
    'de_CH':             ['one', 'other'],
    'de_DE':             ['one', 'other'],
    'de_LU':             ['one', 'other'],
    'doi':               ['one', 'other'],
    'dsb':               ['one', 'two', 'few', 'other'],
    'dtp':               ['one', 'other'],
    'dv':                ['one', 'other'],
    'dz':                ['other'],
    'dz_BT':             ['other'],
    'ee':                ['one', 'other'],
    'el':                ['one', 'other'],
    'el_CY':             ['one', 'other'],
    'el_DE':             ['one', 'other'],
    'el_GR':             ['one', 'other'],
    'en':                ['one', 'other'],
    'en_150':            ['one', 'other'],
    'en_AE':             ['one', 'other'],
    'en_AL':             ['one', 'other'],
    'en_AO':             ['one', 'other'],
    'en_AR':             ['one', 'other'],
    'en_AR-B':           ['one', 'other'],
    'en_AR-C':           ['one', 'other'],
    'en_AT':             ['one', 'other'],
    'en_AU':             ['one', 'other'],
    'en_BA':             ['one', 'other'],
    'en_BA-SRP':         ['one', 'other'],
    'en_BB':             ['one', 'other'],
    'en_BD':             ['one', 'other'],
    'en_BE':             ['one', 'other'],
    'en_BF':             ['one', 'other'],
    'en_BG':             ['one', 'other'],
    'en_BH':             ['one', 'other'],
    'en_BJ':             ['one', 'other'],
    'en_BM':             ['one', 'other'],
    'en_BN':             ['one', 'other'],
    'en_BO':             ['one', 'other'],
    'en_BR':             ['one', 'other'],
    'en_BS':             ['one', 'other'],
    'en_BW':             ['one', 'other'],
    'en_CA':             ['one', 'other'],
    'en_CD':             ['one', 'other'],
    'en_CG':             ['one', 'other'],
    'en_CH':             ['one', 'other'],
    'en_CI':             ['one', 'other'],
    'en_CL':             ['one', 'other'],
    'en_CM':             ['one', 'other'],
    'en_CO':             ['one', 'other'],
    'en_CR':             ['one', 'other'],
    'en_CW':             ['one', 'other'],
    'en_CY':             ['one', 'other'],
    'en_CZ':             ['one', 'other'],
    'en_DE':             ['one', 'other'],
    'en_DK':             ['one', 'other'],
    'en_DO':             ['one', 'other'],
    'en_DZ':             ['one', 'other'],
    'en_EC':             ['one', 'other'],
    'en_ee':             ['one', 'other'],
    'en_EG':             ['one', 'other'],
    'en_ES':             ['one', 'other'],
    'en_FI':             ['one', 'other'],
    'en_FJ':             ['one', 'other'],
    'en_FR':             ['one', 'other'],
    'en_GA':             ['one', 'other'],
    'en_GB':             ['one', 'other'],
    'en_GF':             ['one', 'other'],
    'en_GH':             ['one', 'other'],
    'en_GM':             ['one', 'other'],
    'en_GN':             ['one', 'other'],
    'en_GQ':             ['one', 'other'],
    'en_GR':             ['one', 'other'],
    'en_GT':             ['one', 'other'],
    'en_HK':             ['one', 'other'],
    'en_HN':             ['one', 'other'],
    'en_HR':             ['one', 'other'],
    'en_HT':             ['one', 'other'],
    'en_HU':             ['one', 'other'],
    'en_ID':             ['one', 'other'],
    'en_IE':             ['one', 'other'],
    'en_IL':             ['one', 'other'],
    'en_IN':             ['one', 'other'],
    'en_IQ':             ['one', 'other'],
    'en_IS':             ['one', 'other'],
    'en_IT':             ['one', 'other'],
    'en_JM':             ['one', 'other'],
    'en_JO':             ['one', 'other'],
    'en_JP':             ['one', 'other'],
    'en_KE':             ['one', 'other'],
    'en_KR':             ['one', 'other'],
    'en_KW':             ['one', 'other'],
    'en_KY':             ['one', 'other'],
    'en_LA':             ['one', 'other'],
    'en_LB':             ['one', 'other'],
    'en_LK':             ['one', 'other'],
    'en@lolcat':         ['one', 'other'],
    'en_LR':             ['one', 'other'],
    'en_LS':             ['one', 'other'],
    'en_lt':             ['one', 'other'],
    'en_LU':             ['one', 'other'],
    'en_lv':             ['one', 'other'],
    'en_MA':             ['one', 'other'],
    'en_MG':             ['one', 'other'],
    'en_MK':             ['one', 'other'],
    'en_ML':             ['one', 'other'],
    'en_MM':             ['one', 'other'],
    'en_MO':             ['one', 'other'],
    'en_MT':             ['one', 'other'],
    'en_MU':             ['one', 'other'],
    'en_MW':             ['one', 'other'],
    'en_MX':             ['one', 'other'],
    'en_MY':             ['one', 'other'],
    'en_MZ':             ['one', 'other'],
    'en_NA':             ['one', 'other'],
    'en_NE':             ['one', 'other'],
    'en_NG':             ['one', 'other'],
    'en_NI':             ['one', 'other'],
    'en_NL':             ['one', 'other'],
    'en_NO':             ['one', 'other'],
    'en_NP':             ['one', 'other'],
    'en_NZ':             ['one', 'other'],
    'en_OM':             ['one', 'other'],
    'en@Ontario':        ['one', 'other'],
    'en_PE':             ['one', 'other'],
    'en_PG':             ['one', 'other'],
    'en_PH':             ['one', 'other'],
    'en@pirate':         ['one', 'other'],
    'en_PK':             ['one', 'other'],
    'en_PL':             ['one', 'other'],
    'en_PR':             ['one', 'other'],
    'en_PT':             ['one', 'other'],
    'en_PY':             ['one', 'other'],
    'en_QA':             ['one', 'other'],
    'en_RO':             ['one', 'other'],
    'en_RS':             ['one', 'other'],
    'en_SA':             ['one', 'other'],
    'en_SE':             ['one', 'other'],
    'en_SG':             ['one', 'other'],
    'en@shaw':           ['one', 'other'],
    'en_SI':             ['one', 'other'],
    'en_SK':             ['one', 'other'],
    'en_SN':             ['one', 'other'],
    'en_SV':             ['one', 'other'],
    'en_SZ':             ['one', 'other'],
    'en_TG':             ['one', 'other'],
    'en_TH':             ['one', 'other'],
    'en_TR':             ['one', 'other'],
    'en_TT':             ['one', 'other'],
    'en_TW':             ['one', 'other'],
    'en_TZ':             ['one', 'other'],
    'en_UA':             ['one', 'other'],
    'en_UG':             ['one', 'other'],
    'en_US':             ['one', 'other'],
    'en_UY':             ['one', 'other'],
    'en_VE':             ['one', 'other'],
    'en_VG':             ['one', 'other'],
    'en_VN':             ['one', 'other'],
    'en@ysv':            ['one', 'other'],
    'en_ZA':             ['one', 'other'],
    'en_ZM':             ['one', 'other'],
    'en_ZW':             ['one', 'other'],
    'eo':                ['one', 'other'],
    'es':                ['one', 'many', 'other'],
    'es_150':            ['one', 'many', 'other'],
    'es_419':            ['one', 'many', 'other'],
    'es_AR':             ['one', 'many', 'other'],
    'es_AR-B':           ['one', 'many', 'other'],
    'es_AR-C':           ['one', 'many', 'other'],
    'es_BO':             ['one', 'many', 'other'],
    'es_BR':             ['one', 'many', 'other'],
    'es_CL':             ['one', 'many', 'other'],
    'es_CO':             ['one', 'many', 'other'],
    'es_CR':             ['one', 'many', 'other'],
    'es_CU':             ['one', 'many', 'other'],
    'es_CW':             ['one', 'many', 'other'],
    'es_DO':             ['one', 'many', 'other'],
    'es_EC':             ['one', 'many', 'other'],
    'es_ES':             ['one', 'many', 'other'],
    'es_GQ':             ['one', 'many', 'other'],
    'es_GT':             ['one', 'many', 'other'],
    'es_HN':             ['one', 'many', 'other'],
    'es_MX':             ['one', 'many', 'other'],
    'es_NI':             ['one', 'many', 'other'],
    'es_PA':             ['one', 'many', 'other'],
    'es_PE':             ['one', 'many', 'other'],
    'es_PR':             ['one', 'many', 'other'],
    'es_PY':             ['one', 'many', 'other'],
    'es@qtfiletype':     ['one', 'other'],
    'es_SA':             ['one', 'many', 'other'],
    'es_SV':             ['one', 'many', 'other'],
    'es_US':             ['one', 'many', 'other'],
    'es_UY':             ['one', 'many', 'other'],
    'es_VE':             ['one', 'many', 'other'],
    'es_VG':             ['one', 'many', 'other'],
    'et':                ['one', 'other'],
    'et_EE':             ['one', 'other'],
    'eu':                ['one', 'other'],
    'eu_ES':             ['one', 'other'],
    'fa':                ['one', 'other'],
    'fa_AF':             ['one', 'other'],
    'fa_IR':             ['one', 'other'],
    'ff':                ['one', 'other'],
    'ff_SN':             ['one', 'other'],
    'fi':                ['one', 'other'],
    'fi_FI':             ['one', 'other'],
    'fil':               ['one', 'other'],
    'fil_PH':            ['one', 'other'],
    'fo':                ['one', 'other'],
    'fo_FO':             ['one', 'other'],
    'fr':                ['one', 'many', 'other'],
    'fr_150':            ['one', 'many', 'other'],
    'fr_BE':             ['one', 'many', 'other'],
    'fr_BF':             ['one', 'many', 'other'],
    'fr_BJ':             ['one', 'many', 'other'],
    'fr_CA':             ['one', 'many', 'other'],
    'fr_CD':             ['one', 'many', 'other'],
    'fr_CG':             ['one', 'many', 'other'],
    'fr_CH':             ['one', 'many', 'other'],
    'fr_CI':             ['one', 'many', 'other'],
    'fr_CM':             ['one', 'many', 'other'],
    'fr_DZ':             ['one', 'many', 'other'],
    'fr_FR':             ['one', 'many', 'other'],
    'fr_GA':             ['one', 'many', 'other'],
    'fr_GF':             ['one', 'many', 'other'],
    'fr_GN':             ['one', 'many', 'other'],
    'fr_GP':             ['one', 'many', 'other'],
    'fr_HT':             ['one', 'many', 'other'],
    'fr_LR':             ['one', 'many', 'other'],
    'fr_LU':             ['one', 'many', 'other'],
    'fr_MA':             ['one', 'many', 'other'],
    'fr_MG':             ['one', 'many', 'other'],
    'fr_ML':             ['one', 'many', 'other'],
    'fr_MQ':             ['one', 'many', 'other'],
    'fr_NE':             ['one', 'many', 'other'],
    'fr@Ontario':        ['one', 'many', 'other'],
    'frp':               ['one', 'other'],
    'fr@qtfiletype':     ['one', 'other'],
    'fr_RE':             ['one', 'many', 'other'],
    'fr_SN':             ['one', 'many', 'other'],
    'fr_TG':             ['one', 'many', 'other'],
    'fur':               ['one', 'other'],
    'fy':                ['one', 'other'],
    'fy_NL':             ['one', 'other'],
    'ga':                ['one', 'two', 'many', 'few', 'other'],
    'ga_IE':             ['one', 'two', 'many', 'few', 'other'],
    'gan':               ['other'],
    'gd':                ['one', 'two', 'few', 'other'],
    'gl':                ['one', 'other'],
    'gl_ES':             ['one', 'other'],
    'gos':               ['one', 'other'],
    'grc':               ['one', 'other'],
    'grt':               ['one', 'other'],
    'gsw':               ['one', 'other'],
    'gu':                ['one', 'other'],
    'guc':               ['one', 'other'],
    'gug_PY':            ['one', 'other'],
    'gu_IN':             ['one', 'other'],
    'gum':               ['one', 'other'],
    'gun':               ['one', 'other'],
    'ha':                ['one', 'other'],
    'hak':               ['other'],
    'haw':               ['one', 'other'],
    'he':                ['one', 'two', 'other'],
    'he@female':         ['one', 'two', 'other'],
    'he_IL':             ['one', 'two', 'other'],
    'he@male':           ['one', 'two', 'other'],
    'hi':                ['one', 'other'],
    'hi_IN':             ['one', 'other'],
    'hne':               ['one', 'other'],
    'hr':                ['one', 'few', 'other'],
    'hr_BA':             ['one', 'few', 'other'],
    'hr_BA-SRP':         ['one', 'few', 'other'],
    'hr_HR':             ['one', 'few', 'other'],
    'hrx':               ['one', 'other'],
    'hsb':               ['one', 'two', 'few', 'other'],
    'hsn':               ['other'],
    'ht':                ['one', 'other'],
    'ht_HT':             ['one', 'other'],
    'hu':                ['one', 'other'],
    'hu_HU':             ['one', 'other'],
    'hu_RO':             ['one', 'other'],
    'hu_SK':             ['one', 'other'],
    'hy':                ['one', 'other'],
    'hy_AM':             ['one', 'other'],
    'hye':               ['one', 'other'],
    'hye_RU':            ['one', 'other'],
    'hy_RU':             ['one', 'other'],
    'ia':                ['one', 'other'],
    'id':                ['other'],
    'id_ID':             ['other'],
    'ie':                ['one', 'other'],
    'ig':                ['other'],
    'ilo':               ['one', 'other'],
    'io':                ['one', 'other'],
    'is':                ['one', 'other'],
    'is_IS':             ['one', 'other'],
    'it':                ['one', 'many', 'other'],
    'it_CH':             ['one', 'many', 'other'],
    'it_IT':             ['one', 'many', 'other'],
    'it@qtfiletype':     ['one', 'other'],
    'iu':                ['one', 'two', 'other'],
    'ja':                ['other'],
    'ja-Hira':           ['other'],
    'ja_JP':             ['other'],
    'jam':               ['one', 'other'],
    'jbo':               ['other'],
    'jv':                ['other'],
    'ka':                ['one', 'other'],
    'kaa':               ['one', 'other'],
    'kab':               ['one', 'other'],
    'ka_GE':             ['one', 'other'],
    'kbd':               ['other'],
    'kha':               ['one', 'other'],
    'ki':                ['one', 'other'],
    'kk':                ['one', 'other'],
    'kk@Arab':           ['one', 'other'],
    'kk@Cyrl':           ['one', 'other'],
    'kk_KZ':             ['one', 'other'],
    'kk@latin':          ['one', 'other'],
    'kl':                ['one', 'other'],
    'km':                ['other'],
    'km_KH':             ['other'],
    'kmr':               ['one', 'other'],
    'kn':                ['one', 'other'],
    'kn_IN':             ['one', 'other'],
    'ko':                ['other'],
    'kog':               ['one', 'other'],
    'kok':               ['one', 'other'],
    'ko_KR':             ['other'],
    'kqn':               ['one', 'other'],
    'krc':               ['one', 'other'],
    'krl':               ['one', 'other'],
    'ks':                ['one', 'other'],
    'ksh':               ['zero', 'one', 'other'],
    'ks_IN':             ['one', 'other'],
    'ku':                ['one', 'other'],
    'ku_IQ':             ['one', 'other'],
    'kw':                ['zero', 'one', 'two', 'many', 'few', 'other'],
    'ky':                ['other'],
    'ky@Arab':           ['other'],
    'la':                ['one', 'other'],
    'lad':               ['one', 'other'],
    'lb':                ['one', 'other'],
    'lez':               ['one', 'other'],
    'lfn':               ['one', 'other'],
    'lg':                ['one', 'other'],
    'lg_UG':             ['one', 'other'],
    'li':                ['one', 'other'],
    'lij':               ['one', 'other'],
    'lkt':               ['other'],
    'lmo':               ['zero', 'one', 'other'],
    'ln':                ['one', 'other'],
    'lo':                ['other'],
    'lo_LA':             ['other'],
    'loz_ZM':            ['one', 'other'],
    'lrc':               ['one', 'other'],
    'lt':                ['one', 'many', 'few', 'other'],
    'ltg':               ['zero', 'one', 'other'],
    'lt_LT':             ['one', 'many', 'few', 'other'],
    'lue':               ['one', 'other'],
    'lus':               ['one', 'other'],
    'luz':               ['one', 'other'],
    'lv':                ['zero', 'one', 'other'],
    'lv_LV':             ['zero', 'one', 'other'],
    'lzh':               ['other'],
    'mai':               ['one', 'other'],
    'mfe':               ['one', 'other'],
    'mg':                ['one', 'other'],
    'mh':                ['one', 'other'],
    'mhr':               ['one', 'other'],
    'mhr_RU':            ['one', 'other'],
    'mi':                ['one', 'other'],
    'min':               ['one', 'other'],
    'mk':                ['one', 'other'],
    'mk_MK':             ['one', 'other'],
    'ml':                ['one', 'other'],
    'ml_IN':             ['one', 'other'],
    'mn':                ['one', 'other'],
    'mni':               ['one', 'other'],
    'mn_MN':             ['one', 'other'],
    'mnp':               ['other'],
    'mr':                ['one', 'other'],
    'mr_IN':             ['one', 'other'],
    'ms':                ['other'],
    'ms@Arab':           ['other'],
    'ms_BN':             ['other'],
    'ms_MY':             ['other'],
    'mt':                ['one', 'many', 'few', 'other'],
    'mt_MT':             ['one', 'many', 'few', 'other'],
    'mus':               ['one', 'other'],
    'mw1':               ['one', 'other'],
    'mxp':               ['one', 'other'],
    'my':                ['other'],
    'my_MM':             ['other'],
    'myv':               ['one', 'other'],
    'nah':               ['one', 'other'],
    'nan':               ['other'],
    'nap':               ['one', 'other'],
    'nb':                ['one', 'other'],
    'nb_NO':             ['one', 'other'],
    'nd':                ['one', 'other'],
    'ndc':               ['one', 'other'],
    'nds':               ['one', 'other'],
    'ne':                ['one', 'other'],
    'ne_NP':             ['one', 'other'],
    'nia':               ['one', 'other'],
    'nl':                ['one', 'other'],
    'nl_BE':             ['one', 'other'],
    'nl_CW':             ['one', 'other'],
    'nl_NL':             ['one', 'other'],
    'nn':                ['one', 'other'],
    'nn_NO':             ['one', 'other'],
    'no':                ['one', 'other'],
    'no_NO':             ['one', 'other'],
    'nqo':               ['other'],
    'nr':                ['one', 'other'],
    'nso':               ['one', 'other'],
    'nv':                ['one', 'other'],
    'ny':                ['one', 'other'],
    'ny_MW':             ['one', 'other'],
    'oc':                ['one', 'other'],
    'oc-aranes':         ['one', 'other'],
    'oc-auvern':         ['one', 'other'],
    'oc-cisaup':         ['one', 'other'],
    'oc-gascon':         ['one', 'other'],
    'oc-lemosin':        ['one', 'other'],
    'oc-lengadoc':       ['one', 'other'],
    'oc-nicard':         ['one', 'other'],
    'oc-provenc':        ['one', 'other'],
    'oc-vivaraup':       ['one', 'other'],
    'om':                ['one', 'other'],
    'or':                ['one', 'other'],
    'or_IN':             ['one', 'other'],
    'os':                ['one', 'other'],
    'pa':                ['one', 'other'],
    'pa_IN':             ['one', 'other'],
    'pam':               ['one', 'other'],
    'pap':               ['one', 'other'],
    'pap_AW':            ['one', 'other'],
    'pap_CW':            ['one', 'other'],
    'pa_PK':             ['one', 'other'],
    'pbb':               ['one', 'other'],
    'pcm':               ['one', 'other'],
    'pfl':               ['one', 'other'],
    'pl':                ['one', 'many', 'few', 'other'],
    'pl_PL':             ['one', 'many', 'few', 'other'],
    'pl@qtfiletype':     ['one', 'few', 'other'],
    'pms':               ['one', 'other'],
    'ps':                ['one', 'other'],
    'ps_AF':             ['one', 'other'],
    'pt':                ['one', 'many', 'other'],
    'pt_AO':             ['one', 'many', 'other'],
    'pt_BR':             ['one', 'many', 'other'],
    'pt_CL':             ['one', 'many', 'other'],
    'pt_CO':             ['one', 'many', 'other'],
    'pt_MX':             ['one', 'many', 'other'],
    'pt_MZ':             ['one', 'many', 'other'],
    'pt_PE':             ['one', 'many', 'other'],
    'pt_PT':             ['one', 'many', 'other'],
    'pt@qtfiletype':     ['one', 'other'],
    'qji':               ['one', 'other'],
    'qu':                ['one', 'other'],
    'quc':               ['one', 'other'],
    'qu_EC':             ['one', 'other'],
    'quh':               ['one', 'other'],
    'rap':               ['one', 'other'],
    'rif':               ['one', 'other'],
    'rm':                ['one', 'other'],
    'rn':                ['one', 'other'],
    'ro':                ['one', 'few', 'other'],
    'ro_MD':             ['one', 'few', 'other'],
    'ro_RO':             ['one', 'few', 'other'],
    'ru':                ['one', 'many', 'few', 'other'],
    'rue':               ['one', 'other'],
    'ru_ee':             ['one', 'many', 'few', 'other'],
    'ru_KZ':             ['one', 'many', 'few', 'other'],
    'ru_lt':             ['one', 'many', 'few', 'other'],
    'ru_lv':             ['one', 'many', 'few', 'other'],
    'ru@petr1708':       ['one', 'many', 'few', 'other'],
    'ru@qtfiletype':     ['one', 'few', 'other'],
    'ru_RU':             ['one', 'many', 'few', 'other'],
    'ru_UA':             ['one', 'many', 'few', 'other'],
    'rw':                ['one', 'other'],
    'sa':                ['one', 'two', 'other'],
    'sah':               ['other'],
    'saq':               ['one', 'other'],
    'sat':               ['one', 'two', 'other'],
    'sc':                ['one', 'other'],
    'scn':               ['one', 'other'],
    'sco':               ['one', 'other'],
    'sd':                ['one', 'other'],
    'sdh':               ['one', 'other'],
    'se':                ['one', 'two', 'other'],
    'sg':                ['other'],
    'shi':               ['one', 'few', 'other'],
    'shy':               ['one', 'other'],
    'si':                ['one', 'other'],
    'sid':               ['one', 'other'],
    'si_LK':             ['one', 'other'],
    'sk':                ['one', 'many', 'few', 'other'],
    'skr':               ['one', 'other'],
    'sk_SK':             ['one', 'many', 'few', 'other'],
    'sl':                ['one', 'two', 'few', 'other'],
    'sl_SI':             ['one', 'two', 'few', 'other'],
    'sm':                ['other'],
    'sma':               ['one', 'two', 'other'],
    'sn':                ['one', 'other'],
    'so':                ['one', 'other'],
    'son':               ['other'],
    'sq':                ['one', 'other'],
    'sq_AL':             ['one', 'other'],
    'sr':                ['one', 'few', 'other'],
    'sr_BA@latin':       ['one', 'few', 'other'],
    'sr_BA-SRP':         ['one', 'few', 'other'],
    'sr@Cyrl':           ['one', 'few', 'other'],
    'sr@Ijekavian':      ['one', 'few', 'other'],
    'sr@ijekavianlatin': ['one', 'few', 'other'],
    'sr@latin':          ['one', 'few', 'other'],
    'sr_ME':             ['one', 'few', 'other'],
    'sr_ME@latin':       ['one', 'few', 'other'],
    'sr_RS':             ['one', 'few', 'other'],
    'sr_RS@latin':       ['one', 'few', 'other'],
    'ss':                ['one', 'other'],
    'st':                ['one', 'other'],
    'st_ZA':             ['one', 'other'],
    'su':                ['other'],
    'sv':                ['one', 'other'],
    'sv_FI':             ['one', 'other'],
    'sv_SE':             ['one', 'other'],
    'sw':                ['one', 'other'],
    'sw_CD':             ['one', 'other'],
    'swg':               ['one', 'other'],
    'sw_KE':             ['one', 'other'],
    'sw_TZ':             ['one', 'other'],
    'sw_UG':             ['one', 'other'],
    'szl':               ['one', 'few', 'other'],
    'ta':                ['one', 'other'],
    'ta_IN':             ['one', 'other'],
    'ta_LK':             ['one', 'other'],
    'te':                ['one', 'other'],
    'te_IN':             ['one', 'other'],
    'tet':               ['other'],
    'tg':                ['one', 'other'],
    'tg_TJ':             ['one', 'other'],
    'th':                ['other'],
    'th_TH':             ['other'],
    'ti':                ['one', 'other'],
    'tk':                ['one', 'other'],
    'tk_TM':             ['one', 'other'],
    'tl':                ['one', 'other'],
    'tlh':               ['one', 'other'],
    'tl_PH':             ['one', 'other'],
    'tn':                ['one', 'other'],
    'to':                ['other'],
    'tok':               ['other'],
    'tpi':               ['one', 'other'],
    'tr':                ['one', 'other'],
    'tr_CY':             ['one', 'other'],
    'tr_DE':             ['one', 'other'],
    'tr_TR':             ['one', 'other'],
    'ts':                ['one', 'other'],
    'tsi':               ['one', 'other'],
    'tt':                ['other'],
    'tum':               ['one', 'other'],
    'tuv':               ['one', 'other'],
    'twd':               ['one', 'other'],
    'tzj':               ['one', 'other'],
    'tzl':               ['one', 'other'],
    'tzm':               ['one', 'other'],
    'udm':               ['other'],
    'ug':                ['one', 'other'],
    'ug@Arab':           ['other'],
    'ug@Cyrl':           ['other'],
    'ug@Latin':          ['other'],
    'uk':                ['one', 'many', 'few', 'other'],
    'uk_UA':             ['one', 'many', 'few', 'other'],
    'ur':                ['one', 'other'],
    'ur_PK':             ['one', 'other'],
    'uz':                ['other'],
    'uz@Arab':           ['other'],
    'uz@Cyrl':           ['other'],
    'uz@Latn':           ['other'],
    'uz_UZ':             ['other'],
    've':                ['one', 'other'],
    'vec':               ['one', 'other'],
    'vep':               ['one', 'other'],
    'vi':                ['other'],
    'vi_VN':             ['other'],
    'vls':               ['one', 'other'],
    'vmf':               ['zero', 'one', 'other'],
    'vo':                ['one', 'other'],
    'wa':                ['one', 'other'],
    'war':               ['one', 'other'],
    'wo':                ['other'],
    'wo_SN':             ['other'],
    'wuu':               ['other'],
    'wuu-Hans':          ['other'],
    'wuu-Hant':          ['other'],
    'xcl':               ['one', 'other'],
    'xh':                ['one', 'other'],
    'yi':                ['one', 'other'],
    'yo':                ['other'],
    'yue':               ['other'],
    'yue_CN':            ['other'],
    'zgh':               ['one', 'other'],
    'zh':                ['other'],
    'zh_CN':             ['other'],
    'zh_CN.GB2312':      ['other'],
    'zh-Hans':           ['other'],
    'zh-Hant':           ['other'],
    'zh_HK':             ['other'],
    'zh_MO':             ['other'],
    'zh_SG':             ['other'],
    'zh_TW':             ['other'],
    'zh_TW.Big5':        ['other'],
    'zu':                ['one', 'other'],
    'zu_ZA':             ['one', 'other'],
    'zza':               ['one', 'other'],
}
