#ifndef __Proto4ShowerCalib_2018c_H__
#define __Proto4ShowerCalib_2018c_H__

#include <TFile.h>
#include <TNtuple.h>
#include <fun4all/SubsysReco.h>
#include <stdint.h>
#include <fstream>
#include <string>
#include <map>

class PHCompositeNode;
class PHG4HitContainer;
class Fun4AllHistoManager;
class TH1F;
class TH2F;
class TTree;
class TChain;
class SvtxEvalStack;
class PHG4Particle;
class RawTowerGeom;
class RawTowerContainer;
class SvtxTrack;

/// \class Proto4ShowerCalib to help you get started
class Proto4ShowerCalib : public SubsysReco
{
 public:
  //! constructor
  Proto4ShowerCalib(const std::string &filename = "Proto4ShowerCalib.root");

  //! destructor
  virtual ~Proto4ShowerCalib();

  //! Standard function called at initialization
  int Init(PHCompositeNode *topNode);

  //! Standard function called when a new run is processed
  int InitRun(PHCompositeNode *topNode);

  //! Standard function called at each event
  int process_event(PHCompositeNode *topNode);

  //! Standard function called at the end of processing. Save your stuff here.
  int End(PHCompositeNode *topNode);

  //! Is processing simulation files?
  void
  is_sim(bool b)
  {
    _is_sim = b;
  }

  // ShowerCalib Analysis
  int InitAna();

  int MakeAna();

  int FinishAna();

  void set_energy(int energy)
  {
    _mEnergy = energy;
  }

  void set_numofevents(int numofenvets)
  {
    _mNumOfEvents = numofenvets;
  }


  class Eval_Run : public TObject
  {
   public:
    Eval_Run()
    {
      reset();
    }
    virtual ~Eval_Run()
    {
    }

    void
    reset()
    {
      run = -31454;
      event = -31454;
      beam_mom = -0;
      hodo_h = -31454;
      hodo_v = -31454;
      C2_sum = -31454;
      C1 = -31454;

      valid_hodo_v = false;
      valid_hodo_h = false;
      trigger_veto_pass = false;
      good_e = false;
      good_anti_e = false;

      beam_2CH_mm = -31454;
      beam_2CV_mm = -31454;

      truth_y = -31454;
      truth_z = -31454;

      sum_E_CEMC = -31454;
      sum_E_HCAL_OUT = -31454;
      sum_E_HCAL_IN = -31454;
    }

    int run;
    int event;

    //! beam momentum with beam charge
    float beam_mom;

    //! hodoscope index
    int hodo_h;
    int hodo_v;

    //! Cherenkov sums
    float C2_sum;
    float C1;

    //! has valid hodoscope?
    bool valid_hodo_v;
    bool valid_hodo_h;

    //! has valid veto counter?
    bool trigger_veto_pass;

    //! Good electrons?
    bool good_e;

    //! Good hadron and muons?
    bool good_anti_e;

    //! 2C motion table positions
    float beam_2CH_mm;
    float beam_2CV_mm;

    //! Turth beam position. Simulation only.
    float truth_y;
    float truth_z;

    //! Sum energy of all towers
    double sum_E_CEMC;
    double sum_E_HCAL_OUT;
    double sum_E_HCAL_IN;

    ClassDef(Eval_Run, 10)
  };

  class HCAL_Shower : public TObject
  {
    public:
      HCAL_Shower()
      {
	reset();
      }

      virtual ~HCAL_Shower(){}

      void reset()
      {
	// EMCAL
	emcal_lg_e_raw   = 0.;
	emcal_lg_e_calib = 0.;
	for(int itwr=0; itwr<64; itwr++)
	{
	  emcal_lg_twr_raw[itwr] = 0.;
	  emcal_lg_twr_calib[itwr] = 0.;
	}

	// HCALIN => not used in 2018c
	hcalin_lg_e_raw   = 0.;
	hcalin_lg_e_calib = 0.;
	for(int itwr=0; itwr<16; itwr++)
	{
	  hcalin_lg_twr_raw[itwr] = 0.;
	  hcalin_lg_twr_calib[itwr] = 0.;
	}

	// HCALOUT
	hcalout_lg_e_raw = 0.;
	hcalout_lg_e_calib = 0.;
	for(int itwr=0; itwr<16; itwr++)
	{
	  hcalout_lg_twr_raw[itwr] = 0.;
	  hcalout_lg_twr_calib[itwr] = 0.;
	}
      }

      // EMCAL
      float emcal_lg_e_raw;
      float emcal_lg_twr_raw[64];

      float emcal_lg_e_calib;
      float emcal_lg_twr_calib[64];

      // HCALIN
      float hcalin_lg_e_raw;
      float hcalin_lg_twr_raw[16];

      float hcalin_lg_e_calib;
      float hcalin_lg_twr_calib[16];

      // HCALOUT
      float hcalout_lg_e_raw;
      float hcalout_lg_twr_raw[16];

      float hcalout_lg_e_calib;
      float hcalout_lg_twr_calib[16];

      ClassDef(HCAL_Shower, 10)
  };

 private:

  //! is processing simulation files?
  bool _is_sim;

  //! get manager of histograms
  Fun4AllHistoManager *
  get_HistoManager();

  //! output root file name
  std::string _filename;

  //! simple event counter
  unsigned long _ievent;

  //! run infomation. To be copied to output TTree T
  Eval_Run _eval_run;

  //! hcal infromation. To be copied to output TTree T
  HCAL_Shower _shower;

  // TowerCalib Analysis
  TFile *mFile_OutPut;
  TChain *mChainInPut;
  unsigned long _mStartEvent;
  unsigned long _mStopEvent;
  int _mEnergy;
  int _mNumOfEvents;
  int _mInPut_flag;
  std::string _mMode;

  Eval_Run *_mInfo;
  HCAL_Shower *_mShower;

  TH1F *h_mMomentum;

  TH2F *h_mAsymmEnergy; // MIP study
  TH2F *h_mAsymmEnergy_electron;
  TH2F *h_mAsymmEnergy_pion;

  // balancing
  TH2F *h_mAsymmEnergy_balancing;
  TH2F *h_mAsymmEnergy_electron_balancing;
  TH2F *h_mAsymmEnergy_pion_balancing;

  TH1F *h_mEMCalTower_e_high[64];
  TH1F *h_mHCalTower_e_high[16];
  TH1F *h_mEMCalTower_e_mid[64];
  TH1F *h_mHCalTower_e_mid[16];

  TH1F *h_mEMCalTower_pi_high[64];
  TH1F *h_mHCalTower_pi_high[16];
  TH1F *h_mEMCalTower_pi_mid[64];
  TH1F *h_mHCalTower_pi_mid[16];

  // leveling correction
  TH2F *h_mAsymmEnergy_leveling;
  TH2F *h_mAsymmEnergy_electron_leveling;
  TH2F *h_mAsymmEnergy_pion_leveling;

  // shower correction
  TH2F *h_mAsymmEnergy_showercalib;
  TH2F *h_mAsymmEnergy_electron_showercalib;
  TH2F *h_mAsymmEnergy_pion_showercalib;

  // Outer HCal only study
  TH1F *h_mEnergyOut_electron; // hadron MIP through EMCal
  TH1F *h_mEnergyOut_pion;
  TH1F *h_mEnergyOut_electron_showercalib;
  TH1F *h_mEnergyOut_pion_showercalib;

  // inner HCAL MIP energy extracted from muon
  // const double MIP_mean  = 0.654677;
  // const double MIP_width = 0.16485;
  const double MIP_mean  = 0.655;
  const double MIP_width = 0.165;

  // const double showercalib = 2.66831; // extracted with 16 GeV Test Beam Data
  const double showercalib = 2.668; // extracted with 16 GeV Test Beam Data
  const double showercalib_ohcal = 3.046; // extracted with 16 GeV Test Beam Data

  std::map<float,int> map_momIndex; // mom vs index

  int getChannelNumber(int column, int row);
};

#endif  // __Proto4ShowerCalib_H__
