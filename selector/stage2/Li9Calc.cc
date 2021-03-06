#include "Li9Calc.hh"

#include <fstream>

Li9Calc::Li9Calc()
{
  initTable();
}

void Li9Calc::initTable()
{
  std::ifstream ifs(dataPath(LI9_FILENAME));

  while (true) {
    unsigned site;
    ifs >> site;

    if (not ifs)      // eof
      break;

    MuonBin bin;
    ifs >> bin.lowerPE >> bin.upperPE;

    Data& d = table[Site(site)][bin];
    ifs >> d.nomRate >> d.statUnc >> d.rateNoB12 >> d.rate15pctHe8 >> d.rateNoHe8;
  }
}

std::string Li9Calc::dataPath(const char* filename)
{
  std::string base = IBDSEL_BASE;
  return base + "/" + DATA_DIR + "/" + filename;
}

double Li9Calc::extrapolate(unsigned shower_pe,
                            std::function<double(unsigned)> getter)
{
  const unsigned delta = DELTA_VETO_BDRY_PE;
  const unsigned leftEdge = shower_pe / delta * delta;
  const unsigned rightEdge = leftEdge + delta;
  const double weight = double(shower_pe - leftEdge) / delta;

  return (1 - weight) * getter(leftEdge) + weight * getter(rightEdge);
}


double Li9Calc::measLowRange(Site site)
{
  return get(site, {MIN_PE, MAX_NTAG_PE}).nomRate;
}

double Li9Calc::measMidRange(Site site, unsigned shower_pe)
{
  auto getter = [&](unsigned edge_pe) -> double {
    return get(site, {MAX_NTAG_PE, edge_pe}).nomRate;
  };

  return extrapolate(shower_pe, getter);
}

double Li9Calc::measHighRange(Site site, unsigned shower_pe)
{
  auto getter = [&](unsigned edge_pe) -> double {
    return get(site, {edge_pe, MAX_PE}).nomRate;
  };

  return extrapolate(shower_pe, getter);
}

double Li9Calc::li9daily(Site site, double shower_pe, double showerVeto_ms)
{
  const double totLow = measLowRange(site);
  const double totMid = measMidRange(site, shower_pe);
  const double totHigh = measHighRange(site, shower_pe);

  const double ibdSelSurvProb = exp(-showerVeto_ms / LI9_LIFETIME_MS);
  const double survRatio = ibdSelSurvProb / LI9_SEL_SURV_PROB;

  const size_t isite = int(site) - 1;
  const double denom = LIVEDAYS[isite] * NDET_WEIGHTED[isite] *
    VETO_EFFS[isite] * MULT_EFF * DT_EFF;
  const double lowPePromptEff =
    site == Site::EH3 ? FAR_LOW_PE_PROMPT_EFF : NEAR_LOW_PE_PROMPT_EFF;

  // XXX
  const double highPeFudgeFactor =
    site == Site::EH3 ? LI9_SEL_SURV_PROB : 1;

  const double predCount =
    totLow / lowPePromptEff +
    totMid / lowPePromptEff +
    totHigh / HIGH_PE_PROMPT_EFF * survRatio * highPeFudgeFactor;

  return predCount / denom;
}
