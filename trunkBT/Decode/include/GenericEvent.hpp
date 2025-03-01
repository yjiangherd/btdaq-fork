#ifndef GenericEvent_hpp
#define GenericEvent_hpp

#include <bitset>
#include <cmath>
#include <iostream>
#include <unistd.h>

#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include "TMinuit.h"
#include "TRotation.h"

#include "Cluster.hh"
#include "GenericEvent.hh"

#include "RHClass.hpp"

namespace {
// they store temporarily the result of the fit----------------------------
double mS_sf, mSerr_sf;
double mK_sf, mKerr_sf;
double iDirS_sf, iDirSerr_sf;
double iDirK_sf, iDirKerr_sf;
double iDirZ_sf, iDirZerr_sf;
double theta_sf, thetaerr_sf;
double phi_sf, phierr_sf;
double S0_sf, S0err_sf;
double K0_sf, K0err_sf;
double chisq_sf;
double chisqS_sf;
double chisqK_sf;
std::vector<std::pair<int, std::pair<double, double>>> v_trackS_sf;
std::vector<std::pair<int, std::pair<double, double>>> v_trackK_sf;
std::vector<double> v_trackErrS_sf;
std::vector<double> v_trackErrK_sf;
std::vector<double> v_chilayS_sf;
std::vector<double> v_chilayK_sf;
//-------------------------------------------------------------------------
} // namespace

static inline double _compchisq(std::vector<std::pair<int, std::pair<double, double>>> vec,
                                std::vector<double> &v_chilay, double iDir, double iS, std::vector<double> iSerr,
                                double Z0 = 0);
// static double _compchisq(std::vector<std::pair<int, std::pair<double, double> > > vec, std::vector<double>& v_chilay,
// double iDir, double iS, double iSerr, double Z0=0);
static Double_t _func(double z, double imS, double iS, double Z0 = 0);
#ifdef USEMINUIT
static void _fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag);
#endif

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
bool GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ladderconfnotread = true;

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
bool GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::alignmentnotread = true;

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
bool GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::gaincorrectionnotread = true;

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
int GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::_eventkind = 0;

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
Array3<float, NJINF, NTDRS, NCHAVA *(NVASS + NVASK)> GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::CalSigma{
    {{0}}};

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
Array3<float, NJINF, NTDRS, NCHAVA *(NVASS + NVASK)> GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::CalPed{
    {{0}}};

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
Array3<int, NJINF, NTDRS, NCHAVA *(NVASS + NVASK)> GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::CalStatus{
    {{0}}};

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
template <class calib>
typename GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::template calsarray<calib>
GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCalibrationsArrayFromFile(TFile *file) {
  if (file->FindKey("cals")) {
    return file->Get<Calibrations<NJINF, NTDRS, (NVASS + NVASK) * NCHAVA>>("cals")->GetArray();
  }

  throw(std::runtime_error("No calibrations saved in this TFile"));
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
Calibrations<NJINF, NTDRS, (NVASS + NVASK) * NCHAVA>
GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCalibrationsFromFile(TFile *file) {
  if (file->FindKey("cals")) {
    return *(file->Get<Calibrations<NJINF, NTDRS, (NVASS + NVASK) * NCHAVA>>("cals"));
  }

  throw(std::runtime_error("No calibrations saved in this TFile"));
}

// NOTE: This constructor should not be used, if you create a new Event flavor, specialize its constructor as shown
// below [VF]
template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GenericEvent(const char *ladderconf, const char *gaincorr) {
  _eventkind = 0;

  Cls = new TClonesArray("Cluster", NJINF * NTDRS); // if more than NJINFS*NTDRS anyhow the array will be expanded
  Cls->SetOwner();

  _NTrks = 0;
  ClearTrack();
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::~GenericEvent() {
  if (Cls) {
    Cls->Delete();
    delete Cls;
  }
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::Clear() {
  JINJStatus = 0;
  for (size_t jj = 0; jj < NJINF; jj++) {
    JINFStatus[jj] = 0;
    for (size_t ii = 0; ii < NTDRS; ii++) {
      TDRStatus[jj][ii] = 31;
    }
  }

  NClusTot = 0;

  for (size_t jj = 0; jj < NJINF; jj++) {
    // Viviana: hardcoded
    // ->kk to run onto nlayers?
    for (size_t ii = 0; ii < NTDRS; ii++) { // Viviana: was kk<8
      ReadTDR[jj][ii] = 0;
      ValidTDR[jj][ii] = false;
      for (size_t iv = 0; iv < (NVASS + NVASK); iv++)
        CNoise[jj][ii][iv] = 0;
      NClus[jj][ii][0] = 0;
      NClus[jj][ii][1] = 0;
      for (size_t kk = 0; kk < (NVASS + NVASK) * NCHAVA; kk++) { // Viviana: was 1024
        CalSigma[jj][ii][kk] = 0.0;
        CalPed[jj][ii][kk] = 0.0;
        RawSignal[jj][ii][kk] = 0;
        RawSoN[jj][ii][kk] = 0.0;
        CalStatus[jj][ii][kk] = 0;
      }
    }
  }

  if (Cls)
    Cls->Delete();

  ClearTrack();
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
Cluster *GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::AddCluster(int Jinfnum, int lad, int side) {
  Cluster *pp = (Cluster *)Cls->New(NClusTot);
  pp->SetNChannels(0, NCHAVA * NVASS);
  pp->SetNChannels(1, NCHAVA * NVASK);
  NClus[Jinfnum][lad][side]++;
  NClusTot++;
  return pp;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
Cluster *GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCluster(int ii) {
  return (Cluster *)Cls->At(ii);
}

/*
int Event::NGoldenClus(int lad, int side){
  int num=0;
  for (int ii=0;ii<NClusTot;ii++){
    Cluster* cl=GetCluster(ii);
    if(cl->ladder==lad&&cl->side==side&&cl->golden==1) num++;
  }
  return num;
}
*/

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ReadLadderConf(TString filename, bool DEBUG) {

  printf("Reading ladder configuration from %s:\n", filename.Data());

  LadderConf *ladderconf = LadderConf::Instance();
  ladderconf->InitSize(NJINF, NTDRS);
  ladderconf->Init(filename.Data(), DEBUG);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ReadAlignment(TString filename, bool DEBUG) {

  auto *alignmentPars = AlignmentPars::Instance();
  alignmentPars->InitSize(NJINF, NTDRS);
  alignmentPars->Init(filename.Data());

  alignmentnotread = false;

  if (!DEBUG)
    return;

  for (int jj = 0; jj < NJINF; jj++) {
    for (int tt = 0; tt < NTDRS; tt++) {
      for (int cc = 0; cc < 3; cc++) {
        if (cc == 0)
          printf("JINF %02d TDR %02d)\t", jj, tt);
        printf("%f\t", alignmentPars->GetPar(jj, tt, cc));
        if (cc == 2)
          printf("%d\n", (int)(alignmentPars->GetMultiplicityFlip(jj, tt)));
      }
    }
  }

  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ReadGainCorrection(TString filename, bool DEBUG) {

  auto *gainCorrectionPars = GainCorrectionPars::Instance();
  gainCorrectionPars->InitSize(NJINF, NTDRS, NVASS, NVASK);
  gainCorrectionPars->Init(filename.Data());

  gaincorrectionnotread = false;

  //  if(DEBUG==false) return;
  // per ora (finche' il lavoro non e' finito) utile mostrare la tabellina dei TDR  con valori non di default, perchè
  // NON dovrebbero esserci!
  bool first = true;
  bool everdone = false;
  for (size_t jj = 0; jj < NJINF; jj++) {
    for (size_t tt = 0; tt < NTDRS; tt++) {
      for (size_t vv = 0; vv < (NVASS + NVASK); vv++) {
        if (gainCorrectionPars->GetPar(jj, tt, vv, 0) == 0.0 && gainCorrectionPars->GetPar(jj, tt, vv, 1) == 1.0)
          continue;
        if (first) {
          printf("***************************************\n");
          printf("***************************************\n");
          printf("Non-default gain correction parameters:\n");
        }
        first = false;
        everdone = true;
        printf("JINF %02ld TDR %02ld VA %02ld)\t", jj, tt, vv);
        printf("%f\t", gainCorrectionPars->GetPar(jj, tt, vv, 0));
        printf("%f\t", gainCorrectionPars->GetPar(jj, tt, vv, 1));
        printf("\n");
      }
    }
  }
  if (everdone) {
    printf("***************************************\n");
    printf("***************************************\n");
  }

  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
float GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetGainCorrectionPar(int jinfnum, int tdrnum, int vanum,
                                                                                    int component) {
  if (jinfnum >= NJINF || jinfnum < 0) {
    printf("Jinf %d: not possible, the maximum is %d...\n", jinfnum, NJINF - 1);
    return -9999;
  }
  if (tdrnum >= NTDRS || tdrnum < 0) {
    printf("TDR %d: not possible, the maximum is %d...\n", tdrnum, NTDRS - 1);
    return -9999;
  }
  if (vanum >= (NVASS + NVASK) || vanum < 0) {
    printf("VA %d: not possible, the maximum is %d...\n", vanum, (NVASS + NVASK) - 1);
    return -9999;
  }
  if (component < 0 || component >= 3) {
    printf("Component %d not valid: it can be only up to 2\n", component);
    return -9999;
  }

  auto *gainCorrectionPars = GainCorrectionPars::Instance();

  return gainCorrectionPars->GetPar(jinfnum, tdrnum, vanum, component);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
float GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetAlignPar(int jinfnum, int tdrnum, int component) {

  if (jinfnum >= NJINF || jinfnum < 0) {
    printf("Jinf %d: not possible, the maximum is %d...\n", jinfnum, NJINF - 1);
    return -9999;
  }
  if (tdrnum >= NTDRS || tdrnum < 0) {
    printf("TDR %d: not possible, the maximum is %d...\n", tdrnum, NTDRS - 1);
    return -9999;
  }
  if (component < 0 || component >= 3) {
    printf("Component %d not valid: it can be only up to 2\n", component);
    return -9999;
  }

  auto *alignmentPars = AlignmentPars::Instance();

  return alignmentPars->GetPar(jinfnum, tdrnum, component);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
float GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetMultiplicityFlip(int jinfnum, int tdrnum) {

  if (jinfnum >= NJINF || jinfnum < 0) {
    printf("Jinf %d: not possible, the maximum is %d...\n", jinfnum, NJINF - 1);
    return -9999;
  }
  if (tdrnum >= NTDRS || tdrnum < 0) {
    printf("TDR %d: not possible, the maximum is %d...\n", tdrnum, NTDRS - 1);
    return -9999;
  }

  LadderConf *ladderconf = LadderConf::Instance();

  return ladderconf->GetMultiplicityFlip(jinfnum, tdrnum);
  // return multflip[jinfnum][tdrnum];
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ClearTrack() {
  // CB:
  _TrS.clear();
  _TrK.clear();
  _vertexK = std::make_pair(9999., 9999.);
  _vertexS = std::make_pair(9999., 9999.);

  _chisq = 999999999.9;
  _chisqS = 999999999.9;
  _chisqK = 999999999.9;

  _iDirS = -9999.9;
  _iDirK = -9999.9;
  _iDirZ = -9999.9;
  _iDirSerr = -9999.9;
  _iDirKerr = -9999.9;
  _iDirZerr = -9999.9;

  _mS = -9999.9;
  _mK = -9999.9;
  _mSerr = -9999.9;
  _mKerr = -9999.9;

  _theta = -9999.9;
  _phi = -9999.9;
  _thetaerr = -9999.9;
  _phierr = -9999.9;

  _S0 = -9999.9;
  _K0 = -9999.9;
  _S0err = -9999.9;
  _K0err = -9999.9;

  _v_trackS.clear();
  _v_trackK.clear();

  _v_chilayS.clear();
  _v_chilayK.clear();

  _v_trackhit.clear();

  // not to be cleared since ExcludeTDRFromTrack must be called before the event loop!
  //  _v_ladderS_to_ignore.clear();
  //  _v_ladderK_to_ignore.clear();

  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ClearTrack_sf() {

  chisq_sf = 999999999.9;
  chisqS_sf = 999999999.9;
  chisqK_sf = 999999999.9;

  iDirS_sf = -9999.9;
  iDirK_sf = -9999.9;
  iDirZ_sf = -9999.9;
  iDirSerr_sf = -9999.9;
  iDirKerr_sf = -9999.9;
  iDirZerr_sf = -9999.9;

  mS_sf = -9999.9;
  mK_sf = -9999.9;
  mSerr_sf = -9999.9;
  mKerr_sf = -9999.9;

  theta_sf = -9999.9;
  phi_sf = -9999.9;
  thetaerr_sf = -9999.9;
  phierr_sf = -9999.9;

  S0_sf = -9999.9;
  K0_sf = -9999.9;
  S0err_sf = -9999.9;
  K0err_sf = -9999.9;

  v_trackS_sf.clear();
  v_trackK_sf.clear();

  v_trackErrS_sf.clear();
  v_trackErrK_sf.clear();

  v_chilayS_sf.clear();
  v_chilayK_sf.clear();

  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ExcludeTDRFromTrack(int jinfnum, int tdrnum, int side,
                                                                                  bool verbose) {

  if (verbose)
    printf("From now on excluding JINF=%d, TDR=%d, Side=%d\n", jinfnum, tdrnum, side);

  int item = jinfnum * 100 + tdrnum;

  if (side == 0) {
    _v_ladderS_to_ignore.push_back(item);
  } else {
    _v_ladderK_to_ignore.push_back(item);
  }

  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::IncludeBackTDRFromTrack(int jinfnum, int tdrnum, int side,
                                                                                      bool verbose) {

  if (verbose)
    printf("From now on including back JINF=%d, TDR=%d, Side=%d\n", jinfnum, tdrnum, side);

  int item = jinfnum * 100 + tdrnum;

  if (side == 0) {
    _v_ladderS_to_ignore.erase(std::remove(_v_ladderS_to_ignore.begin(), _v_ladderS_to_ignore.end(), item),
                               _v_ladderS_to_ignore.end());
  } else {
    _v_ladderK_to_ignore.erase(std::remove(_v_ladderK_to_ignore.begin(), _v_ladderK_to_ignore.end(), item),
                               _v_ladderK_to_ignore.end());
  }

  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
bool GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::FindTrackAndFit(int nptsS, int nptsK, bool verbose) {

  ClearTrack();

  std::vector<std::pair<int, std::pair<double, double>>> **v_cog_laddS;
  std::vector<std::pair<int, std::pair<double, double>>> **v_cog_laddK;
  v_cog_laddS = new std::vector<std::pair<int, std::pair<double, double>>> *[NJINF];
  v_cog_laddK = new std::vector<std::pair<int, std::pair<double, double>>> *[NJINF];
  for (int ii = 0; ii < NJINF; ii++) {
    v_cog_laddS[ii] = new std::vector<std::pair<int, std::pair<double, double>>>[NTDRS];
    v_cog_laddK[ii] = new std::vector<std::pair<int, std::pair<double, double>>>[NTDRS];
  }

  for (int index_cluster = 0; index_cluster < NClusTot; index_cluster++) {

    Cluster *current_cluster = GetCluster(index_cluster);

    int jinfnum = current_cluster->GetJinf();
    int tdrnum = current_cluster->GetTDR();
    int item = jinfnum * 100 + tdrnum;

    int side = current_cluster->side;
    if (side == 0) {
      if (!(std::find(_v_ladderS_to_ignore.begin(), _v_ladderS_to_ignore.end(), item) != _v_ladderS_to_ignore.end())) {
        v_cog_laddS[jinfnum][tdrnum].push_back(
            std::make_pair(index_cluster, std::make_pair(current_cluster->GetAlignedPosition(),
                                                         AlignmentPars::Instance()->GetPar(jinfnum, tdrnum, 2))));
      }
    } else {
      if (!(std::find(_v_ladderK_to_ignore.begin(), _v_ladderK_to_ignore.end(), item) != _v_ladderK_to_ignore.end())) {
        v_cog_laddK[jinfnum][tdrnum].push_back(
            std::make_pair(index_cluster, std::make_pair(current_cluster->GetAlignedPosition(),
                                                         AlignmentPars::Instance()->GetPar(jinfnum, tdrnum, 2))));
      }
    }
  }

  /*
  for (int jinfnum=0; jinfnum<NJINF; jinfnum++) {
    for (int tdrnum=0; tdrnum<NTDRS; tdrnum++) {
      printf("JINF: %d, TDR: %d: ", jinfnum, tdrnum);
      for (int ss=0; ss<(int)(v_cog_laddS[jinfnum][tdrnum].size()); ss++) {
        printf("(%d, (%f,%f)) ", v_cog_laddS[jinfnum][tdrnum].at(ss).first,
  v_cog_laddS[jinfnum][tdrnum].at(ss).second.first, v_cog_laddS[jinfnum][tdrnum].at(ss).second.second);
      }
      for (int kk=0; kk<(int)(v_cog_laddK[jinfnum][tdrnum].size()); kk++) {
        printf("(%d, (%f,%f)) ", v_cog_laddK[jinfnum][tdrnum].at(kk).first,
  v_cog_laddK[jinfnum][tdrnum].at(kk).second.first, v_cog_laddK[jinfnum][tdrnum].at(kk).second.second);
      }
      printf("\n");
    }
  }
  */

  std::vector<std::pair<int, std::pair<double, double>>>
      vecS; // actually used just for compatibility with the telescopic function
  std::vector<std::pair<int, std::pair<double, double>>>
      vecK; // actually used just for compatibility with the telescopic function
  double chisq = CombinatorialFit(v_cog_laddS, v_cog_laddK, NJINF, NTDRS, vecS, vecK, nptsS, nptsK, verbose);
  //  printf("chisq = %f\n", chisq);
  //  sleep(10);

  bool ret = true;
  if (chisq >= 999999999.9)
    ret = false;
  else if (chisq < -0.000000001)
    ret = false;

  return ret;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
bool GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::FindHigherChargeTrackAndFit(int nptsS, double threshS,
                                                                                          int nptsK, double threshK,
                                                                                          bool verbose) {

  ClearTrack();

  std::vector<std::pair<int, std::pair<double, double>>> **v_cog_laddS;
  std::vector<std::pair<int, std::pair<double, double>>> **v_cog_laddK;

  std::vector<double> **v_q_laddS;
  std::vector<double> **v_q_laddK;

  v_cog_laddS = new std::vector<std::pair<int, std::pair<double, double>>> *[NJINF];
  v_cog_laddK = new std::vector<std::pair<int, std::pair<double, double>>> *[NJINF];
  v_q_laddS = new std::vector<double> *[NJINF];
  v_q_laddK = new std::vector<double> *[NJINF];
  for (int ii = 0; ii < NJINF; ii++) {
    v_cog_laddS[ii] = new std::vector<std::pair<int, std::pair<double, double>>>[NTDRS];
    v_cog_laddK[ii] = new std::vector<std::pair<int, std::pair<double, double>>>[NTDRS];
    v_q_laddS[ii] = new std::vector<double>[NTDRS];
    v_q_laddK[ii] = new std::vector<double>[NTDRS];
  }

  for (int index_cluster = 0; index_cluster < NClusTot; index_cluster++) {

    Cluster *current_cluster = GetCluster(index_cluster);

    int jinfnum = current_cluster->GetJinf();
    int tdrnum = current_cluster->GetTDR();
    int item = jinfnum * 100 + tdrnum;

    int side = current_cluster->side;
    if (side == 0) {
      if (!(std::find(_v_ladderS_to_ignore.begin(), _v_ladderS_to_ignore.end(), item) != _v_ladderS_to_ignore.end())) {
        if (current_cluster->GetTotSN() > threshS) {
          if (v_q_laddS[jinfnum][tdrnum].size() == 0) {
            v_cog_laddS[jinfnum][tdrnum].push_back(
                std::make_pair(index_cluster, std::make_pair(current_cluster->GetAlignedPosition(),
                                                             AlignmentPars::Instance()->GetPar(jinfnum, tdrnum, 2))));
            v_q_laddS[jinfnum][tdrnum].push_back(current_cluster->GetCharge());
          } else {
            if (current_cluster->GetCharge() > v_q_laddS[jinfnum][tdrnum][0]) {
              v_cog_laddS[jinfnum][tdrnum][0] =
                  std::make_pair(index_cluster, std::make_pair(current_cluster->GetAlignedPosition(),
                                                               AlignmentPars::Instance()->GetPar(jinfnum, tdrnum, 2)));
              v_q_laddS[jinfnum][tdrnum][0] = current_cluster->GetCharge();
            }
          }
        }
      }
    } else {
      if (!(std::find(_v_ladderK_to_ignore.begin(), _v_ladderK_to_ignore.end(), item) != _v_ladderK_to_ignore.end())) {
        if (current_cluster->GetTotSN() > threshK) {
          if (v_q_laddK[jinfnum][tdrnum].size() == 0) {
            v_cog_laddK[jinfnum][tdrnum].push_back(
                std::make_pair(index_cluster, std::make_pair(current_cluster->GetAlignedPosition(),
                                                             AlignmentPars::Instance()->GetPar(jinfnum, tdrnum, 2))));
            v_q_laddK[jinfnum][tdrnum].push_back(current_cluster->GetCharge());
          } else {
            if (current_cluster->GetCharge() > v_q_laddK[jinfnum][tdrnum][0]) {
              v_cog_laddK[jinfnum][tdrnum][0] =
                  std::make_pair(index_cluster, std::make_pair(current_cluster->GetAlignedPosition(),
                                                               AlignmentPars::Instance()->GetPar(jinfnum, tdrnum, 2)));
              v_q_laddK[jinfnum][tdrnum][0] = current_cluster->GetCharge();
            }
          }
        }
      }
    }
  }

  // let's use CombinatorialFit just for simplicity but the std::vector above are with at most one cluster per ladder
  std::vector<std::pair<int, std::pair<double, double>>>
      vecS; // actually used just for compatibility with the telescopic function
  std::vector<std::pair<int, std::pair<double, double>>>
      vecK; // actually used just for compatibility with the telescopic function
  double chisq = CombinatorialFit(v_cog_laddS, v_cog_laddK, NJINF, NTDRS, vecS, vecK, nptsS, nptsK, verbose);
  //  printf("chisq = %f\n", chisq);

  bool ret = true;
  if (chisq >= 999999999.9)
    ret = false;
  else if (chisq < -0.000000001)
    ret = false;

  return ret;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::CombinatorialFit(
    std::vector<std::pair<int, std::pair<double, double>>> **v_cog_laddS,
    std::vector<std::pair<int, std::pair<double, double>>> **v_cog_laddK, int ijinf, int itdr,
    std::vector<std::pair<int, std::pair<double, double>>> v_cog_trackS,
    std::vector<std::pair<int, std::pair<double, double>>> v_cog_trackK, int nptsS, int nptsK, bool verbose) {
  //    printf("ijinf = %d, itdr = %d\n", ijinf, itdr);

  if (itdr == 0) {
    itdr = NTDRS;
    ijinf--;
  }

  if (ijinf != 0) { // recursion
    int sizeS = v_cog_laddS[ijinf - 1][itdr - 1].size();
    int sizeK = v_cog_laddK[ijinf - 1][itdr - 1].size();
    //        printf("size: %d %d\n", sizeS, sizeK);
    for (int ss = 0; ss < std::max(sizeS, 1); ss++) {
      for (int kk = 0; kk < std::max(sizeK, 1); kk++) {
        //	printf("ss=%d, kk=%d\n", ss, kk);
        std::vector<std::pair<int, std::pair<double, double>>> _vecS = v_cog_trackS;
        std::vector<std::pair<int, std::pair<double, double>>> _vecK = v_cog_trackK;
        if (sizeS > 0) {
          _vecS.push_back(v_cog_laddS[ijinf - 1][itdr - 1].at(ss));
          if (verbose)
            printf("S_push_back: %f %f\n", v_cog_laddS[ijinf - 1][itdr - 1].at(ss).second.first,
                   v_cog_laddS[ijinf - 1][itdr - 1].at(ss).second.second);
        }
        if (sizeK > 0) {
          _vecK.push_back(v_cog_laddK[ijinf - 1][itdr - 1].at(kk));
          if (verbose)
            printf("K_push_back: %f %f\n", v_cog_laddK[ijinf - 1][itdr - 1].at(kk).second.first,
                   v_cog_laddK[ijinf - 1][itdr - 1].at(kk).second.second);
        }
        CombinatorialFit(v_cog_laddS, v_cog_laddK, ijinf, itdr - 1, _vecS, _vecK, nptsS, nptsK, verbose);
      }
    }
  } else { // now is time to fit!
    if (verbose) {
      printf("new track to fit\n");
      printf("S: ");
      for (int ss = 0; ss < (int)(v_cog_trackS.size()); ss++) {
        printf("(%f,%f)", v_cog_trackS.at(ss).second.first, v_cog_trackS.at(ss).second.second);
      }
      printf("\n");
      printf("K: ");
      for (int kk = 0; kk < (int)(v_cog_trackK.size()); kk++) {
        printf("(%f,%f)", v_cog_trackK.at(kk).second.first, v_cog_trackK.at(kk).second.second);
      }
      printf("\n");
    }
    if ((int)(v_cog_trackS.size()) >= nptsS && (int)(v_cog_trackK.size()) >= nptsK) {
      double chisq = SingleFit(v_cog_trackS, v_cog_trackK, verbose);
      if (chisq < _chisq) {
        if (verbose)
          printf("Best track) new chisq %f, old one %f\n", chisq, _chisq);
        AssignAsBestTrackFit();
      }
    }
    if (verbose) {
      printf("----------------------\n");
    }
  }

  return _chisq;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::AssignAsBestTrackFit() {

  _chisq = chisq_sf;
  _chisqS = chisqS_sf;
  _chisqK = chisqK_sf;
  _mS = mS_sf;
  _mK = mK_sf;
  _mSerr = mSerr_sf;
  _mKerr = mKerr_sf;
  _iDirS = iDirS_sf;
  _iDirK = iDirK_sf;
  _iDirZ = iDirZ_sf;
  _iDirSerr = iDirSerr_sf;
  _iDirKerr = iDirKerr_sf;
  _iDirZerr = iDirZerr_sf;
  _theta = theta_sf;
  _phi = phi_sf;
  _thetaerr = thetaerr_sf;
  _phierr = phierr_sf;
  _S0 = S0_sf;
  _K0 = K0_sf;
  _S0err = S0err_sf;
  _K0err = K0err_sf;
  _v_trackS = v_trackS_sf;
  _v_trackK = v_trackK_sf;
  _v_chilayS = v_chilayS_sf;
  _v_chilayK = v_chilayK_sf;

  StoreTrackClusterPatterns();
  FillHitVector();

  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::SingleFit(
    std::vector<std::pair<int, std::pair<double, double>>> vS,
    std::vector<std::pair<int, std::pair<double, double>>> vK, bool verbose) {

  ClearTrack_sf();

  /* debug
     static TH1F hchi("hchi", "hchi", 1000, 0.0, 10.0);
     static TH1F htheta("htheta", "htheta", 1000, -TMath::Pi()/2.0, TMath::Pi()/2.0);
     static TH1F hphi("hphi", "hphi", 1000, -TMath::Pi(), TMath::Pi());
     static TH1F hs0("hs0", "hs0", 1000, -1000.0, 1000.0);
     static TH1F hk0("hk0", "hk0", 1000, -1000.0, 1000.0);
  */

  chisq_sf = SingleFit(vS, vK, v_chilayS_sf, v_chilayK_sf, theta_sf, thetaerr_sf, phi_sf, phierr_sf, iDirS_sf,
                       iDirSerr_sf, iDirK_sf, iDirKerr_sf, iDirZ_sf, iDirZerr_sf, mS_sf, mSerr_sf, mK_sf, mKerr_sf,
                       S0_sf, S0err_sf, K0_sf, K0err_sf, chisqS_sf, chisqK_sf, verbose);

  /*
    hchi.Fill(log10(chisq_sf));
    htheta.Fill(theta_sf);
    hphi.Fill(phi_sf);
    hs0.Fill(S0_sf);
    hk0.Fill(K0_sf);
  */

  if (verbose)
    printf("chisq: %f, chisqS: %f, chisqK: %f, theta = %f, phi = %f, S0 = %f, K0 = %f\n", chisq_sf, chisqS_sf,
           chisqK_sf, theta_sf, phi_sf, S0_sf, K0_sf);

  return chisq_sf;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::SingleFit(
    std::vector<std::pair<int, std::pair<double, double>>> vS,
    std::vector<std::pair<int, std::pair<double, double>>> vK, std::vector<double> &v_chilayS,
    std::vector<double> &v_chilayK, double &theta, double &thetaerr, double &phi, double &phierr, double &iDirX,
    double &iDirXerr, double &iDirY, double &iDirYerr, double &iDirZ, double &iDirZerr, double &mX, double &mXerr,
    double &mY, double &mYerr, double &X0, double &X0err, double &Y0, double &Y0err, double &chisqS, double &chisqK,
    bool verbose) {

  v_trackS_sf = vS;
  v_trackK_sf = vK;

  for (int ic = 0; ic < (int)vS.size(); ic++)
    v_trackErrS_sf.push_back(GetCluster(vS.at(ic).first)->GetNominalResolution(0));
  for (int ic = 0; ic < (int)vK.size(); ic++)
    v_trackErrK_sf.push_back(GetCluster(vK.at(ic).first)->GetNominalResolution(1));

  Double_t corrXmX, corrYmY;

  // The fit is done independently in the two X and Y views
  // The fit returns X0 and mX (the angular coefficient)
  // mX = vx/vz, where vx and vz are the projection of the straight line versors into the X and Z axis
  // Considering the definition of directive cosines
  // dirX = vx / |v|   with |v| = sqrt( vx*vx + vy*vy + vz*vz)
  // dirY = vy / |v|   with |v| = sqrt( vx*vx + vy*vy + vz*vz)
  // dirZ = vZ / |v|   with |v| = sqrt( vx*vx + vy*vy + vz*vz)
  // dirX*dirX + dirY*dirY + dirZ*dirZ = 1
  // then we have
  // mX = dirX/dirZ
  // mY = dirY/dirZ
  // and after both two fits, we can calculate
  // dirZ = 1  / sqrt( 1 + mX*mX + mY*mY)
  // dirX = mX / sqrt( 1 + mX*mX + mY*mY)
  // dirY = mY / sqrt( 1 + mX*mX + mY*mY)

#ifdef USEMINUIT
  // Minuit fit
  static TMinuit *minuit = NULL;
  if (!minuit)
    minuit = new TMinuit();
  //  minuit->Clear();
  minuit->SetPrintLevel((int)(verbose)-1);
  minuit->SetFCN(_fcn);

  Double_t arglist[10];
  Int_t ierflg = 0;
  arglist[0] = 1; // chi-sq err-def
  minuit->mnexcm("SET ERR", arglist, 1, ierflg);

  // Set starting values and step sizes for parameters
  static Double_t vstart[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
  static Double_t step[5] = {1.0e-5, 1.0e-5, 1.0e-5, 1.0e-5};
  minuit->mnparm(0, "mS", vstart[0], step[0], 0, 0, ierflg);
  minuit->mnparm(1, "mK", vstart[1], step[1], 0, 0, ierflg);
  minuit->mnparm(2, "S0", vstart[2], step[2], 0, 0, ierflg);
  minuit->mnparm(3, "K0", vstart[3], step[3], 0, 0, ierflg);

  // Now ready for minimization step
  arglist[0] = 50000;
  arglist[1] = 1.;
  minuit->mnexcm("MIGRAD", arglist, 2, ierflg);

  // Print results
  // Double_t amin,edm,errdef;
  // Int_t nvpar,nparx,icstat;
  // minuit->mnstat(amin, edm, errdef, nvpar, nparx, icstat);
  // minuit->mnprin(3,amin);

  minuit->GetParameter(0, mX, mXerr);
  minuit->GetParameter(1, mY, mYerr);
  minuit->GetParameter(2, X0, X0err);
  minuit->GetParameter(3, Y0, Y0err);

  Double_t covmat[4][4];
  minuit->mnemat(&covmat[0][0], 4);
  corrXmX = -covmat[0][2] / (sqrt(covmat[0][0] * covmat[2][2])); // minus is because they shold be anticorrelated
  corrYmY = -covmat[1][3] / (sqrt(covmat[1][1] * covmat[3][3]));

#else
  // Analytical Fit

  // Viviana claimed that everything following was REVERSED (S<->K). I imported the newer version, erasing Viviana's
  // one. Check if is correct.

  Double_t S1 = 0, Sz = 0, Szz = 0;

  // Fit X
  int nx = (int)(vS.size());
  // Why 5 loops when you can do just one... WHY?!?!?!?
  /*
    Double_t S1=0;   for(int i=0; i<(int)nx; i++) S1  += 1./pow(Cluster::GetNominalResolution(0),2);
    Double_t Sz=0;   for(int i=0; i<(int)nx; i++) Sz  += vS.at(i).second.second/pow(Cluster::GetNominalResolution(0),2);
    Double_t Szz=0;  for(int i=0; i<(int)nx; i++) Szz +=
    pow(vS.at(i).second.second,2)/pow(Cluster::GetNominalResolution(0),2); Double_t Sx=0;   for(int i=0; i<(int)nx; i++)
    Sx  += vS.at(i).second.first/pow(Cluster::GetNominalResolution(0),2); Double_t Szx=0;  for(int i=0; i<(int)nx; i++)
    Szx += (vS.at(i).second.first*vS.at(i).second.second)/pow(Cluster::GetNominalResolution(0),2);
  */
  S1 = 0;
  Sz = 0;
  Szz = 0;
  Double_t Sx = 0, Szx = 0;
  for (int i = 0; i < (int)nx; i++) {
    if (verbose)
      printf("SingleFit: cluster %d, Jinf: %d, TDR %d: Z=%f, X=%f\n", vS.at(i).first,
             GetCluster(vS.at(i).first)->GetJinf(), GetCluster(vS.at(i).first)->GetTDR(), vS.at(i).second.second,
             vS.at(i).second.first);
    double sigmasq = pow(GetCluster(vS.at(i).first)->GetNominalResolution(0), 2);
    S1 += 1. / sigmasq;
    Sz += vS.at(i).second.second / sigmasq;
    Szz += pow(vS.at(i).second.second, 2) / sigmasq;
    Sx += vS.at(i).second.first / sigmasq;
    Szx += (vS.at(i).second.first * vS.at(i).second.second) / sigmasq;
  }

  Double_t Dx = S1 * Szz - Sz * Sz;
  X0 = (Sx * Szz - Sz * Szx) / Dx;
  // iDirX = (S1*Szx-Sz*Sx)/Dx; iDirX=-iDirX;
  mX = (S1 * Szx - Sz * Sx) / Dx; // mX=-mX;
  X0err = sqrt(Szz / Dx);
  // iDirXerr = sqrt(S1/Dx);
  mXerr = sqrt(S1 / Dx);
  corrXmX = -Sz / sqrt(Szz * S1);

  // Fit Y
  int ny = (int)(vK.size());
  // Why 5 loops when you can do just one... WHY?!?!?!?
  /*
    S1=0;   for(int i=0; i<(int)ny; i++) S1  += 1./pow(Cluster::GetNominalResolution(1),2);
    Sz=0;   for(int i=0; i<(int)ny; i++) Sz  += vK.at(i).second.second/pow(Cluster::GetNominalResolution(1),2);
    Szz=0;  for(int i=0; i<(int)ny; i++) Szz += pow(vK.at(i).second.second,2)/pow(Cluster::GetNominalResolution(1),2);
    Double_t Sy=0;   for(int i=0; i<(int)ny; i++) Sy  += vK.at(i).second.first/pow(Cluster::GetNominalResolution(1),2);
    Double_t Szy=0;  for(int i=0; i<(int)ny; i++) Szy +=
    (vK.at(i).second.first*vK.at(i).second.second)/pow(Cluster::GetNominalResolution(1),2);
  */
  S1 = 0;
  Sz = 0;
  Szz = 0;
  Double_t Sy = 0, Szy = 0;
  for (int i = 0; i < (int)ny; i++) {
    if (verbose)
      printf("SingleFit: cluster %d, Jinf: %d, TDR %d: Z=%f, Y=%f\n", vK.at(i).first,
             GetCluster(vK.at(i).first)->GetJinf(), GetCluster(vK.at(i).first)->GetTDR(), vK.at(i).second.second,
             vK.at(i).second.first);
    double sigmasq = pow(GetCluster(vK.at(i).first)->GetNominalResolution(1), 2);
    S1 += 1. / sigmasq;
    Sz += vK.at(i).second.second / sigmasq;
    Szz += pow(vK.at(i).second.second, 2) / sigmasq;
    Sy += vK.at(i).second.first / sigmasq;
    Szy += (vK.at(i).second.first * vK.at(i).second.second) / sigmasq;
  }
  Double_t Dy = S1 * Szz - Sz * Sz;
  Y0 = (Sy * Szz - Sz * Szy) / Dy;
  // iDirY = (S1*Szy-Sz*Sy)/Dy; iDirY=-iDirY;
  mY = (S1 * Szy - Sz * Sy) / Dy; // mY=-mY;
  Y0err = sqrt(Szz / Dy);
  // iDirYerr = sqrt(S1/Dy);
  mYerr = sqrt(S1 / Dy);
  corrYmY = -Sz / sqrt(Szz * S1);
#endif

  //  printf("%f %f %f %f\n", mX, mY, X0, Y0);

  //    dirX = mX * dirZ                 -->       dirX = mX / sqrt(1 + mX^2 + mY^2)
  //    dirY = mY * dirZ                 -->       dirX = mY / sqrt(1 + mX^2 + mY^2)
  //    dirZ = 1./sqrt(1 + mX^2 + mY^2)

  //    ∂dirX/∂mX = +dirZ^3 * (1+mY^2)        ∂dirY/∂mX = -dirZ^3 * mX * mY       ∂dirZ/∂mX = -dirZ^3 * mX
  //    ∂dirX/∂mY = -dirZ^3 * mX * mY         ∂dirY/∂mY = +dirZ^3 * (1+mX^2)      ∂dirZ/∂mY = -dirZ^3 * mY
  //    corr(mX,mY)=0  since they come from independent fits

  iDirZ = 1. / sqrt(1 + mX * mX + mY * mY);
  iDirX = mX * iDirZ;
  iDirY = mY * iDirZ;
  Double_t dDirXdmX = +iDirZ * iDirZ * iDirZ * (1 + mY * mY);
  Double_t dDirXdmY = -iDirZ * iDirZ * iDirZ * mX * mY;
  Double_t dDirYdmX = -iDirZ * iDirZ * iDirZ * mX * mY;
  Double_t dDirYdmY = +iDirZ * iDirZ * iDirZ * (1 + mX * mX);
  Double_t dDirZdmX = -iDirZ * iDirZ * iDirZ * mX;
  Double_t dDirZdmY = -iDirZ * iDirZ * iDirZ * mY;
  iDirXerr = sqrt(pow(dDirXdmX * mXerr, 2) + pow(dDirXdmY * mYerr, 2));
  iDirYerr = sqrt(pow(dDirYdmX * mXerr, 2) + pow(dDirYdmY * mYerr, 2));
  iDirZerr = sqrt(pow(dDirZdmX * mXerr, 2) + pow(dDirZdmY * mYerr, 2));

  //------------------------------------------------------------------------------------------

  theta = std::acos(iDirZ);
  phi = std::atan2(iDirY, iDirX);

  // should not happen ------------
  if (theta < 0) {
    theta = fabs(theta);
    phi += TMath::Pi();
  }
  if (phi > TMath::Pi()) {
    phi -= 2.0 * TMath::Pi();
  }
  if (phi < -TMath::Pi()) {
    phi += 2.0 * TMath::Pi();
  }
  //------------------------------

  // theta = acos( dirZ )        --> theta(mX,mY) = acos( 1./sqrt(1+mX*mX+mY*mY) )
  // phi = atan (dirY/dirX)      --> phi(mX,mY)   = atan( mY/mX )
  //
  // ∂phi/∂mX = -mY / (mX^2 + mY^2)                            ∂phi/∂mY = +mX / (mX^2 + mY^2)
  // ∂theta/∂mX = [(1+mX^2+mY^2)*sqrt(mX^2+mY^2)]^{-1}         ∂theta/∂mY = ∂theta/∂mX

  double dthetadmX = 1. / ((1 + mX * mX * mY * mY) * sqrt(mX * mX + mY * mY));
  double dthetadmY = 1. / ((1 + mX * mX * mY * mY) * sqrt(mX * mX + mY * mY));
  double dphidmX = -mY / (mX * mX + mY * mY);
  double dphidmY = +mX / (mX * mX + mY * mY);
  thetaerr = sqrt(pow(dthetadmX * mXerr, 2) + pow(dthetadmY * mYerr, 2));
  phierr = sqrt(pow(dphidmX * mXerr, 2) + pow(dphidmY * mYerr, 2));

  //------------------------------------------------------------------------------------------

  int ndofS = vS.size() - 2;
  int ndofK = vK.size() - 2;

  double chisqS_nored = 0.0;
  double chisqK_nored = 0.0;
  double chisq = 0.0;

  // Viviana claimed REVERSED X->Y. I imported the newew version, erasing her one. We need to check
  if (ndofS >= 0) {
    chisqS_nored = _compchisq(vS, v_chilayS, mX, X0, v_trackErrS_sf);
    chisq += chisqS_nored;
  }
  if (ndofK >= 0) {
    chisqK_nored = _compchisq(vK, v_chilayK, mY, Y0, v_trackErrK_sf);
    chisq += chisqK_nored;
  }

  int ndof = ndofS + ndofK;
  double ret = chisq / ndof;
  if (ndof <= 0) {
    if (ndofS > 0)
      ret = chisqS_nored / ndofS;
    else if (ndofK > 0)
      ret = chisqK_nored / ndofK;
    else if (ndof == 0)
      ret = 0.0;
    else
      ret = -1.0;
  }
  // Viviana claimed REVERSED X->Y. I imported the newew version, erasing her one. We need to check
  chisqS = -1.0;
  if (ndofS > 0)
    chisqS = chisqS_nored / ndofS;
  else if (ndofS == 0)
    chisqS = 0.0;
  chisqK = -1.0;
  if (ndofK > 0)
    chisqK = chisqK_nored / ndofK;
  else if (ndofK == 0)
    chisqK = 0.0;

  if (verbose)
    printf("chisq/ndof = %f/%d = %f, chisqS/ndofS = %f/%d = %f, chisqK/ndofK = %f/%d = %f\n", chisq, ndof, ret,
           chisqS_nored, ndofS, chisqS, chisqK_nored, ndofK, chisqK);

  return ret;
}

#ifdef USEMINUIT
void _fcn(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag) {

  std::vector<double> v_chilay;

  f = _compchisq(v_trackS_sf, v_chilay, par[0], par[2], v_trackErrS_sf) +
      _compchisq(v_trackK_sf, v_chilay, par[1], par[3], v_trackErrK_sf);

  return;
}
#endif

/*
double _compchisq(std::vector<std::pair<int, std::pair<double, double> > > vec, std::vector<double>& v_chilay, double
imS, double iS, double iSerr, double Z0){

  v_chilay.clear();

  static Double_t chisq;
  chisq = 0.0;
  static Double_t delta;
  delta = 0.0;
  for (int pp=0; pp<(int)(vec.size()); pp++) {
    delta = (vec.at(pp).second.first - _func(vec.at(pp).second.second, imS, iS, Z0))/iSerr;
    v_chilay.push_back(delta*delta);
    chisq += delta*delta;
  }

  return chisq;
}
*/

double _compchisq(std::vector<std::pair<int, std::pair<double, double>>> vec, std::vector<double> &v_chilay, double imS,
                  double iS, std::vector<double> iSerr, double Z0) {

  v_chilay.clear();

  static Double_t chisq;
  chisq = 0.0;
  static Double_t delta;
  delta = 0.0;
  for (int pp = 0; pp < (int)(vec.size()); pp++) {
    delta = (vec.at(pp).second.first - _func(vec.at(pp).second.second, imS, iS, Z0)) / iSerr.at(pp);
    v_chilay.push_back(delta * delta);
    chisq += delta * delta;
  }

  return chisq;
}

Double_t _func(double z, double imS, double iS, double Z0) { return iS + (z - Z0) * imS; }

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ExtrapolateTrack(double z, int component) {
  if (component == 0)
    return _func(z, _mS, _S0);
  else if (component == 1)
    return _func(z, _mK, _K0);
  else
    return -9999.99;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
bool GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::IsClusterUsedInTrack(int index_cluster) {

  //  printf("IsClusterUsedInTrack\n");

  for (int ii = 0; ii < (int)(_v_trackS.size()); ii++) {
    //    printf("%d cluster (S) in track\n", _v_trackS.at(ii).first);
    if (_v_trackS.at(ii).first == index_cluster)
      return true;
  }
  for (int ii = 0; ii < (int)(_v_trackK.size()); ii++) {
    //    printf("%d cluster (K) in track\n", _v_trackK.at(ii).first);
    if (_v_trackK.at(ii).first == index_cluster)
      return true;
  }

  return false;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::StoreTrackClusterPatterns() {

  for (int ii = 0; ii < NJINF; ii++) {
    for (int ss = 0; ss < 2; ss++) {
      _track_cluster_pattern[ii][ss] = 0;
    }
  }

  std::vector<std::pair<int, std::pair<double, double>>> _v_track_tmp;
  _v_track_tmp.clear();

  for (int i_side = 0; i_side < 2; i_side++) {
    if (i_side == 0)
      _v_track_tmp = _v_trackS;
    else if (i_side == 1)
      _v_track_tmp = _v_trackK;

    for (int ii = 0; ii < (int)(_v_track_tmp.size()); ii++) {
      int index_cluster = _v_track_tmp.at(ii).first;
      Cluster *cl = GetCluster(index_cluster);
      int tdrnum = cl->GetTDR();
      int jinfnum = cl->GetJinf();
      //      printf("JINF %d, TDR %d , %d cluster (%d) in track\n", jinfnum, tdrnum, index_cluster, i_side);

      //      unsigned long long int tdr_index = (int)(pow(10.0, (double)(tdrnum)));
      int tdr_index = 1 << tdrnum;
      //      printf("TDR %d %d --> %s\n", tdrnum, i_side, std::bitset<NTDRS>(tdr_index).to_string().c_str());
      if (jinfnum < NJINF && i_side < 2)
        _track_cluster_pattern[jinfnum][i_side] += tdr_index;
      else
        printf("Problem: Jinf %d out of %d and side %d out of %d\n", jinfnum, NJINF, i_side, 2);
    }
  }

  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
bool GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::IsTDRInTrack(int side, int tdrnum, int jinfnum) {
  //  return ((bool)(((unsigned long long int)(_track_cluster_pattern[jinfnum][side]/((int)(pow((double)10,
  //  (double)tdrnum)))))%10));
  return ((bool)(_track_cluster_pattern[jinfnum][side] & (1 << tdrnum)));
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::FillHitVector() {

  _v_trackhit.clear();

  std::pair<int, int> coopair[NJINF * NTDRS];
  for (int pp = 0; pp < NJINF * NTDRS; pp++) {
    coopair[pp].first = -1;
    coopair[pp].second = -1;
  }

  for (int index_cluster = 0; index_cluster < NClusTot; index_cluster++) {

    if (!IsClusterUsedInTrack(index_cluster))
      continue;
    Cluster *current_cluster = GetCluster(index_cluster);

    int ladder = current_cluster->ladder;
    int side = current_cluster->side;

    if (side == 0) {
      coopair[ladder].first = index_cluster;
    } else {
      coopair[ladder].second = index_cluster;
    }
  }

  for (int tt = 0; tt < NJINF * NTDRS; tt++) {
    if (coopair[tt].first >= 0 || coopair[tt].second >= 0) {
      _v_trackhit.push_back(std::make_pair(tt, coopair[tt]));
    }
  }

  return;
}

struct sort_pred {
  bool operator()(const std::pair<int, double> &left, const std::pair<int, double> &right) {
    return left.second < right.second;
  }
};

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::RefineTrack(double nsigmaS, int nptsS, double nsigmaK,
                                                                            int nptsK, bool verbose) {

  std::vector<std::pair<int, std::pair<double, double>>> _v_trackS_tmp = _v_trackS;
  std::vector<std::pair<int, std::pair<double, double>>> _v_trackK_tmp = _v_trackK;

  std::vector<std::pair<int, double>> _v_chilayS_tmp;
  for (unsigned int ii = 0; ii < _v_chilayS.size(); ii++) {
    _v_chilayS_tmp.push_back(std::make_pair(ii, _v_chilayS.at(ii)));
  }
  std::sort(_v_chilayS_tmp.begin(), _v_chilayS_tmp.end(), sort_pred());

  std::vector<std::pair<int, double>> _v_chilayK_tmp;
  for (unsigned int ii = 0; ii < _v_chilayK.size(); ii++) {
    _v_chilayK_tmp.push_back(std::make_pair(ii, _v_chilayK.at(ii)));
  }
  std::sort(_v_chilayK_tmp.begin(), _v_chilayK_tmp.end(), sort_pred());

  if (((int)(_v_trackS_tmp.size())) > nptsS + 1) { // so that even removing one we have at least nptsS hits
    if (sqrt(_v_chilayS_tmp.at(_v_chilayS_tmp.size() - 1).second) >
        nsigmaS) { // if the worst residual is above threshold is removed
      _v_trackS_tmp.erase(_v_trackS_tmp.begin() + _v_chilayS_tmp.at(_v_chilayS_tmp.size() - 1).first);
    }
  }
  if (((int)(_v_trackK_tmp.size())) > nptsK + 1) { // so that even removing one we have at least nptsK hits
    if (sqrt(_v_chilayK_tmp.at(_v_chilayK_tmp.size() - 1).second) >
        nsigmaK) { // if the worst residual is above threshold is removed
      _v_trackK_tmp.erase(_v_trackK_tmp.begin() + _v_chilayK_tmp.at(_v_chilayK_tmp.size() - 1).first);
    }
  }

  double ret = SingleFit(_v_trackS_tmp, _v_trackK_tmp, false);
  AssignAsBestTrackFit();

  return ret;
}

// A TRUNCATED MEAN WOULD BE BETTER BUT STICAZZI FOR NOW...
template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetChargeTrack(int side) {

  if (side < 0 || side > 1) {
    printf("Not a valid side: %d\n", side);
    return -99999.9;
  }

  int npts = 0;
  double charge = 0.0;

  for (unsigned int ii = 0; ii < _v_trackhit.size(); ii++) {
    int index_cluster = -1;
    if (side == 0)
      index_cluster = _v_trackhit.at(ii).second.first;
    else
      index_cluster = _v_trackhit.at(ii).second.second;
    if (index_cluster >= 0) {
      Cluster *cl = GetCluster(index_cluster);
      charge += cl->GetCharge();
      npts++;
    }
  }

  charge /= npts;

  return charge;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCalPed_PosNum(int tdrnum, int channel, int Jinfnum) {
  return CalPed[Jinfnum][tdrnum][channel];
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCalSigma_PosNum(int tdrnum, int channel,
                                                                                   int Jinfnum) {
  return CalSigma[Jinfnum][tdrnum][channel];
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetRawSignal_PosNum(int tdrnum, int channel,
                                                                                    int Jinfnum) {
  return RawSignal[Jinfnum][tdrnum][channel]; // FIX ME: to be substituted with DecodeData::m_adcUnits
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCalStatus_PosNum(int tdrnum, int channel,
                                                                                    int Jinfnum) {
  return CalStatus[Jinfnum][tdrnum][channel];
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCN_PosNum(int tdrnum, int va, int Jinfnum) {

  // Viviana: hardcoded n channel
  // array dimension was 1024
  short int array[4096];
  float arraySoN[4096];
  float pede[4096];
  int status[4096];

  for (int chan = 0; chan < 4096; chan++) {
    array[chan] = RawSignal[Jinfnum][tdrnum][chan];
    arraySoN[chan] = RawSoN[Jinfnum][tdrnum][chan];
    pede[chan] = CalPed[Jinfnum][tdrnum][chan];
    status[chan] = CalStatus[Jinfnum][tdrnum][chan];
  }

  // Viviana: hardcoded number of channels per VA
  // MD: but why '256' hardcoded' and not NCHAVA?
  //  return ComputeCN(64, &(array[va*64]), &(pede[va*64]), &(arraySoN[va*64]), &(status[va*64]));
  return ComputeCN(256, &(array[va * 256]), &(pede[va * 256]), &(arraySoN[va * 256]), &(status[va * 256]));
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
float GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetRawSoN_PosNum(int tdrnum, int channel, int Jinfnum) {
  return (RawSignal[Jinfnum][tdrnum][channel] - CalPed[Jinfnum][tdrnum][channel]) / CalSigma[Jinfnum][tdrnum][channel];
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCalPed(RHClass<NJINF, NTDRS> *rh, int tdrnum,
                                                                          int channel, int Jinfnum) {
  int tdrnumraw = rh->FindPos(tdrnum, Jinfnum);
  return GetCalPed_PosNum(tdrnumraw, channel, Jinfnum);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCalSigma(RHClass<NJINF, NTDRS> *rh, int tdrnum,
                                                                            int channel, int Jinfnum) {
  int tdrnumraw = rh->FindPos(tdrnum, Jinfnum);
  return GetCalSigma_PosNum(tdrnumraw, channel, Jinfnum);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetRawSignal(RHClass<NJINF, NTDRS> *rh, int tdrnum,
                                                                             int channel, int Jinfnum) {
  int tdrnumraw = rh->FindPos(tdrnum, Jinfnum);
  return GetRawSignal_PosNum(tdrnumraw, channel, Jinfnum);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCN(RHClass<NJINF, NTDRS> *rh, int tdrnum, int va,
                                                                      int Jinfnum) {
  int tdrnumraw = rh->FindPos(tdrnum, Jinfnum);
  return GetCN_PosNum(tdrnumraw, va, Jinfnum);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetCalStatus(RHClass<NJINF, NTDRS> *rh, int tdrnum,
                                                                             int va, int Jinfnum) {
  int tdrnumraw = rh->FindPos(tdrnum, Jinfnum);
  return GetCalStatus_PosNum(tdrnumraw, va, Jinfnum);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
float GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetRawSoN(RHClass<NJINF, NTDRS> *rh, int tdrnum,
                                                                         int channel, int Jinfnum) {
  int tdrnumraw = rh->FindPos(tdrnum, Jinfnum);
  return GetRawSoN_PosNum(tdrnumraw, channel, Jinfnum);
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
double GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::ComputeCN(int size, short int *RawSignal, float *pede,
                                                                          float *RawSoN, int *status,
                                                                          double threshold) {

  // ------------- MEDIAN method -------------
  auto fill_signals = [&](float _threshold) {
    std::vector<float> sig;
    for (size_t ich = 0; ich < size_t(size); ++ich) {
      if (RawSoN[ich] < _threshold && status[ich] == 0) {
        sig.push_back(float(RawSignal[ich]) - pede[ich]);
      }
    }

    return sig;
  };

  auto sig = fill_signals(threshold);
  // let's try again with a higher threshold
  while (sig.size() < 2) {
    threshold += 1.0;
    sig = fill_signals(threshold);
  }

  std::sort(begin(sig), end(sig));
  float median = (sig.size() % 2) ? sig[sig.size() / 2] : 0.5f * (sig[(sig.size() / 2) - 1] + sig[sig.size() / 2]);
  return median;

  // ------------- MEAN method -------------
  //  double mean = 0.0;
  //  int n = 0;
  //
  //  for (int ii = 0; ii < size; ii++) {
  //    // printf("%d %f %f %d\n", ii, RawSoN[ii], threshold, status[ii]);
  //    if (RawSoN[ii] < threshold && status[ii] == 0) { // to avoid real signal...
  //      n++;
  //      //      printf("    %d) %f %f\n", ii, RawSignal[ii]/8.0, pede[ii]);
  //      mean += (RawSignal[ii] - pede[ii]);
  //    }
  //  }
  //
  //  if (n > 1) {
  //    mean /= n;
  //  } else { // let's try again with an higher threshold
  //    mean = ComputeCN(size, RawSignal, pede, RawSoN, status, threshold + 1.0);
  //  }
  //  //  printf("    CN = %f\n", mean);
  //
  //  return mean;
}

//------------CB: Qui iniziano le cose che ho aggiunto------------//
template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
bool GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::FindTracksAndVertex(bool vertmode) {
  ClearTrack(); // tra le altre cose fa il clear di _TrS e _TrK
  ////////////////////////
  // PHASE 1.a: Hits std::vector//
  ////////////////////////
  int NS = 0, NK = 0;
  // in these std::vectors we collect the positions of every cluster detected as hits
  std::vector<std::pair<int, std::pair<double, double>>> v_hitsS;
  std::vector<std::pair<int, std::pair<double, double>>> v_hitsK;
  // this loop make all the work (copied from FindTrackAndFit())
  for (int ic = 0; ic < NClusTot; ic++) {
    Cluster *cc = GetCluster(ic);

    int jinfnum = cc->GetJinf();
    int tdrnum = cc->GetTDR();
    int item = jinfnum * 100 + tdrnum;

    double noise_threshold = 0;  // 20;
    double mixed_threshold = 30; // 180;
    if (tdrnum < NTDRS - 4 && cc->GetTotSig() > noise_threshold) {
      int side = cc->side;
      if (side == 0) { // ( index, ( X or Y, Z ) )
        if (find(_v_ladderS_to_ignore.begin(), _v_ladderS_to_ignore.end(), item) == _v_ladderS_to_ignore.end()) {
          v_hitsS.push_back(make_pair(
              ic, std::make_pair(cc->GetAlignedPositionMC(), AlignmentPars::Instance()->GetPar(jinfnum, tdrnum, 2))));
          // we double a cluster if it's likely that it's been generated by two merged hits
          if (cc->GetTotSig() > mixed_threshold && (ic == NClusTot - 1 || tdrnum != GetCluster(ic + 1)->GetTDR()) &&
              (ic == 0 || tdrnum != GetCluster(ic - 1)->GetTDR()))
            v_hitsS.push_back(v_hitsS.back());
        }
      } else {
        if (find(_v_ladderK_to_ignore.begin(), _v_ladderK_to_ignore.end(), item) == _v_ladderK_to_ignore.end()) {
          v_hitsK.push_back(make_pair(
              ic, std::make_pair(cc->GetAlignedPositionMC(), AlignmentPars::Instance()->GetPar(jinfnum, tdrnum, 2))));
          if (cc->GetTotSig() > mixed_threshold && (ic == NClusTot - 1 || tdrnum != GetCluster(ic + 1)->GetTDR()) &&
              (ic == 0 || tdrnum != GetCluster(ic - 1)->GetTDR()))
            v_hitsK.push_back(v_hitsK.back());
        }
      }
    }
  } // for loop on clusters
  printf("L'evento ha generato %lu(S) + %lu(K) = %d cluster\n", v_hitsS.size(), v_hitsK.size(), NClusTot);

  if (v_hitsS.size() < 5 || v_hitsK.size() < 5) {
    printf("non arriviamo ad almeno 5 per piano\n");
    return false;
  }

  // we double the first cluster if it is the only one in its ladder-> this should allow a better vertex reco
  int ppside = GetCluster(0)->side, vert = 0;
  if (!ppside) {
    if (v_hitsS[0].second.second != v_hitsS[1].second.second)
      vert = 1;
    // v_hitsS.insert(v_hitsS.begin(),v_hitsS[0]);
  } else {
    if (v_hitsK[0].second.second != v_hitsK[1].second.second)
      vert = 1;
    // v_hitsK.insert(v_hitsK.begin(),v_hitsK[0]);
  }

  /////////////////////////////////
  // PHASE 1.b: Vertex Hits std::vector//
  /////////////////////////////////

  // we are going to copy in these std::vectors the hits recorded in the third 3 layers since the production layer
  std::vector<std::pair<int, std::pair<double, double>>> vert_v_hitsS;
  std::vector<std::pair<int, std::pair<double, double>>> vert_v_hitsK;

  if (vertmode) {
    std::vector<std::pair<int, std::pair<double, double>>>::iterator iter;
    iter = v_hitsS.begin();
    int first_layer = GetCluster((*iter).first)->GetTDR();

    while (GetCluster((*iter).first)->GetTDR() < first_layer + 6) {
      printf("nuova hit aggiunta al set S ristretto\n");
      vert_v_hitsS.push_back(*iter);
      iter++;
      if (iter == v_hitsS.end()) {
        break;
      }
    }
    printf("CAMBIO!!\n");
    iter = v_hitsK.begin();
    first_layer = GetCluster((*iter).first)->GetTDR();
    while (GetCluster((*iter).first)->GetTDR() < first_layer + 6) {
      printf("nuova hit aggiunta al set K ristretto\n");
      vert_v_hitsK.push_back(*iter);
      iter++;
      if (iter == v_hitsK.end()) {
        break;
      }
    }
    printf("vert_hitsS sono %lu\n", vert_v_hitsS.size());
    printf("vert_hitsK sono %lu\n", vert_v_hitsK.size());

    if (vert_v_hitsS.size() < 5 || vert_v_hitsK.size() < 5) {
      printf("non arriviamo ad almeno 5 per piano\n");
      return false;
    }
    printf("abbiamo riempito tutti i v_hits che ci serviranno e tutti contengono almeno 5 hits\n");
  }

  /////////////////////////////////////////////////////////
  // PHASE 2.a:Track Recontruction on the main Hits std::vector//
  /////////////////////////////////////////////////////////

  // in these std::vectors the hits excluded from the tracks
  std::vector<std::pair<int, std::pair<double, double>>> rejectsS;
  std::vector<std::pair<int, std::pair<double, double>>> rejectsK;

  int n_tr_to_search = 2;
  for (int it = 0; it < n_tr_to_search; it++) {
    printf("alla ricerca della %d° traccia\n", it + 1);
    printf("S:\n");
    Track(v_hitsS, rejectsS);
    printf("tracciate->%lu, rigettate->%lu\n", v_hitsS.size(), rejectsS.size());
    if (v_hitsS.size() < 3) {
      printf("poche hit\n"); /*return false;*/
    } else {
      NS++;
      _TrS.emplace_back(v_hitsS); // Here we store the tracks we have already found
    }
    printf("K:\n");
    Track(v_hitsK, rejectsK);
    printf("tracciate->%lu, rigettate->%lu\n", v_hitsK.size(), rejectsK.size());
    if (v_hitsK.size() < 3) {
      printf("poche hit\n"); /*return false;*/
    } else {
      NK++;
      _TrK.emplace_back(v_hitsK);
    }
    // rejected hits are now eligible for the next track to find
    if (vert && !ppside)
      rejectsS.insert(rejectsS.begin(), v_hitsS[0]);
    v_hitsS = rejectsS;
    if (vert && ppside)
      rejectsK.insert(rejectsK.begin(), v_hitsK[0]);
    v_hitsK = rejectsK;

    rejectsS.clear();
    rejectsK.clear();
  }
  _NTrks = std::min(NS, NK);
  // printf("S: %lu, %lu\n",_TrS[0].hits.size(),_TrS[1].hits.size());
  // printf("K: %lu, %lu\n",_TrK[0].hits.size(),_TrK[1].hits.size());
  // if ( _TrS[0].hits.size()<3 || _TrS[1].hits.size()<3 ) {printf("poche hit\n"); return false;}
  // if ( _TrK[0].hits.size()<3 || _TrK[1].hits.size()<3 ) {printf("poche hit\n"); return false;}

  ////////////////////////////////////////////////////////////////////////////////////////////
  // PHASE 2.b: Track Reconstruction on the Vertex Hits std::vector and merge with the main tracks//
  ////////////////////////////////////////////////////////////////////////////////////////////
  //  bool vertmode=1;
  if (vertmode && _NTrks >= 2) {
    // j_joinS is the index of the track
    int j_joinS, j_joinK;
    for (int it = 0; it < n_tr_to_search; it++) {
      printf("VERT:alla ricerca della %d° traccia\n", it + 1);

      Track(vert_v_hitsS, rejectsS);
      if (vert_v_hitsS.size() < 3) {
        printf("poche hit\n");
        return false;
      }
      printf("S:%lu\n", vert_v_hitsS.size());

      Track(vert_v_hitsK, rejectsK);
      if (vert_v_hitsK.size() < 3) {
        printf("poche hit\n");
        return false;
      }
      printf("K:%lu\n", vert_v_hitsK.size());

      if (it == 0) {
        j_joinS = (TMath::Abs(vert_v_hitsS.back().second.first - _TrS[0].hits[2].second.first) <
                   TMath::Abs(vert_v_hitsS.back().second.first - _TrS[1].hits[2].second.first))
                      ? 0
                      : 1;
        //	printf("j_joinS = %d\n",j_joinS);
        j_joinK = (TMath::Abs(vert_v_hitsK.back().second.first - _TrK[0].hits[2].second.first) <
                   TMath::Abs(vert_v_hitsK.back().second.first - _TrK[1].hits[2].second.first))
                      ? 0
                      : 1;
        //	printf("j_joinK = %d\n",j_joinK);
      } else if (it == 1) {
        j_joinS = 1 - j_joinS;
        j_joinK = 1 - j_joinK;
      }
      printf("Individuato punto di raccordo ottimale\n Procediamo al %d° raccordo\n", it + 1);
      // qui vengono raccolti i std::vector di hit appartenenti a ciascuna traccia
      if (vert_v_hitsS.size() <= _TrS[j_joinS].hits.size())
        copy(vert_v_hitsS.begin(), vert_v_hitsS.end(), _TrS[j_joinS].hits.begin());
      printf("raccordo sulle hit S effettuato!\n");
      _TrS[j_joinS].update();
      printf("traccia aggiornata\n");

      if (vert_v_hitsK.size() <= _TrK[j_joinK].hits.size())
        copy(vert_v_hitsK.begin(), vert_v_hitsK.end(), _TrK[j_joinK].hits.begin());
      printf("raccordo sulle hit K effettuato!\n");
      _TrK[j_joinK].update();
      printf("traccia aggiornata\n");

      vert_v_hitsS = rejectsS;
      vert_v_hitsK = rejectsK;

      rejectsS.clear();
      rejectsK.clear();
    }
  }

  ////////////////////////////////
  // PHASE 3:Vertex Recontruction//
  ////////////////////////////////
  std::pair<double, double> vx, vy;
  //  printf("3\n");
  if (_NTrks == 1) {
    vx = std::make_pair(_TrK[0].hits[0].second.second, _TrK[0].hits[0].second.first);
    vy = std::make_pair(_TrS[0].hits[0].second.second, _TrS[0].hits[0].second.first);
    if (ppside)
      vy.second = vx.second;
    else
      vx.second = vy.second;
  } else if (std::min(NS, NK) == 2) {
    if (ppside) {
      vx = vertex(_TrK[0], _TrK[1]);
      if (_TrS[0].hits[0].first == _TrS[1].hits[0].first && _TrS[0].hits[1].first == _TrS[1].hits[1].first) {
        vy.first = tan(_TrS[0].prod_angle) * vx.second + _TrS[0].prod_dist / cos(_TrS[0].prod_angle);
      } else
        vy = vertex(_TrS[0], _TrS[1]);
      vy.second = vx.second;
    } else {
      vy = vertex(_TrS[0], _TrS[1]);
      if (_TrK[0].hits[0].first == _TrK[1].hits[0].first && _TrK[0].hits[1].first == _TrK[1].hits[1].first) {
        vx.first = tan(_TrK[0].prod_angle) * vy.second + _TrK[0].prod_dist / cos(_TrK[0].prod_angle);
      } else
        vx = vertex(_TrK[0], _TrK[1]);
      vx.second = vy.second;
    }
  }
  //  printf("4\n");

  if (vx == std::make_pair(9999., 9999.)) {
    std::cout << "Impossible to find a vertex on the ZX plane!!!\n";
    return false;
  }
  if (vy == std::make_pair(9999., 9999.)) {
    std::cout << "Impossible to find a vertex on the ZY plane!!!\n";
    return false;
  }
  _vertexK = vx;
  _vertexS = vy;
  return true;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::Track(std::vector<Hit> &hits, std::vector<Hit> &rejects) {
  rejects.clear();
  std::vector<Hit> _hits;
  std::pair<double, double> dir;
  double pos;

  // GENERALIZE ME: here assuming just one JINF
  for (int itdr = NTDRS - 1; itdr >= 0; itdr--) {
    dir = Hough(hits);
    pos = AlignmentPars::Instance()->GetPar(0, itdr, 2); // current z position for the study
    //    printf("iter n° %d -> pos = %f\ndir = (%f,%f)\n",itdr,pos,dir.first,dir.second);
    double xexp = dir.second / cos(dir.first) + tan(dir.first) * pos;
    // variables for the search of the minimum distance
    double min = 9999;
    int jmin = -1;
    int s = hits.size();
    for (int j = 0; j < s; j++) {
      if (abs(hits[j].second.second - pos) < 1.) { // layer belonging condition
        //	printf("pos=%f\n",pos);
        //	double xexp = dir.second/cos(dir.first) + tan(dir.first)*hits[j].second.second;
        double d = TMath::Abs(hits[j].second.first - xexp);

        if (d < min) { // among the coplanar hits we look for the nearest to the expected point
          if (jmin > -1)
            rejects.push_back(hits[jmin]);
          min = d;
          jmin = j;
        } else {
          rejects.push_back(hits[j]);
        }
      } else {
        // we keep all the other hits
        _hits.push_back(hits[j]);
      }
    } // ciclo for sugli elementi di hits

    if (jmin > -1) { // we still have to save the winner hit
      hits = {hits[jmin]};
      //      _hits.push_back(hits[jmin]);
    } else
      hits.clear();
    for (int nh = 0; nh < _hits.size(); nh++)
      hits.push_back(_hits[nh]);
    _hits.clear();
  }

  reverse(rejects.begin(), rejects.end());
  return;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
std::pair<double, double> GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::Hough(std::vector<Hit> &vec) {
  int nHits = vec.size();
  HoughSpace h(.00001, .00001);
  double m, th, r;
  for (int ii = 0; ii < nHits; ii++) {
    for (int jj = ii + 1; jj < nHits; jj++) {
      if (vec[ii].second.second != vec[jj].second.second) {
        m = (vec[ii].second.first - vec[jj].second.first) / (vec[ii].second.second - vec[jj].second.second);
        th = atan(m);
        r = cos(th) * vec[ii].second.first - sin(th) * vec[ii].second.second;

        h.Add(th, r);
      }
    }
  }
  return h.GetMax();
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
std::vector<Hit> GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::CleanTrack(std::vector<Hit> &hits) {
  int i = 0, tdr = hits[0].first;
  int imax = 0, smax = 0;
  std::vector<std::vector<Hit>> coll;
  for (auto h : hits) {
    if (h.first - tdr < 4)
      coll[i].push_back(h);
    else {
      if (coll[i].size() > smax) {
        smax = coll[i].size();
        imax = i;
      }
      i++;
    }
    tdr = h.first;
  }
  return coll[imax];
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
void GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::RecombineXY(double ang) {
  std::vector<double> xhitrot, yhitrot;
  int tdrX = 18;
  int tdrY = 19;
  for (int cl = 0; cl < NClusTot; cl++) {
    if (GetCluster(cl)->GetTDR() == tdrY) // questi numeri hardcoded sono brutti
      yhitrot.push_back(GetCluster(cl)->GetAlignedPositionMC() + 0.07);
    if (GetCluster(cl)->GetTDR() == tdrX)
      xhitrot.push_back(GetCluster(cl)->GetAlignedPositionMC() + 0.07);
  }
  if (xhitrot.size() < 2 || yhitrot.size() < 2)
    return;

  // TGraph* gr=new TGraph();
  TRotation rot;
  rot = rot.RotateZ(ang * TMath::Pi() / 180.); // da controllare l'angolo
  double x[4], y[4];
  for (int i = 0; i < 4; i++) {
    TVector3 bo = rot * TVector3(xhitrot[i % 2], yhitrot[i < 2 ? (i % 2) : (1 - i % 2)], 0);
    x[i] = bo[0];
    y[i] = bo[1];
    //    printf("hit ruotata %d --> (%f, %f)\n",i,x[i],y[i]);
    //    gr->SetPoint(gr->GetN(),x[i],y[i]);
  }
  double a1 = (std::min(x[0], x[1]) + std::min(x[2], x[3])) / 2, // dovrebbero essere giusti
      a2 = (std::max(x[0], x[1]) + std::max(x[2], x[3])) / 2, b1 = (std::min(y[0], y[1]) + std::min(y[2], y[3])) / 2,
         b2 = (std::max(y[0], y[1]) + std::max(y[2], y[3])) / 2;

  AlignmentPars *alignmentPars = AlignmentPars::Instance();

  double y1 = tan(_TrS[0].exit_angle) * alignmentPars->GetPar(0, tdrY, 2) + _TrS[0].exit_dist / cos(_TrS[0].exit_angle),
         y2 = tan(_TrS[1].exit_angle) * alignmentPars->GetPar(0, tdrY, 2) + _TrS[1].exit_dist / cos(_TrS[1].exit_angle),
         x1 = tan(_TrK[0].exit_angle) * alignmentPars->GetPar(0, tdrX, 2) + _TrK[0].exit_dist / cos(_TrK[0].exit_angle),
         x2 = tan(_TrK[1].exit_angle) * alignmentPars->GetPar(0, tdrX, 2) + _TrK[1].exit_dist / cos(_TrK[1].exit_angle);
  printf("x1=%f, x2=%f, y1=%f, y2=%f\n", x1, x2, y1, y2);
  // gr->SetPoint(gr->SetPoint(),x1,0);
  // gr->SetPoint(gr->SetPoint(),x2,0);
  // gr->SetPoint(gr->SetPoint(),0,y1);
  // gr->SetPoint(gr->SetPoint(),0,y2);
  int option1 = 0;
  if (x1 > a2 || x1 < a1)
    option1++; // ricontrolla tutto
  if (x2 > a2 || x2 < a1)
    option1++;
  if (y1 < b2 && y1 > b1)
    option1++;
  if (y2 < b2 && y2 > b1)
    option1++;
  std::pair<double, double> hit1, hit2;
  if (option1 < 2) { // this work only if the layer is tilted counterclockwise
    hit1 = std::make_pair(x[0], y[0]);
    hit2 = std::make_pair(x[1], y[1]);
  } else {
    hit1 = std::make_pair(x[2], y[2]);
    hit2 = std::make_pair(x[3], y[3]);
  }
  if (abs(x1 - hit1.first) > abs(x1 - hit2.first)) {
    std::reverse(_TrK.begin(), _TrK.end());
    // Track box=_TrK[0];
    // _TrK.erase(_TrK.begin());
    // _TrK.push_back(box);
  }
  if (abs(y1 - hit1.second) > abs(y1 - hit2.second)) {
    std::reverse(_TrS.begin(), _TrS.end());
  }
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
std::pair<double, double> GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetVertexS() {
  if (_vertexS == std::make_pair(9999., 9999.))
    FindTracksAndVertex();
  return _vertexS;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
std::pair<double, double> GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetVertexK() {
  if (_vertexK == std::make_pair(9999., 9999.))
    FindTracksAndVertex();
  return _vertexK;
}

template <size_t NJINF, size_t NTDRS, size_t NCHAVA, size_t NADCS, size_t NVASS, size_t NVASK>
trackColl *GenericEvent<NJINF, NTDRS, NCHAVA, NADCS, NVASS, NVASK>::GetTracks(int t) {
  if (t)
    return &_TrK;
  else
    return &_TrS;
}

#endif
