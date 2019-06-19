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

  mHIT_CEMC               = NULL;
  mABS_CEMC               = NULL;
  mHIT_HCALIN             = NULL;
  mABS_HCALIN             = NULL;
  mHIT_HCALOUT            = NULL;
  mABS_HCALOUT            = NULL;
  mHIT_CRYO               = NULL;
  mHIT_BLACKHOLE          = NULL;

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

  Fun4AllHistoManager *hm = get_HistoManager();
  assert(hm);

  TH1D * h_info = new TH1D("h_Normalization","Normalization;Items;Count", 10, .5, 10.5);
  int i = 1;
  h_info->GetXaxis()->SetBinLabel(i++, "Event");
  h_info->GetXaxis()->SetBinLabel(i++, "G4Hit Active");
  h_info->GetXaxis()->SetBinLabel(i++, "G4Hit Absor.");
  h_info->GetXaxis()->LabelsOption("v");
  hm->registerHisto(h_info);

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4Simulation::process_event(PHCompositeNode *topNode)
{
  if (Verbosity() > 2) cout << "Proto4Simulation::process_event() entered" << endl;
  if( getNodes(topNode) != Fun4AllReturnCodes::EVENT_OK) return Fun4AllReturnCodes::DISCARDEVENT;

  Fun4AllHistoManager *hm = get_HistoManager();
  assert(hm);

  TH1D* h_info = dynamic_cast<TH1D*>(hm->getHisto("h_Normalization"));
  assert(h_info);

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

  assert(not _truth_container->GetMap().empty());
  const PHG4Particle * last_primary = _truth_container->GetMap().rbegin()->second;
  assert(last_primary);

  if (Verbosity() > 2)
  {    
    cout
      << "QAG4SimulationCalorimeter::process_event_G4Hit() handle this truth particle"
      << endl;
    last_primary->identify();
  }    
  const PHG4VtxPoint* primary_vtx = //
    _truth_container->GetPrimaryVtx(last_primary->get_vtx_id());
  assert(primary_vtx);
  if (Verbosity() > 2)
  {    
    cout
      << "QAG4SimulationCalorimeter::process_event_G4Hit() handle this vertex"
      << endl;
    primary_vtx->identify();
  }

  /*
  const double t0 = primary_vtx->get_t();
  const TVector3 vertex(primary_vtx->get_x(), primary_vtx->get_y(),
      primary_vtx->get_z());

  // projection axis
  TVector3 axis_proj(last_primary->get_px(), last_primary->get_py(),
      last_primary->get_pz());
  if (axis_proj.Mag() == 0)
    axis_proj.SetXYZ(0, 0, 1);
  axis_proj = axis_proj.Unit();

  // azimuthal direction axis
  TVector3 axis_azimuth = axis_proj.Cross(TVector3(0, 0, 1));
  if (axis_azimuth.Mag() == 0)
    axis_azimuth.SetXYZ(1, 0, 0);
  axis_azimuth = axis_azimuth.Unit();

  // polar direction axis
  TVector3 axis_polar = axis_proj.Cross(axis_azimuth);
  assert(axis_polar.Mag() > 0);
  axis_polar = axis_polar.Unit();
  */

  if (mHIT_CEMC)
  {
    h_info->Fill("G4Hit Active", mHIT_CEMC->size());
  }

  if (mABS_CEMC)
  {
    h_info->Fill("G4Hit Absor.", mABS_CEMC->size());
  }

  // at the end, count success events
  h_info->Fill("Event", 1); 

  return Fun4AllReturnCodes::EVENT_OK;
}

int Proto4Simulation::getNodes(PHCompositeNode *topNode)
{
  // G4Hit for EMCal 
  mHIT_CEMC = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_CEMC");
  if (!mHIT_CEMC)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_CEMC" << endl;
    assert(mHIT_CEMC);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  mABS_CEMC = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_ABSORBER_CEMC");
  if (!mABS_CEMC)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_ABSORBER_CEMC" << endl;
    assert(mABS_CEMC);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // G4Hit for Inner HCal 
  mHIT_HCALIN = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_HCALIN");
  if (!mHIT_HCALIN)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_HCALIN" << endl;
    assert(mHIT_HCALIN);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  mABS_HCALIN = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_ABSORBER_HCALIN");
  if (!mABS_HCALIN)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_ABSORBER_HCALIN" << endl;
    assert(mABS_HCALIN);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // G4Hit for Outer HCal 
  mHIT_HCALOUT = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_HCALOUT");
  if (!mHIT_HCALOUT)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_HCALOUT" << endl;
    assert(mHIT_HCALOUT);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  mABS_HCALOUT = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_ABSORBER_HCALOUT");
  if (!mABS_HCALOUT)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_ABSORBER_HCALOUT" << endl;
    assert(mABS_HCALOUT);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // G4Hit for Cryo
  mHIT_CRYO = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_CRYO");
  if (!mHIT_CRYO)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_CRYO" << endl;
    assert(mHIT_CRYO);
    // return Fun4AllReturnCodes::DISCARDEVENT;
  }

  // G4Hit for BlackHole
  mHIT_BLACKHOLE = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_BlackHole");
  if (!mHIT_BLACKHOLE)
  {
    cout << "Proto4Simulation::getNodes - Fatal Error - "
      << "unable to find DST node " << "G4HIT_BlackHole" << endl;
    assert(mHIT_BLACKHOLE);
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
