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
cd ~/wxSamba
echo `pwd`/samba-daq-system/Acquis > ~/.samba_top
echo `pwd`/samba-daq-system/Acquis >> ~/.samba_top
```

* You can now run samba by executing the following from the terminal:
```
cd ~/wxSamba/samba-daq-system.build
samba/samba
```

## Setup for Boulby

* To setup Samba for Boulby, before running do the following to make sure you're loading the correct configuration:
```
cp `pwd`/samba-daq-system/Acquis/SambaArgs_boulby `pwd`/samba-daq-system/Acquis/SambaArgs
cp `pwd`/samba-daq-system/Acquis/TangoArgs_boulby `pwd`/samba-daq-system/Acquis/TangoArgs
```

## Running Tango

* You can run Tango in a similar way to Samba:
```
cd ~/wxSamba/samba-daq-system.build
tango/tango <run_name>
```

* The run name can be found from the list of directories contained in the directory given by:
```
E/e/
```
where both `E` and `e` are given in the `SambaArgs`/`TangoArgs` files

# Config Documentation

## ~/.samba_top

Consists of two lines:
* 1st -> Folder with SambaArgs + TangoArgs
* 2nd -> Locations for binaries (?) for selecting which version to run

## SambaArgs

* `P`: Preferences folder
* `conf`: Main Configuration file within folder specified by `P`
* `info`: Where additional resources (specifically the dictionaries) are located under the dir that contains `SambaArgs`
* `langue`: Name of dictionary file to load. Must (?) be a folder under the `info` folder with the file `dico` within it

## `conf` File (e.g. `Filelist`)

* `Fichier.modeles`: File that lists files that define the models for this config
* `Fichier.simu`: File containing the simulated signal definitions
* `Fichier.reseau`: File the computers that are in the DAQ system and the dispatchers connected to them

## `Fichier.modeles` File (e.g. `ModelsList`)

List of files that define the components of this detetor model

* Detecteurs: Detector components of the model
* Numeriseurs: Digitiser components of the model
* Cablages: Links between detectors and digitisers
* Repartiteurs: Dispatchers used in experiment
* Support: ???
* Environnement: Additional experimental specific variables to be recorded with each run

## `Fichier.modeles`/Detecteurs

This describes the generalised types of components making up the detectors. This includes:

* `Physique` - The physical type of detector channel, e.g. charge, light. Contains values that are mapped to the internal model [line 324, `salsa/detecteurs.h`, `ModelePhysDesc`]
* `Voies` - The different channel types that are available across all detectors, e.g. ball, north, UVlight. Contains values that are mapped to the internal model, including the physical type (see `Physique`) [line 364, `salsa/detecteurs.h`, `ModeleVoieDesc`]
* `Detecteurs` - The full detector types available in the model, each with given channels (see `Voies`), e.g. simple-ball, 3-akhinos. Chosen physical detectors will then be 'instantiated' for the live config. [line 421, `salsa/detecteurs.h`, `ModeleDetDesc`]


## `Fichier.modeles`/Numeriseurs

This describes the generalised types of components making up the digitisers. This includes:

* `ADC` - Available types of ADC and their settings, (`bits` and `polar_Volts`) [line 265, `numeriseurs.h`, `ModeleADCDesc`]
* `FPGA` - Available types of FPGA.
* `Numeriseurs` - The types of digitisers available, providing the ADC types (see `ADC`), gains and resources available. These resources seem to be DAC settings that you can control from Samba (?)

## `Fichier.modeles`/Cablages

This describes the links between types of detector and types of digitiser, specifically which physical channel from the detector connects to which ADC of the given digitiser.

## `Fichier.modeles`/Repartiteurs

This describes the dispatcher types available in the experiment. Crucially, this determines the 'family' and interface settings so Samba knows how to connect to them to get data.

## `Fichier.modeles`/Support

UNKNOWN!!!

```
# [claps]
Detecteurs = { nb = 6, code = a, #
 } 	# Definition du chassis des detecteurs
Digitiseurs = { nb = 6, code = noms, noms = hg/bg/hd/bd } 	# Definition du chassis des numeriseurs
```

## `Fichier.modeles`/Environnement

File defining additional variables that record info associated with the experiment, e.g. Pressure_mb, Shielding, HV. Type and description can be recorded.

## `Fichier.reseau`

Defines the machine(s) involved in the DAQ system. From NEWSG:
```
acquisition = { local := ip = ., runs = p, repartiteurs = RP-U30 }
```
Note that the dispatcher is referenced here. The '30' is from the last value of the IP address (192.168.2.30 in this case)

## Glossary

* Traitement au vol - On-the-fly processing
* Voies - 'lanes'/channel?
* Numeriseurs - Digitisers
* Reglages - Settings
* Capteurs - Sensors
* Repartiteurs - Dispatchers
* Reseau - Network
