# houdini-sop-shapefile

[Houdini](http://www.sidefx.com/index.php) HDK SOP node which loads Shapefile SHP files using [Shapefile C Library](http://shapelib.maptools.org).

## More info
* [Shapefile C Library](http://shapelib.maptools.org)

## Binaries, Houdini 16.0
* [Windows, Houdini 16.0.557]() 

## Building

* Tested on Windows and Houdini 16.0.
  * You would have to patch CMake file to get this building on Linux.
* Define HOUDINI_VERSION env variable to be the version of Houdini 16.0 you wish to build against (for example "16.0.557").
* Alternatively, you can have HFS env variable defined (set when you source houdini_setup).
* Generate build files from CMake for your favorite build system. For Windows builds use MSVC 2015.
* Build the SOP Houdini dso (SOP_Shapefile.dylib or SOP_Shapefile.dll).
* Place the dso in the appropriate Houdini dso folder.
  * On OS X this would be /Users/your_username/Library/Preferences/houdini/16.0/dso/
  * On Windows this would be C:\Users\your_username\Documents\houdini16.0\dso
  * Or $HOUDINI_PATH/dso

## Usage

* Place the SOP into your SOP network.
* Chose .shp file to load and create geometry from.

## License for the plugin

* Copyright Mykola Konyk, 2017
* Distributed under the [MS-RL License.](http://opensource.org/licenses/MS-RL)
* **To further explain the license:**
  * **You cannot re-license any files in this project.**
  * **That is, they must remain under the [MS-RL license.](http://opensource.org/licenses/MS-RL)**
  * **Any other files you add to this project can be under any license you want.**
  * **You cannot use any of this code in a GPL project.**
  * Otherwise you are free to do pretty much anything you want with this code.
  
## License for [Shapefile C Library](http://shapelib.maptools.org)
* MIT
* [Full Shapefile C Library License](http://shapelib.maptools.org/license.html)
