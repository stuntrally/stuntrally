#!/bin/sh

#  To run, you also need here in locale/
#  tx  - is the downloaded Transifex client (cli)

echo --- Generating new template .pot
echo

#../build/sr-translator
./sr-translator

echo
echo --- Uploading new template

./tx push -s

echo
echo --- Pulling .po translations back

./tx pull -a --minimum-perc=10

echo
echo --- Converting .po to .xml

./xml_po_all.py

echo
echo "Press Enter to end ..."
read REPLY
#sleep 5
echo End.
