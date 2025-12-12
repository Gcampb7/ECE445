difference() {
    color("green") {
        cylinder(45, 2.4, 2.4, $fn = 100);
        translate([0, 0, 45]){cylinder(15, 1.45, 1.45, $fn = 100);}
    }
    translate([0,0,-1]){
        cylinder(17, 1.9, 1.9, $fn = 100);
    }
}
