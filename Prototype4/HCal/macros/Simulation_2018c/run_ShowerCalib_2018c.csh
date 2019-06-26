#!/bin/tcsh

date

# echo  run shower calibration for 3 GeV
# root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,3\)
# echo finish 3 GeV shower calibration
# sleep 2

# echo  run shower calibration for 4 GeV
# root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,4\)
# echo finish 4 GeV shower calibration
# sleep 2

# echo  run shower calibration for 5 GeV
# root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,5\)
# echo finish 5 GeV shower calibration
# sleep 2

echo  run shower calibration for 6 GeV
root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,6\)
echo finish 6 GeV shower calibration
sleep 2

echo  run shower calibration for 8 GeV
root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,8\)
echo finish 8 GeV shower calibration
sleep 2

echo  run shower calibration for 12 GeV
root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,12\)
echo finish 12 GeV shower calibration
sleep 2

echo  run shower calibration for 16 GeV
root -l -b -q Simulation_ShowerCalibAna_2018c.C\(-1,16\)
echo finish 16 GeV shower calibration
sleep 2

echo  run shower calibration for 20 GeV
root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,20\)
echo finish 20 GeV shower calibration
sleep 2

echo  run shower calibration for 24 GeV
root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,24\)
echo finish 24 GeV shower calibration
sleep 2

echo  run shower calibration for 28 GeV
root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,28\)
echo finish 28 GeV shower calibration
sleep 2

echo  run shower calibration for 40 GeV
root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,40\)
echo finish 40 GeV shower calibration
sleep 2

# echo  run shower calibration for 50 GeV
# root -l -b -q  Simulation_ShowerCalibAna_2018c.C\(-1,50\)
# echo finish 50 GeV shower calibration
