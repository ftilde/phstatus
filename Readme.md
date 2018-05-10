phstatus
========

phstatus is an alternative status bar for i3. The main feature compared to i3status is the ability to specify active workspaces independently of the monitor the bar is displayed on. This is useful, e.g., if you have multiple monitors, but want the bar to be displayed on a single one of them with information about workspaces on all monitors. Apart from that there are some additional plugins like "i3activewindow" and "weather".

**State**

I consider phstatus to be mostly complete and use it for myself. In retrospect I'm not too happy with some design decisions that I made along the way, but not enough to rewrite it. It still works as it is.
Moreover, the documentation is quite lacking. If you want to use phstatus, I can write some up some more information.

**Building**

* Install dependencies in `dependencies.txt`.
* Configure it: `mkdir build && cd build && cmake ../`
* Build it: `make`

**Use**

DO NOT specify phstatus as the `status_command` in your i3 configuration file.
phstatus is a standalone binary. Just run it like any other program: `./phstatus`

**Licensing**

This work is available under the GNU General Public License (See LICENSE).

Copyright © 2018, ftilde

All rights reserved.
