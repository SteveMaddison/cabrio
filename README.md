Cabrio
======

Cabrio is an emulator front-end primarily designed for use with arcade
cabinets. It features a simple graphical interface which is easy to use
with limited controls (e.g. a joystick).

Installation
------------
You will need the following libraries and their headers/development packages:

  * SDL
  * SDL_image
  * SDL_gfx
  * SDL_mixer
  * SDL_ttf
  * glut
  * libxml2
  * ffmpeg

To configure, compile and install the program run:
```ShellScript
./configure --prefix=/usr
make -j 3
make install
```

To make a **.rpm** package:

```ShellScript
./configure --prefix=/usr
make -j 3
make pkg-rpm
```

To make a **.deb** package:

```ShellScript
./configure --prefix=/usr
make -j 3
make pkg-deb
```

See http://www.cabrio-fe.org/support/compile.html for more information.

Configuration
-------------
Please see http://www.cabrio-fe.org/support/quickstart.html


License
-------
Copyright (c) 2009 - Steve Maddison <steve@cosam.org>
Distributed under the GNU General Public License (see the COPYING file
for details).


Credits
-------
Default background, "Star-Forming Region LH 95 in the Large Magellanic
Cloud": NASA, ESA, and the Hubble Heritage Team (STScI/AURA)-ESA/Hubble
Collaboration, Acknowledgment: D. Gouliermis (Max Planck Institute for
Astronomy, Heidelberg). http://hubblesite.org/

Default font "FreeSans.ttf" courtesy of the GNU FreeFont project:
http://www.gnu.org/software/freefont/

