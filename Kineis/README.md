This is package is releasing the Kinéis Stack SW library with two demo-application examples (standalone or AT commands) on a STM32WL55xx-based HW reference design (KRD_MP).



One demo app can be compiled at the same time. You can choose it by changing the Makefile's APP variable to GUI or STDLN.
```c
# Select APPlication. Can be:
# * STDLN: for the standalone application sending one message at startup
# * GUI: for the application using the AT commands from UART link
APP = STDLN
```

By default,  this package compiles the standalone APP, starting to send one user message immediately with the MAC BASIC profile. This will lead to:
* start kineis stack MAC profile BASIC
* transmit A message once immediately
* close the kineis stack MAC profile
* The device goes into LPM shutdown then

By changing the Makefile's MAC_PRFL variable to BLIND, the standalone application will re-transmit
the message several times as per profile's configuration in a way to reach some expected QoS.
* start kineis stack MAC profile BLIND
* 4 retransmissions of this messages each 60s
* The device goes into LPM standby between each retx
* close the kineis stack MAC profile
* Once all retransmissions are completed, the device goes to LPM shutdown

Refer also to the \ref kns_app_page for extra details about the applications.


The kineis stack is meant to be integrated in an OS-like environment supporting LPM. In this package, you will find:

* the application is embedded in a task
* the kineis stack is embedded in another task
* a third task (very-low priority IDLE task) is used to enter in low power mode once the others task becomes inactive.

# Getting started: configuration

## Credentials managment

This kineis stack needs some credentials (ID, address, secret key) to run properly. 

\attention Please contact Kinéis to get your own credentials file

\attention Credentials are delivered by Kineis after certificication of your device

\attention Once credentials are delivered by Kineis, it becomes your responsability

It is up to the integrator to store it as per its own security strategy. The kineis stack is just
requesting them through APIs called MCU_NVM wrapper (mcu_nvm.c):

* "get device ID" and "get device address" are said not to be critical from a security point of
view. Thus it remains visible when the API is called
* "AES init" and "AES encrypt" are used to build the frame which is transmitted over the air.
\note Here Kineis stack never knows the secret key.

In this package, the credentials are just hardcoded in the code, meaning clearly stored in
RAM/FLASH without any protections. One can change the code of these wrappers to fit its security
strategy (e.g. set the secret key directly in some AES HW module or secure element). 

## Radio configuration managment

This FW package needs some radio configuration (transmission frequency range, transmission
modulation, ...) to run properly.

\attention Please contact Kinéis to get your own radio configuration binary file when you will
deploy your devices.

It is up to you to handle how to store those informations in the devices. The kineis stack is just
requesting them through one API. 

In this package, the configuration binary file is just hardcoded in the code as a table of bytes in mcu_nvm.c. You will get several examples of it. You can comment, uncomment the one you want.

\note In case of GUI APP, it is also possible to change the radio configuration through the AT
command "AT+RCONF" (\ref bMGR_AT_CMD_RCONF_cmd)

All the Kineis Stack needs is to be able to get the radio configuration at initialization.

\note In case the user application needs to change the radio configuration on the fly, Kineis Stack needs to be stopped, then restarted.

## MAC profile configuration managment

The Kineis MAC protocols (a.k.a. profiles) can be tuned according to the device capabilities and user's application needs, such as:

* Timer capabilities
* Kineis service subscription

So far, it may be possible to choose between two different protocols:

* BASIC: Transmit the user message once immediately. Here the QoS is not guaranteed

* BLIND: The user message will be re-transmitted several times periodically in a way to guarantee some QoS

\attention Please contact Kinéis to get configuration for your profile when you will deploy your devices.

so far, you can choose the protocol by changing Makefile variable MAC_PRFL:

```c
# Select Kineis stack MAC profile. Can be:
# * BASIC: basic profile, sending message once immediately 
# * BLIND: blind profile, sending message sevral times periodically 
MAC_PRFL = BASIC
```

Then, once the protocol is chosen, some specific configuration may be tuned. Regarding BLIND protocol, refer to \ref KNS_MAC_BLIND_usrCfg_t in kns_app.c:

* number of repetitions

* repetition period

# Getting started: entry point

## main

The entry point of this package is the main function. 

Cf dedicated page here \ref main_page for all the details.

## OS

The OS embedded in this package is the "Kineis baremetal OS" which has very minimalist footprint. Behind the "queue create" and "task register" APIs, all is actually statically declared ( cf kns_q_conf.c, kns_q_conf.h, kns_os_conf.h files in \ref Kineis/Extdep/Conf folder).

The purpose here, is to show the "KNS_OS..." and "KNS_Q..." abstraction APIs internally used by the Kineis stack.
An integrator can use its own OS, by adapting the "KNS_OS..." and "KNS_Q..." source code to it.

\attention Take care the Kineis task is actually polling 3 queues in loop. Ensure your implementation of KNS_Q push/pop is not waiting indefinitively on a a single queue.

\ref kns_os_page

# Package content

All Kinéis related informations are located in `Kineis` folder. Typical source tree looks like
below.

## View

```
.
├── App
│   ├── Kineis_os
│   │   ├── KNS_OS
│   │   │   ├── Inc
│   │   │   │   └── kns_os.h
│   │   │   └── Src
│   │   │       └── kns_os.c
│   │   └── KNS_Q
│   │       ├── Inc
│   │       │   └── kns_q.h
│   │       └── Src
│   │           ├── kns_q_baremetal.c
│   │           ├── kns_q.c
│   │           ├── kns_q_cmsis_os2.c
│   │           └── kns_q_freertos.c
│   ├── kns_app.c
│   ├── kns_app.h
│   ├── Libs
│   │   ├── STRUTIL
│   │   │   ├── Inc
│   │   │   │   └── strutil_lib.h
│   │   │   └── Src
│   │   │       └── strutil_lib.c
│   │   └── USERDATA
│   │       ├── Inc
│   │       │   └── user_data.h
│   │       └── Src
│   │           └── user_data.c
│   ├── Managers
│   │   └── MGR_AT_CMD
│   │       ├── Inc
│   │       │   ├── mgr_at_cmd_common.h
│   │       │   ├── mgr_at_cmd.h
│   │       │   ├── mgr_at_cmd_list_certif.h
│   │       │   ├── mgr_at_cmd_list_general.h
│   │       │   ├── mgr_at_cmd_list.h
│   │       │   ├── mgr_at_cmd_list_mac.h
│   │       │   ├── mgr_at_cmd_list_previpass.h
│   │       │   └── mgr_at_cmd_list_user_data.h
│   │       └── Src
│   │           ├── mgr_at_cmd.c
│   │           ├── mgr_at_cmd_common.c
│   │           ├── mgr_at_cmd_list.c
│   │           ├── mgr_at_cmd_list_certif.c
│   │           ├── mgr_at_cmd_list_general.c
│   │           ├── mgr_at_cmd_list_mac.c
│   │           ├── mgr_at_cmd_list_previpass.c
│   │           └── mgr_at_cmd_list_user_data.c
│   └── Mcu
│       ├── Inc
│       │   └── mcu_at_console.h
│       └── Src
│           ├── mcu_at_console.c
│           └── mcu_at_console_stm.c
├── Appconf
│   ├── kns_app_conf.h
│   ├── kns_os_conf.h
│   └── mgr_log_conf.h
├── Doc
│   ├── krd_fw
│   │   ├── doc_version.txt
│   │   └── html
│   │       ├── index.html
│   ├── libkineis
│   │   ├── doc_version.txt
│   │   └── html
│   │       ├── index.html
│   └── libknsrf_wl
│       ├── doc_version.txt
│       └── html
│   │       ├── index.html
├── doxy_warn_log_file.txt
├── Extdep
│   ├── Conf
│   │   ├── kineis_sw_conf.h
│   │   ├── kns_assert.c
│   │   ├── kns_assert.h
│   │   ├── kns_cs.c
│   │   ├── kns_cs.h
│   │   ├── kns_q_conf.c
│   │   └── kns_q_conf.h
│   ├── Mcu
│   │   ├── Inc
│   │   │   ├── aes.h
│   │   │   ├── mcu_aes.h
│   │   │   ├── mcu_misc.h
│   │   │   ├── mcu_nvm.h
│   │   │   └── mcu_tim.h
│   │   └── Src
│   │       ├── aes.c
│   │       ├── mcu_aes.c
│   │       ├── mcu_misc.c
│   │       ├── mcu_nvm.c
│   │       └── mcu_tim.c
│   └── MGR_LOG
│       ├── Inc
│       │   ├── mgr_log.h
│       │   └── mgr_log_rtc.h
│       └── Src
│           ├── mgr_log.c
│           └── mgr_log_rtc.c
├── Lib
│   ├── kns_cfg.h
│   ├── kns_glossary.h
│   ├── kns_mac_evt.h
│   ├── kns_mac.h
│   ├── kns_mac_prfl_cfg.h
│   ├── kns_rf.h
│   ├── kns_srvc_common.h
│   ├── kns_types.h
│   ├── libkineis.a
│   ├── libkineis_info.c
│   ├── libkineis_info.h
│   ├── libkineis_undefined_symbols.txt
│   ├── libknsrf_wl.a
│   ├── libknsrf_wl_info.c
│   ├── libknsrf_wl_info.h
│   └── libknsrf_wl_undefined_symbols.txt
├── Lpm
│   ├── Inc
│   │   ├── lpm_cli_kstk.h
│   │   ├── lpm.h
│   │   └── mgr_lpm.h
│   └── Src
│       ├── lpm.c
│       ├── lpm_cli_kstk.c
│       └── mgr_lpm.c
├── README.md
└── version.txt
```

## Description

**App** folder contains some generic code for:

* **Kineis_os** contains the Kineis baremetal OS used to register queues and tasks.

  The APPlications task containing demos mentioned earlier (kns_app.c). The GUI app relies on an AT command manager.

* **Managers/MGR_AT_CMD** contains the AT command manager used by the GUI application.

  It is about parsing a ASCII data stream coming from UART driver. 

  Entry point of this manager is mgr_at_cmd.h.

  mgr_at_cmd_list.h is the entry file referencing the AT cmds supported by this firmware. Then, the other mgr_at_cmd_list files refers to subsets of AT cmds per functionnalities (such as general, user_data). General subset is to get ID or firmware version. User data subset is to transmit data over the air.	

* **Libs/STRUTIL** and **Libs/USERDATA** are pure sw libraries (no HW dependencies) used in AT cmd manager.

* **Mcu** contains low level hardware drivers (called wrappers) used by the components above.

  Typically, it contains the wrapper used to handle a terminal/console to post/receive AT cmds  (mcu_at_console.h).

  \note So far, this at_cmd console wrapper is derived from STM32_HAL_UART drivers.

  This is actually an UART driver changed to handle a continuous ACII data stream. It may be optimized as your convenience or based on STM LL drivers instead. Each time a character is received on UART, it is invoking a stream-parsing callback (so far \ref MGR_AT_CMD_parseStreamCb in this package).



**Appconf** folder contains the static configurations which are specific to

* This package
* This microcontroller (STM32WL55xx)



**Extdep** folder contains external dependencies of the stack. So far, it is about

* **Conf** contains static configurations used by the entire project.

  As mentionned earlier, you will find here all things concerning OS task and Queue definitions. There is also some utilities to catch the asserts of the kineis stack and to implement critical sections.

* **Mcu** contains low level hardware drivers (called wrappers) used by the kineis stack library.

  This is about AES (mcu_aes.c), credentials (mcu_nvm.c), extra drivers such as external PA (mcu_misc.c). The integrator is expected to implement its own version of those wrappers as per his platform/HW needs. By default, this package is delivering a simple template of each wrapper.

* **MGR_LOG** The logger (a.k.a logging manager in mgr_log.h) used by the library to log important informations during execution.

  \note In typical package delivery the kineis stack is compiled without debug info. This actually contains empty macros.

  So far, this logger is also used by the application software described later below



**Lpm** contains all code related to the support of low power on this microcontroller (STM32WL55xx) with interactionto the kineis stack task and the application task.

* lpm.h is the entry point for low power managment. It does the bridge between generic lpm_mgr and STM32WL55xx's specifics
* mgr_lpm.h is the generic part of LPM managment. Tthis code can be ported on any other uC.
* lpm_cli refers to "LPM clients" such as the lpm_cli_kstk.h, which is the Kineis stack LPM client. This code actually converts the Kineis stack ressource status (KNS_MAC_getRsrcStatus) into low power mode supported by this plateform (MgrLpm_LPM_t)



**Lib** Contains some Kineis libraries and its APIs

* libkineis.a is the kineis stack libraries [doc link](../../../../../Doc/libkineis/html/index.html)
* libknsrf_wl.a contains the RF driver for the STM32WL55xx uC [doc link](../../../../../Doc/libknsrf_wl/html/index.html)
* top-APIs and bottow-APIs are present through the header files



**version.txt** file contains references to the software version of the package



# How to build

This project is based on makefiles. It is possible to invoke it from IDE or from console then.

## From STM32CubeIDE

### Open package in IDE

Install STM32CubeIDE and open project present in the Kinéis package.
Toolchain (GCC, GNU Make) should direcly come up with your installation.

Present package was tested with following STM32CubeIDE v1.10.1 on windows 10.
Present package was tested with following STM32CubeIDE v1.18.0 on Linux

Once STM32CubeIDE is properly installed:

* Go to following menu: `File > Open Projects from File System...`
* In popped-up windows
    * Click on `Archive...` button in a way to select Kinéis .tar .gz archive of
    this package (should be named something like `krd_fw_package_.*.tar.gz`.
    * Click on `Open`
    * Let STM32CubeIDE parse the package once `Folder` sub-window shows items
    * In this `Folder` sub-windows, select only the found `Eclipse project`
    * Click on `Finish`. `krd_fw` project should open and be available in `Project Explorer` panel
* In `Project Explorer` panel, you can select the `krd_fw` project. It should open properly.

### Build

\attention You always need to **clean the build before any rebuild**

Right click on project and select `Clean Project` to clean all. In build console, you should see
something like:

```
make clean 
rm -fR build
...
```

Right click on project and select `Build Project` to build. you should see something like:
```
make all 
mkdir -p build
arm-none-eabi-gcc ...
```

Binary file `krd_fw.elf` file should be present in `build` folder of the project and ready to be
flashed in KRD_MP HW.

## From terminal

This project was tested and validated with following toolchain
```bash
GNU Make 4.4.1_st_20231030-1220
Built for x86_64-pc-linux-gnu
Copyright (C) 1988-2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
```
```bash
arm-none-eabi-gcc (GNU Tools for STM32 12.3.rel1.20240926-1715) 12.3.1 20230626
Copyright (C) 2022 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

In a way to build same binary from STM32CubeIDE or console whenever you are in windows or linux, it
is recommended to point on same toolchain coming-up with STM32CubeIDE installation. You may need to
update your PATH variable to point on this toolchain.

* Linux: Depending on you IDE/toolchain version, add something like following in your ~/.bashrc
```bash
export PATH=/opt/st/stm32cubeide_1.17.0/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.linux64_1.1.0.202410170702/tools/bin/:$PATH
export PATH=/opt/st/stm32cubeide_1.17.0/plugins/com.st.stm32cube.ide.mcu.externaltools.make.linux64_2.2.0.202409170845/tools/bin/:$PATH
```
* Windows: Update System PATH variable as usual.

### Build

\attention You need to **clean the current build before any rebuild**

Once toolchain is correctly available, go to root directory of the package and run following commands to build for default HW

```
make clean
make
```
Binary file `*.elf` file should be present in `build` folder of the project and ready to be
flashed in KRD_MP HW.

## Compilation flags (log level, LPM level, ...)

The makefile contains several compilation flags which can be changed as per your convinience:

```makefile
DEBUG = 1             # enable/disable log with debug level
VERBOSE = 0           # enable/disable log with verbose level (impacts real-time)
USE_BAREMETAL = 1     # compile with/without kineis baremetal OS
APP = STDLN           # compile standalone or AT-cmd-like application
MAC_PRFL = BASIC      # compile stabdalone application with BASIC/BLIND profile (Note this is useless in case of GUI application)
LPM = SHUTDOWN        # deeepest low power mode allowed, can be: NONE, SLEEP, STOP, STANDBY, SHUTDOWN
KRD_BOARD = KRD_FW_MP # KRD board HW type, choose between: KRD_FW_LP, KRD_FW_MP
```

# Doc

Documentation folder is containing doxygen auto-generated documentation. Read html documentation by
opening index.html files present in sub-folders of `Kineis/Doc/..` with a web browser.
