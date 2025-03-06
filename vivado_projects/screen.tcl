#*****************************************************************************************
# Vivado (TM) v2024.1 (64-bit)
#
# screen.tcl: Tcl script for re-creating project 'screen'
#
#*****************************************************************************************

################################################################################
# define names
################################################################################

set project_name screen

################################################################################
# define paths
################################################################################

# Set the reference directory for source file relative paths (by default the value is script directory path)
set origin_dir 		"."

set path_design $origin_dir/../src/vhdl/design
set path_tb     $origin_dir/../src/vhdl/testbench
set path_constr $origin_dir/../src/vhdl/constraint

set path_board  $origin_dir/../pynq-z2

# Set the directory path for the original project from where this script was exported
set orig_proj_dir "[file normalize "$origin_dir/$project_name"]"

################################################################################
# setup the project
################################################################################

# Create project
create_project $project_name $origin_dir/$project_name -part xc7z020clg400-1 -force

# Set the directory path for the new project
set proj_dir [get_property directory [current_project]]

# Set project properties
set obj [current_project]
set_property -name "board_part_repo_paths" -value "[file normalize "$path_board"]" -objects $obj
set_property -name "board_part" -value "tul.com.tw:pynq-z2:part0:1.0" -objects $obj
set_property -name "platform.board_id" -value "pynq-z2" -objects $obj
set_property -name "default_lib" -value "xil_defaultlib" -objects $obj
set_property -name "ip_cache_permissions" -value "read write" -objects $obj
set_property -name "sim.ip.auto_export_scripts" -value "1" -objects $obj
set_property -name "simulator_language" -value "Mixed" -objects $obj
set_property "target_language" "VHDL" $obj
set_property "xpm_libraries" "XPM_CDC XPM_MEMORY" $obj

################################################################################
# setup sources
################################################################################

# Create 'sources_1' fileset (if not found)
if {[string equal [get_filesets -quiet sources_1] ""]} {
  create_fileset -srcset sources_1
}

# Add design files
add_files -fileset sources_1 $path_design/spi_master.vhd
add_files -fileset sources_1 $path_design/screen_controller.vhd
add_files -fileset sources_1 $path_design/screen_tester.vhd
add_files -fileset sources_1 $path_design/top.vhd

################################################################################
# setup constraints
################################################################################

# Create 'constrs_1' fileset (if not found)
if {[string equal [get_filesets -quiet constrs_1] ""]} {
  create_fileset -constrset constrs_1
}

# Add constraint files
add_files -fileset constrs_1 $path_constr/screen.xdc

################################################################################
# setup simulation
################################################################################

# Create 'sim_1' fileset (if not found)
if {[string equal [get_filesets -quiet sim_1] ""]} {
  create_fileset -simset sim_1
}

# Add testbench files
add_files -fileset sim_1 $path_tb/spi_master_tb.vhd
add_files -fileset sim_1 $path_tb/screen_controller_tb.vhd
add_files -fileset sim_1 $path_tb/screen_tester_tb.vhd

################################################################################
# finish setup of the full project
################################################################################

# Update the compile order
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1

puts "INFO: Project created:${project_name}"

