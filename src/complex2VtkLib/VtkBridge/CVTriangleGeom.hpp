#pragma once

#include <memory>

#include <vtkIdTypeArray.h>
#include <vtkMappedUnstructuredGrid.h>

#include "complex/DataStructure/Geometry/TriangleGeom.hpp"

#include "complex2VtkLib/VtkBridge/VtkMacros.hpp"
#include "complex2VtkLib/complex2VtkLib_export.hpp"

namespace CV
{
/**
 * @class CV::TriangleGeom
 * @brief This class is used as an implementation class for vtkMappedUnstructuredGrid to
 * be used with complex's TriangleGeom. The implementation maps the cell and point IDs
 * from the complex geometry but the vertex points must be copied into the
 * unstructured grid separately.
 */
class COMPLEX2VTKLIB_EXPORT TriangleGeom : public vtkObject
{
public:
  static VTK_PTR(vtkDataSet) CreateFromGeom(const std::shared_ptr<complex::TriangleGeom>& geom);
  static TriangleGeom* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(TriangleGeom, vtkObject);

  /**
   * @brief Sets the complex geometry.
   * @param geom
   */
  void SetGeometry(const std::shared_ptr<complex::TriangleGeom>& geom);

  /**
   * @brief Returns the number of cells in the geometry.
   * @return vtkIdType
   */
  vtkIdType GetNumberOfCells();

  /**
   * @brief Returns the cell type for the given cell ID.
   * @param cellId
   * @return int
   */
  int GetCellType(vtkIdType cellId);

  /**
   * @brief Gets a list of point IDs used by the cell ID.
   * @param cellId
   * @param ptIds
   */
  void GetCellPoints(vtkIdType cellId, vtkIdList* ptIds);

  /**
   * @brief Gets a list of cell IDs that use the given point ID.
   * @param ptId
   * @param cellIds
   */
  void GetPointCells(vtkIdType ptId, vtkIdList* cellIds);

  /**
   * @brief Returns the maximum cell size.
   * @return int
   */
  int GetMaxCellSize();

  /**
   * @brief Gets a list of all cell IDs of a given type.
   * @param type
   * @param array
   */
  void GetIdsOfCellsOfType(int type, vtkIdTypeArray* array);

  /**
   * @brief Returns whether or not all cells are of the same type.
   * @return int
   */
  int IsHomogeneous();

  /**
   * @brief Required by vtkMappedUnstructuredGrid but should not be called on
   * this read-only implementation.
   * @param numCells
   * @param extSize
   */
  void Allocate(vtkIdType numCells, int extSize = 1000);

  /**
   * @brief Required by vtkMappedUnstructuredGrid but should not be called on
   * this read-only implementation.
   * @param type
   * @param ptIds
   * @return vtkIdType
   */
  vtkIdType InsertNextCell(int type, const vtkIdList ptIds[]);

  /**
   * @brief Required by vtkMappedUnstructuredGrid but should not be called on
   * this read-only implementation.
   * @param type
   * @param npts
   * @param ptIds
   * @return vtkIdType
   */
  vtkIdType InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[]);

  /**
   * @brief Required by vtkMappedUnstructuredGrid but should not be called on
   * this read-only implementation.
   * @param type
   * @param npts
   * @param ptIds
   * @param nfaces
   * @param faces
   * @return vtkIdType
   */
  vtkIdType InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[]);

  /**
   * @brief Required by vtkMappedUnstructuredGrid but should not be called on
   * this read-only implementation.
   * @param cellId
   * @param npts
   * @param pts
   */
  void ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[]);

protected:
  /**
   * @brief Default constructor
   */
  TriangleGeom();

private:
  std::shared_ptr<complex::TriangleGeom> m_Geom = nullptr;
  float m_MaxCellSize = 0.0f;
  const int CELL_TYPE = VTK_TRIANGLE;
};

vtkMakeMappedUnstructuredGrid(CVTriangleGrid, TriangleGeom);
} // namespace CV
