#pragma once

#include <memory>

#include <vtkIdTypeArray.h>
#include <vtkImageData.h>

#include "complex/DataStructure/Geometry/ImageGeom.hpp"

#include "complex2VtkLib/VtkBridge/VtkMacros.hpp"
#include "complex2VtkLib/complex2VtkLib_export.hpp"

namespace CV
{
/**
 * @class CV::ImageGeom
 * @brief This class is used as an implementation class for vtkMappedUnstructuredGrid to
 * be used with complex's ImageGeom. The implementation maps the cell and point IDs
 * from the complex geometry.
 */
class COMPLEX2VTKLIB_EXPORT ImageGeom : public vtkImageData
{
public:
  static VTK_PTR(vtkDataSet) CreateFromGeom(const std::shared_ptr<complex::ImageGeom>& geom);
  static ImageGeom* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(ImageGeom, vtkImageData);

  /**
   * @brief Sets the complex geometry.
   * @param geom
   */
  void SetGeometry(const std::shared_ptr<complex::ImageGeom>& geom);

protected:
  /**
   * @brief Default constructor
   */
  ImageGeom();

private:
  std::shared_ptr<complex::ImageGeom> m_Geom = nullptr;
};
} // namespace CV
