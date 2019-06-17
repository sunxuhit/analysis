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

void extractLevelingPar_2018c()
{
  int mEnergy[12] = {3,4,5,6,8,12,16,20,24,28,40,50};
  float momentum[12];

  TH2F *h_mAsymmEnergy_pion[12];
  TProfile *p_mAsymmEnergy_pion[12];
  TH1F *h_mMomentum[12];
  TFile *File_InPut[12];

  for(int i_energy = 0; i_energy < 12; ++i_energy)
  {
    
    string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/TestBeam/ShowerCalibAna_2018c/Proto4ShowerCalib_%dGeV_2018c.root",mEnergy[i_energy]);
    File_InPut[i_energy] = TFile::Open(inputfile.c_str());
    h_mAsymmEnergy_pion[i_energy] = (TH2F*)File_InPut[i_energy]->Get("h_mAsymmEnergy_pion_balancing");
    p_mAsymmEnergy_pion[i_energy] = (TProfile*)h_mAsymmEnergy_pion[i_energy]->ProfileX("p_mAsymmEnergy_pion",1,-1,"i");
    h_mMomentum[i_energy] = (TH1F*)File_InPut[i_energy]->Get("h_mMomentum");
    momentum[i_energy] = TMath::Abs(h_mMomentum[i_energy]->GetMean());
  }

  float fit_start = -0.8;
  float fit_stop  = 0.8;
  float c_in[12];
  float c_out[12];
  TF1 *f_pol[12];

  TCanvas *c_AsymmEnergy = new TCanvas("c_AsymmEnergy","c_AsymmEnergy",2000,1500);
  c_AsymmEnergy->Divide(4,3);
  for(int i_pad = 0; i_pad < 12; ++i_pad)
  {
    c_AsymmEnergy->cd(i_pad+1);
    c_AsymmEnergy->cd(i_pad+1)->SetLeftMargin(0.15);
    c_AsymmEnergy->cd(i_pad+1)->SetBottomMargin(0.15);
    c_AsymmEnergy->cd(i_pad+1)->SetTicks(1,1);
    c_AsymmEnergy->cd(i_pad+1)->SetGrid(0,0);
    string inputenergy = Form("Beam Momentum %1.0f GeV",momentum[i_pad]);
    h_mAsymmEnergy_pion[i_pad]->SetTitle(inputenergy.c_str());
    h_mAsymmEnergy_pion[i_pad]->GetXaxis()->SetTitle("Energy Asymmetry");
    h_mAsymmEnergy_pion[i_pad]->GetXaxis()->CenterTitle();
    h_mAsymmEnergy_pion[i_pad]->GetYaxis()->SetTitle("Total Energy (GeV)");
    h_mAsymmEnergy_pion[i_pad]->GetYaxis()->CenterTitle();
    h_mAsymmEnergy_pion[i_pad]->GetYaxis()->SetRangeUser(0.0,momentum[i_pad]);
    h_mAsymmEnergy_pion[i_pad]->Draw("colz");
    p_mAsymmEnergy_pion[i_pad]->SetMarkerSize(1.1);
    p_mAsymmEnergy_pion[i_pad]->SetMarkerColor(1);
    p_mAsymmEnergy_pion[i_pad]->SetMarkerStyle(20);
    p_mAsymmEnergy_pion[i_pad]->Draw("pE same");

    string FunName = Form("f_pol_%d",i_pad);
    f_pol[i_pad] = new TF1(FunName.c_str(),"pol1",-1.0,1.0);
    f_pol[i_pad]->SetRange(fit_start,fit_stop);
    p_mAsymmEnergy_pion[i_pad]->Fit(f_pol[i_pad],"NR");
    f_pol[i_pad]->SetLineColor(2);
    f_pol[i_pad]->SetLineWidth(4);
    f_pol[i_pad]->SetLineStyle(2);
    f_pol[i_pad]->Draw("l same");

    string formula = Form("%2.2f#timesE_{Asymm}+%2.2f",f_pol[i_pad]->GetParameter(1),f_pol[i_pad]->GetParameter(0));
    c_in[i_pad] = f_pol[i_pad]->Eval(0.0)/f_pol[i_pad]->Eval(1.0);
    c_out[i_pad] = f_pol[i_pad]->Eval(0.0)/f_pol[i_pad]->Eval(-1.0);

    string leg_energy = Form("%1.0f GeV",momentum[i_pad]);
    TLegend *leg = new TLegend(0.35,0.7,0.75,0.85);
    leg->SetBorderSize(0);
    leg->SetFillColor(10);
    leg->AddEntry(h_mAsymmEnergy_pion[i_pad],leg_energy.c_str(),"h");
    leg->AddEntry(f_pol[i_pad],formula.c_str(),"l");
    leg->Draw("same");
  }
  c_AsymmEnergy->SaveAs("../figures/HCAL_ShowerCalib_2018c/c_EnergyBalancing_2018c.eps");

  ofstream File_OutPut("leveling_corr.txt");
  for(int i_energy = 0; i_energy < 12; ++i_energy)
  {
    cout << momentum[i_energy] << " GeV: c_in = " << c_in[i_energy] << ", c_out = " << c_out[i_energy] << endl;
    File_OutPut << momentum[i_energy] << " GeV: c_in = " << c_in[i_energy] << ", c_out = " << c_out[i_energy] << endl;
  }
  File_OutPut.close();
}
