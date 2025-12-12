module hollow_sphere ()
{
    translate ([0, 0, 9.25]) {
        difference() {
        sphere(9.25);
        sphere(9.05);
        }
    }
}

difference() {
    hollow_sphere();
    cube([18.5, 18.5, 9.25], center = true);
}
//cube([18.5, 0.7, 9.25], center = true);