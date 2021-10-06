#include "CVTriangleGeom.hpp"

#include <cmath>

#include <vtkCellType.h>
#include <vtkCellTypes.h>
#include <vtkIdTypeArray.h>

using namespace CV;

CVTriangleGrid* CVTriangleGrid::New()
{
  return new CVTriangleGrid();
}

TriangleGeom* TriangleGeom::New()
{
  TriangleGeom* geom = new TriangleGeom();
  return geom;
}

void TriangleGeom::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Elements: " << GetNumberOfCells() << endl;
  os << indent << "CellType: " << vtkCellTypes::GetClassNameFromTypeId(CELL_TYPE) << endl;
  os << indent << "CellSize: " << GetMaxCellSize() << endl;
  os << indent << "NumberOfCells: " << GetNumberOfCells() << endl;
}

VTK_PTR(vtkDataSet) CV::TriangleGeom::CreateFromGeom(const std::shared_ptr<complex::TriangleGeom>& geom)
{
  if(geom == nullptr)
  {
    return nullptr;
  }
  VTK_NEW(CVTriangleGrid, dataSet);
  auto* cvGeom = dataSet->GetImplementation();
  cvGeom->SetGeometry(geom);

  return dataSet;
}

TriangleGeom::TriangleGeom()
: vtkObject()
{
}

void TriangleGeom::SetGeometry(const std::shared_ptr<complex::TriangleGeom>& geom)
{
  m_Geom = geom;
  geom->findElementsContainingVert();
  geom->findElementSizes();
}

vtkIdType TriangleGeom::GetNumberOfCells()
{
  if(nullptr == m_Geom)
  {
    vtkErrorMacro("Wrapper Geometry missing a Geometry object");
    return -1;
  }

  return m_Geom->getNumberOfElements();
}

int TriangleGeom::GetCellType(vtkIdType cellId)
{
  if(0 == GetNumberOfCells())
  {
    return VTK_EMPTY_CELL;
  }

  return CELL_TYPE;
}

void TriangleGeom::GetCellPoints(vtkIdType cellId, vtkIdList* ptIds)
{
  const int numVerts = 3;

  size_t verts[numVerts];
  m_Geom->getVertsAtTri(cellId, verts);

  ptIds->SetNumberOfIds(numVerts);
  for(int i = 0; i < numVerts; i++)
  {
    ptIds->SetId(i, verts[i]);
  }
}

void TriangleGeom::GetPointCells(vtkIdType ptId, vtkIdList* cellIds)
{
  auto elementsContainingList = m_Geom->getElementsContainingVert();
  complex::AbstractGeometry::ElementDynamicList::ElementList listArray = elementsContainingList->getElementList(ptId);

  cellIds->SetNumberOfIds(listArray.numCells);
  for(int i = 0; i < listArray.numCells; i++)
  {
    cellIds->SetId(i, listArray.cells[i]);
  }
}

int TriangleGeom::GetMaxCellSize()
{
  return std::ceil(m_MaxCellSize);
}

void TriangleGeom::GetIdsOfCellsOfType(int type, vtkIdTypeArray* array)
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

int TriangleGeom::IsHomogeneous()
{
  return 1;
}

void TriangleGeom::Allocate(vtkIdType numCells, int extSize)
{
  vtkErrorMacro("Read only container.");
}

vtkIdType TriangleGeom::InsertNextCell(int type, const vtkIdList ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType TriangleGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[])
{
  vtkErrorMacro("Read only container.");
  return -1;
}

vtkIdType TriangleGeom::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[])
{
  // TriangleGeometry should probably not be modified by VTK calls
  // To prevent spamming errors when loading this data, do not call vtkErrorMacro
  return -1;
}

void TriangleGeom::ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[])
{
  vtkErrorMacro("Read only container.");
}
