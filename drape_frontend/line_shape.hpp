#pragma once

#include "drape_frontend/map_shape.hpp"
#include "drape_frontend/shape_view_params.hpp"

#include "geometry/spline.hpp"

namespace df
{

class LineShape : public MapShape
{
public:
  LineShape(m2::SharedSpline const & spline,
            LineViewParams const & params);

  virtual void Draw(dp::RefPointer<dp::Batcher> batcher, dp::RefPointer<dp::TextureManager> textures) const;

private:
  LineViewParams m_params;
  m2::SharedSpline m_spline;
};

} // namespace df

