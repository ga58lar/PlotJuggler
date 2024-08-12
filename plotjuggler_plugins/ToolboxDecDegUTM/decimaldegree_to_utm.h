#ifndef DECIMALDEGREE_TO_UTM_H
#define DECIMALDEGREE_TO_UTM_H

#include "PlotJuggler/transform_function.h"

class DecimalDegreeToUTM : public PJ::TransformFunction
{
public:
  DecimalDegreeToUTM();

  const char* name() const override
  {
    return "decimaldegree_to_utm";
  }

  void reset() override;

  int numInputs() const override
  {
    return 2;
  }

  int numOutputs() const override
  {
    return 2;
  }

  void setScale(double scale)
  {
    _scale = scale;
  }

  void calculate() override;

  void calculateNextPoint(size_t index, const std::array<double, 2>& latlon,
                          std::array<double, 2>& utm);

private:
  double _prev_utmx = 0;
  double _prev_utmy = 0;
  double _utmx_offset = 0;
  double _utmy_offset = 0;
  double _scale = 1.0;
  double _last_timestamp = std::numeric_limits<double>::lowest();
};

#endif  // DECIMALDEGREE_TO_UTM_H
