#include "Proto4ShowerCalib_2018c.h"

#include <calobase/RawTowerContainer.h>
#include <ffaobjects/EventHeader.h>
#include <pdbcalbase/PdbParameterMap.h>
#include <phparameter/PHParameters.h>
#include <prototype4/RawTower_Prototype4.h>
#include <prototype4/RawTower_Temperature.h>

#include <Event/EventTypes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/PHTFileServer.h>
#include <fun4all/SubsysReco.h>
#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

#include <phool/PHCompositeNode.h>

#include <g4main/PHG4Particle.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4VtxPoint.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLorentzVector.h>
#include <TNtuple.h>
#include <TVector3.h>
#include <TChain.h>
#include <TMath.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <exception>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

ClassImp(Proto4ShowerCalib::HCAL_Shower);
ClassImp(Proto4ShowerCalib::Eval_Run);

Proto4ShowerCalib::Proto4ShowerCalib(const std::string &filename)
  : SubsysReco("Proto4ShowerCalib")
  , _is_sim(false)
  , _filename(filename)
  , _ievent(0)
{
  Verbosity(1);

  _eval_run.reset();
  _shower.reset();

  _mInPut_flag = 1;
  _mStartEvent = -1;
  _mStopEvent = -1;
  _mEnergy = -1;
  _mNumOfEvents = -1;
}

Proto4ShowerCalib::~Proto4ShowerCalib()
{
}

Fun4AllHistoManager *
Proto4ShowerCalib::get_HistoManager()
{
  Fun4AllServer *se = Fun4AllServer::instance();
  Fun4AllHistoManager *hm = se->getHistoManager("Proto4ShowerCalib_HISTOS");

  if (not hm)
  {
    cout
        << "Proto4ShowerCalib::get_HistoManager - Making Fun4AllHistoManager Proto4ShowerCalib_HISTOS"
        << endl;
    hm = new Fun4AllHistoManager("Proto4ShowerCalib_HISTOS");
    se->registerHistoManager(hm);
  }

  assert(hm);

  return hm;
}

int Proto4ShowerCalib::InitRun(PHCompositeNode *topNode)
{
  if (Verbosity())
    cout << "Proto4ShowerCalib::InitRun" << endl;

  _ievent = 0;

  PHNodeIterator iter(topNode);
  PHCompositeNode *dstNode = static_cast<PHCompositeNode *>(iter.findFirst(
      "PHCompositeNode", "DST"));
  if (!dstNode)
  {
    std::cerr << PHWHERE << "DST Node missing, doing nothing." << std::endl;
    throw runtime_error(
        "Failed to find DST node in EmcRawTowerBuilder::CreateNodes");
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4ShowerCalib::End(PHCompositeNode *topNode)
{
  cout << "Proto4ShowerCalib::End - write to " << _filename << endl;
  PHTFileServer::get().cd(_filename);

  Fun4AllHistoManager *hm = get_HistoManager();
  assert(hm);
  for (unsigned int i = 0; i < hm->nHistos(); i++)
    hm->getHisto(i)->Write();

  //  if (_T_EMCalTrk)
  //    _T_EMCalTrk->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4ShowerCalib::Init(PHCompositeNode *topNode)
{
  _ievent = 0;

  cout << "Proto4ShowerCalib::get_HistoManager - Making PHTFileServer "
       << _filename << endl;
  PHTFileServer::get().open(_filename, "RECREATE");

  Fun4AllHistoManager *hm = get_HistoManager();
  assert(hm);

  //! Histogram of Cherenkov counters
  TH2F *hCheck_Cherenkov = new TH2F("hCheck_Cherenkov", "hCheck_Cherenkov",
                                    110, -200, 20000, 5, .5, 5.5);
  hCheck_Cherenkov->GetYaxis()->SetBinLabel(1, "C1");
  hCheck_Cherenkov->GetYaxis()->SetBinLabel(2, "C2 in");
  hCheck_Cherenkov->GetYaxis()->SetBinLabel(3, "C2 out");
  hCheck_Cherenkov->GetYaxis()->SetBinLabel(4, "C2 sum");
  hCheck_Cherenkov->GetXaxis()->SetTitle("Amplitude");
  hCheck_Cherenkov->GetYaxis()->SetTitle("Cherenkov Channel");
  hm->registerHisto(hCheck_Cherenkov);

  //! Envent nomalization
  TH1F *hNormalization = new TH1F("hNormalization", "hNormalization", 10, .5,
                                  10.5);
  hNormalization->GetXaxis()->SetBinLabel(1, "ALL");
  hNormalization->GetXaxis()->SetBinLabel(2, "C2-e");
  hNormalization->GetXaxis()->SetBinLabel(3, "trigger_veto_pass");
  hNormalization->GetXaxis()->SetBinLabel(4, "valid_hodo_h");
  hNormalization->GetXaxis()->SetBinLabel(5, "valid_hodo_v");
  hNormalization->GetXaxis()->SetBinLabel(6, "good_e");
  hNormalization->GetXaxis()->SetBinLabel(7, "good_anti_e");
  hNormalization->GetXaxis()->SetTitle("Cuts");
  hNormalization->GetYaxis()->SetTitle("Event count");
  hm->registerHisto(hNormalization);

  hm->registerHisto(new TH1F("hCheck_Veto", "hCheck_Veto", 1000, -500, 1500));
  hm->registerHisto(
      new TH1F("hCheck_Hodo_H", "hCheck_Hodo_H", 1000, -500, 1500));
  hm->registerHisto(
      new TH1F("hCheck_Hodo_V", "hCheck_Hodo_V", 1000, -500, 1500));

  hm->registerHisto(new TH1F("hBeam_Mom", "hBeam_Mom", 1200, -120, 120));

  // EMCAL QA
  TH1F *h_emcalin_lg_tower_calib[64];
  for(int i_tower = 0; i_tower < 64; ++i_tower)
  {
    string HistName = Form("h_emcal_lg_tower_%d_calib",i_tower);
    h_emcalin_lg_tower_calib[i_tower] = new TH1F(HistName.c_str(),HistName.c_str(),1000,-1.05,98.95); // GeV
    hm->registerHisto(h_emcalin_lg_tower_calib[i_tower]);
  }

  // HCALIN LG QA
  TH1F *h_hcalin_lg_tower_calib[16];
  for(int i_tower = 0; i_tower < 16; ++i_tower)
  {
    string HistName = Form("h_hcalin_lg_tower_%d_calib",i_tower);
    h_hcalin_lg_tower_calib[i_tower] = new TH1F(HistName.c_str(),HistName.c_str(),1000,-1.05,98.95);
    hm->registerHisto(h_hcalin_lg_tower_calib[i_tower]);
  }

  // HCALOUT LG QA
  TH1F *h_hcalout_lg_tower_calib[16];
  for(int i_tower = 0; i_tower < 16; ++i_tower)
  {
    string HistName= Form("h_hcalout_lg_tower_%d_calib",i_tower);
    h_hcalout_lg_tower_calib[i_tower] = new TH1F(HistName.c_str(),HistName.c_str(),1000,-1.05,98.95);
    hm->registerHisto(h_hcalout_lg_tower_calib[i_tower]);
  }

  // EMCal + HCALOUT QA
  TH2F *h_tower_energy_calib = new TH2F("h_tower_energy_calib","h_tower_energy_calib",105,-1.05,1.05,2000,-1.05,198.95);
  hm->registerHisto(h_tower_energy_calib);

  // help index files with TChain
  TTree *T = new TTree("HCAL_Info", "HCAL_Info");
  assert(T);
  hm->registerHisto(T);

  T->Branch("info", &_eval_run);
  T->Branch("shower", &_shower);

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4ShowerCalib::process_event(PHCompositeNode *topNode)
{
  if (Verbosity() > 2)
    cout << "Proto4ShowerCalib::process_event() entered" << endl;

  // init eval objects
  _eval_run.reset();
  _shower.reset();

  Fun4AllHistoManager *hm = get_HistoManager();
  assert(hm);

  if (not _is_sim)
  {
    PdbParameterMap *info = findNode::getClass<PdbParameterMap>(topNode,
                                                                "RUN_INFO");

    assert(info);

    PHParameters run_info_copy("RunInfo");
    run_info_copy.FillFrom(info);

    _eval_run.beam_mom = run_info_copy.get_double_param("beam_MTNRG_GeV");

    TH1F *hBeam_Mom = dynamic_cast<TH1F *>(hm->getHisto("hBeam_Mom"));
    assert(hBeam_Mom);

    hBeam_Mom->Fill(_eval_run.beam_mom);

    _eval_run.beam_2CH_mm = run_info_copy.get_double_param("beam_2CH_mm");
    _eval_run.beam_2CV_mm = run_info_copy.get_double_param("beam_2CV_mm");
  }

  if (not _is_sim)
  {
    EventHeader *eventheader = findNode::getClass<EventHeader>(topNode,
	"EventHeader");

    if (eventheader->get_EvtType() != DATAEVENT)
    {
      cout << __PRETTY_FUNCTION__
	<< " disgard this event: " << endl;

      eventheader->identify();

      return Fun4AllReturnCodes::DISCARDEVENT;
    }

    assert(eventheader);

    _eval_run.run = eventheader->get_RunNumber();
    if (Verbosity() > 4)
      cout << __PRETTY_FUNCTION__ << _eval_run.run << endl;

    _eval_run.event = eventheader->get_EvtSequence();
  }

  if (_is_sim)
  {
    PHG4TruthInfoContainer *truthInfoList = findNode::getClass<
        PHG4TruthInfoContainer>(topNode, "G4TruthInfo");

    assert(truthInfoList);

    _eval_run.run = -1;

    const PHG4Particle *p =
        truthInfoList->GetPrimaryParticleRange().first->second;
    assert(p);

    const PHG4VtxPoint *v = truthInfoList->GetVtx(p->get_vtx_id());
    assert(v);

    _eval_run.beam_mom = sqrt(
        p->get_px() * p->get_px() + p->get_py() * p->get_py() + p->get_pz() * p->get_pz());
    _eval_run.truth_y = v->get_y();
    _eval_run.truth_z = v->get_z();
  }

  // normalization
  TH1F *hNormalization = dynamic_cast<TH1F *>(hm->getHisto("hNormalization"));
  assert(hNormalization);

  hNormalization->Fill("ALL", 1);

  // get DST nodes

  // EMCAL+HCAL information
  RawTowerContainer *TOWER_RAW_CEMC = findNode::getClass<RawTowerContainer>(
      topNode, _is_sim ? "TOWER_RAW_LG_CEMC" : "TOWER_RAW_CEMC");
  if (not _is_sim)
  {
    assert(TOWER_RAW_CEMC);
  }

  RawTowerContainer *TOWER_RAW_LG_HCALIN = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_RAW_LG_HCALIN");
  if (not _is_sim)
  {
    assert(TOWER_RAW_LG_HCALIN);
  }

  RawTowerContainer *TOWER_RAW_LG_HCALOUT = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_RAW_LG_HCALOUT");
  if (not _is_sim)
  {
    assert(TOWER_RAW_LG_HCALOUT);
  }

  RawTowerContainer *TOWER_CALIB_CEMC = findNode::getClass<RawTowerContainer>(
      topNode, _is_sim ? "TOWER_CALIB_LG_CEMC" : "TOWER_CALIB_CEMC");
  if (not _is_sim)
  {
    assert(TOWER_CALIB_CEMC);
  }

  RawTowerContainer *TOWER_CALIB_LG_HCALIN = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_CALIB_LG_HCALIN");
  if (not _is_sim)
  {
    assert(TOWER_CALIB_LG_HCALIN);
  }

  RawTowerContainer *TOWER_CALIB_LG_HCALOUT = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_CALIB_LG_HCALOUT");
  if (not _is_sim)
  {
    assert(TOWER_CALIB_LG_HCALOUT);
  }

  /*
  RawTowerContainer *TOWER_RAW_HG_HCALOUT = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_RAW_HG_HCALOUT");
  assert(TOWER_RAW_HG_HCALOUT);

  RawTowerContainer *TOWER_CALIB_HG_HCALOUT = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_CALIB_HG_HCALOUT");
  assert(TOWER_CALIB_HG_HCALOUT);
  */

  // other nodes
  RawTowerContainer *TOWER_CALIB_TRIGGER_VETO = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_CALIB_TRIGGER_VETO");
  if (not _is_sim)
  {
    assert(TOWER_CALIB_TRIGGER_VETO);
  }

  RawTowerContainer *TOWER_CALIB_HODO_HORIZONTAL = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_CALIB_HODO_HORIZONTAL");
  if (not _is_sim)
  {
    assert(TOWER_CALIB_HODO_HORIZONTAL);
  }
  RawTowerContainer *TOWER_CALIB_HODO_VERTICAL = findNode::getClass<
      RawTowerContainer>(topNode, "TOWER_CALIB_HODO_VERTICAL");
  if (not _is_sim)
  {
    assert(TOWER_CALIB_HODO_VERTICAL);
  }

  RawTowerContainer *TOWER_CALIB_C1 = findNode::getClass<RawTowerContainer>(
      topNode, "TOWER_CALIB_C1");
  if (not _is_sim)
  {
    assert(TOWER_CALIB_C1);
  }
  RawTowerContainer *TOWER_CALIB_C2 = findNode::getClass<RawTowerContainer>(
      topNode, "TOWER_CALIB_C2");
  if (not _is_sim)
  {
    assert(TOWER_CALIB_C2);
  }

  // Cherenkov
  bool cherekov_e = false;
  bool cherekov_anti_e = false;
  if (not _is_sim)
  {
    RawTower *t_c2_in = NULL;
    RawTower *t_c2_out = NULL;

    t_c2_in = TOWER_CALIB_C2->getTower(0);
    t_c2_out = TOWER_CALIB_C2->getTower(1);

    assert(t_c2_in);
    assert(t_c2_out);

    const double c2_in = t_c2_in->get_energy();
    const double c2_out = t_c2_out->get_energy();
    const double c1 = TOWER_CALIB_C1->getTower(0)->get_energy();

    _eval_run.C2_sum = c2_in + c2_out;
    _eval_run.C1 = c1;

    cherekov_e = (_eval_run.C2_sum) > (abs(_eval_run.beam_mom) >= 10 ? 1000 : 1500);
    cherekov_anti_e = (_eval_run.C2_sum) < 100;
    hNormalization->Fill("C2-e", cherekov_e);

    TH2F *hCheck_Cherenkov = dynamic_cast<TH2F *>(hm->getHisto(
        "hCheck_Cherenkov"));
    assert(hCheck_Cherenkov);
    hCheck_Cherenkov->Fill(c1, "C1", 1);
    hCheck_Cherenkov->Fill(c2_in, "C2 in", 1);
    hCheck_Cherenkov->Fill(c2_out, "C2 out", 1);
    hCheck_Cherenkov->Fill(c2_in + c2_out, "C2 sum", 1);
  }

  // veto
  TH1F *hCheck_Veto = dynamic_cast<TH1F *>(hm->getHisto("hCheck_Veto"));
  assert(hCheck_Veto);
  bool trigger_veto_pass = true;
  if (not _is_sim)
  {
    auto range = TOWER_CALIB_TRIGGER_VETO->getTowers();
    for (auto it = range.first; it != range.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);

      hCheck_Veto->Fill(tower->get_energy());

      if (abs(tower->get_energy()) > 0.2)
        trigger_veto_pass = false;
    }
  }
  hNormalization->Fill("trigger_veto_pass", trigger_veto_pass);
  _eval_run.trigger_veto_pass = trigger_veto_pass;

  // hodoscope
  TH1F *hCheck_Hodo_H = dynamic_cast<TH1F *>(hm->getHisto("hCheck_Hodo_H"));
  assert(hCheck_Hodo_H);
  int hodo_h_count = 0;
  if (not _is_sim)
  {
    auto range = TOWER_CALIB_HODO_HORIZONTAL->getTowers();
    for (auto it = range.first; it != range.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);

      hCheck_Hodo_H->Fill(tower->get_energy());

      if (abs(tower->get_energy()) >  0.5)
      {
        hodo_h_count++;
        _eval_run.hodo_h = tower->get_id();
      }
    }
  }
  const bool valid_hodo_h = hodo_h_count == 1;
  hNormalization->Fill("valid_hodo_h", valid_hodo_h);
  _eval_run.valid_hodo_h = valid_hodo_h;

  TH1F *hCheck_Hodo_V = dynamic_cast<TH1F *>(hm->getHisto("hCheck_Hodo_V"));
  assert(hCheck_Hodo_V);
  int hodo_v_count = 0;
  if (not _is_sim)
  {
    auto range = TOWER_CALIB_HODO_VERTICAL->getTowers();
    for (auto it = range.first; it != range.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);

      hCheck_Hodo_V->Fill(tower->get_energy());

      if (abs(tower->get_energy()) >  0.5)
      {
        hodo_v_count++;
        _eval_run.hodo_v = tower->get_id();
      }
    }
  }
  const bool valid_hodo_v = hodo_v_count == 1;
  _eval_run.valid_hodo_v = valid_hodo_v;
  hNormalization->Fill("valid_hodo_v", valid_hodo_v);

  const bool good_e = (valid_hodo_v and valid_hodo_h and cherekov_e and trigger_veto_pass) and (not _is_sim);
  const bool good_anti_e = (valid_hodo_v and valid_hodo_h and cherekov_anti_e and trigger_veto_pass) and (not _is_sim);
  hNormalization->Fill("good_e", good_e);
  hNormalization->Fill("good_anti_e", good_anti_e);

  // process EMCAL LG
  double emcal_sum_lg_e_raw = 0;
  double emcal_sum_lg_e_calib = 0;
  {
    auto range_emcal_lg_raw = TOWER_RAW_CEMC->getTowers(); // raw
    for (auto it = range_emcal_lg_raw.first; it != range_emcal_lg_raw.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);
      // cout << "EMCAL RAW: ";
      // tower->identify();

      const int col = tower->get_bineta();
      const int row = tower->get_binphi();
      const int chanNum = row + 8*col; // temp for EMCal

      if (col < 0 or col >= 8)
        continue;
      if (row < 0 or row >= 8)
        continue;

      const double energy_raw = tower->get_energy(); // raw
      emcal_sum_lg_e_raw += energy_raw;
      _shower.emcal_lg_twr_raw[chanNum] = energy_raw;
    }
    _shower.emcal_lg_e_raw = emcal_sum_lg_e_raw;

    auto range_emcal_lg_calib = TOWER_CALIB_CEMC->getTowers(); // calib
    for (auto it = range_emcal_lg_calib.first; it != range_emcal_lg_calib.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);

      const int col = tower->get_bineta();
      const int row = tower->get_binphi();
      const int chanNum = row + 8*col; // temp for EMCal

      if (col < 0 or col >= 8)
        continue;
      if (row < 0 or row >= 8)
        continue;

      const double energy_calib = tower->get_energy();
      emcal_sum_lg_e_calib += energy_calib;
      _shower.emcal_lg_twr_calib[chanNum] = energy_calib;

      string HistName_calib = Form("h_emcal_lg_tower_%d_calib",chanNum);
      TH1F *h_emcal_lg_calib = dynamic_cast<TH1F *>(hm->getHisto(HistName_calib.c_str()));
      assert(h_emcal_lg_calib);
      h_emcal_lg_calib->Fill(_shower.emcal_lg_twr_calib[chanNum]);
    }
    _shower.emcal_lg_e_calib = emcal_sum_lg_e_calib;
  }

  // process HCALIN LG
  double hcalin_sum_lg_e_raw = 0;
  double hcalin_sum_lg_e_calib = 0;
  {
    auto range_hcalin_lg_raw = TOWER_RAW_LG_HCALIN->getTowers(); // raw
    for (auto it = range_hcalin_lg_raw.first; it != range_hcalin_lg_raw.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);
      // cout << "HCALIN RAW: ";
      // tower->identify();

      const int col = tower->get_bineta();
      const int row = tower->get_binphi();
      const int chanNum = getChannelNumber(row,col);

      if (col < 0 or col >= 4)
        continue;
      if (row < 0 or row >= 4)
        continue;

      const double energy_raw = tower->get_energy(); // raw
      hcalin_sum_lg_e_raw += energy_raw;
      _shower.hcalin_lg_twr_raw[chanNum] = energy_raw;
    }
    _shower.hcalin_lg_e_raw = hcalin_sum_lg_e_raw;

    auto range_hcalin_lg_calib = TOWER_CALIB_LG_HCALIN->getTowers(); // calib
    for (auto it = range_hcalin_lg_calib.first; it != range_hcalin_lg_calib.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);

      const int col = tower->get_bineta();
      const int row = tower->get_binphi();
      const int chanNum = getChannelNumber(row,col);

      if (col < 0 or col >= 4)
        continue;
      if (row < 0 or row >= 4)
        continue;

      const double energy_calib = tower->get_energy();
      hcalin_sum_lg_e_calib += energy_calib;
      _shower.hcalin_lg_twr_calib[chanNum] = energy_calib;

      string HistName_calib = Form("h_hcalin_lg_tower_%d_calib",chanNum);
      TH1F *h_hcalin_lg_calib = dynamic_cast<TH1F *>(hm->getHisto(HistName_calib.c_str()));
      assert(h_hcalin_lg_calib);
      h_hcalin_lg_calib->Fill(_shower.hcalin_lg_twr_calib[chanNum]);
    }
    _shower.hcalin_lg_e_calib = hcalin_sum_lg_e_calib;
  }

  // process HCALOUT LG
  double hcalout_sum_lg_e_raw = 0;
  double hcalout_sum_lg_e_calib = 0;
  {
    auto range_hcalout_lg_raw = TOWER_RAW_LG_HCALOUT->getTowers(); // raw
    for (auto it = range_hcalout_lg_raw.first; it != range_hcalout_lg_raw.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);
      // cout << "HCALOUT RAM: ";
      // tower->identify();

      const int col = tower->get_bineta();
      const int row = tower->get_binphi();
      const int chanNum = getChannelNumber(row,col);

      if (col < 0 or col >= 4)
        continue;
      if (row < 0 or row >= 4)
        continue;

      const double energy_raw = tower->get_energy(); // raw
      hcalout_sum_lg_e_raw += energy_raw;
      _shower.hcalout_lg_twr_raw[chanNum] = energy_raw;
    }
    _shower.hcalout_lg_e_raw = hcalout_sum_lg_e_raw;

    auto range_hcalout_lg_calib = TOWER_CALIB_LG_HCALOUT->getTowers(); // calib
    for (auto it = range_hcalout_lg_calib.first; it != range_hcalout_lg_calib.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);

      const int col = tower->get_bineta();
      const int row = tower->get_binphi();
      const int chanNum = getChannelNumber(row,col);

      if (col < 0 or col >= 4)
        continue;
      if (row < 0 or row >= 4)
        continue;

      const double energy_calib = tower->get_energy();
      hcalout_sum_lg_e_calib += energy_calib;
      _shower.hcalout_lg_twr_calib[chanNum] = energy_calib;

      string HistName_calib = Form("h_hcalout_lg_tower_%d_calib",chanNum);
      TH1F *h_hcalout_lg_calib = dynamic_cast<TH1F *>(hm->getHisto(HistName_calib.c_str()));
      assert(h_hcalout_lg_calib);
      h_hcalout_lg_calib->Fill(_shower.hcalout_lg_twr_calib[chanNum]);
    }
    _shower.hcalout_lg_e_calib = hcalout_sum_lg_e_calib;
  }

  float energy_sum_calib = emcal_sum_lg_e_calib + hcalout_sum_lg_e_calib;
  float asym_calib = (emcal_sum_lg_e_calib - hcalout_sum_lg_e_calib)/(emcal_sum_lg_e_calib + hcalout_sum_lg_e_calib);
  TH2F *h_tower_energy_calib = dynamic_cast<TH2F *>(hm->getHisto("h_tower_energy_calib"));
  assert(h_tower_energy_calib);
  if(emcal_sum_lg_e_calib > 0.0 && hcalout_sum_lg_e_calib > 0.0) 
  {
    h_tower_energy_calib->Fill(asym_calib,energy_sum_calib); 
  }

  _eval_run.sum_E_CEMC = emcal_sum_lg_e_calib;
  _eval_run.sum_E_HCAL_IN = hcalin_sum_lg_e_calib;
  _eval_run.sum_E_HCAL_OUT = hcalout_sum_lg_e_calib;

  _eval_run.good_e = good_e;
  _eval_run.good_anti_e = good_anti_e;

  TTree *T = dynamic_cast<TTree *>(hm->getHisto("HCAL_Info"));
  assert(T);
  T->Fill();

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4ShowerCalib::getChannelNumber(int row, int column)
{
  if(!_is_sim)
  {
    int hbdchanIHC[4][4] = {{4, 8, 12, 16},
                            {3, 7, 11, 15},
                            {2, 6, 10, 14},
                            {1, 5,  9, 13}};

    return hbdchanIHC[row][column] - 1;
  }

  if(_is_sim)
  {
    int hbdchanIHC[4][4] = {{13,  9, 5, 1},
                            {14, 10, 6, 2},
                            {15, 11, 7, 3},
                            {16, 12, 8, 4}};

    return hbdchanIHC[row][column] - 1;
  }

  return -1;
}

//---------------------Shower Calibration Analysis----------------------
int Proto4ShowerCalib::InitAna()
{
  if(_is_sim) _mMode = "SIM";
  if(!_is_sim) _mMode = "Calib";
  string inputdir = Form("/sphenix/user/xusun/TestBeam/ShowerCalib_2018c/%dGeV/",_mEnergy);
  string InPutList = Form("/direct/phenix+u/xusun/WorkSpace/sPHENIX/analysis/Prototype4/HCal/macros/list/ShowerCalib_2018c/Proto4ShowerInfo%s_%dGeV.list",_mMode.c_str(),_mEnergy);

  mChainInPut = new TChain("HCAL_Info");

  if (!InPutList.empty())   // if input file is ok
  { 
    cout << "Open input database file list: " << InPutList.c_str() << endl;
    ifstream in(InPutList.c_str());  // input stream
    if(in)
    { 
      cout << "input database file list is ok" << endl;
      char str[255];       // char array for each file name
      Long64_t entries_save = 0;
      while(in)
      { 
	in.getline(str,255);  // take the lines of the file list
	if(str[0] != 0)
	{ 
	  string addfile;
	  addfile = str;
	  addfile = inputdir+addfile;
	  mChainInPut->AddFile(addfile.c_str(),-1,"HCAL_Info");
	  long file_entries = mChainInPut->GetEntries();
	  cout << "File added to data chain: " << addfile.c_str() << " with " << (file_entries-entries_save) << " entries" << endl;
	  entries_save = file_entries;
	}
      }
    }
    else
    { 
      cout << "WARNING: input database file input is problemtic" << endl;
      _mInPut_flag = 0;
    }
  }

  // Set the input tree
  if (_mInPut_flag == 1 && !mChainInPut->GetBranch( "info" ))
  {
    cerr << "ERROR: Could not find branch 'info' in tree!" << endl;
  }

  _mInfo = new Proto4ShowerCalib::Eval_Run();
  _mShower = new Proto4ShowerCalib::HCAL_Shower();
  if(_mInPut_flag == 1)
  {
    mChainInPut->SetBranchAddress("info", &_mInfo);
    mChainInPut->SetBranchAddress("shower", &_mShower);

    long NumOfEvents = (long)mChainInPut->GetEntries();
    cout << "total number of events: " << NumOfEvents << endl;
    _mStartEvent = 0;
    _mStopEvent = 100;
    if(_mNumOfEvents < NumOfEvents) _mStopEvent = _mNumOfEvents;
    if(_mNumOfEvents < 0) _mStopEvent = NumOfEvents;

    cout << "New nStartEvent = " << _mStartEvent << ", new nStopEvent = " << _mStopEvent << endl;
  }

  if(!_is_sim)
  {
    h_mMomentum = new TH1F("h_mMomentum", "h_mMomentum", 241, -120.5, 120.5);
    h_mAsymmEnergy = new TH2F("h_mAsymmEnergy","h_mAsymmEnergy",105,-1.05,1.05,2000,-1.05,198.95);
    h_mAsymmEnergy_electron = new TH2F("h_mAsymmEnergy_electron","h_mAsymmEnergy_electron",105,-1.05,1.05,2000,-1.05,198.95);
    h_mAsymmEnergy_pion = new TH2F("h_mAsymmEnergy_pion","h_mAsymmEnergy_pion",105,-1.05,1.05,2000,-1.05,198.95);

    // balancing
    h_mAsymmEnergy_balancing = new TH2F("h_mAsymmEnergy_balancing","h_mAsymmEnergy_balancing",105,-1.05,1.05,2000,-1.05,198.95);
    h_mAsymmEnergy_electron_balancing = new TH2F("h_mAsymmEnergy_electron_balancing","h_mAsymmEnergy_electron_balancing",105,-1.05,1.05,2000,-1.05,198.95);
    h_mAsymmEnergy_pion_balancing = new TH2F("h_mAsymmEnergy_pion_balancing","h_mAsymmEnergy_pion_balancing",105,-1.05,1.05,2000,-1.05,198.95);

    // leveling correction
    h_mAsymmEnergy_leveling = new TH2F("h_mAsymmEnergy_leveling","h_mAsymmEnergy_leveling",105,-1.05,1.05,2000,-1.05,198.95);
    h_mAsymmEnergy_electron_leveling = new TH2F("h_mAsymmEnergy_electron_leveling","h_mAsymmEnergy_electron_leveling",105,-1.05,1.05,2000,-1.05,198.95);
    h_mAsymmEnergy_pion_leveling = new TH2F("h_mAsymmEnergy_pion_leveling","h_mAsymmEnergy_pion_leveling",105,-1.05,1.05,2000,-1.05,198.95);

    // shower correction
    h_mAsymmEnergy_showercalib = new TH2F("h_mAsymmEnergy_showercalib","h_mAsymmEnergy_showercalib",105,-1.05,1.05,2000,-1.05,198.95);
    h_mAsymmEnergy_electron_showercalib = new TH2F("h_mAsymmEnergy_electron_showercalib","h_mAsymmEnergy_electron_showercalib",105,-1.05,1.05,2000,-1.05,198.95);
    h_mAsymmEnergy_pion_showercalib = new TH2F("h_mAsymmEnergy_pion_showercalib","h_mAsymmEnergy_pion_showercalib",105,-1.05,1.05,2000,-1.05,198.95);

    // Outer HCal only study
    h_mAsymmEnergy_MIP = new TH2F("h_mAsymmEnergy_MIP","h_mAsymmEnergy_MIP",105,-1.05,1.05,2000,-1.05,198.95);
    h_mEnergyOut_electron = new TH1F("h_mEnergyOut_electron","h_mEnergyOut_electron",2000,-1.05,198.95);
    h_mEnergyOut_pion = new TH1F("h_mEnergyOut_pion","h_mEnergyOut_pion",2000,-1.05,198.95);
  }

  // initialize momIndex map
  const float momentum[12] = {3.0,4.0,5.0,6.0,8.0,12.0,16.0,20.0,24.0,28.0,40.0,50.0};
  map_momIndex.clear();
  for(int i_mom = 0; i_mom < 12; ++i_mom)
  {
    float temp_mom = momentum[i_mom];
    int temp_index = i_mom;
    map_momIndex[temp_mom] = temp_index;
  }
  /*
  for(std::map<float,int>::iterator it=map_momIndex.begin(); it!=map_momIndex.end(); ++it)
  {
    cout << it->first << " => " << it->second << endl;
  }
  */

  return 0;
}

int Proto4ShowerCalib::MakeAna()
{
  cout << "Make()" << endl;

  // const float c_in_leveling[12] = {0.797772, 1.0791, 0.880801, 0.911312, 0.786898, 0.82447, 0.797038, 0.807286, 0.790069, 0.783492, 0.790613, 0.793504};
  // const float c_out_leveling[12] = {1.33957, 0.931703, 1.15651, 1.10781, 1.37139, 1.27049, 1.34164, 1.31357, 1.36186, 1.38186, 1.36025, 1.35178}; 

  const float c_in_leveling[12] = {0.814269, 0.87434, 0.930247, 0.870155, 0.841073, 0.812163, 0.805016, 0.804375, 0.796104, 0.785911, 0.792748, 0.793076};
  const float c_out_leveling[12] = {1.2955, 1.16784, 1.08106, 1.17539, 1.23298, 1.30086, 1.31963, 1.32136, 1.3443, 1.3744, 1.35398, 1.35302};

  unsigned long start_event_use = _mStartEvent;
  unsigned long stop_event_use = _mStopEvent;

  mChainInPut->SetBranchAddress("info", &_mInfo);
  mChainInPut->SetBranchAddress("shower", &_mShower);
  mChainInPut->GetEntry(0);

  for(unsigned long i_event = start_event_use; i_event < stop_event_use; ++i_event)
  // for(unsigned long i_event = 20; i_event < 40; ++i_event)
  {
    if (!mChainInPut->GetEntry( i_event )) // take the event -> information is stored in event
      break;  // end of data chunk

    if (i_event != 0  &&  i_event % 1000 == 0)
      cout << "." << flush;
    if (i_event != 0  &&  i_event % 10000 == 0)
    {
      if((stop_event_use-start_event_use) > 0)
      {
	double event_percent = 100.0*((double)(i_event-start_event_use))/((double)(stop_event_use-start_event_use));
	cout << " " << i_event-start_event_use << " (" << event_percent << "%) " << "\n" << "==> Processing data (ShowerCalib) " << flush;
      }
    }

    if(!_is_sim) // beam test data
    {
      const bool good_electron = _mInfo->good_e;
      const bool good_pion = _mInfo->good_anti_e;

      const float beam_momentum = _mInfo->beam_mom;
      h_mMomentum->Fill(beam_momentum);

      // find momentum index
      int momIndex = -1;
      std::map<float,int>::iterator it_momId = map_momIndex.find(TMath::Abs(beam_momentum));
      if(it_momId == map_momIndex.end())
      {
	std::cout << "Make() -> could not find in beam lists!" << std::endl;
	return -999;
      }
      else
      {
	// std::cout << "Make() -> beam momentum: " << it_momId->first << " => momIndex: " << it_momId->second << std::endl;
	momIndex = it_momId->second;
      }
      const float c_in = c_in_leveling[momIndex];
      const float c_out = c_out_leveling[momIndex];

      const float energy_emcal_calib = _mShower->emcal_lg_e_calib;
      const float energy_hcalout_calib = _mShower->hcalout_lg_e_calib;

      float energy_calib = energy_emcal_calib + energy_hcalout_calib;
      float asymm_calib = (energy_emcal_calib - energy_hcalout_calib)/(energy_emcal_calib + energy_hcalout_calib);

      if(energy_emcal_calib > 0.001 && energy_hcalout_calib > 0.001) // remove ped
      { // extract MIP
	h_mAsymmEnergy->Fill(asymm_calib,energy_calib);
	if(good_electron) h_mAsymmEnergy_electron->Fill(asymm_calib,energy_calib);
	if(good_pion) h_mAsymmEnergy_pion->Fill(asymm_calib,energy_calib);
      }

      // 3 sigma MIP energy cut for Balancing Correction
      const double MIP_energy_cut = MIP_mean+3.0*MIP_width;
      if(energy_emcal_calib > 0.01 && energy_hcalout_calib > 0.001 && energy_calib > MIP_energy_cut)
      { 
	// balancing without muon
	h_mAsymmEnergy_balancing->Fill(asymm_calib,energy_calib);
	if(good_electron) h_mAsymmEnergy_electron_balancing->Fill(asymm_calib,energy_calib);
	if(good_pion) h_mAsymmEnergy_pion_balancing->Fill(asymm_calib,energy_calib);
      }

      if(energy_emcal_calib > 0.001 && energy_hcalout_calib > 0.001 && energy_calib > MIP_energy_cut)
      {
	// apply leveling
	const float energy_leveling = c_in*energy_emcal_calib + c_out*energy_hcalout_calib;
	const float asymm_leveling = (c_in*energy_emcal_calib - c_out*energy_hcalout_calib)/energy_leveling;
	h_mAsymmEnergy_leveling->Fill(asymm_leveling,energy_leveling);
	if(good_electron) h_mAsymmEnergy_electron_leveling->Fill(asymm_leveling,energy_leveling);
	if(good_pion) h_mAsymmEnergy_pion_leveling->Fill(asymm_leveling,energy_leveling);
      }

      if(energy_emcal_calib > 0.001 && energy_hcalout_calib > 0.001 && energy_calib > MIP_energy_cut)
      {
	// apply shower calibration
	const float energy_showercalib = showercalib*c_in*energy_emcal_calib + showercalib*c_out*energy_hcalout_calib;
	const float asymm_showercalib = (showercalib*c_in*energy_emcal_calib - showercalib*c_out*energy_hcalout_calib)/energy_showercalib;
	h_mAsymmEnergy_showercalib->Fill(asymm_showercalib,energy_showercalib);
	if(good_electron) h_mAsymmEnergy_showercalib->Fill(asymm_showercalib,energy_showercalib);
	if(good_pion) h_mAsymmEnergy_pion_showercalib->Fill(asymm_showercalib,energy_showercalib);
      }


      // 1 sigma MIP energy cut for HCALOUT only study
      const double MIP_cut = MIP_mean+MIP_width;
      if(energy_emcal_calib <= MIP_cut && energy_hcalout_calib > 0.001 && energy_calib > MIP_cut)
      { // OHCal with MIP event through EMCal
	h_mAsymmEnergy_MIP->Fill(asymm_calib,energy_calib);
	if(good_electron) h_mEnergyOut_electron->Fill(energy_hcalout_calib);
	if(good_pion) h_mEnergyOut_pion->Fill(energy_hcalout_calib);
      }

    }
  }

  cout << "." << flush;
  cout << " " << stop_event_use-start_event_use << "(" << 100 << "%)";
  cout << endl;

  return 1;
}

int Proto4ShowerCalib::FinishAna()
{
  cout << "Finish()" << endl;
  mFile_OutPut = new TFile(_filename.c_str(),"RECREATE");
  mFile_OutPut->cd();
  if(!_is_sim) // beam test data
  {
    h_mMomentum->Write();
    h_mAsymmEnergy->Write();
    h_mAsymmEnergy_electron->Write();
    h_mAsymmEnergy_pion->Write();

    h_mAsymmEnergy_balancing->Write();
    h_mAsymmEnergy_electron_balancing->Write();
    h_mAsymmEnergy_pion_balancing->Write();

    h_mAsymmEnergy_leveling->Write();
    h_mAsymmEnergy_electron_leveling->Write();
    h_mAsymmEnergy_pion_leveling->Write();

    h_mAsymmEnergy_showercalib->Write();
    h_mAsymmEnergy_electron_showercalib->Write();
    h_mAsymmEnergy_pion_showercalib->Write();

    h_mAsymmEnergy_MIP->Write();
    h_mEnergyOut_electron->Write();
    h_mEnergyOut_pion->Write();
  }
  mFile_OutPut->Close();

  return 1;
}
