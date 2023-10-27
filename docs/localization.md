*How to translate Stunt Rally 2.x (old) game/editor to a language.*

Note: Since SR 2.x is outdated by SR 3.x (using Weblate) I'd recommend
[translating](https://github.com/stuntrally/stuntrally3/blob/main/docs/Localization.md) it instead.

## Introduction

Stunt Rally supports translating (nearly all) visible strings in Game
and Editor.  
*Supplying translations, even small typo fixes, is a great way to
contribute.*

You can easily and conveniently translate on the webpage:  
https://www.transifex.com/projects/p/stuntrally/  

*You can also download the translation from there and translate it
locally using a tool like eg.
[PoEdit](http://sourceforge.net/projects/poedit/).*  

  

## How to start

1.  Get yourself an account on Transifex, if you don't have one already
2.  Pick your language and join its translation team (on website, link
    above)
3.  Add a language if it is not there (if you intend to translate at
    least 20% of it)
4.  You can tell us on Forum or IRC that you did, so we can approve it
    faster
5.  Start translating

  

## Order

There are lots of strings to translate, above 1100.  
This is the recommended order in which to translate.

1.  First translate the strings that have 'HUD' or 'MAIN' in **Developer
    comment**  
    *This way even with few translations a language can already be
    useful*
2.  Next translate only one word strings (for game and editor, Gui tab
    captions etc.)
3.  Then short game strings, related to challenges etc.
4.  Game Hints and other contents of Welcome screen
5.  Editor Input texts (keys shown in editor help window, long but
    crucial for editor users)
6.  Leave tooltips for last (they have 'TIP' in Developer comment and
    are quite long)

  

## General Hints

-   Try to keep your translated strings at equal length (or shorter) of
    the english strings,  
    to make sure they will fit into the GUI (for tooltips this doesn't
    matter).
-   Full translation requires time and patience, also good game and
    editor experience to catch the text meaning.

<!-- -->

-   Check **Developer note:**
    -   This is my comment, usually a group name for few strings telling
        what are they for.
    -   Those having 'Main' are more important than others.
    -   Usually if this comment has a long group name it's not that
        important as the shorter ones.
    -   If you see 'Input' then this is a text for either mouse or
        keyboard action
    -   Only few strings have some unusual longer remarks
-   See [this image](http://i.imgur.com/hgmuHyJ.png) for reference
-   Unwind the pane More details, it shows more info, namely:
-   **Occurrences**
    -   for Gui .layout files shows in which, at which line and  
        **Widget name hierarchy** (parent widget captions)  
        ending with widget **type** on which this string is.
    -   for source .cpp files shows path (inside source/), file name and
        at which line.
-   Context, this is a string id from xml  
    e.g. all input texts start with Input, tips start with Tip etc.
    (apart from that it's not very helpful)

  

## How to read the occurrences text

Example for Boost string:

    Game.layout :1076..SingleRace.Game.Main.Text
     :1133..SingleRace.Game.Tab
     :2842..Options.View.Camera.Check
    ogre/Challenges.cpp:596
    ogre/Gui_Init.cpp:521
    ogre/Gui_Network.cpp:132

So, this string Boost, can be found in Gui:

-   file Game.layout lines 1076,1133 and 2842
-   in 'Single Race' window, tab 'Game', subtab 'Main' as a Text
-   in 'Single Race' window, tab 'Game', as Tab caption
-   in 'Options' window, tab 'View', subtab 'Camera' as CheckBox caption

And in above mentioned source files with line numbers.

Line numbers are combined if in same file e.g.  

    ogre/Gui_Network.cpp:29:345:567

  

## How to check your strings back

To do this you need to get the .po file back from webpage (download).  
Or if you translate with a program locally just save it and get the new
.po file.  

Run the Python3 script locale/xml_po_parser.py on the .po file to get a
\*\_tag.xml file back.  
e.g. inside locale/

    xml_po_parser.py de.po core_language_de_tag.xml

Move the xml into data/gui. Start game or editor and test.

  

## Sync translations

If you build from sources, and you translated something recently, then  
when you see on master a commit 'Sync translations.' this means that  
translations have been updated and you can now check them (after pull).

  

------------------------------------------------------------------------

  

## Technical Details

We use gettext's .po and .pot files for web translations, it is a very
popular text format.

All strings in Stunt Rally are in **the xml file**
data/gui/core_language_en_tag.xml.  
Used by MyGUI's own translation system.  

This file is edited manually, each new string is added there.  
It also has xml comments with group names,  
which then show up as Develeoper comment on web (as #. in .pot file).

A C++ program (source/transl/main.cpp) generates sr.pot templates file
from the xml file.  
*It also searches for string references in sources and Gui layouts, also
getting widgets hierarchy.*

Web translation gives translated .po files back, which are then
converted to  
MyGUI's translations (other languages \*\_tag.xml)  
by the locale/xml_po_parser.py Python3 script.

  

## Setup Transifex

This is a section for developers, who want to do fast translation
updates.

Download the [Transifex
client](https://docs.transifex.com/client/installing-the-client)
(command line tool), or the [new
one](https://github.com/transifex/cli/releases).  
It can both pull new .po translations from website and push new .pot to
it.  
The commands for that are: 'tx pull -a' and 'tx push -s'.

See Transifex
[documentation](https://docs.transifex.com/client/introduction).

To setup, I used (in our locale/ dir):

    tx init
    tx set --auto-remote

and manually edited .tx/config file to put repo url, and source_file
with .pot.

This is the final .tx/config file used (it is already in SR repo):

    [main]
    host = https://www.transifex.com

    [o:stuntrally:p:stuntrally:r:srpot]
    file_filter  = translations/stuntrally.srpot/<lang>.po
    source_file  = sr.pot
    source_lang  = en
    type         = PO
    minimum_perc = 10

Apparently there is no .transifexrc in your OS user dir, only API token
is used, generated on transifex website.

  

## Doing Sync translations

Currently to sync translations, you need to have  
tx (or tx.exe) in locale/ dir - the newest, downloaded Transifex client
(cli) binary.

On Linux ../build/sr-translator (or SR-Translator.exe in locale\\ on
Windows) - it is built from source/transl/main.cpp, with CMake  
On Linux \*.sh and \*.py files need to have execute attributes.

Having that set up, run in locale/ dir:  
1upd-all.sh (or tx_1upd-all.bat)  
*(to update .pot, push it, pull .po and convert all to .xml)*  

and then:  
2git.sh (or tx_2git.bat)  
*(to commit and push).*  

  

## Adding new languages

See example commit adding Czech (cs) language:
[link](https://github.com/stuntrally/stuntrally/commit/18018ecff5ddc27eea7d26f023e2ecea554d5e88)

Needs to be added in 4 places:

data/gui/core_language.xml

    <Info name="cs">
      <Source>core_language_cs_tag.xml</Source>
    </Info>

data/gui/core_language_en_tag.xml

    <Tag name="LANG_CS">Čeština</Tag>

Language name in this language (found on e.g. Wikipedia).  
This will be visible in combobox on Gui.  
Note that it needs translation update after, to have the new tag in all
languages.

source/ogre/Localization.h  

    else if (!strcmp(buf,"Czech")) loc = "cs";

English name of language from windows GetLocaleInfoA function.

source/ogre/common/GuiCom_Util.cpp

    languages["cs"] = TR("#{LANG_CS}");

