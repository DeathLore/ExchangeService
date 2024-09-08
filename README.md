# ExchangeService

Plans:
 - Notification
 - Unit-tests
 - Processing bad Client's info
 - Add C++20 Coroutines
 - Canceling trade requests
 - Viewing opened trade requests by client
 - Password

# Build
1. In project's directory enter in terminal: `cmake -DCMAKE_BUILD_TYPE=Release -S src -B build`.
2. Switch to build directory and enter: `make`.

# How to use
After building the project, switch to build directory, there would be 2 files: `Server` and `Client`.

First of all, you have to start `Server` (it blocks terminal session).<br>
Then you can start `Client` in another terminal session; all Client's interactions are supposed to happen via terminal.