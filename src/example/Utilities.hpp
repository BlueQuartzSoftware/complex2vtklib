#pragma once

#include "complex2VtkLib/VtkBridge/VtkBridge.hpp"
#include "data_dirs.h"

#include "complex/DataStructure/DataStructure.hpp"
#include "complex/DataStructure/DataGroup.hpp"
#include "complex/DataStructure/Geometry/ImageGeom.hpp"
#include "complex/DataStructure/DataStore.hpp"
#include "complex/Utilities/DataArrayUtilities.hpp"
#include "complex/Utilities/UnitTestCommon.hpp"
#include "complex/DataStructure/Geometry/TriangleGeom.hpp"
#include "complex/Parameters/ArrayCreationParameter.hpp"
#include "complex/Parameters/FileSystemPathParameter.hpp"


#include "ComplexCore/Filters/CalculateTriangleAreasFilter.hpp"
#include "ComplexCore/Filters/StlFileReaderFilter.hpp"


#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;


using namespace complex;
using namespace complex::UnitTest;
using namespace complex::Constants;


void ImportStlFile(std::shared_ptr<DataStructure>& dataStructure)
{
  std::string triangleGeometryName = "[Triangle Geometry]";
  std::string triangleFaceDataGroupName = "Face Data";
  std::string normalsDataArrayName = "Normals";

  DataStructure& dataGraph = *dataStructure;
  {
    StlFileReaderFilter filter;
    Arguments args;

    DataGroup::Create(dataGraph, k_LevelZero);

    DataPath parentPath = DataPath({k_LevelZero});

    DataPath normalsDataPath = parentPath.createChildPath(triangleGeometryName).createChildPath(triangleFaceDataGroupName).createChildPath(normalsDataArrayName);

    std::string inputFile = fmt::format("{}/Blade.stl", complex::complex2vtk::k_DataDir.str());

    // Create default Parameters for the filter.
    args.insertOrAssign(StlFileReaderFilter::k_StlFilePath_Key, std::make_any<FileSystemPathParameter::ValueType>(fs::path(inputFile)));
    args.insertOrAssign(StlFileReaderFilter::k_GeometryDataPath_Key, std::make_any<DataPath>({triangleGeometryName}));
    args.insertOrAssign(StlFileReaderFilter::k_FaceGroupDataPath_Key, std::make_any<DataPath>({triangleFaceDataGroupName}));
    args.insertOrAssign(StlFileReaderFilter::k_FaceNormalsDataPath_Key, std::make_any<DataPath>(normalsDataPath));

    // Preflight the filter and check result
    auto preflightResult = filter.preflight(dataGraph, args);

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataGraph, args);

    TriangleGeom& triangleGeom = dataGraph.getDataRefAs<TriangleGeom>(parentPath.createChildPath(triangleGeometryName));
  }

  {
    CalculateTriangleAreasFilter filter;
    Arguments args;
    std::string triangleAreasName = "Triangle Areas";

    DataPath geometryPath = DataPath({k_LevelZero, triangleGeometryName});

    // Create default Parameters for the filter.
    DataPath triangleAreasDataPath = geometryPath.createChildPath(triangleFaceDataGroupName).createChildPath(triangleAreasName);
    args.insertOrAssign(CalculateTriangleAreasFilter::k_TriangleGeometryDataPath_Key, std::make_any<DataPath>(geometryPath));
    args.insertOrAssign(CalculateTriangleAreasFilter::k_CalculatedAreasDataPath_Key, std::make_any<DataPath>(triangleAreasDataPath));

    // Preflight the filter and check result
    auto preflightResult = filter.preflight(dataGraph, args);

    // Execute the filter and check the result
    auto executeResult = filter.execute(dataGraph, args);
  }
}


/**
 *
 * @return
 */
void CreateEbsdTestDataStructure(std::shared_ptr<DataStructure>& dataGraph, StringLiteral topLevel = complex::Constants::k_SmallIN100)
{

  DataGroup* group = complex::DataGroup::Create(*dataGraph, topLevel);
  DataGroup* scanData = complex::DataGroup::Create(*dataGraph, complex::Constants::k_EbsdScanData, group->getId());

  // Create an Image Geometry grid for the Scan Data
  ImageGeom* imageGeom = ImageGeom::Create(*dataGraph, k_SmallIn100ImageGeom, scanData->getId());
  imageGeom->setSpacing({0.25f, 0.25f, 0.25f});
  imageGeom->setOrigin({0.0f, 0.0f, 0.0f});
  complex::SizeVec3 imageGeomDims = {100, 100, 100};
  imageGeom->setDimensions(imageGeomDims); // Listed from slowest to fastest (Z, Y, X)

  // Create some DataArrays; The DataStructure keeps a shared_ptr<> to the DataArray so DO NOT put
  // it into another shared_ptr<>
  std::vector<size_t> compDims = {1};
  std::vector<size_t> tupleDims = {100, 100, 100};

  std::string filePath = complex::complex2vtk::k_DataDir.str() + "/";

  std::string fileName = "ConfidenceIndex.raw";
  complex::ImportFromBinaryFile<float>(filePath + fileName, k_ConfidenceIndex, dataGraph.get(), tupleDims, compDims, scanData->getId());

  fileName = "FeatureIds.raw";
  complex::ImportFromBinaryFile<int32_t>(filePath + fileName, k_FeatureIds, dataGraph.get(), tupleDims, compDims, scanData->getId());

  fileName = "ImageQuality.raw";
  complex::ImportFromBinaryFile<float>(filePath + fileName, k_ImageQuality, dataGraph.get(), tupleDims, compDims, scanData->getId());

  fileName = "Phases.raw";
  complex::ImportFromBinaryFile<int32_t>(filePath + fileName, k_Phases, dataGraph.get(), tupleDims, compDims, scanData->getId());

  fileName = "IPFColors.raw";
  compDims = {3};
  complex::ImportFromBinaryFile<uint8_t>(filePath + fileName, k_IpfColors, dataGraph.get(), tupleDims, compDims, scanData->getId());

  // Add in another group that is just information about the grid data.
  DataGroup* phaseGroup = complex::DataGroup::Create(*dataGraph, k_PhaseData, group->getId());
  Int32Array::CreateWithStore<Int32DataStore>(*dataGraph, k_LaueClass, {2}, compDims, phaseGroup->getId());
}
