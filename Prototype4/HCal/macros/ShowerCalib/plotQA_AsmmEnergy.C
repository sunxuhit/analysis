#include <string>
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TCanvas.h"

void plotQA_AsmmEnergy()
{
  string inputfile = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/ROOT5/Proto4ShowerInfoRAW_0571.root";
  TFile *File_InPut = TFile::Open(inputfile.c_str());
  // TH2F *h_mAsymmEnergy_balancing = (TH2F*)File_InPut->Get("h_mAsymmEnergy_pion_balancing");
  // TH2F *h_mAsymmEnergy_leveling  = (TH2F*)File_InPut->Get("h_mAsymmEnergy_pion_leveling");
  // TH2F *h_mAsymmEnergy_ShowerCalib = (TH2F*)File_InPut->Get("h_mAsymmEnergy_pion_ShowerCalib");
  TH2F *h_mAsymmEnergy_balancing = (TH2F*)File_InPut->Get("h_mAsymmEnergy_electron_balancing");
  TH2F *h_mAsymmEnergy_leveling  = (TH2F*)File_InPut->Get("h_mAsymmEnergy_electron_leveling");
  TH2F *h_mAsymmEnergy_ShowerCalib = (TH2F*)File_InPut->Get("h_mAsymmEnergy_electron_ShowerCalib");

  TCanvas *c_AsymmEnergy = new TCanvas("c_AsymmEnergy","c_AsymmEnergy",10,10,1500,500);
  c_AsymmEnergy->Divide(3,1);
  for(int i_pad = 0; i_pad < 3; ++i_pad)
  {
    c_AsymmEnergy->cd(i_pad+1);
    c_AsymmEnergy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_AsymmEnergy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_AsymmEnergy->cd(i_pad+1)->SetTicks(1,1);
    c_AsymmEnergy->cd(i_pad+1)->SetGrid(0,0);
  }

  c_AsymmEnergy->cd(1);
  h_mAsymmEnergy_balancing->SetStats(0);
  h_mAsymmEnergy_balancing->SetTitle("Before Balancing");
  h_mAsymmEnergy_balancing->GetYaxis()->SetTitle("Energy (GeV)");
  h_mAsymmEnergy_balancing->GetYaxis()->CenterTitle();
  h_mAsymmEnergy_balancing->GetYaxis()->SetNdivisions(505);
  h_mAsymmEnergy_balancing->GetXaxis()->SetTitle("E_{Asymm}");
  h_mAsymmEnergy_balancing->GetXaxis()->CenterTitle();
  h_mAsymmEnergy_balancing->Draw("colz");

  c_AsymmEnergy->cd(2);
  h_mAsymmEnergy_leveling->SetStats(0);
  h_mAsymmEnergy_leveling->SetTitle("After Balancing");
  h_mAsymmEnergy_leveling->GetYaxis()->SetTitle("Energy (GeV)");
  h_mAsymmEnergy_leveling->GetYaxis()->CenterTitle();
  h_mAsymmEnergy_leveling->GetYaxis()->SetNdivisions(505);
  h_mAsymmEnergy_leveling->GetXaxis()->SetTitle("E_{Asymm}");
  h_mAsymmEnergy_leveling->GetXaxis()->CenterTitle();
  h_mAsymmEnergy_leveling->Draw("colz");

  c_AsymmEnergy->cd(3);
  h_mAsymmEnergy_ShowerCalib->SetStats(0);
  h_mAsymmEnergy_ShowerCalib->SetTitle("After ShowerCalib");
  h_mAsymmEnergy_ShowerCalib->GetYaxis()->SetTitle("Energy (GeV)");
  h_mAsymmEnergy_ShowerCalib->GetYaxis()->CenterTitle();
  h_mAsymmEnergy_ShowerCalib->GetYaxis()->SetNdivisions(505);
  h_mAsymmEnergy_ShowerCalib->GetXaxis()->SetTitle("E_{Asymm}");
  h_mAsymmEnergy_ShowerCalib->GetXaxis()->CenterTitle();
  h_mAsymmEnergy_ShowerCalib->Draw("colz");

  // c_AsymmEnergy->SaveAs("../figures/HCAL_ShowerCalib/c_AsymmEnergy_pion_12GeV_2018a.eps");
  c_AsymmEnergy->SaveAs("../figures/HCAL_ShowerCalib/c_AsymmEnergy_electron_12GeV_2018a.eps");
}
