#!/bin/bash -e

echo "Generating new template..."
./xml_po_parser.py ../data/gui/core_language_en_tag.xml ./stuntrally.pot

echo "Fetching new translations..."
(
	# Clone the LP translation branch if it doesn't exist,
	# update otherwise
	if [ ! -d ./translations-export ]; then
		mkdir ./translations-export
		cd ./translations-export
		bzr branch lp:~stuntrally-team/stuntrally/pofiles
	else
		cd ./translations-export/pofiles
		bzr pull
	fi
)

echo "Generating languages..."
LOCALES="de fi ro pl"
for loc in $LOCALES; do
	./xml_po_parser.py ./translations-export/pofiles/locale/${loc}.po ../data/gui/core_language_${loc}_tag.xml
done

echo "Done."
