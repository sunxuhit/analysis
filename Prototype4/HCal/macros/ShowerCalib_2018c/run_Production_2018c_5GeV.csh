#!/bin/tcsh

date

echo  run production for 5 GeV
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,5,2601\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,5,2602\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,5,2603\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,5,2604\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,5,2605\)
echo finish 5 GeV production
