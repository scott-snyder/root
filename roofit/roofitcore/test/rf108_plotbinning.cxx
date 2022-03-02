/////////////////////////////////////////////////////////////////////////
//
// 'BASIC FUNCTIONALITY' RooFit tutorial macro #108
//
// Plotting unbinned data with alternate and variable binnings
//
//
// 07/2008 - Wouter Verkerke
//
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGaussModel.h"
#include "RooDecay.h"
#include "RooBMixDecay.h"
#include "RooCategory.h"
#include "RooBinning.h"
#include "RooPlot.h"
#include "TCanvas.h"
#include "TH1.h"
using namespace RooFit ;


class TestBasic108 : public RooFitTestUnit
{
public:
  TestBasic108(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooFitTestUnit("Non-standard binning in counting and asymmetry plots",refFile,writeRef,verbose) {} ;
  Bool_t testCode() {

    // S e t u p   m o d e l
    // ---------------------

    // Build a B decay p.d.f with mixing
    RooRealVar dt("dt","dt",-20,20) ;
    RooRealVar dm("dm","dm",0.472) ;
    RooRealVar tau("tau","tau",1.547) ;
    RooRealVar w("w","mistag rate",0.1) ;
    RooRealVar dw("dw","delta mistag rate",0.) ;

    RooCategory mixState("mixState","B0/B0bar mixing state") ;
    mixState.defineType("mixed",-1) ;
    mixState.defineType("unmixed",1) ;
    RooCategory tagFlav("tagFlav","Flavour of the tagged B0") ;
    tagFlav.defineType("B0",1) ;
    tagFlav.defineType("B0bar",-1) ;

    // Build a gaussian resolution model
    RooRealVar dterr("dterr","dterr",0.1,1.0) ;
    RooRealVar bias1("bias1","bias1",0) ;
    RooRealVar sigma1("sigma1","sigma1",0.1) ;
    RooGaussModel gm1("gm1","gauss model 1",dt,bias1,sigma1) ;

    // Construct Bdecay (x) gauss
    RooBMixDecay bmix("bmix","decay",dt,mixState,tagFlav,tau,dm,w,dw,gm1,RooBMixDecay::DoubleSided) ;


    // S a m p l e   d a t a   f r o m   m o d e l
    // --------------------------------------------

    // Sample 2000 events in (dt,mixState,tagFlav) from bmix
    RooDataSet *data = bmix.generate(RooArgSet(dt,mixState,tagFlav),2000) ;



    // S h o w   d t   d i s t r i b u t i o n   w i t h   c u s t o m   b i n n i n g
    // -------------------------------------------------------------------------------

    // Make plot of dt distribution of data in range (-15,15) with fine binning for dt>0 and coarse binning for dt<0

    // Create binning object with range (-15,15)
    RooBinning tbins(-15,15) ;

    // Add 60 bins with uniform spacing in range (-15,0)
    tbins.addUniform(60,-15,0) ;

    // Add 15 bins with uniform spacing in range (0,15)
    tbins.addUniform(15,0,15) ;

    // Make plot with specified binning
    RooPlot* dtframe = dt.frame(Range(-15,15),Title("dt distribution with custom binning")) ;
    data->plotOn(dtframe,Binning(tbins)) ;
    bmix.plotOn(dtframe) ;

    // NB: Note that bin density for each bin is adjusted to that of default frame binning as shown
    // in Y axis label (100 bins --> Events/0.4*Xaxis-dim) so that all bins represent a consistent density distribution


    // S h o w   m i x s t a t e   a s y m m e t r y  w i t h   c u s t o m   b i n n i n g
    // ------------------------------------------------------------------------------------

    // Make plot of dt distribution of data asymmetry in 'mixState' with variable binning

    // Create binning object with range (-10,10)
    RooBinning abins(-10,10) ;

    // Add boundaries at 0, (-1,1), (-2,2), (-3,3), (-4,4) and (-6,6)
    abins.addBoundary(0) ;
    abins.addBoundaryPair(1) ;
    abins.addBoundaryPair(2) ;
    abins.addBoundaryPair(3) ;
    abins.addBoundaryPair(4) ;
    abins.addBoundaryPair(6) ;

    // Create plot frame in dt
    RooPlot* aframe = dt.frame(Range(-10,10),Title("mixState asymmetry distribution with custom binning")) ;

    // Plot mixState asymmetry of data with specified customg binning
    data->plotOn(aframe,Asymmetry(mixState),Binning(abins)) ;

    // Plot corresponding property of p.d.f
    bmix.plotOn(aframe,Asymmetry(mixState)) ;

    // Adjust vertical range of plot to sensible values for an asymmetry
    aframe->SetMinimum(-1.1) ;
    aframe->SetMaximum(1.1) ;

    // NB: For asymmetry distributions no density corrects are needed (and are thus not applied)


    regPlot(dtframe,"rf108_plot1") ;
    regPlot(aframe,"rf108_plot2") ;

    delete data ;

    return kTRUE ;
  }
} ;
