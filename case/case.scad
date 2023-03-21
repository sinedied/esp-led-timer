/* [Dimensions] ---------------------------------------- */

panel_width = 260;
panel_height = 128;
panel_depth = 13;
wall_width = 3;

// Target panel angle
angle = 50;

// Space in front of the panel for protection
front_bump = 2;

// Space behind panel for the circuit
circuit_depth = 15;

// Panel border
inside_border_width = 11.5;

usb_hole_width = 14;
usb_hole_height = 8;

// Usb hole centered offset from right side of the panel
usb_hole_offset_x = 95;

// Usb hole centered offset from bottom of the panel
usb_hole_offset_y = 30;

button_diameter = 11.5;
button_hold_size = 10;
button_height = 8.5;
button_offset_x = 91;

// Bevel of lock tabs
lock_tab_width = 0.4;
lock_tab_size = 20;

// screw_diameter = 2.5;
// screw_offset = 7;

// TODO: larger width tabs
// TODO: button hole
// TODO: support angle

/* [Other] --------------------------------------------- */

top_view = true;

/* [Hidden] */

tolerance = 0.15;
fudge = .001;

total_width = panel_width + tolerance*2 + wall_width*2;
total_height = panel_height + tolerance*2 + wall_width*2;
total_depth = panel_depth + circuit_depth + front_bump + wall_width;

panel_hole_width = panel_width + tolerance*2;
panel_hole_height = panel_height + tolerance*2;
panel_hole_depth = panel_depth + tolerance*4 + front_bump;

final_usb_offset_x = panel_width/2 - usb_hole_offset_x;
final_usb_offset_y = -panel_height/2 + usb_hole_offset_y;
final_usb_depth = wall_width + tolerance*2;

$fa = 10;
$fs = .4;
$vpr = [
  top_view ? 22.5 : 67.5,
  0,
  top_view ? 0 : 22.5
];
$vpd = 700;
$vpt = [0, 0, 0];

// ----------------------------------------------------------

module lock_tab(width = true, mirror = false) {
  points = [
    [0, 0],
    [0, front_bump],
    [lock_tab_width, 0],
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
    translate([-lock_tab_width/2, -front_bump/2, -tab_size/2])
    linear_extrude(tab_size) { 
      polygon(points, path);
    }
  } else {
    tab_size = lock_tab_size;
    // tab_size = panel_hole_height;
    color("orange")
    scale([mirror ? -1 : 1, 1, 1])
    translate([0, 0, front_bump/2])
    rotate([90, 0, 0])
    translate([-lock_tab_width/2, -front_bump/2, -tab_size/2])
    linear_extrude(tab_size) { 
      polygon(points, path);
    }    
  }
}

module lock_tabs() {
  // Top tab
  translate([0, -lock_tab_width/2 + panel_hole_height/2, total_depth - front_bump])
  lock_tab(true, false);
  // Bottom tab
  translate([0, lock_tab_width/2 + -panel_hole_height/2, total_depth - front_bump])
  lock_tab(true, true);
  // Left tab
  translate([lock_tab_width/2 - panel_hole_width/2, 0, total_depth - front_bump])
  lock_tab(false, false);
  // Right tab
  translate([-lock_tab_width/2 + panel_hole_width/2, 0, total_depth - front_bump])
  lock_tab(false, false);
}

module case() {
  union() {
    difference() {
      translate([0, 0, total_depth/2]) {
        cube([total_width, total_height, total_depth], center=true);
      }

      // USB hole
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
  }
}

case();