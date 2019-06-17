#!/bin/tcsh

date

echo  run production for 40 GeV
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,40,2671\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,40,2672\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,40,2673\)
echo finish 40 GeV production
