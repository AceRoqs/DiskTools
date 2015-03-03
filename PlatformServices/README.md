_PlatformServices_ is a shared library that contains platform agnostic abstractions
of services that require platform specific implementations.  Initially, this is just
UTF-8 services, as MSVC (unlike Linux compilers) uses ANSI instead of UTF-8 for its
narrow character default for printf, etc.

_PlatformServices_ requires the _PortableRuntime_ library.

Toby Jones \([www.turbohex.com](http://www.turbohex.com), [ace.roqs.net](http://ace.roqs.net)\)
