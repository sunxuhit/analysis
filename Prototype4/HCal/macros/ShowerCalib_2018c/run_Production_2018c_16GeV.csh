#!/bin/tcsh

date

echo  run production for 16 GeV
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2631\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2632\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2633\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2634\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2635\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2636\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2637\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2638\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2639\)
sleep 2
root -l -b -q Fun4All_TestBeam_HCAL_ShowerCalib_2018c.C\(5000000,16,2640\)
echo finish 16 GeV production
