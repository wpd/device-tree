###############################################################################
#
#       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
#       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
#       SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
#       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
#       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
#       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
#       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
#       FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
#       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
#       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
#       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
#       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#       FOR A PARTICULAR PURPOSE.
#
#       (c) Copyright 2005-2006 Xilinx Inc.
#       All rights reserved.
#
#
# ---------------------------------------------------------------------------
#
# This TCL script updates Linux kernel configuration file (i.e., .config)
# based on a specific XPS design.
#
# All of the following input files are needed for this script to work
# correctly.
#
#   .config           --- an existing configuration file as the base to update
#   xilinx_all.cud    --- to specify all devices/features supported
#                         by Xilinx
#   xps_connected.cud --- to specify what is included in the current XPS design
#   enable_dep.cud    --- to specify which .config item(s) to enable based
#                         on a feature turned on in the current XPS design
#   disable_dep.cud   --- to specify which .config item(s) to disable
#                         if a device/feature is disabled in the current 
#                         XPS design
#
# All input files, except the first one, are created by Xilinx Linux MLD tools
# and should not be modified manually.
#
# ---------------------------------------------------------------------------
#
# MODIFICATION HISTORY
#
# Ver   Who  Date     Changes
# ----- ---- -------- -------------------------------------------------------
# 1.00a xd   08/08/06 First release
#
###############################################################################

###############################################################################
#
# parse a input file from EDK. This file could contains 
#   1. all Xilinx related device list, or 
#   2. all devices a XPS design contains along with board type, boot 
#      line and root file system location info
#
###############################################################################
proc parse_edk_input {design_items filename} {
  
    upvar $design_items item_array
    
    # open file
    set in_file [open $filename "r"]

    while {![eof $in_file]} {

        # read a line
        set length [gets $in_file item]

        # parse the line if not empty or not comments started with "#"
        if {[expr $length >= 1] && [expr [regexp -nocase "^(\[ \t\]*)#(.*)" $item] == 0] } {

            # if a "=" sign is in the current item, it is string value
            set equal_pos [string first "=" $item]
            if { $equal_pos > 0 } {

                # the current item has a string value
                set index [string range $item 0 [expr $equal_pos-1]]
                set val [string range $item [expr $equal_pos+1] [expr $length-1]]
                set item_array($index) $val

            } else {
                
                # the current item do not have string value
                set item_array($item) YES
            }
        }
    }

    # close file
    close $in_file
}

###############################################################################
#
# parse dependency file and find out all according .config items depending on 
# XPS design items
#
###############################################################################
proc map_to_config_items {design_items config_items filename} {

    upvar $design_items dsn_items
    upvar $config_items cfg_items

    # open file
    set depend [open $filename "r"]
    
    while {![eof $depend]} {

        # read a line
        set length [gets $depend item]

        # parse the line if not empty or not comments started with "#"
        if {[expr $length >= 1] && [expr [regexp -nocase "^(\[ \t\]*)#(.*)" $item] == 0] } {

            # build a list given the line
            set lst [split $item]

            # find out if the xps_design uses current dependency info 
            set depender [lindex $lst 1]
            set dep_found [array names dsn_items $depender]
            if { [string length $dep_found] >= 1 } {

                switch [lindex $lst 0] {
                    {boolean} {
                    
                        # boolean type. use 'y' as value.
                        for {set i 2} {$i < [llength $lst]} {incr i} {
                            set cfg_items([lindex $lst $i]) y
                        }
                    }
                    {string} {
                    
                        # string type. use value found in xps_design entry
                        # if no forced-value (specified by "=") is found.
                        for {set i 2} {$i < [llength $lst]} {incr i} {
                            set complex_item [lindex $lst $i]
                            
                            # if a "=" sign is in the current item, it has default value.
                            # use the specified value if existing. otherwise use default value
                            #
                            set equal_pos [string first "=" $complex_item]
                            if { $equal_pos > 0 } {
                                
                                set index [string range $complex_item 0 [expr $equal_pos-1]]
                                set dft_val [string range $complex_item [expr $equal_pos+1] end]
                                
                                if { [string match YES $dsn_items($depender)] == 0} {
                                    set cfg_items($index) $dsn_items($depender)
                                } else {
                                    set cfg_items($index) $dft_val
                                }
                            } elseif { [string length $complex_item] > 0} {
                                set cfg_items($complex_item) $dsn_items($depender)
                            }
                        }
                    }
                    default   { 
                        puts "invalid dependency list format. Abort" 
                        exit 2
                    }
                }
            }
        }
    }
    
    # close file
    close $depend
}


###############################################################################
#
# Go through each line of old config file and create new config file
# with items in a input list enabled. The original file will not be changed
#
###############################################################################
proc enable_config_items {config_items_to_enable in_filename out_filename} {

    upvar $config_items_to_enable to_enable_items
    
    # open files
    set in_file [open $in_filename "r"]
    set out_file [open $out_filename "w"]
    
    # set option of output file so linefeed is used as newline character
    fconfigure $out_file -translation lf
    
    while {![eof $in_file]} {
        
        # reset flag
        set to_enable 0
        
        # read a line
        set length [gets $in_file line]
        
        # handle the line if not empty
        if { $length >= 0} {

        
            # is current line a comment line?
            set pound_sign_pos [string first "#" $line]

            if { $pound_sign_pos == 0 } {
            
                # check if the current line contains a disabled item
                if { [regexp -nocase {(.*)(is not set$)} $line] == 1 } {

                    # a disabled item found
                    scan $line {# %s is not set} item

                    # check if updating is needed
                    set cfg_itm_found [array names to_enable_items $item]
                    if { [string length $cfg_itm_found] >= 1 } {
                        set to_enable 1
                    } 
                } 

            } else {

                # enabled item line or blank line. enabled item line contains "="
                set equal_pos [string first "=" $line]
                if { $equal_pos > 0 } {

                    # enabled item line
                    set item [string range $line 0 [expr $equal_pos-1]]

                    # check if updating is needed
                    set cfg_itm_found [array names to_enable_items $item]
                    if { [string length $cfg_itm_found] >= 1 } {
                        set to_enable 1
                    } 
                }
            }
        
            # output a updated line if an item needs to be enabled and the item
            # was not output before, or the original line otherwise.
            #
            if { $to_enable == 1 } {

                if { [string match "USED" $to_enable_items($item)] == 0 } {
                    
                    puts $out_file "$item=$to_enable_items($item)"

                    # mark the array element as used
                    set to_enable_items($item) "USED"
                }
        
            } else {

                # no update needed. output the original line
                puts $out_file $line
            }
        }
    }
    
    # close files
    close $in_file
    close $out_file
    
    # append config items that should be enabled but not found in .config
    append_unfound_items to_enable_items $out_filename

}

###############################################################################
#
# Some config items should be enabled but not found in the old config 
# file. Append those items to the new config file.
#
###############################################################################
proc append_unfound_items {config_items_to_enable out_filename} {

    upvar $config_items_to_enable to_enable_items

    set out_file [open $out_filename "a+"]
        
    # reset flag that tells if the comment title is printed or not
    set append_title_flag 0
    set append_title "\n#\n# Item(s) added by Xilinx .config updater script\n#"
    
    foreach index [array names to_enable_items *] {
        if { [string match "USED" $to_enable_items($index)] == 0 } {

            #output a title if not done yet
            if { $append_title_flag == 0 } {
                puts $out_file $append_title
                set append_title_flag 1
            }

            # append the current found item
            puts $out_file "$index=$to_enable_items($index)"
        }
    }
    
    # close file
    close $out_file
}

###############################################################################
#
# Go through each line of old config file and create new config file
# with items in a input list disabled. The original file will not be changed
#
###############################################################################
proc disable_config_items {config_items_to_disable in_filename out_filename} {

    upvar $config_items_to_disable to_disable_items
    
    # open files
    set in_file [open $in_filename "r"]
    set out_file [open $out_filename "w"]

    # set option of output file so linefeed is used as newline character
    fconfigure $out_file -translation lf
    
    while {![eof $in_file]} {
        
        # reset flag
        set to_disable 0
        
        # read a line
        set length [gets $in_file line]
        
        # handle the line if not empty
        if { $length >= 0} {
        
            # we don't need to handle comment lines
            set pound_sign_pos [string first "#" $line]

            if { $pound_sign_pos < 0 } {

                # enabled item line or blank line. enabled item line contains "="
                set equal_pos [string first "=" $line]
                if { $equal_pos > 0 } {

                    # enabled item line
                    set item [string range $line 0 [expr $equal_pos-1]]

                    # check if updating is needed
                    set cfg_itm_found [array names to_disable_items $item]
                    if { [string length $cfg_itm_found] >= 1 } {
                        set to_disable 1
                    } 
                }
            }
        
            # output a updated line if an item needs to be disabled and the item
            # was not output before, or the original line otherwise.
            #
            if { $to_disable == 1 } {

                if { [string match "USED" $to_disable_items($item)] == 0 } {
                    
                    puts $out_file "# $item is not set"

                    # mark the array element as used
                    set to_disable_items($item) "USED"
                }
        
            } else {

                # no update needed. output the original line
                puts $out_file $line
            }
        }
    }
    
    # close files
    close $in_file
    close $out_file
}

###############################################################################
#
# Check the existence of a file
#
###############################################################################
proc check_file_existance { filename } {
    if { [file isfile $filename] == 0 } {
        puts "Input file \"$filename\" is missing! Abort."
        exit 1
    }
}

###############################################################################
#
# Main script
#
###############################################################################

puts -nonewline "Updating Linux .config file ...... "
flush stdout

# check the existence of all necessary input files

check_file_existance "xilinx_all.cud"
check_file_existance "xps_connected.cud"
check_file_existance "disable_dep.cud"
check_file_existance "enable_dep.cud"
check_file_existance ".config"

# disable all Xilinx related .config items first

parse_edk_input      xilinx_all_items "xilinx_all.cud"
map_to_config_items  xilinx_all_items config_items_to_disable "disable_dep.cud"
set file_bak_name    ".config.bak.[clock format [clock seconds] -format "%B_%d_%Y_%H_%M_%S"]"
file rename          ".config" $file_bak_name
disable_config_items config_items_to_disable $file_bak_name ".config.dis"

# enable all .config items only needed by a specific XPS design

parse_edk_input      xps_design_items "xps_connected.cud"
map_to_config_items  xps_design_items config_items_to_enable "enable_dep.cud"
enable_config_items  config_items_to_enable ".config.dis" ".config"

puts "Done."
