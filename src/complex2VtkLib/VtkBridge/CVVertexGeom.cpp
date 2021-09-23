#include "CVVertexGeom.hpp"

#include <cmath>

#include <vtkCellType.h>
#include <vtkCellTypes.h>
#include <vtkIdTypeArray.h>

using namespace CV;

CVVertexGrid* CVVertexGrid::New()
{
  return new CVVertexGrid();
}

VertexGeom* VertexGeom::New()
{
  return new VertexGeom();
}

void VertexGeom::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Elements: " << GetNumberOfCells() << endl;
  os << indent << "CellType: " << vtkCellTypes::GetClassNameFromTypeId(CELL_TYPE) << endl;
  os << indent << "CellSize: " << GetMaxCellSize() << endl;
  os << indent << "NumberOfCells: " << GetNumberOfCells() << endl;
}

VTK_PTR(vtkDataSet) CV::VertexGeom::CreateFromGeom(const std::shared_ptr<complex::VertexGeom>& geom)
{
  if(geom == nullptr)
  {
    return nullptr;
  }
  VTK_NEW(CVVertexGrid, dataSet);
  auto* cvGeom = dataSet->GetImplementation();
  cvGeom->SetGeometry(geom);

  return dataSet;
}

VertexGeom::VertexGeom()
: vtkObject()
{
}

void VertexGeom::SetGeometry(const std::shared_ptr<complex::VertexGeom>& VertexGeom)
{
  m_Geom = VertexGeom;
}

vtkIdType VertexGeom::GetNumberOfCells()
{
  if(nullptr == m_Geom)
  {
    vtkErrorMacro("Wrapper Geometry missing a Geometry object");
    return -1;
  }

  return m_Geom->getNumberOfElements();
}

int VertexGeom::GetCellType(vtkIdType cellId)
{
  if(0 == GetNumberOfCells())
  {
    return VTK_EMPTY_CELL;
  }

  return CELL_TYPE;
}

void VertexGeom::GetCellPoints(vtkIdType cellId, vtkIdList* ptIds)
{
  const int numVerts = 1;

  ptIds->SetNumberOfIds(numVerts);
  ptIds->SetId(0, cellId);
}

void VertexGeom::GetPointCells(vtkIdType ptId, vtkIdList* cellIds)
{
  int count = m_Geom->getVertices()->getNumberOfTuples();

  for(int i = 0; i < count; i++)
  {
    cellIds->SetId(i, i);
  }
}

int VertexGeom::GetMaxCellSize()
{
  return std::ceil(m_MaxCellSize);
}

void VertexGeom::GetIdsOfCellsOfType(int type, vtkIdTypeArray* array)
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

int VertexGeom::IsHomogeneous()
{
  return 1;
}

void VertexGeom::Allocate(vtkIdType numCells, int extSize)
{
  vtkErrorMacro("Read only container.");
}

vtkIdType VertexGeom::InsertNextCell(int type, const vtkIdList ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType VertexGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType VertexGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[])
{
  return -1;
}

void VertexGeom::ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[])
{
  vtkErrorMacro("Read only container.");
}
