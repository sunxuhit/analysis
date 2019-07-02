#ifndef __Proto4Simulation_2018c_H__
#define __Proto4Simulation_2018c_H__

#include <TFile.h>
#include <TNtuple.h>
#include <fun4all/SubsysReco.h>
#include <stdint.h>
#include <fstream>
#include <string>
#include <map>

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

  // ShowerCalib Analysis
  int InitAna();

  int MakeAna();

  int FinishAna();

  void set_momentum(int momentum)
  {
    _mMom = momentum;
  }

  void set_pid(std::string pid)
  {
    _mPID = pid;
  }

  void set_numofevents(int numofenvets)
  {
    _mNumOfEvents = numofenvets;
  }

  class HCAL_Sim : public TObject
  {
    public:
      HCAL_Sim()
      {
	reset();
      }

      virtual ~HCAL_Sim(){}

      void reset()
      {
	// Tower information
	emcal_lg_e_calib = 0.; // EMCal
	for(int itwr=0; itwr<64; itwr++)
	{
	  emcal_lg_twr_calib[itwr] = 0.;
	}

	hcalin_lg_e_calib = 0.; // HCALIN => not used in 2018c
	for(int itwr=0; itwr<16; itwr++)
	{
	  hcalin_lg_twr_calib[itwr] = 0.;
	}

	hcalout_lg_e_calib = 0.; // HCALOUT
	for(int itwr=0; itwr<16; itwr++)
	{
	  hcalout_lg_twr_calib[itwr] = 0.;
	}

	// G4Hit information
	emcal_e_hit = 0.0;
	emcal_e_abs = 0.0;
	hcalin_e_hit = 0.0;
	hcalin_e_abs = 0.0;
	hcalout_e_hit = 0.0;
	hcalout_e_abs = 0.0;
	cryo_e_hit = 0.0;
	blackhole_e_hit = 0.0;
	truth_e = 0.0;
      }

      // Tower information
      float emcal_lg_e_calib; // EMCAL
      float emcal_lg_twr_calib[64];

      float hcalin_lg_e_calib; // HCALIN
      float hcalin_lg_twr_calib[16];

      float hcalout_lg_e_calib; // HCALOUT
      float hcalout_lg_twr_calib[16];

      // G4Hit information
      float emcal_e_hit;
      float emcal_e_abs;
      float hcalin_e_hit;
      float hcalin_e_abs;
      float hcalout_e_hit;
      float hcalout_e_abs;
      float cryo_e_hit;
      float blackhole_e_hit;
      float truth_e;

      ClassDef(HCAL_Sim, 10)
  };

 private:

  //! get manager of histograms
  Fun4AllHistoManager *get_HistoManager();

  //! hcal infromation. To be copied to output TTree T
  HCAL_Sim _sim;

  //! output root file name
  std::string _filename;

  RawTowerContainer *mTOWER_CALIB_LG_CEMC; // tower for EMCal
  RawTowerContainer *mTOWER_CALIB_LG_HCALIN;
  RawTowerContainer *mTOWER_CALIB_LG_HCALOUT;

  PHG4HitContainer *mG4HIT_CEMC; // G4Hit for EMCal 
  PHG4HitContainer *mG4ABS_CEMC; // G4Hit for absorber
  PHG4HitContainer *mG4HIT_HCALIN;
  PHG4HitContainer *mG4ABS_HCALIN;
  PHG4HitContainer *mG4HIT_HCALOUT;
  PHG4HitContainer *mG4ABS_HCALOUT;
  PHG4HitContainer *mG4HIT_CRYO;
  PHG4HitContainer *mG4HIT_BLACKHOLE;

  PHG4TruthInfoContainer* _truth_container;

  int initHisto(); // register histograms

  // utilities
  int getNodes(PHCompositeNode *topNode); // extract nodes needed for analysis

  int getChannelNumber(int column, int row);

  HCAL_Sim *_mSimulation;

  // Shower Analysis
  TFile *mFile_InPut;
  TFile *mFile_OutPut;
  TTree *mTTreeInPut;
  unsigned long _mStartEvent;
  unsigned long _mStopEvent;
  int _mMom;
  std::string _mPID;
  int _mNumOfEvents;

  std::map<float,int> map_momIndex; // mom vs index

  TH2F *h_mAsymmEnergy; // MIP study

  // balancing
  TH2F *h_mAsymmEnergy_balancing;

  // leveling correction
  TH2F *h_mAsymmEnergy_leveling;

  // shower correction
  TH2F *h_mAsymmEnergy_showercalib;

  TH1F *h_mRatio_Truth;
  TH1F *h_mRatio_Ana;
  TH1F *h_mRatio_Tower;

  const double MIP_mean  = 0.65;
  const double MIP_width = 0.16;
  // const double showercalib = 1.59349; // extracted with 16 GeV Test Beam Data
  const double showercalib = 1.63; // extracted with 16 GeV Test Beam Data
};

#endif  // __Proto4Simulation_H__
