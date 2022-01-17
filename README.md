# Samba DAQ System with wxWidgets Support

This is a fork of the Samba DAQ system that adds support for using wxWidgets. This should
allow it to be compiled and run across any platforms supported by wxWidgets though in
practise, it has currently only been tested on Mac and Linux.


## Installation

Currently, there are only instructions for Linux and MacOS. Windows may follow but should be possible by
modifying the instructions here.

### Obtaining wxWidgets

Frist, you will need to download and build wxWidgets. To do this, following these steps:

* Download the source tarball from here: https://www.wxwidgets.org/downloads/
* Open a terminal and do the following:
```
cd
mkdir wxSamba
cd wxSamba
mv ~/Downloads/wxWidgets* ~/wxSamba/.
tar -xvf wxWidgets*

# change the below to the version of wxwidgets you have
WXWIDGETS_HOME=`pwd`/wxWidgets-3.1.5

cd $WXWIDGETS_HOME
./configure
make
```
* This will build wxWidgets in the WXWIDGETS_HOME directory. You can do a `make install` if you want but this isn't ncecessary.

### Obtaining CMake

Now CMake needs to be installed as this is used to configure the build of Samba

* For linux, this is likely to be installed anyway, if not, install using the appropriate package manager (apt, dnf, yum, etc.)
* For MacOS, you can use homebrew to install it by just doing `brew install cmake`
* If neither of these work, you can always just download it from the website:

https://cmake.org/download/

### Building Samba

* You can now clone the Samba repo:
```
cd ~/wxSamba
git clone https://github.com/drmarkwslater/samba-daq-system.git
```
* Running the following should then configure and build Samba using wxWidgets:
```
mkdir samba-daq-system.build
cd samba-daq-system.build
cmake -D CMAKE_PREFIX_PATH=$WXWIDGETS_HOME -D WXWIDGETS=1 ../samba-daq-system
make
```

Samba is now built and ready to run.


## Running Samba

* Before running Samba, run the following to setup the `~/.samba_top` file:
```
echo `pwd`/samba-daq-system/Acquis > ~/.samba_top
echo `pwd`/samba-daq-system/Acquis >> ~/.samba_top
```

* You can now run samba by executing the following from the terminal:
```
cd ~/wxSamba/samba-daq-system.build
samba/samba
```

