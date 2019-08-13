#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLegend.h"

void plotQA_TowerEnergy_EMCal(const int momentum = 12)
{
  string inputfile = Form("/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/Proto4ShowerCalib_%dGeV_2018c.root",momentum); 
  TFile *File_InPut = TFile::Open(inputfile.c_str());

  TH1F *h_emcal_tower_e_high[64];
  TH1F *h_emcal_tower_e_mid[64];
  TH1F *h_emcal_tower_pi_high[64];
  TH1F *h_emcal_tower_pi_mid[64];
  for(int i_tower = 0; i_tower < 64; ++i_tower)
  {
    string HistName;
    HistName= Form("h_mEMCalTower_e_high_%d",i_tower);
    h_emcal_tower_e_high[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
    HistName= Form("h_mEMCalTower_e_mid_%d",i_tower);
    h_emcal_tower_e_mid[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
    HistName= Form("h_mEMCalTower_pi_high_%d",i_tower);
    h_emcal_tower_pi_high[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
    HistName= Form("h_mEMCalTower_pi_mid_%d",i_tower);
    h_emcal_tower_pi_mid[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
  }

  TCanvas *c_emcal_e_high = new TCanvas("c_emcal_e_high","c_emcal_e_high",10,10,1600,1600);
  c_emcal_e_high->Divide(8,8);
  for(int i_row = 0; i_row < 8; ++i_row)
  {
    for(int i_col = 0; i_col < 8; ++i_col)
    {
      int i_tower = i_row + 8*i_col;
      c_emcal_e_high->cd(i_tower+1);
      c_emcal_e_high->cd(i_tower+1)->SetLogy();
      h_emcal_tower_pi_high[i_tower]->GetXaxis()->SetRangeUser(0,momentum);
      h_emcal_tower_pi_high[i_tower]->Draw("hE");
    }
  }
  string FigEMCal_e_high = Form("../figures/HCAL_ShowerCalib_2018c/c_emcal_e_high_%dGeV.eps",momentum);
  c_emcal_e_high->SaveAs(FigEMCal_e_high.c_str());

  TCanvas *c_emcal_e_mid = new TCanvas("c_emcal_e_mid","c_emcal_e_mid",10,10,1600,1600);
  c_emcal_e_mid->Divide(8,8);
  for(int i_row = 0; i_row < 8; ++i_row)
  {
    for(int i_col = 0; i_col < 8; ++i_col)
    {
      int i_tower = i_row + 8*i_col;
      c_emcal_e_mid->cd(i_tower+1);
      c_emcal_e_mid->cd(i_tower+1)->SetLogy();
      h_emcal_tower_pi_mid[i_tower]->GetXaxis()->SetRangeUser(0,momentum);
      h_emcal_tower_pi_mid[i_tower]->Draw("hE");
    }
  }
  string FigEMCal_e_mid = Form("../figures/HCAL_ShowerCalib_2018c/c_emcal_e_mid_%dGeV.eps",momentum);
  c_emcal_e_mid->SaveAs(FigEMCal_e_mid.c_str());

  /*
  TCanvas *c_emcal_pi_high = new TCanvas("c_emcal_pi_high","c_emcal_pi_high",10,10,1600,1600);
  c_emcal_pi_high->Divide(8,8);
  for(int i_row = 0; i_row < 8; ++i_row)
  {
    for(int i_col = 0; i_col < 8; ++i_col)
    {
      int i_tower = i_row + 8*i_col;
      c_emcal_pi_high->cd(i_tower+1);
      c_emcal_pi_high->cd(i_tower+1)->SetLogy();
      h_emcal_tower_e_high[i_tower]->GetXaxis()->SetRangeUser(0,momentum);
      h_emcal_tower_e_high[i_tower]->Draw("hE");
    }
  }
  string FigEMCal_pi_high = Form("../figures/HCAL_ShowerCalib_2018c/c_emcal_pi_high_%dGeV.eps",momentum);
  c_emcal_pi_high->SaveAs(FigEMCal_pi_high.c_str());

  TCanvas *c_emcal_pi_mid = new TCanvas("c_emcal_pi_mid","c_emcal_pi_mid",10,10,1600,1600);
  c_emcal_pi_mid->Divide(8,8);
  for(int i_row = 0; i_row < 8; ++i_row)
  {
    for(int i_col = 0; i_col < 8; ++i_col)
    {
      int i_tower = i_row + 8*i_col;
      c_emcal_pi_mid->cd(i_tower+1);
      c_emcal_pi_mid->cd(i_tower+1)->SetLogy();
      h_emcal_tower_e_mid[i_tower]->GetXaxis()->SetRangeUser(0,momentum);
      h_emcal_tower_e_mid[i_tower]->Draw("hE");
    }
  }
  string FigEMCal_pi_mid = Form("../figures/HCAL_ShowerCalib_2018c/c_emcal_pi_mid_%dGeV.eps",momentum);
  c_emcal_pi_mid->SaveAs(FigEMCal_pi_mid.c_str());
  */
}
