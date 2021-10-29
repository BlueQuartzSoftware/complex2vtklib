#pragma once

#include <memory>
#include <vector>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
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
 * @param dataStructure
 * @return std::vector<VTK_PTR(vtkDataSet)>
 */
COMPLEX2VTKLIB_EXPORT std::vector<VTK_PTR(vtkDataSet)> wrapDataStructure(const complex::DataStructure& dataStructure);

/**
 * @brief Attempts to create a vtkObject wrapping the specified complex geometry.
 * A std::shared_ptr to the geometry is stored in the wrapped geometry, preventing
 * it from being cleaned up if the DataStructure goes out of scope before the
 * vtkObject does.
 *
 * Returns nullptr if the geometry is not recognized for wrapping.
 * @param geom
 * @return VTK_PTR(vtkDataSet)
 */
COMPLEX2VTKLIB_EXPORT VTK_PTR(vtkDataSet) wrapGeometry(const std::shared_ptr<complex::AbstractGeometry>& geom);

/**
 * @brief Attempts to create a vtkObject wrapping the specified complex geometry.
 * A std::shared_ptr to the geometry is stored in the wrapped geometry, preventing
 * it from being cleaned up if the DataStructure goes out of scope before the
 * vtkObject does.
 *
 * Returns nullptr if the geometry is not recognized for wrapping.
 * @param geom
 * @return VTK_PTR(vtkDataSet)
 */
COMPLEX2VTKLIB_EXPORT VTK_PTR(vtkDataSet) wrapGeometryWithArrays(const std::shared_ptr<complex::AbstractGeometry>& geom);

/**
 * @brief Attempts to wrap a complex DataArray found within the specified
 * DataStructure using the target DataObject::IdType.
 * @param dataStructure
 * @param arrayId
 * @return vtkDataArray*
 */
COMPLEX2VTKLIB_EXPORT vtkDataArray* wrapDataArray(const complex::DataStructure& dataStructure, complex::DataObject::IdType arrayId);

/**
 * @brief Attempts to wrap a complex DataArray found within the specified
 * @param dataArray
 * @return vtkDataArray*
 */
COMPLEX2VTKLIB_EXPORT vtkDataArray* wrapDataArray(const std::shared_ptr<complex::DataObject>& dataArray);
} // namespace VtkBridge
} // namespace CV
