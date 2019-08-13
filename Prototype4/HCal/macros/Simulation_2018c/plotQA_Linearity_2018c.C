#include <string>
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"

void plotQA_Linearity_2018c()
{
  // 2018c pion
  string input_2018c_pion = "/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/T1044_2018c_pion.root";
  TFile *File_2018c_pion = TFile::Open(input_2018c_pion.c_str());
  TGraphAsymmErrors *g_linearity_2018c_pion = (TGraphAsymmErrors*)File_2018c_pion->Get("g_linearity_2018c_pion");

  // 2018c pion simulation 
  string input_2018c_pion_simulation = "/sphenix/user/xusun/Simulation/ShowerCalibAna/Simulation_2018c_pion.root";
  TFile *File_2018c_pion_simulation = TFile::Open(input_2018c_pion_simulation.c_str());
  TGraphAsymmErrors *g_linearity_2018c_pion_simulation = (TGraphAsymmErrors*)File_2018c_pion_simulation->Get("g_linearity_2018c_pion");

  // 2018a pion
  string input_2018a_pion = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/T1044_2018_pion.root";
  TFile *File_2018a_pion = TFile::Open(input_2018a_pion.c_str());
  TGraphAsymmErrors *g_linearity_2018a_pion = (TGraphAsymmErrors*)File_2018a_pion->Get("g_linearity_2018_pion");

  /*
  // 2018c electron simulation 
  string input_2018c_electron_simulation = "/sphenix/user/xusun/Simulation/ShowerCalibAna/Simulation_2018c_electron.root";
  TFile *File_2018c_electron_simulation = TFile::Open(input_2018c_electron_simulation.c_str());
  TGraphAsymmErrors *g_linearity_2018c_electron_simulation = (TGraphAsymmErrors*)File_2018c_electron_simulation->Get("g_linearity_2018c_electron");

  // 2018a electron
  string input_2018a_electron = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/T1044_2018_electron.root";
  TFile *File_2018a_electron = TFile::Open(input_2018a_electron.c_str());
  TGraphAsymmErrors *g_linearity_2018a_electron = (TGraphAsymmErrors*)File_2018a_electron->Get("g_linearity_2018_electron");

  // 2018c electron
  string input_2018c_electron = "/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/T1044_2018c_electron.root";
  TFile *File_2018c_electron = TFile::Open(input_2018c_electron.c_str());
  TGraphAsymmErrors *g_linearity_2018c_electron = (TGraphAsymmErrors*)File_2018c_electron->Get("g_linearity_2018c_electron");
  */

  // 2017 pion
  string input_2017_pion = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/T1044_2017_pion.root";
  TFile *File_2017_pion = TFile::Open(input_2017_pion.c_str());
  TGraphAsymmErrors *g_linearity_2017_pion = (TGraphAsymmErrors*)File_2017_pion->Get("g_linearity_2017_pion");

  TCanvas *c_Linearity = new TCanvas("c_Linearity","c_Linearity",10,10,800,800);
  c_Linearity->cd();
  c_Linearity->cd()->SetLeftMargin(0.15);
  c_Linearity->cd()->SetBottomMargin(0.15);
  c_Linearity->cd()->SetTicks(1,1);
  c_Linearity->cd()->SetGrid(0,0);

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

  h_play->GetYaxis()->SetTitle("Tower Calibrated Energy (GeV)");
  h_play->GetYaxis()->CenterTitle();
  h_play->GetYaxis()->SetRangeUser(0.0,60.0);
  h_play->DrawCopy("pE");

  TLine *l_unity = new TLine(1.0,1.0,59.0,59.0);
  l_unity->SetLineColor(4);
  l_unity->SetLineStyle(2);
  l_unity->SetLineWidth(2);
  l_unity->Draw("l same");

  g_linearity_2018a_pion->SetMarkerStyle(34);
  g_linearity_2018a_pion->SetMarkerColor(kAzure+1);
  g_linearity_2018a_pion->SetMarkerSize(1.4);
  g_linearity_2018a_pion->Draw("pE same");

  /*
  TF1 *f_pol = new TF1("f_pol","pol1",0.0,59.0);
  f_pol->SetRange(3.5,59.0);
  g_linearity_2018c_pion->Fit(f_pol,"NR");
  f_pol->SetLineColor(2);
  f_pol->SetLineWidth(4);
  f_pol->SetLineStyle(2);
  f_pol->Draw("l same");
  float chi2 = f_pol->GetChisquare();
  float ndf = f_pol->GetNDF();
  string chi2_ndf = Form("#chi^{2}/NDF = %1.1f/%1.1f",chi2,ndf);
  */

  g_linearity_2017_pion->SetMarkerStyle(21);
  g_linearity_2017_pion->SetMarkerColor(2);
  g_linearity_2017_pion->SetMarkerSize(1.4);
  g_linearity_2017_pion->Draw("pE same");

  g_linearity_2018c_pion->SetMarkerStyle(20);
  g_linearity_2018c_pion->SetMarkerColor(kGray+2);
  g_linearity_2018c_pion->SetMarkerSize(2.0);
  g_linearity_2018c_pion->Draw("pE same");

  g_linearity_2018c_pion_simulation->SetMarkerStyle(24);
  g_linearity_2018c_pion_simulation->SetMarkerColor(4);
  g_linearity_2018c_pion_simulation->SetMarkerSize(2.0);
  g_linearity_2018c_pion_simulation->Draw("pE same");

  /*
  g_linearity_2018c_electron_simulation->SetMarkerStyle(28);
  g_linearity_2018c_electron_simulation->SetMarkerColor(4);
  g_linearity_2018c_electron_simulation->SetMarkerSize(2.0);
  g_linearity_2018c_electron_simulation->Draw("pE same");

  g_linearity_2018a_electron->SetMarkerStyle(34);
  g_linearity_2018a_electron->SetMarkerColor(kAzure+1);
  g_linearity_2018a_electron->SetMarkerSize(2.0);
  g_linearity_2018a_electron->Draw("pE same");

  g_linearity_2018c_electron->RemovePoint(11);
  g_linearity_2018c_electron->RemovePoint(10);
  g_linearity_2018c_electron->SetMarkerStyle(20);
  g_linearity_2018c_electron->SetMarkerColor(6);
  g_linearity_2018c_electron->SetMarkerSize(2.0);
  g_linearity_2018c_electron->Draw("pE same");
  */

  TLegend *leg_linear = new TLegend(0.5,0.2,0.8,0.4);
  leg_linear->SetBorderSize(0);
  leg_linear->SetFillColor(0);
  leg_linear->AddEntry(g_linearity_2018c_pion,"#pi- T1044-2018c EMCal+OHCal","p");
  leg_linear->AddEntry(g_linearity_2018c_pion_simulation,"#pi- Sim-2018c EMCal+OHCal","p");
  leg_linear->AddEntry(g_linearity_2018a_pion,"#pi- T1044-2018a IHCal+OHCal","p");
  leg_linear->AddEntry(g_linearity_2017_pion,"#pi- T1044-2017 IHCal+OHCal","p");
  // leg_linear->AddEntry(g_linearity_2018c_electron,"e- T1044-2018c EMCal+OHCal","p");
  // leg_linear->AddEntry(g_linearity_2018a_electron,"e- T1044-2018a IHCal+OHCal","p");
  // leg_linear->AddEntry(g_linearity_2018c_electron_simulation,"e- Sim EMCal+OHCal","p");
  leg_linear->AddEntry(l_unity,"unity","l");
  leg_linear->Draw("same");

  c_Linearity->SaveAs("./figures/c_Linearity_2018c.eps");
  c_Linearity->SaveAs("./figures/c_Linearity_2018c.png");
}
