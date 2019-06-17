#!/bin/tcsh

date

echo  run production for 50 GeV
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,50,2674\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,50,2675\)
echo finish 50 GeV production
