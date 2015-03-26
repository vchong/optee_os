optee_os Porting Guide (Under construction)
==================================

Contents
--------
1.  Introduction
2.  Getting started<br>
    2.1   Prerequisites<br>
    2.2   References<br>
    2.3   Boot sequence
3.  Creating a New Platform
4.  Platform specific changes<br>
    4.1   Compiler and linker options<br>
    4.2   Defines<br>
    4.3   Memory configuration<br>
    4.4   Source files<br>
    4.5   Platform initialization<br>
    4.6   Threads
5. Test/Verify Port
6. Initial Task Checklist

- - - - - - - - - - - - - - - - - -

1. Introduction
----------------
Porting optee_os involves creating a new platform and changing certain platform specific definition, functions and variables. To help ease the porting effort, defaults are provided which can be easily overridden.

Most of these definition, functions and variable can be found in `core/arch/$(ARCH)/plat-$(PLATFORM)`. For example, [core/arch/arm32/plat-vexpress](https://github.com/OP-TEE/optee_os/tree/master/core/arch/arm32/plat-vexpress).

2. Getting started
------------------------
The below lists a few things to be mindful of before starting the port.

2.1 Prerequisites
------------------------
For ARMv8, ARM Trusted Firmware must have already been ported based on [https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/porting-guide.md] (https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/porting-guide.md).

A good reference for porting the ARM Trusted Firmware is this [patch](https://github.com/ARM-software/arm-trusted-firmware/pull/196) that adds support for Juno ARMv8-a board to ARM Trusted Firmware, and shows what the platform specific changes are.

For ARMv7, this is not necessary as each platform has its own vendor-provided platform specific trusted firmware and does not use ARM Trusted Firmware.

Often is the case that an engineer is porting on a development board that still does not have a fully-developed kernel, but at least make sure it is in stable running condition, i.e. definitely no crashes, and has all the config options required to support ARM-TF and OP-TEE. A good idea is to compare your kernel config with that of Juno's.
Reference:<br>
https://github.com/cedric-chaumont-st-dev/optee_os/blob/maintenance/juno_setup/scripts/config.linux-linaro-tracking.a226b22057c22b433caafc58eeae6e9b13ac6c8d.patch

Also, make sure memory map in the kernel device tree (and/or UEFI) properly reflects OP-TEE usage and has no overlap.
Reference:<br>
https://github.com/jforissier/linux/commit/405225aac61c16372c3363ebc142f2ffde094f48<br>
https://github.com/jforissier/edk2/commit/9ebbb05b166d6a738352017290d05ed13fd62129<br>
https://github.com/OP-TEE/optee_os/pull/208/files#diff-075a9ec48ec3243e9add4ad0418605c6<br>
(see scripts/juno.dts.linux-linaro-tracking.a226b22057c22b433caafc58eeae6e9b13ac6c8d.patch)

2.2 References
------------------------
Going through the below documents might be helpful too in understanding the different components involved in a full system and how they interact with one another.<br>
 * [https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/user-guide.md](https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/user-guide.md)<br>
 * [https://github.com/jbech-linaro/linaro-trusted-firmware/blob/master/verified_u-boot_on_armv8.md] (https://github.com/jbech-linaro/linaro-trusted-firmware/blob/master/verified_u-boot_on_armv8.md)<br>
 * [https://github.com/OP-TEE/optee_os/blob/master/scripts/setup_fvp_optee.sh](https://github.com/OP-TEE/optee_os/blob/master/scripts/setup_fvp_optee.sh)

2.3 Boot sequence
------------------------
Understanding the system boot sequence can also be helpful during porting.

The ARM Trusted Firmware [firmware design](https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/firmware-design.md) document has a section on [cold boot](https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/firmware-design.md#2--cold-boot) that describes a reference boot sequence for an ARMv8 system. optee_os corresponds to stage 3-2 of the sequence.

For ARMv7, the vendor-provided trusted firmware typically loads optee_os directly.

3. Creating a New Platform
----------------------------------
A new platform can be created by choosing an existing platform that is closest to the new target, and cloning the plat-\<platform\> directory. For example, `core/arch/arm32/plat-vexpress` --> `core/arch/arm32/plat-<myplat>`.

For details on platforms and flavors, refer to the [build system](https://github.com/OP-TEE/optee_os/tree/master/documentation/build_system.md#platform--platform_flavor-hardware-platform) document. Also reference the [OP-TEE design](https://github.com/OP-TEE/optee_os/blob/master/documentation/optee_design.md) document if necessary.

4. Platform specific changes
------------------------
This section lists the platform specific changes commonly required for porting.

4.1 Compiler and linker options
----------------------------------
### Compiler options (conf.mk)
This file contains platform specific compiler flags and configuration variables for TEE Core and TA libraries. Edit the following variables as required.

*   **CROSS_PREFIX**<br>
    Default: `arm-linux-gnueabihf`

*   **CROSS_COMPILE**<br>
    Default: `$(CROSS_PREFIX)-`

*   **PLATFORM_FLAVOR**<br>
    E.g. `<myflav>`, `juno`, `fvp`, `qemu`, `qemu-virt`, `orly2`, `cannes`

*   **platform-cpuarch**<br>
    E.g. `cortex-a9`, `cortex-a15`, `cortex-a53`, `cortex-a57`

*   **platform-cflags**<br>
    E.g. `-mcpu`, `-mfloat-abi`

For details on conf.mk, refer to [build system](https://github.com/OP-TEE/optee_os/tree/master/documentation/build_system.md#platform-specific-configuration-confmk) document.

### Linker options (link.mk)
This file contains `make` recipes and rules to link the TEE Core. Edit if necessary but these can usually be left as is.

*   **link-out-dir**<br>
    Default: `$(out-dir)/core`

*   **link-script**<br>
    Default: `$(platform-dir)/kern.ld.S`

*   **link-ldflags**<br>
    Default: `$(LDFLAGS)`

For details on link.mk, refer to [build system](https://github.com/OP-TEE/optee_os/tree/master/documentation/build_system.md#platform-specific-link-recipes-linkmk) document.

4.2 Defines
----------------------------------
### Platform-specific definitions (platform_config.h)
This file include definitions for areas such as:<br>
\- GIC base<br>
\- UART<br>
\- Stack sizes (tmp, abt, thread)<br>
\- Memory bases and sizes (dram, trusted dram, TEE ram, shared memory)<br>
\- Number of cores

*   **#define GIC_BASE**

*   **#define UART0_BASE**<br>
    Definition for FPGA UART0

*   **#define UART1_BASE**<br>
    Definition for FPGA UART1

*   **#define UART2_BASE**<br>
    Definition for SOC UART0

*   **#define UART3_BASE**<br>
    Definition for SOC UART1

*   **#define UART0_CLK_IN_HZ**
*   **#define UART1_CLK_IN_HZ**
*   **#define UART2_CLK_IN_HZ**
*   **#define UART3_CLK_IN_HZ**
*   **#define IT_UART3**

*   **#define CONSOLE_UART_BASE**<br>
    Default: `UART3_BASE`

*   **#define IT_CONSOLE_UART**<br>
    Default: `IT_UART3`

*   **#define CONSOLE_UART_CLK_IN_HZ**<br>
    Default: `UART3_CLK_IN_HZ`

*   **define STACK_TMP_SIZE**<br>
    Default: `1024`

*   **#define STACK_ABT_SIZE**<br>
    Default: `1024`

*   **#define STACK_THREAD_SIZE**<br>
    Default: `8192`

*   **#define DRAM0_BASE**
*   **#define DRAM0_SIZE**

*   **#define TZDRAM_BASE**
*   **#define TZDRAM_SIZE**<br>
    Location of trusted dram on the base fvp. TZDRAM_SIZE can be smaller than what is allocatable due to SECTION_SIZE alignment if mapping using big pages.

*   **#define CFG_TEE_CORE_NB_CORE**

*   **#define DDR_PHYS_START**<br>
    Default: `DRAM0_BASE`

*   **#define DDR_SIZE**<br>
    Default: `DRAM0_SIZE`

*   **#define CFG_DDR_START**<br>
    Default: `DDR_PHYS_START`

*   **#define CFG_DDR_SIZE**<br>
    Default: `DDR_SIZE`

*   **#define CFG_DDR_TEETZ_RESERVED_START**<br>
    Default: `TZDRAM_BASE`

*   **#define CFG_DDR_TEETZ_RESERVED_SIZE**<br>
    Default: `TZDRAM_SIZE`

*   **#define TEE_RAM_START**<br>
    Default: `(TZDRAM_BASE)`

*   **#define TEE_RAM_SIZE**

*   **#define CFG_SHMEM_START**<br>
    E.g.: `(DRAM0_BASE + DRAM0_SIZE - CFG_SHMEM_SIZE)`

*   **#define CFG_SHMEM_SIZE**
*   **#define GICC_OFFSET**
*   **#define GICD_OFFSET**

4.3 Memory configuration
----------------------------------
### core_bootcfg.c
This file defines the layout and sizes of various secure and unsecure memory areas, including shared memory. Platform IO devices like UART, GIC, etc. can also be added to `struct map_area bootcfg_stih416_memory`. For example:

```C
/*
 * TEE/TZ RAM layout:
 *
 *  +-----------------------------------------+  <- CFG_DDR_TEETZ_RESERVED_START
 *  | TEETZ private RAM  |  TEE_RAM           |   ^
 *  |                    +--------------------+   |
 *  |                    |  TA_RAM            |   |
 *  +-----------------------------------------+   | CFG_DDR_TEETZ_RESERVED_SIZE
 *  |                    |      teecore alloc |   |
 *  |  TEE/TZ and NSec   |  PUB_RAM   --------|   |
 *  |   shared memory    |         NSec alloc |   |
 *  +-----------------------------------------+   v
 *
 *  TEE_RAM : 1MByte
 *  PUB_RAM : 1MByte
 *  TA_RAM  : all what is left (at least 2MByte !)
 */

static struct map_area bootcfg_stih416_memory[] = {
        {       /* teecore execution RAM */
         .type = MEM_AREA_TEE_RAM,
         .pa = CFG_TEE_RAM_START, .size = CFG_TEE_RAM_SIZE,
         .cached = true, .secure = true, .rw = true, .exec = true,
         },

        {       /* teecore TA load/exec RAM - Secure, exec user only! */
         .type = MEM_AREA_TA_RAM,
         .pa = CFG_TA_RAM_START, .size = CFG_TA_RAM_SIZE,
         .cached = true, .secure = true, .rw = true, .exec = false,
         },

        {       /* teecore public RAM - NonSecure, non-exec. */
         .type = MEM_AREA_NSEC_SHM,
         .pa = CFG_PUB_RAM_START, .size = SECTION_SIZE,
         .cached = true, .secure = false, .rw = true, .exec = false,
         },

         /*
          * Add platform IO devices here
          */
        {
         .type = MEM_AREA_IO_SEC,
         .pa = (GIC_BASE + GICD_OFFSET) & ~SECTION_MASK, .size = SECTION_SIZE,
         .device = true, .secure = true, .rw = true,
         },

        {.type = MEM_AREA_NOTYPE}
};
```

The shared memory pool configuration is platform specific. The memory mapping, including the area `MEM_AREA_NSEC_SHM` (shared memory with non-secure world), is retrieved by calling the platform-specific function `bootcfg_get_memory()`. Please refer to this function and the area type `MEM_AREA_NSEC_SHM` to see the configuration for the platform of interest.

For details on [shared memory](https://github.com/OP-TEE/optee_os/blob/master/documentation/optee_design.md#shared-memory-allocation), refer to the [OP-TEE design](https://github.com/OP-TEE/optee_os/blob/master/documentation/optee_design.md) document.

4.4 Source files
----------------------------------
Any additional platform-specific sources can be included into the build by adding the files in `sub.mk`.

For details on sub.mk, refer to the [build system](https://github.com/OP-TEE/optee_os/tree/master/documentation/build_system.md#source-files) document.

4.5 Platform initialization
----------------------------------
The illustration below shows the flow of platform specific initializations from boot. In cases where changes are necessary here, it is useful to know what is going on.

```
ARMv8 (with ARM-TF)

                        kern.ld.S   <-- Linker script sets _start
                            |           as the ENTRY function
                            |
                            V
Cache and MMU init  --> _start
                            |
                            V
                        main_init
                            |
                            V
                        main_init_helper
                    (plat-stm has this in main_init)
                            |
Init UART, clear BSS,       |
init canaries, init thread  |
handlers and stacks,	    |
init GIC and finally	    |
init TEE core   		    |
                            |
                            V
                        main_init_sec_mon
                        (do nothing)
                    (plat-stm has this in main_init)
                        |                       |
                        V                       V
                    sm_init                 sm_set_entry_vector
                    (do nothing)            (do nothing)


ARMv7 (with monitor)
                        kern.ld.S   <-- Linker script sets _start
                            |           as the ENTRY function
                            |           (plat-stm uses tz_template.lds instead)
                            |
                            V
Cache and MMU init, --> _start (plat-stm uses tz_sinit function instead)
CPU init (all cores)        |
                            |
                            V
                        main_init
                            |
                            V
                        main_init_helper
                    (plat-stm has this in main_init)
                            |
Init UART, clear BSS,	    |
init canaries, init thread  |
handlers and stacks,	    |
init GIC and secure         |
monitor, and finally	    |
init TEE core               |
                            |
                            V
                        main_init_sec_mon
                    (plat-stm has this in main_init)
                        |                       |
                        V                       V
                    sm_init                 sm_set_entry_vector
                        |                       |
                        V                       V
            Sets monitor stack and          Sets thread vector table
            monitor vector table (MVBAR),	(which matches vector
            which handles smc and FIQ       table supplied by ARM-TF)
```

For details on [platform initialization](https://github.com/OP-TEE/optee_os/blob/master/documentation/optee_design.md#2-platform-initialization), refer to the [OP-TEE design](https://github.com/OP-TEE/optee_os/blob/master/documentation/optee_design.md) document.

4.6 Threads
----------------------------------
The Trusted OS uses a couple of threads to be able to support running jobs in parallel (not fully enabled!). There are handlers for different purposes. In [thread.c](https://github.com/OP-TEE/optee_os/blob/master/core/arch/arm32/kernel/thread.c) you will find a function called `thread_init_handlers` which assigns handlers (functions) that should be called when Trusted OS receives standard or fast calls, FIQ, SVC and ABORT and even PSCI calls. These handlers are platform specific, therefore this is something that needs to be implemented by each platform.

The [SMC section](https://github.com/OP-TEE/optee_os/blob/master/documentation/optee_design.md#31-smc-handling) of the [OP-TEE design](https://github.com/OP-TEE/optee_os/blob/master/documentation/optee_design.md) document also describes some of these handlers in greater details.

5. Test/Verify Port
------------------------
To manually test and verify the port:
- Build a Trusted Application and copy it to the rootfs at /lib/teetz/\<filename\>.ta with permission 444.
- Build the corresponding Normal World program and copy the executable to the rootfs at /bin/ with permission 755.

A sample hello_world test program can be found at [http://github.com/jenswi-linaro/lcu14_optee_hello_world](http://github.com/jenswi-linaro/lcu14_optee_hello_world).

There is also a private repo, containing a set of tests and the sample above, that can be included in the build [scripts](https://github.com/OP-TEE/optee_os/tree/master/scripts), and run directly from the command prompt after system booting. Due to licensing issues, however, it is only available to authorized users.

###Sample Log
**Normal World**
```Shell
root@Vexpress:/ modprobe optee
misc teetz: no TZ l2cc mutex service supported
misc teetz: outer cache shared mutex disabled
root@Vexpress:/ sleep 0.1
root@Vexpress:/ tee-supplicant&
root@Vexpress:/ hello_world
Invoking TA to increment 42
TA incremented value to 43
root@Vexpress:/
```

**Secure World**
```Shell
DBG TEE-CORE:tee_ta_init_static_ta_session:1196:    Lookup for Static TA 8aaaf200-2450-11e4
DBG TEE-CORE:tee_ta_init_session_with_signed_ta:1240:    Load dynamic TA
DBG TEE-CORE:tee_ta_load:651: Loaded TA at 0x7e100000, ro_size 26872, rw_size 564, zi_size 14516
DBG TEE-CORE:tee_ta_load:652: ELF load address 0x200000
DBG TEE-CORE:tee_ta_init_session_with_signed_ta:1249:       dyn TA : 8aaaf200-2450-11e4
tee_user_mem_alloc:343: Allocate: link:[0x1fffc4], buf:[0x1fffd4:16]
INF USER-TA:TA_OpenSessionEntryPoint:56: Hello World!
DBG TEE-CORE:tee_ta_close_session:1067: tee_ta_close_session(7df7917c)
DBG TEE-CORE:tee_ta_close_session:1082:    ... Destroy session
INF USER-TA:TA_CloseSessionEntryPoint:68: Goodbye!
tee_user_mem_free:442: Free: link:[0x1fffc4], buf:[0x1fffd4:16]
DBG TEE-CORE:tee_ta_destroy_context:1020:    ... Destroy TA ctx
```

6. Initial Task Checklist
------------------------
- [ ] Port ARM-TF with U-Boot/UEFI (as bl33.bin) but without optee_os (bl32.bin)
- [ ] Make platform specific changes to optee_os
  - [ ] Add new platform
    - [ ] core/arch/arm32/plat-\<myplat\>
  - [ ] conf.mk
    - [ ] CROSS_PREFIX
    - [ ] CROSS_COMPILE
    - [ ] PLATFORM_FLAVOR
    - [ ] platform-cpuarch
    - [ ] platform-cflags
  - [ ] link.mk
    - [ ] link-out-dir
    - [ ] link-script
    - [ ] link-ldflags
  - [ ] platform_config.h
    - [ ] GIC_BASE
    - [ ] UART0_BASE
    - [ ] UART1_BASE
    - [ ] UART2_BASE
    - [ ] UART3_BASE
    - [ ] UART0_CLK_IN_HZ
    - [ ] UART1_CLK_IN_HZ
    - [ ] UART2_CLK_IN_HZ
    - [ ] UART3_CLK_IN_HZ
    - [ ] IT_UART3
    - [ ] CONSOLE_UART_BASE
    - [ ] IT_CONSOLE_UART
    - [ ] CONSOLE_UART_CLK_IN_HZ
    - [ ] STACK_TMP_SIZE
    - [ ] STACK_ABT_SIZE
    - [ ] STACK_THREAD_SIZE
    - [ ] DRAM0_BASE
    - [ ] DRAM0_SIZE
    - [ ] TDRAM_BASE
    - [ ] TDRAM_SIZE
    - [ ] CFG_TEE_CORE_NB_CORE
    - [ ] DDR_PHYS_START
    - [ ] DDR_SIZE
    - [ ] CFG_DDR_START
    - [ ] CFG_DDR_SIZE
    - [ ] CFG_DDR_TEETZ_RESERVED_START
    - [ ] CFG_DDR_TEETZ_RESERVED_SIZE
    - [ ] TEE_RAM_START
    - [ ] TEE_RAM_SIZE
    - [ ] CFG_SHMEM_START
    - [ ] CFG_SHMEM_SIZE
    - [ ] GICC_OFFSET
    - [ ] GICD_OFFSET
  - [ ] core_bootcfg.c
    - [ ] struct map_area bootcfg_stih416_memory
  - [ ] Source files (if new ones are added)
    - [ ] sub.mk
    - [ ] Add new files
  - [ ] Platform initialization (if required)
  - [ ] Thread handlers (if required)
- [ ] Build optee_os
- [ ] Rebuild ARM-TF with U-Boot/UEFI as bl33.bin and optee_os as bl32.bin
- [ ] Build other required system components
  - [ ] kernel
  - [ ] rootfs
  - [ ] optee_client
  - [ ] optee driver module
  - [ ] test cases
- [ ] Test/Verify
  - [ ] Build Trusted Application
  - [ ] Build Normal World program
  - [ ] Update rootfs
  - [ ] Flash all images
  - [ ] Verify run
- [ ] Coffee/tea break :D
