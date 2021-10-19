# Build phosh-antispam for MS windows

The scripts in this directory shall build phosh-antispam
for MS windows platform using MinGW environment.  An
installer can also be created using msitools.
Until GTK4 is realeased, only GTK3 is supported.


## Required software pieces

To build phosh-antispam for MS windows, latest version
of Fedora is required (64 bit recommended).

The following tools are used to build msi:
* [msitools](https://wiki.gnome.org/msitools)
* [MinGW](http://www.mingw.org)


## Building phosh-antispam

To build for windows and to create an installer do:

```
# From build-aux/w32 directory
./make-w32.sh
# Follow the instruction given by the script
# to build msi installer.
```

This will create an installable `msi` file.  Please note that
when running the above command for the first time, packages of
size to several 100 MiBs shall be downloaded.


## Install and run using wine

You may use wine to test the application.  To run the application
in wine, do the following after installing wine:

Assuming that the file is `installer.msi`

```sh
# Installing installer.msi, writing installation logs to log.txt
wine msiexec /i installer.msi /l*v log.txt
# Running the application.
# Change the path to match you installation
cd "$HOME/.wine/drive_c/Program Files/Anti-Spam"
XDG_DATA_DIRS="./share" wine bin/phosh-antispam.exe
```
