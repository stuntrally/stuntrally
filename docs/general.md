*General info about the project.*

### Main

See [Compiling](compile.md) section if building from sources.  
If you'd like to contribute please read
[Contributing](https://github.com/stuntrally/stuntrally3/blob/main/docs/Contributing.md).  
For latest info on progress check [Roadmap](https://github.com/stuntrally/stuntrally3/blob/main/docs/Roadmap.md) and developer
[Tasks](https://stuntrally.tuxfamily.org/mantis/view_all_bug_page.php).  
Page for [Donations](https://cryham.tuxfamily.org/donate/) if you'd like
to support SR financially.

### Media

**[Videos](https://www.youtube.com/user/TheCrystalHammer)** from
gameplay and editor tutorials.  
**[Screenshots gallery](https://stuntrally.tuxfamily.org/gallery)**
(from all versions) and [Development
gallery](https://stuntrally.tuxfamily.org/gallery-dev) (lots of funny
screenshots).  
Old presentation document of the project (aka SR book, 196 pages), topic
with [info
here](https://forum.freegamedev.net/viewtopic.php?f=81&t=7411) and [repo
here](https://github.com/stuntrally/presentation).  
Main developer [CryHam's website](https://cryham.tuxfamily.org/), has
pages for [game](https://cryham.tuxfamily.org/portfolio/stuntrally/) and
longer with
[editor](https://cryham.tuxfamily.org/portfolio/2015-sr-track-editor/).  

### Feedback

Be sure to [Read before
posting](https://forum.freegamedev.net/viewtopic.php?f=78&t=3814)
first.  
You can post on
[Forum](https://forum.freegamedev.net/viewforum.php?f=77) or create new
Issue on [github](https://github.com/stuntrally/stuntrally/issues) (if
not already present).  
Since years, IRC is not used, and was team's development channel.  

------------------------------------------------------------------------

### Technologies and Libraries used

Code is written in [C++](https://en.wikipedia.org/wiki/C%2B%2B). Uses
[CMake](https://cmake.org/) and [Conan](https://conan.io/) to build.  
[Git](https://git-scm.com/) for repository. Located on [Github
here](https://github.com/stuntrally/). Licensed under [GPL
v3.0](https://www.gnu.org/licenses/gpl-3.0.en.html).  
Car simulation is done by [VDrift](https://vdrift.net/) also using
[bullet physics](https://bulletphysics.org/).  
Sound with [OpenAL Soft](https://openal-soft.org/).  
Rendering by [OGRE 3D](https://www.ogre3d.org/). Gui by
[MyGUI](https://github.com/MyGUI/mygui).  
In new (upcoming) SR 3.0 PBR materials with
[HLMS](https://ogrecave.github.io/ogre-next/api/latest/hlms.html) in
Ogre-Next.  
In old SR 2.7: trees/grass by
[PagedGeometry](https://code.google.com/p/ogre-paged/) (does severe
delays, lags), materials managed by
[shiny](https://forums.ogre3d.org/viewtopic.php?f=11&t=71117&start=50).  
Road is based on a 3D
[spline](https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Catmull.E2.80.93Rom_spline)
and it's fully customizable in editor.  
Both road and SR Track Editor are fully implemented our own code by
CryHam.

### Website

Website is made with [CMS Made Simple](https://www.cmsmadesimple.org/),
[DokuWiki](https://www.dokuwiki.org/dokuwiki) and [Mantis Bug
Tracker](https://www.mantisbt.org/).  
Hosted on [TuxFamily](https://www.tuxfamily.org/en/about).
