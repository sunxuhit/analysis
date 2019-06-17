#!/bin/tcsh

date

echo  run production for 12 GeV
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2621\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2622\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2623\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2624\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2625\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2626\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2627\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2628\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2629\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,12,2630\)
echo finish 12 GeV production
