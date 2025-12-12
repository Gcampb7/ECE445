module motor_mount () {
    difference() {
        // The $fn = 100 makes it circular, getting rid of it will make it hexagonal
        translate([0, 0, -1]) {
            cylinder(8, 1.85, 1.85); 
        }
        cylinder(20, 1.65, 1.65, center = true);
        translate([-0.75, 0, -1]) {
            cube([1.5, 10, 20]);
        }   
    }
}


// This is the box
module hollow_enclosure () {
    translate([0,0,4]) {
        difference() {
            // This is the outer layer of the box, length and width of 15 cm
            cube([15, 15, 11.2], center = true);
            translate ([0,0,1]) {
                // This is the inner box that we subtract
                cube([14.5, 14.5, 12.2], center = true);
                }
            
        
           translate ([0,7,-2.5]) {
                        // This is the small cube we get rid of to make the hole for power
                        cube([2, 8, 2], center = true);
            }
       }
    }
}

difference() {
    difference(){
        hollow_enclosure();
        translate([0,0,0.5]) {
            cylinder(5, 0.8, 1.65, center = true, $fn = 100);
            }
        }
    translate([0,0,6]) {
        cylinder(5, 1.4, 1.65, center = true, $fn = 100 );
    }
    }
motor_mount();