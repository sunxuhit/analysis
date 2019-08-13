#include <string>
#include <TString.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TGraphAsymmErrors.h>
#include <TLegend.h>

void plotQA_Resolution_2018c()
{
  // 2018c pion
  string input_2018c_pion = "/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/T1044_2018c_pion.root";
  TFile *File_2018c_pion = TFile::Open(input_2018c_pion.c_str());
  TGraphAsymmErrors *g_resolution_2018c_pion = (TGraphAsymmErrors*)File_2018c_pion->Get("g_resolution_2018c_pion");

  // 2018c pion with outer hcal only
  string input_2018c_pion_ohcal = "/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/T1044_2018c_pion_hcalout.root";
  TFile *File_2018c_pion_ohcal = TFile::Open(input_2018c_pion_ohcal.c_str());
  TGraphAsymmErrors *g_resolution_2018c_pion_hcalout = (TGraphAsymmErrors*)File_2018c_pion_ohcal->Get("g_resolution_2018c_pion");

  // 2018a pion
  string input_2018a_pion = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/T1044_2018_pion.root";
  TFile *File_2018a_pion = TFile::Open(input_2018a_pion.c_str());
  TGraphAsymmErrors *g_resolution_2018a_pion = (TGraphAsymmErrors*)File_2018a_pion->Get("g_resolution_2018_pion");

  // 2017 pion
  string input_2017_pion = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/T1044_2017_pion.root";
  TFile *File_2017_pion = TFile::Open(input_2017_pion.c_str());
  TGraphAsymmErrors *g_resolution_2017_pion = (TGraphAsymmErrors*)File_2017_pion->Get("g_resolution_2017_pion");

  // 2018 pion with outer hcal only
  string input_2018a_ohcal = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/T1044_2018_pion_hcalout.root";
  TFile *File_2018a_ohcal = TFile::Open(input_2018a_ohcal.c_str());
  TGraphAsymmErrors *g_resolution_2018a_pion_hcalout = (TGraphAsymmErrors*)File_2018a_ohcal->Get("g_resolution_2018_pion_hcalout");

  // 2018a electron
  string input_2018a_electron = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/T1044_2018_electron.root";
  TFile *File_2018a_electron = TFile::Open(input_2018a_electron.c_str());
  TGraphAsymmErrors *g_resolution_2018a_electron = (TGraphAsymmErrors*)File_2018a_electron->Get("g_resolution_2018_electron");

  // 2018c electron
  string input_2018c_electron = "/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/T1044_2018c_electron.root";
  TFile *File_2018c_electron = TFile::Open(input_2018c_electron.c_str());
  TGraphAsymmErrors *g_resolution_2018c_electron = (TGraphAsymmErrors*)File_2018c_electron->Get("g_resolution_2018c_electron");

  TCanvas *c_Resolution = new TCanvas("c_Resolution","c_Resolution",10,10,800,800);
  c_Resolution->cd();
  c_Resolution->cd()->SetLeftMargin(0.15);
  c_Resolution->cd()->SetBottomMargin(0.15);
  c_Resolution->cd()->SetTicks(1,1);
  c_Resolution->cd()->SetGrid(0,0);

  TH1F *h_play = new TH1F("h_play","h_play",100,0.0,100.0);
  for(int i_bin = 0; i_bin < 100; ++i_bin)
  {
    h_play->SetBinContent(i_bin+1,-10.0);
    h_play->SetBinError(i_bin+1,1.0);
  }
  h_play->SetTitle("");
  h_play->SetStats(0);
  h_play->GetXaxis()->SetTitle("input Energy (GeV)");
  h_play->GetXaxis()->CenterTitle();
  h_play->GetXaxis()->SetNdivisions(505);
  h_play->GetXaxis()->SetRangeUser(0.0,60.0);

  h_play->GetYaxis()->SetTitle("#DeltaE/<E>");
  h_play->GetYaxis()->CenterTitle();
  h_play->GetYaxis()->SetRangeUser(0.0,0.8);
  h_play->DrawCopy("pE");

  g_resolution_2018c_pion->SetMarkerStyle(20);
  g_resolution_2018c_pion->SetMarkerColor(kGray+2);
  g_resolution_2018c_pion->SetMarkerSize(2.0);
  g_resolution_2018c_pion->Draw("pE same");

  g_resolution_2018c_pion_hcalout->SetMarkerStyle(24);
  g_resolution_2018c_pion_hcalout->SetMarkerColor(kGray+2);
  g_resolution_2018c_pion_hcalout->SetMarkerSize(2.0);
  g_resolution_2018c_pion_hcalout->Draw("pE same");

  /*
  g_resolution_2018a_pion->SetMarkerStyle(24);
  g_resolution_2018a_pion->SetMarkerColor(4);
  g_resolution_2018a_pion->SetMarkerSize(2.0);
  g_resolution_2018a_pion->Draw("pE same");
  */

  g_resolution_2017_pion->SetMarkerStyle(21);
  g_resolution_2017_pion->SetMarkerColor(2);
  g_resolution_2017_pion->SetMarkerSize(2.0);
  g_resolution_2017_pion->Draw("pE same");

  g_resolution_2018a_pion_hcalout->SetMarkerStyle(34);
  g_resolution_2018a_pion_hcalout->SetMarkerColor(6);
  g_resolution_2018a_pion_hcalout->SetMarkerSize(2.0);
  g_resolution_2018a_pion_hcalout->Draw("pE same");

  /*
  g_resolution_2018a_electron->SetMarkerStyle(34);
  g_resolution_2018a_electron->SetMarkerColor(kAzure+1);
  g_resolution_2018a_electron->SetMarkerSize(2.0);
  g_resolution_2018a_electron->Draw("pE same");

  g_resolution_2018c_electron->RemovePoint(11);
  g_resolution_2018c_electron->RemovePoint(10);
  g_resolution_2018c_electron->SetMarkerStyle(20);
  g_resolution_2018c_electron->SetMarkerColor(6);
  g_resolution_2018c_electron->SetMarkerSize(2.0);
  g_resolution_2018c_electron->Draw("pE same");
  */

  TLegend *leg_linear = new TLegend(0.4,0.6,0.85,0.75);
  leg_linear->SetBorderSize(0);
  leg_linear->SetFillColor(0);
  leg_linear->AddEntry(g_resolution_2018c_pion,"#pi- T1044-2018c EMCal+OHCal","p");
  leg_linear->AddEntry(g_resolution_2018c_pion_hcalout,"#pi- T1044-2018c OHCal","p");
  // leg_linear->AddEntry(g_resolution_2018a_pion,"#pi- T1044-2018a IHCal+OHCal","p");
  leg_linear->AddEntry(g_resolution_2017_pion,"#pi- T1044-2017 IHCal+OHCal","p");
  leg_linear->AddEntry(g_resolution_2018a_pion_hcalout,"#pi- T1044-2018a & OHCal","p");
  // leg_linear->AddEntry(g_resolution_2018c_electron,"e- T1044-2018c EMCal+OHCal","p");
  // leg_linear->AddEntry(g_resolution_2018a_electron,"e- T1044-2018a IHCal+OHCal","p");
  leg_linear->Draw("same");

  c_Resolution->SaveAs("./figures/c_Resolution_2018c.eps");
  c_Resolution->SaveAs("./figures/c_Resolution_2018c.png");
}
