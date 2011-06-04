#!/bin/bash

echo "Generating new template..."
./xml_po_parser.py ../data/gui/core_language_en_tag.xml ./stuntrally.pot

echo "Fetching new translations..."

if [ ! -d ./translations-export ]; then
	mkdir ./translations-export
	cd ./translations-export
	bzr branch lp:~stuntrally-team/stuntrally/pofiles
	cd ..
fi

cd ./translations-export/pofiles
bzr pull
cd ../..

echo "Generating languages..."
./xml_po_parser.py ./translations-export/pofiles/locale/de.po ../data/gui/core_language_de_tag.xml
./xml_po_parser.py ./translations-export/pofiles/locale/fi.po ../data/gui/core_language_fi_tag.xml

echo "Done"
