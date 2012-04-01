#!/bin/bash -e

echo "Generating new template..."
`which python2` ./xml_po_parser.py ../data/gui/core_language_en_tag.xml ./stuntrally.pot

if [ "$1" != "--no-fetch" ]; then
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
fi

echo "Generating languages..."
LOCALES="de fi ro pl fr ru"
for loc in $LOCALES; do
	`which python2` ./xml_po_parser.py ./translations-export/pofiles/locale/${loc}.po ../data/gui/core_language_${loc}_tag.xml
done

echo "Done."
