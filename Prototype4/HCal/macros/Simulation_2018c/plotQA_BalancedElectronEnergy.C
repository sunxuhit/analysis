#include <string>
#include <TString.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphAsymmErrors.h>
#include <TProfile.h>
#include <TLine.h>

void plotQA_BalancedElectronEnergy()
{
  int mEnergy[12] = {3,4,5,6,8,12,16,20,24,28,40,50};
  float momentum[12];

  TH2F *h_mAsymmEnergy_electron[12];
  TH1F *h_mEnergy_electron[12];
  TFile *File_InPut[12];

  for(int i_energy = 0; i_energy < 12; ++i_energy)
  {
    
    string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/Simulation/ShowerCalibAna/Proto4Simulation_2018c_electron_%dGeV.root",mEnergy[i_energy]);
    File_InPut[i_energy] = TFile::Open(inputfile.c_str());
    h_mAsymmEnergy_electron[i_energy] = (TH2F*)File_InPut[i_energy]->Get("h_mAsymmEnergy_leveling");
    h_mEnergy_electron[i_energy] = (TH1F*)h_mAsymmEnergy_electron[i_energy]->ProjectionY()->Clone("h_mEnergy_electron");

    momentum[i_energy] = (float)mEnergy[i_energy];
  }

  TCanvas *c_AsymmEnergy = new TCanvas("c_AsymmEnergy","c_AsymmEnergy",2000,1500);
  c_AsymmEnergy->Divide(4,3);
  for(int i_pad = 0; i_pad < 12; ++i_pad)
  {
    c_AsymmEnergy->cd(i_pad+1);
    c_AsymmEnergy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_AsymmEnergy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_AsymmEnergy->cd(i_pad+1)->SetTicks(1,1);
    c_AsymmEnergy->cd(i_pad+1)->SetGrid(0,0);
    string inputenergy = Form("Beam Momentum %1.0f GeV",momentum[i_pad]);
    h_mAsymmEnergy_electron[i_pad]->SetTitle(inputenergy.c_str());
    h_mAsymmEnergy_electron[i_pad]->GetXaxis()->SetTitle("Energy Asymmetry");
    h_mAsymmEnergy_electron[i_pad]->GetXaxis()->CenterTitle();
    h_mAsymmEnergy_electron[i_pad]->GetYaxis()->SetTitle("Total Energy (GeV)");
    h_mAsymmEnergy_electron[i_pad]->GetYaxis()->CenterTitle();
    h_mAsymmEnergy_electron[i_pad]->GetYaxis()->SetRangeUser(0.0,2.0*momentum[i_pad]);
    h_mAsymmEnergy_electron[i_pad]->Draw("colz");
  }
  c_AsymmEnergy->SaveAs("./figures/c_EnergyAsymmBalancing_electron_2018c.eps");

  TCanvas *c_Energy = new TCanvas("c_Energy","c_Energy",2000,1500);
  c_Energy->Divide(4,3);
  for(int i_pad = 0; i_pad < 12; ++i_pad)
  {
    c_Energy->cd(i_pad+1);
    c_Energy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_Energy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_Energy->cd(i_pad+1)->SetTicks(1,1);
    c_Energy->cd(i_pad+1)->SetGrid(0,0);
    string inputenergy = Form("Beam Momentum %1.0f GeV",momentum[i_pad]);
    h_mEnergy_electron[i_pad]->SetTitle(inputenergy.c_str());
    h_mEnergy_electron[i_pad]->GetXaxis()->SetTitle("Total Energy (GeV)");
    h_mEnergy_electron[i_pad]->GetXaxis()->SetRangeUser(0.0,2.0*momentum[i_pad]);
    h_mEnergy_electron[i_pad]->GetXaxis()->CenterTitle();
    h_mEnergy_electron[i_pad]->GetYaxis()->SetTitle("");
    h_mEnergy_electron[i_pad]->GetYaxis()->CenterTitle();
    h_mEnergy_electron[i_pad]->Draw();
  }
  c_Energy->SaveAs("./figures/c_EnergyBalancing_electron_2018c.eps");
}
