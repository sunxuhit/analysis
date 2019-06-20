#ifndef __Proto4Simulation_2018c_H__
#define __Proto4Simulation_2018c_H__

#include <TFile.h>
#include <TNtuple.h>
#include <fun4all/SubsysReco.h>
#include <stdint.h>
#include <fstream>
#include <string>

class PHCompositeNode;
class PHG4HitContainer;
class PHG4TruthInfoContainer;
class Fun4AllHistoManager;
class TH1F;
class TH2F;
class TH3F;
class TTree;
class PHG4Particle;
class RawTowerGeom;
class RawTowerContainer;

/// \class Proto4Simulation to help you get started
class Proto4Simulation : public SubsysReco
{
 public:
  //! constructor
  Proto4Simulation(const std::string &filename);

  //! destructor
  virtual ~Proto4Simulation();

  //! Standard function called at initialization
  int Init(PHCompositeNode *topNode);

  //! Standard function called when a new run is processed
  int InitRun(PHCompositeNode *topNode);

  //! Standard function called at each event
  int process_event(PHCompositeNode *topNode);

  //! Standard function called at the end of processing. Save your stuff here.
  int End(PHCompositeNode *topNode);

 private:

  //! get manager of histograms
  Fun4AllHistoManager *get_HistoManager();

  //! output root file name
  std::string _filename;

  PHG4HitContainer *mG4HIT_CEMC; // G4Hit for EMCal 
  PHG4HitContainer *mG4ABS_CEMC; // G4Hit for absorber
  PHG4HitContainer *mG4HIT_HCALIN;
  PHG4HitContainer *mG4ABS_HCALIN;
  PHG4HitContainer *mG4HIT_HCALOUT;
  PHG4HitContainer *mG4ABS_HCALOUT;
  PHG4HitContainer *mG4HIT_CRYO;
  PHG4HitContainer *mG4HIT_BLACKHOLE;

  RawTowerContainer *mTOWER_CALIB_LG_CEMC; // tower for EMCal
  RawTowerContainer *mTOWER_CALIB_LG_HCALIN;
  RawTowerContainer *mTOWER_CALIB_LG_HCALOUT;

  PHG4TruthInfoContainer* _truth_container;

  int initHisto(); // register histograms

  // utilities
  int getNodes(PHCompositeNode *topNode); // extract nodes needed for analysis
};

#endif  // __Proto4Simulation_H__
