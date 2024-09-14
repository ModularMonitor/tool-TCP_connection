# TCP connection tool

*This was a project to test a way to send and receive data through TCP. As the project was supposed to have a TCP connection through 4G, this would be the host. This is an early test code just for testing, but functional.*

*As the project went by, we didn't succeed to host this in a IPV4 port-forwarded environment, and because of the lack of IPV6 support on our device (I still don't understand why), we decided to "archive" this.*

### How it works

*By starting the app, after compiling using CMake, you can run as **host** or **client** by typing 's' or 'c' when prompted at the beginning.*

#### As client

*You'll have to type a IP to connect to. It expects an IPV6 ready network, so be sure to type a valid IPV6 host. DNS should work.*

*After that, just type an id for the 'file' as a device id would be in a real scenario and then type whatever you want in text, up to 127 characters (128 counting end of string)*

#### As host

*The host will indefinitely keep listening for packages, getting its 'file' id and data, saving them in a file named by its id, appending every message.*

*You can also check the source IP, port and family of the sender on each package on the terminal.*
