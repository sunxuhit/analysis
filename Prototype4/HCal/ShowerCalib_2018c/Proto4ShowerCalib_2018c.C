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

  // process HCALIN LG
  double emcal_sum_lg_e_raw = 0;
  double emcal_sum_lg_e_calib = 0;
  {
    auto range_emcal_lg_raw = TOWER_RAW_CEMC->getTowers(); // raw
    for (auto it = range_emcal_lg_raw.first; it != range_emcal_lg_raw.second; ++it)
    {
      RawTower *tower = it->second;
      assert(tower);
      // cout << "HCALIN RAW: ";
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

  float energy_calib = emcal_sum_lg_e_calib + hcalout_sum_lg_e_calib;
  float asym_calib = (emcal_sum_lg_e_calib - hcalout_sum_lg_e_calib)/(emcal_sum_lg_e_calib + hcalout_sum_lg_e_calib);
  TH2F *h_tower_energy_calib = dynamic_cast<TH2F *>(hm->getHisto("h_tower_energy_calib"));
  assert(h_tower_energy_calib);
  if(emcal_sum_lg_e_calib > 0.0 && hcalout_sum_lg_e_calib > 0.0) h_tower_energy_calib->Fill(asym_calib,energy_calib); 

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
