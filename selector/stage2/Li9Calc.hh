#pragma once

#include "../common/Constants.hh"

#include <functional>
#include <map>
#include <string>

class Li9Calc {
public:
  Li9Calc();

  // This return the rate (per AD per day) predicted in our raw spectrum
  // assuming perfect veto/dmc efficiencies
  double li9daily(Site site, unsigned shower_pe, double showerVeto_ms);

private:
  static constexpr unsigned MIN_PE = 5'000;
  static constexpr unsigned MAX_NTAG_PE = 160'000;
  static constexpr unsigned MIN_VETO_BDRY_PE = 250'000;
  static constexpr unsigned MAX_VETO_BDRY_PE = 350'000;
  static constexpr unsigned DELTA_VETO_BDRY_PE = 10'000;
  static constexpr unsigned MAX_PE = 9'999'999;
  static constexpr double LI9_SEL_SHOWER_VETO_MS = 400;
  static constexpr double LI9_LIVETIME_MS = 257;
  static constexpr double NTAG_EFF = 0.45;
  static constexpr const char* DATA_DIR = "static/li9_data";
  static constexpr const char* LI9_FILENAME = "matt_Li9_rates.txt";
  static constexpr const char* LIVETIME_FILENAME = "p17b_livetimes.txt";

  // These structs should be private, but then the _dict.cxx generated by
  // rootcling will still try to access them (because we enabled the
  // nestedclasses pragma?), causing a compiler error. Maybe we should report
  // this as a ROOT bug. (Why doesn't ACLiC fail?)
public:
  struct Data {
    double nomRate, statUnc, rateNoB12, rate15pctHe8, rateNoHe8;
  };

  struct MuonBin {
    unsigned lowerPE, upperPE;

    bool operator<(const MuonBin& rhs) const // needed for std::map
    {
      return lowerPE < rhs.lowerPE ||
        (lowerPE == rhs.lowerPE && upperPE < rhs.upperPE);
    }
  };

private:
  Data& get(Site site, MuonBin bin)
  {
    return table[site][bin];
  }

  using SiteTable = std::map<MuonBin, Data>;
  using Table = std::map<Site, SiteTable>;
  using LivetimeTable = std::map<Site, double>;

  std::string dataPath(const char* filename);
  void initTable();
  void initLivetimes();
  double measLowRange(Site site);
  double measMidRange(Site site, unsigned shower_pe);
  double measHighRange(Site site, unsigned shower_pe);
  double extrapolate(unsigned shower_pe,
                     std::function<double(unsigned)> getter);

  Table table;
  LivetimeTable adLivedays;
};

