/* [Dimensions] ---------------------------------------- */

panel_width = 260;
panel_height = 128;
panel_depth = 13;
wall_width = 3;
front_bump = 2;
circuit_depth = 15;
inside_border_width = 11.5;
// screw_diameter = 2.5;
// screw_offset = 7;
usb_hole_width = 14;
usb_hole_height = 8;
usb_hole_offset_x = 95;
usb_hole_offset_y = 30;
angle = 50;
button_diameter = 11.5;
button_hold_size = 10;
button_height = 8.5;
button_offset_x = 91;
lock_tab_width = 0.4;
lock_tab_size = 20;

/* [Other] --------------------------------------------- */

flat_view = false;


/* [Hidden] */

tolerance = 0.15;
fudge = .001;

total_width = panel_width + tolerance*2 + wall_width*2;
total_height = panel_height + tolerance*2 + wall_width*2;
total_depth = panel_depth + circuit_depth + front_bump + wall_width;

panel_hole_width = panel_width + tolerance*2;
panel_hole_height = panel_height + tolerance*2;
panel_hole_depth = panel_depth + tolerance*2 + front_bump;

$fa = 10;
$fs = .4;
$vpr = [
  flat_view ? 0 : 67.5,
  0,
  flat_view ? 0 : 22.5
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

module case() {
  union() {
    difference() {
      translate([0, 0, total_depth/2]) {
        cube([total_width, total_height, total_depth], center=true);
      }

      // Space for the panel
      translate([0, 0, panel_hole_depth/2 + circuit_depth + wall_width + fudge]) {
        cube([panel_hole_width, panel_hole_height, panel_hole_depth], center=true);
      }
    }
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
}

case();