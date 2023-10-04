#!/usr/bin/env python3
#
MAJOR_RE = r'set\(MAJOR_VERSION\s+(.+)\)'
MINOR_RE = r'set\(MINOR_VERSION\s+(\d+)'
PATCH_RE = r'set\(PATCH_VERSION\s+(\d+)'
BETA_RE  = r'set(BETA_VERSION.\s(.*)\)\s$'


import re
import datetime
import xml.etree.ElementTree as ET
import subprocess

# Get current version of the program
def get_version():
	major_ver = None
	minor_ver = None
	patch_ver = None
	beta_ver  = None

	with open('../CMakeLists.txt') as f:
		n=1
		for line in f:

			result = re.match(MAJOR_RE, line)
			if result:
				major_ver = result.group(1).strip()

			result = re.match(MINOR_RE, line)
			if result:
				minor_ver = result.group(1).strip()

			result = re.match(PATCH_RE, line)
			if result:
				patch_ver = result.group(1).strip()

			result = re.match(PATCH_RE, line)
			if result:
				patch_ver = result.group(1).strip()


			if n > 50:
				break
			n+=1

	if major_ver == None:
		raise Exception("Can't found major version")

	if minor_ver == None:
		raise Exception("Can't found minor version")

	if patch_ver == None:
		raise Exception("Can't found patch version")

	return major_ver, minor_ver, patch_ver, beta_ver


def update_metainfo(file):
	major_ver, minor_ver, patch_ver, beta_ver = get_version()
	if beta_ver:
		return

	ver = "%s.%s.%s" % (major_ver, minor_ver, patch_ver)

	doc = ET.parse(file)
	if doc.find("./releases/release[@version='%s']" % ver):
		return False


	vers = {}
	vers[datetime.date.today().strftime("%Y-%m-%d")] = ver
	for r in doc.findall("./releases/release"):
		vers[r.attrib["date"]] = r.attrib["version"]


	releases = doc.find("./releases")
	releases.clear()
	releases.text = "\n" + (' ' * 4)
	releases.tail = "\n\n" + (' ' * 2)

	for date in sorted(vers.keys(), reverse=True):
		release = ET.SubElement(releases, 'release')
		release.attrib["date"] = date
		release.attrib["version"] = vers[date]
		release.tail = "\n" + (' ' * 4)

		if vers[date] == ver:
			release.text = "\n" + (' ' * 6)


			desc = ET.SubElement(release, "description")
			desc.tail = "\n" + (' ' * 4)

			p = ET.SubElement(desc, "p")
			p.text = "Latest version of Flacon on Flathub."

	release.tail = "\n" + (' ' * 2)

	doc.write(file, encoding='utf-8', xml_declaration=True)


if __name__ == "__main__":
	update_metainfo("com.github.Flacon.metainfo.xml.in")

