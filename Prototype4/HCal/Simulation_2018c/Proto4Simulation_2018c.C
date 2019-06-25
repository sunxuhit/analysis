#include "Proto4Simulation_2018c.h"

#include <g4main/PHG4Hit.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4VtxPoint.h>
           
#include <calobase/RawTowerContainer.h>
#include <calobase/RawTowerGeomContainer.h>
#include <calobase/RawTower.h>
#include <calobase/RawClusterContainer.h>
#include <calobase/RawCluster.h>
           
#include <g4eval/CaloEvalStack.h>
#include <g4eval/CaloRawClusterEval.h> 
#include <g4eval/CaloRawTowerEval.h>
#include <g4eval/CaloTruthEval.h>
#include <g4eval/SvtxEvalStack.h>
           
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/PHTFileServer.h>
#include <fun4all/Fun4AllReturnCodes.h>
           
#include <phool/getClass.h>
#include <phool/PHCompositeNode.h>
           
#include <TString.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TMath.h>
           
#include <exception>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cassert>
#include <cmath>

using namespace std;

Proto4Simulation::Proto4Simulation(const std::string &filename)
  : SubsysReco("Proto4Simulation"),
  _filename(filename)
{
  Verbosity(1);

  mG4HIT_CEMC               = NULL;
  mG4ABS_CEMC               = NULL;
  mG4HIT_HCALIN             = NULL;
  mG4ABS_HCALIN             = NULL;
  mG4HIT_HCALOUT            = NULL;
  mG4ABS_HCALOUT            = NULL;
  mG4HIT_CRYO               = NULL;
  mG4HIT_BLACKHOLE          = NULL;

  mTOWER_CALIB_LG_CEMC    = NULL;
  mTOWER_CALIB_LG_HCALIN  = NULL;
  mTOWER_CALIB_LG_HCALOUT = NULL;

  _truth_container        = NULL;

  _sim.reset();

  _mStartEvent = -1;
  _mStopEvent = -1;
  _mMom = -1;
  _mNumOfEvents = -1;
}

Proto4Simulation::~Proto4Simulation()
{
}

Fun4AllHistoManager* Proto4Simulation::get_HistoManager()
{
  Fun4AllServer *se = Fun4AllServer::instance();
  Fun4AllHistoManager *hm = se->getHistoManager("Proto4Simulation_HISTOS");

  if (not hm)
  {
    cout
        << "Proto4Simulation::get_HistoManager - Making Fun4AllHistoManager Proto4Simulation_HISTOS"
        << endl;
    hm = new Fun4AllHistoManager("Proto4Simulation_HISTOS");
    se->registerHistoManager(hm);
  }

  assert(hm);

  return hm;
}

int Proto4Simulation::InitRun(PHCompositeNode *topNode)
{
  if (Verbosity())
    cout << "Proto4Simulation::InitRun" << endl;

  PHNodeIterator iter(topNode);
  PHCompositeNode *dstNode = static_cast<PHCompositeNode *>(iter.findFirst(
      "PHCompositeNode", "DST"));
  assert(dstNode);

  if (!dstNode)
  {
    std::cerr << PHWHERE << "DST Node missing, doing nothing." << std::endl;
    throw runtime_error(
        "Failed to find DST node in EmcRawTowerBuilder::CreateNodes");
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4Simulation::End(PHCompositeNode *topNode)
{
  cout << "Proto4Simulation::End - write to " << _filename << endl;
  PHTFileServer::get().cd(_filename);

  Fun4AllHistoManager *hm = get_HistoManager();
  assert(hm);
  for (unsigned int i = 0; i < hm->nHistos(); i++)
    hm->getHisto(i)->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4Simulation::Init(PHCompositeNode *topNode)
{
  cout << "Proto4Simulation::get_HistoManager - Making PHTFileServer "
       << _filename << endl;
  PHTFileServer::get().open(_filename, "RECREATE");

  int is_histo = initHisto();
  if(is_histo != Fun4AllReturnCodes::EVENT_OK) 
  {
    cout << "Proto4Simulation::initHisto - Fatal Error - "
      << "unable to initialize histograms" << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4Simulation::process_event(PHCompositeNode *topNode)
{
  if (Verbosity() > 2) cout << "Proto4Simulation::process_event() entered" << endl;
  if( getNodes(topNode) != Fun4AllReturnCodes::EVENT_OK) return Fun4AllReturnCodes::DISCARDEVENT;

  // init event object
  _sim.reset();

  Fun4AllHistoManager *hm = get_HistoManager();
  assert(hm);

  // get primary
  assert(_truth_container); 
  PHG4TruthInfoContainer::ConstRange primary_range = _truth_container->GetPrimaryParticleRange();
  double total_primary_energy = 1e-9; //make it zero energy epsilon samll so it can be used for denominator
  for (PHG4TruthInfoContainer::ConstIterator particle_iter = primary_range.first;
      particle_iter != primary_range.second; ++particle_iter)
  {           
    const PHG4Particle *particle = particle_iter->second;
    assert(particle);     
    total_primary_energy += particle->get_e();
  }           
  TH1F * h_mBeamEnergy = dynamic_cast<TH1F*>(hm->getHisto("h_mBeamEnergy"));
  assert(h_mBeamEnergy);
  h_mBeamEnergy->Fill(total_primary_energy);
  _sim.truth_e = total_primary_energy;

  TH1F *h_info = dynamic_cast<TH1F*>(hm->getHisto("h_Normalization"));
  assert(h_info);

  float eHIT_CEMC = 0.0; // active energy deposition
  float eABS_CEMC = 0.0; 
  float eHIT_HCALIN = 0.0;
  float eABS_HCALIN = 0.0;
  float eHIT_HCALOUT = 0.0;
  float eABS_HCALOUT = 0.0;
  float eHIT_CRYO = 0.0;
  float eHIT_BLACKHOLE = 0.0;

  // G4Hit CEMC
  if (mG4HIT_CEMC)
  {
    TH2F *h_mG4HitXY_CEMC = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXY_CEMC"));
    assert(h_mG4HitXY_CEMC);

    TH2F *h_mG4HitXZ_CEMC = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXZ_CEMC"));
    assert(h_mG4HitXZ_CEMC);

    TH1F * h_mEHit_CEMC = dynamic_cast<TH1F*>(hm->getHisto("h_mEHit_CEMC"));
    assert(h_mEHit_CEMC);

    PHG4HitContainer::ConstRange cemc_hit_range = mG4HIT_CEMC->getHits();
    for (PHG4HitContainer::ConstIterator hit_iter = cemc_hit_range.first;
	hit_iter != cemc_hit_range.second; hit_iter++)
    {

      PHG4Hit *this_hit = hit_iter->second;
      assert(this_hit);

      eHIT_CEMC += this_hit->get_edep();

      const TVector3 hit(this_hit->get_avg_x(), this_hit->get_avg_y(), this_hit->get_avg_z());
      h_mG4HitXY_CEMC->Fill(hit.X(), hit.Y(), this_hit->get_edep());
      h_mG4HitXZ_CEMC->Fill(hit.X(), hit.Z(), this_hit->get_edep());
    }
    h_mEHit_CEMC->Fill(eHIT_CEMC);
    _sim.emcal_e_hit = eHIT_CEMC;
    h_info->Fill("CEMC Active", mG4HIT_CEMC->size());
  }

  if (mG4ABS_CEMC)
  {
    TH2F *h_mG4AbsXY_CEMC = dynamic_cast<TH2F*>(hm->getHisto("h_mG4AbsXY_CEMC"));
    assert(h_mG4AbsXY_CEMC);

    TH2F *h_mG4AbsXZ_CEMC = dynamic_cast<TH2F*>(hm->getHisto("h_mG4AbsXZ_CEMC"));
    assert(h_mG4AbsXZ_CEMC);

    TH1F * h_mEAbs_CEMC = dynamic_cast<TH1F*>(hm->getHisto("h_mEAbs_CEMC"));
    assert(h_mEAbs_CEMC);

    PHG4HitContainer::ConstRange cemc_abs_range = mG4ABS_CEMC->getHits();
    for (PHG4HitContainer::ConstIterator hit_iter = cemc_abs_range.first;
	hit_iter != cemc_abs_range.second; hit_iter++)
    {

      PHG4Hit *this_hit = hit_iter->second;
      assert(this_hit);

      eABS_CEMC += this_hit->get_edep();

      const TVector3 hit(this_hit->get_avg_x(), this_hit->get_avg_y(), this_hit->get_avg_z());
      h_mG4AbsXY_CEMC->Fill(hit.X(), hit.Y(), this_hit->get_edep());
      h_mG4AbsXZ_CEMC->Fill(hit.X(), hit.Z(), this_hit->get_edep());
    }
    h_mEAbs_CEMC->Fill(eABS_CEMC);
    _sim.emcal_e_abs = eABS_CEMC;
    h_info->Fill("CEMC Absor.", mG4ABS_CEMC->size());
  }

  // G4Hit HCALIN
  if (mG4HIT_HCALIN)
  {
    TH2F *h_mG4HitXY_HCALIN = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXY_HCALIN"));
    assert(h_mG4HitXY_HCALIN);

    TH2F *h_mG4HitXZ_HCALIN = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXZ_HCALIN"));
    assert(h_mG4HitXZ_HCALIN);

    TH1F * h_mEHit_HCALIN = dynamic_cast<TH1F*>(hm->getHisto("h_mEHit_HCALIN"));
    assert(h_mEHit_HCALIN);

    PHG4HitContainer::ConstRange hcalin_hit_range = mG4HIT_HCALIN->getHits();
    for (PHG4HitContainer::ConstIterator hit_iter = hcalin_hit_range.first;
	hit_iter != hcalin_hit_range.second; hit_iter++)
    {

      PHG4Hit *this_hit = hit_iter->second;
      assert(this_hit);

      eHIT_HCALIN += this_hit->get_edep();

      const TVector3 hit(this_hit->get_avg_x(), this_hit->get_avg_y(), this_hit->get_avg_z());
      h_mG4HitXY_HCALIN->Fill(hit.X(), hit.Y(), this_hit->get_edep());
      h_mG4HitXZ_HCALIN->Fill(hit.X(), hit.Z(), this_hit->get_edep());
    }
    h_mEHit_HCALIN->Fill(eHIT_HCALIN);
    _sim.hcalin_e_hit = eHIT_HCALIN;
    h_info->Fill("HCALIN Active", mG4HIT_HCALIN->size());
  }

  if (mG4ABS_HCALIN)
  {
    TH2F *h_mG4AbsXY_HCALIN = dynamic_cast<TH2F*>(hm->getHisto("h_mG4AbsXY_HCALIN"));
    assert(h_mG4AbsXY_HCALIN);

    TH2F *h_mG4AbsXZ_HCALIN = dynamic_cast<TH2F*>(hm->getHisto("h_mG4AbsXZ_HCALIN"));
    assert(h_mG4AbsXZ_HCALIN);

    TH1F * h_mEAbs_HCALIN = dynamic_cast<TH1F*>(hm->getHisto("h_mEAbs_HCALIN"));
    assert(h_mEAbs_HCALIN);

    PHG4HitContainer::ConstRange hcalin_abs_range = mG4ABS_HCALIN->getHits();
    for (PHG4HitContainer::ConstIterator hit_iter = hcalin_abs_range.first;
	hit_iter != hcalin_abs_range.second; hit_iter++)
    {

      PHG4Hit *this_hit = hit_iter->second;
      assert(this_hit);

      eABS_HCALIN += this_hit->get_edep();

      const TVector3 hit(this_hit->get_avg_x(), this_hit->get_avg_y(), this_hit->get_avg_z());
      h_mG4AbsXY_HCALIN->Fill(hit.X(), hit.Y(), this_hit->get_edep());
      h_mG4AbsXZ_HCALIN->Fill(hit.X(), hit.Z(), this_hit->get_edep());
    }
    h_mEAbs_HCALIN->Fill(eABS_HCALIN);
    _sim.hcalin_e_abs = eABS_HCALIN;
    h_info->Fill("HCALIN Absor.", mG4ABS_HCALIN->size());
  }

  // G4Hit HCALOUT
  if (mG4HIT_HCALOUT)
  {
    TH2F *h_mG4HitXY_HCALOUT = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXY_HCALOUT"));
    assert(h_mG4HitXY_HCALOUT);

    TH2F *h_mG4HitXZ_HCALOUT = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXZ_HCALOUT"));
    assert(h_mG4HitXZ_HCALOUT);

    TH1F * h_mEHit_HCALOUT = dynamic_cast<TH1F*>(hm->getHisto("h_mEHit_HCALOUT"));
    assert(h_mEHit_HCALOUT);

    PHG4HitContainer::ConstRange hcalout_hit_range = mG4HIT_HCALOUT->getHits();
    for (PHG4HitContainer::ConstIterator hit_iter = hcalout_hit_range.first;
	hit_iter != hcalout_hit_range.second; hit_iter++)
    {

      PHG4Hit *this_hit = hit_iter->second;
      assert(this_hit);

      eHIT_HCALOUT += this_hit->get_edep();

      const TVector3 hit(this_hit->get_avg_x(), this_hit->get_avg_y(), this_hit->get_avg_z());
      h_mG4HitXY_HCALOUT->Fill(hit.X(), hit.Y(), this_hit->get_edep());
      h_mG4HitXZ_HCALOUT->Fill(hit.X(), hit.Z(), this_hit->get_edep());
    }
    h_mEHit_HCALOUT->Fill(eHIT_HCALOUT);
    _sim.hcalout_e_hit = eHIT_HCALOUT;
    h_info->Fill("HCALOUT Active", mG4HIT_HCALOUT->size());
  }

  if (mG4ABS_HCALOUT)
  {
    TH2F *h_mG4AbsXY_HCALOUT = dynamic_cast<TH2F*>(hm->getHisto("h_mG4AbsXY_HCALOUT"));
    assert(h_mG4AbsXY_HCALOUT);

    TH2F *h_mG4AbsXZ_HCALOUT = dynamic_cast<TH2F*>(hm->getHisto("h_mG4AbsXZ_HCALOUT"));
    assert(h_mG4AbsXZ_HCALOUT);

    TH1F * h_mEAbs_HCALOUT = dynamic_cast<TH1F*>(hm->getHisto("h_mEAbs_HCALOUT"));
    assert(h_mEAbs_HCALOUT);

    PHG4HitContainer::ConstRange hcalout_abs_range = mG4ABS_HCALOUT->getHits();
    for (PHG4HitContainer::ConstIterator hit_iter = hcalout_abs_range.first;
	hit_iter != hcalout_abs_range.second; hit_iter++)
    {

      PHG4Hit *this_hit = hit_iter->second;
      assert(this_hit);

      eABS_HCALOUT += this_hit->get_edep();

      const TVector3 hit(this_hit->get_avg_x(), this_hit->get_avg_y(), this_hit->get_avg_z());
      h_mG4AbsXY_HCALOUT->Fill(hit.X(), hit.Y(), this_hit->get_edep());
      h_mG4AbsXZ_HCALOUT->Fill(hit.X(), hit.Z(), this_hit->get_edep());
    }
    h_mEAbs_HCALOUT->Fill(eABS_HCALOUT);
    _sim.hcalout_e_abs = eABS_HCALOUT;
    h_info->Fill("HCALOUT Absor.", mG4ABS_HCALOUT->size());
  }

  // G4Hit CRYO
  if (mG4HIT_CRYO)
  {
    TH2F *h_mG4HitXY_CRYO = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXY_CRYO"));
    assert(h_mG4HitXY_CRYO);

    TH2F *h_mG4HitXZ_CRYO = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXZ_CRYO"));
    assert(h_mG4HitXZ_CRYO);

    TH1F * h_mEHit_CRYO = dynamic_cast<TH1F*>(hm->getHisto("h_mEHit_CRYO"));
    assert(h_mEHit_CRYO);

    PHG4HitContainer::ConstRange cryo_hit_range = mG4HIT_CRYO->getHits();
    for (PHG4HitContainer::ConstIterator hit_iter = cryo_hit_range.first;
	hit_iter != cryo_hit_range.second; hit_iter++)
    {

      PHG4Hit *this_hit = hit_iter->second;
      assert(this_hit);

      eHIT_CRYO += this_hit->get_edep();

      const TVector3 hit(this_hit->get_avg_x(), this_hit->get_avg_y(), this_hit->get_avg_z());
      h_mG4HitXY_CRYO->Fill(hit.X(), hit.Y(), this_hit->get_edep());
      h_mG4HitXZ_CRYO->Fill(hit.X(), hit.Z(), this_hit->get_edep());
    }
    h_mEHit_CRYO->Fill(eHIT_CRYO);
    _sim.cryo_e_hit = eHIT_CRYO;
    h_info->Fill("CRYO Active", mG4HIT_CRYO->size());
  }

  // G4Hit BlackHole
  if (mG4HIT_BLACKHOLE)
  {
    TH2F *h_mG4HitXY_BLACKHOLE = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXY_BLACKHOLE"));
    assert(h_mG4HitXY_BLACKHOLE);

    TH2F *h_mG4HitXZ_BLACKHOLE = dynamic_cast<TH2F*>(hm->getHisto("h_mG4HitXZ_BLACKHOLE"));
    assert(h_mG4HitXZ_BLACKHOLE);

    TH1F * h_mEHit_BLACKHOLE = dynamic_cast<TH1F*>(hm->getHisto("h_mEHit_BLACKHOLE"));
    assert(h_mEHit_BLACKHOLE);

    PHG4HitContainer::ConstRange cryo_hit_range = mG4HIT_BLACKHOLE->getHits();
    for (PHG4HitContainer::ConstIterator hit_iter = cryo_hit_range.first;
	hit_iter != cryo_hit_range.second; hit_iter++)
    {

      PHG4Hit *this_hit = hit_iter->second;
      assert(this_hit);

      eHIT_BLACKHOLE += this_hit->get_edep();

      const TVector3 hit(this_hit->get_avg_x(), this_hit->get_avg_y(), this_hit->get_avg_z());
      h_mG4HitXY_BLACKHOLE->Fill(hit.X(), hit.Y(), this_hit->get_edep());
      h_mG4HitXZ_BLACKHOLE->Fill(hit.X(), hit.Z(), this_hit->get_edep());
    }
    h_mEHit_BLACKHOLE->Fill(eHIT_BLACKHOLE);
    _sim.blackhole_e_hit = eHIT_BLACKHOLE;
    h_info->Fill("BLACKHOLE Active", mG4HIT_BLACKHOLE->size());
  }

  // at the end, count success events
  h_info->Fill("Event", 1); 

  TH1F * h_mEnergy_Vis = dynamic_cast<TH1F*>(hm->getHisto("h_mEnergy_Vis"));
  assert(h_mEnergy_Vis);
  const double energy_vis = eHIT_CEMC + eABS_CEMC + eHIT_HCALOUT + eABS_HCALOUT;
  h_mEnergy_Vis->Fill(energy_vis);

  TH1F * h_mEnergy_Leak = dynamic_cast<TH1F*>(hm->getHisto("h_mEnergy_Leak"));
  assert(h_mEnergy_Leak);
  const double energy_leak = eHIT_HCALIN + eABS_HCALIN + eHIT_CRYO + eHIT_BLACKHOLE;
  h_mEnergy_Leak->Fill(energy_leak);

  TH1F * h_mEnergy_total = dynamic_cast<TH1F*>(hm->getHisto("h_mEnergy_total"));
  assert(h_mEnergy_total);
  h_mEnergy_total->Fill(energy_vis+energy_leak);

  TH2F * h_mG4Energy = dynamic_cast<TH2F*>(hm->getHisto("h_mG4Energy"));
  assert(h_mG4Energy);
  h_mG4Energy->Fill(total_primary_energy,energy_vis+energy_leak);

  // EMCAL TOWER LG
  float emcal_sum_lg_e_calib = 0;
  {
    auto range_emcal_lg_calib = mTOWER_CALIB_LG_CEMC->getTowers(); // calib
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

      const float energy_calib = tower->get_energy();
      emcal_sum_lg_e_calib += energy_calib;
      _sim.emcal_lg_twr_calib[chanNum] = energy_calib;

      string HistName_calib = Form("h_emcal_lg_tower_%d_calib",chanNum);
      TH1F *h_emcal_lg_calib = dynamic_cast<TH1F *>(hm->getHisto(HistName_calib.c_str()));
      assert(h_emcal_lg_calib);
      h_emcal_lg_calib->Fill(_sim.emcal_lg_twr_calib[chanNum]);
    }
    _sim.emcal_lg_e_calib = emcal_sum_lg_e_calib;
  }

  // process HCALIN LG
  float hcalin_sum_lg_e_calib = 0;
  {
    auto range_hcalin_lg_calib = mTOWER_CALIB_LG_HCALIN->getTowers(); // calib
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
      _sim.hcalin_lg_twr_calib[chanNum] = energy_calib;

      string HistName_calib = Form("h_hcalin_lg_tower_%d_calib",chanNum);
      TH1F *h_hcalin_lg_calib = dynamic_cast<TH1F *>(hm->getHisto(HistName_calib.c_str()));
      assert(h_hcalin_lg_calib);
      h_hcalin_lg_calib->Fill(_sim.hcalin_lg_twr_calib[chanNum]);
    }
    _sim.hcalin_lg_e_calib = hcalin_sum_lg_e_calib;
  }

  // process HCALOUT LG
  float hcalout_sum_lg_e_calib = 0;
  {
    auto range_hcalout_lg_calib = mTOWER_CALIB_LG_HCALOUT->getTowers(); // calib
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

      const float energy_calib = tower->get_energy();
      hcalout_sum_lg_e_calib += energy_calib;
      _sim.hcalout_lg_twr_calib[chanNum] = energy_calib;

      string HistName_calib = Form("h_hcalout_lg_tower_%d_calib",chanNum);
      TH1F *h_hcalout_lg_calib = dynamic_cast<TH1F *>(hm->getHisto(HistName_calib.c_str()));
      assert(h_hcalout_lg_calib);
      h_hcalout_lg_calib->Fill(_sim.hcalout_lg_twr_calib[chanNum]);
    }
    _sim.hcalout_lg_e_calib = hcalout_sum_lg_e_calib;
  }

  float energy_sum_calib = emcal_sum_lg_e_calib + hcalout_sum_lg_e_calib;
  float asym_calib = (emcal_sum_lg_e_calib - hcalout_sum_lg_e_calib)/(emcal_sum_lg_e_calib + hcalout_sum_lg_e_calib);
  TH2F *h_tower_energy_calib = dynamic_cast<TH2F *>(hm->getHisto("h_tower_energy_calib"));
  assert(h_tower_energy_calib);
  if(emcal_sum_lg_e_calib > 0.0 && hcalout_sum_lg_e_calib > 0.0) 
  {
    h_tower_energy_calib->Fill(asym_calib,energy_sum_calib); 
  }

  TTree *T = dynamic_cast<TTree *>(hm->getHisto("HCAL_Info"));
  assert(T);
  T->Fill();

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4Simulation::getNodes(PHCompositeNode *topNode)
{
  // G4Hit for EMCal 
  mG4HIT_CEMC = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_CEMC");
  if (!mG4HIT_CEMC)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_CEMC" << endl;
    assert(mG4HIT_CEMC);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  mG4ABS_CEMC = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_ABSORBER_CEMC");
  if (!mG4ABS_CEMC)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_ABSORBER_CEMC" << endl;
    assert(mG4ABS_CEMC);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // G4Hit for Inner HCal 
  mG4HIT_HCALIN = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_HCALIN");
  if (!mG4HIT_HCALIN)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_HCALIN" << endl;
    assert(mG4HIT_HCALIN);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  mG4ABS_HCALIN = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_ABSORBER_HCALIN");
  if (!mG4ABS_HCALIN)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_ABSORBER_HCALIN" << endl;
    assert(mG4ABS_HCALIN);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // G4Hit for Outer HCal 
  mG4HIT_HCALOUT = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_HCALOUT");
  if (!mG4HIT_HCALOUT)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_HCALOUT" << endl;
    assert(mG4HIT_HCALOUT);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  mG4ABS_HCALOUT = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_ABSORBER_HCALOUT");
  if (!mG4ABS_HCALOUT)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_ABSORBER_HCALOUT" << endl;
    assert(mG4ABS_HCALOUT);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // G4Hit for Cryo
  mG4HIT_CRYO = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_CRYO");
  if (!mG4HIT_CRYO)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_CRYO" << endl;
    assert(mG4HIT_CRYO);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // G4Hit for BlackHole
  mG4HIT_BLACKHOLE = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_BlackHole");
  if (!mG4HIT_BLACKHOLE)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_BlackHole" << endl;
    assert(mG4HIT_BLACKHOLE);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // tower for EMCal
  mTOWER_CALIB_LG_CEMC = findNode::getClass<RawTowerContainer>(topNode, "TOWER_CALIB_LG_CEMC");
  if (!mTOWER_CALIB_LG_CEMC)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "TOWER_CALIB_LG_CEMC" << endl;
    assert(mTOWER_CALIB_LG_CEMC);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // tower for Inner HCal 
  mTOWER_CALIB_LG_HCALIN = findNode::getClass<RawTowerContainer>(topNode, "TOWER_CALIB_LG_HCALIN");
  if (!mTOWER_CALIB_LG_HCALIN)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "TOWER_CALIB_LG_HCALIN" << endl;
    assert(mTOWER_CALIB_LG_HCALIN);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // tower for Outer HCal 
  mTOWER_CALIB_LG_HCALOUT = findNode::getClass<RawTowerContainer>(topNode, "TOWER_CALIB_LG_HCALOUT");
  if (!mTOWER_CALIB_LG_HCALOUT)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "TOWER_CALIB_LG_HCALOUT" << endl;
    assert(mTOWER_CALIB_LG_HCALOUT);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  _truth_container = findNode::getClass<PHG4TruthInfoContainer>(topNode, "G4TruthInfo");
  if (!_truth_container)
  {      
    cout << "Proto4Simulation::InitRun - Fatal Error - "
      << "unable to find DST node " << "G4TruthInfo" << endl;
    assert(_truth_container);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }     

  return Fun4AllReturnCodes::EVENT_OK;
}


int Proto4Simulation::initHisto()
{
  Fun4AllHistoManager *hm = get_HistoManager();
  assert(hm);

  TH1F * h_info = new TH1F("h_Normalization","Normalization;Items;Count", 10, .5, 10.5);
  int i = 1;
  h_info->GetXaxis()->SetBinLabel(i++, "Event");
  h_info->GetXaxis()->SetBinLabel(i++, "CEMC Active");
  h_info->GetXaxis()->SetBinLabel(i++, "CEMC Absor.");
  h_info->GetXaxis()->SetBinLabel(i++, "HCALIN Active");
  h_info->GetXaxis()->SetBinLabel(i++, "HCALIN Absor.");
  h_info->GetXaxis()->SetBinLabel(i++, "HCALOUT Active");
  h_info->GetXaxis()->SetBinLabel(i++, "HCALOUT Absor.");
  h_info->GetXaxis()->SetBinLabel(i++, "CRYO Active");
  h_info->GetXaxis()->SetBinLabel(i++, "BLACKHOLE Active");
  h_info->GetXaxis()->LabelsOption("v");
  hm->registerHisto(h_info);

  // G4Hit for EMCal => xy & xz & energy
  TH2F *h_mG4HitXY_CEMC = new TH2F("h_mG4HitXY_CEMC","h_mG4HitXY_CEMC",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_CEMC);

  TH2F *h_mG4HitXZ_CEMC = new TH2F("h_mG4HitXZ_CEMC","h_mG4HitXZ_CEMC",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_CEMC);

  TH1F *h_mEHit_CEMC = new TH1F("h_mEHit_CEMC","h_mEHit_CEMC",2000,-1.05,198.95);
  hm->registerHisto(h_mEHit_CEMC);

  TH2F *h_mG4AbsXY_CEMC = new TH2F("h_mG4AbsXY_CEMC","h_mG4AbsXY_CEMC",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4AbsXY_CEMC);

  TH2F *h_mG4AbsXZ_CEMC = new TH2F("h_mG4AbsXZ_CEMC","h_mG4AbsXZ_CEMC",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4AbsXZ_CEMC);

  TH1F *h_mEAbs_CEMC = new TH1F("h_mEAbs_CEMC","h_mEAbs_CEMC",2000,-1.05,198.95);
  hm->registerHisto(h_mEAbs_CEMC);

  // G4Hit for Inner HCal 
  TH2F *h_mG4HitXY_HCALIN = new TH2F("h_mG4HitXY_HCALIN","h_mG4HitXY_HCALIN",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_HCALIN);

  TH2F *h_mG4HitXZ_HCALIN = new TH2F("h_mG4HitXZ_HCALIN","h_mG4HitXZ_HCALIN",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_HCALIN);

  TH1F *h_mEHit_HCALIN = new TH1F("h_mEHit_HCALIN","h_mEHit_HCALIN",2000,-1.05,198.95);
  hm->registerHisto(h_mEHit_HCALIN);

  TH2F *h_mG4AbsXY_HCALIN = new TH2F("h_mG4AbsXY_HCALIN","h_mG4AbsXY_HCALIN",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4AbsXY_HCALIN);

  TH2F *h_mG4AbsXZ_HCALIN = new TH2F("h_mG4AbsXZ_HCALIN","h_mG4AbsXZ_HCALIN",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4AbsXZ_HCALIN);

  TH1F *h_mEAbs_HCALIN = new TH1F("h_mEAbs_HCALIN","h_mEAbs_HCALIN",2000,-1.05,198.95);
  hm->registerHisto(h_mEAbs_HCALIN);

  // G4Hit for Outer HCal 
  TH2F *h_mG4HitXY_HCALOUT = new TH2F("h_mG4HitXY_HCALOUT","h_mG4HitXY_HCALOUT",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_HCALOUT);

  TH2F *h_mG4HitXZ_HCALOUT = new TH2F("h_mG4HitXZ_HCALOUT","h_mG4HitXZ_HCALOUT",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_HCALOUT);

  TH1F *h_mEHit_HCALOUT = new TH1F("h_mEHit_HCALOUT","h_mEHit_HCALOUT",2000,-1.05,198.95);
  hm->registerHisto(h_mEHit_HCALOUT);

  TH2F *h_mG4AbsXY_HCALOUT = new TH2F("h_mG4AbsXY_HCALOUT","h_mG4AbsXY_HCALOUT",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4AbsXY_HCALOUT);

  TH2F *h_mG4AbsXZ_HCALOUT = new TH2F("h_mG4AbsXZ_HCALOUT","h_mG4AbsXZ_HCALOUT",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4AbsXZ_HCALOUT);

  TH1F *h_mEAbs_HCALOUT = new TH1F("h_mEAbs_HCALOUT","h_mEAbs_HCALOUT",2000,-1.05,198.95);
  hm->registerHisto(h_mEAbs_HCALOUT);

  // G4Hit for Cryo
  TH2F *h_mG4HitXY_CRYO = new TH2F("h_mG4HitXY_CRYO","h_mG4HitXY_CRYO",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_CRYO);

  TH2F *h_mG4HitXZ_CRYO = new TH2F("h_mG4HitXZ_CRYO","h_mG4HitXZ_CRYO",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_CRYO);

  TH1F *h_mEHit_CRYO = new TH1F("h_mEHit_CRYO","h_mEHit_CRYO",2000,-1.05,198.95);
  hm->registerHisto(h_mEHit_CRYO);

  // G4Hit for BlackHole
  TH2F *h_mG4HitXY_BLACKHOLE = new TH2F("h_mG4HitXY_BLACKHOLE","h_mG4HitXY_BLACKHOLE",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_BLACKHOLE);

  TH2F *h_mG4HitXZ_BLACKHOLE = new TH2F("h_mG4HitXZ_BLACKHOLE","h_mG4HitXZ_BLACKHOLE",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_BLACKHOLE);

  TH1F *h_mEHit_BLACKHOLE = new TH1F("h_mEHit_BLACKHOLE","h_mEHit_BLACKHOLE",2000,-1.05,198.95);
  hm->registerHisto(h_mEHit_BLACKHOLE);

  // energy distribution
  TH1F *h_mEnergy_Vis = new TH1F("h_mEnergy_Vis","h_mEnergy_Vis",2000,-1.05,198.95);
  hm->registerHisto(h_mEnergy_Vis); // EMCAL + HCALOUT:  G4HIT & G4ABS

  TH1F *h_mEnergy_Leak = new TH1F("h_mEnergy_Leak","h_mEnergy_Leak",2000,-1.05,198.95);
  hm->registerHisto(h_mEnergy_Leak); // HCALIN + CRYO + BLCAKHOLE: G4HIT & G4ABS

  TH1F *h_mEnergy_total = new TH1F("h_mEnergy_total","h_mEnergy_total",2000,-1.05,198.95);
  hm->registerHisto(h_mEnergy_total); // total energy

  TH1F *h_mBeamEnergy = new TH1F("h_mBeamEnergy","h_mBeamEnergy",2000,-1.05,198.95);
  hm->registerHisto(h_mBeamEnergy); // total energy

  TH2F *h_mG4Energy = new TH2F("h_mG4Energy","h_mG4Energy",500,-0.5,49.5,500,-0.5,49.5);
  hm->registerHisto(h_mG4Energy);

  // EMCAL TOWER LG QA
  TH1F *h_emcalin_lg_tower_calib[64];
  for(int i_tower = 0; i_tower < 64; ++i_tower)
  {
    string HistName = Form("h_emcal_lg_tower_%d_calib",i_tower);
    h_emcalin_lg_tower_calib[i_tower] = new TH1F(HistName.c_str(),HistName.c_str(),1000,-1.05,98.95); // GeV
    hm->registerHisto(h_emcalin_lg_tower_calib[i_tower]);
  }

  // HCALIN TOWER LG QA
  TH1F *h_hcalin_lg_tower_calib[16];
  for(int i_tower = 0; i_tower < 16; ++i_tower)
  {
    string HistName = Form("h_hcalin_lg_tower_%d_calib",i_tower);
    h_hcalin_lg_tower_calib[i_tower] = new TH1F(HistName.c_str(),HistName.c_str(),1000,-1.05,98.95);
    hm->registerHisto(h_hcalin_lg_tower_calib[i_tower]);
  }

  // HCALOUT TOWER LG QA
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

  T->Branch("sim", &_sim);

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4Simulation::getChannelNumber(int row, int column)
{
  if (column < 0 || column >= 4) return -1;
  if (row < 0 || row >= 4) return -1;

  int hbdchanIHC[4][4] = {{13,  9, 5, 1},
                          {14, 10, 6, 2},
                          {15, 11, 7, 3},
                          {16, 12, 8, 4}};

  return hbdchanIHC[row][column] - 1;
}

//---------------------Shower Calibration Analysis----------------------
int Proto4Simulation::InitAna()
{
  string inputfile = Form("/sphenix/user/xusun/Simulation/ShowerCalib/Proto4Simulation_2018c_pion_%dGeV.root",_mMom);
  _mSimulation = new Proto4Simulation::HCAL_Sim();

  mFile_InPut = TFile::Open(inputfile.c_str());
  mTTreeInPut = (TTree*)mFile_InPut->Get("HCAL_Info");
  mTTreeInPut->SetBranchAddress("sim", &_mSimulation);

  long NumOfEvents = (long)mTTreeInPut->GetEntries();
  cout << "total number of events: " << NumOfEvents << endl;
  _mStartEvent = 0;
  _mStopEvent = 100;
  if(_mNumOfEvents < NumOfEvents) _mStopEvent = _mNumOfEvents;
  if(_mNumOfEvents < 0) _mStopEvent = NumOfEvents;

    cout << "New nStartEvent = " << _mStartEvent << ", new nStopEvent = " << _mStopEvent << endl;

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

  h_mAsymmEnergy = new TH2F("h_mAsymmEnergy","h_mAsymmEnergy",105,-1.05,1.05,2000,-1.05,198.95);

  // balancing
  h_mAsymmEnergy_balancing = new TH2F("h_mAsymmEnergy_balancing","h_mAsymmEnergy_balancing",105,-1.05,1.05,2000,-1.05,198.95);

  // leveling correction
  h_mAsymmEnergy_leveling = new TH2F("h_mAsymmEnergy_leveling","h_mAsymmEnergy_leveling",105,-1.05,1.05,2000,-1.05,198.95);

  // shower correction
  h_mAsymmEnergy_showercalib = new TH2F("h_mAsymmEnergy_showercalib","h_mAsymmEnergy_showercalib",105,-1.05,1.05,2000,-1.05,198.95);

  h_mRatio_Truth = new TH1F("h_mRatio_Truth", "h_mRatio_Truth", 1000, -0.5, 99.5);

  return 0;
}

int Proto4Simulation::MakeAna()
{
  cout << "Make()" << endl;

  const float c_in_leveling[12] = {0.814269, 0.87434, 0.930247, 0.870155, 0.841073, 0.812163, 0.805016, 0.804375, 0.796104, 0.785911, 0.792748, 0.793076};
  const float c_out_leveling[12] = {1.2955, 1.16784, 1.08106, 1.17539, 1.23298, 1.30086, 1.31963, 1.32136, 1.3443, 1.3744, 1.35398, 1.35302};

  unsigned long start_event_use = _mStartEvent;
  unsigned long stop_event_use = _mStopEvent;

  mTTreeInPut->SetBranchAddress("sim", &_mSimulation);
  mTTreeInPut->GetEntry(0);

  for(unsigned long i_event = start_event_use; i_event < stop_event_use; ++i_event)
    // for(unsigned long i_event = 20; i_event < 40; ++i_event)
  {
    if (!mTTreeInPut->GetEntry( i_event )) // take the event -> information is stored in event
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

    // find momentum index
    int momIndex = -1;
    std::map<float,int>::iterator it_momId = map_momIndex.find(_mMom);
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

    const float energy_emcal_calib = _mSimulation->emcal_lg_e_calib;
    const float energy_hcalout_calib = _mSimulation->hcalout_lg_e_calib;

    float energy_calib = energy_emcal_calib + energy_hcalout_calib;
    float asymm_calib = (energy_emcal_calib - energy_hcalout_calib)/(energy_emcal_calib + energy_hcalout_calib);

    if(energy_emcal_calib > 0.001 && energy_hcalout_calib > 0.001) // remove ped
    { // extract MIP
      h_mAsymmEnergy->Fill(asymm_calib,energy_calib);
    }

    // 3 sigma MIP energy cut for Balancing Correction
    // const double MIP_energy_cut = MIP_mean+3.0*MIP_width;
    const double MIP_energy_cut = 1.0;
    if(energy_emcal_calib > 0.01 && energy_hcalout_calib > 0.001 && energy_calib > MIP_energy_cut)
    { 
      // balancing without muon
      h_mAsymmEnergy_balancing->Fill(asymm_calib,energy_calib);
    }

    if(energy_emcal_calib > 0.001 && energy_hcalout_calib > 0.001 && energy_calib > MIP_energy_cut)
    {
      // apply leveling
      const float energy_leveling = c_in*energy_emcal_calib + c_out*energy_hcalout_calib;
      const float asymm_leveling = (c_in*energy_emcal_calib - c_out*energy_hcalout_calib)/energy_leveling;
      h_mAsymmEnergy_leveling->Fill(asymm_leveling,energy_leveling);
    }
  }

  cout << "." << flush;
  cout << " " << stop_event_use-start_event_use << "(" << 100 << "%)";
  cout << endl;

  return 0;
}

int Proto4Simulation::FinishAna()
{
  cout << "Finish()" << endl;
  mFile_OutPut = new TFile(_filename.c_str(),"RECREATE");
  mFile_OutPut->cd();

  h_mAsymmEnergy->Write();
  h_mAsymmEnergy_balancing->Write();
  h_mAsymmEnergy_leveling->Write();
  h_mAsymmEnergy_showercalib->Write();
  h_mRatio_Truth->Write();

  mFile_OutPut->Close();


  return 0;
}
