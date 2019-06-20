#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllNoSyncDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <proto4showercalib/Proto4ShowerCalib.h>
#include <proto4samplefrac/Proto4SampleFrac.h>

R__LOAD_LIBRARY(libPrototype4.so)
R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(/direct/phenix+u/xusun/WorkSpace/sPHENIX/install/lib/libProto4Simulation_2018c.so)
#endif

int Fun4All_Simulation_2018c(const int nEvents = 100, const int beam_mom = 12)
{

  //---------------
  // Load libraries
  //---------------
  gSystem->Load("libPrototype4.so");
  gSystem->Load("libfun4all.so");                                                                        

  //---------------
  // Fun4All server
  //---------------

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(1);

  //-------------- 
  // IO management
  //--------------

  // Hits file
  Fun4AllInputManager *hitsin = new Fun4AllDstInputManager("DSTin");
  // std::string inputlist = "./list/Simulation_2018c_electron_" + std::to_string(beam_mom) + "GeV.list";
  std::string inputlist = "./list/Simulation_2018c_pion_" + std::to_string(beam_mom) + "GeV.list";
  hitsin->AddListFile(inputlist); // you can also choose this and give a list of DST file names in the file.
  se->registerInputManager(hitsin);

  //load your analysis module's lib
  gSystem->Load("/direct/phenix+u/xusun/WorkSpace/sPHENIX/install/lib/libProto4Simulation_2018c.so");

  //load your analysis module.
  // std::string outputfile = "/sphenix/user/xusun/Simulation/ShowerCalib/Proto4Simulation_2018c_electron_" + std::to_string(beam_mom) + "GeV.root";
  std::string outputfile = "/sphenix/user/xusun/Simulation/ShowerCalib/Proto4Simulation_2018c_pion_" + std::to_string(beam_mom) + "GeV.root";
  Proto4Simulation *hcal_ana = new Proto4Simulation(outputfile.c_str());
  se->registerSubsystem(hcal_ana);

  se->run(nEvents);
  se->End();

  std::cout << "All done" << std::endl;
  delete se;

  return 0;
}
