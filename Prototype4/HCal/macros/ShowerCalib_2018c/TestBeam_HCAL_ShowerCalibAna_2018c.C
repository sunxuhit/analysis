#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
// #include <proto4showercalib/Proto4ShowerCalib.h>
R__LOAD_LIBRARY(/direct/phenix+u/xusun/WorkSpace/sPHENIX/install/lib/libProto4HCalShowerCalib_2018c.so)
#endif

int TestBeam_HCAL_ShowerCalibAna_2018c(const int nEvents = 10000000, const int energy = 8) // -8 GeV meson
{
  gSystem->Load("/direct/phenix+u/xusun/WorkSpace/sPHENIX/install/lib/libProto4HCalShowerCalib_2018c.so");

  bool _is_sim = false;

  std::string outputfile;
  if(!_is_sim) outputfile = Form("/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/Proto4ShowerCalib_%dGeV_2018c.root",energy);
  if(_is_sim) outputfile = Form("/sphenix/user/xusun/Simulation/ShowerCalibAna_2018c/Proto4ShowerSIM_%d.root",energy);

  Proto4ShowerCalib* hcal_ana = new Proto4ShowerCalib(outputfile.c_str());
  hcal_ana->is_sim(_is_sim);
  hcal_ana->set_energy(energy);
  hcal_ana->set_numofevents(nEvents);
  hcal_ana->InitAna();
  hcal_ana->MakeAna();
  hcal_ana->FinishAna();

  std::cout << "All done" << std::endl;

  return 0;
}
