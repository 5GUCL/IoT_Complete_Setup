# Vidcentum Smart Data Logger Platform

```
/*==========================LICENSE NOTICE==========================*/
/*
 * Software:  Smart Data Logger Lite (Modbus).
 * Product code: SDL-MODBUS
 * Platform code: PM.ODP (Operational Data Platform)
 * Type of software: Application.
 * License: Refer to LICENSE file of the software package. GPL-v3.
 * Company: Vidcentum R&D Pvt Ltd, India (CIN: U72200TG2009PTC063723).
 * Copyright (c) 2020-2021.
 * Email: support@vidcentum.com
 * Website: https://vidcentum.com
 * Address: Plot No. 1-62/20, Kavuri Hills, 
 *          Hyderabad - 500081, Telangana, India.
*/
/*========================END LICENSE NOTICE========================*/
```
---
## Smart Data Logger Lite (Modbus) 
### ```Product Code: VC.SDL.LITE.MODBUS```
Data collection is a critical step to sustain the production and the processes. Vidcentum Smart Data Loggers integrate factory, 
plant, and field operations. Smart Data Logger Lite (Modbus) is a software program / framework to develop Modbus/TCP based applications. 

### Advantages
1. Reduces the application development time. It is easy to build and run in the lab or application environment.
2. Change the software to suit the target environment.
3. Full control over the data. 
4. Extend it with new features or functionalities.
5. Acts as application test-bed.
6. Regular software updates.
7. Flexible licensing for better development and run-time management.
8. Get professional support (choose the plan that is suitable for you).

### Features
1. Modbus/TCP support. 
2. Simple configuration with YAML files.
3. Fine-grain control of reading Modbus/TCP registers, coils and input bits.
4. Support multiple Modbus devices simultaneously.
5. Manage metadata easily.
6. HDF5 datasets.
7. Upstream integrations.
8. Logging.
9. Immediate mode GUI.
10. Deploy compiled sources (C/C++).
11. SI units.
12. Network Capabilities with MQTT, Websockets, REST etc.

### System Requirements
#### Development Setup
1. Ubuntu 20.04 Linux, x86_64
2. Compile on Raspberry Pi target.
3. CodeLite IDE
4. C++17, CMake

#### Deployment (Target System)
1. Ubuntu 20.04 Linux, x86_64 desktop / workstation or cloud VM 
2. Raspberry Pi with at least 2GB RAM, 64 Bit and with GUI

#### Dependencies
1. See  ```docs/open-source-software-and-dependecies.txt``` for more details.

### Support
1. Contact ```support@vidcentum.com```
2. Choose the support plan suitable for your application development.

---

### Source code (excluding ```oss``` directory and other open source software modules)

Extract the software archive to a convenient location on your development system

```
You can see the source tree as below.

src/
├── app-gui
│   ├── main-gui
│   │   └── main_gui.cpp
│   └── node-gui
│       └── modbus
│           ├── mb_node_master_gui_ws_client.cpp
│           └── mb_node_master_imgui_main.cpp
├── h-data
│   └── hdf5
│       └── node
│           └── modbus
│               ├── mb_node_master_dataset_ws_client.cpp
│               └── mb_node_master_hdf5_recorder.cpp
├── launcher
│   └── node
│       └── modbus
│           ├── mb_controllers_launcher.cpp
│           ├── mb_ncap_mqtt_launcher.cpp
│           ├── mb_node_master_hdf5_recorder_launcher.cpp
│           └── mb_node_master_imgui_launcher.cpp
├── main.cpp
├── ncap
│   └── network
│       └── mqtt
│           ├── mb_ncap_mqtt_ws_client.cpp
│           └── ncap_mqtt_backend.cpp
├── node
│   └── modbus
│       ├── libmodbus
│       │   └── libmodbus_wrapper.cpp
│       ├── modbus_master_node_agent.cpp
│       ├── modbus_master_node_controller.cpp
│       ├── modbus_master_node.cpp
│       ├── modbus_server_node_agent.cpp
│       ├── modbus_server_node_controller.cpp
│       └── modbus_server_node.cpp
├── sf-sys
│   ├── si-units
│   │   └── compact_si_units.cpp
│   └── utils
│       └── time_utils.cpp
└── supervisor
    └── supervisor.cpp

    
include/
├── app-gui
│   ├── app_imgui_opengl_inc.h
│   ├── main-gui
│   │   └── main_gui.h
│   └── node-gui
│       └── modbus
│           ├── mb_node_master_gui_ws_client.h
│           └── mb_node_master_imgui_main.h
├── h-data
│   └── hdf5
│       └── node
│           └── modbus
│               └── mb_node_master_hdf5_recorder.h
├── launcher
│   └── node
│       └── modbus
│           ├── mb_controllers_launcher.h
│           ├── mb_ncap_mqtt_launcher.h
│           ├── mb_node_master_hdf5_recorder_launcher.h
│           └── mb_node_master_imgui_launcher.h
├── loggers
│   └── logger_registry.h
├── meta-data-models
│   └── thing.h
├── ncap
│   └── network
│       ├── mqtt
│       │   └── ncap_mqtt_backend.h
│       └── ncap_backend_defs.h
├── node
│   └── modbus
│       ├── libmodbus
│       │   └── libmodbus_wrapper.h
│       ├── modbus_master_node_agent.h
│       ├── modbus_master_node_controller.h
│       ├── modbus_master_node.h
│       ├── modbus_node_agent.h
│       ├── modbus_node_controller.h
│       ├── modbus_node_event.h
│       ├── modbus_node.h
│       ├── modbus_node_registry.h
│       ├── modbus_protocol.h
│       ├── modbus_server_node_agent.h
│       ├── modbus_server_node_controller.h
│       └── modbus_server_node.h
├── README.md
├── sf-sys
│   ├── si-units
│   │   ├── compact_si_units.h
│   │   └── si_unit_constants.h
│   └── utils
│       ├── cpp_prog_utils.h
│       └── time_utils.h
└── supervisor
    └── supervisor.h

Open source libraries included in the build.
oss/
├── cpp-base64
├── cpp_util_3
├── cpr
├── fmt
├── imgui
├── imnodes
├── implot
├── ixwebsocket
├── mqtt_cpp
├── nlohmann
├── SI
├── so_5
├── so_5_extra
├── spdlog
├── stb
├── taskflow
├── uSockets
└── uWebSockets

```
#### Formatting and style.
1. Source code is formatted with ```astyle``` utility.
2. ```astyle -A3 -s2 -C -f -y -xU -U --recursive "src/*.cpp"``` and 
   
   ```astyle -A3 -s2 -C -f -y -xU -U --recursive "include/*.h"```

#### Static code analysis.
1. Used ```cppcheck``` on the source code for static analysis.

### Build process

#### Prerequisites

##### Install GCC / G++ 
Install latest gcc/g++, build-essential, binutils, autoconf, cmake etc from the linux distribution.

##### Install YAML/JSON utilities

```
Software: nodejs. Install from the linux distribution packages.
npm packages: 
sudo npm install -g yamltojson
sudo npm install -g jsontoyaml
```
#### Install ```Boost Libraries```
```
Software Module: Boost C++ 
You may compile the library and install locally. Follow the instructins
of the package.
Source: https://www.boost.org/
Licence: https://www.boost.org/users/license.html

Linux Package:
You may install Boost libraries from the Linux package manager as well
sudo apt-get install libbost-dev

Or follow the build instructions - https://www.boost.org/
```

##### Install ```openssl-dev and curl```
```
Software Module: cUrl
Source: https://github.com/curl/curl
Licence: Open Software
https://github.com/curl/curl/blob/master/COPYING

sudo apt install libcurl4-openssl-dev

Note: OpenSSL is used.
Prerequisites: openssl dev

```
##### Install ```OpenGL development libraries```
```
sudo apt-get install build-essential libgl1-mesa-dev  # Debian, Ubuntu
sudo apt-get install libglew-dev libsdl2-dev libsdl2-image-dev libglm-dev libfreetype6-dev  # Debian, Ubuntu
sudo apt-get install libglfw3-dev
sudo apt-get install libfreetype-dev libfreetype6 libfreetype6-dev
````

##### Install ```libmodbus```
```
Software Module: libmodbus. Install from the linux distribution packages.
You may compile the library and install locally. Follow the instructions
of the package.
Source: https://github.com/stephane/libmodbus/
Licence: GNU Lesser General Public License v2.1.
https://github.com/stephane/libmodbus/blob/master/COPYING.LESSER

Linux Package:
You may install libmodbus from the Linux package manager as well
sudo apt install libmodbus-dev

Build libmodbus
Clone the libmodbus sources.
git clone --recursive --recurse-submodules https://github.com/stephane/libmodbus.git

// System-wide installation
export CFLAGS+=" -Os "
export CPPFLAGS+=" -g -Wall -fPIC "
export CXXFLAGS+=" -g -Wall -fPIC "

./autogen.sh
./configure  --enable-static

make
make check
sudo make install

For static linking
    libmodbus.a

For dynamic linking
    modbus
```
##### Install ```HDF5```
```
Software Module: HDF5. Install from the linux distribution packages.
You may compile the library and install locally. Follow the instructions of the package.
Source: https://www.hdfgroup.org/solutions/hdf5/
Licence: HDF5 (Hierarchical Data Format 5) Software Library and Utilities.
https://github.com/HDFGroup/hdf5/blob/develop/COPYING

HDF5 Version: 1.10 version. HDF5 Viewer is built with HDF5 1.10. 

Linux Package:
You may install HDF5 from the Linux package manager as well
sudo apt install libhdf5-dev 

Build HDF5
If you would like to build HDF5 on target, you may follow the instructions below.
export CFLAGS+=" -Os "
export CPPFLAGS+=" -g -Wall -fPIC "
export CXXFLAGS+=" -g -Wall -fPIC "

./configure --prefix=/usr/local \
            --with-zlib \
            --enable-build-mode=production \
            --with-pic\
            --enable-hl \
            --enable-cxx \
            --enable-build-all \
            --enable-java

make
make check
sudo make install

C/C++ Libs: (preserve the order)
For static linking
    libhdf5_hl_cpp.a    - HDF5 High Level C++ APIs 
    libhdf5_cpp.a       - HDF5 C++ Library  
    libhdf5_hl.a        - HDF5 High Level C APIs
    libhdf5.a           - HDF5 C Library

For dynamic linking
    hdf5_hl_cpp    - HDF5 High Level C++ APIs 
    hdf5_cpp       - HDF5 C++ Library  
    hdf5_hl        - HDF5 High Level C APIs
    hdf5           - HDF5 C Library
```

#### Install OpenJDK 15 for HDFView

```

Released on 15 September 2020, the OpenJDK 15 is the latest feature release of JDK. 
But it is not the part of Ubuntu 20.04 base repository.
To install OpenJDK 15, download its Debian package from the Oracle official website 
https://www.oracle.com/java/technologies/javase-jdk15-downloads.html.
(OR)
JAVA JDK Archives: http://jdk.java.net/archive/ 
https://download.java.net/java/GA/jdk15.0.2/0d1cfde4252546c6931946de8db48ee2/7/GPL/openjdk-15.0.2_linux-x64_bin.tar.gz

Download the above openjdk-15.0.2_linux-x64_bin.tar.gz
extarct 
..> jdk-15.0.2
copy to /usr/lib/java/  or (or wherever you prefer for example, /opt/java/) 
cd /usr/lib/java/
                +/jdk-15.0.2
                
Use the above path (or wherever you prefer) 


$ export PATH=$PATH:"/usr/lib/java/jdk-15.0.2/"
$ export JAVA_HOME="/usr/lib/java/jdk-15.0.2"

$ HDFView

```


##### Install ```libuv```

```
Software Module: libuv. Install from the linux distribution packages.
You may compile the library and install locally. Follow the instructions of the package.
Source: https://github.com/libuv/libuv
Licence: Open Source.
https://github.com/libuv/libuv/blob/v1.x/LICENSE

Linux Package:
You may install libuv from the Linux package manager as well
sudo apt install libuv1-dev

Build libuv
Clone the libmodbus sources.
wget https://github.com/libuv/libuv/archive/v1.41.0.tar.gz

// System-wide installation
export CFLAGS+=" -Os "
export CPPFLAGS+=" -g -Wall -fPIC "
export CXXFLAGS+=" -g -Wall -fPIC "
./autogen.sh
./configure 

make
sudo make install

For static linking
    libuv.a

For dynamic linking
    uv
```
#### Building Smart Data Logger Modbus (VC.SDL.LITE (Modbus))
```
cd to the source tree directory of the software.
$<source directory>
    mkdir build
    cd build
    cmake ..
    make
    cd ..

cd to the install directory.
$<source directory>
    install/
    ├── bin
    ├── config
    ├── datasets
    ├── fonts
    └── README.md

cd install/bin

Execute the sdl-lite-modbus
./sdl-lite-modbus 

You may use process management tools such as pm2 to run the app.
https://www.npmjs.com/package/pm2

```
#### Datasets
```
$<source directory>
    install/
    ├── datasets

The smart-data-logger-lite generates HDF5 datasets in the above directory. 
You can take dataset backups from the GUI of smart-data-logger-lite.

Download HDF5 viewer 
https://datacloud.vidcentum.com/s/tQyBrCZf5YxoH8j

File: HDFView-3.1.2-Linux-x86_64.tar.gz

Install the viewer.
$ tar -xvf HDFView-3.1.2-Linux-x86_64.tar.gz
$ sudo dpkg -i hdfview_3.1.2-1_amd64.deb

``` 
#### Configuration

```
$<source directory>
    install/
    ├── config
```
---
## General FAQ
#### 1. What is the use of the "Smart Data Logger Lite (Modbus)"?
Modbus is a proven industry protocol to interface with sensors / transducers / PLCs etc., It is widely used in the automation systems. Industrial IoT (IIoT) brings all new applications connecting the assets (industrial, factory etc.,) to the digital platforms. SDL-Lite (Modbus) is designed to connect the existing and/or new Modbus (TCP) based systems with the IIoT digital platforms.

#### 2. I just want to learn Modbus protocol. Can "Smart Data Logger Lite (Modbus)" be useful?
Yes. Absolutely. Modbus standard is available from `https://www.modbus.org/`. Get familiar with the standard first.  "Smart Data Logger Lite (Modbus)" is a tool that surely helps you understand the protocol. It has inbuilt Modbus/TCP server and you really need not have a physical device to understand the protocol. Also, you can browse the data collected from the Modbus/TCP server simulations using HDF5 viewer. You can get a copy of the HDF5 viewer from `https://datacloud.vidcentum.com/s/tQyBrCZf5YxoH8j`.

#### 3. Can I modify the software?
Yes. You can. It is released under GNU GPL-3. 

#### 4. What is a support package subscription?
You gain access to free training, development material, early releases, software updates. Also, you get discounts if you hire us for your projects.

#### 5. Why should I subscribe to a support package?
Let's make it win-win together. We are investing in developing useful software for industrial applications. It needs money and support. However, we do not compromise on software freedom. You get a copy of the software and you are free to modify as per your requirements. Subscribing to a support package get early access to the new features and updates. Furthermore, you can attend the training sessions for free or heavily discounted prices. Contact **`sales@vidcentum.com`** for further details.

#### 6. Does support package subscription include development services?
No. If you need more help in your project, you can hire us on a contract basis. If you have a support subscription, you may get an additional discount on contract value.  Contact **`sales@vidcentum.com`** for further details.

#### 7. Do you support academic or open source projects?
Yes. Please do contact **`sales@vidcentum.com`** for further details. 

#### 8. How frequently do you release the updates (or patches)?
We have plans to release monthly updates. If no update is available, it will be informed to all subscribers in advance.

#### 9. Is there any refund of the support package?
No. Sorry. We understand your concerns. Start small. Start with community support. Subscribe to a support package for a minimum period and see if it works for you. If you are convinced, go for 6 months to 12 months subscriptions.

#### 10. Are you going to add other industrial protocols to the framework?
Yes, progressively.

#### 11. Will my support package subscription cover all protocols?
No. You need additional subscriptions. Your support subscription covers updates, bug fixes, enhancements of one product line; for example, VC.SDL.LITE (Modbus) support subscriptions cover only VC.SDL.LITE (Modbus) updates, bug fixes etc.,

#### 12. Will you provide on-site support?
No. Due to pandemic situations, we prefer remote support. Since is it software, it is easy to support remotely.

#### 13. What is the mode of training?
Online. 

#### 14. Can I use "Smart Data Logger Lite (Modbus)" in my clients' projects?
Yes. You may need additional support package licenses for your customers. 

#### 15. Can I conduct a training using "Smart Data Logger Lite (Modbus)"?
Yes. Please do contact **`sales@vidcentum.com`** for further details. 

---

### Screenshots

![Listing Modbus devices](/assets/1.png)

![Ploting of Modbus data](/assets/2.png)

![Multiple Ploting of Modbus data](/assets/3.png)

---
