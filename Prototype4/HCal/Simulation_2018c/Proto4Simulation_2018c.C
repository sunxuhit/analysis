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


  TH1F *h_info = dynamic_cast<TH1F*>(hm->getHisto("h_Normalization"));
  assert(h_info);

  double eHIT_CEMC = 0.0; // active energy deposition
  double eABS_CEMC = 0.0; 
  double eHIT_HCALIN = 0.0;
  double eABS_HCALIN = 0.0;
  double eHIT_HCALOUT = 0.0;
  double eABS_HCALOUT = 0.0;
  double eHIT_CRYO = 0.0;
  double eHIT_BLACKHOLE = 0.0;

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

  TH1F *h_mEHit_CEMC = new TH1F("h_mEHit_CEMC","h_mEHit_CEMC",200,-1.05,198.95);
  hm->registerHisto(h_mEHit_CEMC);

  TH2F *h_mG4AbsXY_CEMC = new TH2F("h_mG4AbsXY_CEMC","h_mG4AbsXY_CEMC",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4AbsXY_CEMC);

  TH2F *h_mG4AbsXZ_CEMC = new TH2F("h_mG4AbsXZ_CEMC","h_mG4AbsXZ_CEMC",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4AbsXZ_CEMC);

  TH1F *h_mEAbs_CEMC = new TH1F("h_mEAbs_CEMC","h_mEAbs_CEMC",200,-1.05,198.95);
  hm->registerHisto(h_mEAbs_CEMC);

  // G4Hit for Inner HCal 
  TH2F *h_mG4HitXY_HCALIN = new TH2F("h_mG4HitXY_HCALIN","h_mG4HitXY_HCALIN",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_HCALIN);

  TH2F *h_mG4HitXZ_HCALIN = new TH2F("h_mG4HitXZ_HCALIN","h_mG4HitXZ_HCALIN",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_HCALIN);

  TH1F *h_mEHit_HCALIN = new TH1F("h_mEHit_HCALIN","h_mEHit_HCALIN",200,-1.05,198.95);
  hm->registerHisto(h_mEHit_HCALIN);

  TH2F *h_mG4AbsXY_HCALIN = new TH2F("h_mG4AbsXY_HCALIN","h_mG4AbsXY_HCALIN",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4AbsXY_HCALIN);

  TH2F *h_mG4AbsXZ_HCALIN = new TH2F("h_mG4AbsXZ_HCALIN","h_mG4AbsXZ_HCALIN",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4AbsXZ_HCALIN);

  TH1F *h_mEAbs_HCALIN = new TH1F("h_mEAbs_HCALIN","h_mEAbs_HCALIN",200,-1.05,198.95);
  hm->registerHisto(h_mEAbs_HCALIN);

  // G4Hit for Outer HCal 
  TH2F *h_mG4HitXY_HCALOUT = new TH2F("h_mG4HitXY_HCALOUT","h_mG4HitXY_HCALOUT",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_HCALOUT);

  TH2F *h_mG4HitXZ_HCALOUT = new TH2F("h_mG4HitXZ_HCALOUT","h_mG4HitXZ_HCALOUT",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_HCALOUT);

  TH1F *h_mEHit_HCALOUT = new TH1F("h_mEHit_HCALOUT","h_mEHit_HCALOUT",200,-1.05,198.95);
  hm->registerHisto(h_mEHit_HCALOUT);

  TH2F *h_mG4AbsXY_HCALOUT = new TH2F("h_mG4AbsXY_HCALOUT","h_mG4AbsXY_HCALOUT",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4AbsXY_HCALOUT);

  TH2F *h_mG4AbsXZ_HCALOUT = new TH2F("h_mG4AbsXZ_HCALOUT","h_mG4AbsXZ_HCALOUT",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4AbsXZ_HCALOUT);

  TH1F *h_mEAbs_HCALOUT = new TH1F("h_mEAbs_HCALOUT","h_mEAbs_HCALOUT",200,-1.05,198.95);
  hm->registerHisto(h_mEAbs_HCALOUT);

  // G4Hit for Cryo
  TH2F *h_mG4HitXY_CRYO = new TH2F("h_mG4HitXY_CRYO","h_mG4HitXY_CRYO",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_CRYO);

  TH2F *h_mG4HitXZ_CRYO = new TH2F("h_mG4HitXZ_CRYO","h_mG4HitXZ_CRYO",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_CRYO);

  TH1F *h_mEHit_CRYO = new TH1F("h_mEHit_CRYO","h_mEHit_CRYO",200,-1.05,198.95);
  hm->registerHisto(h_mEHit_CRYO);

  // G4Hit for BlackHole
  TH2F *h_mG4HitXY_BLACKHOLE = new TH2F("h_mG4HitXY_BLACKHOLE","h_mG4HitXY_BLACKHOLE",2000,-50.0,350.0,750,-75.0,75.0);
  hm->registerHisto(h_mG4HitXY_BLACKHOLE);

  TH2F *h_mG4HitXZ_BLACKHOLE = new TH2F("h_mG4HitXZ_BLACKHOLE","h_mG4HitXZ_BLACKHOLE",2000,-50.0,350.0,2000,-50.0,350.0);
  hm->registerHisto(h_mG4HitXZ_BLACKHOLE);

  TH1F *h_mEHit_BLACKHOLE = new TH1F("h_mEHit_BLACKHOLE","h_mEHit_BLACKHOLE",200,-1.05,198.95);
  hm->registerHisto(h_mEHit_BLACKHOLE);

  // energy distribution
  TH1F *h_mEnergy_Vis = new TH1F("h_mEnergy_Vis","h_mEnergy_Vis",200,-1.05,198.95);
  hm->registerHisto(h_mEnergy_Vis); // EMCAL + HCALOUT:  G4HIT & G4ABS

  TH1F *h_mEnergy_Leak = new TH1F("h_mEnergy_Leak","h_mEnergy_Leak",200,-1.05,198.95);
  hm->registerHisto(h_mEnergy_Leak); // HCALIN + CRYO + BLCAKHOLE: G4HIT & G4ABS

  TH1F *h_mEnergy_total = new TH1F("h_mEnergy_total","h_mEnergy_total",200,-1.05,198.95);
  hm->registerHisto(h_mEnergy_total); // total energy

  TH1F *h_mBeamEnergy = new TH1F("h_mBeamEnergy","h_mBeamEnergy",200,-1.05,198.95);
  hm->registerHisto(h_mBeamEnergy); // total energy

  return Fun4AllReturnCodes::EVENT_OK;
}
