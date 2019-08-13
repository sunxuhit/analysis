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

void plotQA_Balancing(int mEnergy = 12)
{
  string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/Proto4ShowerCalib_%dGeV_2018c.root",mEnergy);
  TFile *File_InPut = TFile::Open(inputfile.c_str());

  TH2F *h_mAsymmEnergy_pion_before = (TH2F*)File_InPut->Get("h_mAsymmEnergy_pion_balancing");
  TH2F *h_mAsymmEnergy_pion_after = (TH2F*)File_InPut->Get("h_mAsymmEnergy_pion_leveling");
  TH2F *h_mAsymmEnergy_electron_before = (TH2F*)File_InPut->Get("h_mAsymmEnergy_electron_balancing");
  TH2F *h_mAsymmEnergy_electron_after = (TH2F*)File_InPut->Get("h_mAsymmEnergy_electron_leveling");

  TCanvas *c_AsymmEnergy = new TCanvas("c_AsymmEnergy","c_AsymmEnergy",800,800);
  c_AsymmEnergy->Divide(2,2);
  for(int i_pad = 0; i_pad < 4; ++i_pad)
  {
    c_AsymmEnergy->cd(i_pad+1);
    c_AsymmEnergy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_AsymmEnergy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_AsymmEnergy->cd(i_pad+1)->SetTicks(1,1);
    c_AsymmEnergy->cd(i_pad+1)->SetGrid(0,0);
  }

  c_AsymmEnergy->cd(1);
  h_mAsymmEnergy_pion_before->SetTitle("Before Balacing: #pi^{-}");
  h_mAsymmEnergy_pion_before->SetStats(0);
  h_mAsymmEnergy_pion_before->GetXaxis()->SetTitle("Energy Asymmetry");
  h_mAsymmEnergy_pion_before->GetXaxis()->CenterTitle();
  h_mAsymmEnergy_pion_before->GetYaxis()->SetTitle("Total Energy (GeV)");
  h_mAsymmEnergy_pion_before->GetYaxis()->CenterTitle();
  h_mAsymmEnergy_pion_before->GetYaxis()->SetRangeUser(0.0,2.0*mEnergy);
  h_mAsymmEnergy_pion_before->Draw("colz");

  c_AsymmEnergy->cd(2);
  h_mAsymmEnergy_pion_after->SetTitle("After Balacing: #pi^{-}");
  h_mAsymmEnergy_pion_after->SetStats(0);
  h_mAsymmEnergy_pion_after->GetXaxis()->SetTitle("Energy Asymmetry");
  h_mAsymmEnergy_pion_after->GetXaxis()->CenterTitle();
  h_mAsymmEnergy_pion_after->GetYaxis()->SetTitle("Total Energy (GeV)");
  h_mAsymmEnergy_pion_after->GetYaxis()->CenterTitle();
  h_mAsymmEnergy_pion_after->GetYaxis()->SetRangeUser(0.0,2.0*mEnergy);
  h_mAsymmEnergy_pion_after->Draw("colz");

  c_AsymmEnergy->cd(3);
  h_mAsymmEnergy_electron_before->SetTitle("Before Balacing: e^{-}");
  h_mAsymmEnergy_electron_before->SetStats(0);
  h_mAsymmEnergy_electron_before->GetXaxis()->SetTitle("Energy Asymmetry");
  h_mAsymmEnergy_electron_before->GetXaxis()->CenterTitle();
  h_mAsymmEnergy_electron_before->GetYaxis()->SetTitle("Total Energy (GeV)");
  h_mAsymmEnergy_electron_before->GetYaxis()->CenterTitle();
  h_mAsymmEnergy_electron_before->GetYaxis()->SetRangeUser(0.0,2.5*mEnergy);
  h_mAsymmEnergy_electron_before->Draw("colz");

  c_AsymmEnergy->cd(4);
  h_mAsymmEnergy_electron_after->SetTitle("After Balacing: e^{-}");
  h_mAsymmEnergy_electron_after->SetStats(0);
  h_mAsymmEnergy_electron_after->GetXaxis()->SetTitle("Energy Asymmetry");
  h_mAsymmEnergy_electron_after->GetXaxis()->CenterTitle();
  h_mAsymmEnergy_electron_after->GetYaxis()->SetTitle("Total Energy (GeV)");
  h_mAsymmEnergy_electron_after->GetYaxis()->CenterTitle();
  h_mAsymmEnergy_electron_after->GetYaxis()->SetRangeUser(0.0,2.5*mEnergy);
  h_mAsymmEnergy_electron_after->Draw("colz");

  c_AsymmEnergy->SaveAs("./figures/c_EnergyBalancing_12GeV.eps");
}
