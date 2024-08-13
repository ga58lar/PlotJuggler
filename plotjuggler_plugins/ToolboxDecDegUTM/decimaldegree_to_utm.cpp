#include "decimaldegree_to_utm.h"
#include "DecDegUTM/latlong_utm_conversion.h"
#include <array>
#include <math.h>

DecimalDegreeToUTM::DecimalDegreeToUTM()
{
  reset();
}

void DecimalDegreeToUTM::reset()
{
  _prev_utmx = 0;
  _prev_utmy = 0;
  _utmx_offset = 0;
  _utmy_offset = 0;
  _last_timestamp = std::numeric_limits<double>::lowest();
}

void DecimalDegreeToUTM::calculate()
{
  auto& data_lat = *_src_vector[0];
  auto& data_lon = *_src_vector[1];

  auto& data_utmx = *_dst_vector[0];
  auto& data_utmy = *_dst_vector[1];

  data_utmx.setMaximumRangeX(data_utmx.maximumRangeX());
  data_utmy.setMaximumRangeX(data_utmx.maximumRangeX());

  data_utmx.clear();
  data_utmy.clear();

  if (data_lat.size() == 0 || data_lat.size() != data_lon.size())
  {
    return;
  }

  int pos = data_lat.getIndexFromX(_last_timestamp);
  size_t index = pos < 0 ? 0 : static_cast<size_t>(pos);

  while (index < data_lat.size())
  {
    auto& point_x = data_lat.at(index);
    double timestamp = point_x.x;
    double lat = point_x.y;
    double lon = data_lon.at(index).y;

    if (timestamp >= _last_timestamp)
    {
      std::array<double, 2> utm;
      calculateNextPoint(index, { lat, lon}, utm);

      data_utmx.pushBack({ timestamp, _scale * (utm[0] + _utmx_offset) });
      data_utmy.pushBack({ timestamp, _scale * (utm[1] + _utmy_offset) });

      _last_timestamp = timestamp;
    }
    index++;
  }
}

void DecimalDegreeToUTM::calculateNextPoint(size_t index,
                                                  const std::array<double, 2>& latlon,
                                                  std::array<double, 2>& utm)
{
  double lat = latlon[0];
  double lon = latlon[1];

  double utmx, utmy;
  int ellipsoid = 23;
  LLtoUTM(ellipsoid, lat, lon, utmy, utmx);

  if (_mgrs) {
    utmx = std::fmod(utmx, 100000);
    utmy = std::fmod(utmy, 100000);
  }

  _prev_utmx = utmx;
  _prev_utmy = utmy;

  utm = { utmx, utmy };
}
