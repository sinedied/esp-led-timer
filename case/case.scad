/* [Dimensions] ---------------------------------------- */

// LCD panel width
panel_width = 260;

// LCD panel height
panel_height = 128;

// LCD panel depth
panel_depth = 13;

// Wall thickness
wall_width = 3;

// LCD panel back border
inside_border_width = 11.5;

// Space in front of the panel for protection
front_bump = 2;

// Space behind panel for the circuit
circuit_depth = 15;

/* [Hole for usb cable] -------------------------------- */

// Usb hole width
usb_hole_width = 14;

// Usb hole height
usb_hole_height = 8;

// Usb hole centered offset from right side of the panel
usb_hole_offset_x = 95;

// Usb hole centered offset from bottom of the panel
usb_hole_offset_y = 18;

// Cable diameter
cable_diameter = 2.8;

// Cable holder height
cable_holder_height = 5;

/* [Space and support for button] ---------------------- */

// Push button inner diameter
button_inner_diameter = 11.5;

// Push button outer diameter
button_diameter = 12.8;

// Button switch side width
button_hold_size = 12;

// Space for the button legs and wires
button_side_space = 2;

// Total height of switch + push button (excl. legs)
button_total_height = 12;

// Push button height (excl. switch body)
button_push_height = 8.5;

// Button hole centered offset from right side of the panel
button_offset_x = 95;

// Extra button support height
button_extra_height = 10;

// Connector width
button_connector_width = 2;

// Connector height
button_connector_height = 6;

// Connector depth
button_connector_depth = 6;

/* [Lock tabs] ----------------------------------------- */

// Lock tabs thickness
lock_tab_thickness = 0.4;

// Lock tabs base width
lock_tab_size = 20;

/* [Angle support] ------------------------------------- */

// Target panel angle
angle = 50;

// Support width
angle_support_width = 20;

// Support min height
angle_support_min_height = 3;

// Support extra depth
angle_support_extra_depth = 30;

// Offset from each side
angle_support_offset_x = 30;

// Connector height
support_connector_height = 10;

// Connector width
support_connector_width = 15;

// Connector depth
support_connector_depth = 3;

/* [Other] --------------------------------------------- */

// Extra space to allow for fit tolerance (reduce for tighter fit)
tolerance = 0.15;

// Left case only
left_case = false;

// Right case only
right_case = false;

// Top button support only
top_support_button = false;

// Angle support only
angle_support = false;

// Show top view
top_view = false;

// Show right view
right_view = true;

// Show model with assembled pieces (NOT FOR PRINTING!)
final_view = true;

/* [Hidden] */

fudge = .001;

total_width = panel_width + tolerance*2 + wall_width*2;
total_height = panel_height + tolerance*2 + wall_width*2;
total_depth = panel_depth + circuit_depth + front_bump + wall_width;

panel_hole_width = panel_width + tolerance*2;
panel_hole_height = panel_height + tolerance*2;
panel_hole_depth = panel_depth + tolerance*4 + front_bump;

final_circuit_depth = total_depth - panel_hole_depth + tolerance*2 - wall_width;

final_usb_offset_x = panel_width/2 - usb_hole_offset_x;
final_usb_offset_y = -panel_height/2 + usb_hole_offset_y;
final_usb_depth = wall_width + tolerance*2;

cable_holder_width = usb_hole_width;
cable_holder_offset_y = final_usb_offset_y + usb_hole_height/2 + cable_holder_height/2;
cable_holder_depth = final_circuit_depth;

button_hole_width = max(button_hold_size + button_side_space, button_diameter + tolerance*2);
button_hole_height = inside_border_width + fudge;
button_hole_depth = button_diameter + tolerance*2;
button_ext_diameter = button_inner_diameter + tolerance*2;

final_button_hole_offset_x = panel_width/2 - button_offset_x;
final_button_hole_offset_y = panel_height/2 + tolerance - inside_border_width + button_hole_height/2 - fudge;
final_button_hole_offset_z = total_depth - panel_hole_depth + tolerance*2 - button_hole_depth + fudge;

button_holder_width = button_hold_size;
button_holder_height = button_hole_height - button_push_height + button_extra_height;
button_holder_depth = final_circuit_depth;
button_holder_offset_y = -button_holder_height/2 + panel_hole_height/2 - button_push_height;

button_support_height = button_total_height - button_push_height;
button_support_depth = button_holder_depth - button_hole_depth + (button_diameter - button_hold_size);

button_support_cut_depth = button_holder_depth - button_support_depth;

echo("button_holder_depth", button_holder_depth);
echo("button_support_depth", button_support_depth);
echo("button_support_cut_depth ", button_support_cut_depth);

$fa = 10;
$fs = .4;
$vpr = [
  top_view ? 22.5 : right_view ? 90 : 67.5,
  0,
  top_view ? 0 : right_view ? 90 : 22.5
];
$vpd = 700;
$vpt = [0, 0, 0];

// ----------------------------------------------------------

module lock_tab(width = true, mirror = false) {
  points = [
    [0, 0],
    [0, front_bump],
    [lock_tab_thickness, 0],
  ];
  path = [
    [0, 1, 2]
  ];
  if (width) {
    tab_size = lock_tab_size;
    // tab_size = panel_hole_width;
    color("red")
    scale([1, mirror ? -1 : 1, 1])
    translate([0, 0, front_bump/2])
    rotate([90, 0, -90])
    translate([-lock_tab_thickness/2, -front_bump/2, -tab_size])
    linear_extrude(tab_size*2) { 
      polygon(points, path);
    }
  } else {
    tab_size = lock_tab_size;
    // tab_size = panel_hole_height;
    color("orange")
    scale([mirror ? -1 : 1, 1, 1])
    translate([0, 0, front_bump/2])
    rotate([90, 0, 0])
    translate([-lock_tab_thickness/2, -front_bump/2, -tab_size/2])
    linear_extrude(tab_size) { 
      polygon(points, path);
    }    
  }
}

module lock_tabs() {
  // Top tab
  translate([0, -lock_tab_thickness/2 + panel_hole_height/2 + fudge, total_depth - front_bump])
  lock_tab(true, false);
  // Bottom tab
  translate([0, lock_tab_thickness/2 + -panel_hole_height/2 - fudge, total_depth - front_bump])
  lock_tab(true, true);
  // Left tab
  translate([lock_tab_thickness/2 - panel_hole_width/2 - fudge, 0, total_depth - front_bump])
  lock_tab(false, false);
  // Right tab
  translate([-lock_tab_thickness/2 + panel_hole_width/2 + fudge, 0, total_depth - front_bump])
  lock_tab(false, false);
}

module button_hole() {
  diff_diameter = button_hole_depth - button_ext_diameter;
  union() {
    // Button space
    translate([0, -0.5, button_hole_depth/2 + 0.5])
    cube([button_hole_width, button_hole_height + 1, button_hole_depth + 1], center = true);

    // Push hole
    translate([0, button_hole_height/2 - 0.5, button_ext_diameter/2 + diff_diameter/2])
    rotate([-90, 0, 0])
    linear_extrude(wall_width + 1) 
    circle(d = button_ext_diameter);
  }
}

module button_support(bottom = true) {
  top_part_depth_offset = button_holder_depth/2 + 1 - button_connector_depth;

  color("yellow")
  translate(bottom ? [final_button_hole_offset_x, button_holder_offset_y, wall_width - fudge] : [0, 0, 0])
  translate(bottom ? [0, 0, 0] : [0, 0, (button_holder_depth - top_part_depth_offset)/2])
  rotate(bottom ? [0, 0, 0] : [180, 0, 0])
  translate(bottom ? [0, 0, 0] : [0, 0, -(button_holder_depth + top_part_depth_offset)/2])
  union() {
    difference() {
      translate([0, 0, button_holder_depth/2])
      cube([button_holder_width, button_holder_height, button_holder_depth], center=true);

      // Space for button swicth
      translate([0, button_holder_height/2 - button_support_height/2 + 0.5, button_support_cut_depth/2 + button_support_depth + fudge + 1])
      cube([button_holder_width + 1, button_support_height + 1, button_support_cut_depth + 1], center=true);

      if (bottom && !final_view) {
        // Connector hole
        translate([0, -button_support_height/2, button_support_cut_depth/2])
        cube([button_connector_width + tolerance*2, button_connector_height + tolerance*2, button_connector_depth*2 + tolerance*2], center=true);

        // Cut top half
        translate([0, 0, button_holder_depth/2 + button_holder_depth/4 + 0.5])
        cube([button_holder_width + 1, button_holder_height + 1, button_holder_depth/2 + 1], center=true);
      } else if (!bottom) {
        // Cut bottom half
        translate([0, 0, button_holder_depth/4 - 0.5])
        cube([button_holder_width + 1, button_holder_height + 1, button_holder_depth/2 + 1], center=true);
      }
    }
    if (!bottom) {
      // Connector
      translate([0, -button_support_height/2, button_holder_depth/2])
      cube([button_connector_width, button_connector_height, button_connector_depth*2], center=true);
    }
  }
}

module cable_holder() {
  color("yellow")
  translate([final_usb_offset_x, cable_holder_offset_y, wall_width + cable_holder_depth/2 - fudge])
  difference() {
    cube([cable_holder_width, cable_holder_height, cable_holder_depth], center=true);

    // Cable hole
    cube([cable_diameter + tolerance*2, cable_holder_height + 1, cable_holder_depth + 1], center = true);
  }
}

angle = 50;
angle_support_width = 20;
angle_support_min_height = 3;
angle_support_extra_depth = 30;
angle_support_offset_x = 30;
support_connector_height = 10;
support_connector_width = 15;
support_connector_depth = 3;

module support_hole() {

}

module support() {

}

module case() {
  union() {
    difference() {
      translate([0, 0, total_depth/2]) {
        cube([total_width, total_height, total_depth], center=true);
      }

      translate([final_button_hole_offset_x, final_button_hole_offset_y, final_button_hole_offset_z])
      button_hole();

      // Usb hole
      translate([final_usb_offset_x, final_usb_offset_y, final_usb_depth/2 - tolerance])
      cube([usb_hole_width, usb_hole_height, final_usb_depth], center = true);

      // Space for the panel
      translate([0, 0, panel_hole_depth/2 + circuit_depth + wall_width - tolerance*2]) {
        cube([panel_hole_width, panel_hole_height, panel_hole_depth], center=true);
      }

      // Space for the circuits
      color("green")
      translate([0, 0, circuit_depth/2 + wall_width])
      cube([
        panel_hole_width - inside_border_width*2, panel_hole_height - inside_border_width*2,
        circuit_depth
      ], center=true);
    }
    lock_tabs();
    button_support();
    cable_holder();
  }
}

// TODO: support angle + hole
// TODO: split case in half + connectors

rotate(final_view ? [angle, 0, 0] : [0, 0, 0])
case();