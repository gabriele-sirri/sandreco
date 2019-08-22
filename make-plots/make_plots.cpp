// Copyright 2019 Matteo Tenti

#include <TGeoManager.h>
#include <TGeoTube.h>
#include <TDatabasePDG.h>

TGeoManager* geo = 0;
TTree* t = 0;
TFile* f = 0;

bool initialized = false;
  
TCut isCC = "EvtCode.String().Contains(\"CC\") != 0";
TCut isDIS = "EvtCode.GetString().Contains(\"DIS\") != 0";
TCut isQE = "EvtCode.GetString().Contains(\"QE\") != 0";
TCut isRES = "EvtCode.GetString().Contains(\"RES\") != 0";
TCut isMEC = "EvtCode.GetString().Contains(\"MEC\") != 0";
TCut isHMULT = "EvtCode.GetString().Contains(\"hmult\") != 0";
TCut isIMD = "EvtCode.GetString().Contains(\"IMD\") != 0";
TCut isNUEEL = "EvtCode.GetString().Contains(\"NuEEL\") != 0";
TCut Htgt = "EvtCode.GetString().Contains(\"1000010010\") != 0";
TCut Ctgt = "EvtCode.GetString().Contains(\"1000060120\") != 0";
TCut Ntgt = "EvtCode.GetString().Contains(\"1000070140\") != 0";
TCut Otgt = "EvtCode.GetString().Contains(\"1000080160\") != 0";
TCut Artgt = "EvtCode.GetString().Contains(\"1000180400\") != 0";
TCut isLAr = "isLAr!=0";
TCut isStt = "isStt!=0";
TCut isMu = "particles.pdg==13";
TCut isPi = "abs(particles.pdg)==211";
TCut isPi0 = "particles.pdg==111";
TCut isP = "particles.pdg==2212";
TCut isN = "particles.pdg==2112";
TCut isGamma = "particles.pdg==22";
TCut isPrimary = "particles.primary==1";
TCut isGoodTrack = "particles.tr.ret_cr==0&&particles.tr.ret_ln==0";

double p_loc[] = {0, 0, 0};
double p_mst[] = {0, 0, 0};

TGraphErrors* DrawAndFill(TCanvas* c, TH2D* h2, TTree* t, const char* var, const char* vcut, TCut* sel, int nbins, double min, double max, double fmin, double fmax, const char* pdffile)
{
  TGraphErrors* gr = new TGraphErrors(h2->GetNbinsX());

  for(int i = 0; i < h2->GetNbinsX(); i++)
  {
    double vmin = h2->GetXaxis()->GetBinLowEdge(i+1);
    double v = h2->GetXaxis()->GetBinCenter(i+1);
    double vmax = h2->GetXaxis()->GetBinUpEdge(i+1);
    double dv = 0.5*h2->GetXaxis()->GetBinWidth(i+1);
    
    TString scan = TString::Format("%s>%f&&%s<=%f",vcut,vmin,vcut,vmax);
    TString varexp = TString::Format("%s>>h(%d,%f,%f)",var,nbins,min,max);
    
    TH1F* h;
    TFitResultPtr r;
    
    c->cd();
    t->Draw(varexp.Data(), scan.Data() && (*sel), "E0");
    h = static_cast<TH1F*>(gDirectory->Get("h"));
    r = h->Fit("gaus","QS","",fmin,fmax);
    if(r.Get() != 0)
    {
      gr->SetPoint(i,v,r->GetParams()[2]);
      gr->SetPointError(i,dv,r->GetErrors()[2]);
    }
    h->Draw("E0");
    c->Print(pdffile);
  }
  return gr;
}

void init(const char* ifile)
{

  f = new TFile(ifile);
  TTree* tEvent = reinterpret_cast<TTree*>(f->Get("tEvent"));
  TTree* tReco = reinterpret_cast<TTree*>(f->Get("tReco"));
  TTree* tDigit = reinterpret_cast<TTree*>(f->Get("tDigit"));
  TTree* tEdep = reinterpret_cast<TTree*>(f->Get("EDepSimEvents"));
  TTree* tGenie = reinterpret_cast<TTree*>(f->Get("gRooTracker"));

  tEvent->AddFriend(tReco);
  tEvent->AddFriend(tDigit);
  tEvent->AddFriend(tEdep);
  tEvent->AddFriend(tGenie);

  t = tEvent;

  geo = reinterpret_cast<TGeoManager*>(f->Get("EDepSimGeometry"));
  
  if(f != 0 && t != 0 && geo != 0)
  {  
    geo->cd(
      "volWorld_PV/volDetEnclosure_PV_0/volKLOEFULLECALSENSITIVE_EXTTRK_NEWGAP_PV_0"
      "/KLOETrackingRegion_volume_PV_0/volKLOESTTFULLNEWCONFNEWGAPLAR_PV_0");
  
    geo->LocalToMaster(p_loc, p_mst);
    
    TGeoTubeSeg* lar = reinterpret_cast<TGeoTubeSeg*>(geo->GetVolume("LArTarget_volume_PV")->GetShape());
    double lar_rmin = lar->GetRmin();
    double lar_rmax = lar->GetRmax();
    double lar_dz = lar->GetDz();
    double phi1 = lar->GetPhi1();
    double phi2 = lar->GetPhi2();
    
    phi1 = phi1 > 180. ? phi1 - 360. : phi1;
    phi2 = phi2 > 180. ? phi2 - 360. : phi2;
    
    phi1 *= TMath::Pi()/180.;
    phi2 *= TMath::Pi()/180.;
    
    TGeoTube* stt = reinterpret_cast<TGeoTube*>(geo->GetVolume("StrawTubeFULL_for_STTPlane2FULL_0_0_lv_PV")->GetShape());
    double stt_dz = stt->GetDz();
    
    t->SetAlias("isLAr",TString::Format("r_kloe > %f && r_kloe < %f && abs(x - %f) < %f &&"
      "(TMath::ATan2(y - %f, z - %f) > %f || TMath::ATan2(y - %f, z - %f) < %f)", 
      lar_rmin, lar_rmax, p_mst[0], lar_dz, p_mst[1], p_mst[2], phi1, p_mst[1], p_mst[2], phi2).Data());
    
    t->SetAlias("isStt",TString::Format("r_kloe < %f && abs(x - %f) < %f", lar_rmin, p_mst[0], stt_dz).Data());
    
    t->SetAlias("pnu","sqrt(pxnu*pxnu+pynu*pynu+pznu*pznu)");
    t->SetAlias("r_kloe",TString::Format("sqrt((y - %f)*(y - %f)+(z - %f)*(z - %f))",p_mst[1],p_mst[1],p_mst[2],p_mst[2]));
    t->SetAlias("pt_reco","sqrt(particles.pyreco*particles.pyreco+particles.pzreco*particles.pzreco)");
    t->SetAlias("pt_true","sqrt(particles.pytrue*particles.pytrue+particles.pztrue*particles.pztrue)");
    t->SetAlias("p_reco","sqrt(particles.pxreco*particles.pxreco+particles.pyreco*particles.pyreco+particles.pzreco*particles.pzreco)");
    t->SetAlias("p_true","sqrt(particles.pxtrue*particles.pxtrue+particles.pytrue*particles.pytrue+particles.pztrue*particles.pztrue)");
    t->SetAlias("b_reco","sqrt(1.-particles.mass*particles.mass/(particles.Ereco*particles.Ereco))");
    t->SetAlias("b_true","sqrt(1.-particles.mass*particles.mass/(particles.Etrue*particles.Etrue))");
    t->SetAlias("b_res","1.-b_reco/b_true");
    t->SetAlias("pt_res","1-pt_true/pt_reco");
    t->SetAlias("dip_reco","TMath::ATan2(particles.pxreco,sqrt(particles.pyreco*particles.pyreco+particles.pzreco*particles.pzreco))");
    t->SetAlias("dip_true","TMath::ATan2(particles.pxtrue,sqrt(particles.pytrue*particles.pytrue+particles.pztrue*particles.pztrue))");
    t->SetAlias("dip_res","dip_reco-dip_true");
    t->SetAlias("Enu_res","1.-Enureco/Enu");
    t->SetAlias("E_res","1.-particles.Ereco/particles.Etrue");
    t->SetAlias("m_reco","sqrt(particles.Ereco*particles.Ereco-p_reco*p_reco)");
    
    initialized = true;
  }
}

void plot(const char* ofile) 
{
  if(!initialized)
  {
    std::cout << "Tree not initialized" << std::endl;
    return;
  }

  gROOT->SetBatch();
  gStyle->SetOptFit(1111);
  
  TString pdffile = ofile;
  pdffile.ReplaceAll(".root",".pdf");

  TCanvas c;
  
  // X vertex
  t->SetLineColor(kRed);
  t->Draw("x>>hx(100,-1800,1800)",isStt,"");
  t->SetLineColor(kBlue);
  t->Draw("x",isLAr,"SAME");
  t->SetLineColor(kBlack);
  t->Draw("x",!isLAr&&!isStt,"SAME");
  c.Print(TString::Format("%s(",pdffile.Data()));
  
  // YZ vertex
  t->SetMarkerColor(kRed);
  t->Draw(TString::Format("y:z>>hyz(100,%f-2000.,%f+2000.,100,%f-2000.,%f+2000.)",p_mst[2],p_mst[2],p_mst[1],p_mst[1]),isStt,"");
  t->SetMarkerColor(kBlue);
  t->Draw("y:z",isLAr,"SAME");
  t->SetMarkerColor(kBlack);
  t->Draw("y:z",!isLAr&&!isStt,"SAME");
  c.Print(pdffile.Data());
  
  // nu energy
  c.SetLogy(true);
  t->Draw("Enu>>henu(1000,0,50000)","","");
  c.Print(pdffile.Data());
  
  // nu direction
  c.SetLogy(false);
  t->Draw("pxnu/pznu:pynu/pznu>>htxty(10,-0.11,-0.1,10,-0.01,0.01)","","COLZ TEXT");
  c.Print(pdffile.Data());
  
  // CC vs NC
  TH1D hCC("hCC","CC/NC",2,0.,2.);
  t->Draw("1>>+hCC",isCC);
  t->Draw("0>>+hCC",!isCC);
  hCC.GetXaxis()->CenterLabels();
  hCC.GetXaxis()->SetBinLabel(1,"NC");
  hCC.GetXaxis()->SetBinLabel(2,"CC");
  hCC.Draw("HIST TEXT");
  c.Print(pdffile.Data());
  
  // interaction type
  TH1D hintType("hintType","interaction type",7,0.,7.);
  t->Draw("0>>+hintType",isDIS);
  t->Draw("1>>+hintType",isQE);
  t->Draw("2>>+hintType",isRES);
  t->Draw("3>>+hintType",isMEC);
  t->Draw("4>>+hintType",isHMULT);
  t->Draw("5>>+hintType",isIMD);
  t->Draw("6>>+hintType",isNUEEL);
  hintType.GetXaxis()->CenterLabels();
  hintType.GetXaxis()->SetBinLabel(1,"DIS");
  hintType.GetXaxis()->SetBinLabel(2,"QE");
  hintType.GetXaxis()->SetBinLabel(3,"RES");
  hintType.GetXaxis()->SetBinLabel(4,"MEC");
  hintType.GetXaxis()->SetBinLabel(5,"HMULT");
  hintType.GetXaxis()->SetBinLabel(6,"IMD");
  hintType.GetXaxis()->SetBinLabel(7,"NUEEL");
  hintType.Draw("HIST TEXT");
  c.Print(pdffile.Data());
  
  // target nuclei
  TH1D htgt("htgt","target nuclei",5,0.,5.);
  t->Draw("0>>+htgt",Htgt);
  t->Draw("1>>+htgt",Ctgt);
  t->Draw("2>>+htgt",Ntgt);
  t->Draw("3>>+htgt",Otgt);
  t->Draw("4>>+htgt",Artgt);
  htgt.GetXaxis()->CenterLabels();
  htgt.GetXaxis()->SetBinLabel(1,"H");
  htgt.GetXaxis()->SetBinLabel(2,"C");
  htgt.GetXaxis()->SetBinLabel(3,"N");
  htgt.GetXaxis()->SetBinLabel(4,"O");
  htgt.GetXaxis()->SetBinLabel(5,"Ar");
  htgt.Draw("HIST TEXT");
  c.Print(pdffile.Data());
  
  // detector
  TH1D hdet("hdet","detector",3,0.,3.);
  t->Draw("0>>+hdet",isLAr);
  t->Draw("1>>+hdet",isStt);
  t->Draw("2>>+hdet",!isStt&&!isLAr);
  hdet.GetXaxis()->CenterLabels();
  hdet.GetXaxis()->SetBinLabel(1,"LAr");
  hdet.GetXaxis()->SetBinLabel(2,"Stt");
  hdet.GetXaxis()->SetBinLabel(3,"none");
  hdet.Draw("HIST TEXT");
  c.Print(pdffile.Data());
  
  // particles
  int npart = t->Draw("Primaries.PDGCode","","");
  double* pdg = t->GetV1();
  
  std::map<int,int> pdgpart;
  
  for(int i = 0; i < npart; i++)
  {
    pdgpart[static_cast<int>(pdg[i])]++;
  }
  
  int npdg = pdgpart.size();
  TH1D hpart("hpart","particles",npdg, 0., npdg);
  int ibin = 0;
  TDatabasePDG db;
  TParticlePDG* p;
  
  for(std::map<int,int>::iterator it=pdgpart.begin(); it!=pdgpart.end(); ++it)
  {
    hpart.SetBinContent(++ibin, it->second);
    p = db.GetParticle(it->first);
    if(p)
      hpart.GetXaxis()->SetBinLabel(ibin,p->GetName());
    else
      hpart.GetXaxis()->SetBinLabel(ibin,TString::Format("%d",it->first).Data());
  }
  hpart.Draw("HIST TEXT");
  c.Print(pdffile.Data());
  
  TH1F* h;
  TH2D* h2;
  TFitResultPtr r;
  TGraphErrors* gr;
  int nbins;
  
  // muon  
  double ebin1[] = {0., 500., 1000., 2000., 3000., 4000., 5000., 10000.};
  nbins = sizeof(ebin1)/sizeof(double)-1;
  TCut c1 = isMu && isGoodTrack;
  
  h2 = new TH2D("",";P_{t} GeV; #sigma(1-P_{t}^{true}/P_{t}^{reco})",nbins,ebin1,1,0.02,0.1);
  h2->GetYaxis()->SetTitleOffset(1.5);
  h2->SetStats(false);
  
  gr = DrawAndFill(&c, h2, t, "pt_res", "pt_true", &c1, 50, -1., 1., -0.5, 0.5, pdffile.Data());
  
  c.cd();
  h2->Draw();
  gr->Draw("P SAME");
  c.Print(pdffile.Data());
  
  t->Draw("dip_res>>h(50,-0.025,0.025)",c1,"");
  h = static_cast<TH1F*>(gDirectory->Get("h"));
  r = h->Fit("gaus","QS","",-0.0025,0.0025);
  h->Draw("E0");
  c.Print(pdffile.Data());
  
  // proton  
  double ebin2[] = {0., 500., 1000., 1500., 2000., 2500., 3000.};
  nbins = sizeof(ebin2)/sizeof(double)-1;
  TCut c2 = isP && isGoodTrack;
  
  h2 = new TH2D("",";P_{t} GeV; #sigma(1-P_{t}^{true}/P_{t}^{reco})",nbins,ebin2,1,0.0,0.3);
  h2->GetYaxis()->SetTitleOffset(1.5);
  h2->SetStats(false);
  
  gr = DrawAndFill(&c, h2, t, "pt_res", "pt_true", &c2, 50, -1., 1., -0.5, 0.5, pdffile.Data());
  
  c.cd();
  h2->Draw();
  gr->Draw("P SAME");
  c.Print(pdffile.Data());
  
  t->Draw("dip_res>>h(50,-0.025,0.025)",c2,"");
  h = static_cast<TH1F*>(gDirectory->Get("h"));
  r = h->Fit("gaus","QS","",-0.0025,0.0025);
  h->Draw("E0");
  c.Print(pdffile.Data());
  
  // pi  
  double ebin3[] = {0., 500., 1000., 1500., 2000., 2500., 3000.};
  nbins = sizeof(ebin3)/sizeof(double)-1;
  TCut c3 = isPi && isGoodTrack;
  
  h2 = new TH2D("",";P_{t} GeV; #sigma(1-P_{t}^{true}/P_{t}^{reco})",nbins,ebin3,1,0.0,0.3);
  h2->GetYaxis()->SetTitleOffset(1.5);
  h2->SetStats(false);
  
  gr = DrawAndFill(&c, h2, t, "pt_res", "pt_true", &c3, 50, -1., 1., -0.5, 0.5, pdffile.Data());
  
  c.cd();
  h2->Draw();
  gr->Draw("P SAME");
  c.Print(pdffile.Data());
  
  t->Draw("dip_res>>h(50,-0.025,0.025)",c2,"");
  h = static_cast<TH1F*>(gDirectory->Get("h"));
  r = h->Fit("gaus","QS","",-0.0025,0.0025);
  h->Draw("E0");
  c.Print(pdffile.Data());
  
  // n
  double ebin4[] = {0., 0.2, 0.4, 0.6, 0.8, 1.};
  nbins = sizeof(ebin4)/sizeof(double)-1;
  TCut c4 = isN;
  
  t->Draw("b_reco:b_true>>h(100,0.,1.,100,0.,1.)",c4,"COLZ");
  TH2F* hh = static_cast<TH2F*>(gDirectory->Get("h"));
  hh->SetStats(false);
  c.Print(pdffile.Data());
  
  h2 = new TH2D("",";#beta_{n} GeV; #sigma(1-#beta_{n}^{reco}/#beta_{n}^{true})",nbins,ebin4,1,0.0,0.3);
  h2->GetYaxis()->SetTitleOffset(1.5);
  h2->SetStats(false);
  
  gr = DrawAndFill(&c, h2, t, "b_res", "b_true", &c4, 50, -1., 1., -0.5, 0.5, pdffile.Data());
  
  c.cd();
  h2->Draw();
  gr->Draw("P SAME");
  c.Print(pdffile.Data());
  
  // gamma
  double ebin5[] = {0., 500., 1000., 2000., 5000.};
  nbins = sizeof(ebin5)/sizeof(double)-1;
  TCut c5 = isGamma;
  
  h2 = new TH2D("",";E_{#gamma} GeV; #sigma(1-E_{#gamma}^{reco}/E_{#gamma}^{true})",nbins,ebin5,1,0.0,0.3);
  h2->GetYaxis()->SetTitleOffset(1.5);
  h2->SetStats(false);
  
  gr = DrawAndFill(&c, h2, t, "E_res", "particles.Etrue", &c5, 50, -1., 1., -0.5, 0.5, pdffile.Data());
  
  c.cd();
  h2->Draw();
  gr->Draw("P SAME");
  c.Print(pdffile.Data());
  
  // pi0
  TCut c6 = isPi0;
  t->Draw("m_reco>>h(100,0,500.)",c6,"E0");
  h = static_cast<TH1F*>(gDirectory->Get("h"));
  r = h->Fit("gaus","QS","",0,500.);
  h->Draw("E0");
  c.Print(pdffile.Data());
  
  // nu 
  double ebin7[] = {0., 500., 1000., 2000., 3000., 4000., 5000.};
  nbins = sizeof(ebin4)/sizeof(double)-1;
  TCut c7 = isCC;
  
  h2 = new TH2D("",";E_{#nu} GeV; #sigma(1-E_{#nu}^{reco}/E_{#nu}^{true})",nbins,ebin7,1,0.0,0.3);
  h2->GetYaxis()->SetTitleOffset(1.5);
  h2->SetStats(false);
  
  gr = DrawAndFill(&c, h2, t, "Enu_res", "Enu", &c7, 50, -1., 1., -0.5, 0.5, pdffile.Data());
  
  c.cd();
  h2->Draw();
  gr->Draw("P SAME");  
  c.Print(TString::Format("%s)",pdffile.Data()));
}

void make_plots()
{
  gSystem->Load("/wd/dune-it/enurec/analysis/kloe-simu/lib/libStruct.so");

  init("/home/dune-it/data/reco/numu_geoV12_1000.0.reco.nokalman.root");
  plot("nokalman.pdf");
  init("/home/dune-it/data/reco/numu_geoV12_1000.0.reco.kalman.root");
  plot("kalman.pdf");
}
