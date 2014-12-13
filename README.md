These are some of the tools that I wrote when I was developing
[TurboHex](http://www.turbohex.com). Most of these tools were probably derived
from code I wrote in the 1990s,
\(see [BootSectorDisassemblies](https://github.com/AceRoqs/BootSectorDisassemblies)\),
as illustrated by some of the DOS based tools.  I've tried to bring them up to
speed, in the sense that the code is somewhat more consistent with other code
that I have posted on GitHub \(JSF coding style, C++11 required\).

The files in the _DOS_ folder are buildable with Turbo C++ 3.0. The code is
reasonably clean and simple, so I would not be surprised if the code built on
something more modern like [Digital Mars](http://www.digitalmars.com/features.html).
However, because they do direct sector reads via INT 13h, Windows will refuse
to run them.

All other apps are buildable with Visual C++ 2013, and have a dependency on
C++11.

* _GetSector_ will read a given sector from the first physical disk.
* _RipISO_ will create an ISO CD image from the first CD drive.
* _PartitionInfo_ will display the partition table information from the
[MBR](http://en.wikipedia.org/wiki/Master_boot_record) of the first physical
disk.
* _WinPartitionInfo_ is a GUI program which is a bit more complete than the other
utilities. It will display the complete partition information \(including
extended partitions\) of the first two physical disks.
* _DiskTools_ is a shared library for disk reading and other code that is tool
agnostic. The pretty printing code is probably useful to others.

All of the tools must be run elevated \(as Administrator\), except for
_WinPartitionInfo_, which contains manifest information to auto-prompt for elevation.
They all assume a sector size of 512 bytes, which was reasonable at the time
they were developed, but is becoming less true now.  That should be easy to
change if necessary.

_WinPartitionInfo_ also has a limit of 32 total partitions, but it might be improved
if it had a limit per-disk instead of across all disks. It comes to mind that
if some minor restrictions are lifted, a few functions in the app might be
suitable to be extracted into the shared library:
`read_disk_partitions_from_handle()`, `read_disk_partitions()`,
`add_listview_headers()`

I haven't had a disk with extended partitions for some time, so I can't say
that the extended partition code survived my refactoring. If anyone ever confirms
this case, please let me know.

Toby Jones \([www.turbohex.com](http://www.turbohex.com), [ace.roqs.net](http://ace.roqs.net)\)

