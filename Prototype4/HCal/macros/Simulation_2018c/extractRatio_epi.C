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

void extractRatio_epi()
{
  int mEnergy[12] = {3,4,5,6,8,12,16,20,24,28,40,50};
  float momentum[12];

  TFile *File_InPut_pion[12];
  TH2F *h_mAsymmEnergy_pion[12];
  TH1F *h_mEnergy_pion[12];

  TFile *File_InPut_electron[12];
  TH2F *h_mAsymmEnergy_electron[12];
  TH1F *h_mEnergy_electron[12];

  TF1 *f_gaus_pion[12];
  TF1 *f_gaus_electron[12];
  TLegend *leg[12];

  for(int i_energy = 0; i_energy < 12; ++i_energy)
  {
    momentum[i_energy] = (float)mEnergy[i_energy];

    string input_pion = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/Simulation/ShowerCalibAna/Proto4Simulation_2018c_pion_%dGeV.root",mEnergy[i_energy]);
    File_InPut_pion[i_energy] = TFile::Open(input_pion.c_str());
    h_mAsymmEnergy_pion[i_energy] = (TH2F*)File_InPut_pion[i_energy]->Get("h_mAsymmEnergy_showercalib");
    h_mEnergy_pion[i_energy] = (TH1F*)h_mAsymmEnergy_pion[i_energy]->ProjectionY()->Clone("h_mEnergy_pion");

    string input_electron = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/Simulation/ShowerCalibAna/Proto4Simulation_2018c_electron_%dGeV.root",mEnergy[i_energy]);
    File_InPut_electron[i_energy] = TFile::Open(input_electron.c_str());
    h_mAsymmEnergy_electron[i_energy] = (TH2F*)File_InPut_electron[i_energy]->Get("h_mAsymmEnergy_showercalib");
    h_mEnergy_electron[i_energy] = (TH1F*)h_mAsymmEnergy_electron[i_energy]->ProjectionY()->Clone("h_mEnergy_electron");
  }

  float val_pion_mean[12];
  float err_pion_mean[12];
  float val_electron_mean[12];
  float err_electron_mean[12];
  float val_ratio[12];
  float err_ratio[12];

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
    h_mEnergy_pion[i_pad]->SetTitle(inputenergy.c_str());
    h_mEnergy_pion[i_pad]->SetStats(0);
    h_mEnergy_pion[i_pad]->GetXaxis()->SetTitle("Total Energy (GeV)");
    h_mEnergy_pion[i_pad]->GetXaxis()->CenterTitle();
    h_mEnergy_pion[i_pad]->GetXaxis()->SetRangeUser(0.0,3.5*momentum[i_pad]);
    h_mEnergy_pion[i_pad]->GetYaxis()->SetTitle();
    h_mEnergy_pion[i_pad]->GetYaxis()->CenterTitle();
    h_mEnergy_pion[i_pad]->GetYaxis()->SetRangeUser(0.0,1.4*h_mEnergy_pion[i_pad]->GetMaximum());
    h_mEnergy_pion[i_pad]->SetLineColor(1);
    h_mEnergy_pion[i_pad]->SetLineWidth(1);
    h_mEnergy_pion[i_pad]->SetLineStyle(1);
    h_mEnergy_pion[i_pad]->Draw("h");

    h_mEnergy_electron[i_pad]->SetLineColor(2);
    h_mEnergy_electron[i_pad]->SetLineWidth(1);
    h_mEnergy_electron[i_pad]->SetLineStyle(1);
    h_mEnergy_electron[i_pad]->Draw("h same");

    string FuncName;
    FuncName = Form("f_gaus_pion_%d",i_pad);
    f_gaus_pion[i_pad] = new TF1(FuncName.c_str(),"gaus",0,100);
    f_gaus_pion[i_pad]->SetParameter(0,1.0);
    f_gaus_pion[i_pad]->SetParameter(1,h_mEnergy_pion[i_pad]->GetMean());
    f_gaus_pion[i_pad]->SetParameter(2,1.0);
    f_gaus_pion[i_pad]->SetRange(0,2.0*momentum[i_pad]);
    h_mEnergy_pion[i_pad]->Fit(f_gaus_pion[i_pad],"NQR");

    float norm_pion  = f_gaus_pion[i_pad]->GetParameter(0);
    float mean_pion  = f_gaus_pion[i_pad]->GetParameter(1); 
    float sigma_pion = f_gaus_pion[i_pad]->GetParameter(2);
    f_gaus_pion[i_pad]->SetParameter(0,norm_pion);
    f_gaus_pion[i_pad]->SetParameter(1,mean_pion);
    f_gaus_pion[i_pad]->SetParameter(2,sigma_pion);
    f_gaus_pion[i_pad]->SetRange(mean_pion-2.0*sigma_pion,mean_pion+2.0*sigma_pion);
    h_mEnergy_pion[i_pad]->Fit(f_gaus_pion[i_pad],"NQR");

    f_gaus_pion[i_pad]->SetLineColor(4);
    f_gaus_pion[i_pad]->SetLineStyle(2);
    f_gaus_pion[i_pad]->SetLineWidth(3);
    f_gaus_pion[i_pad]->Draw("l same");
    val_pion_mean[i_pad] = f_gaus_pion[i_pad]->GetParameter(1);
    err_pion_mean[i_pad] = f_gaus_pion[i_pad]->GetParError(1);
    string leg_pion = Form("pion: %1.2f",val_pion_mean[i_pad]);

    FuncName = Form("f_gaus_electron_%d",i_pad);
    f_gaus_electron[i_pad] = new TF1(FuncName.c_str(),"gaus",0,100);
    f_gaus_electron[i_pad]->SetParameter(0,1.0);
    f_gaus_electron[i_pad]->SetParameter(1,h_mEnergy_electron[i_pad]->GetMean());
    f_gaus_electron[i_pad]->SetParameter(2,1.0);
    f_gaus_electron[i_pad]->SetRange(0,3.0*momentum[i_pad]);
    h_mEnergy_electron[i_pad]->Fit(f_gaus_electron[i_pad],"NQR");

    float norm_electron  = f_gaus_electron[i_pad]->GetParameter(0);
    float mean_electron  = f_gaus_electron[i_pad]->GetParameter(1); 
    float sigma_electron = f_gaus_electron[i_pad]->GetParameter(2);
    f_gaus_electron[i_pad]->SetParameter(0,norm_electron);
    f_gaus_electron[i_pad]->SetParameter(1,mean_electron);
    f_gaus_electron[i_pad]->SetParameter(2,sigma_electron);
    f_gaus_electron[i_pad]->SetRange(mean_electron-2.0*sigma_electron,mean_electron+2.0*sigma_electron);
    h_mEnergy_electron[i_pad]->Fit(f_gaus_electron[i_pad],"NQR");

    f_gaus_electron[i_pad]->SetLineColor(2);
    f_gaus_electron[i_pad]->SetLineStyle(2);
    f_gaus_electron[i_pad]->SetLineWidth(3);
    f_gaus_electron[i_pad]->Draw("l same");
    val_electron_mean[i_pad] = f_gaus_electron[i_pad]->GetParameter(1);
    err_electron_mean[i_pad] = f_gaus_electron[i_pad]->GetParError(1);
    string leg_electron = Form("electron: %1.2f",val_electron_mean[i_pad]);

    leg[i_pad] = new TLegend(0.35,0.6,0.85,0.85);
    leg[i_pad]->SetBorderSize(0);
    leg[i_pad]->SetFillColor(0);
    leg[i_pad]->AddEntry(f_gaus_pion[i_pad],leg_pion.c_str(),"l");
    leg[i_pad]->AddEntry(f_gaus_electron[i_pad],leg_electron.c_str(),"l");
    leg[i_pad]->Draw("same");

    val_ratio[i_pad] = val_electron_mean[i_pad]/val_pion_mean[i_pad];
    err_ratio[i_pad] = ErrDiv(val_electron_mean[i_pad],val_pion_mean[i_pad],err_electron_mean[i_pad],err_pion_mean[i_pad]);
  }
  c_Energy->SaveAs("./figures/c_EnergySpec_epi.eps");

  TGraphAsymmErrors *g_ratio = new TGraphAsymmErrors();
  for(int i_point = 0; i_point < 12; ++i_point)
  {
    g_ratio->SetPoint(i_point,momentum[i_point],val_ratio[i_point]);
    g_ratio->SetPointError(i_point,0.0,0.0,err_ratio[i_point],err_ratio[i_point]);
  }

  string outputfile = "/sphenix/user/xusun/Simulation/ShowerCalibAna/Simulation_2018c_ratio.root";
  TFile *File_OutPut = new TFile(outputfile.c_str(),"RECREATE");
  File_OutPut->cd();
  g_ratio->SetName("g_ratio_2018c");
  g_ratio->Write();
  File_OutPut->Close();
}
