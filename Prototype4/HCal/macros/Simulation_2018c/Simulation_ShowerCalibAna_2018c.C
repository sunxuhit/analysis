#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
// #include <proto4showercalib/Proto4ShowerCalib.h>
R__LOAD_LIBRARY(/direct/phenix+u/xusun/WorkSpace/sPHENIX/install/lib/libProto4Simulation_2018c.so)
#endif

int Simulation_ShowerCalibAna_2018c(const int nEvents = 10000000, const int beam_mom = 8) // -8 GeV meson
{
  gSystem->Load("/direct/phenix+u/xusun/WorkSpace/sPHENIX/install/lib/libProto4Simulation_2018c.so");

  std::string outputfile = "/sphenix/user/xusun/Simulation/ShowerCalibAna/Proto4Simulation_2018c_pion_" + std::to_string(beam_mom) + "GeV.root";

  Proto4Simulation * hcal_ana = new Proto4Simulation(outputfile.c_str());
  hcal_ana->set_momentum(beam_mom);
  hcal_ana->set_numofevents(nEvents);
  hcal_ana->InitAna();
  hcal_ana->MakeAna();
  hcal_ana->FinishAna();

  std::cout << "All done" << std::endl;

  return 0;
}
