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

void plotQA_Trigger(int mEnergy = 12, int mRunId = 2624)
{
  string inputfile = Form("/gpfs/mnt/gpfs02/sphenix/user/xusun/TestBeam/ShowerCalib_2018c/%dGeV/Proto4ShowerInfoRaw_%d_2018c.root",mEnergy,mRunId);
  TFile *File_InPut = TFile::Open(inputfile.c_str());
  TH1F *h_Check_c2 = (TH1F*)File_InPut->Get("hCheck_C2");
  TH1F *h_Check_veto = (TH1F*)File_InPut->Get("hCheck_Veto");
  TH1F *h_Check_hodo_h = (TH1F*)File_InPut->Get("hCheck_Hodo_H");
  TH1F *h_Check_hodo_v = (TH1F*)File_InPut->Get("hCheck_Hodo_V");

  TCanvas *c_trigger = new TCanvas("c_trigger","c_trigger",800,800);
  c_trigger->Divide(2,2);
  for(int i_pad = 0; i_pad < 4; ++i_pad)
  {
    c_trigger->cd(i_pad+1);
    c_trigger->cd(i_pad+1)->SetLeftMargin(0.15);
    c_trigger->cd(i_pad+1)->SetBottomMargin(0.15);
    c_trigger->cd(i_pad+1)->SetTicks(1,1);
    c_trigger->cd(i_pad+1)->SetGrid(0,0);
    c_trigger->cd(i_pad+1)->SetLogy();
  }

  c_trigger->cd(1);
  h_Check_c2->SetTitle("C2 Sum");
  h_Check_c2->SetStats(0);
  h_Check_c2->Draw("hE");
  TLine *l_c2 = new TLine(1500,h_Check_c2->GetMinimum(),1500,h_Check_c2->GetMaximum()*0.8);
  l_c2->SetLineColor(2);
  l_c2->SetLineStyle(2);
  l_c2->SetLineWidth(4);
  l_c2->Draw("l same");

  c_trigger->cd(2);
  h_Check_veto->SetTitle("Veto");
  h_Check_veto->SetStats(0);
  h_Check_veto->GetXaxis()->SetRangeUser(-2,10);
  h_Check_veto->Draw("hE");
  TLine *l_veto = new TLine(0.2,h_Check_veto->GetMinimum(),0.2,h_Check_veto->GetMaximum()*0.8);
  l_veto->SetLineColor(2);
  l_veto->SetLineStyle(2);
  l_veto->SetLineWidth(4);
  l_veto->Draw("l same");

  c_trigger->cd(3);
  h_Check_hodo_h->SetTitle("HODO H");
  h_Check_hodo_h->SetStats(0);
  h_Check_hodo_h->GetXaxis()->SetRangeUser(-2,10);
  h_Check_hodo_h->Draw("hE");
  TLine *l_hodo_h = new TLine(0.5,h_Check_hodo_h->GetMinimum(),0.5,h_Check_hodo_h->GetMaximum()*0.8);
  l_hodo_h->SetLineColor(2);
  l_hodo_h->SetLineStyle(2);
  l_hodo_h->SetLineWidth(4);
  l_hodo_h->Draw("l same");

  c_trigger->cd(4);
  h_Check_hodo_v->SetTitle("HODO H");
  h_Check_hodo_v->SetStats(0);
  h_Check_hodo_v->GetXaxis()->SetRangeUser(-2,10);
  h_Check_hodo_v->Draw("hE");
  TLine *l_hodo_v = new TLine(0.5,h_Check_hodo_v->GetMinimum(),0.5,h_Check_hodo_v->GetMaximum()*0.8);
  l_hodo_v->SetLineColor(2);
  l_hodo_v->SetLineStyle(2);
  l_hodo_v->SetLineWidth(4);
  l_hodo_v->Draw("l same");

  c_trigger->SaveAs("./figures/c_trigger_12GeV.eps");
}
