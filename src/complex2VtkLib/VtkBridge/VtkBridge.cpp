#include "VtkBridge.hpp"

#include "vtkCellData.h"

#include "complex/DataStructure/Geometry/AbstractGeometry.hpp"
#include "complex/DataStructure/DataArray.hpp"
#include "complex/DataStructure/Geometry/ImageGeom.hpp"
#include "complex/DataStructure/Geometry/LinkedGeometryData.hpp"



#include "complex2VtkLib/VtkBridge/CVArray.hpp"
#include "complex2VtkLib/VtkBridge/CVEdgeGeom.hpp"
#include "complex2VtkLib/VtkBridge/CVImageGeom.hpp"
#include "complex2VtkLib/VtkBridge/CVQuadGeom.hpp"
#include "complex2VtkLib/VtkBridge/CVTetrahedralGeom.hpp"
#include "complex2VtkLib/VtkBridge/CVTriangleGeom.hpp"
#include "complex2VtkLib/VtkBridge/CVVertexGeom.hpp"

std::vector<std::shared_ptr<complex::AbstractGeometry>> findGeometries(complex::BaseGroup* parent)
{
  std::vector<std::shared_ptr<complex::AbstractGeometry>> geoms;
  for(const auto& [id, data] : *parent)
  {
    if(auto geom = std::dynamic_pointer_cast<complex::AbstractGeometry>(data))
    {
      geoms.push_back(geom);
    }
    else if(auto group = std::dynamic_pointer_cast<complex::BaseGroup>(data))
    {
      auto additional = findGeometries(group.get());
      geoms.insert(geoms.end(), additional.begin(), additional.end());
    }
  }
  return geoms;
}

/**
 * @brief Finds and returns a collection of all geometries within the specified DataStructure.
 * @param ds
 * @return std::vector<complex::AbstractGeometry*>
*/
std::vector<std::shared_ptr<complex::AbstractGeometry>> findGeometries(const complex::DataStructure& ds)
{
  std::vector<std::shared_ptr<complex::AbstractGeometry>> geoms;
  for(const auto& [id, data] : ds)
  {
    if(auto geom = std::dynamic_pointer_cast<complex::AbstractGeometry>(data))
    {
      geoms.push_back(geom);
    }
    else if(auto group = std::dynamic_pointer_cast<complex::BaseGroup>(data))
    {
      auto additional = findGeometries(group.get());
      geoms.insert(geoms.end(), additional.begin(), additional.end());
    }
  }
  return geoms;
}

std::vector<complex::DataObject::IdType> findGeometryIds(const complex::DataStructure& ds)
{
  auto geometries = findGeometries(ds);
  std::vector<complex::DataObject::IdType> ids;
  for(const auto& geometry : geometries)
  {
    ids.push_back(geometry->getId());
  }
  return ids;
}

std::vector<complex::DataObject::IdType> findGeometryIds(complex::BaseGroup* parent)
{
  auto geometries = findGeometries(parent);
  std::vector<complex::DataObject::IdType> ids;
  for(const auto& geometry : geometries)
  {
    ids.push_back(geometry->getId());
  }
  return ids;
}

/**
 * @brief Finds and returns an array of DataObject IDs within the specified group.
 * @param geom 
 * @return std::vector<complex::DataObject::IdType>
 */
std::vector<complex::DataObject::IdType> findDataArrays(const std::shared_ptr<complex::BaseGroup>& parent)
{
  std::vector<complex::DataObject::IdType> ids;
  for(const auto& [id, data] : *parent)
  {
    if(std::dynamic_pointer_cast<complex::DataArray<int8_t>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<int16_t>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<int32_t>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<int64_t>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<uint8_t>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<uint16_t>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<uint32_t>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<uint64_t>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<float>>(data))
    {
      ids.push_back(id);
    }
    else if(std::dynamic_pointer_cast<complex::DataArray<double>>(data))
    {
      ids.push_back(id);
    }
    else if(auto group = std::dynamic_pointer_cast<complex::BaseGroup>(data))
    {
      auto additional = findGeometryIds(group.get());
      ids.insert(ids.end(), additional.begin(), additional.end());
    }
  }
  return ids;
}

std::vector<VTK_PTR(vtkDataSet)> CV::VtkBridge::wrapDataStructure(const complex::DataStructure& ds)
{
  std::vector<VTK_PTR(vtkDataSet)> wrappedGeoms;
  auto geoms = findGeometries(ds);
  for(const auto& geom : geoms)
  {
    auto wrappedGeom = wrapGeometryWithArrays(geom);
    wrappedGeoms.push_back(wrappedGeom);
  }

  return wrappedGeoms;
}

VTK_PTR(vtkDataSet) CV::VtkBridge::wrapGeometry(const std::shared_ptr<complex::AbstractGeometry>& geom)
{
  if(auto edge = std::dynamic_pointer_cast<complex::EdgeGeom>(geom))
  {
    return CV::EdgeGeom::CreateFromGeom(edge);
  }
  if(auto image = std::dynamic_pointer_cast<complex::ImageGeom>(geom))
  {
    return CV::ImageGeom::CreateFromGeom(image);
  }
  if(auto quad = std::dynamic_pointer_cast<complex::QuadGeom>(geom))
  {
    return CV::QuadGeom::CreateFromGeom(quad);
  }
  if(auto tetrahedral = std::dynamic_pointer_cast<complex::TetrahedralGeom>(geom))
  {
    return CV::TetrahedralGeom::CreateFromGeom(tetrahedral);
  }
  if(auto tri = std::dynamic_pointer_cast<complex::TriangleGeom>(geom))
  {
    return CV::TriangleGeom::CreateFromGeom(tri);
  }
  if(auto verts = std::dynamic_pointer_cast<complex::VertexGeom>(geom))
  {
    return CV::VertexGeom::CreateFromGeom(verts);
  }
  return nullptr;
}

VTK_PTR(vtkDataSet) CV::VtkBridge::wrapGeometryWithArrays(const std::shared_ptr<complex::AbstractGeometry>& geom)
{
  vtkSmartPointer<vtkDataSet> wrappedGeom = wrapGeometry(geom);
  const size_t geomTupleCount = geom->getNumberOfElements();
  const complex::DataStructure* dataStructure = geom->getDataStructure();

  complex::LinkedGeometryData& geomData = geom->getLinkedGeometryData();
  std::set<complex::DataPath> dataPaths = geomData.getCellDataPaths();
  for(const auto& dataPath : dataPaths)
  {
    complex::DataObject::IdType objectId = dataStructure->getId(dataPath).value();
    vtkDataArray* wrappedArray = wrapDataArray(*dataStructure, objectId);
    if(wrappedArray == nullptr)
    {
      continue;
    }
    if(geomTupleCount != wrappedArray->GetNumberOfTuples())
    {
      continue;
    }

    vtkCellData* cellData = wrappedGeom->GetCellData();
    cellData->AddArray(wrappedArray);
    cellData->SetActiveScalars(wrappedArray->GetName());
  }

  return wrappedGeom;
}

vtkDataArray* CV::VtkBridge::wrapDataArray(const complex::DataStructure& ds, complex::DataObject::IdType arrayId)
{
  auto dataArray = ds.getSharedData(arrayId);
  return wrapDataArray(dataArray);
}

vtkDataArray* CV::VtkBridge::wrapDataArray(const std::shared_ptr<complex::DataObject>& dataArray)
{
  if(dataArray == nullptr)
  {
    return nullptr;
  }

  if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<int8_t>>(dataArray))
  {
    return new CV::Array<int8_t>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<int16_t>>(dataArray))
  {
    return new CV::Array<int16_t>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<int32_t>>(dataArray))
  {
    return new CV::Array<int32_t>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<int64_t>>(dataArray))
  {
    return new CV::Array<int64_t>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<uint8_t>>(dataArray))
  {
    return new CV::Array<uint8_t>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<uint16_t>>(dataArray))
  {
    return new CV::Array<uint16_t>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<uint32_t>>(dataArray))
  {
    return new CV::Array<uint32_t>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<uint64_t>>(dataArray))
  {
    return new CV::Array<uint64_t>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<float>>(dataArray))
  {
    return new CV::Array<float>(castArr);
  }
  else if(auto castArr = std::dynamic_pointer_cast<complex::DataArray<double>>(dataArray))
  {
    return new CV::Array<double>(castArr);
  }

  return nullptr;
}
