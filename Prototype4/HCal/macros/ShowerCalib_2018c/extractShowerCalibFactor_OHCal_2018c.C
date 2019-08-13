#include <string>
#include <TString.h>
#include <TFile.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphAsymmErrors.h>
#include <iostream>
#include <fstream>

void extractShowerCalibFactor_OHCal_2018c()
{
  int mEnergy[12] = {3,4,5,6,8,12,16,20,24,28,40,50};
  float momentum[12];

  TH1F *h_mMomentum[12];
  TH1F *h_mEnergyOut_pion[12];
  TFile *File_InPut[12];

  TF1 *f_gaus[12];
  TLegend *leg[12];
  float showercalib[12];

  for(int i_energy = 0; i_energy < 12; ++i_energy)
  {
    
    string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/Proto4ShowerCalib_%dGeV_2018c.root",mEnergy[i_energy]);
    File_InPut[i_energy] = TFile::Open(inputfile.c_str());
    h_mMomentum[i_energy] = (TH1F*)File_InPut[i_energy]->Get("h_mMomentum");
    momentum[i_energy] = TMath::Abs(h_mMomentum[i_energy]->GetMean());

    h_mEnergyOut_pion[i_energy] = (TH1F*)File_InPut[i_energy]->Get("h_mEnergyOut_pion");
  }

  TCanvas *c_Energy = new TCanvas("c_Energy","c_Energy",2000,1500);
  c_Energy->Divide(4,3);
  for(int i_pad = 0; i_pad < 12; ++i_pad)
  {
    c_Energy->cd(i_pad+1);
    c_Energy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_Energy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_Energy->cd(i_pad+1)->SetTicks(1,1);
    c_Energy->cd(i_pad+1)->SetGrid(0,0);

    string inputenergy = Form("Beam Momentum %1.0f GeV",momentum[i_pad]);
    h_mEnergyOut_pion[i_pad]->SetTitle(inputenergy.c_str());
    h_mEnergyOut_pion[i_pad]->SetStats(0);
    h_mEnergyOut_pion[i_pad]->GetXaxis()->SetTitle("Total Energy (GeV)");
    h_mEnergyOut_pion[i_pad]->GetXaxis()->CenterTitle();
    h_mEnergyOut_pion[i_pad]->GetXaxis()->SetRangeUser(0.0,momentum[i_pad]);
    h_mEnergyOut_pion[i_pad]->GetYaxis()->SetTitle();
    h_mEnergyOut_pion[i_pad]->GetYaxis()->CenterTitle();
    h_mEnergyOut_pion[i_pad]->GetYaxis()->SetRangeUser(0.0,1.7*h_mEnergyOut_pion[i_pad]->GetMaximum());
    h_mEnergyOut_pion[i_pad]->SetLineColor(1);
    h_mEnergyOut_pion[i_pad]->SetLineWidth(1);
    h_mEnergyOut_pion[i_pad]->SetLineStyle(1);
    h_mEnergyOut_pion[i_pad]->Draw("h");

    string FuncName = Form("f_gaus_%d",i_pad);
    f_gaus[i_pad] = new TF1(FuncName.c_str(),"gaus",0,50);
    f_gaus[i_pad]->SetParameter(0,1.0);
    f_gaus[i_pad]->SetParameter(1,h_mEnergyOut_pion[i_pad]->GetMean());
    f_gaus[i_pad]->SetParameter(2,1.0);
    f_gaus[i_pad]->SetRange(0,momentum[i_pad]);
    h_mEnergyOut_pion[i_pad]->Fit(f_gaus[i_pad],"NQR");

    float norm  = f_gaus[i_pad]->GetParameter(0);
    float mean  = f_gaus[i_pad]->GetParameter(1); 
    float sigma = f_gaus[i_pad]->GetParameter(2);
    f_gaus[i_pad]->SetParameter(0,norm);
    f_gaus[i_pad]->SetParameter(1,mean);
    f_gaus[i_pad]->SetParameter(2,sigma);
    if(momentum[i_pad]< 10.0) f_gaus[i_pad]->SetRange(mean-1.0*sigma,mean+2.0*sigma);
    else f_gaus[i_pad]->SetRange(mean-2.0*sigma,mean+2.0*sigma);
    h_mEnergyOut_pion[i_pad]->Fit(f_gaus[i_pad],"NQR");

    f_gaus[i_pad]->SetLineColor(4);
    f_gaus[i_pad]->SetLineStyle(2);
    f_gaus[i_pad]->SetLineWidth(3);
    f_gaus[i_pad]->Draw("l same");

    float energy_mean = f_gaus[i_pad]->GetParameter(1);
    float energy_width = f_gaus[i_pad]->GetParameter(2);
    showercalib[i_pad] = momentum[i_pad]/energy_mean;
    cout << "Energy after balancing for " << inputenergy << " is: " << energy_mean << " +/- " << energy_width << endl;

    string leg_energy = Form("energy = %1.2f #pm %1.2f",energy_mean,energy_width);
    string leg_showercalib = Form("showercalib: %1.2f",showercalib[i_pad]);
    leg[i_pad] = new TLegend(0.35,0.6,0.85,0.85);
    leg[i_pad]->SetBorderSize(0);
    leg[i_pad]->SetFillColor(0);
    leg[i_pad]->AddEntry(h_mEnergyOut_pion[i_pad],"OHCal #pi^{-}","l");
    leg[i_pad]->AddEntry(f_gaus[i_pad],leg_energy.c_str(),"l");
    leg[i_pad]->AddEntry((TObject*)0,leg_showercalib.c_str()," ");
    leg[i_pad]->Draw("same");
  }
  c_Energy->SaveAs("./figures/c_EnergyOut_2018c.eps");

  ofstream File_OutPut("showercalib_ohcal.txt");
  for(int i_energy = 0; i_energy < 12; ++i_energy)
  {
    cout << momentum[i_energy] << " GeV: showercalib = " << showercalib[i_energy] << endl;
    File_OutPut << momentum[i_energy] << " GeV: showercalib = " << showercalib[i_energy] << endl;
  }
  File_OutPut.close();
}
