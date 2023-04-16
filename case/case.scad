// Compile this file into an STL using OpenSCAD:
// https://openscad.org/

// Requires BOSL2 to be installed, see:
// https://github.com/revarbat/BOSL2
include <BOSL2/std.scad>

/* [Dimensions] ---------------------------------------- */

// LCD panel width
panel_width = 256;

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
circuit_depth = 17;

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

// Extra height for button support
button_extra_height = 10;

// Connector width
button_connector_width = 3;

// Connector height
button_connector_height = 6;

// Connector depth
button_connector_depth = 5;

/* [Panel fixation] ------------------------------------ */

// Use screws to fix the panel instead of tabs
use_screws = false;

// Lock tabs thickness
lock_tab_thickness = 0.8;

// Lock tabs base width
lock_tab_size = 20;

// Screw diameter
screw_diameter = 2.5;

// Screw head diameter
screw_head_diameter = 6.5;

// Screw head depth
screw_head_depth = 5;

// Screw hole position offset from the panel edge
screw_offset = 7;

/* [Angle support] ------------------------------------- */

// Target panel angle
angle = 65;

// Support width
angle_support_width = 20;

// Support extra height
angle_support_extra_height = 50;

// Support min depth
angle_support_min_depth = 3;

// Offset from each side
angle_support_offset_x = 30;

// Connector height
support_connector_height = 5;

// Connector width
support_connector_width = 12;

// Connector depth
support_connector_depth = 6;

/* [Other] --------------------------------------------- */

// Extra space to allow for fit tolerance (reduce for tighter fit)
tolerance = 0.35;

// Fit tolerance for the split parts (reduce for tighter fit)
split_tolerance = 0.2;

// Split case on Y axis instead of Z axis
case_split_y = true;

// Disable case split (if your printer can handle it)
case_no_split = false;

// Left case only
left_case = false;

// Right case only
right_case = false;

// Angle and top button supports only
support_and_button = false;

// Show model with assembled pieces (NOT FOR PRINTING!)
final_view = false;

// Test prints, for checking fit and tolerance value
test_prints = false;

/* [Hidden] */

fudge = .001;
sep = 10;

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

button_hole_width = max(button_hold_size + button_side_space*2, button_diameter + tolerance*2);
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

angle_height= total_depth * sin(angle);
angle_depth = sqrt(pow(total_depth, 2) - pow(angle_height, 2));
angle_support_height = angle_height + angle_support_extra_height;
angle_support_depth = angle_depth + angle_support_min_depth;
angle_support_offset_y = angle_support_extra_height/2;

support_left_offset_x = angle_support_width/2 - total_width/2 + angle_support_offset_x;
support_right_offset_x = -angle_support_width/2 + total_width/2 - angle_support_offset_x;
connector_y_offset = angle_support_height/2 - angle_height;

$fa = 10;
$fs = .4;
$vpr = [
  final_view ? 85 : 22.5,
  0,
  final_view ? 65 : 0
];
$vpd = final_view ? 700 : 800;
$vpt = final_view ? [0, 0, total_height/2] : [(angle_support_width + sep*5 + button_holder_width)/2, 0, 0];

// ----------------------------------------------------------

module triangle(size, rot = 0) {
  w = size[0];
  h = size[1];
  d = size[2];
  points = [
    [0, 0],
    [0, d],
    [h, 0],
  ];
  path = [
    [0, 1, 2]
  ];
  rotate([90, 0, 270 - rot])
  translate([-h/2, -d/2, -w/2])
  linear_extrude(w) { 
    polygon(points, path);
  }
}

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
  lock_tab(false, true);
}

module screws_hole() {
  inner_diameter = screw_diameter + tolerance*2;
  outer_diameter = screw_head_diameter + tolerance*2;
  width_offset = total_width/2 - wall_width/2 - screw_offset;
  height_offset = total_height/2 - wall_width/2 - screw_offset;

  // Top left
  translate([-width_offset, height_offset, 0]) {
    union() {
      cylinder(total_depth*2 + 1, d=inner_diameter, center=true);

      translate([0, 0, screw_head_depth/2 - fudge])
      cylinder(screw_head_depth, d=outer_diameter, center=true);
    }
  }

  // Top right
  translate([width_offset, height_offset, 0]) {
    union() {
      cylinder(total_depth*2 + 1, d=inner_diameter, center=true);

      translate([0, 0, screw_head_depth/2 - fudge])
      cylinder(screw_head_depth, d=outer_diameter, center=true);
    }
  }

  // Bottom left
  translate([-width_offset, -height_offset, 0]) {
    union() {
      cylinder(total_depth*2 + 1, d=inner_diameter, center=true);

      translate([0, 0, screw_head_depth/2 - fudge])
      cylinder(screw_head_depth, d=outer_diameter, center=true);
    }
  }

  // Bottom right
  translate([width_offset, -height_offset, 0]) {
    union() {
      cylinder(total_depth*2 + 1, d=inner_diameter, center=true);

      translate([0, 0, screw_head_depth/2 - fudge])
      cylinder(screw_head_depth, d=outer_diameter, center=true);
    }
  }
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
  translate(bottom ? [0, 0, 0] : [0, 0, (button_holder_depth - top_part_depth_offset)/2])
  rotate(bottom ? [0, 0, 0] : [180, 0, 0])
  translate(bottom ? [0, 0, 0] : [0, 0, -(button_holder_depth + top_part_depth_offset)/2])
  union() {
    difference() {
      translate([0, 0, button_holder_depth/2])
      cube([button_holder_width, button_holder_height, button_holder_depth], center=true);

      // Space for button swicth
      translate([0, (bottom ? 0: tolerance) + button_holder_height/2 - button_support_height/2 + 1, button_support_cut_depth/2 + button_support_depth + fudge + 1])
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

      // Front tab
      translate([0, button_holder_height/2 - wall_width/4 + tolerance*2 + fudge, button_holder_depth/2 + button_support_depth/2 + tolerance/2])
      cube([button_holder_width, wall_width/2 + tolerance*2, button_holder_depth - button_support_depth - tolerance], center=true);
    }
  }
}

module support_hole(left = true) {
  translate([0, -total_height/2, total_depth/2])
  rotate([-angle, 0, 0])
  translate([0, angle_support_offset_y, -angle_depth/2 - angle_support_min_depth])
  translate(left ?
    [support_left_offset_x, 0, 0] :
    [support_right_offset_x, 0, 0]
  )
  translate([0, -connector_y_offset, angle_support_min_depth])
  rotate([angle - 90, 0, 0])
  translate([0, -final_circuit_depth/2 - wall_width, support_connector_depth/2])
  cube([support_connector_width + tolerance*2, support_connector_height + tolerance*3, support_connector_depth + tolerance*2], center=true);
}

module support(left = true) {
  translate(left ?
    [support_left_offset_x, 0, 0] :
    [support_right_offset_x, 0, 0]
  )
  union() {
    difference() {
      translate([0, 0, angle_support_depth/2])
      difference() {
        cube([angle_support_width, angle_support_height, angle_support_depth], center=true);

        // Angle cut
        translate([0, 0, angle_support_min_depth/2 + 0.1])
        rotate([0, 180, 0])
        triangle([angle_support_width + 1, angle_support_height + 1, angle_depth]);
      }

      // Case cut
      translate([0, -angle_support_offset_y, angle_depth/2 + angle_support_min_depth + fudge])
      rotate([angle, 0, 0])
      translate([0, total_height/2, -total_depth/2])
      translate([0, 0, total_depth/2])
      cube([total_width, total_height, total_depth - tolerance/2], center=true);
    }

    // Connector
    translate([0, -connector_y_offset, angle_support_min_depth])
    rotate([angle - 90, 0, 0])
    translate([0, -final_circuit_depth/2 - wall_width, support_connector_depth/2])
    cube([support_connector_width, support_connector_height, support_connector_depth], center=true);
  }
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

      // Left support hole
      support_hole(true);

      // Right support hole
      support_hole(false);

      if (use_screws) {
        screws_hole();
      }
    }
    if (!use_screws) {
      lock_tabs();
    }

    translate([final_button_hole_offset_x, button_holder_offset_y, wall_width - fudge])
    button_support();
  }
}

module split(split_height = false) {
  req_children($children);
  if (case_no_split) {
    children();
  } else {
    partition(
      size=split_height ? [total_height, total_depth*2, total_width] : [total_width, total_height, total_depth*2],
      spread=final_view ? 0 : sep + (split_height ? 6 : 8),
      cutpath="dovetail",
      spin=split_height ? [90, 90, 0] : 90,
      cutsize=split_height ? 8 : 6,
      gap= split_height ? 1.5 : 8,
      $slop=split_tolerance
    )
    children();
  }
}

module only_left_or_right() {
  req_children($children);
  if (final_view) {
    children();
  } else if (left_case) {
    translate([total_width/4 + sep, 0, 0])
    left_half((total_width + sep*2)*2)
    children();
  } else if (right_case) {
    translate([-total_width/4 - sep, 0, 0])
    right_half((total_width + sep*2)*2)
    children();
  } else {
    children();
  }
}

module all() {
  if (final_view || !support_and_button) {
    translate(final_view ? [0, -angle_support_offset_y, angle_depth/2 + angle_support_min_depth] : [0, 0, 0])
    rotate(final_view ? [angle, 0, 0] : [0, 0, 0])
    translate(final_view ? [0, total_height/2, -total_depth/2] : [0, 0, 0])
    only_left_or_right()
    split(case_split_y)
    case();
  }

  if (final_view || (!left_case && !right_case)) {
    translate(final_view || support_and_button ? [0, 0, 0] : [total_width/2 + angle_support_width + sep*2.5, 0, 0])
    {
      // Left support
      translate(final_view ? [0, 0, 0] : [-angle_support_width/2 - sep/2 - support_left_offset_x, 0, 0])
      support();

      // Right support
      translate(final_view ? [0, 0, 0] : [angle_support_width/2 + sep/2 - support_right_offset_x, 0, 0])
      support(false);
    }

    if (!final_view) {
      translate(support_and_button ? 
        [angle_support_width/2 + sep*2.5 + button_holder_width/2, 0, 0] :
        [total_width/2 + angle_support_width + sep*2.5 + angle_support_width/2 + sep*2.5 + button_holder_width/2, 0, 0])
      button_support(false);
    }
  }
}

module test_prints() {
  translate([20, 0, 0])
  button_support(true);

  button_support(false);

  translate([-30, -8, case_split_y ? 10 : 5])
  split(case_split_y)
  translate([0, 10, 0])
  difference() {
    cube([15, case_split_y ? 15 : 30, case_split_y ? 20 : 10], center=true);
    
    translate([0, 8, 3])
    cube([11, 15, case_split_y ? 20 : 10], center=true);
  }
}

if (test_prints) {
  test_prints();
} else {
  all();
}
