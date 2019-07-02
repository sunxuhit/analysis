
void extractRatio_epi()
{
  string inputfile = "/sphenix/user/xusun/TestBeam/ShowerCalibAna/ROOT5/Proto4ShowerInfoRAW_0571.root";
  TFile *File_InPut = TFile::Open(inputfile.c_str());

  TH2F *h_mAsymmEnergy_pion_ShowerCalib = (TH2F*)File_InPut->Get("h_mAsymmEnergy_pion_ShowerCalib");
  TH1F *h_mEnergy_pion = (TH1F*)h_mAsymmEnergy_pion_ShowerCalib->ProjectionY("h_mEnergy_pion");
  TH2F *h_mAsymmEnergy_electron_ShowerCalib = (TH2F*)File_InPut->Get("h_mAsymmEnergy_electron_ShowerCalib");
  TH1F *h_mEnergy_electron = (TH1F*)h_mAsymmEnergy_electron_ShowerCalib->ProjectionY("h_mEnergy_electron");

  TCanvas *c_Energy = new TCanvas("c_Energy","c_Energy",10,10,800,800);
  c_Energy->cd(1);
  c_Energy->cd(1)->SetLeftMargin(0.15);
  c_Energy->cd(1)->SetBottomMargin(0.15);
  c_Energy->cd(1)->SetTicks(1,1);
  c_Energy->cd(1)->SetGrid(0,0);
  h_mEnergy_electron->SetTitle("");
  h_mEnergy_electron->SetStats(0);
  h_mEnergy_electron->SetLineColor(2);
  h_mEnergy_electron->GetYaxis()->SetRangeUser(0,h_mEnergy_electron->GetMaximum()*1.3);
  h_mEnergy_electron->Draw("hE");
  TF1 *f_gaus_electron = new TF1("f_gaus_electron","gaus",0,100);
  f_gaus_electron->SetParameter(0,1.0);
  f_gaus_electron->SetParameter(1,h_mEnergy_electron->GetMean());
  f_gaus_electron->SetParameter(2,1.0);
  f_gaus_electron->SetRange(12.0,22.0);
  h_mEnergy_electron->Fit(f_gaus_electron,"NR");
  float energy_electron = f_gaus_electron->GetParameter(1);
  string leg_electron = Form("electron: %1.2f",energy_electron);

  f_gaus_electron->SetLineColor(2);
  f_gaus_electron->SetLineStyle(2);
  f_gaus_electron->SetLineWidth(2);
  f_gaus_electron->Draw("l same");


  h_mEnergy_pion->SetLineColor(4);
  h_mEnergy_pion->Draw("hE same");
  TF1 *f_gaus_pion = new TF1("f_gaus_pion","gaus",0,100);
  f_gaus_pion->SetParameter(0,1.0);
  f_gaus_pion->SetParameter(1,h_mEnergy_pion->GetMean());
  f_gaus_pion->SetParameter(2,1.0);
  f_gaus_pion->SetRange(6.0,18.0);
  h_mEnergy_pion->Fit(f_gaus_pion,"NR");
  float energy_pion = f_gaus_pion->GetParameter(1);
  string leg_pion = Form("pion: %1.2f",energy_pion);

  f_gaus_pion->SetLineColor(4);
  f_gaus_pion->SetLineStyle(2);
  f_gaus_pion->SetLineWidth(2);
  f_gaus_pion->Draw("l same");

  TLegend *leg = new TLegend(0.45,0.75,0.85,0.85);
  leg->SetBorderSize(0);
  leg->SetFillColor(10);
  leg->AddEntry(f_gaus_electron,leg_electron.c_str(),"l");
  leg->AddEntry(f_gaus_pion,leg_pion.c_str(),"l");
  leg->Draw();

  c_Energy->SaveAs("../figures/HCAL_ShowerCalib/c_RatioEPi_2018a.eps");
}
