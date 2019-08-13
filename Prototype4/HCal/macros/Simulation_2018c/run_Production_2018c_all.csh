#!/bin/tcsh

date

echo  run production for 3 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,3\)
echo finish 3 GeV production
sleep 2

echo  run production for 4 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,4\)
echo finish 4 GeV production
sleep 2

echo  run production for 5 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,5\)
echo finish 5 GeV production
sleep 2

echo  run production for 6 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,6\)
echo finish 6 GeV production
sleep 2

echo  run production for 8 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,8\)
echo finish 8 GeV production
sleep 2

echo  run production for 12 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,12\)
echo finish 12 GeV production
sleep 2

echo  run production for 16 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,16\)
echo finish 16 GeV production
sleep 2

echo  run production for 20 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,20\)
echo finish 20 GeV production
sleep 2

echo  run production for 24 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,24\)
echo finish 24 GeV production
sleep 2

echo  run production for 28 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,28\)
echo finish 28 GeV production
sleep 2

echo  run production for 40 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,40\)
echo finish 40 GeV production
sleep 2

echo  run production for 50 GeV
root -l -b -q Fun4All_Simulation_2018c.C\(50000,50\)
echo finish 50 GeV production
sleep 2

# echo  run production for 120 GeV
# root -l -b -q Fun4All_Simulation_2018c.C\(50000,120\)
# echo finish 120 GeV production