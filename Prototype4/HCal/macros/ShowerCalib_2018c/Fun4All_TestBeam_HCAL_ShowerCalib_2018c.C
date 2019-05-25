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

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libPrototype4.so)
R__LOAD_LIBRARY(/direct/phenix+u/xusun/WorkSpace/sPHENIX/install/lib/libProto4HCalShowerCalib_2018c.so)
#endif

int Fun4All_TestBeam_HCAL_ShowerCalib_2018c(const int nEvents = 100000, const string runID = "2610") // -8 GeV meson
{

  //---------------
  // Load libraries
  //---------------
  gSystem->Load("libPrototype4.so");
  gSystem->Load("libfun4all.so");                                                                        
  // gSystem->Load("libg4detectors.so");
  // gSystem->Load("libphhepmc.so");
  // gSystem->Load("libg4testbench.so");
  // gSystem->Load("libg4hough.so");
  // gSystem->Load("libg4eval.so");


  //---------------
  // Fun4All server
  //---------------

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(1);

  //-------------- 
  // IO management
  //--------------

  bool _is_sim = false;
  // bool _is_sim = true;
  // Hits file
  Fun4AllInputManager *hitsin = new Fun4AllDstInputManager("DSTin");
  std::string inputfile;
  if(!_is_sim) inputfile= Form("/sphenix/data/data02/sphenix/t1044/production/production_HCal_0502/beam_0000%s-0000.root",runID.c_str());
  if(_is_sim) inputfile = Form("/sphenix/user/xusun/Simulation/Production/BeamTest_24GeV_%s.root",runID.c_str());
  // if(!_is_sim) inputfile= Form("/sphenix/user/xusun/TestBeam/Production/beam_0000%s.root",runID.c_str());
  hitsin->fileopen(inputfile);
//  hitsin->AddListFile(inputFile); // you can also choose this and give a list of DST file names in the file.
  se->registerInputManager(hitsin);

  //load your analysis module.
  gSystem->Load("/direct/phenix+u/xusun/WorkSpace/sPHENIX/install/lib/libProto4HCalShowerCalib_2018c.so");
  std::string outputfile;
  if(!_is_sim) outputfile = Form("/sphenix/user/xusun/TestBeam/ShowerCalib_2018c/Proto4ShowerInfoRaw_%s_2018c.root",runID.c_str());
  if(_is_sim) outputfile = Form("/sphenix/user/xusun/Simulation/ShowerCalib_2018c/Proto4ShowerInfoSIM_%s_2018c.root",runID.c_str());

  // This one is an example defined in ../ExampleAnalysisModule/
  Proto4ShowerCalib* hcal_ana = new Proto4ShowerCalib(outputfile.c_str());
  hcal_ana->is_sim(_is_sim);
  se->registerSubsystem(hcal_ana);

  se->run(nEvents);
  se->End();

  std::cout << "All done" << std::endl;
  delete se;
  // gSystem->Exit(0);

  return 0;
}
