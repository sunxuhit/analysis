#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLegend.h"

void plotQA_TowerEnergy(const int runId = 2658)
{
  int hbdchanIHC[4][4] = {{16, 12, 8, 4},
                          {15, 11, 7, 3},
                          {14, 10, 6, 2},
                          {13,  9, 5, 1}};

  string inputfile = Form("/sphenix/user/xusun/TestBeam/ShowerCalib_2018c/Proto4ShowerInfoRaw_%d_2018c.root",runId); 
  TFile *File_InPut = TFile::Open(inputfile.c_str());

  TH1F *h_hcalout_tower_calib[16];
  for(int i_tower = 0; i_tower < 16; ++i_tower)
  {
    string HistName= Form("h_hcalout_lg_tower_%d_calib",i_tower);
    h_hcalout_tower_calib[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
  }

  TH1F *h_emcal_tower_calib[64];
  for(int i_tower = 0; i_tower < 64; ++i_tower)
  {
    string HistName= Form("h_emcal_lg_tower_%d_calib",i_tower);
    h_emcal_tower_calib[i_tower] = (TH1F*)File_InPut->Get(HistName.c_str())->Clone();
  }

  TCanvas *c_emcal = new TCanvas("c_emcal","c_emcal",10,10,1600,1600);
  c_emcal->Divide(8,8);
  for(int i_row = 0; i_row < 8; ++i_row)
  {
    for(int i_col = 0; i_col < 8; ++i_col)
    {
      int i_tower = i_row + 8*i_col;
      c_emcal->cd(i_tower+1);
      c_emcal->cd(i_tower+1)->SetLogy();
      h_emcal_tower_calib[i_tower]->Draw("hE");
    }
  }
  string FigEMCal = Form("../figures/HCAL_ShowerCalib_2018c/c_emcal_%d.eps",runId);
  c_emcal->SaveAs(FigEMCal.c_str());

  TCanvas *c_hcal_out = new TCanvas("c_hcal_out","c_hcal_out",10,10,1600,1600);
  c_hcal_out->Divide(4,4);
  for(int i_row = 0; i_row < 4; ++i_row)
  {
    for(int i_col = 0; i_col < 4; ++i_col)
    {
      int i_pad = 4*i_row+i_col+1;
      int i_tower = hbdchanIHC[i_row][i_col]-1;
      c_hcal_out->cd(i_pad);
      c_hcal_out->cd(i_pad)->SetLogy();
      h_hcalout_tower_calib[i_tower]->Draw("hE");
    }
  }
  string FigOHCal = Form("../figures/HCAL_ShowerCalib_2018c/c_hcal_out_%d.eps",runId);
  c_hcal_out->SaveAs(FigOHCal.c_str());
}
