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
#include <TObject.h>

void plotQA_EnergyLeakage_2018c()
{
  int mEnergy[12] = {3,4,5,6,8,12,16,20,24,28,40,50};
  float momentum[12];

  TH1F *h_mRatio_Truth[12];
  TH1F *h_mRatio_Tower[12];
  TFile *File_InPut[12];

  TLegend *leg[12];

  for(int i_energy = 3; i_energy < 11; ++i_energy) // read in simulation 
  {
    string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/Simulation/ShowerCalibAna/Proto4Simulation_2018c_pion_%dGeV.root",mEnergy[i_energy]);
    File_InPut[i_energy] = TFile::Open(inputfile.c_str());
    momentum[i_energy] = (float)mEnergy[i_energy];

    h_mRatio_Truth[i_energy] = (TH1F*)File_InPut[i_energy]->Get("h_mRatio_Truth");
    h_mRatio_Tower[i_energy] = (TH1F*)File_InPut[i_energy]->Get("h_mRatio_Tower");
  }

  TCanvas *c_Energy = new TCanvas("c_Energy","c_Energy",2000,1500);
  c_Energy->Divide(4,3);
  for(int i_pad = 3; i_pad < 11; ++i_pad)
  {
    c_Energy->cd(i_pad+1);
    c_Energy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_Energy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_Energy->cd(i_pad+1)->SetTicks(1,1);
    c_Energy->cd(i_pad+1)->SetGrid(0,0);

    // string inputenergy = Form("Beam Momentum %1.0f GeV",momentum[i_pad]);
    // h_mEnergy_sim[i_pad]->SetTitle(inputenergy.c_str());
    h_mRatio_Tower[i_pad]->SetTitle("" );
    h_mRatio_Tower[i_pad]->SetStats(0);
    h_mRatio_Tower[i_pad]->GetXaxis()->SetTitle("Energy Leakage (%)");
    h_mRatio_Tower[i_pad]->GetXaxis()->CenterTitle();
    h_mRatio_Tower[i_pad]->GetXaxis()->SetRangeUser(0.0,1.0);
    h_mRatio_Tower[i_pad]->GetYaxis()->SetTitle();
    h_mRatio_Tower[i_pad]->GetYaxis()->CenterTitle();
    h_mRatio_Tower[i_pad]->GetYaxis()->SetRangeUser(0.0,1.2*h_mRatio_Tower[i_pad]->GetMaximum());
    h_mRatio_Tower[i_pad]->SetMarkerColor(1);
    h_mRatio_Tower[i_pad]->SetMarkerStyle(20);
    h_mRatio_Tower[i_pad]->SetMarkerSize(1.2);
    h_mRatio_Tower[i_pad]->Draw("pE");

    h_mRatio_Truth[i_pad]->SetMarkerColor(4);
    h_mRatio_Truth[i_pad]->SetMarkerStyle(24);
    h_mRatio_Truth[i_pad]->SetMarkerSize(1.2);
    h_mRatio_Truth[i_pad]->Draw("pE same");

    string inputenergy = Form("%1.0f GeV",momentum[i_pad]);
    leg[i_pad] = new TLegend(0.65,0.6,0.85,0.85);
    leg[i_pad]->SetBorderSize(0);
    leg[i_pad]->SetFillColor(0);
    leg[i_pad]->AddEntry((TObject*)0,inputenergy.c_str()," ");
    leg[i_pad]->AddEntry(h_mRatio_Tower[i_pad],"BH/Tower","p");
    leg[i_pad]->AddEntry(h_mRatio_Truth[i_pad],"BH/Truth","p");
    leg[i_pad]->Draw("same");
  }
  c_Energy->SaveAs("./figures/c_EnergyLeakage_2018c.eps");
}

