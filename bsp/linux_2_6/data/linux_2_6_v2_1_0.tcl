##############################################################################
#
#     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
#     SOLELY FOR USE IN DEVELOPING PROGRAMS AND SOLUTIONS FOR
#     XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION
#     AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE, APPLICATION
#     OR STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS
#     IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
#     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
#     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
#     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
#     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
#     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
#     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#     FOR A PARTICULAR PURPOSE.
#
#     (c) Copyright 2004-2008 Xilinx, Inc.
#     All rights reserved.
#
#     Modification History:
#     ---------------------
#     04/20/05 rpm    Added adapter/driver dependency capability to Tcl.
#                     Removed emac_v1.00c support. Removed
#                     emac_v1.00e/gemac_v1.00f drivers from MLD directory since
#                     MV already delivers these versions.
#     12/10/05 jb&xd  Added temac Linux kernel 2.4 adapter v1.00a support.
#                     Added code to support Linux Makefile in build/linux of
#                     bsp or driver directories
#
#     12/10/05 ecm    Added emac Linux kernel 2.4 adapter v1.01a support.
#                     Includes DRE and Checksum offload support.
#
#     03/08/06 jvb    Added temac Linux kernel 2.4 adapter v2.00a support.
#                     Includes DRE and Checksum offload support.
#
#     08/28/06 jvb    This is a new MLD for linux 2.6 (MontaVista Linux and
#                     WindRiver Linux), but it is originally based on the MLD
#                     for MontaVista Linux Pro 3.1 (kernel 2.4). Much of this
#                     script has been revised to streamline the MLD flow
#                     along with adding support for updating the linux
#                     .config file.
#     01/19/07 rpm    Updated mapping of temac driver to v2.00c adapter with PHY
#                                         detection code
#     12/01/06 mta    Added the IIC adapter support to the MLD.
#     05/05/07 ecm    Added PCI support for ML410 board specifically.
#     05/21/07 rpm    Added new driver versions to adapter list
#     07/15/07 rpm    Removed redefines for several drivers since driver Tcl
#                     now takes care of these
#     09/17/07 ecm    Added the external PCI interrupt support,bridge interrupt
#                     still handled by the intc tcl.
#     11/29/07 sv     Added the USB device support.
#     12/03/07 ecm    Added the iic v1.13.b driver support
#     02/20/08 jvb    Added support for ppc440
#     03/24/08 jvb    Fixed i2c on WRL1.3
#                     Added WRL2.0 support
#                     Split kernel files out into their own distribution
#                         directories
#                     Merged the ML410 changes into the main kernel files
#                     Added FPU spurious illegal instruction exception
#                         workaround
#                     Always copy certain drivers (usb_gadget, dmav2, and
#                         dmav3) so linux will build
#                     Added ",tcp" option to boot command line for nfs root
#                     Fixed bug not GPL'izing files
#                     Fixed problem enabling emac instead of temac in the
#                         automatic config updater script
#                     Added 'make oldconfig' to xmake
#                     Various other bug fixes
#     04/24/08 jvb    Fixed problem needing caches disabled on WRL1.3 and
#                         WRL2.0
#                     Removed the 'boards' directory
#                     Moved the linux distribution directories under the new
#                          'dist' directory.
#                     Fixed a bug needing iic when using temac on WRL1.3
#                     Added a driver_default_versions array so default
#                          drivers (drivers that are always copied) can be
#                          added easily.
#                     Added 2 new FPU workarounds
#                     Fixed end memory address for temac in virtex.c
#                     Fixed a problem with config updater getting stuck in
#                         'make oldconfig' on WRL2.0
#                     Added setup of vlim registers in head_44x.S
#     05/14/08 sv     Added the spi v1.12.a driver support
#     05/30/08 sd     Added xps_ps2 driver support
#     07/28/08 sv     Added the spi v2.00.a driver support
#     07/28/08 ecm    Added the ML510 with PCI support
#     08/29/08 sv     Added the spi v2.01.a, lltemac v1.01.a 
#                        iic v1.14.a, usb v1.02.a driver support
#     09/01/08 sv     Added the Gpio v2.13.a driver support 
#     09/15/08 jvb    Added support for MVL5.0. Uses new fdt device tree
#                     generation
#
##############################################################################

#
# Global MLD version
#
# ~v1.02a in EDK
set MLD_version "linux_2_6"
set device_tree_MLD_version "device-tree_v1_00_a"

#
# Global Driver-Adapter dependency list
# This list maintains the dependency between versions of drivers & adapters
# (examples are shown below in comments)
#
set driver_adapter_list(temac_v2_00_a) "temac_linux_2_6_v2_00_c"
set driver_adapter_list(temac_v2_10_a) "temac_linux_2_6_v2_00_c"
set driver_adapter_list(lltemac_v1_00_a) "lltemac_linux_2_6_v1_00_a"
set driver_adapter_list(lltemac_v1_00_b) "lltemac_linux_2_6_v1_00_a"
set driver_adapter_list(lltemac_v1_01_a) "lltemac_linux_2_6_v1_00_a"
set driver_adapter_list(iic_v1_03_a) "iic_linux_2_6_v2_00_a"
set driver_adapter_list(iic_v1_13_a) "iic_linux_2_6_v2_00_a"
set driver_adapter_list(iic_v1_13_b) "iic_linux_2_6_v2_00_a"
set driver_adapter_list(iic_v1_14_a) "iic_linux_2_6_v2_00_a"
set driver_adapter_list(usb_v1_01_a) "usb_linux_2_6_v2_00_a"
set driver_adapter_list(usb_v1_02_a) "usb_linux_2_6_v2_00_a"
set driver_adapter_list(gpio_v2_11_a) "gpio_linux_2_6_v2_12_a"
set driver_adapter_list(gpio_v2_12_a) "gpio_linux_2_6_v2_12_a"
set driver_adapter_list(gpio_v2_13_a) "gpio_linux_2_6_v2_12_a"
set driver_adapter_list(spi_v1_12_a) "spi_linux_2_6_v1_00_a"
set driver_adapter_list(spi_v2_00_a) "spi_linux_2_6_v1_00_a"
set driver_adapter_list(spi_v2_01_a) "spi_linux_2_6_v1_00_a"

#
# Default driver versions list
# This maintains a list of default versions for drivers that need to be copied
# even if not in the connected_perips list. Some drivers are needed to be
# always present because they're needed to build the linux kernel (even if not
# enabled). Note that this problem only exists when we add a new driver not
# already present in the base BSP from Montavista or Wind River, etc.
#
set driver_default_versions(usb) "usb_v1_02_a"
set driver_default_versions(iic) "iic_v1_14_a"

#
# Globals
#
lappend drvlist
set edk_install [file join $env(XILINX_EDK)]
set board "other";
set distribution "";
array set console_devices []
array set console_device_types []

set did_pci false

#
# This is the great big comment how we create the BSP for linux.
#
# This script will build a sparse linux kernel directory tree containing
# driver files and hadware configuration related files. This linux kernel tree
# that is built is meant to overlay an existing linux kernel tree. If the
# variable TARGET_DIR is set in XPS, then this script can build the sparse
# linux tree right over the top of an existing tree, avoiding the need to copy
# the sparse tree after it's generated.
#
# This script will copy or generate the following files:
# * driver files including linux adapter driver files
# * xparameters.h - this c header file defines preprocessor symbols for the
#                   current hardware configuration. The current plan is for
#                   only irtex.c to use this header file. Eventually we'll
#                   want to allow for the hardware information to be passed to
#                   the kernel at boot time, possibly by the bootloader.
# * Makefiles for building the drivers
# * Config updater script and supporting files
# * Static linux tree - for files that apply to multiple drivers or are not
#                       related to any driver.
#
# In general, this script first copies in the driver files from the project area
# and/or the install area. Then it overlays the files from the MLD over the
# driver files just copied. Then the script copies over the static linux
# tree. Finally the .config file is updated if target_dir is set.
#
# If the project area has an MLD, then that project MLD supersedes the one found
# in the install area. So if the project's MLD is missing a file, this script
# will not search the MLD in the install area for it.
#
# We always want the files in the project to have priority over those in the
# install area. This way an individual project can override drivers and MLD
# files.
#
#
# Driver File Copying
# --------------------
# When we copy driver files, we copy the main driver files first, then we
# overlay those files with the ones found in the MLD directory.
#
# Libgen, before this script is called, will copy over the driver files into
# a separate staging area. Libgen uses this separate staging area to build a
# static c library which is not used for linux.
#
# Since Libgen does not copy over any of the OS specific files, this script
# now ignores the files in the Libgen staging area. This is new behavior from
# previous versions. The reason is that if this script copies the source from
# the original repositories (instead of copying from the staging area), we can
# prevent accidentally copying the wrong OS specific files which may not match
# what was copied in the Libgen staging area (Perhaps Libgen changes the way it
# works). It also turns out the code is much simpler in this script if it
# copies the driver files and OS related files together in the same fashion.
#
# Xilinx drivers are structured so that the main driver files can be used on
# any system, with the OS specific part residing in what we call an adapter
# driver. In linux the adapter driver can be (though, not required to be)
# delivered separately from the main driver code. However, adapter drivers are
# dependent on a specific version of the main driver. When the main driver
# files are copied into the sparse linux kernel tree, the correct adapter
# driver is brought over at the same time.
#
# Specific versions of the adapter drivers are associated with a version of a
# main driver in the table, driver_adapter_list. driver_adapter_list is just
# an associative array with the main driver and version as a text string index
# and the adapter driver name and version as the value.
#
# Perhaps someday, the user will be able to select adapter driver version
# separately from that of the main driver. For now we'll just use
# driver_adapter_list.
#
# Linux adapter drivers will always be delivered in with the MLD for Linux.
#
# So the overall copying process generally looks like this. For each driver:
# 1. if the driver exists in the project area,
#    copy driver files from the project to the linux kernel tree.
# 2. else if the driver exists in the install area,
#    copy driver files from the install to the linux kernel tree.
# 3. if there's an MLD in the project area
#    attempt to overlay the driver files from the project's MLD to the linux
#    kernel tree. Also copy any associated adapter drivers from this MLD.
# 4. else if the driver exists in the MLD of the install area,
#    attempt to overlay driver files from the installed MLD to the linux kernel
#    tree. Also copy any associated adapter drivers from this MLD.
#
# This script also copies subdriver files. Any driver that is a dependency for
# a given driver is copied over.
#
# Building xparameters.h
# --------------------
# Libgen creates xparameters.h for us. The only problem is that the
# preprocessor symbols created are based on the names of the cores used in XPS.
# So from one project to the next there will be different preprocessor symbols
# for the same basic IP cores.
#
# To make this a little less complicated, this script will append preprocessor
# symbols definitions that create a consistent set of aliases so that linux can
# rely on a consistent set of preprocessor symbols.  (Note: In EDK 9.2 and
# later some drivers are starting to create these "canonical constants", so as
# they are created by the driver they will be removed from this Tcl.)
#
# This script reads xparameters.h out of the Libgen staging area
# (<prj>/ppc405_0/libsrc), and appends the #define symbol aliases to the file.
#
# At the end of this process, this script copies xparameters.h is to
#     linux/arch/ppc/platforms/4xx/xparameters
# During this copy, this script also renamed xparameters.h based on the fpga type:
#     Virtex2Pro -> xparaemters_ml300.h
#     Virtex4FX -> xparaemters_ml403.h
#     Virtex5FX  -> xparameters_ml507.h
#
#
# Makefile Copying
# --------------------
# When we deliver our own adapter drivers for linux, sometimes we need to
# deliver a linux Makefile for the driver. This is mainly for the case when
# the Makefile already provided in the MontaVista or WindRiver LSP no longer
# works with the new driver code being delivered.
#
# Makefiles originally reside in the build/linux2_6 directory inside each driver's
# top level directory. Here is an examples:
#     <prj>/drivers/emac_v1_00_f/build/linux2_6/Makefile
#
# While the driver source is being copied over, this script will check if there
# is a corresponding Makefile. If so, the Makefile is copied over too.
#
# Static Linux Tree
# --------------------
# The static linux tree is copied over after the drivers and Makefiles are
# copied into the target directory. While this sequence may seem
# counter-intuitive at first, the static linux tree can be used to overlay in
# updates to the sub-drivers in the xilinx_common subdirectory (which requires
# this step to be done after the drivers).
#
#
# Config updater script and supporting files
# --------------------
# This script copies the config updater script, cow.tcl, and most of it's
# related input files. In addition, this script generates the input file which
# indicates what items to enable in the linux .config file (xps_connected.cud).
#
# This script enables 3 types of items:
# * The first type simply enables the driver for a given core that is
#   included (and used for linux) in the XPS project.
# * The second type sets where the root file system will reside. The user
#   provides this information in the 'OS and Libraries' section of the
#   'Software Platform Settings' dialog box.
# * The third type sets the kernel command line options. The root file system
#   selection implies some of the command line options, while the user
#   provides other information, such as the ip address.
#
# The config updater script then uses it's input files (*.cud), to update the
# linux kernel .config file. If target_dir is set and it appears to refer to a
# valid linux kernel tree (there's a .config file in target_dir), then this
# script will run the config updater for the user. Otherwise, this script will
# just copy the config updater script to be used on the target.
#
# The config updater script can be run on the target using a couple of methods.
# 1) The user can run the config updater script by hand (tclsh cow.tcl)
# 2) The user can indirectly run the config updater script by using the xmake
#    shell script.
# Note that tcl needs to be installed on the target for this to work.
#
#

# ----------------------------------
# When we get into this tcl script,
# * The current working directory is:  <prj>/ppc405_0/libsrc/$MLD_version
#
# * The project's driver directory is: <prj>/drivers/
#                                      ../../../drivers
# * The project's MLD directory is:    <prj>/bsp/$MLD_version
#                                      ../../../bsp/$MLD_version
# * The install's MLD directory is:    <install>/sw/ThirdParty/bsp/$MLD_version
# * The install's driver directory is: <install>/sw/XilinxProcessorIPLib/drivers
# ----------------------------------
#

proc dbg_puts {prefix str} {
    #
    # To enable debug, change 0 to 1 below.
    #
    if {1} {
        puts "\[$prefix\] $str"
    }
}

proc linux_drc {lib_handle} {
    puts "\#--------------------------------------"
    puts "\# Linux BSP DRC...!"
    puts "\#--------------------------------------"
}


proc get_bsp_dir {MLD_version} {
        global env

        set MLD_file "data/linux_2_6_v2_1_0.mld"

#        set bspdir_install [file join $env(XILINX_EDK) "sw/ThirdParty/bsp/${MLD_version}"]
#        set bspdir_proj [file join "../../.." "bsp/${MLD_version}"]
        set install_dir $env(XILINX_EDK)
        set bspdir_install "$install_dir/sw/ThirdParty/bsp/${MLD_version}"
        set bspdir_proj "../../../bsp/${MLD_version}"

        if {[file isfile $bspdir_proj/$MLD_file]} {
                return $bspdir_proj
        } elseif {[file isfile $bspdir_install/$MLD_file]} {
                return $bspdir_install
        }
}

proc device_tree_bspscript {device_tree_MLD_version} {
        global env

        set device_tree_MLD_script "data/device-tree_v2_1_0.tcl"

        set edk_install_dir $env(XILINX_EDK)
        set device_tree_bspdir_install "$edk_install_dir/sw/ThirdParty/bsp/$device_tree_MLD_version"
        set device_tree_bspdir_proj "../../../bsp/$device_tree_MLD_version"

	dbg_puts "device_tree_bspscript" "checking $device_tree_bspdir_proj/$device_tree_MLD_script"
	dbg_puts "device_tree_bspscript" "    then $device_tree_bspdir_install/$device_tree_MLD_script"
        if {[file isfile $device_tree_bspdir_proj/$device_tree_MLD_script]} {
		dbg_puts "device_tree_bspscript" "returning $device_tree_bspdir_proj/$device_tree_MLD_script"
                return $device_tree_bspdir_proj/$device_tree_MLD_script
        } elseif {[file isfile $device_tree_bspdir_install/$device_tree_MLD_script]} {
		dbg_puts "device_tree_bspscript" "returning $device_tree_bspdir_install/$device_tree_MLD_script"
                return $device_tree_bspdir_install/$device_tree_MLD_script
        } else {
		dbg_puts "device_tree_bspscript" "returning empty"
                return ""
        }
}

proc generate {libname} {
    variable edk_install
    variable MLD_version
    global bspdir

    puts "\#--------------------------------------"
    puts "\# Linux BSP generate..."
    puts "\#--------------------------------------"


    #
    # Find the source bsp directory. Look first at the project and then
    # default to the install area. Note that the bsp /data directory (MLD/Tcl)
    # needs to be completely populated wherever it resides. This does not
    # currently support a sparse bsp directory.
    #
    # The /drivers directory can be sparsely populated with source files as
    # this MLD first copies the project's driver directory then overlays any
    # driver files also released with the MLD.
    #
    set bspdir [get_bsp_dir $MLD_version]
}

#
# Get the processor clock frequency from the PROCESSOR section
# of the MSS file
#
proc xget_corefreq {} {

    set sw_processor [xget_libgen_proc_handle]
    set processor [xget_handle $sw_processor "IPINST"]
    set name [xget_value $processor "NAME"]
    set processor_driver [xget_swhandle [xget_value $processor "NAME"]]

    if {[string compare -nocase $processor_driver ""] != 0} {
        set arg "CORE_CLOCK_FREQ_HZ"
        set retval [xget_dname [xget_value $processor_driver "NAME"] $arg]
        return $retval
    }
}

#
# Get the FPGA type, Virtex2Pro, Virtex4, Virtex5, etc
#
# Right now the processor information includes the fpga type.
# Virtex 5FXT has ppc440.
#
proc xget_fpga_type {} {
    set sw_processor [xget_libgen_proc_handle]
    set processor [xget_handle $sw_processor "IPINST"]
    set processor_type [xget_hw_value $processor]
    set name [xget_value $processor "NAME"]
    set processor_driver [xget_swhandle [xget_value $processor "NAME"]]


    if {[string compare -nocase $processor_type "ppc440_virtex5"] == 0} {
        dbg_puts "FPGA type" "virtex5"
        return "virtex5";
    } elseif {[string compare -nocase $processor_type "ppc405_virtex4"] == 0} {
        dbg_puts "FPGA type" "virtex4"
        return "virtex4";
    } else {
        dbg_puts "FPGA type" "virtex2Pro"
        return "virtex2Pro";
    }
}

#
# Procedure that adds defines to xparameters.h as XPAR_argument. Only
# the MLD parameters are handled here.
#
proc xredefine_params {handle file_name args} {

    # Open include file
    set file_handle [xopen_include_file $file_name]

    foreach arg $args {
        #
        # Use a special function to get processor frequency, otherwise just
        # get the parameter value as entered by the user.
        #
        if {[string compare -nocase $arg "CORE_CLOCK_FREQ_HZ"] == 0} {
            set value [xget_corefreq]
        } else {
            set value [xget_value $handle "PARAMETER" $arg]
        }

        dbg_puts "xredefine_params" "$arg $value"
        #
        # Rename the parameters to constant names that Linux expects.
        #
        if {$value != ""} {
            set value [xformat_addr_string $value $arg]

            if {[string compare -nocase $arg "memory size"] == 0} {
                set name "DDR_0_SIZE"
            } elseif {[string compare -nocase $arg "uart16550 bus clock freq"] == 0} {
                set name "PLB_CLOCK_FREQ_HZ"
            } elseif {[string compare -nocase $arg "IIC persistent baseaddr"] == 0} {
                set name "PERSISTENT_0_IIC_0_BASEADDR"
            } elseif {[string compare -nocase $arg "IIC persistent highaddr"] == 0} {
                set name "PERSISTENT_0_IIC_0_HIGHADDR"
            } elseif {[string compare -nocase $arg "IIC persistent eepromaddR"] == 0} {
                set name "PERSISTENT_0_IIC_0_EEPROMADDR"
            } elseif {[string compare -nocase $arg "powerdown baseaddr"] == 0} {
                set name "POWER_0_POWERDOWN_BASEADDR"
            } elseif {[string compare -nocase $arg "powerdown highaddr"] == 0} {
                set name "POWER_0_POWERDOWN_HIGHADDR"
            } elseif {[string compare -nocase $arg "powerdown value"] == 0} {
                set name "POWER_0_POWERDOWN_VALUE"
            } else {
                set name [string toupper $arg]
            }

            set name [format "XPAR_%s" $name]
            puts $file_handle "#define $name $value"
        }
    }

    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#
# Procedure that adds static defines to xparameters.h
#
proc xstatic_defines {handle file_name args} {

    variable did_pci

    #
    # If PCI was not in the user-defined system, Linux expects this constant to
    # be defined as 0
    #
    if {! $did_pci} {
        # Open include file
        set file_handle [xopen_include_file $file_name]
        puts $file_handle "#define XPAR_PCI_0_CLOCK_FREQ_HZ    0"
        puts $file_handle "\n/******************************************************************/\n"
        close $file_handle
    }
}

#
# uart redefines
# Note: still redefine BASEADDR since Linux driver expects RX/TX register
# to be at baseaddr+0x1000
#
proc xredefine_uartns550 {drvhandle file_name ver} {
    dbg_puts "xredefine_uartns550" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "uartns550" "C_BASEADDR"

}

#
# intc redefines
#
proc xredefine_external_intc {drvhandle file_name ver} {
    dbg_puts "xredefine_external_intc for PCI interface external interrupts" "ver: $ver"

    # Open include file
    set config_inc [xopen_include_file $file_name]

    set periphs [xget_periphs $drvhandle]
    set device_id 0
    set periph_name [string toupper "intc"]

    foreach periph $periphs {

        # Get the edk based name of peripheral for printing redefines
        set edk_periph_name [xget_value $periph "NAME"]

        # Get ports that are driving the interrupt
        set source_ports [xget_interrupt_sources $periph]
        set i 0
        foreach source_port $source_ports {
            set source_port_name($i) [xget_value $source_port "NAME"]
            set source_periph($i) [xget_handle $source_port "PARENT"]
            set source_name($i) [xget_value $source_periph($i) "NAME"]
            incr i
        }
        set num_intr_inputs [xget_value $periph "PARAMETER" "C_NUM_INTR_INPUTS" ]
        for {set i 0} {$i < $num_intr_inputs} {incr i} {

            # Skip system (external) ports excluding PCI
            if {[string match -nocase $source_name($i) "syst"] == 0 && [string match -nocase $source_port_name($i) "pci"] != 0} {
                continue
            }

                #
                # Handle PCI slightly differently with A-D interrupts.  For all other
                # devices/drivers, write the redefine if the driver is not "generic"
                #

            if {[string match -nocase $source_name($i) "syst"] == 0 && [string match -nocase $source_port_name($i) "pci"] == 0} {
                set second_part [format "XPAR_%s_%s_%s_INTR" [string toupper $edk_periph_name] [string toupper $source_name($i)] [string toupper $source_port_name($i)] ]
                set first_part ""
                if {[string match {*_SBR_*} $source_port_name($i)] == 1} {
                    set first_part [format "#define XPAR_%s_%s_PCI_0_VEC_ID_SBR" $periph_name $device_id]
                    puts $config_inc "$first_part $second_part"
                }
                if {[string match {*_INTA*} $source_port_name($i)] == 1} {
                    set first_part [format "#define XPAR_%s_%s_PCI_0_VEC_ID_A" $periph_name $device_id]
                    puts $config_inc "$first_part $second_part"
        }
        if {[string match {*_INTB*} $source_port_name($i)] == 1} {
            set first_part [format "#define XPAR_%s_%s_PCI_0_VEC_ID_B" $periph_name $device_id]
            puts $config_inc "$first_part $second_part"
        }
        if {[string match {*_INTC*} $source_port_name($i)] == 1} {
            set first_part [format "#define XPAR_%s_%s_PCI_0_VEC_ID_C" $periph_name $device_id]
            puts $config_inc "$first_part $second_part"
        }
        if {[string match {*_INTD*} $source_port_name($i)] == 1} {
            set first_part [format "#define XPAR_%s_%s_PCI_0_VEC_ID_D" $periph_name $device_id]
            puts $config_inc "$first_part $second_part"
        }

            }
        }
        incr device_id
    }

    puts $config_inc "\n/******************************************************************/\n"
    close $config_inc
}

#
# Get the HW instance number for a particular device. This will be used to enumerate
# the vector ID defines if more than one interrupt from the core is connected to the
# interrupt controller.
#
proc xfind_instance {drvhandle instname} {

    set instlist [xget_periphs $drvhandle]
    set i 0
    foreach inst $instlist {
        set name [xget_value $inst "NAME"]
        if {[string compare -nocase $instname $name] == 0} {
            return $i
        }
        incr i
    }
    set i 0
    return $i
}

#
# iic redefines
#
proc xredefine_iic {drvhandle file_name ver} {
    dbg_puts "xredefine_iic" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "iic" "C_BASEADDR" "C_HIGHADDR" "C_TEN_BIT_ADR" "DEVICE_ID"

}

#
# emac redefines
#
proc xredefine_emac {drvhandle file_name ver} {
    dbg_puts "xredefine_emac" "ver: $ver"

    if {[string compare -nocase $ver "1.00.e"] == 0} {
        xredefine_include_file $drvhandle $file_name "emac" "C_BASEADDR" "C_HIGHADDR" "C_DMA_PRESENT" "C_MII_EXIST" "C_ERR_COUNT_EXIST" "DEVICE_ID"
    } else {
        xredefine_include_file $drvhandle $file_name "emac" "C_BASEADDR" "C_HIGHADDR" "C_DMA_PRESENT" "C_MII_EXIST" "C_ERR_COUNT_EXIST" "C_CAM_EXIST" "C_JUMBO_EXIST" "C_TX_DRE_TYPE" "C_RX_DRE_TYPE" "C_TX_INCLUDE_CSUM" "C_RX_INCLUDE_CSUM" "DEVICE_ID"
    }
}

#
# gemac redefines
#
proc xredefine_gemac {drvhandle file_name ver} {
    dbg_puts "xredefine_gemac" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "gemac" "C_BASEADDR" "C_HIGHADDR" "C_DMA_PRESENT" "C_MII_EXIST" "C_ERR_COUNT_EXIST" "DEVICE_ID"

}

#
# temac redefines
#
proc xredefine_temac {drvhandle file_name ver} {
    dbg_puts "xredefine_temac" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "temac" "C_BASEADDR" "C_HIGHADDR" "C_DMA_TYPE" "RXFIFO_DEPTH" "TXFIFO_DEPTH" "MAC_FIFO_DEPTH" "TX_DRE_TYPE" "RX_DRE_TYPE" "INCLUDE_TX_CSUM" "INCLUDE_RX_CSUM" "DEVICE_ID"
}

#
# lltemac redefines
#
# proc xredefine_lltemac {drvhandle file_name ver} {
#    dbg_puts "xredefine_lltemac" "ver: $ver"
#
#    xredefine_include_file $drvhandle $file_name "lltemac" "C_BASEADDR" "C_HIGHADDR" "INCLUDE_TX_CSUM" "INCLUDE_RX_CSUM" "PHY_TYPE" "DEVICE_ID"
#}

#
# gpio redefines
#
proc xredefine_gpio {drvhandle file_name ver} {
    dbg_puts "xredefine_gpio" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "gpio" "C_BASEADDR" "C_HIGHADDR" "C_IS_DUAL" "DEVICE_ID"

}

#
# sysace redefines
#
proc xredefine_sysace {drvhandle file_name ver} {
    dbg_puts "xredefine_sysace" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "sysace" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID"

}

#
# ps2 redefines
#
proc xredefine_ps2 {drvhandle file_name ver} {
    dbg_puts "xredefine_ps2" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "ps2" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID"

}

#
# uartlite redefines
#
proc xredefine_uartlite {drvhandle file_name ver} {
    dbg_puts "xredefine_uartlite" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "uartlite" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID"

}

#
# spi redefines
#
proc xredefine_spi {drvhandle file_name ver} {
    dbg_puts "xredefine_spi" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "spi" "C_BASEADDR" "C_HIGHADDR" "FIFO_EXIST" "SPI_SLAVE_ONLY" "NUM_SS_BITS" "DEVICE_ID"
}

#
# touchscreen redefines
#
proc xredefine_touchscreen {drvhandle file_name ver} {
    dbg_puts "xredefine_touchscreen" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "touchscreen" "C_BASEADDR" "C_HIGHADDR" "DEVICE_ID"
}

#
# tft redefines
#
proc xredefine_tft {drvhandle file_name ver} {
    dbg_puts "xredefine_tft" "ver: $ver"

    xredefine_include_file $drvhandle $file_name "tft" "C_BASEADDR"
}

#
# pci redefines
#
proc xredefine_pci {drvhandle file_name ver} {
    dbg_puts "xredefine_pci" "ver: $ver"

    variable did_pci

    # coalesce Memory and IO address spaces
    # open include file
    set file_handle [xopen_include_file "xparameters.h"]
    # default PCI clock frequency
    puts $file_handle "#define XPAR_XPCI_CLOCK_HZ 33333333"
    # get all peripherals connected to this driver
    set periphs [xget_periphs $drvhandle]
    set device_id 0
    foreach periph $periphs {
        # addresses for PCI config, memory and i/o space
        puts $file_handle "#define [format "%s %s" [xget_name $periph "CONFIG_ADDR"] [xget_value $periph "PARAMETER" "C_BASEADDR"]]+0x10c"
        puts $file_handle "#define [format "%s %s" [xget_name $periph "CONFIG_DATA"] [xget_value $periph "PARAMETER" "C_BASEADDR"]]+0x110"
        puts $file_handle "#define [format "%s %s" [xget_name $periph "LCONFIG_ADDR"] [xget_value $periph "PARAMETER" "C_BASEADDR"]]"
        # check for number of defined BARs
        set num_defined_bars [xget_value $periph "PARAMETER" "C_IPIFBAR_NUM"]
        if {$num_defined_bars > 0} {
            # coalesce all memory & IO address spaces
            set memspace_base_list [list]
            set iospace_base_list [list]
            set memspace_high_list [list]
            set iospace_high_list [list]
            # Rules: 1) Only supports contiguous ranges
            #        2) First range encountered is taken as the basis
            #        3) First range of any space type must be the smallest addresses
            for {set i 0} {$i < $num_defined_bars} {incr i} { # read each BAR
                # get type of space
                set param_string [format "C_IPIF_SPACETYPE_%s" $i]
                if {[xget_value $periph "PARAMETER" $param_string] == 1} { # memory space
                    # gather list of base and high addresses"
                    # get base address
                    set param_string [format "C_IPIFBAR_%s" $i]
                    lappend memspace_base_list [xget_value $periph "PARAMETER" $param_string]
                    # get high address
                    set param_string [format "C_IPIF_HIGHADDR_%s" $i]
                    lappend memspace_high_list [xget_value $periph "PARAMETER" $param_string]
                } elseif {[xget_value $periph "PARAMETER" $param_string] == 0} { # I/O space
                    # gather list of base and high addresses"
                    # get base address
                    set param_string [format "C_IPIFBAR_%s" $i]
                    lappend iospace_base_list [xget_value $periph "PARAMETER" $param_string]
                    # get high address
                    set param_string [format "C_IPIF_HIGHADDR_%s" $i]
                    lappend iospace_high_list [xget_value $periph "PARAMETER" $param_string]
                }
            }
            # coalesce address space ranges
            # memory space range
            set num_addr [llength $memspace_base_list]
            if {$num_addr > 0} { # memory spaces are defined
                set final_mem_baseaddr [lindex $memspace_base_list 0]
                set final_mem_highaddr [lindex $memspace_high_list 0]
            } else {
                puts "No memory spaces defined. Setting MEM_BASEADDR, MEM_HIGHADDR to default values"
                set final_mem_baseaddr 0xFFFFFFFF
                set final_mem_highaddr 0x00000000
            }
            for {set i 1} {$i < $num_addr} {incr i} {
                set next_base [lindex $memspace_base_list $i]
                set compare [expr $next_base - $final_mem_highaddr]
                if {$compare == 1} { # continguous range found, assign new high addr
                    set final_mem_highaddr [lindex $memspace_high_list $i]
                } elseif {$compare < 1} {
                    puts stderr "Error: Please define memory address range(s) in ascending order, starting from lower addresses."
                    puts stderr "Address range(s) before: [lindex $memspace_base_list $i] - [lindex $memspace_high_list $i] should be smaller."
                    exit
                } else {
                    puts "PCI memory address coalescing note: Ignoring address range [lindex $memspace_base_list $i] - [lindex $memspace_high_list $i]"
                }
            }

            # IO space range
            set num_addr [llength $iospace_base_list]
            if {$num_addr > 0} { # IO spaces are defined
                set final_io_baseaddr [lindex $iospace_base_list 0]
                set final_io_highaddr [lindex $iospace_high_list 0]
            } else {
                puts "No IO spaces defined. Setting IO_BASEADDR, IO_HIGHADDR to default values"
                set final_io_baseaddr 0xFFFFFFFF
                set final_io_highaddr 0x00000000
            }
            for {set i 1} {$i < $num_addr} {incr i} {
                set temp [lindex $iospace_base_list $i]
                set compare [expr $temp - $final_io_highaddr]
                if {$compare == 1} { # contiguous range found, assign new high addr
                    set final_io_highaddr [lindex $iospace_high_list $i]
                } elseif {$compare < 1} {
                    puts stderr "Error: Please define IO address range(s) in ascending order, starting from lower addresses."
                    puts stderr "Address range(s) before: [lindex $iospace_base_list $i] - [lindex $iospace_high_list $i] should be smaller."
                    exit
                } else {
                    puts "PCI IO address coalescing note: Ignoring address range [lindex $iospace_base_list $i] - [lindex $iospace_high_list $i]"
                }
            }
            incr device_id
        } else { # set address ranges to default values
            set final_mem_baseaddr 0xFFFFFFFF
            set final_mem_highaddr 0x00000000
            set final_io_baseaddr 0xFFFFFFFF
            set final_io_highaddr 0x00000000
        }
        # write address ranges to file
        puts $file_handle "#define [format "%s %s" [xget_name $periph "MEM_BASEADDR"] $final_mem_baseaddr]"
        puts $file_handle "#define [format "%s %s" [xget_name $periph "MEM_HIGHADDR"] $final_mem_highaddr]"
        puts $file_handle "#define [format "%s %s" [xget_name $periph "IO_BASEADDR"] $final_io_baseaddr]"
        puts $file_handle "#define [format "%s %s" [xget_name $periph "IO_HIGHADDR"] $final_io_highaddr]"
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle

    xredefine_include_file $drvhandle $file_name "pci" "CONFIG_ADDR" "CONFIG_DATA" "LCONFIG_ADDR" "MEM_BASEADDR" "MEM_HIGHADDR" "IO_BASEADDR" "IO_HIGHADDR" "CLOCK_HZ"
    set did_pci true
}

#
# Linux uses fixed constant names, whereas the xparameters.h as defined by XPS
# uses the hardware instance in the constant names, which can change from design
# to design.  This procedure redefines the xparameters.h constants to those that
# Linux expects.
#
proc xredefine_include_file {drv_handle file_name drv_string args} {

    # Open include file
    set file_handle [xopen_include_file $file_name]

    # Get all peripherals connected to this driver
    set periphs [xget_periphs $drv_handle]

    set pname [format "XPAR_%s_" [string toupper $drv_string]]

    # Print all parameters for all peripherals
    set device_id 0
    foreach periph $periphs {

        if {[xget_value $periph "PARAMETER" "C_IS_DUAL"] == 1 || [string compare -nocase $drv_string "ps2"] == 0} {
            # Certain peripherals, like gpio and ps2, have two peripherals in one
            set sub_periphs 2
        } else {
            set sub_periphs 1
        }

        dbg_puts "xredefine_include_file" "$drv_string sub_periphs: $sub_periphs"
        # for each sub-peripheral (most have just one)
        for {set i 0} {$i < $sub_periphs} {incr i} {
            foreach arg $args {
                set name "${pname}${device_id}_"

                if {[string compare -nocase "CLOCK_HZ" $arg] == 0} {
                    set xdrv_string [format "%s%s" "X" $drv_string]
                    set value [xget_dname $xdrv_string $arg]
                    set name "${name}CLOCK_FREQ_HZ"
                } else {
                    if {[string match C_* $arg]} {
                        set name [format "%s%s" $name [string range $arg 2 end]]
                    } else {
                        set name "${name}${arg}"
                    }
                    if {[string compare -nocase $drv_string "tft"] == 0} {
                        set value [xget_name $periph C_DCR_BASEADDR]
                    } else {
                        set value [xget_name $periph $arg]
                    }
                }

                if {($sub_periphs > 1) && (! [regexp "gpio|GPIO" $drv_string])} {
                    set value "${value}_$i"
                }

                if {[string compare -nocase "uartns550" $drv_string] == 0} {
                    if {[string compare -nocase "C_BASEADDR" $arg] == 0} {
                        set value [format "(%s%s%s)" $value "+" "0x1000"]
                        puts $file_handle "#undef $name"
                    }
                }

                puts $file_handle "#define $name $value"
                if {[string compare -nocase "DEVICE_ID" $arg] == 0} {
                    incr device_id
                }
            }
        }
    }
    puts $file_handle "\n/******************************************************************/\n"
    close $file_handle
}

#
# execute a tcl script w/o reloading the tcl intperpreter
#
# Note: we probably can just use the 'source' command instead
#
proc eval_script {script_name} {
    set fh [open "$script_name" r]
    set script_text [read $fh]
    close $fh
    eval "$script_text"
}

#
# console_device_types and console_devices are setup early in post_generate()
#
proc detect_console_ip {lib_handle} {
    global console_devices
    global console_device_types

    set console_ip ""
    set serial_console 0
    set uart16550_console 0

    #
    # When we autodetect a device, we always pick the first device of the type 
    # autodetected. E.g. When there are 2 uarts, we pick the first one.
    #
    puts "searching IP for suitable console device"
    foreach device_name [array names console_device_types] {
        #
        # any serial console wins over tft/vga console
        # uart16550 console wins over uartlite console
        #
        set type $console_device_types($device_name)
        if {$type == "tft"} {
            if {$serial_console == 0} {
                # enable console on frame buffer
                if {$console_devices($device_name) == 0} {
                    set console_ip $device_name
                    dbg_puts "detect_console_ip" "setting console on tft: $console_ip"
                }
            }
        } elseif {$type == "uartlite"} {
            if {$uart16550_console == 0} {
                if {$console_devices($device_name) == 0} {
                    set console_ip $device_name
                    dbg_puts "detect_console_ip" "setting console on uartlite: $console_ip"
                }
                set serial_console 1
            }
        } elseif {$type == "uart16550"} {
            if {$console_devices($device_name) == 0} {
                set console_ip $device_name
                dbg_puts "detect_console_ip" "setting console on uart16550: $console_ip"
            }
            set serial_console 1
            set uart16550_console 1
        }
    }
    if {$console_ip == ""} {
        puts "Warning: no ip found suitable for console, $console_ip"
    }
    return $console_ip
}

proc build_kernel_cmdline {lib_handle drvlist distribution console_ip} {
    global console_devices
    global console_device_types
    set console ""

    set rootfs_type [xget_value $lib_handle "PARAMETER" "rootfs type"]
    set rootfs_nfs_info_source [xget_value $lib_handle "PARAMETER" "NFS info source"]
    set rootfs_nfs_server [xget_value $lib_handle "PARAMETER" "NFS server"]
    set rootfs_nfs_share [xget_value $lib_handle "PARAMETER" "NFS share"]
    set rootfs_sysace_partition [xget_value $lib_handle "PARAMETER" "sysace partition"]
    set IP_address [xget_value $lib_handle "PARAMETER" "IP address"]
    set Additional_cmd_line [xget_value $lib_handle "PARAMETER" "Additional kernel command line items"]

    dbg_puts "build_kernel_cmdline" "distribution: $distribution"

    # make sure nfs share has '/' prependted to it
    if {[string compare [string index $rootfs_nfs_share 0] {/}] != 0} {
        set rootfs_nfs_share "/$rootfs_nfs_share"
    }

    if {$console_ip != ""  && [info exists console_devices($console_ip)]} {
        set tty_num $console_devices($console_ip)
        set drv [xget_swhandle $console_ip]
        set drvname [xget_value $drv "NAME"]
        if {[regexp "tft" $drvname]} {
            incr tty_num
            set console "console=tty$tty_num"
        } elseif {[regexp "uartlite" $drvname]} {
            if {$distribution == "Wind River GPP 2.0" || $distribution == "MontaVista 5.0" || $distribution == "Open Source"} {
                set console "console=ttyUL$tty_num"
            } else {
                set console "console=ttl$tty_num"
            }
        } elseif {[regexp "550" $drvname]} {
            set console "console=ttyS$tty_num,9600"
        }
        puts "setting console on $console_device_types($console_ip) ($console_ip) \[$console\]"
    } else {
        puts "Warning: 'console device' not set or not autodetected."
    }
    
    set kcmd_line ""
    if {[regexp {console[ \t]*=} $Additional_cmd_line] == 0} {
        set kcmd_line $console
    }
    if {[regexp {ip[ \t]*=} $Additional_cmd_line] == 0} {
        set kcmd_line "$kcmd_line ip=$IP_address"
    }
    set kcmd_line "$kcmd_line $Additional_cmd_line"

    #
    # I've always seen the root fs options in the command line appear last.
    # I'm not sure if this is required, but just in case ...
    #
    if {[regexp {root[ \t]*=} $Additional_cmd_line] == 0} {
        if {$rootfs_type == "sysace"} {
            puts "setting root on compat flash (sysace)"
            if {$distribution == "MontaVista 5.0" || $distribution == "Open Source"} {
                set kcmd_line "$kcmd_line root=/dev/xsa$rootfs_sysace_partition rw"
            } else {
                set kcmd_line "$kcmd_line root=/dev/xsysace$rootfs_sysace_partition rw"
            }
        } elseif {$rootfs_type == "ramdisk"} {
            puts "setting root on ramdisk"
            set kcmd_line "$kcmd_line root=/dev/ram rw"
        } elseif {$rootfs_type == "nfs"} {
            puts "setting root on nfs"
            if {$rootfs_nfs_info_source == "dhcp"} {
                set kcmd_line "$kcmd_line root=/dev/nfs rw"
            } else {
                #
                # putting ',tcp' at the end forces tcp protocol and resolves the problem with
                # small fifo's when using fifo mode ethernet.
                #
                set kcmd_line "$kcmd_line nfsroot=$rootfs_nfs_server:$rootfs_nfs_share,tcp"
                set kcmd_line "$kcmd_line rw"
            }
        }
    }
    return $kcmd_line
}

proc prj_dir {} {
    set old_cwd [pwd]
    cd "../../.."
    set _prj_dir [pwd]
    cd $old_cwd

    return [exec bash -c "basename $_prj_dir"]
}

proc dts_name {} {
    return "xilinx.dts";
}

#
# generate_os_config_update_files
#
# generate_os_config_update_files generates and copies all the files needed to
# update development hosts's .confg file.
#
# generate_os_config_update_files does the following:
# 1. generates a list of cores that are enabled in the system
# 2. appends to the above list, a set of linux specific .confg items to enable
#    This includes building up the kernel command string and sending that along
# 3. copy the config updater script
# 4. copy the total core list
# 5. copy the kernel .config dependency list
#
#
proc generate_os_config_update_files {lib_handle drvlist kcmd_line} {
    global bspdir
    global target_dir
    global board
    global distribution

    dbg_puts "generate_os_config_update_files" "%%%%%%%$$$$$$$%%%%%%%"

    ##################################################3
    # Processes the rootfs and other options
    # 1. Build kernel command line
    # 2. Enabled some other options that we need
    ##################################################3
    set enable_list ""

    #
    # A table of config items to set for each core
    #
    # The array indicies are regular expressions for each core becuase the
    # actual core names are something like 'opb_ethernet_0', and we aren't
    # interested in distinquishing cores by bus or instance number.
    #
    # The items in the array are a list of things that need to go into the
    # xps_connected.cud file for the config updater script based on the
    # presence of the core.
    #
    # "cow = change options work?"
    #
    array set driver_config_table [list \
    {fpu} "fpu" \
    {iic} "iic" \
    {\<emac} "emac" \
    {gemac} "gemac" \
    {temac} "temac" \
    {gpio} "gpio" \
    {sysace} "sysace" \
    {ps2} "ps2" \
    {^(ps2_ref)} "ps2_ref" \
    {uartlite} "uartlite" \
    {uartns550} "uart uart_cnt=4" \
    {spi} "spi" \
    {tft} "tft_ref" \
    {usb} "usb_gadget" \
    ]

    #
    # build up the list of board specific items to enable.
    #
    if {[string compare -nocase [xget_fpga_type] "virtex5"] == 0} {
        if {[string compare -nocase $board "ml510"] == 0} {
            lappend enable_list ml510
            lappend enable_list pci
            lappend enable_list ide
            lappend enable_list ide-disk
            lappend enable_list ide-cd
            lappend enable_list ide-tape
            lappend enable_list ide-pci
            lappend enable_list ide-ali15x3
            lappend enable_list scsi
            lappend enable_list usb
        } else {
            lappend enable_list ml507
        }
    } elseif {[string compare -nocase [xget_fpga_type] "virtex4"] == 0} {
        if {[string compare -nocase $board "ml410"] == 0} {
            lappend enable_list ml410
            lappend enable_list pci
            lappend enable_list ide
            lappend enable_list ide-disk
            lappend enable_list ide-cd
            lappend enable_list ide-tape
            lappend enable_list ide-pci
            lappend enable_list ide-ali15x3
            lappend enable_list scsi
            lappend enable_list usb
        } else {
            lappend enable_list ml40x
        }
    } else {
        lappend enable_list ml300
    }

    if {$distribution == "MontaVista 5.0" || $distribution == "Open Source"} {
        lappend enable_list "dts=\"[dts_name]\""
    }

    # For each periph
    foreach drv $drvlist {

        set periph [xget_value $drv "NAME"]
        dbg_puts "generate_os_config_update_files" "processing periph/drv: $periph"

        #
        # enable config items based on what's connected
        #
        # The only way, that I know right now, to search the array using
        # the regexp operation in the right direction, is to use this
        # foreach loop
        #
        foreach regex [array names driver_config_table] {
            if {[regexp $regex $periph]} {
                set item_list $driver_config_table($regex)
                foreach item $item_list {
                    puts "enabling $item in the linux kernel configuration"
                    if {[lsearch $enable_list $item] == -1} {lappend enable_list $item}
                }

                break;
            }
        }
    }


    set rootfs_type [xget_value $lib_handle "PARAMETER" "rootfs type"]
    set rootfs_ramsize [xget_value $lib_handle "PARAMETER" "ramdisk size"]

    #
    # enable root fs items
    #
    set Additional_cmd_line [xget_value $lib_handle "PARAMETER" "Additional kernel command line items"]
    if {[regexp {root[ \t]*=} $Additional_cmd_line] == 0} {
        if {$rootfs_type == "ramdisk"} {
            # enable ram root config item
            set item "root_ramdisk"
            if {[lsearch $enable_list $item] == -1} {lappend enable_list $item}
            set item "root_ramdisk_size=$rootfs_ramsize"
            if {[lsearch $enable_list $item] == -1} {lappend enable_list $item}
            set item "root_ramdisk_cnt=16"
            if {[lsearch $enable_list $item] == -1} {lappend enable_list $item}
            puts "setting root on ramdisk"
        } elseif {$rootfs_type == "nfs"} {
            # enable nfs root config item
            set item root_nfs
            if {[lsearch $enable_list $item] == -1} {lappend enable_list $item}
            puts "setting root on nfs"
        }
    }
    lappend enable_list "bootline=\"$kcmd_line\""

    # debug code
    dbg_puts "generate_os_config_update_files" "enable list:"
    foreach item $enable_list {
        dbg_puts "generate_os_config_update_files" "    $item"
    }
    dbg_puts "generate_os_config_update_files" "end enable list"
    # end debug code

    set fh [open "$target_dir/xps_connected.cud" w]
    puts $fh "# $distribution"
    foreach item $enable_list {
        puts $fh "$item"
    }
    close $fh

    #
    # the related config files are copied over from the static linux tree
    #
    if {[file isfile "$target_dir/.config"]} {
        set old_cwd [pwd]
        cd $target_dir
        if {[file isfile "cow.tcl"] == 0} {
            puts "cow.tcl not found"
        }
        eval_script "cow.tcl"
        cd $old_cwd
    }

    dbg_puts "generate_os_config_update_files" "%%%%%%%$$$$$$$%%%%%%%"
}

#
# post_generate
#
# post_generate creates a sparse linux kernel tree that can overlay a full
# linux 2.6 kernel source tree. This sparse linux kernel tree is used to
# inject drivers and hardware configuration information into a linux kernel
# source tree so that the kernel can run on the project's hardware platform.
#
# if 'target_dir' is set in the OS settings in XPS, then post_generate will
# create this sparse linux kernel tree in the 'target_dir' which could,
# in fact, be actual linux tree the user is working on.
#
# post_generate uses the parameter 'lib_handle' to access information about
# the drivers and hardware configuration as needed.
#
# post_generate performs the following main tasks:
# * append Linux Redefine preprocessor symbols to xparameters.h
# * copy driver files including makefiles and linux adapter drivers to the
#   sparse linux tree
# * 'linuxize' the source files so they better fit linux styles
#
# (see the "great big comment on how we create the BSP for linux" for more
#  detailed information on this process).
#
# post_generate does not return any value
#
# [bones 20060623]
# My understanding of the way this MLD stuff is organized is starting to
# percolate in my mind. This routine, I believe, is named based on when
# it is called. libgen will call 'generate', then it will generate some stuff
# itself (with the cooperation of the MDD files), then post_generate will be
# called.
#
# There's no magic here. The only reason to put our BSP generation code here
# in post_generate() vs generate() is that post_generate can make use of some
# files generated by libgen. In particular, we are interested in just modifying
# and copying xparameters.h.
#
# Of course, some of the functionality could be placed in generate(), but there
# is probably no compelling reason, at this time, to split it up.
#
proc post_generate {lib_handle} {
    global bspdir
    global target_dir
    global board
    global distribution
    global console_devices
    global console_device_types
    variable drvlist
    variable edk_install
   
    set distribution [xget_value $lib_handle "PARAMETER" "linux distribution"]
    set target_dir [xget_value $lib_handle "PARAMETER" "target directory"]
    set console_detection [xget_value $lib_handle "PARAMETER" "console detection"]
    set console_device [xget_value $lib_handle "PARAMETER" "console device"]

    if {$target_dir == ""} {
        set target_dir linux
    }
    puts "using target directory: $target_dir"
    if {[file isdirectory $target_dir] == 0} {
        exec bash -c "mkdir -p $target_dir"
    }

    # Get list of peripherals connected to linux
    set conn_periphs [xget_handle $lib_handle "ARRAY" "connected_periphs"]

    dbg_puts "post_generate" "conn_periphs: $conn_periphs";

    if {[string compare -nocase $conn_periphs ""] != 0} {
        set conn_periphs_elems [xget_handle $conn_periphs "ELEMENTS" "*"]

        # For each periph
        set tft_cnt 0
        set uart16550_cnt 0
        set uartlite_cnt 0
        foreach periph_elem $conn_periphs_elems {
            set periph [xget_value $periph_elem "PARAMETER" "periph_name"]
            # 1. Get driver
            set drv [xget_swhandle $periph]
            # 2. Add the driver to the list only if it's not already in the list
            set posn [lsearch -exact $drvlist $drv]
            if {$posn == -1} {
                lappend drvlist $drv
            }
            # 3. Keep track of the console devices, and their mhs ordering
            set drvname [xget_value $drv "NAME"]
            if {[string compare -nocase $drvname "tft_ref"] == 0} {
                set console_device_types($periph) "tft"
                set console_devices($periph) $tft_cnt
                incr tft_cnt
            } elseif {[string compare -nocase $drvname "uartlite"] == 0} {
                set console_device_types($periph) "uartlite"
                set console_devices($periph) $uartlite_cnt
                incr uartlite_cnt
            } elseif {[string compare -nocase $drvname "uartns550"] == 0} {
                set console_device_types($periph) "uart16550"
                set console_devices($periph) $uart16550_cnt
                incr uart16550_cnt
            }
        }
        # 
        # Determine the order of the console devices. The order depends on:
        # (1) device tree used: order is sorted alphabetical on ip name
        #     In this case, we need to reorder the devices in the
        #     console_devices  array
        # (2) device tree not used: order is based on order of devices in the
        #     mhs file.
        #     In this case, the console_devices array is already set up above.
        #
        if {$distribution == "MontaVista 5.0" || $distribution == "Open Source"} {
            set tft_cnt 0
            set uart16550_cnt 0
            set uartlite_cnt 0
            foreach device_name [lsort [array names console_device_types]] {
                set type $console_device_types($device_name)
                if {$type == "tft"} {
                    set console_devices($device_name) $tft_cnt
                    incr tft_cnt
                } elseif {$type == "uartlite"} {
                    set console_devices($device_name) $uartlite_cnt
                    incr uartlite_cnt
                } elseif {$type == "uart16550"} {
                    set console_devices($device_name) $uart16550_cnt
                    set incr uart16550_cnt
                }
            }
        }

        set file_handle [xopen_include_file "xparameters.h"]
        puts $file_handle "\n/******************************************************************/\n"
        puts $file_handle "/* Cannonical Constant Names */"
        puts $file_handle "\n/******************************************************************/\n"
        close $file_handle

        foreach drv $drvlist {

            set drvname [xget_value $drv "NAME"]
            set ver [xget_value $drv "PARAMETER" "DRIVER_VER"]

            #Redefines xparameters.h

            if {[string compare -nocase $drvname "tft_ref"] == 0} {
                xredefine_tft $drv "xparameters.h" $ver
            }

            # redefines for 16550
            if {[string compare -nocase $drvname "uartns550"] == 0} {
                 xredefine_uartns550 $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "intc"] == 0} {
                 xredefine_external_intc $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "iic"] == 0} {
# Removed 7/15/07                xredefine_iic $drv "xparameters.h" $ver
            }  elseif {[string compare -nocase $drvname "emac"] == 0} {
# Removed 7/15/07                xredefine_emac $drv "xparameters.h" $ver
            }  elseif {[string compare -nocase $drvname "gemac"] == 0} {
                xredefine_gemac $drv "xparameters.h" $ver
            #
            # Moving to cannonical names generated by the driver tcl instead
            #
            # }  elseif {[string compare -nocase $drvname "lltemac"] == 0} {
            #     xredefine_lltemac $drv "xparameters.h" $ver
            }  elseif {[string compare -nocase $drvname "temac"] == 0} {
# Removed 7/15/07                xredefine_temac $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "gpio"] == 0} {
               xredefine_gpio $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "sysace"] == 0} {
# Removed 7/15/07                xredefine_sysace $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "ps2_ref"] == 0} {
                xredefine_ps2 $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "uartlite"] == 0} {
# Removed 7/15/07                xredefine_uartlite $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "spi"] == 0} {
# Removed 7/15/07                xredefine_spi $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "touchscreen_ref"] == 0} {
                xredefine_touchscreen $drv "xparameters.h" $ver
            } elseif {[string compare -nocase $drvname "pci"] == 0} {
                xredefine_pci $drv "xparameters.h" $ver
            }
        }
    } else {
        puts "Warning: connected_periphs list is empty. This is probably not what you want."
    }

    # define core_clock, plb_clock, mem_size
    xredefine_params $lib_handle "xparameters.h" "uart16550 bus clock freq" "CORE_CLOCK_FREQ_HZ" "memory size"

    # define the values for the persistent storage in IIC, if any IIC device is existing in the system
    foreach drv $drvlist {
        set drvname [xget_value $drv "NAME"]
        if {[string compare -nocase $drvname "iic"] == 0} {
                xredefine_params $lib_handle "xparameters.h" "IIC persistent baseaddr" "IIC persistent highaddr" "IIC persistent eepromaddr"
        }
    }

    # define the powerdown values if "powerdown baseaddr" exists
    if {[xget_value $lib_handle "PARAMETER" "powerdown baseaddr"] != ""} {
        xredefine_params $lib_handle "xparameters.h" "powerdown baseaddr" "powerdown highaddr" "powerdown value"
    }

    xstatic_defines $lib_handle "xparameters.h"

    # Create Linux tree structure
    set pwd [pwd]
    set simple "$target_dir/arch/ppc/boot/simple"
    set xparameters "$target_dir/arch/ppc/platforms/4xx/xparameters"

    set xilinx_common "$target_dir/drivers/xilinx_common"
    set block "$target_dir/drivers/block"
    set char "$target_dir/drivers/char"
    set i2c "$target_dir/drivers/i2c"
    set net "$target_dir/drivers/net"
    set kernel "$target_dir/arch/ppc/kernel"


    if {$distribution == "MontaVista 5.0" || $distribution == "Open Source"} {
        exec bash -c "mkdir -p $target_dir/arch/powerpc/boot/dts/"
        exec bash -c "mkdir -p $xparameters"
    } else {
        exec bash -c "mkdir -p $simple;mkdir -p $xparameters;mkdir -p $xilinx_common;mkdir -p $kernel"
        exec bash -c "mkdir -p $block;mkdir -p $char;mkdir -p $i2c;mkdir -p $net"
    }

    # if board parameter is specified, copy board-specific files
    set board [xget_value $lib_handle "PARAMETER" "PCI Board"]

    #
    # copy all the drivers and their dependencies
    #
    if {$distribution != "MontaVista 5.0" && $distribution != "Open Source"} {
        xcopy_drivers $drvlist
    }


    #
    # Move xparameters.h around
    #
    set xparameters_file_name "xparameters_ml300.h"
    if {[string compare -nocase [xget_fpga_type] "virtex5"] == 0} {
        set xparameters_file_name "xparameters_ml507.h"
        if {[string compare -nocase $board "ml510"] == 0} {
            set xparameters_file_name "xparameters_ml510.h"
        }     
    } elseif {[string compare -nocase [xget_fpga_type] "virtex4"] == 0} {
        set xparameters_file_name "xparameters_ml40x.h"
        if {[string compare -nocase $board "ml410"] == 0} {
            set xparameters_file_name "xparameters_ml410.h"
        } elseif {$distribution == "Wind River GPP 2.0"} {
            set xparameters_file_name "xparameters_ml403.h"
        }
    }
    exec bash -c "cp ../../include/xparameters.h $xparameters/$xparameters_file_name"
    exec bash -c "rm -f $xparameters/xparameters.h"

    #
    # static linux tree is copied based on the distribution chosen
    #
    set distdir "dist/linux"
    if {$distribution == "Wind River GPP 1.3"} {
        set distdir "dist/wrl1.3"
    } elseif {$distribution == "Wind River GPP 2.0"} {
        set distdir "dist/wrl2.0"
    } elseif {$distribution == "MontaVista 4.01"} {
        set distdir "dist/mvl4.01"
    } elseif {$distribution == "MontaVista 5.0"} {
        set distdir "dist/mvl5.0"
    } elseif {$distribution == "Open Source"} {
        set distdir "dist/linux"
    }

    # copy static linux tree
    if {[file isdirectory "$bspdir/$distdir"] == 1} {
        exec bash -c "cp -rf $bspdir/$distdir/* $target_dir/"
    } else {
        puts "distribution specific portion, $bspdir/$distdir, does not exist, skipping ..."
    }

    if {$console_detection == "Autodetect"} {
        if {[string compare -nocase $conn_periphs ""] != 0} {
            set console_device [detect_console_ip $lib_handle]
        } else {
            puts "Warning: connected_periphs list is empty. Autodetection of console device is inoperable."
            set console_device ""
        }
    }

    dbg_puts "post_generate" "about to call build_kernel_cmdline"
    #
    #  Generate .config update stuff
    #
    set kcmd_line [build_kernel_cmdline $lib_handle $drvlist $distribution $console_device]

    if {$distribution != "MontaVista 5.0" && $distribution != "Open Source"} {
        generate_os_config_update_files $lib_handle $drvlist $kcmd_line
        exec bash -c "chmod +x $target_dir/xmake"
    }

    #
    #  Generate the device tree, if needed
    #
    
    if {$distribution == "MontaVista 5.0" || $distribution == "Open Source"} {
        variable device_tree_MLD_version
	#
	# Open Source Linux device_tree MLD doesn't have a version number in its name
	#
        if {$distribution == "Open Source"} {
            set device_tree_MLD_version "device-tree"
        }
	dbg_puts "post_generate" "device_tree MLD: $device_tree_MLD_version"
        namespace eval device_tree source "[device_tree_bspscript $device_tree_MLD_version]"

        device_tree::generate_device_tree "$target_dir/arch/powerpc/boot/dts/[dts_name]" $kcmd_line $console_device
    }

    # Change permissions to RW
#    exec bash -c "chmod -Rf +w $target_dir"
}

#
# xcopy_drivers
#
# xcopy_drivers copies all the driver files over to the sparse linux kernel
# source tree based on the list of drivers selected in XPS (think
# 'connected_periphs') given in the parameter 'drvlist'.
#
# xcopy_drivers copies each driver based on the driver table within this
# xcopy_drivers procedure. The format of this driver table is described below.
#
# In addition to copying the drivers, xcopy_drivers will copy the dependency
# drivers into the drivers/xilinx_common directory.
#
# xcopy_drivers will call xcopy_driver on each driver to accomplish the actual
# driver copying.
#
# xcopy_drivers does not return any value.
#
proc xcopy_drivers {drvlist} {
    global target_dir
    global distribution
    variable driver_default_versions

    set iic_dirname ''
    #
    # Format for each array entry is as follows:
    # indexed by core base name
    # version {version list} targetdir {directory string} warn {warning message}
    #   version list: a tcl list of regular expressions for driver versions
    #   targetdir: directory to which the driver will be copied
    #   warn: a warning message if there is no version match
    #
    array set driver_info_list [list \
    {iic} [list version [list ".*"] targetdir "drivers/i2c/algos/xilinx_iic" warn ""] \
    {emac} [list version [list ".*"] targetdir "drivers/net/xilinx_emac" warn ""] \
    {gemac} [list version [list ".*"] targetdir "drivers/net/xilinx_gemac" warn ""] \
    {llfifo} [list version [list ".*"] targetdir "drivers/xilinx_common" warn ""] \
    {lldma} [list version [list ".*"] targetdir "drivers/xilinx_common" warn ""] \
    {lltemac} [list version [list ".*"] targetdir "drivers/net/xilinx_temac" warn ""] \
    {temac} [list version [list ".*"] targetdir "drivers/net/xilinx_temac" warn ""] \
    {gpio} [list version [list ".*"] targetdir "drivers/char/xilinx_gpio" warn ""] \
    {sysace} [list version [list ".*"] targetdir "drivers/block/xilinx_sysace" warn ""] \
    {ps2_ref} [list version [list ".*"] targetdir "drivers/input/serio/xilinx_ps2" warn ""] \
    {uartlite} [list version [list ".*"] targetdir "drivers/char/xilinx_uartlite" warn ""] \
    {spi} [list version [list ".*"] targetdir "drivers/char/xilinx_spi" warn ""] \
    {tft_ref} [list version [list ".*"] targetdir "drivers/char/xilinx_ts" warn ""] \
    {usb} [list version [list ".*"] targetdir "drivers/usb/gadget/xilinx_usb" warn ""] \
    ]

    foreach drv $drvlist {
        set drvname [xget_value $drv "NAME"]
        set ver [xget_value $drv "PARAMETER" "DRIVER_VER"]
        set ver [string map {. _} $ver]
        set dirname [format "%s_v%s" $drvname $ver]

        if {[lsearch [array names driver_info_list] $drvname] > -1} {
            set drv_info $driver_info_list($drvname)
            array set tmp_ar $drv_info;
            dbg_puts "xcopy_drivers" "$drvname"
            dbg_puts "xcopy_drivers" "$tmp_ar(version), $ver"
            set supported 0
            dbg_puts "xcopy_drivers" "xcopy $drvname $dirname"
            foreach supp_ver $tmp_ar(version) {
                if {[regexp $supp_ver $ver]} {
                    set dstdirname [format "%s%s" "$target_dir/" $tmp_ar(targetdir)]
                    set supported 1

                    puts "copying $dirname to $tmp_ar(targetdir)"
                    xcopy_driver $dirname $tmp_ar(targetdir)

                    #
                    # if the driver is uartlite, copy over the generated _g.c file.
                    # uartlite is a special case
                    #
                    if {[string compare -nocase $drvname "uartlite"] == 0} {
                        dbg_puts xcopy_drivers "copying uartlite configuration [pwd]/../$dirname/src/*_g.c to $dstdirname"
                        exec bash -c "cp -f ../$dirname/src/xuartlite_g.c $dstdirname"
                    }
                    #
                    # if the driver is iic remember it's location so we can later copy xiic_l.c into
                    # arch/ppc/boot/simple directory.
                    #
                    if {[string compare -nocase $drvname "iic"] == 0} {
                        set iic_dirname $dirname
                    }

                    if {[info exists driver_default_versions($drvname)]} {
                        unset driver_default_versions($drvname)
                    }

                    #
                    # xltype_file puts the GPL header on files
                    #
                    xltype_file $dstdirname

                    break;
                }
            }

            if {($supported == 0) && [string length $tmp_ar(warn)]} {
                puts "WARNING: $drvname driver version $ver is not supported with $distribution"
                puts "         $tmp_ar(warn)"
            }

        }
        #
        # The depends list is just a list of names, so we dont' get a regular
        # driver handle. In addition the depends list can have duplicate
        # entries between various drivers, so we use an array to record unique
        # depends items and then process the array below. This way we don't
        # copy driver files more than once.
        #
        set depends [xget_value $drv "OPTION" "DEPENDS"]
        foreach dep $depends {
            set deps($dep) 1
        }
    }
    #
    # Copy default driver versions if not already copied.
    # driver_default_versions should only contain items that have not yet been
    # copied.
    #
    foreach driver [array names driver_default_versions] {
            puts "copying default driver: $driver"

        set drv_info $driver_info_list($driver)
        array set tmp_ar $drv_info;
        set dstdirname [format "%s%s" "$target_dir/" $tmp_ar(targetdir)]
        set dirname $driver_default_versions($driver)

            #
            # if the driver is iic remember it's location so we can later copy xiic_l.c into
            # arch/ppc/boot/simple directory.
            #
            if {[string compare -nocase $driver "iic"] == 0} {
                set iic_dirname $dirname
            }

        puts "copying $dirname to $tmp_ar(targetdir)"
        xcopy_driver $dirname $tmp_ar(targetdir)
        xltype_file $dstdirname
    }

    #
    # copy xiic_l.c to arch/ppc/boot/simple
    #
    variable edk_install
    dbg_puts xcopy_drivers "copying xiic_l.c to $target_dir/arch/ppc/boot/simple"
    set srcdir ""
    set driverdir_install "$edk_install/sw/XilinxProcessorIPLib/drivers"
    set driverdir_project "../../../drivers"

    if {[file isdirectory "$driverdir_project/$iic_dirname"] == 1} {
        set srcdir "$driverdir_project/$iic_dirname/src"
    } elseif {[file isdirectory "$driverdir_install/$iic_dirname"] == 1} {
        set srcdir "$driverdir_install/$iic_dirname/src"
    }

    if {[string length $srcdir]} {
        exec bash -c "cp -f $srcdir/xiic_l.c $target_dir/arch/ppc/boot/simple"
        #
        # xltype_file puts the GPL header on files
        #
        xltype_file $target_dir/arch/ppc/boot/simple/xiic_l.c
    }

    #
    # Add some permanent items to the dependency list, so linux will always build
    #
    set deps(dma_v2_00_a) 1
    set deps(dma_v3_00_a) 1

    foreach dep [array names deps] {
        puts "copying dependency driver $dep to drivers/xilinx_common"
        xcopy_driver $dep drivers/xilinx_common
    }

    # if xparameters.h is in the xilinx_common directory, remove it
    if {[file isfile "$target_dir/drivers/xilinx_common/xparameters.h"] == 1} {
        exec bash -c "rm -f $target_dir/drivers/xilinx_common/xparameters.h"
    }

    #
    # xltype_file puts the GPL header on files
    #
    set dstdirname [format "%s%s" "$target_dir/" drivers/xilinx_common]
    xltype_file $dstdirname
}


#}

#
# xcopy_dir
#
# xcopy_dir copies the driver files, identified by 'srcdir', to a target
# directory given in 'dstdir'. xcopy_dir ensures that any related Makefiles.
#
# If the parameter 'args' is the keyword, 'recursive', xcopy_dir recursively
# copies the directory tree starting at 'srcdir'. Note that 'args' is an
# optional parameter.
#
# xcopy_dir exits before attempting to copying any files if the directory
# '$srcdir' cannot be found.
#
# xcopy_dir does not return any value
#
proc xcopy_dir {srcdir dstdir args} {
    global target_dir
    set basename [exec bash -c "basename $srcdir"]
    set dstdirname [format "%s%s" "$target_dir/" $dstdir]
    set cp_opts ""

    if {[lsearch $args "recur*"] > -1} {
        set cp_opts "-r"
    }

    #
    # Find the source directory. Check absolute first, then relative.
    #
    if {[file isdirectory "$srcdir"] == 1} {
        set source_dir $srcdir
    } elseif {[file isdirectory "../$srcdir"] == 1} {
        set source_dir "../$srcdir"
    } else {
        dbg_puts "xcopy_dir" "$srcdir does not exist, skipping ..."
        return
    }

    #
    # Copy files from src to dst
    #
    exec bash -c "mkdir -p $dstdirname"

    #
    # We check to see if $srcdir/'src' exists first becuase this routine is
    # used to copy mld driver dirs too, which may just contain build files.
    #
    if {[file isdirectory "$srcdir/src"] == 1} {
        dbg_puts "xcopy_dir" "cp -f $cp_opts $source_dir/src/*.\[ch\] $dstdirname"
        exec bash -c "cp -f $cp_opts $source_dir/src/*.\[ch\] $dstdirname"
    }

    #
    # If there's a Makefile here, copy it over too
    #
    if {[file isfile "$source_dir/build/linux2_6/Makefile"] == 1} {
        dbg_puts "xcopy_dir" "cp -f $cp_opts $source_dir/build/linux2_6/Makefile $dstdirname"
        exec bash -c "cp -f $cp_opts $source_dir/build/linux2_6/Makefile $dstdirname"
    }
}

#
# xcopy_driver
#
# xcopy_driver copies the driver files, identified by 'dirname', to a target
# directory given in 'dstdir'.  xcopy_driver ensures that the driver files in
# the MLD overlay over the regular driver files. xcopy_driver calls xcopy_dir
# twice, once to copy the driver and second to overlay the files from the MLD.
#
# xcopy_driver passes along the parameter 'args' to calls to xcopy_dir. The
# comments for xcopy_dir describes how 'args' is used.
#
# An associated adapter driver, if exists in the MLD, is also copied over.
# main driver to adapter driver associations are found in the global variable,
# driver_adapter_list.
#
# Since the project can override any driver, each time, xcopy_driver must check
# the project's driver area for the driver and copy the project's driver
# instead of the one in the install area.
#
# The location of the MLD is already predetermined earlier in the script and
# used through the global variable 'bspdir'
#
# The location of the EDK install is already predetermined earlier in the script and
# used through the global variable 'edk_install'
#
# xcopy_driver does not return any value.
#
proc xcopy_driver {dirname dstdir args} {
    global bspdir
    global target_dir
    variable edk_install
    variable driver_adapter_list

    set dstdirname [format "%s%s" "$target_dir/" $dstdir]
    set driverdir_install "$edk_install/sw/XilinxProcessorIPLib/drivers"
    set driverdir_project "../../../drivers"


    dbg_puts "xcopy_driver" "testing existence of: $driverdir_project/$dirname"
    dbg_puts "xcopy_driver" "testing existence of: $driverdir_install/$dirname"

    set srcdir ""
    if {[file isdirectory "$driverdir_project/$dirname"] == 1} {
        set srcdir "$driverdir_project/$dirname"
    } elseif {[file isdirectory "$driverdir_install/$dirname"] == 1} {
        set srcdir "$driverdir_install/$dirname"
    }

    if {[string length $srcdir]} {
        puts "copying driver files: $srcdir -> $dstdir"
        xcopy_dir $srcdir $dstdir $args
    }

    dbg_puts "xcopy_driver" "testing existence of: $bspdir/drivers/$dirname"
    if {[file isdirectory "$bspdir/drivers/$dirname"] == 1} {
        puts "overlaying bsp driver files: $bspdir/drivers/$dirname -> $dstdir"
        xcopy_dir "$bspdir/drivers/$dirname" $dstdir $args
    }

    #
    # Check to see if an adapter for this driver exists in the bsp directory also.
    # Linux distribution provides it by default.
    #
    set drv_adapter_el [array get driver_adapter_list $dirname]
    dbg_puts "xcopy_driver" "dirname: $dirname, drv_adapter_el: $drv_adapter_el"
    if {[llength $drv_adapter_el] > 0} {
        set drv_adapter [lindex $drv_adapter_el 1]

        if {[file isdirectory "$bspdir/drivers/$drv_adapter"] == 1} {
            puts "copying adapter driver $bspdir/drivers/$drv_adapter to $dstdir"
#            xcopy_driver $bspdir/drivers/$drv_adapter $dstdir $args
            xcopy_driver $drv_adapter $dstdir $args
        }
    }
}

proc xltype_file {filename} {

    global bspdir

    set ltypes [file join "$bspdir/data" "Ltypes"]

    dbg_puts "xltype_file" "Re-licensing files to GPL: $filename"

    if {[file isdirectory $filename]} {
        foreach entry [glob -nocomplain [file join $filename *]] {
            xltype_file $entry
        }
    } else {
        # operate only on ASM and C/C++ source files
        if {[regexp {\.(c|h|S|cpp|hpp)$} $filename]} {
            exec bash -c "$ltypes $filename"
        }
    }
}

