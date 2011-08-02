These are some of the tools that I wrote when I was developing
[TurboHex](http://www.turbohex.com). Most of these tools were probably derived
from code I wrote in the 1990s,
\(see [bootsector\_disassemblies](https://github.com/AceRoqs/bootsector_disassemblies)\),
as illustrated by some of the DOS based tools.  I've tried to bring them up to
speed, in the sense that the code is somewhat more consistent with other code
that I have posted on GitHub.

The files in the DOS folder are buildable with Turbo C++ 3.0. The code is
reasonably clean and simple, so I would not be surprised if the code built on
something more modern like Digital Mars. However, because they do direct
sector reads via INT 13h, Windows will refuse to run them.

All other apps are buildable with Visual C++ 2010, and have a dependency on
C++0x.

* _getsect_ will read a given sector from the first physical disk.
* _ripiso_ will create an ISO CD image from the first CD drive.
* _partinfo_ will display the partition table information from the
[MBR](http://en.wikipedia.org/wiki/Master_boot_record) of the first physical
disk.
* _partinfow_ is a GUI program which is a bit more complete thanthe other
utilities. It will display the complete partition information \(including
extended partitions\) of the first two physical disks.
* _shared_ is a shared library of disk reading and other code that is tool
agnostic. The pretty printing code is probably useful to others.

All of the tools must be run elevated \(as Administrator\), except for
_partinfow_, which contains manifest information to auto-prompt for elevation.
They all assume a sector size of 512 bytes, which was reasonable at the time
they were developed, but is becoming less true now.  That should be easy to
change if necessary.

_Partinfow_ also has a limit of 32 total partitions, but it might be improved
if it had a limit per-disk instead of across all disks. It comes to mind that
if some minor restrictions are lifted, a few functions in the app might be
suitable to be extracted into the shared library:
'read_disk_partitions_from_handle()', 'read_disk_partitions()',
'add_listview_headers()'

I haven't had a disk with extended partitions for some time, so I can't say
that the extended partition code survived my refactoring. The code also builds
clean for x64, but as I don't have a 64\-bit machine, I cannot test this. If
anyone ever confirms either of these cases, please let me know.

Toby Jones \([www.turbohex.com](http://www.turbohex.com), [ace.roqs.net](http://ace.roqs.net)\)

