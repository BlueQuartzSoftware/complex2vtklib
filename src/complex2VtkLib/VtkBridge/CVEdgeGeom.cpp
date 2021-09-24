#include "CVEdgeGeom.hpp"

#include <cmath>

#include <vtkCellType.h>
#include <vtkCellTypes.h>
#include <vtkIdTypeArray.h>

using namespace CV;

CVEdgeGrid* CVEdgeGrid::New()
{
  return new CVEdgeGrid();
}

EdgeGeom* EdgeGeom::New()
{
  return new EdgeGeom();
}

void EdgeGeom::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Elements: " << GetNumberOfCells() << endl;
  os << indent << "CellType: " << vtkCellTypes::GetClassNameFromTypeId(CELL_TYPE) << endl;
  os << indent << "CellSize: " << GetMaxCellSize() << endl;
  os << indent << "NumberOfCells: " << GetNumberOfCells() << endl;
}

VTK_PTR(vtkDataSet) CV::EdgeGeom::CreateFromGeom(const std::shared_ptr<complex::EdgeGeom>& geom)
{
  if(geom == nullptr)
  {
    return nullptr;
  }
  VTK_NEW(CVEdgeGrid, dataSet);
  EdgeGeom* edgeGeom = dataSet->GetImplementation();
  edgeGeom->SetGeometry(geom);

  return dataSet;
}

EdgeGeom::EdgeGeom()
: vtkObject()
{
}

void EdgeGeom::SetGeometry(const std::shared_ptr<complex::EdgeGeom>& geom)
{
  m_Geom = geom;
  m_Geom->findElementsContainingVert();
}

vtkIdType EdgeGeom::GetNumberOfCells()
{
  if(nullptr == m_Geom)
  {
    vtkErrorMacro("Wrapper Geometry missing a Geometry object");
    return -1;
  }

  return m_Geom->getNumberOfElements();
}

int EdgeGeom::GetCellType(vtkIdType cellId)
{
  if(0 == GetNumberOfCells())
  {
    return VTK_EMPTY_CELL;
  }

  return CELL_TYPE;
}

void EdgeGeom::GetCellPoints(vtkIdType cellId, vtkIdList* ptIds)
{
  const int numVerts = 2;

  size_t verts[numVerts];
  m_Geom->getVertsAtEdge(cellId, verts);

  ptIds->SetNumberOfIds(numVerts);
  for(int i = 0; i < numVerts; i++)
  {
    ptIds->SetId(i, verts[i]);
  }
}

void EdgeGeom::GetPointCells(vtkIdType ptId, vtkIdList* cellIds)
{
  auto elementsContainingList = m_Geom->getElementsContainingVert();
  complex::DynamicListArray<uint16_t, size_t>::ElementList listArray = elementsContainingList->getElementList(ptId);

  cellIds->SetNumberOfIds(listArray.numCells);
  for(int i = 0; i < listArray.numCells; i++)
  {
    cellIds->SetId(i, listArray.cells[i]);
  }
}

int EdgeGeom::GetMaxCellSize()
{
  return std::ceil(m_MaxCellSize);
}

void EdgeGeom::GetIdsOfCellsOfType(int type, vtkIdTypeArray* array)
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

int EdgeGeom::IsHomogeneous()
{
  return 1;
}

void EdgeGeom::Allocate(vtkIdType numCells, int extSize)
{
  vtkErrorMacro("Read only container.");
}

vtkIdType EdgeGeom::InsertNextCell(int type, const vtkIdList ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType EdgeGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType EdgeGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[])
{
  return -1;
}

void EdgeGeom::ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[])
{
  vtkErrorMacro("Read only container.");
}