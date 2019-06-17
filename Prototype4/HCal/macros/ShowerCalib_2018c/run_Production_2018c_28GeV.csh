#!/bin/tcsh

date

echo  run production for 28 GeV
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2661\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2662\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2663\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2664\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2665\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2666\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2667\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2668\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2669\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,28,2670\)
echo finish 28 GeV production
