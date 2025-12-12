difference(){
    translate([-.75, -.75, 0 ]){cube([1.5, 1.5, 10.4]);}
    translate([0,0,9.9]){
        cylinder(3, .55, .55, $fn = 100);
    }
}