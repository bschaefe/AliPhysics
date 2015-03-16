/**************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

/////////////////////////////////////////////////////////////
//
// Class deriving from AliHFMassFitter for the fit of D0 invariant mass distribution
// including the possibility of using templates for reflections
//
// Main changes:
//   -   more flexibility in defining the number of back, signal, and reflection parameters (--> background function). 
//   -   add possibility to fit with reflection template (more options available)
//
// Probably a shorter and more efficient version could have been obtained writing a totally independent class. 
// Author: A. Rossi, andrea.rossi@cern.ch
/////////////////////////////////////////////////////////////

#include <Riostream.h>
#include <TMath.h>
#include <TNtuple.h>
#include <TH1F.h>
#include <TF1.h>
#include <TList.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TVirtualPad.h>
#include <TGraph.h>
#include <TVirtualFitter.h>
#include <TMinuit.h>
#include <TStyle.h>
#include <TPaveText.h>
#include <TDatabasePDG.h>
#include "AliHFMassFitter.h"
#include "AliHFMassFitterVAR.h"
#include "AliVertexingHFUtils.h"


using std::cout;
using std::endl;

ClassImp(AliHFMassFitterVAR)

 //************** constructors
AliHFMassFitterVAR::AliHFMassFitterVAR() : 
AliHFMassFitter(),
  fNparSignal(3),
  fNparBack(2),
  fNparRefl(0),
  fhTemplRefl(0x0),
  fSignParNames(0x0),
  fBackParNames(0x0),
  fReflParNames(0x0),
  fSmoothRefl(kFALSE),
  fReflInit(0),
  fFixParSign(0x0),
  fFixParBack(0x0),
  fFixParRefl(0x0),
  fFixParSignExternalValue(0x0),
  fFixParBackExternalValue(0x0),
  fFixParReflExternalValue(0x0),
  fparSignFixExt(0x0),
  fparBackFixExt(0x0),
  fparReflFixExt(0x0),
  fRawYieldHelp(0)
{
  // default constructor
 
  cout<<"Default constructor:"<<endl;
  cout<<"Remember to set the Histo, the Type, the FixPar"<<endl;

}

//___________________________________________________________________________
AliHFMassFitterVAR::AliHFMassFitterVAR (const TH1F *histoToFit, Double_t minvalue, Double_t maxvalue, Int_t rebin,Int_t fittypeb,Int_t fittypes): 
  AliHFMassFitter(),
  fNparSignal(3),
  fNparBack(2),
  fNparRefl(0),
  fhTemplRefl(0x0),
  fSignParNames(0x0),
  fBackParNames(0x0),
  fReflParNames(0x0),
  fSmoothRefl(kFALSE),
  fReflInit(0),
  fFixParSign(0x0),
  fFixParBack(0x0),
  fFixParRefl(0x0),
  fFixParSignExternalValue(0x0),
  fFixParBackExternalValue(0x0),
  fFixParReflExternalValue(0x0),
  fparSignFixExt(0x0),
  fparBackFixExt(0x0),
  fparReflFixExt(0x0),
  fRawYieldHelp(0)
{
  // standard constructor

  fhistoInvMass= (TH1F*)histoToFit->Clone("fhistoInvMass");
  fhistoInvMass->SetDirectory(0);
  fminMass=minvalue; 
  fmaxMass=maxvalue;
  if(rebin!=1) RebinMass(rebin); 
  else fNbin=(Int_t)fhistoInvMass->GetNbinsX();
  CheckRangeFit();
  ftypeOfFit4Bkg=fittypeb;
  ftypeOfFit4Sgn=fittypes;
  if(ftypeOfFit4Bkg!=0 && ftypeOfFit4Bkg!=1 && ftypeOfFit4Bkg!=2 && ftypeOfFit4Bkg!=4 && ftypeOfFit4Bkg!=5) fWithBkg=kFALSE;
  else fWithBkg=kTRUE;
  if (!fWithBkg) cout<<"Fit Histogram of Signal only"<<endl;
  else  cout<<"Type of fit For Background = "<<ftypeOfFit4Bkg<<endl;

  ComputeParSize();
  ComputeNFinalPars();
  fFitPars=new Float_t[fParsSize];

  SetDefaultFixParam();

  fContourGraph=new TList();
  fContourGraph->SetOwner();

}


AliHFMassFitterVAR::AliHFMassFitterVAR(const AliHFMassFitterVAR &mfit):
  AliHFMassFitter(mfit),
  fNparSignal(mfit.fNparSignal),
  fNparBack(mfit.fNparBack),
  fNparRefl(mfit.fNparRefl),
  fhTemplRefl(mfit.fhTemplRefl),
  fSignParNames(0x0),
  fBackParNames(0x0),
  fReflParNames(0x0),
  fSmoothRefl(mfit.fSmoothRefl),
  fReflInit(mfit.fReflInit),
  fFixParSign(0x0),
  fFixParBack(0x0),
  fFixParRefl(0x0),
  fFixParSignExternalValue(0x0),
  fFixParBackExternalValue(0x0),
  fFixParReflExternalValue(0x0),
  fparSignFixExt(0x0),
  fparBackFixExt(0x0),
  fparReflFixExt(0x0),
  fRawYieldHelp(mfit.fRawYieldHelp)
{
  //copy constructor
  fSignParNames=new TString[fNparSignal];
  for(Int_t j=0;j<fNparSignal;j++){
    fSignParNames[j]=mfit.fSignParNames[j];
  }

  fBackParNames=new TString[fNparBack];
  for(Int_t j=0;j<fNparBack;j++){
    fBackParNames[j]=mfit.fBackParNames[j];
  }

  fReflParNames=new TString[fNparRefl];
  for(Int_t j=0;j<fNparRefl;j++){
    fReflParNames[j]=mfit.fReflParNames[j];
  }


  fFixParSign=new Bool_t[fNparSignal];
  fFixParSignExternalValue=new Bool_t[fNparSignal];
  fparSignFixExt=new Double_t[fNparSignal];
  
  fFixParBack=new Bool_t[fNparBack];
  fFixParBackExternalValue=new Bool_t[fNparBack];
  fparBackFixExt=new Double_t[fNparBack];
  
  fFixParRefl=new Bool_t[fNparRefl];
  fFixParReflExternalValue=new Bool_t[fNparRefl];
  fparReflFixExt=new Double_t[fNparRefl];

  memcpy(fFixParSign,mfit.fFixParSign,mfit.fNparSignal*sizeof(Bool_t));
  memcpy(fFixParBack,mfit.fFixParBack,mfit.fNparBack*sizeof(Bool_t));
  memcpy(fFixParRefl,mfit.fFixParRefl,mfit.fNparRefl*sizeof(Double_t));
  
  memcpy(fFixParSignExternalValue,mfit.fFixParSignExternalValue,mfit.fNparSignal*sizeof(Bool_t));
  memcpy(fFixParBackExternalValue,mfit.fFixParBackExternalValue,mfit.fNparBack*sizeof(Bool_t));
  memcpy(fFixParReflExternalValue,mfit.fFixParReflExternalValue,mfit.fNparRefl*sizeof(Double_t));
  
  memcpy(fparSignFixExt,mfit.fparSignFixExt,mfit.fNparSignal*sizeof(Bool_t));
  memcpy(fparBackFixExt,mfit.fparBackFixExt,mfit.fNparBack*sizeof(Bool_t));
  memcpy(fparReflFixExt,mfit.fparReflFixExt,mfit.fNparRefl*sizeof(Double_t));
  

}

//_________________________________________________________________________

AliHFMassFitterVAR::~AliHFMassFitterVAR() {
  //destructor

  delete fhTemplRefl;
  delete [] fSignParNames;
  delete [] fBackParNames;
  delete [] fReflParNames;

  delete [] fFixParSign;
  delete [] fFixParBack;
  delete [] fFixParRefl;


  delete [] fFixParSignExternalValue;
  delete [] fFixParBackExternalValue;
  delete [] fFixParReflExternalValue;

  delete [] fparSignFixExt;
  delete [] fparBackFixExt;
  delete [] fparReflFixExt;


}

//_________________________________________________________________________

AliHFMassFitterVAR& AliHFMassFitterVAR::operator=(const AliHFMassFitterVAR &mfit){

  //assignment operator

  if(&mfit == this) return *this;
  AliHFMassFitter::operator=(mfit); 

  fNparSignal=mfit.fNparSignal;
  fNparBack=mfit.fNparBack;
  fNparRefl=mfit.fNparRefl;
  fhTemplRefl=mfit.fhTemplRefl;

  fSmoothRefl=mfit.fSmoothRefl;
  fRawYieldHelp=mfit.fRawYieldHelp;

  delete [] fSignParNames;
  delete [] fBackParNames;
  delete [] fReflParNames;
  
  fSignParNames=new TString[fNparSignal];
  fBackParNames=new TString[fNparBack];
  fReflParNames=new TString[fNparRefl];

  delete [] fparSignFixExt;
  delete [] fparBackFixExt;
  delete [] fparReflFixExt;

  fparSignFixExt=new Double_t[fNparSignal];
  fparBackFixExt=new Double_t[fNparBack];
  fparReflFixExt=new Double_t[fNparRefl];
  

  delete [] fFixParSign;
  delete [] fFixParBack;
  delete [] fFixParRefl;
  
  fFixParSign=new Bool_t[fNparSignal];
  fFixParBack=new Bool_t[fNparBack];
 fFixParRefl=new Bool_t[fNparRefl];


  delete [] fFixParSignExternalValue;
  delete [] fFixParBackExternalValue;
  delete [] fFixParReflExternalValue;
  
  fFixParSignExternalValue=new Bool_t[fNparSignal];
  fFixParBackExternalValue=new Bool_t[fNparBack];
 fFixParReflExternalValue=new Bool_t[fNparRefl];


 fSignParNames=new TString[fNparSignal];
  for(Int_t j=0;j<fNparSignal;j++){
    fSignParNames[j]=mfit.fSignParNames[j];
  }

  fBackParNames=new TString[fNparBack];
  for(Int_t j=0;j<fNparBack;j++){
    fBackParNames[j]=mfit.fBackParNames[j];
  }

  fReflParNames=new TString[fNparRefl];
  for(Int_t j=0;j<fNparRefl;j++){
    fReflParNames[j]=mfit.fReflParNames[j];
  }




  memcpy(fFixParSign,mfit.fFixParSign,mfit.fNparSignal*sizeof(Bool_t));
  memcpy(fFixParBack,mfit.fFixParBack,mfit.fNparBack*sizeof(Bool_t));
  memcpy(fFixParRefl,mfit.fFixParRefl,mfit.fNparRefl*sizeof(Bool_t));
  
  memcpy(fFixParSignExternalValue,mfit.fFixParSignExternalValue,mfit.fNparSignal*sizeof(Bool_t));
  memcpy(fFixParBackExternalValue,mfit.fFixParBackExternalValue,mfit.fNparBack*sizeof(Bool_t));
  memcpy(fFixParReflExternalValue,mfit.fFixParReflExternalValue,mfit.fNparRefl*sizeof(Bool_t));

  
  memcpy(fparSignFixExt,mfit.fparSignFixExt,mfit.fNparSignal*sizeof(Double_t));
  memcpy(fparBackFixExt,mfit.fparBackFixExt,mfit.fNparBack*sizeof(Double_t));
  memcpy(fparReflFixExt,mfit.fparReflFixExt,mfit.fNparRefl*sizeof(Double_t));

  
  return *this;
}

TH1F*  AliHFMassFitterVAR::SetTemplateReflections(const TH1 *h, TString opt,Double_t minRange,Double_t maxRange){
  fhTemplRefl=(TH1F*)h->Clone("hTemplRefl");  
  
  if(opt.EqualTo("templ")||opt.EqualTo("Templ")||opt.EqualTo("TEMPL")||opt.EqualTo("template")||opt.EqualTo("Template")||opt.EqualTo("TEMPLATE")){
    return fhTemplRefl;
  }
  TF1 *f;
  Bool_t isPoissErr=kTRUE;
  if(opt.EqualTo("1gaus")||opt.EqualTo("singlegaus")){
    if(minRange>=0&&maxRange>=0){
      f=new TF1("mygaus","gaus",TMath::Max(minRange,h->GetBinLowEdge(1)),TMath::Min(maxRange,h->GetXaxis()->GetBinUpEdge(h->GetNbinsX())));
    }
    else f=new TF1("mygaus","gaus",h->GetBinLowEdge(1),h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
    f->SetParameter(0,h->GetMaximum());
    //    f->SetParLimits(0,0,100.*h->Integral());
    f->SetParameter(1,1.865);
    f->SetParameter(2,0.050);

    fhTemplRefl->Fit(f,"RELMI","");//,h->GetBinLowEdge(1),h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
    for(Int_t j=1;j<=fhTemplRefl->GetNbinsX();j++){
      fhTemplRefl->SetBinContent(j,f->Integral(fhTemplRefl->GetBinLowEdge(j),fhTemplRefl->GetXaxis()->GetBinUpEdge(j))/fhTemplRefl->GetBinWidth(j));
      if(fhTemplRefl->GetBinContent(j)>=0.&&TMath::Abs(h->GetBinError(j)*h->GetBinError(j)-h->GetBinContent(j))>0.1*h->GetBinContent(j))isPoissErr=kFALSE;
    }
    for(Int_t j=1;j<=fhTemplRefl->GetNbinsX();j++){
      if(isPoissErr){
	fhTemplRefl->SetBinError(j,TMath::Sqrt(fhTemplRefl->GetBinContent(j)));
      }
      else fhTemplRefl->SetBinError(j,0.001*fhTemplRefl->GetBinContent(j));
    }
    return fhTemplRefl;
  }


 if(opt.EqualTo("2gaus")||opt.EqualTo("doublegaus")){
   if(minRange>=0&&maxRange>=0){
     f=new TF1("my2gaus","[0]*([3]/( TMath::Sqrt(2.*TMath::Pi())*[2])*TMath::Exp(-(x-[1])*(x-[1])/(2.*[2]*[2]))+(1.-[3])/( TMath::Sqrt(2.*TMath::Pi())*[5])*TMath::Exp(-(x-[4])*(x-[4])/(2.*[5]*[5])))",TMath::Max(minRange,h->GetBinLowEdge(1)),TMath::Min(maxRange,h->GetXaxis()->GetBinUpEdge(h->GetNbinsX())));
   }
   else f=new TF1("my2gaus","[0]*([3]/( TMath::Sqrt(2.*TMath::Pi())*[2])*TMath::Exp(-(x-[1])*(x-[1])/(2.*[2]*[2]))+(1.-[3])/( TMath::Sqrt(2.*TMath::Pi())*[5])*TMath::Exp(-(x-[4])*(x-[4])/(2.*[5]*[5])))",h->GetBinLowEdge(1),h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));

    f->SetParameter(0,h->GetMaximum());
    //    f->SetParLimits(0,0,100.*h->Integral());
    f->SetParLimits(3,0.,1.);
    f->SetParameter(3,0.5);

    f->SetParameter(1,1.84);
    f->SetParameter(2,0.050);

    f->SetParameter(4,1.88);
    f->SetParameter(5,0.050);

    fhTemplRefl->Fit(f,"RELMI","");//,h->GetBinLowEdge(1),h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
    for(Int_t j=1;j<=fhTemplRefl->GetNbinsX();j++){
      fhTemplRefl->SetBinContent(j,f->Integral(fhTemplRefl->GetBinLowEdge(j),fhTemplRefl->GetXaxis()->GetBinUpEdge(j))/fhTemplRefl->GetBinWidth(j));
      if(fhTemplRefl->GetBinContent(j)>=0.&&TMath::Abs(h->GetBinError(j)*h->GetBinError(j)-h->GetBinContent(j))>0.1*h->GetBinContent(j))isPoissErr=kFALSE;
    }
    for(Int_t j=1;j<=fhTemplRefl->GetNbinsX();j++){
      if(isPoissErr){
	fhTemplRefl->SetBinError(j,TMath::Sqrt(fhTemplRefl->GetBinContent(j)));
      }
      else fhTemplRefl->SetBinError(j,0.001*fhTemplRefl->GetBinContent(j));
    }
    return fhTemplRefl;
  }


  if(opt.EqualTo("pol3")||opt.EqualTo("POL3")){
    if(minRange>=0&&maxRange>=0){
      f=new TF1("mypol3","pol3",TMath::Max(minRange,h->GetBinLowEdge(1)),TMath::Min(maxRange,h->GetXaxis()->GetBinUpEdge(h->GetNbinsX())));
    }
    else f=new TF1("mypol3","pol3",h->GetBinLowEdge(1),h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
    f->SetParameter(0,h->GetMaximum());
    
    //    f->SetParLimits(0,0,100.*h->Integral());
    // Hard to initialize the other parameters...
    for(Int_t nf=0;nf<10;nf++){
      fhTemplRefl->Fit(f,"RELMI","");
      //,h->GetBinLowEdge(1),h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
    }
    //    Printf("We USED %d POINTS in the Fit",f->GetNumberFitPoints());
    for(Int_t j=1;j<=fhTemplRefl->GetNbinsX();j++){
      fhTemplRefl->SetBinContent(j,f->Integral(fhTemplRefl->GetBinLowEdge(j),fhTemplRefl->GetXaxis()->GetBinUpEdge(j))/fhTemplRefl->GetBinWidth(j));
      if(fhTemplRefl->GetBinContent(j)>=0.&&TMath::Abs(h->GetBinError(j)*h->GetBinError(j)-h->GetBinContent(j))>0.1*h->GetBinContent(j))isPoissErr=kFALSE;
    }
    for(Int_t j=1;j<=fhTemplRefl->GetNbinsX();j++){
      if(isPoissErr){
	fhTemplRefl->SetBinError(j,TMath::Sqrt(fhTemplRefl->GetBinContent(j)));
      }
      else fhTemplRefl->SetBinError(j,0.001*fhTemplRefl->GetBinContent(j));
    }
    return fhTemplRefl;
  }


  if(opt.EqualTo("pol6")||opt.EqualTo("POL6")){

    if(minRange>=0&&maxRange>=0){
      f=new TF1("mypol6","pol6",TMath::Max(minRange,h->GetBinLowEdge(1)),TMath::Min(maxRange,h->GetXaxis()->GetBinUpEdge(h->GetNbinsX())));
    }
    else f=new TF1("mypol6","pol6",h->GetBinLowEdge(1),h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
    f->SetParameter(0,h->GetMaximum());
    //    f->SetParLimits(0,0,100.*h->Integral());
    // Hard to initialize the other parameters...

    fhTemplRefl->Fit(f,"RLEMI","");//,h->GetBinLowEdge(1),h->GetXaxis()->GetBinUpEdge(h->GetNbinsX()));
    
   
    for(Int_t j=1;j<=fhTemplRefl->GetNbinsX();j++){
      fhTemplRefl->SetBinContent(j,f->Integral(fhTemplRefl->GetBinLowEdge(j),fhTemplRefl->GetXaxis()->GetBinUpEdge(j))/fhTemplRefl->GetBinWidth(j));
      if(fhTemplRefl->GetBinContent(j)>=0.&&TMath::Abs(h->GetBinError(j)*h->GetBinError(j)-h->GetBinContent(j))>0.1*h->GetBinContent(j))isPoissErr=kFALSE;
    }
    for(Int_t j=1;j<=fhTemplRefl->GetNbinsX();j++){
      if(isPoissErr){
	fhTemplRefl->SetBinError(j,TMath::Sqrt(fhTemplRefl->GetBinContent(j)));
      }
      else fhTemplRefl->SetBinError(j,0.001*fhTemplRefl->GetBinContent(j));
    }
    return fhTemplRefl;
  }

  // no good option passed
  Printf("AlliHFMassFitterVAR::SetTemplateReflections:  bad option");
  return 0x0;


}


//___________________________________________________________________________

Double_t AliHFMassFitterVAR::FitFunction4MassDistr (Double_t *x, Double_t *par){
  // Fit function for signal+background

  //exponential or linear fit
  //
  // par[0] = tot integral
  // par[1] = slope
  // par[2] = gaussian integral
  // par[3] = gaussian mean
  // par[4] = gaussian sigma
  
  Double_t bkg=0,sgn=0,refl=0.;
  Double_t parbkg[fNparBack];
  sgn = FitFunction4Sgn(x,&par[fNparBack]);  
  
  if(ftypeOfFit4Sgn==2)  parbkg[0]=par[0]-par[fNparBack]-par[fNparBack+fNparSignal]*par[fNparBack];
  else  parbkg[0]=par[0]-par[fNparBack];
  for(Int_t jb=1;jb<fNparBack;jb++){
    parbkg[jb]=par[jb];
  }  
  bkg = FitFunction4Bkg(x,parbkg);


  if(ftypeOfFit4Sgn==2)refl=FitFunction4Refl(x,&par[fNparBack+fNparSignal]);
  
  return sgn+bkg+refl;


}

//_________________________________________________________________________
Double_t AliHFMassFitterVAR::FitFunction4Sgn (Double_t *x, Double_t *par){
  // Fit function for the signal

  //gaussian = A/(sigma*sqrt(2*pi))*exp(-(x-mean)^2/2/sigma^2)
  //Par:
  // * [0] = integralSgn
  // * [1] = mean
  // * [2] = sigma
  //gaussian = [0]/TMath::Sqrt(2.*TMath::Pi())/[2]*exp[-(x-[1])*(x-[1])/(2*[2]*[2])]
  fRawYieldHelp=par[0]/fhistoInvMass->GetBinWidth(1);
  return par[0]/TMath::Sqrt(2.*TMath::Pi())/par[2]*TMath::Exp(-(x[0]-par[1])*(x[0]-par[1])/2./par[2]/par[2]);

}
/////___________________________________________
Double_t AliHFMassFitterVAR::FitFunction4BkgAndReflDraw(Double_t *x, Double_t *par){
  if(ftypeOfFit4Sgn!=2){
    Printf("FitFunction4BkgAndRefl called but w/o reasons: this is just for drawing and requires parameters to be set from outside!!!");
  }
  // The following lines useful for debugging:
  //   Double_t parbkg[fNparBack];
  //   parbkg[0]=par[0]-fRawYieldHelp-par[fNparBack]*fRawYieldHelp;  
  //   for(Int_t jb=1;jb<fNparBack;jb++){
  //     parbkg[jb]=par[jb];    
  //   }
  //  Printf("Raw Yield: %f Func For Refl at x=%f: %f",fRawYieldHelp,x[0],FitFunction4Refl(x,&par[fNparBack]));
  return FitFunction4Bkg(x,par)+FitFunction4Refl(x,&par[fNparBack]);

}
/////_______________________________________________________________________
Double_t AliHFMassFitterVAR::FitFunction4Refl(Double_t *x,Double_t *par){
  
  Int_t bin =fhTemplRefl->FindBin(x[0]);
  Double_t value=fhTemplRefl->GetBinContent(bin);
  Int_t binmin=fhTemplRefl->FindBin(fminMass*1.00001);
  Int_t binmax=fhTemplRefl->FindBin(fmaxMass*0.99999);
  Double_t norm=fhTemplRefl->Integral(binmin,binmax)*fhTemplRefl->GetBinWidth(bin);
  if(TMath::Abs(value)<1.e-14&&fSmoothRefl){// very rough, assume a constant trend, much better would be a pol1 or pol2 over a broader range
    value+=fhTemplRefl->GetBinContent(bin-1)+fhTemplRefl->GetBinContent(bin+1);
    value/=3.;
  }
  return par[0]*value/norm*fRawYieldHelp*fhistoInvMass->GetBinWidth(1);
  
}

//__________________________________________________________________________

Double_t AliHFMassFitterVAR::FitFunction4Bkg(Double_t *x, Double_t *par){
  // Fit function for the background

  Double_t maxDeltaM = 4.*fSigmaSgn;
  if(fSideBands && TMath::Abs(x[0]-fMass) < maxDeltaM) {
    TF1::RejectPoint();
    return 0;
  }

  Int_t firstPar=0;
  Double_t gaus2=0,total=-1;
  if(ftypeOfFit4Sgn == 1){
    firstPar=3;
    //gaussian = A/(sigma*sqrt(2*pi))*exp(-(x-mean)^2/2/sigma^2)
    //Par:
    // * [0] = integralSgn
    // * [1] = mean
    // * [2] = sigma
    //gaussian = [0]/TMath::Sqrt(2.*TMath::Pi())/[2]*exp[-(x-[1])*(x-[1])/(2*[2]*[2])]
    gaus2 = FitFunction4Sgn(x,par);
  }

  switch (ftypeOfFit4Bkg){
  case 0:
    //exponential
    //exponential = A*exp(B*x) -> integral(exponential)=A/B*exp(B*x)](min,max)
    //-> A = B*integral/(exp(B*max)-exp(B*min)) where integral can be written
    //as integralTot- integralGaus (=par [2])
    //Par:
    // * [0] = integralBkg;
    // * [1] = B;
    //exponential = [1]*[0]/(exp([1]*max)-exp([1]*min))*exp([1]*x)
    total = par[0+firstPar]*par[1+firstPar]/(TMath::Exp(par[1+firstPar]*fmaxMass)-TMath::Exp(par[1+firstPar]*fminMass))*TMath::Exp(par[1+firstPar]*x[0]);
    break;
  case 1:
    //linear
    //y=a+b*x -> integral = a(max-min)+1/2*b*(max^2-min^2) -> a = (integral-1/2*b*(max^2-min^2))/(max-min)=integral/(max-min)-1/2*b*(max+min)
    // * [0] = integralBkg;
    // * [1] = b;
    total= par[0+firstPar]/(fmaxMass-fminMass)+par[1+firstPar]*(x[0]-0.5*(fmaxMass+fminMass));
    break;
  case 2:
    //polynomial
    //y=a+b*x+c*x**2 -> integral = a(max-min) + 1/2*b*(max^2-min^2) +
    //+ 1/3*c*(max^3-min^3) -> 
    //a = (integral-1/2*b*(max^2-min^2)-1/3*c*(max^3-min^3))/(max-min)
    // * [0] = integralBkg;
    // * [1] = b;
    // * [2] = c;
    total = par[0+firstPar]/(fmaxMass-fminMass)+par[1+firstPar]*(x[0]-0.5*(fmaxMass+fminMass))+par[2+firstPar]*(x[0]*x[0]-1/3.*(fmaxMass*fmaxMass*fmaxMass-fminMass*fminMass*fminMass)/(fmaxMass-fminMass));
    break;
  case 3:
    total=par[0+firstPar];
    break;
  case 4:  
    //power function 
    //y=a(x-m_pi)^b -> integral = a/(b+1)*((max-m_pi)^(b+1)-(min-m_pi)^(b+1))
    //
    //a = integral*(b+1)/((max-m_pi)^(b+1)-(min-m_pi)^(b+1))
    // * [0] = integralBkg;
    // * [1] = b;
    // a(power function) = [0]*([1]+1)/((max-m_pi)^([1]+1)-(min-m_pi)^([1]+1))*(x-m_pi)^[1]
    {
    Double_t mpi = TDatabasePDG::Instance()->GetParticle(211)->Mass();

    total = par[0+firstPar]*(par[1+firstPar]+1.)/(TMath::Power(fmaxMass-mpi,par[1+firstPar]+1.)-TMath::Power(fminMass-mpi,par[1+firstPar]+1.))*TMath::Power(x[0]-mpi,par[1+firstPar]);
    }
    break;
  case 5:
   //power function wit exponential
    //y=a*Sqrt(x-m_pi)*exp(-b*(x-m_pi))  
    { 
    Double_t mpi = TDatabasePDG::Instance()->GetParticle(211)->Mass();

    total = par[1+firstPar]*TMath::Sqrt(x[0] - mpi)*TMath::Exp(-1.*par[2+firstPar]*(x[0]-mpi));
    } 
    break;
//   default:
//     Types of Fit Functions for Background:
//     * 0 = exponential;
//     * 1 = linear;
//     * 2 = polynomial 2nd order
//     * 3 = no background"<<endl;
//     * 4 = Power function 
//     * 5 = Power function with exponential 

  }
  return total+gaus2;
}

//__________________________________________________________________________
Bool_t AliHFMassFitterVAR::SideBandsBounds(){

  //determines the ranges of the side bands

  if (fNbin==0) fNbin=fhistoInvMass->GetNbinsX();
  Double_t minHisto=fhistoInvMass->GetBinLowEdge(1);
  Double_t maxHisto=fhistoInvMass->GetBinLowEdge(fNbin+1);

  Double_t sidebandldouble,sidebandrdouble;
  Bool_t leftok=kFALSE, rightok=kFALSE;

  if(fMass-fminMass < 0 || fmaxMass-fMass <0) {
    cout<<"Left limit of range > mean or right limit of range < mean: change left/right limit or initial mean value"<<endl;
    return kFALSE;
  } 
  
  //histo limit = fit function limit
  if((TMath::Abs(fminMass-minHisto) < 1e6 || TMath::Abs(fmaxMass - maxHisto) < 1e6) && (fMass-4.*fSigmaSgn-fminMass) < 1e6){
    Double_t coeff = (fMass-fminMass)/fSigmaSgn;
    sidebandldouble=(fMass-0.5*coeff*fSigmaSgn);
    sidebandrdouble=(fMass+0.5*coeff*fSigmaSgn);
    cout<<"Changed number of sigma from 4 to "<<0.5*coeff<<" for the estimation of the side bands"<<endl;
    if (coeff<3) cout<<"Side bands inside 3 sigma, may be better use ftypeOfFit4Bkg = 3 (only signal)"<<endl;
    if (coeff<2) {
      cout<<"Side bands inside 2 sigma. Change mode: ftypeOfFit4Bkg = 3"<<endl;
      ftypeOfFit4Bkg=3;
      //set binleft and right without considering SetRangeFit- anyway no bkg!
      sidebandldouble=(fMass-4.*fSigmaSgn);
      sidebandrdouble=(fMass+4.*fSigmaSgn);
    }
  }
  else {
    sidebandldouble=(fMass-4.*fSigmaSgn);
    sidebandrdouble=(fMass+4.*fSigmaSgn);
  }

  cout<<"Left side band ";
  Double_t tmp=0.;
  tmp=sidebandldouble;
  //calculate bin corresponding to fSideBandl
  fSideBandl=fhistoInvMass->FindBin(sidebandldouble);
  if (sidebandldouble >= fhistoInvMass->GetBinCenter(fSideBandl)) fSideBandl++;
  sidebandldouble=fhistoInvMass->GetBinLowEdge(fSideBandl);
  
  if(TMath::Abs(tmp-sidebandldouble) > 1e-6){
    cout<<tmp<<" is not allowed, changing it to the nearest value allowed: ";
    leftok=kTRUE;
  }
  cout<<sidebandldouble<<" (bin "<<fSideBandl<<")"<<endl;

  cout<<"Right side band ";
  tmp=sidebandrdouble;
  //calculate bin corresponding to fSideBandr
  fSideBandr=fhistoInvMass->FindBin(sidebandrdouble);
  if (sidebandrdouble < fhistoInvMass->GetBinCenter(fSideBandr)) fSideBandr--;
  sidebandrdouble=fhistoInvMass->GetBinLowEdge(fSideBandr+1);

  if(TMath::Abs(tmp-sidebandrdouble) > 1e-6){
    cout<<tmp<<" is not allowed, changing it to the nearest value allowed: ";
    rightok=kTRUE;
  }
  cout<<sidebandrdouble<<" (bin "<<fSideBandr<<")"<<endl;
  if (fSideBandl==0 || fSideBandr==fNbin) {
    cout<<"Error! Range too little"; 
    return kFALSE;
  }
  return kTRUE;
}

//__________________________________________________________________________
Bool_t AliHFMassFitterVAR::CheckRangeFit(){
  //check if the limit of the range correspond to the limit of bins. If not reset the limit to the nearer value which satisfy this condition

  if (!fhistoInvMass) {
    cout<<"No histogram to fit! SetHisto(TH1F* h) before! "<<endl;
    return kFALSE;
  }
  Bool_t leftok=kFALSE, rightok=kFALSE;
  Int_t nbins=fhistoInvMass->GetNbinsX();
  Double_t minhisto=fhistoInvMass->GetBinLowEdge(1), maxhisto=fhistoInvMass->GetBinLowEdge(nbins+1);

  //check if limits are inside histogram range

  if( fminMass-minhisto < 0. ) {
    cout<<"Out of histogram left bound! Setting to "<<minhisto<<endl;
    fminMass=minhisto;
  }
  if( fmaxMass-maxhisto > 0. ) {
    cout<<"Out of histogram right bound! Setting to"<<maxhisto<<endl;
    fmaxMass=maxhisto;
  }

  Double_t tmp=0.;
  tmp=fminMass;
  //calculate bin corresponding to fminMass
  fminBinMass=fhistoInvMass->FindBin(fminMass);
  if (fminMass >= fhistoInvMass->GetBinCenter(fminBinMass)) fminBinMass++;
  fminMass=fhistoInvMass->GetBinLowEdge(fminBinMass);
  if(TMath::Abs(tmp-fminMass) > 1e-6){
    cout<<"Left bound "<<tmp<<" is not allowed, changing it to the nearest value allowed: "<<fminMass<<endl;
    leftok=kTRUE;
  }
 
  tmp=fmaxMass;
  //calculate bin corresponding to fmaxMass
  fmaxBinMass=fhistoInvMass->FindBin(fmaxMass);
  if (fmaxMass < fhistoInvMass->GetBinCenter(fmaxBinMass)) fmaxBinMass--;
  fmaxMass=fhistoInvMass->GetBinLowEdge(fmaxBinMass+1);
  if(TMath::Abs(tmp-fmaxMass) > 1e-6){
    cout<<"Right bound "<<tmp<<" is not allowed, changing it to the nearest value allowed: "<<fmaxMass<<endl;
    rightok=kTRUE;
  }

  return (leftok && rightok);
 
}


//__________________________________________________________________________

Bool_t AliHFMassFitterVAR::MassFitter(Bool_t draw){  
  // Main method of the class: performs the fit of the histogram
  
  //Set default fitter Minuit in order to use gMinuit in the contour plots    
  TVirtualFitter::SetDefaultFitter("Minuit");

  Bool_t isBkgOnly=kFALSE;
  Double_t slope1=-1,slope2=1,slope3=1;
  Double_t diffUnderBands=0;
  
  Int_t fit1status=RefitWithBkgOnly(kFALSE);
  if(fit1status){
    Int_t checkinnsigma=4;
    Double_t range[2]={fMass-checkinnsigma*fSigmaSgn,fMass+checkinnsigma*fSigmaSgn};
    TF1* func=GetHistoClone()->GetFunction("funcbkgonly");
    Double_t intUnderFunc=func->Integral(range[0],range[1]);
    Double_t intUnderHisto=fhistoInvMass->Integral(fhistoInvMass->FindBin(range[0]),fhistoInvMass->FindBin(range[1]),"width");
    cout<<"Pick zone: IntFunc = "<<intUnderFunc<<"; IntHist = "<<intUnderHisto<<"\tDiff = "<<intUnderHisto-intUnderFunc<<"\tRelDiff = "<<(intUnderHisto-intUnderFunc)/intUnderFunc<<endl;
    Double_t diffUnderPick=(intUnderHisto-intUnderFunc);
    intUnderFunc=func->Integral(fminMass,fminMass+checkinnsigma*fSigmaSgn);
    intUnderHisto=fhistoInvMass->Integral(fhistoInvMass->FindBin(fminMass),fhistoInvMass->FindBin(fminMass+checkinnsigma*fSigmaSgn),"width");
    cout<<"Band (l) zone: IntFunc = "<<intUnderFunc<<"; IntHist = "<<intUnderHisto<<"\tDiff = "<<intUnderHisto-intUnderFunc<<"\tRelDiff = "<<(intUnderHisto-intUnderFunc)/intUnderFunc<<endl;
    diffUnderBands=(intUnderHisto-intUnderFunc);
    Double_t relDiff=diffUnderPick/diffUnderBands;
    cout<<"Relative difference = "<<relDiff<<endl;
    if(TMath::Abs(relDiff) < 0.25) isBkgOnly=kTRUE;
    else{
      cout<<"Relative difference = "<<relDiff<<": I suppose there is some signal, continue with total fit!"<<endl;
    }
  }
  if(isBkgOnly) {
    cout<<"INFO!! The histogram contains only background"<<endl;
    if(draw)DrawFit();

    //increase counter of number of fits done
    fcounter++;

    return kTRUE;
  }


  //function names
  TString bkgname="funcbkg";
  TString bkg1name="funcbkg1";
  TString massname="funcmass";

  //Total integral
  Double_t totInt = fhistoInvMass->Integral(fminBinMass,fmaxBinMass,"width");
  //cout<<"Here tot integral is = "<<totInt<<"; integral in whole range is "<<fhistoInvMass->Integral("width")<<endl;
  fSideBands = kTRUE;
  Double_t width=fhistoInvMass->GetBinWidth(8);
  //cout<<"fNbin = "<<fNbin<<endl;
  if (fNbin==0) fNbin=fhistoInvMass->GetNbinsX();

  Bool_t ok=SideBandsBounds();
  if(!ok) return kFALSE;
  
  //sidebands integral - first approx (from histo)
  Double_t sideBandsInt=(Double_t)fhistoInvMass->Integral(1,fSideBandl,"width") + (Double_t)fhistoInvMass->Integral(fSideBandr,fNbin,"width");
  cout<<"------nbin = "<<fNbin<<"\twidth = "<<width<<"\tbinleft = "<<fSideBandl<<"\tbinright = "<<fSideBandr<<endl;
  cout<<"------sideBandsInt - first approx = "<<sideBandsInt<<endl;
  if (sideBandsInt<=0) {
    cout<<"! sideBandsInt <=0. There's a problem, cannot start the fit"<<endl;
    return kFALSE;
  }
  
  /*Fit Bkg*/
  TF1 *funcbkg = new TF1(bkgname.Data(),this,&AliHFMassFitter::FitFunction4Bkg,fminMass,fmaxMass,fNparBack,"AliHFMassFitter","FitFunction4Bkg");
  //   TF1 *funcbkg = GetBackgroundFunc();
  cout<<"Function name = "<<funcbkg->GetName()<<endl<<endl;
  
  funcbkg->SetLineColor(2); //red
  

  //   cout<<"\nBACKGROUND FIT - only combinatorial"<<endl;
  Int_t ftypeOfFit4SgnBkp=ftypeOfFit4Sgn;
  Double_t parBackInit[3]={totInt,slope1,slope2};
  for(Int_t j=0;j<fNparBack;j++){
    //    Printf(" HERE back %d",j);
    funcbkg->SetParName(j,fBackParNames[j]);
    funcbkg->SetParameter(j,parBackInit[j]);
    if(fFixParBackExternalValue[j]){
      funcbkg->FixParameter(j,fparBackFixExt[j]);
    }
    else if(fFixParBack[j]){
      funcbkg->FixParameter(j,parBackInit[j]);
    }
  }

   Double_t intbkg1=0,conc1=0;
  //if only signal and reflection: skip
  if (!(ftypeOfFit4Bkg==3 && ftypeOfFit4Sgn==1)) {
    //    ftypeOfFit4Sgn=0;
    fhistoInvMass->Fit(bkgname.Data(),"R,L,E,0");
   
    fSideBands = kFALSE;
    //intbkg1 = funcbkg->GetParameter(0);

    intbkg1 = funcbkg->Integral(fminMass,fmaxMass);
    if(ftypeOfFit4Bkg!=3) slope1 = funcbkg->GetParameter(1);
    if(ftypeOfFit4Bkg==2) conc1 = funcbkg->GetParameter(2);
    if(ftypeOfFit4Bkg==5) conc1 = funcbkg->GetParameter(2); 
 

    //cout<<"First fit: \nintbkg1 = "<<intbkg1<<"\t(Compare with par0 = "<<funcbkg->GetParameter(0)<<")\nslope1= "<<slope1<<"\nconc1 = "<<conc1<<endl;
  } 
  else cout<<"\t\t//"<<endl;





  //sidebands integral - second approx (from fit)
  fSideBands = kFALSE;

  Double_t bkgInt;
  //cout<<"Compare intbkg1 = "<<intbkg1<<" and integral = ";

  if(ftypeOfFit4Sgn == 1) bkgInt=funcbkg->Integral(fminMass,fmaxMass);
  else bkgInt=funcbkg->Integral(fminMass,fmaxMass);
  //cout<</*"------BkgInt(Fit) = "<<*/bkgInt<<endl;

  //Signal integral - first approx
  Double_t sgnInt=diffUnderBands;
  sgnInt = totInt-bkgInt;
  Printf("Estimates: bkgInt: %f, totInt: %f, diffUndBan:%f",bkgInt,totInt,diffUnderBands);
  //cout<<"------TotInt = "<<totInt<<"\tsgnInt = "<<sgnInt<<endl;
  /*Fit All Mass distribution with exponential + gaussian (+gaussian braodened) */
  
  //  Double_t sgnInt=diffUnderBands;
  if(sgnInt<0)sgnInt=0.1*totInt;
  TF1 *funcmass = new TF1(massname.Data(),this,&AliHFMassFitterVAR::FitFunction4MassDistr,fminMass,fmaxMass,fNFinalPars,"AliHFMassFitterVAR","FitFunction4MassDistr");
  cout<<"Function name = "<<funcmass->GetName()<<endl<<endl;
  funcmass->SetLineColor(4); //blue

  //Set parameters
  cout<<"\nTOTAL FIT"<<endl;

  Double_t parSignInit[3]={sgnInt,fMass,fSigmaSgn};  
  Double_t parReflInit[2]={fReflInit,0.};
  fRawYieldHelp=sgnInt;
  Printf("Initial parameters: \n back: tot int: %f, slope1:%f, slope2:%f \n sign: sgnInt %f, mean %f, sigma %f",totInt,slope1,slope2,sgnInt,fMass,fSigmaSgn);
  for(Int_t j=0;j<fNparBack;j++){

    funcmass->SetParName(j,fBackParNames[j]);
    funcmass->SetParameter(j,parBackInit[j]);
    if(fFixParBackExternalValue[j]){
      funcmass->FixParameter(j,fparBackFixExt[j]);
    }
    else if(fFixParBack[j]){
      funcmass->FixParameter(j,parBackInit[j]);
    }
  }
  for(Int_t j=0;j<fNparSignal;j++){

    funcmass->SetParName(j+fNparBack,fSignParNames[j]);
    funcmass->SetParameter(j+fNparBack,parSignInit[j]);
    if(fFixParSignExternalValue[j]){
      funcmass->FixParameter(j+fNparBack,fparSignFixExt[j]);
    }
    else if(fFixParSign[j]){
      funcmass->FixParameter(j+fNparBack,parSignInit[j]);
    }

  }
  for(Int_t j=0;j<fNparRefl;j++){

    funcmass->SetParName(j+fNparBack+fNparSignal,fReflParNames[j]);
    funcmass->SetParameter(j+fNparBack+fNparSignal,parReflInit[j]);
    funcmass->SetParLimits(j+fNparBack+fNparSignal,0.,1.);
    if(fFixParReflExternalValue[j]){
      funcmass->FixParameter(j+fNparBack+fNparSignal,fparReflFixExt[j]);
    }
    else if(fFixParRefl[j]){
      funcmass->FixParameter(j+fNparBack+fNparSignal,parReflInit[j]);
    }

  }

  Int_t status;
  Printf("Fitting");
  status = fhistoInvMass->Fit(massname.Data(),"R,L,E,+,0");
  if (status != 0){
    cout<<"Minuit returned "<<status<<endl;
    return kFALSE;
  }
  Printf("Fitted");

  cout<<"fit done"<<endl;
  //reset value of fMass and fSigmaSgn to those found from fit
  fMass=funcmass->GetParameter(fNparBack+1);
  fMassErr=funcmass->GetParError(fNparBack+1);
  fSigmaSgn=funcmass->GetParameter(fNparBack+2);
  fSigmaSgnErr=funcmass->GetParError(fNparBack+2);
  fRawYield=funcmass->GetParameter(fNparBack)/fhistoInvMass->GetBinWidth(1);
  fRawYieldHelp=fRawYield;
  fRawYieldErr=funcmass->GetParError(fNparBack)/fhistoInvMass->GetBinWidth(1);

  // The following lines can be useful for debugging
  //   for(Int_t i=0;i<fNFinalPars;i++){
  //     fFitPars[i+2*bkgPar-3]=funcmass->GetParameter(i);
  //     fFitPars[fNFinalPars+4*bkgPar-6+i]= funcmass->GetParError(i);
  //     //cout<<i+2*bkgPar-3<<"\t"<<funcmass->GetParameter(i)<<"\t\t"<<fNFinalPars+4*bkgPar-6+i<<"\t"<<funcmass->GetParError(i)<<endl;
  //   }
  //   /*
  //   //check: cout parameters  
  //   for(Int_t i=0;i<2*(fNFinalPars+2*bkgPar-3);i++){
  //     cout<<i<<"\t"<<fFitPars[i]<<endl;
  //     }
  //   */
  
  if(funcmass->GetParameter(fNparBack+2) <0 || funcmass->GetParameter(fNparBack+1) <0 || funcmass->GetParameter(fNparBack) <0 ) {
    cout<<"IntS or mean or sigma negative. You may tray to SetInitialGaussianSigma(..) and SetInitialGaussianMean(..)"<<endl;
    return kFALSE;
  }
  
  //increase counter of number of fits done
  fcounter++;

  
  delete funcbkg;
  delete funcmass;
  
  Printf("Now calling add functions to hist in MassFitter");
  AddFunctionsToHisto();
  Printf("After calling add functions to hist in MassFitter");
  if (draw) DrawFit();
 
  
  return kTRUE;
}

//______________________________________________________________________________

Bool_t AliHFMassFitterVAR::RefitWithBkgOnly(Bool_t draw){

  //perform a fit with background function only. Can be useful to try when fit fails to understand if it is because there's no signal
  //If you want to change the backgroud function or range use SetType or SetRangeFit before

  TString bkgname="funcbkgonly";
  fSideBands = kFALSE;
  Int_t typesSave=ftypeOfFit4Sgn;
  if(ftypeOfFit4Sgn==2)ftypeOfFit4Sgn=0;
  TF1* funcbkg = new TF1(bkgname.Data(),this,&AliHFMassFitterVAR::FitFunction4Bkg,fminMass,fmaxMass,fNparBack,"AliHFMassFitterVAR","FitFunction4Bkg");

  funcbkg->SetLineColor(kBlue+3); //dark blue

  Double_t integral=fhistoInvMass->Integral(fhistoInvMass->FindBin(fminMass),fhistoInvMass->FindBin(fmaxMass),"width");

  switch (ftypeOfFit4Bkg) {
  case 0: //gaus+expo
    funcbkg->SetParNames("BkgInt","Slope"); 
    funcbkg->SetParameters(integral,-2.); 
    break;
  case 1:
    funcbkg->SetParNames("BkgInt","Slope");
    funcbkg->SetParameters(integral,-100.); 
    break;
  case 2:
    funcbkg->SetParNames("BkgInt","Coef1","Coef2");
    funcbkg->SetParameters(integral,-10.,5);
    break;
  case 3:
    cout<<"Warning! This choice does not make a lot of sense..."<<endl;
    if(ftypeOfFit4Sgn==0){
      funcbkg->SetParNames("Const");
      funcbkg->SetParameter(0,0.);
      funcbkg->FixParameter(0,0.);
    }
    break;
  case 4:     
    funcbkg->SetParNames("BkgInt","Coef1");
    funcbkg->SetParameters(integral,0.5);
    break;
  case 5:    
    funcbkg->SetParNames("BkgInt","Coef1","Coef2");
    funcbkg->SetParameters(integral,-10.,5.);
    break;
  default:
    cout<<"Wrong choise of ftypeOfFit4Bkg ("<<ftypeOfFit4Bkg<<")"<<endl;
    return kFALSE;
    break;
  }


  Int_t status=fhistoInvMass->Fit(bkgname.Data(),"R,L,E,+,0");
  if (status != 0){
    cout<<"Minuit returned "<<status<<endl;
    return kFALSE;
  }
  AddFunctionsToHisto();

  if(draw) DrawFit();
  ftypeOfFit4Sgn=typesSave;
  return kTRUE;

}


//_________________________________________________________________________
void AliHFMassFitterVAR::IntS(Float_t *valuewitherror) const {

  //gives the integral of signal obtained from fit parameters
  if(!valuewitherror) {
    printf("AliHFMassFitterVAR::IntS: got a null pointer\n");
    return;
  }

  //  Int_t index=fParsSize/2 - 3;
  valuewitherror[0]=fRawYield;
  valuewitherror[1]=fRawYieldErr;
}


//_________________________________________________________________________
void AliHFMassFitterVAR::AddFunctionsToHisto(){

  //Add the background function in the complete range to the list of functions attached to the histogram

  //cout<<"AddFunctionsToHisto called"<<endl;
  TString bkgname = "funcbkg";

  Bool_t done1=kFALSE,done2=kFALSE;

  TString bkgnamesave=bkgname;
  TString testname=bkgname;
  testname += "FullRange";
  TF1 *testfunc=(TF1*)fhistoInvMass->FindObject(testname.Data());
  if(testfunc){
    done1=kTRUE;
    testfunc=0x0;
  }
  testname="funcbkgonly";
  testfunc=(TF1*)fhistoInvMass->FindObject(testname.Data());
  if(testfunc){
    done2=kTRUE;
    testfunc=0x0;
  }

  if(done1 && done2){
    cout<<"AddFunctionsToHisto already used: exiting...."<<endl;
    return;
  }

  TList *hlist=fhistoInvMass->GetListOfFunctions();
  hlist->ls();

  if(!done2){
    TF1 *bonly=(TF1*)hlist->FindObject(testname.Data());
    if(!bonly){
      cout<<testname.Data()<<" not found looking for complete fit"<<endl;
    }else{
      bonly->SetLineColor(kBlue+3);
      hlist->Add((TF1*)bonly->Clone());
      delete bonly;
    }

  }

  if(!done1){
    TF1 *b=(TF1*)hlist->FindObject(bkgname.Data());
    if(!b){
      cout<<bkgname<<" not found, cannot produce "<<bkgname<<"FullRange and "<<bkgname<<"Recalc"<<endl;
      return;
    }
  
    bkgname += "FullRange";
    TF1 *bfullrange=new TF1(bkgname.Data(),this,&AliHFMassFitterVAR::FitFunction4Bkg,fminMass,fmaxMass,fNparBack,"AliHFMassFitterVAR","FitFunction4Bkg");
    //cout<<bfullrange->GetName()<<endl;
    for(Int_t i=0;i<fNparBack;i++){
      bfullrange->SetParName(i,b->GetParName(i));
      bfullrange->SetParameter(i,b->GetParameter(i));
      bfullrange->SetParError(i,b->GetParError(i));
    }
    bfullrange->SetLineStyle(4);
    bfullrange->SetLineColor(14);
    
    
    bkgnamesave += "Recalc";

    TF1 *blastpar;
    if(ftypeOfFit4Sgn<2)blastpar=new TF1(bkgnamesave.Data(),this,&AliHFMassFitterVAR::FitFunction4Bkg,fminMass,fmaxMass,fNparBack,"AliHFMassFitterVAR","FitFunction4Bkg");
    else blastpar=new TF1(bkgnamesave.Data(),this,&AliHFMassFitterVAR::FitFunction4BkgAndReflDraw,fminMass,fmaxMass,fNparBack+fNparRefl,"AliHFMassFitterVAR","FitFunction4Bkg");

    TF1 *mass=fhistoInvMass->GetFunction("funcmass");

    if (!mass){
      cout<<"funcmass doesn't exist "<<endl;
      return;
    }

  

    //intBkg=intTot-intS
    blastpar->SetParameter(0,mass->GetParameter(0)-mass->GetParameter(fNparBack));
    blastpar->SetParError(0,mass->GetParError(fNparBack));
    for(Int_t jb=1;jb<fNparBack;jb++){
      blastpar->SetParameter(jb,mass->GetParameter(jb));
      blastpar->SetParError(jb,mass->GetParError(jb));
    }
    if(ftypeOfFit4Sgn==2){
      blastpar->SetParameter(0,mass->GetParameter(0)-mass->GetParameter(fNparBack)-mass->GetParameter(fNparBack+fNparSignal)*mass->GetParameter(fNparBack));
      blastpar->SetParError(0,mass->GetParError(fNparBack));

      blastpar->SetParameter(fNparBack,mass->GetParameter(fNparBack+fNparSignal));
      for(Int_t jr=1;jr<fNparRefl;jr++){
	blastpar->SetParameter(jr+fNparBack,mass->GetParameter(fNparBack+fNparSignal+jr));
	blastpar->SetParError(jr+fNparBack,mass->GetParError(fNparBack+fNparSignal+jr));
      }
    }
    else for(Int_t jr=0;jr<fNparRefl;jr++){
      blastpar->SetParameter(jr+fNparBack,mass->GetParameter(fNparBack+fNparSignal+jr));
      blastpar->SetParError(jr+fNparBack,mass->GetParError(fNparBack+fNparSignal+jr));
    }

    blastpar->SetLineStyle(1);
    blastpar->SetLineColor(2);

    hlist->Add((TF1*)bfullrange->Clone());
    hlist->Add((TF1*)blastpar->Clone());
    hlist->ls();
  
    delete bfullrange;
    delete blastpar;
    
  }


}
//_________________________________________________________________________
void AliHFMassFitterVAR::WriteCanvas(TString userIDstring,TString path,Double_t nsigma,Int_t writeFitInfo,Bool_t draw) const{

 //write the canvas in a root file

  gStyle->SetOptStat(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);

  TString type="";

  switch (ftypeOfFit4Bkg){
  case 0:
    type="Exp"; //3+2
    break;
  case 1:
    type="Lin"; //3+2
    break;
  case 2:
    type="Pl2"; //3+3
    break;
  case 3:
    type="noB"; //3+1
    break;
  case 4:  
    type="Pow"; //3+3
    break;
  case 5:
    type="PowExp"; //3+3
    break;
  }

  TString filename=Form("%sMassFit.root",type.Data());
  filename.Prepend(userIDstring);
  path.Append(filename);

  TFile* outputcv=new TFile(path.Data(),"update");

  TCanvas* c=(TCanvas*)GetPad(nsigma,writeFitInfo);
  c->SetName(Form("%s%s%s",c->GetName(),userIDstring.Data(),type.Data()));
  if(draw)c->DrawClone();
  outputcv->cd();
  c->Write();
  outputcv->Close();
}


//_________________________________________________________________________

void AliHFMassFitterVAR::PlotFit(TVirtualPad* pd,Double_t nsigma,Int_t writeFitInfo)const{
  //plot histogram, fit functions and write parameters according to verbosity level (0,1,>1)
  gStyle->SetOptStat(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetFrameFillColor(0);

  cout<<"nsigma = "<<nsigma<<endl;
  cout<<"Verbosity = "<<writeFitInfo<<endl;

  TH1F* hdraw=GetHistoClone();
  
  if(!hdraw->GetFunction("funcmass") && !hdraw->GetFunction("funcbkgFullRange") && !hdraw->GetFunction("funcbkgRecalc")&& !hdraw->GetFunction("funcbkgonly")){
    cout<<"Probably fit failed and you didn't try to refit with background only, there's no function to be drawn"<<endl;
    return;
  }
 
  if(hdraw->GetFunction("funcbkgonly")){ //Warning! if this function is present, no chance to draw the other!
    cout<<"Drawing background fit only"<<endl;
    hdraw->SetMinimum(0);
    hdraw->GetXaxis()->SetRangeUser(fminMass,fmaxMass);
    pd->cd();
    hdraw->SetMarkerStyle(20);
    hdraw->DrawClone("PE");
    hdraw->GetFunction("funcbkgonly")->DrawClone("sames");

    if(writeFitInfo > 0){
      TPaveText *pinfo=new TPaveText(0.6,0.86,1.,1.,"NDC");     
      pinfo->SetBorderSize(0);
      pinfo->SetFillStyle(0);
      TF1* f=hdraw->GetFunction("funcbkgonly");
      for (Int_t i=0;i<fNparBack;i++){
	pinfo->SetTextColor(kBlue+3);
	TString str=Form("%s = %.3f #pm %.3f",f->GetParName(i),f->GetParameter(i),f->GetParError(i));
	pinfo->AddText(str);
      }

      for (Int_t i=fNparBack;i<fNparBack+fNparSignal;i++){
	pinfo->SetTextColor(kBlue+3);
	TString str=Form("%s = %.3f #pm %.3f",f->GetParName(i),f->GetParameter(i),f->GetParError(i));
	pinfo->AddText(str);
      }

      for (Int_t i=fNparBack+fNparSignal;i<fNparBack+fNparSignal+fNparRefl;i++){
	pinfo->SetTextColor(kBlue+3);
	TString str=Form("%s = %.3f #pm %.3f",f->GetParName(i),f->GetParameter(i),f->GetParError(i));
	pinfo->AddText(str);
      }

      pinfo->AddText(Form("Reduced #chi^{2} = %.3f",f->GetChisquare()/f->GetNDF()));
      pd->cd();
      pinfo->DrawClone();
      
      
    }

    return;
  }
  
  hdraw->SetMinimum(0);
  hdraw->GetXaxis()->SetRangeUser(fminMass,fmaxMass);
  pd->cd();
  hdraw->SetMarkerStyle(20);
  hdraw->DrawClone("PE");
//   if(hdraw->GetFunction("funcbkgFullRange")) hdraw->GetFunction("funcbkgFullRange")->DrawClone("same");
//   if(hdraw->GetFunction("funcbkgRecalc")) hdraw->GetFunction("funcbkgRecalc")->DrawClone("same");
  if(hdraw->GetFunction("funcmass")) hdraw->GetFunction("funcmass")->DrawClone("same");

  if(writeFitInfo > 0){
    TPaveText *pinfob=new TPaveText(0.6,0.86,1.,1.,"NDC");
    TPaveText *pinfom=new TPaveText(0.6,0.7,1.,.87,"NDC");
    pinfob->SetBorderSize(0);
    pinfob->SetFillStyle(0);
    pinfom->SetBorderSize(0);
    pinfom->SetFillStyle(0);
    TF1* ff=fhistoInvMass->GetFunction("funcmass");

    // the following was a previous choice    
    //     for (Int_t i=0;i<fNparBack;i++){
    //       pinfom->SetTextColor(kBlue);
    //       TString str=Form("%s = %.3f #pm %.3f",ff->GetParName(i),ff->GetParameter(i),ff->GetParError(i));
    //       if(!(writeFitInfo==1 && i==fNparBack)) pinfom->AddText(str);
    //     }

    for (Int_t i=fNparBack;i<fNparBack+fNparSignal;i++){
      pinfom->SetTextColor(kBlue);
      TString str=Form("%s = %.3f #pm %.3f",ff->GetParName(i),ff->GetParameter(i),ff->GetParError(i));
      if(!(writeFitInfo==1 && i==fNparBack)) pinfom->AddText(str);
    }
    for (Int_t i=fNparBack+fNparSignal;i<fNparBack+fNparSignal+fNparRefl;i++){
      pinfom->SetTextColor(kBlue+3);
      TString str=Form("%s = %.3f #pm %.3f",ff->GetParName(i),ff->GetParameter(i),ff->GetParError(i));
      pinfom->AddText(str);
    }
    
    pd->cd();
    pinfom->DrawClone();

    TPaveText *pinfo2=new TPaveText(0.1,0.1,0.6,0.4,"NDC");
    pinfo2->SetBorderSize(0);
    pinfo2->SetFillStyle(0);

    Double_t signif, signal, bkg, errsignif, errsignal, errbkg;

    signal=fRawYield;
    errsignal=fRawYieldErr;//    Signal(nsigma,signal,errsignal);
    Background(nsigma,bkg, errbkg);
    AliVertexingHFUtils::ComputeSignificance(signal,errsignal,bkg,errbkg,signif,errsignif);


    TString str=Form("Significance (%.0f#sigma) %.1f #pm %.1f ",nsigma,signif,errsignif);
    pinfo2->AddText(str);
    str=Form("S (%.0f#sigma) %.0f #pm %.0f ",nsigma,signal,errsignal);
    pinfo2->AddText(str);
    str=Form("B (%.0f#sigma) %.0f #pm %.0f",nsigma,bkg,errbkg);
    pinfo2->AddText(str);
    if(bkg>0) str=Form("S/B (%.0f#sigma) %.4f ",nsigma,signal/bkg); 
    pinfo2->AddText(str);

    pd->cd();
    pinfo2->Draw();
    if(writeFitInfo==1){
      TF1 *fitBkgRec=hdraw->GetFunction("funcbkgRecalc");
      fitBkgRec->SetLineColor(14); // does not work
      fitBkgRec->SetLineStyle(2); // does not work
      TF1 *fitCp = new TF1("ciccio",this,&AliHFMassFitter::FitFunction4Bkg,fminMass,fmaxMass,fNparBack,"AliHFMassFitter","FitFunction4Bkg");
      //      TF1 *fitCp=(TF1*)fitBkgRec->Clone("funcbkgRecalcNoRefl");
      fitCp->SetParameter(fNparBack,0);
      for(Int_t ibk=0;ibk<fNparBack;ibk++){
	fitCp->SetParameter(ibk,fitBkgRec->GetParameter(ibk));
      }
      fitCp->SetLineColor(kGreen);
      fitCp->SetLineStyle(9);


      fitCp->Draw("same");
      //Printf("WHERE I SHOULD BE: npars func=%d; par 0=%f, par1=%f,par2=%f",fitCp->GetNpar(),fitCp->GetParameter(0),fitCp->GetParameter(1),fitCp->GetParameter(2));

    }
    
    if(writeFitInfo > 1){
      for (Int_t i=0;i<fNparBack+fNparSignal;i++){
	pinfob->SetTextColor(kRed);
	str=Form("%s = %f #pm %f",ff->GetParName(i),ff->GetParameter(i),ff->GetParError(i));
	pinfob->AddText(str);
      }
      pd->cd();
      pinfob->DrawClone();
    }


  }
  return;
}


//_________________________________________________________________________

void AliHFMassFitterVAR::Signal(Double_t nOfSigma,Double_t &signal,Double_t &errsignal) const {
  // Return signal integral in mean+- n sigma

  if(fcounter==0) {
    cout<<"Use MassFitter method before Signal"<<endl;
    return;
  }

  Double_t min=fMass-nOfSigma*fSigmaSgn;
  Double_t max=fMass+nOfSigma*fSigmaSgn;

  Signal(min,max,signal,errsignal);


  return;

}
//______________________________________________
Double_t AliHFMassFitterVAR::GetReflOverSignal(Double_t &err)const{
  TString massname="funcmass";


  TF1 *funcmass=fhistoInvMass->GetFunction(massname.Data());
  if(!funcmass){
    cout<<"AliHFMassFitterVAR::Signal() ERROR -> Mass distr function not found!"<<endl;
    return -1;
  }
  if(ftypeOfFit4Sgn != 2){
    Printf("The fit was done w/o reflection template");
    return -1;
  }
  err=funcmass->GetParError(fNparBack+fNparSignal);
  return funcmass->GetParameter(fNparBack+fNparSignal);
  
}
//_________________________________________________________________________

void AliHFMassFitterVAR::Signal(Double_t min, Double_t max, Double_t &signal,Double_t &errsignal) const {

  // Return signal integral in a range
  
  if(fcounter==0) {
    cout<<"Use MassFitter method before Signal"<<endl;
    return;
  }

  //functions names
  TString bkgname="funcbkgRecalc";
  TString bkg1name="funcbkg1Recalc";
  TString massname="funcmass";


  TF1 *funcbkg=0;
  TF1 *funcmass=fhistoInvMass->GetFunction(massname.Data());
  if(!funcmass){
    cout<<"AliHFMassFitterVAR::Signal() ERROR -> Mass distr function not found!"<<endl;
    return;
  }

  if(ftypeOfFit4Sgn == 1) funcbkg=fhistoInvMass->GetFunction(bkg1name.Data());
  else funcbkg=fhistoInvMass->GetFunction(bkgname.Data());

  if(!funcbkg){
    cout<<"AliHFMassFitterVAR::Signal() ERROR -> Bkg function not found!"<<endl;
    return;
  }

  Int_t np=fNparBack;

  Double_t intS,intSerr;

  //relative error evaluation
  intS=funcmass->GetParameter(np);
  intSerr=funcmass->GetParError(np);

  cout<<"Sgn relative error evaluation from fit: "<<intSerr/intS<<endl;
  Double_t background,errbackground;
  Background(min,max,background,errbackground);

  //signal +/- error in the range

  Double_t mass=funcmass->Integral(min, max)/fhistoInvMass->GetBinWidth(4);
  signal=mass - background;
  errsignal=(intSerr/intS)*signal;/*assume relative error is the same as for total integral*/

}

//_________________________________________________________________________

void AliHFMassFitterVAR::Background(Double_t nOfSigma,Double_t &background,Double_t &errbackground) const {
  // Return background integral in mean+- n sigma

  if(fcounter==0) {
    cout<<"Use MassFitter method before Background"<<endl;
    return;
  }
  Double_t min=fMass-nOfSigma*fSigmaSgn;
  Double_t max=fMass+nOfSigma*fSigmaSgn;

  Background(min,max,background,errbackground);

  return;
  
}
//___________________________________________________________________________

void AliHFMassFitterVAR::Background(Double_t min, Double_t max, Double_t &background,Double_t &errbackground) const {
  // Return background integral in a range

  if(fcounter==0) {
    cout<<"Use MassFitter method before Background"<<endl;
    return;
  }

  //functions names
  TString bkgname="funcbkgRecalc";
  TString bkg1name="funcbkg1Recalc";

  TF1 *funcbkg=0;
  if(ftypeOfFit4Sgn == 1) funcbkg=fhistoInvMass->GetFunction(bkg1name.Data());
  else funcbkg=fhistoInvMass->GetFunction(bkgname.Data());
  if(!funcbkg){
    cout<<"AliHFMassFitterVAR::Background() ERROR -> Bkg function not found!"<<endl;
    return;
  }


  Double_t intB,intBerr;

  //relative error evaluation: from final parameters of the fit
  if(ftypeOfFit4Bkg==3 && ftypeOfFit4Sgn == 0) cout<<"No background fit: Bkg relative error evaluation put to zero"<<endl;
  else{
    intB=funcbkg->GetParameter(0);
    intBerr=funcbkg->GetParError(0);
    cout<<"Bkg relative error evaluation: from final parameters of the fit: "<<intBerr/intB<<endl;
  }

  //relative error evaluation: from histo
   
  intB=fhistoInvMass->Integral(1,fSideBandl)+fhistoInvMass->Integral(fSideBandr,fNbin);
  Double_t sum2=0;
  for(Int_t i=1;i<=fSideBandl;i++){
    sum2+=fhistoInvMass->GetBinError(i)*fhistoInvMass->GetBinError(i);
  }
  for(Int_t i=fSideBandr;i<=fNbin;i++){
    sum2+=fhistoInvMass->GetBinError(i)*fhistoInvMass->GetBinError(i);
  }

  intBerr=TMath::Sqrt(sum2);
  cout<<"Bkg relative error evaluation: from histo: "<<intBerr/intB<<endl;
  
  cout<<"Last estimation of bkg error is used"<<endl;

  //backround +/- error in the range
  if (ftypeOfFit4Bkg == 3 && ftypeOfFit4Sgn == 0) {
    background = 0;
    errbackground = 0;
  }
  else{
    background=funcbkg->Integral(min,max)/(Double_t)fhistoInvMass->GetBinWidth(2);
    errbackground=intBerr/intB*background; // assume relative error is the same as for total integral
    //cout<<"integral = "<<funcbkg->Integral(min, max)<<"\tbinW = "<<fhistoInvMass->GetBinWidth(2)<<endl;
  }
  return;

}


//__________________________________________________________________________

void AliHFMassFitterVAR::Significance(Double_t nOfSigma,Double_t &significance,Double_t &errsignificance) const  {
  // Return significance in mean+- n sigma
 
  Double_t min=fMass-nOfSigma*fSigmaSgn;
  Double_t max=fMass+nOfSigma*fSigmaSgn;
  Significance(min, max, significance, errsignificance);

  return;
}

//__________________________________________________________________________

void AliHFMassFitterVAR::Significance(Double_t min, Double_t max, Double_t &significance,Double_t &errsignificance) const {
  // Return significance integral in a range

  Double_t signal,errsignal,background,errbackground;
  Signal(min, max,signal,errsignal);
  Background(min, max,background,errbackground);

  if (signal+background <= 0.){
    cout<<"Cannot calculate significance because of div by 0!"<<endl;
    significance=-1;
    errsignificance=0;
    return;
  }

  AliVertexingHFUtils::ComputeSignificance(signal,errsignal,background,errbackground,significance,errsignificance);
  
  return;
}


void AliHFMassFitterVAR::ComputeNFinalPars() {

  //compute the number of parameters of the total (signal+bgk) function
  cout<<"Info:ComputeNFinalPars... ";
  switch (ftypeOfFit4Bkg) {//npar background func
  case 0:
    fNparBack=2;
    fBackParNames=new TString[fNparBack];
    fBackParNames[0]="BkgInt";
    fBackParNames[1]="Slope";
    break;
  case 1:
    fNparBack=2;
    fBackParNames=new TString[fNparBack];
    fBackParNames[0]="BkgInt";
    fBackParNames[1]="Slope";
    break;
  case 2:
    fNparBack=3;
    fBackParNames=new TString[fNparBack];
    fBackParNames[0]="BkgInt";
    fBackParNames[1]="Coef1";
    fBackParNames[2]="Coef2";
    break;
  case 3:
    fNparBack=1;
    fBackParNames=new TString[fNparBack];
    fBackParNames[0]="Const";
    break;
  case 4:
    fNparBack=2;	
    fBackParNames=new TString[fNparBack];
    fBackParNames[0]="BkgInt";
    fBackParNames[1]="Coef1";
    break;
  case 5:
    fNparBack=3;	
    fBackParNames=new TString[fNparBack];
    fBackParNames[0]="BkgInt";
    fBackParNames[1]="Coef1";
    fBackParNames[2]="Coef2";
    break;
  default:
    cout<<"Error in computing fNparBack: check ftypeOfFit4Bkg"<<endl;
    break;
  }
  
  fFixParBack=new Bool_t[fNparBack];
  fFixParBackExternalValue=new Bool_t[fNparBack];
  fparBackFixExt=new Double_t[fNparBack];

  for(Int_t ib=0;ib<fNparBack;ib++){
    fFixParBack[ib]=kFALSE;
    fFixParBackExternalValue[ib]=kFALSE;
    fparBackFixExt[ib]=0.;
  }
  
  fNparSignal=3;
  fSignParNames=new TString[3];
  fSignParNames[0]="Signal";
  fSignParNames[1]="Mean";
  fSignParNames[2]="Sigma";
  
  fFixParSign=new Bool_t[fNparSignal];
  fFixParSignExternalValue=new Bool_t[fNparSignal];
  fparSignFixExt=new Double_t[fNparSignal];

  for(Int_t ib=0;ib<fNparSignal;ib++){
    fFixParSign[ib]=kFALSE;
    fFixParSignExternalValue[ib]=kFALSE;
    fparSignFixExt[ib]=0.;
  }


  fNparRefl=0;
  if(ftypeOfFit4Sgn==2){
    fNparRefl=1;
    fReflParNames=new TString[fNparRefl];
    fReflParNames[0]="ReflOverS";
  }

  fFixParRefl=new Bool_t[fNparRefl];
  fFixParReflExternalValue=new Bool_t[fNparRefl];
  fparReflFixExt=new Double_t[fNparRefl];


  for(Int_t ib=0;ib<fNparRefl;ib++){
    fFixParRefl[ib]=kFALSE;
    fFixParReflExternalValue[ib]=kFALSE;
    fparReflFixExt[ib]=0.;
  }

  fNFinalPars=fNparBack+fNparSignal+fNparRefl;

  cout<<": "<<fNFinalPars<<endl;

  Printf("Total number of par: %d, Back:%d, Sign:%d, Refl: %d",fNFinalPars,fNparBack,fNparSignal,fNparRefl);
}
