#include "CVQuadGeom.hpp"

#include <cmath>

#include <vtkCellType.h>
#include <vtkCellTypes.h>
#include <vtkIdTypeArray.h>
#include <vtkPoints.h>

using namespace CV;

CVQuadGrid* CVQuadGrid::New()
{
  return new CVQuadGrid();
}

QuadGeom* QuadGeom::New()
{
  return new QuadGeom();
}

void QuadGeom::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Elements: " << GetNumberOfCells() << endl;
  os << indent << "CellType: " << vtkCellTypes::GetClassNameFromTypeId(CELL_TYPE) << endl;
  os << indent << "CellSize: " << GetMaxCellSize() << endl;
  os << indent << "NumberOfCells: " << GetNumberOfCells() << endl;
}

QuadGeom::QuadGeom()
: vtkObject()
{
}

void QuadGeom::SetGeometry(const std::shared_ptr<complex::QuadGeom>& geom)
{
  m_Geom = geom;
  m_Geom->findElementsContainingVert();
}

vtkIdType QuadGeom::GetNumberOfCells()
{
  if(nullptr == m_Geom)
  {
    vtkErrorMacro("Wrapper Geometry missing a Geometry object");
    return -1;
  }

  return m_Geom->getNumberOfElements();
}

int QuadGeom::GetCellType(vtkIdType cellId)
{
  if(0 == GetNumberOfCells())
  {
    return VTK_EMPTY_CELL;
  }

  return CELL_TYPE;
}

void QuadGeom::GetCellPoints(vtkIdType cellId, vtkIdList* ptIds)
{
  const int numVerts = 4;

  size_t verts[numVerts];
  m_Geom->getVertsAtQuad(cellId, verts);

  ptIds->SetNumberOfIds(numVerts);
  for(int i = 0; i < numVerts; i++)
  {
    ptIds->SetId(i, verts[i]);
  }
}

void QuadGeom::GetPointCells(vtkIdType ptId, vtkIdList* cellIds)
{
  auto elementsContainingList = m_Geom->getElementsContainingVert();
  complex::DynamicListArray<uint16_t, size_t>::ElementList listArray = elementsContainingList->getElementList(ptId);

  cellIds->SetNumberOfIds(listArray.ncells);
  for(int i = 0; i < listArray.ncells; i++)
  {
    cellIds->SetId(i, listArray.cells[i]);
  }
}

int QuadGeom::GetMaxCellSize()
{
  return std::ceil(m_MaxCellSize);
}

void QuadGeom::GetIdsOfCellsOfType(int type, vtkIdTypeArray* array)
{
  if(CELL_TYPE == type)
  {
    int numValues = GetNumberOfCells();

    array = vtkIdTypeArray::New();
    array->SetNumberOfTuples(numValues);
    array->SetNumberOfComponents(1);

    vtkIdType* arrayValues = new vtkIdType[numValues];
    for(int i = 0; i < numValues; i++)
    {
      arrayValues[i] = i;
    }

    array->SetVoidArray(arrayValues, numValues, 0);
  }
}

int QuadGeom::IsHomogeneous()
{
  return 1;
}

void QuadGeom::Allocate(vtkIdType numCells, int extSize)
{
  vtkErrorMacro("Read only container.");
}

vtkIdType QuadGeom::InsertNextCell(int type, const vtkIdList ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType QuadGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType QuadGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[])
{
  return -1;
}

void QuadGeom::ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[])
{
  vtkErrorMacro("Read only container.");
}