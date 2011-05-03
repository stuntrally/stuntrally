#!/bin/bash

echo "Generating new template..."
./xml_po_parser.py ../data/gui/core_language_english_tag.xml ./stuntrally.pot

echo "Updating languages..."
./xml_po_parser.py ./translations-export/locale/de.po ../data/gui/core_language_german_tag.xml
./xml_po_parser.py ./translations-export/locale/fi.po ../data/gui/core_language_finnish_tag.xml

echo "Done"
