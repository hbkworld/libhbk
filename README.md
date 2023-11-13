# What is it for?

The library contains
- Functionality concerning communication and system events often used for system programming for Linux and Micosoft Windows.
- Some convenience classes for string and exception handling.

## Library
Center of all is a library made up from classes from different scopes:

- Communication: Classes concerning network interfaces and communication
- Exceptions: Often used types of exceptions
- String: Common string manipulation methods
- System: Classes concerning system programming (i.e. event loop, timers and notifiers)

Sources are to be found below `lib`. Headers needed to interface with the library are to be found under `include`.



## License

Copyright (c) 2024 Hottinger Brüel & Kjær. See the [LICENSE](LICENSE) file for license rights and limitations.

## Prerequisites

### Used Libraries
We try to use as much existing and prooved software as possbile in order to keep implementation and testing effort as low as possible. All libraries used carry a generous license. See the licenses for details.

The open source project jsoncpp is being used as JSON composer and parser.
The unit tests provided do use the google test libraries. Refer to [googletest](https://github.com/google/googletest/ "") for details.


### Build System
The cmake build system is being used. Install it using your distribution package system. 


#### Windows
MSVC uses `vcpkg` to integrate gtest please read ´https://github.com/microsoft/vcpkg` for details. If exeuting vcpkg from `powershell` it might refuse downloading any package.
If so, please execute `Set-ExecutionPolicy AllSigned` and try again.

## Test Coverage (Linux)

gcovr is used to measure the test coverage

change into the build directory and execute:

```
gcovr -r <source directory> -e <source directory>/test  --object-dir=.  --branches --html-details -o coverage.html
```

-r tells where sources are
-e excludes a directory
--object-dir where to find the compiled object files

