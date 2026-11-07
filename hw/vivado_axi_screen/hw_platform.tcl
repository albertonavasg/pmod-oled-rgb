# Create Vivado project
source [file join [file dirname [info script]] axi_screen.tcl]

# Run synthesis and implementation.
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1

# Check implementation status before exporting.
set status [get_property STATUS [get_runs impl_1]]
if {![string match "*Complete!*" $status]} {
    error "Implementation failed: $status"
}

# Export the hardware platform, including the bitstream.
set output_dir [file join [file dirname [info script]] build]
file mkdir $output_dir

write_hw_platform \
    -fixed \
    -include_bit \
    -force \
    -file [file join $output_dir screen.xsa]
