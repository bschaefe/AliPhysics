/*************************************************************************************
 *	Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved.	*
 *											*
 *	Author: The ALICE Off-line Project.						*
 *	Contributors are mentioned in the code where appropriate.			*
 *											*
 *	Permission to use, copy, modify and distribute this software and its		*
 *	documentation strictly for non-commercial purposes is hereby granted		*
 *	without fee, provided that the above copyright notice appears in all		*
 *	copies and that both the copyright notice and this permission notice		*
 *	appear in the supporting documentation. The authors make no claims		*
 *	about the suitability of this software for any purpose. It is			*
 *	provided "as is" without express or implied warranty.				*
 *											*
 *************************************************************************************/

/*************************************************************************************
 *											*
 *	Class for the Selection of Non-Heavy-Flavour-Electrons trought		*
 *	the invariant mass method. The selection can be done from two			*
 *	different algorithms, which can be choosed calling the function		*
 *		"SetAlgorithm(TString Algorithm)".	  				*
 *											*
 *		Authors: R.Bailhache, C.A.Schmidt					*
 *											*
 *************************************************************************************/

#include "TVector2.h"
#include "THnSparse.h"
#include "TMath.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TList.h"
#include "TDatabasePDG.h"

#include "AliVEvent.h"
#include "AliMCEvent.h"
#include "AliESDEvent.h"
#include "AliMCParticle.h"
#include "AliAODMCParticle.h"
#include "AliAODEvent.h"
#include "AliAODVertex.h"
#include "AliAODTrack.h"
#include "AliVTrack.h"
#include "AliESDtrack.h"
#include "AliESDtrackCuts.h"
#include "AliPIDResponse.h"
#include "AliPID.h"

#include "AliKFParticle.h"
#include "AliKFVertex.h"

#include "AliHFEcuts.h"
#include "AliHFEpid.h"
#include "AliHFEpidQAmanager.h"
#include "AliHFEtools.h"

#include "AliHFENonPhotonicElectron.h"

ClassImp(AliHFENonPhotonicElectron)
//________________________________________________________________________
AliHFENonPhotonicElectron::AliHFENonPhotonicElectron(const char *name, const Char_t *title)
  :TNamed		(name, title)
  ,fIsAOD		(kFALSE)
  ,fMCEvent		(NULL)
  ,fAODArrayMCInfo	(NULL)
  ,fHFEBackgroundCuts	(NULL)
  ,fPIDBackground	(0x0)
  ,fPIDBackgroundQA	(0)
  ,fkPIDRespons		(NULL)
  ,fAlgorithmMA		(kTRUE)
  ,fChi2OverNDFCut	(3.0)
  ,fMaxDCA		(3.0)
//  ,fMaxOpeningTheta	(0.02)
//  ,fMaxOpeningPhi	(0.1)
  ,fMaxOpening3D	(TMath::Pi())
  ,fMaxInvMass		(1000)
  ,fSetMassConstraint	(kFALSE)
  ,fSelectCategory1tracks(kTRUE)
  ,fSelectCategory2tracks(kFALSE)
  ,fITSmeanShift(0.)
  ,fArraytrack		(NULL)
  ,fCounterPoolBackground	(0)
  ,fnumberfound			(0)
  ,fListOutput		(NULL)
  ,fAssElectron		(NULL)
  ,fIncElectron		(NULL)
  ,fUSign		(NULL)
  ,fLSign		(NULL)
  ,fUSmatches(NULL)
  ,fLSmatches(NULL)
  ,fHnsigmaITS(NULL)
//  ,fUSignAngle	(NULL)
//  ,fLSignAngle	(NULL)
{
  //
  // Constructor
  //
  fPIDBackground   = new AliHFEpid("hfePidBackground");
  fPIDBackgroundQA = new AliHFEpidQAmanager;
}

//________________________________________________________________________
AliHFENonPhotonicElectron::AliHFENonPhotonicElectron()
  :TNamed		()
  ,fIsAOD		(kFALSE)
  ,fMCEvent		(NULL)
  ,fAODArrayMCInfo	(NULL)
  ,fHFEBackgroundCuts	(NULL)
  ,fPIDBackground	(0x0)
  ,fPIDBackgroundQA	(0)
  ,fkPIDRespons		(NULL)
  ,fAlgorithmMA		(kTRUE)
  ,fChi2OverNDFCut	(3.0)
  ,fMaxDCA		(3.0)
//  ,fMaxOpeningTheta	(0.02)
//  ,fMaxOpeningPhi	(0.1)
  ,fMaxOpening3D	(TMath::TwoPi())
  ,fMaxInvMass		(1000)
  ,fSetMassConstraint	(kFALSE)
  ,fSelectCategory1tracks(kTRUE)
  ,fSelectCategory2tracks(kFALSE)
  ,fITSmeanShift(0.)
  ,fArraytrack		(NULL)
  ,fCounterPoolBackground	(0)
  ,fnumberfound			(0)
  ,fListOutput		(NULL)
  ,fAssElectron		(NULL)
  ,fIncElectron		(NULL)
  ,fUSign		(NULL)
  ,fLSign		(NULL)
  ,fUSmatches(NULL)
  ,fLSmatches(NULL)
  ,fHnsigmaITS(NULL)
//  ,fUSignAngle	(NULL)
//  ,fLSignAngle	(NULL)
{
  //
  // Constructor
  //
  fPIDBackground   = new AliHFEpid("hfePidBackground");
  fPIDBackgroundQA = new AliHFEpidQAmanager;
}

//________________________________________________________________________
AliHFENonPhotonicElectron::AliHFENonPhotonicElectron(const AliHFENonPhotonicElectron &ref)
  :TNamed(ref)
  ,fIsAOD		(ref.fIsAOD)
  ,fMCEvent		(NULL)
  ,fAODArrayMCInfo	(NULL)
  ,fHFEBackgroundCuts	(ref.fHFEBackgroundCuts)
  ,fPIDBackground	(ref.fPIDBackground)
  ,fPIDBackgroundQA	(ref.fPIDBackgroundQA)
  ,fkPIDRespons		(ref.fkPIDRespons)
  ,fAlgorithmMA		(ref.fAlgorithmMA)
  ,fChi2OverNDFCut	(ref.fChi2OverNDFCut)
  ,fMaxDCA		(ref.fMaxDCA)
//  ,fMaxOpeningTheta	(ref.fMaxOpeningTheta)
//  ,fMaxOpeningPhi	(ref.fMaxOpeningPhi)
  ,fMaxOpening3D	(ref.fMaxOpening3D)
  ,fMaxInvMass		(ref.fMaxInvMass)
  ,fSetMassConstraint	(ref.fSetMassConstraint)
  ,fSelectCategory1tracks(ref.fSelectCategory1tracks)
  ,fSelectCategory2tracks(ref.fSelectCategory2tracks)
  ,fITSmeanShift(ref.fITSmeanShift)
  ,fArraytrack		(NULL)
  ,fCounterPoolBackground	(0)
  ,fnumberfound			(0)
  ,fListOutput		(ref.fListOutput)
  ,fAssElectron		(ref.fAssElectron)
  ,fIncElectron		(ref.fIncElectron)
  ,fUSign		(ref.fUSign)
  ,fLSign		(ref.fLSign)
  ,fUSmatches(ref.fUSmatches)
  ,fLSmatches(ref.fLSmatches)
  ,fHnsigmaITS(ref.fHnsigmaITS)
//  ,fUSignAngle	(ref.fUSignAngle)
//  ,fLSignAngle	(ref.fLSignAngle)
{
  //
  // Copy Constructor
  //
  ref.Copy(*this);
}

//____________________________________________________________
AliHFENonPhotonicElectron &AliHFENonPhotonicElectron::operator=(const AliHFENonPhotonicElectron &ref){
  //
  // Assignment operator
  //
  if(this == &ref) ref.Copy(*this);
  return *this;
}

//_________________________________________
AliHFENonPhotonicElectron::~AliHFENonPhotonicElectron()
{
  //
  // Destructor
  //
  if(fArraytrack)		delete fArraytrack;
  //if(fHFEBackgroundCuts)	delete fHFEBackgroundCuts;
  if(fPIDBackground)		delete fPIDBackground;
  if(fPIDBackgroundQA)		delete fPIDBackgroundQA;
}

//_____________________________________________________________________________________________
void AliHFENonPhotonicElectron::Init()
{
  //
  // Init
  //

  //printf("Analysis Mode for AliHFENonPhotonicElectron: %s Analysis\n", fIsAOD ? "AOD" : "ESD");

  if(!fListOutput) fListOutput = new TList;
  fListOutput->SetName("HFENonPhotonicElectron");
  fListOutput->SetOwner();

  if(!fHFEBackgroundCuts) fHFEBackgroundCuts = new AliHFEcuts();
  if(fIsAOD) fHFEBackgroundCuts->SetAOD();
  fHFEBackgroundCuts->Initialize();
  if(fHFEBackgroundCuts->IsQAOn()) {
    fListOutput->Add(fHFEBackgroundCuts->GetQAhistograms());
  }

  // Initialize PID
  if(!fPIDBackground) fPIDBackground = new AliHFEpid("default pid");
  if(fMCEvent || fAODArrayMCInfo) fPIDBackground->SetHasMCData(kTRUE); // does nothing since the fMCEvent are set afterwards at the moment
  if(!fPIDBackground->GetNumberOfPIDdetectors())
  {
    //fPIDBackground->AddDetector("TOF", 0);
    fPIDBackground->AddDetector("TPC", 0);
  }
  AliInfo("PID Background QA switched on");
  fPIDBackgroundQA->Initialize(fPIDBackground);
  fListOutput->Add(fPIDBackgroundQA->MakeList("HFENP_PID_Background"));
  fPIDBackground->SortDetectors();

  Int_t nBinsPt = 35;
  //Double_t binLimPt[25] = {0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.3, 1.4, 1.5, 1.75, 2., 2.25, 2.5, 3., 3.5, 4., 5., 6.};
  Double_t binLimPt[36] = {0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1., 1.1, 1.2, 1.3, 1.4, 1.5, 1.75, 2., 2.25, 2.5, 2.75, 3., 3.5, 4., 4.5, 5., 5.5, 6., 7., 8., 10., 12., 14., 16., 18., 20.};
  const Int_t kBinsEtaInclusive = 8;
  Double_t binLimEtaInclusive[kBinsEtaInclusive+1] = {-0.8, -0.6, -0.4, -0.2, 0., 0.2, 0.4, 0.6, 0.8};
  const Int_t kBinsEtaAssociated = 30;
  Double_t binLimEtaAssociat[kBinsEtaAssociated+1] = {-1.5, -1.4, -1.3, -1.2, -1.1, -1.0, -0.9, -0.8, -0.7, -0.6, -0.5, -0.4, -0.3, -0.2, -0.1, 0., 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5};

  //Int_t nBinsP = 400;
  //Double_t minP = 0.0;
  //Double_t maxP = 20.0;
  //Double_t binLimP[nBinsP+1];
  //for(Int_t i=0; i<=nBinsP; i++) binLimP[i]=(Double_t)minP + (maxP-minP)/nBinsP*(Double_t)i ;

  Int_t nBinsC = 11;
  Double_t minC = 0.0;
  Double_t maxC = 11.0;
  Double_t binLimC[nBinsC+1];
  for(Int_t i=0; i<=nBinsC; i++) binLimC[i]=(Double_t)minC + (maxC-minC)/nBinsC*(Double_t)i ;

  Int_t nBinsSource = 10;
  Double_t minSource = 0.;
  Double_t maxSource = 10.;
  Double_t binLimSource[nBinsSource+1];
  for(Int_t i=0; i<=nBinsSource; i++) binLimSource[i]=(Double_t)minSource + (maxSource-minSource)/nBinsSource*(Double_t)i ;

  Int_t nBinsInvMass = 100;
  Double_t minInvMass = 0.;
  Double_t maxInvMass = 1.;
  Double_t binLimInvMass[nBinsInvMass+1];
  for(Int_t i=0; i<=nBinsInvMass; i++) binLimInvMass[i]=(Double_t)minInvMass + (maxInvMass-minInvMass)/nBinsInvMass*(Double_t)i ;

  Int_t nBinsPhi = 180;
  Double_t minPhi = 0.0;
  Double_t maxPhi = TMath::Pi();
  Double_t binLimPhi[nBinsPhi+1];
  for(Int_t i=0; i<=nBinsPhi; i++)
  {
    binLimPhi[i]=(Double_t)minPhi + (maxPhi-minPhi)/nBinsPhi*(Double_t)i ;
    AliDebug(2,Form("bin phi is %f for %d",binLimPhi[i],i));
  }

  Int_t nBinsAngle = 180;
  Double_t minAngle = 0.0;
  Double_t maxAngle = TMath::Pi();
  Double_t binLimAngle[nBinsAngle+1];
  for(Int_t i=0; i<=nBinsAngle; i++)
  {
    binLimAngle[i]=(Double_t)minAngle + (maxAngle-minAngle)/nBinsAngle*(Double_t)i ;
    AliDebug(2,Form("bin phi is %f for %d",binLimAngle[i],i));
  }

  // Constrain histograms
  const Int_t nDimSingle=4;
  const Int_t nDimPair=9;
  Int_t nBinPair[nDimPair] = {nBinsPhi,nBinsC,nBinsPt,nBinsInvMass,nBinsSource,nBinsAngle,nBinsPt,kBinsEtaInclusive,kBinsEtaAssociated};
  
  // Associated Electron
  Int_t nBinAssElectron[nDimSingle] = {nBinsC,nBinsPt,nBinsSource,kBinsEtaAssociated};
  fAssElectron = new THnSparseF("fAssElectron","fAssElectron",nDimSingle,nBinAssElectron);
  fAssElectron->SetBinEdges(0,binLimC);
  fAssElectron->SetBinEdges(1,binLimPt);
  fAssElectron->SetBinEdges(2,binLimSource);
  fAssElectron->SetBinEdges(3,binLimEtaAssociat);
  fAssElectron->Sumw2();
  AliDebug(2,"AliHFENonPhotonicElectron: fAssElectron");

  // Inclusive Electron
  Int_t nBinIncElectron[nDimSingle] = {nBinsC,nBinsPt,nBinsSource,kBinsEtaInclusive};
  fIncElectron = new THnSparseF("fIncElectron","fIncElectron",nDimSingle,nBinIncElectron);
  fIncElectron->SetBinEdges(0,binLimC);
  fIncElectron->SetBinEdges(1,binLimPt);
  fIncElectron->SetBinEdges(2,binLimSource);
  fIncElectron->SetBinEdges(3,binLimEtaInclusive);
  fIncElectron->Sumw2();
  AliDebug(2,"AliHFENonPhotonicElectron: fIncElectron");

  // ee invariant mass Unlike Sign
  fUSign = new THnSparseF("fUSign","fUSign",nDimPair,nBinPair);
  fUSign->SetBinEdges(0,binLimPhi);
  fUSign->SetBinEdges(1,binLimC);
  fUSign->SetBinEdges(2,binLimPt);
  fUSign->SetBinEdges(3,binLimInvMass);
  fUSign->SetBinEdges(4,binLimSource);
  fUSign->SetBinEdges(5,binLimAngle);
  fUSign->SetBinEdges(6,binLimPt);
  fUSign->SetBinEdges(7,binLimEtaInclusive);
  fUSign->SetBinEdges(8,binLimEtaAssociat);
  fUSign->Sumw2();
  AliDebug(2,"AliHFENonPhotonicElectron: fUSign");

  // ee invariant mass Like Sign
  fLSign = new THnSparseF("fLSign","fLSign",nDimPair,nBinPair);
  fLSign->SetBinEdges(0,binLimPhi);
  fLSign->SetBinEdges(1,binLimC);
  fLSign->SetBinEdges(2,binLimPt);
  fLSign->SetBinEdges(3,binLimInvMass);
  fLSign->SetBinEdges(4,binLimSource);
  fLSign->SetBinEdges(5,binLimAngle);
  fLSign->SetBinEdges(6,binLimPt);
  fLSign->SetBinEdges(7,binLimEtaInclusive);
  fLSign->SetBinEdges(8,binLimEtaAssociat);
  fLSign->Sumw2();
  AliDebug(2,"AliHFENonPhotonicElectron: fLSign");

  // Histograms counting the number of like sign / unlike sign matches per inclusive track
  const Int_t nBinsMatches = 50;
  Double_t binLimMatches[nBinsMatches+1];
  for(int ib = 0; ib <= nBinsMatches; ib++) binLimMatches[ib] = ib;
  const Int_t nDimMatches = 3;  // centrality, pt_inc, number of matches 
  const Int_t nBinsMatchHist[nDimMatches] = {nBinsC, nBinsPt, nBinsMatches};
  fUSmatches = new THnSparseF("fUSmatches", "fUSmatches", nDimMatches, nBinsMatchHist);
  fUSmatches->SetBinEdges(0,binLimC);
  fUSmatches->SetBinEdges(1,binLimPt);
  fUSmatches->SetBinEdges(2,binLimMatches);

  fLSmatches = new THnSparseF("fLSmatches", "fLSmatches", nDimMatches, nBinsMatchHist);
  fLSmatches->SetBinEdges(0,binLimC);
  fLSmatches->SetBinEdges(1,binLimPt);
  fLSmatches->SetBinEdges(2,binLimMatches);

/*
  // ee angle Unlike Sign
  const Int_t nDimUSignAngle=3;
  Int_t nBinUSignAngle[nDimUSignAngle] = {nBinsAngle,nBinsC,nBinsSource};
  fUSignAngle = new THnSparseF("fUSignAngle","fUSignAngle",nDimUSignAngle,nBinUSignAngle);
  fUSignAngle->SetBinEdges(0,binLimAngle);
  fUSignAngle->SetBinEdges(1,binLimC);
  fUSignAngle->SetBinEdges(2,binLimSource);
  fUSignAngle->Sumw2();
  AliDebug(2,"AliHFENonPhotonicElectron: fUSignAngle");

  // ee angle Like Sign
  const Int_t nDimLSignAngle=3;
  Int_t nBinLSignAngle[nDimLSignAngle] = {nBinsAngle,nBinsC,nBinsSource};
  fLSignAngle = new THnSparseF("fLSignAngle","fLSignAngle",nDimLSignAngle,nBinLSignAngle);
  fLSignAngle->SetBinEdges(0,binLimAngle);
  fLSignAngle->SetBinEdges(1,binLimC);
  fLSignAngle->SetBinEdges(2,binLimSource);
  fLSignAngle->Sumw2();
  AliDebug(2,"AliHFENonPhotonicElectron: fLSignAngle");
*/

  // control histogram for ITS PID
  fHnsigmaITS = new TH2F("fHnsigmaITS", "Number of sigmas in the ITS", 30, 0., 0.3, 1200, -10., 10.);

  fListOutput->Add(fAssElectron);
  fListOutput->Add(fIncElectron);
  fListOutput->Add(fUSign);
  fListOutput->Add(fLSign);
  fListOutput->Add(fUSmatches);
  fListOutput->Add(fLSmatches);
  fListOutput->Add(fHnsigmaITS);
//  fListOutput->Add(fUSignAngle);
//  fListOutput->Add(fLSignAngle);

}

//_____________________________________________________________________________________________
void AliHFENonPhotonicElectron::InitRun(const AliVEvent *inputEvent,const AliPIDResponse *pidResponse)
{
  //
  // Init run
  //

  if(!pidResponse)
  {
    AliDebug(1, "Using default PID Response");
    Bool_t hasmc = kFALSE;
    if(fMCEvent || fAODArrayMCInfo) hasmc=kTRUE;
    pidResponse = AliHFEtools::GetDefaultPID(hasmc, inputEvent->IsA() == AliESDEvent::Class());
  }

  if(!fPIDBackground) return;
  fPIDBackground->SetPIDResponse(pidResponse);

  if(!fPIDBackground->IsInitialized())
  {
    // Initialize PID with the given run number
    fPIDBackground->InitializePID(inputEvent->GetRunNumber());
  }

}

//_____________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::FillPoolAssociatedTracks(AliVEvent *inputEvent, Int_t binct)
{
  //
  // Fill the pool of associated tracks
  // Return the number of associated tracks
  //

  fnumberfound = 0;

  fHFEBackgroundCuts->SetRecEvent(inputEvent);
  Int_t nbtracks = inputEvent->GetNumberOfTracks();

  if( fArraytrack ){
    fArraytrack->~TArrayI();
    new(fArraytrack) TArrayI(nbtracks);
  } else {
    fArraytrack = new TArrayI(nbtracks);
  }

  fCounterPoolBackground = 0;

  Bool_t isSelected(kFALSE);
  Bool_t isAOD = (dynamic_cast<AliAODEvent *>(inputEvent) != NULL);
  AliDebug(2, Form("isAOD: %s", isAOD ? "yes" : "no"));
  for(Int_t k = 0; k < nbtracks; k++) {
    AliVTrack *track = (AliVTrack *) inputEvent->GetTrack(k);
    if(!track) continue;
    
    //
    isSelected = kFALSE;
    if(fSelectCategory1tracks && FilterCategory1Track(track, isAOD, binct)) isSelected = kTRUE;
    else if(fSelectCategory2tracks && FilterCategory2Track(track, isAOD)) isSelected = kTRUE;

    if(isSelected){
	    AliDebug(2,Form("fCounterPoolBackground %d, track %d",fCounterPoolBackground,k));
      fArraytrack->AddAt(k,fCounterPoolBackground);
      fCounterPoolBackground++;
    }
  } // loop tracks

  //printf(Form("Associated Pool: Tracks %d, fCounterPoolBackground %d \n", nbtracks, fCounterPoolBackground));

  return fCounterPoolBackground;

}

//_____________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::CountPoolAssociated(AliVEvent *inputEvent, Int_t binct)
{
  //
  // Count the pool of assiocated tracks
  //


  if(fnumberfound > 0) //!count only events with an inclusive electron
  {
    Double_t valueAssElectron[4] = { binct, -1, -1};		//Centrality	Pt	Source
    Int_t iTrack2 = 0;
    Int_t indexmother2 = -1;
    AliVTrack *track2 = 0x0;

    for(Int_t ii = 0; ii < fCounterPoolBackground; ii++){
      iTrack2 = fArraytrack->At(ii);
      AliDebug(2,Form("track %d",iTrack2));
      track2 = (AliVTrack *)inputEvent->GetTrack(iTrack2);

      if(!track2){
	      //printf("ERROR: Could not receive track %d", iTrack2);
	      continue;
      }

      // if MC look
      if(fMCEvent || fAODArrayMCInfo) valueAssElectron[2] = FindMother(TMath::Abs(track2->GetLabel()), indexmother2) ;

      fkPIDRespons = fPIDBackground->GetPIDResponse();

      valueAssElectron[1] = track2->Pt() ;
      valueAssElectron[3] = track2->Eta() ;

      fAssElectron->Fill( valueAssElectron) ;
    }
  //printf(Form("Associated Pool: fCounterPoolBackground %d \n", fCounterPoolBackground));
  }
  return fnumberfound;
}

//_____________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::LookAtNonHFE(Int_t iTrack1, AliVTrack *track1, AliVEvent *vEvent, Double_t weight, Int_t binct, Double_t deltaphi, Int_t source, Int_t indexmother)
{
  //
  // Look At Non HFE
  //

  /***********************************************************************************
   *											*
   *	iTrack1:	index of the tagged electrons in AliVEvent			*
   *	track1:		tagged electron							*
   *	vEvent:		event								*
   *	weight:		weight in pt if not realistic					*
   *	binct:		centrality bin							*
   *	deltaphi:	phi-phi event plane for v2					*
   *	source:		MC sources							*
   *	indexmother:	MC index mother							*
   *											*
   *											*
   *	return -1  if  nothing								*
   *	return  2  if  opposite		charge		within the mass range		*
   *	return  4  if      like		charge		within the mass range		*
   *	return  6  if  opposite & like charge		within the mass range		*
   *											*
   ***********************************************************************************/

  AliAODEvent *aodeventu = dynamic_cast<AliAODEvent*>(vEvent);
  Int_t taggedphotonic = -1;

  AliDebug(2,Form("fCounterPoolBackground %d in LookAtNonHFE!!!",fCounterPoolBackground));
  if(!fArraytrack) return taggedphotonic;
  AliDebug(2,Form("process track %d",iTrack1));
  AliDebug(1,Form("Inclusive source is %d\n", source));

  fkPIDRespons = fPIDBackground->GetPIDResponse();

  //Set Fill-Arrays for THnSparse
  Double_t valueIncElectron[4]	= { binct, track1->Pt(), source, track1->Eta()};	//Centrality	Pt	Source	P	
  Double_t valueSign[9]		= { deltaphi, binct, track1->Pt(), -1, source, -1, -1, track1->Eta(), -1};			//DeltaPhi	Centrality	Pt	InvariantMass	Source	Angle	Pt
  //Double_t valueAngle[3]	= { -1, binct, source};								//Angle		Centrality	Source

  Int_t pdg1 = CheckPdg(TMath::Abs(track1->GetLabel()));
  AliKFParticle::SetField(vEvent->GetMagneticField());
  AliKFVertex primV(*(vEvent->GetPrimaryVertex()));

  AliVTrack *track2(NULL);
  Int_t iTrack2 = 0;
  Int_t indexmother2 = -1;
  Int_t pdg2 = -100;
  Int_t source2 = -1;
  Float_t fCharge2 = 0;

  // count number of matches with opposite/same sign track in the given mass range
  Int_t countsMatchLikesign(0),
        countsMatchUnlikesign(0);

  // Values to fill
  Double_t angle(-1.);
  Double_t invmass(-1);

  Float_t fCharge1 = track1->Charge();							//Charge from track1

  Bool_t kUSignPhotonic = kFALSE;
  Bool_t kLSignPhotonic = kFALSE;

  //! FILL Inclusive Electron
  fIncElectron->Fill(valueIncElectron,weight);
  fnumberfound++;
  //printf(Form("Inclusive Pool: TrackNr. %d, fnumberfound %d \n", iTrack1, fnumberfound));

  for(Int_t idex = 0; idex < fCounterPoolBackground; idex++){
    iTrack2 = fArraytrack->At(idex);
    AliDebug(2,Form("track %d",iTrack2));
    track2 = (AliVTrack *)vEvent->GetTrack(iTrack2);

    if(!track2){
      //printf("ERROR: Could not receive track %d", iTrack2);
      continue;
    }

    fCharge2 = track2->Charge();		//Charge from track2

    // Reset the MC info
    //valueAngle[2] = source;
    valueSign[4] = source;
    valueSign[6] = track2->Pt();
    valueSign[8] = track2->Eta();

    // track cuts and PID already done

    // Checking if it is the same Track!
    if(iTrack2==iTrack1) continue;
    AliDebug(2,"Different");

    // if MC look
    if(fMCEvent || fAODArrayMCInfo){
      AliDebug(2, "Checking for source");
      source2	 = FindMother(TMath::Abs(track2->GetLabel()), indexmother2);
      AliDebug(2, Form("source is %d", source2));
      pdg2	 = CheckPdg(TMath::Abs(track2->GetLabel()));

      if(source == kElectronfromconversion){
        AliDebug(2, Form("Electron from conversion (source %d), paired with source %d", source, source2));
        AliDebug(2, Form("Index of the mothers: incl %d, associated %d", indexmother, indexmother2));
        AliDebug(2, Form("PDGs: incl %d, associated %d", pdg1, pdg2));
      }

      if(source2 >=0 ){
	      if((indexmother2 == indexmother) && (source == source2) && ((pdg1*pdg2)<0.0)){
          AliDebug(1, "Real pair");
          switch(source){
            case kElectronfromconversion: 
                 valueSign[4] = kElectronfromconversionboth; 
                 break;
            case kElectronfrompi0: 
                 valueSign[4] = kElectronfrompi0both; 
                 break;
            case kElectronfrometa:
                 valueSign[4] = kElectronfrometaboth;
                 break;
          };
        }
      }
    }

    if(fAlgorithmMA){
      // Use TLorentzVector
      if(!MakePairDCA(track1, track2, vEvent, (aodeventu != NULL), invmass, angle)) continue;
    } else {
      // Use AliKF package
      if(!MakePairKF(track1, track2, primV, invmass, angle)) continue;
    }

    valueSign[3] = invmass;
    valueSign[5] = angle;

    //if((fCharge1*fCharge2)>0.0)	fLSignAngle->Fill(&valueAngle[0],weight);
    //else				fUSignAngle->Fill(&valueAngle[0],weight);

    if(angle > fMaxOpening3D) continue;				 //! Cut on Opening Angle
    if(invmass > fMaxInvMass) continue;				//! Cut on Invariant Mass

    if((fCharge1*fCharge2)>0.0){	
      if(invmass < 1.0) fLSign->Fill( valueSign, weight);
      // count like-sign background matched pairs per inclusive based on mass cut
      if(invmass < 0.14) countsMatchLikesign++;
      AliDebug(1, "Selected Like sign");
    } else {
      if(invmass < 1.0)fUSign->Fill( valueSign, weight);
      // count unlike-sign matched pairs per inclusive based on mass cut
      if(invmass < 0.14) countsMatchUnlikesign++;
      AliDebug(1, "Selected Unike sign");
    }

    if((fCharge1*fCharge2)>0.0)	kLSignPhotonic=kTRUE;
    else				kUSignPhotonic=kTRUE;
  }

  // Fill counted
  Double_t valCountsLS[3] = {binct, track1->Pt(), countsMatchLikesign},
           valCountsUS[3] = {binct, track1->Pt(), countsMatchUnlikesign}; 
  fUSmatches->Fill(valCountsUS);
  fLSmatches->Fill(valCountsLS);

  if( kUSignPhotonic &&  kLSignPhotonic) taggedphotonic = 6;
  if(!kUSignPhotonic &&  kLSignPhotonic) taggedphotonic = 4;
  if( kUSignPhotonic && !kLSignPhotonic) taggedphotonic = 2;

  return taggedphotonic;
}

//_________________________________________________________________________
Int_t AliHFENonPhotonicElectron::FindMother(Int_t tr, Int_t &indexmother) const {
  //
  // Find the mother if MC
  //

  if(!fMCEvent && !fAODArrayMCInfo) return 0;

  Int_t pdg = CheckPdg(tr);
  if(TMath::Abs(pdg)!= 11)
  {
    indexmother = -1;
    return kNoElectron;
  }

  indexmother = IsMotherGamma(tr);
  if(indexmother > 0) return kElectronfromconversion;
  indexmother = IsMotherPi0(tr);
  if(indexmother > 0) return kElectronfrompi0;
  indexmother = IsMotherC(tr);
  if(indexmother > 0) return kElectronfromC;
  indexmother = IsMotherB(tr);
  if(indexmother > 0) return kElectronfromB;
  indexmother = IsMotherEta(tr);
  if(indexmother > 0) return kElectronfrometa;

  return kElectronfromother;
}

//________________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::CheckPdg(Int_t tr) const {

  //
  // Return the pdg of the particle
  //

  Int_t pdgcode = -1;
  if(tr < 0) return pdgcode;

  AliMCParticle *mctrackesd = NULL; AliAODMCParticle *mctrackaod = NULL;
  if(fMCEvent){
    AliVParticle *mctrack = fMCEvent->GetTrack(tr);
    if(mctrack){
      if((mctrackesd = dynamic_cast<AliMCParticle *>(fMCEvent->GetTrack(TMath::Abs(tr))))) pdgcode = mctrackesd->PdgCode();
      else if((mctrackaod = dynamic_cast<AliAODMCParticle *>(fMCEvent->GetTrack(TMath::Abs(tr))))) pdgcode = mctrackaod->GetPdgCode(); 
    }
  } else if(fAODArrayMCInfo) {
    if(tr < fAODArrayMCInfo->GetEntriesFast()){
      mctrackaod = (AliAODMCParticle *) fAODArrayMCInfo->At(tr);
      if(mctrackaod) return pdgcode = mctrackaod->GetPdgCode();
    }
  }

  return pdgcode;
}

//_______________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::GetMotherPDG(Int_t tr, Int_t &motherIndex) const {
  //
  // Returns the mother PDG of the track (return value) and the index of the
  // mother track 
  //
  if(tr < 0) return -1;

  Int_t pdg(-1);
  AliMCParticle *mctrackesd(NULL); AliAODMCParticle *mctrackaod(NULL);

  motherIndex = -1;
  if(fMCEvent) {
    AliDebug(2, "Using MC Event");
    AliVParticle *mctrack = fMCEvent->GetTrack(tr);
    if(mctrack){
      if((mctrackesd = dynamic_cast<AliMCParticle *>(fMCEvent->GetTrack(TMath::Abs(tr))))){
        // Case ESD
        TParticle *particle = mctrackesd->Particle();

        // Take mother
        if(particle){
          motherIndex   = particle->GetFirstMother();
          if(motherIndex >= 0){
            AliMCParticle *mothertrack = NULL;
            if((mothertrack = dynamic_cast<AliMCParticle *>(fMCEvent->GetTrack(TMath::Abs(motherIndex))))){
              TParticle * mother = mothertrack->Particle();
              pdg = mother->GetPdgCode();
            }
          }
        }
      } else if((mctrackaod = dynamic_cast<AliAODMCParticle *>(fMCEvent->GetTrack(TMath::Abs(tr))))){
        // Case AOD
        // Take mother
        motherIndex = mctrackaod->GetMother();
        if(motherIndex >= 0){
          AliAODMCParticle *mothertrack = dynamic_cast<AliAODMCParticle *>(fMCEvent->GetTrack(motherIndex)); 
          if(mothertrack) pdg = mothertrack->GetPdgCode();
        }
      }
    }
  } else if(fAODArrayMCInfo) {
    AliDebug(2, "Using AOD list");
    if(tr < fAODArrayMCInfo->GetEntriesFast()){ 
      mctrackaod = (AliAODMCParticle *) fAODArrayMCInfo->At(tr);

      // Take mother
      if(mctrackaod){
        motherIndex = mctrackaod->GetMother();
        if(motherIndex >= 0 && motherIndex < fAODArrayMCInfo->GetEntriesFast()){
          AliAODMCParticle *mothertrack = dynamic_cast<AliAODMCParticle *>(fAODArrayMCInfo->At(TMath::Abs(motherIndex)));
          if(mothertrack) pdg = mothertrack->GetPdgCode();
        }
      }
    }
  }
  return pdg;
}

//_______________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::IsMotherGamma(Int_t tr) const {

  //
  // Return the lab of gamma mother or -1 if not gamma
  //

  Int_t imother(-1), pdg(-1);
  pdg = GetMotherPDG(tr, imother);
  
  // Check gamma
  if(imother >= 0){
    if(TMath::Abs(pdg) == 22){
      AliDebug(2, "Gamma Mother selected");
      return imother;
    }
    if(TMath::Abs(pdg) == 11){
      AliDebug(2, "Mother is electron - look further in hierarchy");
      return IsMotherGamma(imother);
    }
    AliDebug(2, "Nothing selected");
    return -1;
  }
  AliDebug(2, "Not mother");
  return -1;
}

//________________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::IsMotherPi0(Int_t tr) const {

  //
  // Return the lab of pi0 mother or -1 if not pi0
  //

  Int_t imother(-1), pdg(-1);
  pdg = GetMotherPDG(tr, imother);

  // Check pi0
  if(imother >= 0){
    if(TMath::Abs(pdg) == 111){
      AliDebug(2, "Pi0 Mother selected");
      return imother;
    }
    if(TMath::Abs(pdg) == 11){
      AliDebug(2, "Mother is electron - look further in hierarchy");
      return IsMotherPi0(imother);
    }
    AliDebug(2, "Nothing selected");
    return -1;
  }
  AliDebug(2, "Not mother");
  return -1;
}
//________________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::IsMotherC(Int_t tr) const {

  //
  // Return the lab of signal mother or -1 if not from C
  //

  Int_t imother(-1), pdg(-1);
  pdg = GetMotherPDG(tr, imother);

    // Check C
  if(imother >= 0){
    if((TMath::Abs(pdg)==411) || (TMath::Abs(pdg)==421) || (TMath::Abs(pdg)==431) || (TMath::Abs(pdg)==4122) || (TMath::Abs(pdg)==4132) || (TMath::Abs(pdg)==4232) || (TMath::Abs(pdg)==43320)){ 
      AliDebug(2, "Charm Mother selected");
      return imother;
    }
    if(TMath::Abs(pdg) == 11){
      AliDebug(2, "Mother is electron - look further in hierarchy");
      return IsMotherC(imother);
    }
    AliDebug(2, "Nothing selected");
    return -1;
  }
  AliDebug(2, "Not mother");
  return -1;
}

//_______________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::IsMotherB(Int_t tr) const {

  //
  // Return the lab of signal mother or -1 if not B
  //

  Int_t imother(-1), pdg(-1);
  pdg = GetMotherPDG(tr, imother);

  // Check B
  if(imother >= 0){  
    if((TMath::Abs(pdg)==511) || (TMath::Abs(pdg)==521) || (TMath::Abs(pdg)==531) || (TMath::Abs(pdg)==5122) || (TMath::Abs(pdg)==5132) || (TMath::Abs(pdg)==5232) || (TMath::Abs(pdg)==53320)){
      AliDebug(2, "Bottom Mother selected");
      return imother;
    }
    if(TMath::Abs(pdg) == 11){
      return IsMotherB(imother);
      AliDebug(2, "Mother is electron - look further in hierarchy");
    }
    AliDebug(2, "Nothing selected");
    return -1;
  }
  AliDebug(2, "Not mother");
  return -1;
}

//_______________________________________________________________________________________________
Int_t AliHFENonPhotonicElectron::IsMotherEta(Int_t tr) const {

  //
  // Return the lab of eta mother or -1 if not eta
  //

  Int_t imother(-1), pdg(-1);
  pdg = GetMotherPDG(tr, imother);

  // Check eta
  if(imother >= 0){  
    if(TMath::Abs(pdg) == 221){
      AliDebug(2, "Eta mother selected");
      return imother;
    }
    if(TMath::Abs(pdg) == 11){
      AliDebug(2, "Mother is electron - look further in hierarchy");
      return IsMotherEta(imother);
    }
    AliDebug(2, "Nothing selected");
    return -1;
  }
  AliDebug(2, "Not mother");
  return -1;
}

//_______________________________________________________________________________________________
Bool_t AliHFENonPhotonicElectron::MakePairDCA(const AliVTrack *inclusive, const AliVTrack *associated, AliVEvent *vEvent, Bool_t isAOD, Double_t &invMass, Double_t &angle) const {
  //
  // Make Pairs of electrons using TLorentzVector
  //
  Double_t eMass = TDatabasePDG::Instance()->GetParticle(11)->Mass(); //Electron mass in GeV
  Double_t bfield = vEvent->GetMagneticField();

  AliESDtrack *esdtrack1, *esdtrack2;
  if(isAOD){
    // call copy constructor for AODs
    esdtrack1 = new AliESDtrack(inclusive);
    esdtrack2 = new AliESDtrack(associated);
  } else {
    // call copy constructor for ESDs
    esdtrack1 = new AliESDtrack(*(static_cast<const AliESDtrack *>(inclusive)));
    esdtrack2 = new AliESDtrack(*(static_cast<const AliESDtrack *>(associated)));
  }
  if((!esdtrack1) || (!esdtrack2)){ 
    delete esdtrack1;
    delete esdtrack2;
    return kFALSE;
  }

  Double_t xt1 = 0; //radial position track 1 at the DCA point
  Double_t xt2 = 0; //radial position track 2 at the DCA point
  Double_t dca = esdtrack2->GetDCA(esdtrack1,bfield,xt2,xt1);		//DCA track1-track2
  if(dca > fMaxDCA){
    // Apply DCA cut already in the function
    delete esdtrack1;
    delete esdtrack2;
    return kFALSE;
  }

  //Momenta of the track extrapolated to DCA track-track
  Double_t p1[3] = {0,0,0};
  Double_t p2[3] = {0,0,0};
  Bool_t kHasdcaT1 = esdtrack1->GetPxPyPzAt(xt1,bfield,p1);		//Track1
  Bool_t kHasdcaT2 = esdtrack2->GetPxPyPzAt(xt2,bfield,p2);		//Track2
  if(!kHasdcaT1 || !kHasdcaT2) AliWarning("It could be a problem in the extrapolation");

  TLorentzVector electron1, electron2, mother;
  electron1.SetXYZM(p1[0], p1[1], p1[2], eMass);
  electron2.SetXYZM(p2[0], p2[1], p2[2], eMass);

  mother      = electron1 + electron2;
  invMass  = mother.M();
  angle    = TVector2::Phi_0_2pi(electron1.Angle(electron2.Vect()));

  delete esdtrack1;
  delete esdtrack2;
  return kTRUE;
}

//_______________________________________________________________________________________________
Bool_t AliHFENonPhotonicElectron::MakePairKF(const AliVTrack *inclusive, const AliVTrack *associated, AliKFVertex &primV, Double_t &invMass, Double_t &angle) const {
  //
  // Make pairs of electrons using the AliKF package
  //

  //printf("AOD HFE non photonic\n");

  Int_t fPDGtrack1 = 11;
  if(inclusive->Charge()>0) fPDGtrack1 = -11;
  Int_t fPDGtrack2 = 11;
  if(associated->Charge()>0) fPDGtrack2 = -11;

  AliKFParticle ktrack1(*inclusive, fPDGtrack1);
  AliKFParticle ktrack2(*associated, fPDGtrack2);
  AliKFParticle recoGamma(ktrack1,ktrack2);

  if(recoGamma.GetNDF()<1) return kFALSE;				//! Cut on Reconstruction

  Double_t chi2OverNDF = recoGamma.GetChi2()/recoGamma.GetNDF();
  if(TMath::Sqrt(TMath::Abs(chi2OverNDF))>fChi2OverNDFCut) return kFALSE;

  if(fSetMassConstraint){
	  primV += recoGamma;
	  primV -= ktrack1;
	  primV -= ktrack2;
	  recoGamma.SetProductionVertex(primV);
	  recoGamma.SetMassConstraint(0,0.0001);
  }

  Double_t width(0.);
  recoGamma.GetMass(invMass,width);
  angle = ktrack1.GetAngle(ktrack2);
  return kTRUE;
}

//_______________________________________________________________________________________________
Bool_t AliHFENonPhotonicElectron::FilterCategory1Track(const AliVTrack * const track, Bool_t isAOD, Int_t binct){
  //
  // Selection of good associated tracks for the pool
  // selection is done using strong cuts
  // Tracking in the TPC and the ITS is a minimal requirement
  // 
  Bool_t survivedbackground = kTRUE;

  if(!fHFEBackgroundCuts->CheckParticleCuts(AliHFEcuts::kStepRecKineITSTPC + AliHFEcuts::kNcutStepsMCTrack, (TObject *) track)) survivedbackground = kFALSE;
  AliDebug(3, Form("First cut: %s\n", survivedbackground == kTRUE ? "yes" : "no"));
  if(!fHFEBackgroundCuts->CheckParticleCuts(AliHFEcuts::kStepRecPrim       + AliHFEcuts::kNcutStepsMCTrack, (TObject *) track)) survivedbackground = kFALSE;
  AliDebug(3, Form("Second cut: %s\n", survivedbackground == kTRUE ? "yes" : "no"));

  if(survivedbackground){
    // PID track cuts
    AliHFEpidObject hfetrack2;

    if(!isAOD)	hfetrack2.SetAnalysisType(AliHFEpidObject::kESDanalysis);
    else 		hfetrack2.SetAnalysisType(AliHFEpidObject::kAODanalysis);

    hfetrack2.SetRecTrack(track);
    if(binct>-1){
     hfetrack2.SetCentrality((Int_t)binct);
     AliDebug(3,Form("centrality %d and %d",binct,hfetrack2.GetCentrality()));
     hfetrack2.SetPbPb();
    }

    if(fPIDBackground->IsSelected(&hfetrack2,0x0,"recTrackCont",fPIDBackgroundQA)){
      survivedbackground = kTRUE;
    } else survivedbackground = kFALSE;
  }
  AliDebug(3, Form("PID: %s\n", survivedbackground == kTRUE ? "yes" : "no"));
  return survivedbackground;
}

//_______________________________________________________________________________________________
Bool_t AliHFENonPhotonicElectron::FilterCategory2Track(const AliVTrack * const track, Bool_t isAOD){
  //
  // Selection of category 2 tracks: These tracks are exclusively low pt tracks below
  // 300 MeV/c which have at least 2 hits in the 4 outer ITS layers and are identified as
  // electron candidates by the ITS
  //
  if(TMath::Abs(track->Pt()) > 0.3) return kFALSE;
  Int_t nclustersITS(0), nclustersOuter(0);
  if(isAOD){
    const AliAODTrack *aodtrack = static_cast<const AliAODTrack *>(track);
    if(!(aodtrack->TestFilterBit(AliAODTrack::kTrkGlobalNoDCA) || aodtrack->TestFilterBit(AliAODTrack::kTrkITSsa))) return kFALSE;
    nclustersITS = aodtrack->GetITSNcls();
    for(int ily = 2; ily < 5; ily++)
      if(aodtrack->HasPointOnITSLayer(ily)) nclustersOuter++;
  } else {
    const AliESDtrack *esdtrack = static_cast<const AliESDtrack *>(track);
    if(esdtrack->GetStatus() & AliESDtrack::kITSpureSA) return kFALSE;
    nclustersITS = esdtrack->GetITSclusters(NULL);
    for(int ily = 2; ily < 5; ily++)
      if(esdtrack->HasPointOnITSLayer(ily)) nclustersOuter++;
  }
  if(nclustersITS < 4) return kFALSE;
  if(nclustersOuter < 3) return kFALSE;

  // Do ITS PID
  Double_t nsigmaITS = fPIDBackground->GetPIDResponse()->NumberOfSigmasITS(track, AliPID::kElectron);
  fHnsigmaITS->Fill(track->Pt(), nsigmaITS);
  if(TMath::Abs(nsigmaITS - fITSmeanShift) > 3.) return kFALSE;
  // if global track, we apply also TPC PID
  return kTRUE;
}
