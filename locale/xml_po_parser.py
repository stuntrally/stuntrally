#!/usr/bin/python3

import sys, time, os, io

fallback_file = "core_language_en_tag.xml"

header = "# SOME DESCRIPTIVE TITLE.\n\
# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER\n\
# This file is distributed under the same license as the PACKAGE package.\n\
# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n\
#\n\
#, fuzzy\n\
msgid \"\"\n\
msgstr \"\"\n\
\"Project-Id-Version: PACKAGE VERSION\"\n\
\"Report-Msgid-Bugs-To: \"\n\
\"POT-Creation-Date: " +  time.strftime("%Y-%m-%d %H:%M%z") + "\"\n\
\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\"\n\
\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\"\n\
\"Language-Team: LANGUAGE <LL@li.org>\"\n\
\"Language: \"\n\
\"MIME-Version: 1.0\"\n\
\"Content-Type: text/plain; charset=CHARSET\"\n\
\"Content-Transfer-Encoding: 8bit\"\n"

ignore_tags = ["GameVersion", "PageURL"]

def usage():
	print("Usage: " + sys.argv[0] + " someFile.xml someFile.pot")
	print("Usage: " + sys.argv[0] + " someFile.po someFile.xml")

if (len(sys.argv)) != 3:
	usage()
	sys.exit(0)

file1 = sys.argv[1]
file2 = sys.argv[2]
#print(file1)
#print(file2)

f1 = io.open(file1, 'r', encoding="utf8")
f2 = io.open(file2, 'w', encoding="utf8")

msgs = {}
# msgs = {"foo":"bar", "asdf":"ghij"}

if file1.endswith(".xml") and file2.endswith(".pot"):
	#xml2po
	
	# read xml
	for line in f1:
		if line.strip().startswith("<Tag name="):
			msgid = line.split("=\"")[1].split("\"")[0]
			msgstr = line.split(">")[1].split("<")[0]
			# ignored
			if not(msgid[0:5] == 'LANG_' or msgid in ignore_tags):
				msgs[msgid] = msgstr
			
	# write pot
	
	# header
	result = header + "\n"
	
	# translations
	for mid, mstr in msgs.items():
		#result += "# " + mid + "\n"
		result += "msgctxt \"" + mid + "\"\n"
		result += "msgid \"" + mstr + "\"\n"
		result += "msgstr \"\"\n"
		result += "\n"
		
	f2.write(result)
	
elif file1.endswith(".po") and file2.endswith(".xml"):
	#po2xml
	
	msgid = ""
	msgstr = ""
	last = "msgstr"
	# read po
	for line in f1:
		#print(line)
		if line.strip().startswith("\""):
			if last == "msgctxt":
				#msgid += line.strip().split("\"")[1].replace("\\n", "\n")
				pass
			elif last == "msgstr":
				msgstr += line.strip().split("\"")[1].replace("\\n", "\n")
		if line.strip().startswith("msgctxt"):
			# add previous msgstr
			if msgid != "":
				msgs[msgid] = msgstr
			
			msgid = line.split("\"")[1].replace("\\n", "\n")
			last = "msgctxt"
		if line.strip().startswith("msgstr"):
			msgstr = line.split("\"")[1].replace("\\n", "\n")
			last = "msgstr"
	# add last line
	msgs[msgid] = msgstr
			
	# write xml
	result = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<MyGUI>\n"

	# read english xml for fallback when untranslated
	msgs_f = {}
	f_f = io.open(os.path.join(os.path.dirname(file2), fallback_file), encoding="utf8")
	for line in f_f:
		if line.strip().startswith("<Tag name="):
			msgid = line.split("=\"")[1].split("\"")[0]
			msgstr = line.split(">")[1].split("<")[0]
			msgs_f[msgid] = msgstr

	for mid, mstr in msgs.items():
		if mid in msgs_f:
			if mstr.strip() != "" and mid not in ignore_tags:
				result += "\t<Tag name=\"" + mid + "\">" + mstr + "</Tag>\n"
			else:
				# if untranslated or not to be translated (ignore), use english string
				result += "\t<Tag name=\"" + mid + "\">" + msgs_f[mid] + "</Tag>\n"
				
	# put all stuff that is not in .po yet, but is in template, in it's english version
	for mid, mstr in msgs_f.items():
		if not mid in msgs and mid not in ignore_tags:
				result += "\t<Tag name=\"" + mid + "\">" + mstr + "</Tag>\n"
		
	# use english string for ignored
	for ignored in ignore_tags:
		result += "\t<Tag name=\"" + ignored + "\">" + msgs_f[ignored] + "</Tag>\n"
		
	result += "\n</MyGUI>"
	f2.write(result)
		
else:
	usage()

f1.close()
f2.close()
