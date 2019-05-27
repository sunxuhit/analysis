#include <string>
#include <TString.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphAsymmErrors.h>
#include <TMath.h>
#include <TLegend.h>
#include <iostream>
#include <fstream>

float ErrorAdd(float x, float y)
{
    return sqrt(x*x+y*y);
}

float ErrTimes(float x, float y, float dx, float dy)
{
    return x*y*ErrorAdd(dx/x,dy/y);
}

float ErrDiv(float x, float y, float dx, float dy)
{
    return x/y*ErrorAdd(dx/x,dy/y);
}

void extractMIPEnergy_2018c()
{
  int runId[12] = {2598, 2588, 2604, 2580, 2610, 2624, 2636, 2645, 2658, 2666, 2672, 2674};

  TH2F *h_mAsymmEnergy[12];
  TH1F *h_mMIPEnergy[12];
  TH1F *hBeam_Mom[12];
  TFile *File_InPut[12];
  float energy[12];

  for(int i_energy = 0; i_energy < 12; ++i_energy)
  {
    
    string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/TestBeam/ShowerCalib_2018c/Proto4ShowerInfoRaw_%d_2018c.root",runId[i_energy]);
    File_InPut[i_energy] = TFile::Open(inputfile.c_str());
    h_mAsymmEnergy[i_energy] = (TH2F*)File_InPut[i_energy]->Get("h_tower_energy_calib");
    h_mMIPEnergy[i_energy] = (TH1F*)h_mAsymmEnergy[i_energy]->ProjectionY()->Clone();
    hBeam_Mom[i_energy] = (TH1F*)File_InPut[i_energy]->Get("hBeam_Mom");
    energy[i_energy] = TMath::Abs(hBeam_Mom[i_energy]->GetMean());
  }

  TF1 *f_gaus_MIP[12];
  TLegend *leg_MIP[12];
  float MIP_mean[12];
  float MIP_width[12];
  TCanvas *c_MIPEnergy = new TCanvas("c_MIPEnergy","c_MIPEnergy",1500,2000);
  c_MIPEnergy->Divide(4,3);
  for(int i_pad = 0; i_pad < 12; ++i_pad)
  {
    string inputenergy = Form("Beam Energy %1.0f GeV",energy[i_pad]);
    c_MIPEnergy->cd(i_pad+1);
    c_MIPEnergy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_MIPEnergy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_MIPEnergy->cd(i_pad+1)->SetTicks(1,1);
    c_MIPEnergy->cd(i_pad+1)->SetGrid(0,0);
    h_mMIPEnergy[i_pad]->SetTitle(inputenergy.c_str());
    h_mMIPEnergy[i_pad]->GetXaxis()->SetTitle("Tower Calibrated Energy (GeV)");
    h_mMIPEnergy[i_pad]->GetXaxis()->SetRangeUser(-1.05,50.05);
    h_mMIPEnergy[i_pad]->Draw("hE");
    string FuncName = Form("f_gaus_MIP_%d",i_pad);
    f_gaus_MIP[i_pad] = new TF1(FuncName.c_str(),"gaus",0,5);
    f_gaus_MIP[i_pad]->SetParameter(0,1.0);
    f_gaus_MIP[i_pad]->SetParameter(1,1.0);
    f_gaus_MIP[i_pad]->SetParameter(2,0.1);
    f_gaus_MIP[i_pad]->SetRange(0.4,0.9);
    h_mMIPEnergy[i_pad]->Fit(f_gaus_MIP[i_pad],"NQR");
    f_gaus_MIP[i_pad]->SetLineColor(2);
    f_gaus_MIP[i_pad]->SetLineStyle(2);
    f_gaus_MIP[i_pad]->SetLineWidth(2);
    f_gaus_MIP[i_pad]->Draw("l same");
    MIP_mean[i_pad] = f_gaus_MIP[i_pad]->GetParameter(1);
    MIP_width[i_pad] = f_gaus_MIP[i_pad]->GetParameter(2);
    cout << "MIP for " << inputenergy << " is: " << MIP_mean[i_pad] << " +/- " << MIP_width[i_pad] << endl;
    string leg_energy = Form("MIP = %1.2f #pm %1.2f",MIP_mean[i_pad],MIP_width[i_pad]);
    leg_MIP[i_pad] = new TLegend(0.4,0.6,0.9,0.8);
    leg_MIP[i_pad]->SetBorderSize(0);
    leg_MIP[i_pad]->SetFillColor(0);
    leg_MIP[i_pad]->AddEntry(f_gaus_MIP[i_pad],leg_energy.c_str(),"l");
    leg_MIP[i_pad]->Draw("same");
  }

  int i_MIP = 5; // use 12 GeV as defualt MIP cut
  cout << "default MIP for all energy chosen to be " << energy[i_MIP] << " GeV is: " << MIP_mean[i_MIP] << " +/- " << MIP_width[i_MIP] << endl;

  ofstream File_OutPut("MIPEnergy_2018c.txt");
  File_OutPut << "default MIP at " << energy[i_MIP] << " GeV is: " << MIP_mean[i_MIP] << " +/- " << MIP_width[i_MIP] << endl;
  File_OutPut.close();

  c_MIPEnergy->SaveAs("../figures/HCAL_ShowerCalib_2018c/c_MIPEnergy_2018c.eps");
}
