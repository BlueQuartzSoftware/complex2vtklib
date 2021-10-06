#include "CVTetrahedralGeom.hpp"

#include <cmath>

#include <vtkCellTypes.h>
#include <vtkIdTypeArray.h>
#include <vtkPoints.h>

using namespace CV;

CVTetrahedralGrid* CVTetrahedralGrid::New()
{
  return new CVTetrahedralGrid();
}

TetrahedralGeom* TetrahedralGeom::New()
{
  return new TetrahedralGeom();
}

void TetrahedralGeom::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Elements: " << GetNumberOfCells() << endl;
  os << indent << "CellType: " << vtkCellTypes::GetClassNameFromTypeId(CELL_TYPE) << endl;
  os << indent << "CellSize: " << GetMaxCellSize() << endl;
  os << indent << "NumberOfCells: " << GetNumberOfCells() << endl;
}

VTK_PTR(vtkDataSet) CV::TetrahedralGeom::CreateFromGeom(const std::shared_ptr<complex::TetrahedralGeom>& geom)
{
  if(geom == nullptr)
  {
    return nullptr;
  }
  VTK_NEW(CVTetrahedralGrid, dataSet);
  auto* cvGeom = dataSet->GetImplementation();
  cvGeom->SetGeometry(geom);

  return dataSet;
}

TetrahedralGeom::TetrahedralGeom()
: vtkObject()
{
}

void TetrahedralGeom::SetGeometry(const std::shared_ptr<complex::TetrahedralGeom>& geom)
{
  m_Geom = geom;
}

vtkIdType TetrahedralGeom::GetNumberOfCells()
{
  if(nullptr == m_Geom)
  {
    vtkErrorMacro("Wrapper Geometry missing a Geometry object");
    return -1;
  }

  return m_Geom->getNumberOfElements();
}

int TetrahedralGeom::GetCellType(vtkIdType cellId)
{
  if(0 == GetNumberOfCells())
  {
    return VTK_EMPTY_CELL;
  }

  return CELL_TYPE;
}

void TetrahedralGeom::GetCellPoints(vtkIdType cellId, vtkIdList* ptIds)
{
  const int numVerts = 4;

  size_t verts[numVerts];
  m_Geom->getVertsAtTet(cellId, verts);

  ptIds->SetNumberOfIds(numVerts);
  for(int i = 0; i < numVerts; i++)
  {
    ptIds->SetId(i, verts[i]);
  }
}

void TetrahedralGeom::GetPointCells(vtkIdType ptId, vtkIdList* cellIds)
{
  auto elementsContainingList = m_Geom->getElementsContainingVert();
  complex::AbstractGeometry::ElementDynamicList::ElementList listArray = elementsContainingList->getElementList(ptId);

  cellIds->SetNumberOfIds(listArray.numCells);
  for(int i = 0; i < listArray.numCells; i++)
  {
    cellIds->SetId(i, listArray.cells[i]);
  }
}

int TetrahedralGeom::GetMaxCellSize()
{
  return std::ceil(m_MaxCellSize);
}

void TetrahedralGeom::GetIdsOfCellsOfType(int type, vtkIdTypeArray* array)
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

int TetrahedralGeom::IsHomogeneous()
{
  return 1;
}

void TetrahedralGeom::Allocate(vtkIdType numCells, int extSize)
{
  vtkErrorMacro("Read only container.");
}

vtkIdType TetrahedralGeom::InsertNextCell(int type, const vtkIdList ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType TetrahedralGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType TetrahedralGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[])
{
  return -1;
}

void TetrahedralGeom::ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[])
{
  vtkErrorMacro("Read only container.");
}
