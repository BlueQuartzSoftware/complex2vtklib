#include "CVImageGeom.hpp"

#include <cmath>

#include <vtkCellType.h>
#include <vtkCellTypes.h>
#include <vtkIdTypeArray.h>
#include <vtkPoints.h>

using namespace CV;

ImageGeom* ImageGeom::New()
{
  return new ImageGeom();
}

void ImageGeom::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Elements: " << GetNumberOfCells() << endl;
  os << indent << "CellSize: " << GetMaxCellSize() << endl;
  os << indent << "NumberOfCells: " << GetNumberOfCells() << endl;
}

VTK_PTR(vtkDataSet) CV::ImageGeom::CreateFromGeom(const std::shared_ptr<complex::ImageGeom>& geom)
{
  if(geom == nullptr)
  {
    return nullptr;
  }
  VTK_NEW(ImageGeom, dataSet);
  dataSet->SetGeometry(geom);

  return dataSet;
}

ImageGeom::ImageGeom()
: vtkImageData()
{
}

void ImageGeom::SetGeometry(const std::shared_ptr<complex::ImageGeom>& imageGeom)
{
  m_Geom = imageGeom;
  if(imageGeom == nullptr)
  {
    SetDimensions(0, 0, 0);
    return;
  }

  complex::SizeVec3 imageDims = imageGeom->getDimensions();
  SetDimensions(imageDims.getX() + 1, imageDims.getY() + 1, imageDims.getZ() + 1);
}
