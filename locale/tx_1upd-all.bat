@echo off

::  To run, you also need here in locale/
::  sr_transl.exe  - is build from source/transl/main.cpp, proj/sr_transl.vcproj
::  tx.exe  - is the downloaded Transifex client

echo.
echo Generating new template .pot
echo.

sr_transl.exe

::echo.
::echo Uploading new template
echo.

tx push -s

::echo.
::echo Pulling .po translations back
echo.

tx pull -a --minimum-perc=10

echo.
echo Converting .po to .xml

python xml_po_all.py

echo End.
pause