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

void plotQA_EnergySpec_2018c()
{
  int mEnergy[12] = {3,4,5,6,8,12,16,20,24,28,40,50};
  float momentum[12];

  TH2F *h_mAsymmEnergy_data[12];
  TH1F *h_mEnergy_data[12];
  TFile *File_InPut_data[12];

  TH2F *h_mAsymmEnergy_sim[12];
  TH1F *h_mEnergy_sim[12];
  TFile *File_InPut_sim[12];

  TLegend *leg[12];

  for(int i_energy = 0; i_energy < 12; ++i_energy) // read in data
  {
    string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/Proto4ShowerCalib_%dGeV_2018c.root",mEnergy[i_energy]);
    File_InPut_data[i_energy] = TFile::Open(inputfile.c_str());

    h_mAsymmEnergy_data[i_energy] = (TH2F*)File_InPut_data[i_energy]->Get("h_mAsymmEnergy_pion_showercalib");
    h_mEnergy_data[i_energy] = (TH1F*)h_mAsymmEnergy_data[i_energy]->ProjectionY()->Clone("h_mEnergy_data");
  }


  for(int i_energy = 0; i_energy < 12; ++i_energy) // read in simulation 
  {
    string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/Simulation/ShowerCalibAna/Proto4Simulation_2018c_pion_%dGeV.root",mEnergy[i_energy]);
    File_InPut_sim[i_energy] = TFile::Open(inputfile.c_str());
    momentum[i_energy] = (float)mEnergy[i_energy];

    h_mAsymmEnergy_sim[i_energy] = (TH2F*)File_InPut_sim[i_energy]->Get("h_mAsymmEnergy_showercalib");
    h_mEnergy_sim[i_energy] = (TH1F*)h_mAsymmEnergy_sim[i_energy]->ProjectionY()->Clone("h_mEnergy_sim");
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

    // string inputenergy = Form("Beam Momentum %1.0f GeV",momentum[i_pad]);
    // h_mEnergy_sim[i_pad]->SetTitle(inputenergy.c_str());
    h_mEnergy_sim[i_pad]->SetTitle("" );
    h_mEnergy_sim[i_pad]->SetStats(0);
    h_mEnergy_sim[i_pad]->GetXaxis()->SetTitle("Total Energy (GeV)");
    h_mEnergy_sim[i_pad]->GetXaxis()->CenterTitle();
    h_mEnergy_sim[i_pad]->GetXaxis()->SetRangeUser(0.0,2.5*momentum[i_pad]);
    h_mEnergy_sim[i_pad]->GetYaxis()->SetTitle();
    h_mEnergy_sim[i_pad]->GetYaxis()->CenterTitle();
    h_mEnergy_sim[i_pad]->GetYaxis()->SetRangeUser(0.0,1.2*h_mEnergy_sim[i_pad]->GetMaximum());
    h_mEnergy_sim[i_pad]->SetLineColor(4);
    h_mEnergy_sim[i_pad]->SetLineWidth(1);
    h_mEnergy_sim[i_pad]->SetLineStyle(1);
    h_mEnergy_sim[i_pad]->SetFillColor(4);
    h_mEnergy_sim[i_pad]->SetFillStyle(3002);
    h_mEnergy_sim[i_pad]->Draw("h");

    float Inte_data = h_mEnergy_data[i_pad]->Integral();
    float Inte_sim  = h_mEnergy_sim[i_pad]->Integral();
    float scale_factor = Inte_sim/Inte_data;

    h_mEnergy_data[i_pad]->Scale(scale_factor);
    h_mEnergy_data[i_pad]->SetMarkerColor(1);
    h_mEnergy_data[i_pad]->SetMarkerStyle(20);
    h_mEnergy_data[i_pad]->SetMarkerSize(0.8);
    h_mEnergy_data[i_pad]->Draw("pE same");

    string inputenergy = Form("%1.0f GeV",momentum[i_pad]);
    leg[i_pad] = new TLegend(0.65,0.6,0.85,0.85);
    leg[i_pad]->SetBorderSize(0);
    leg[i_pad]->SetFillColor(0);
    leg[i_pad]->AddEntry((TObject*)0,inputenergy.c_str()," ");
    leg[i_pad]->AddEntry(h_mEnergy_data[i_pad],"data","p");
    leg[i_pad]->AddEntry(h_mEnergy_sim[i_pad],"sim","f");
    leg[i_pad]->Draw("same");
  }
  c_Energy->SaveAs("./figures/c_EnergySpec_2018c.eps");
}

