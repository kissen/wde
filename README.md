wde: watch directory entries
============================

Output a message when a file gets added to or removed from a
directory. Only works on Linux because it uses
[inotify](https://en.wikipedia.org/wiki/Inotify). I use it to watch
`/dev`.


Usage
-----

* Call `wde` with any number of directories
* If the `-r` flag is set, directories are added recursively


Bugs
----

* It is written in C.
* [There already is a tool that does it.](https://github.com/rvoicilas/inotify-tools/wiki)


License
-------

(C) 2017 Andreas Schärtl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.