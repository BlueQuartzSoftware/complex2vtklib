#pragma once

#include <memory>
#include <vector>

#include <vtkDataArray.h>
#include <vtkObject.h>

#include "complex/DataStructure/DataStructure.hpp"

#include "complex2VtkLib/VtkBridge/VtkMacros.hpp"
#include "complex2VtkLib/complex2VtkLib_export.hpp"

namespace complex
{
class AbstractGeometry;
} // namespace complex

namespace CV
{
namespace VtkBridge
{
/**
 * @brief Returns a vector of vtkObject pointers wrapping available geometries
 * within the specified DataStructure. Wrapped geometries store a std::shared_ptr
 * to the target complex geometry and will not be cleaned up if the DataStructure
 * goes out of scope before the vtkObject does.
 * @param ds
 * @return std::vector<VTK_PTR(vtkObject)>
 */
COMPLEX2VTKLIB_EXPORT std::vector<VTK_PTR(vtkObject)> wrapDataStructure(const complex::DataStructure& ds);

/**
 * @brief Attempts to create a vtkObject wrapping the specified complex geometry.
 * A std::shared_ptr to the geometry is stored in the wrapped geometry, preventing
 * it from being cleaned up if the DataStructure goes out of scope before the
 * vtkObject does.
 *
 * Returns nullptr if the geometry is not recognized for wrapping.
 * @param geom
 * @return VTK_PTR(vtkObject)
 */
COMPLEX2VTKLIB_EXPORT VTK_PTR(vtkObject) wrapGeometry(const std::shared_ptr<complex::AbstractGeometry>& geom);

/**
 * @brief Attempts to wrap a complex DataArray found within the specified
 * DataStructure using the target DataObject::IdType.
 * @param arrayId
 * @return vtkDataArray*
 */
COMPLEX2VTKLIB_EXPORT vtkDataArray* wrapDataArray(const complex::DataStructure& ds, complex::DataObject::IdType arrayId);
} // namespace VtkBridge
} // namespace CV
