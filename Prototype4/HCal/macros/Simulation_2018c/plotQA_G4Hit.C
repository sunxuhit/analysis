#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLegend.h"

void plotQA_G4Hit(int beam_mom = 24)
{
  string pid = "pion";
  // string pid = "electron";
  string inputfile = Form("/sphenix/user/xusun/Simulation/ShowerCalib/Proto4Simulation_2018c_%s_%dGeV.root",pid.c_str(),beam_mom);
  TFile *File_InPut = TFile::Open(inputfile.c_str());

  // G4Hit for EMCal => xy & xz & energy
  TH2F *h_mG4HitXY_CEMC = (TH2F*)File_InPut->Get("h_mG4HitXY_CEMC");
  TH2F *h_mG4HitXZ_CEMC = (TH2F*)File_InPut->Get("h_mG4HitXZ_CEMC");

  TH2F *h_mG4AbsXY_CEMC = (TH2F*)File_InPut->Get("h_mG4AbsXY_CEMC");
  TH2F *h_mG4AbsXZ_CEMC = (TH2F*)File_InPut->Get("h_mG4AbsXZ_CEMC");

  // G4Hit for Inner HCal 
  TH2F *h_mG4HitXY_HCALIN = (TH2F*)File_InPut->Get("h_mG4HitXY_HCALIN");
  TH2F *h_mG4HitXZ_HCALIN = (TH2F*)File_InPut->Get("h_mG4HitXZ_HCALIN");

  TH2F *h_mG4AbsXY_HCALIN = (TH2F*)File_InPut->Get("h_mG4AbsXY_HCALIN");
  TH2F *h_mG4AbsXZ_HCALIN = (TH2F*)File_InPut->Get("h_mG4AbsXZ_HCALIN");

  // G4Hit for Outer HCal 
  TH2F *h_mG4HitXY_HCALOUT = (TH2F*)File_InPut->Get("h_mG4HitXY_HCALOUT");
  TH2F *h_mG4HitXZ_HCALOUT = (TH2F*)File_InPut->Get("h_mG4HitXZ_HCALOUT");

  TH2F *h_mG4AbsXY_HCALOUT = (TH2F*)File_InPut->Get("h_mG4AbsXY_HCALOUT");
  TH2F *h_mG4AbsXZ_HCALOUT = (TH2F*)File_InPut->Get("h_mG4AbsXZ_HCALOUT");

  // G4Hit for Cryo
  TH2F *h_mG4HitXY_CRYO = (TH2F*)File_InPut->Get("h_mG4HitXY_CRYO");
  TH2F *h_mG4HitXZ_CRYO = (TH2F*)File_InPut->Get("h_mG4HitXZ_CRYO");

  // G4Hit for BlackHole
  TH2F *h_mG4HitXY_BLACKHOLE = (TH2F*)File_InPut->Get("h_mG4HitXY_BLACKHOLE");
  TH2F *h_mG4HitXZ_BLACKHOLE = (TH2F*)File_InPut->Get("h_mG4HitXZ_BLACKHOLE");

  // energy distribution
  TH1F *h_mEnergy_Vis = (TH1F*)File_InPut->Get("h_mEnergy_Vis");
  TH1F *h_mEnergy_Leak = (TH1F*)File_InPut->Get("h_mEnergy_Leak");
  TH1F *h_mEnergy_total = (TH1F*)File_InPut->Get("h_mEnergy_total");
  TH1F *h_mBeamEnergy = (TH1F*)File_InPut->Get("h_mBeamEnergy");
  TH2F *h_mG4Energy = (TH2F*)File_InPut->Get("h_mG4Energy");

  TCanvas *c_G4Hit_XY = new TCanvas("c_G4Hit_XY","c_G4Hit_XY",10,10,800,800);
  c_G4Hit_XY->cd();
  c_G4Hit_XY->cd()->SetLeftMargin(0.15);
  c_G4Hit_XY->cd()->SetBottomMargin(0.15);
  c_G4Hit_XY->cd()->SetTicks(1,1);
  c_G4Hit_XY->cd()->SetGrid(0,0);

  h_mG4AbsXY_HCALOUT->SetTitle("G4 Hits for Prototype4");
  h_mG4AbsXY_HCALOUT->SetStats(0);
  h_mG4AbsXY_HCALOUT->GetXaxis()->SetTitle("X (cm)");
  h_mG4AbsXY_HCALOUT->GetXaxis()->CenterTitle();
  h_mG4AbsXY_HCALOUT->GetYaxis()->SetTitle("Y (cm)");
  h_mG4AbsXY_HCALOUT->GetYaxis()->CenterTitle();
  h_mG4AbsXY_HCALOUT->Draw("colz");

  h_mG4HitXY_CRYO->Draw("col same");
  h_mG4AbsXY_HCALIN->Draw("col same");
  h_mG4AbsXY_CEMC->Draw("col same");
  h_mG4HitXY_BLACKHOLE->Draw("col same");

  string FigName = Form("../figures/HCAL_Simulation_2018c/c_G4Hit_XY_%s_%dGeV.gif",pid.c_str(),beam_mom);
  c_G4Hit_XY->SaveAs(FigName.c_str());

  TCanvas *c_G4Hit_XZ = new TCanvas("c_G4Hit_XZ","c_G4Hit_XZ",10,10,800,800);
  c_G4Hit_XZ->cd();
  c_G4Hit_XZ->cd()->SetLeftMargin(0.15);
  c_G4Hit_XZ->cd()->SetBottomMargin(0.15);
  c_G4Hit_XZ->cd()->SetTicks(1,1);
  c_G4Hit_XZ->cd()->SetGrid(0,0);

  h_mG4AbsXZ_HCALOUT->SetTitle("G4 Hits for Prototype4");
  h_mG4AbsXZ_HCALOUT->SetStats(0);
  h_mG4AbsXZ_HCALOUT->GetXaxis()->SetTitle("Z (cm)");
  h_mG4AbsXZ_HCALOUT->GetXaxis()->CenterTitle();
  h_mG4AbsXZ_HCALOUT->GetYaxis()->SetTitle("X (cm)");
  h_mG4AbsXZ_HCALOUT->GetYaxis()->CenterTitle();
  h_mG4AbsXZ_HCALOUT->Draw("colz");

  h_mG4HitXZ_CRYO->Draw("col same");
  h_mG4AbsXZ_HCALIN->Draw("col same");
  h_mG4AbsXZ_CEMC->Draw("col same");
  h_mG4HitXZ_BLACKHOLE->Draw("col same");

  FigName = Form("../figures/HCAL_Simulation_2018c/c_G4Hit_XZ_%s_%dGeV.gif",pid.c_str(),beam_mom);
  c_G4Hit_XZ->SaveAs(FigName.c_str());

  TCanvas *c_G4Energy = new TCanvas("c_G4Energy","c_G4Energy",10,10,1000,500);
  c_G4Energy->Divide(2,1);
  for(int i_pad = 0; i_pad < 2; ++i_pad)
  {
    c_G4Energy->cd(i_pad+1);
    c_G4Energy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_G4Energy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_G4Energy->cd(i_pad+1)->SetTicks(1,1);
    c_G4Energy->cd(i_pad+1)->SetGrid(0,0);
  }

  c_G4Energy->cd(1);
  h_mEnergy_total->SetTitle("G4 Hits Energy");
  // h_mEnergy_total->SetStats(0);
  h_mEnergy_total->GetXaxis()->SetTitle("Energy (GeV)");
  h_mEnergy_total->GetXaxis()->CenterTitle();
  h_mEnergy_total->GetXaxis()->SetRangeUser(0,beam_mom*1.1);
  h_mEnergy_total->GetYaxis()->SetRangeUser(0,1.2*h_mEnergy_total->GetMaximum());
  h_mEnergy_total->SetLineColor(1);
  h_mEnergy_total->SetLineStyle(1);
  h_mEnergy_total->SetLineWidth(2);
  h_mEnergy_total->Draw("hE");

  h_mEnergy_Vis->SetStats(0);
  h_mEnergy_Vis->SetLineColor(2);
  h_mEnergy_Vis->SetLineStyle(1);
  h_mEnergy_Vis->SetLineWidth(1);
  h_mEnergy_Vis->Draw("hE same");

  h_mEnergy_Leak->SetStats(0);
  h_mEnergy_Leak->SetLineColor(4);
  h_mEnergy_Leak->SetLineStyle(1);
  h_mEnergy_Leak->SetLineWidth(1);
  h_mEnergy_Leak->Draw("hE same");

  TLegend *leg = new TLegend(0.25,0.6,0.65,0.75);
  leg->SetBorderSize(0);
  leg->SetFillColor(0);
  leg->AddEntry(h_mEnergy_Vis,"visible energy","l");
  leg->AddEntry(h_mEnergy_Leak,"un-visible energy","l");
  leg->AddEntry(h_mEnergy_total,"total energy","l");
  leg->Draw("same ");

  c_G4Energy->cd(2);
  h_mG4Energy->SetTitle("G4 Hits Energy");
  h_mG4Energy->SetStats(0);
  h_mG4Energy->GetXaxis()->SetTitle("truth energy (GeV)");
  h_mG4Energy->GetXaxis()->CenterTitle();
  h_mG4Energy->GetXaxis()->SetRangeUser(beam_mom*0.9,beam_mom*1.1);
  h_mG4Energy->GetYaxis()->SetTitle("reconstructed energy (GeV)");
  h_mG4Energy->GetYaxis()->CenterTitle();
  h_mG4Energy->GetYaxis()->SetRangeUser(beam_mom*0.75,beam_mom*1.1);
  h_mG4Energy->Draw("colz");

  FigName = Form("../figures/HCAL_Simulation_2018c/c_G4Energy_%s_%dGeV.gif",pid.c_str(),beam_mom);
  c_G4Energy->SaveAs(FigName.c_str());
}
