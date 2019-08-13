#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLegend.h"

void plotQA_TowerEnergy_HCal(const int momentum = 12)
{
  int hbdchanIHC[4][4] = {{16, 12, 8, 4},
                          {15, 11, 7, 3},
                          {14, 10, 6, 2},
                          {13,  9, 5, 1}};

  string inputfile = Form("/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/Proto4ShowerCalib_%dGeV_2018c.root",momentum); 
  TFile *File_InPut = TFile::Open(inputfile.c_str());

  TH1F *h_hcal_tower_e_high[16];
  TH1F *h_hcal_tower_e_mid[16];
  TH1F *h_hcal_tower_pi_high[16];
  TH1F *h_hcal_tower_pi_mid[16];
  for(int i_tower = 0; i_tower < 16; ++i_tower)
  {
    string HistName;
    HistName= Form("h_mHCalTower_e_high_%d",i_tower);
    h_hcal_tower_e_high[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
    HistName= Form("h_mHCalTower_e_mid_%d",i_tower);
    h_hcal_tower_e_mid[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
    HistName= Form("h_mHCalTower_pi_high_%d",i_tower);
    h_hcal_tower_pi_high[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
    HistName= Form("h_mHCalTower_pi_mid_%d",i_tower);
    h_hcal_tower_pi_mid[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
  }

  TCanvas *c_hcal_e_high = new TCanvas("c_hcal_e_high","c_hcal_e_high",10,10,1600,1600);
  c_hcal_e_high->Divide(4,4);
  for(int i_row = 0; i_row < 4; ++i_row)
  {
    for(int i_col = 0; i_col < 4; ++i_col)
    {
      int i_pad = 4*i_row+i_col+1;
      int i_tower = hbdchanIHC[i_row][i_col]-1;
      c_hcal_e_high->cd(i_pad);
      c_hcal_e_high->cd(i_pad)->SetLogy();
      h_hcal_tower_e_high[i_tower]->GetXaxis()->SetRangeUser(0,momentum);
      h_hcal_tower_e_high[i_tower]->Draw("hE");
    }
  }
  string FigHCal_e_high = Form("../figures/HCAL_ShowerCalib_2018c/c_hcal_e_high_%dGeV.eps",momentum);
  c_hcal_e_high->SaveAs(FigHCal_e_high.c_str());

  TCanvas *c_hcal_e_mid = new TCanvas("c_hcal_e_mid","c_hcal_e_mid",10,10,1600,1600);
  c_hcal_e_mid->Divide(4,4);
  for(int i_row = 0; i_row < 4; ++i_row)
  {
    for(int i_col = 0; i_col < 4; ++i_col)
    {
      int i_pad = 4*i_row+i_col+1;
      int i_tower = hbdchanIHC[i_row][i_col]-1;
      c_hcal_e_mid->cd(i_pad);
      c_hcal_e_mid->cd(i_pad)->SetLogy();
      h_hcal_tower_e_mid[i_tower]->GetXaxis()->SetRangeUser(0,momentum);
      h_hcal_tower_e_mid[i_tower]->Draw("hE");
    }
  }
  string FigHCal_e_mid = Form("../figures/HCAL_ShowerCalib_2018c/c_hcal_e_mid_%dGeV.eps",momentum);
  c_hcal_e_mid->SaveAs(FigHCal_e_mid.c_str());
}
