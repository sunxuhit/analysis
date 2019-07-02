#include <string>
#include <TString.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TCanvas.h>
#include <TGraphAsymmErrors.h>
#include <TLegend.h>

void plotQA_Ratio_2018c()
{
  // 2018c simulation
  string input_2018c_simulation = "/sphenix/user/xusun/Simulation/ShowerCalibAna/Simulation_2018c_ratio.root";
  TFile *File_2018c_simulation = TFile::Open(input_2018c_simulation.c_str());
  TGraphAsymmErrors *g_ratio_2018c_simulation = (TGraphAsymmErrors*)File_2018c_simulation->Get("g_ratio_2018c");

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
  h_play->GetYaxis()->SetRangeUser(1.0,2.2);
  h_play->DrawCopy("pE");

  g_ratio_2018c_simulation->SetMarkerStyle(24);
  g_ratio_2018c_simulation->SetMarkerColor(4);
  g_ratio_2018c_simulation->SetMarkerSize(2.0);
  g_ratio_2018c_simulation->Draw("pE same");

  TLegend *leg_linear = new TLegend(0.4,0.6,0.85,0.75);
  leg_linear->SetBorderSize(0);
  leg_linear->SetFillColor(0);
  leg_linear->AddEntry(g_ratio_2018c_simulation,"Sim EMCal+OHCal","p");
  leg_linear->Draw("same");

  c_Resolution->SaveAs("./figures/c_Ratio_2018c.eps");
}
