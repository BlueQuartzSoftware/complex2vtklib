#include "complex2VtkLib/VtkBridge/VtkBridge.hpp"

#include "complex/Common/Types.hpp"
#include "complex/DataStructure/DataArray.hpp"
#include "complex/DataStructure/DataGroup.hpp"
#include "complex/DataStructure/DataPath.hpp"
#include "complex/DataStructure/DataStore.hpp"
#include "complex/DataStructure/DataStructure.hpp"
#include "complex/DataStructure/Geometry/ImageGeom.hpp"
#include "complex/Filter/FilterHandle.hpp"
#include "complex/Utilities/Parsing/HDF5/H5FileReader.hpp"
#include "complex/Utilities/Parsing/HDF5/H5FileWriter.hpp"


#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkImageData.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

#include "data_dirs.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>

namespace fs = std::filesystem;

using namespace complex;

template <typename T>
DataArray<T>* ReadFromFile(const std::string& filename, const std::string& name, DataStructure* dataGraph, size_t numTuples, size_t numComponents, DataObject::IdType parentId = {})
{
  std::cout << "  Reading file " << filename << std::endl;
  using DataStoreType = DataStore<T>;
  using ArrayType = DataArray<T>;
  constexpr size_t defaultBlocksize = 1048576;

  if(!fs::exists(filename))
  {
    std::cout << "File Does Not Exist:'" << filename << "'" << std::endl;
    return nullptr;
  }

  auto dataStore = new DataStoreType({numTuples}, {numComponents});
  ArrayType* dataArray = ArrayType::Create(*dataGraph, name, dataStore, parentId);

  const size_t fileSize = fs::file_size(filename);
  const size_t numBytesToRead = dataArray->getSize() * sizeof(T);
  if(numBytesToRead != fileSize)
  {
    std::cout << "FileSize and Allocated Size do not match" << std::endl;
    return nullptr;
  }

  FILE* f = std::fopen(filename.c_str(), "rb");
  if(f == nullptr)
  {
    return nullptr;
  }

  std::byte* chunkptr = reinterpret_cast<std::byte*>(dataStore->data());

  // Now start reading the data in chunks if needed.
  size_t chunkSize = std::min(numBytesToRead, defaultBlocksize);

  size_t masterCounter = 0;
  while(masterCounter < numBytesToRead)
  {
    size_t bytesRead = std::fread(chunkptr, sizeof(std::byte), chunkSize, f);
    chunkptr += bytesRead;
    masterCounter += bytesRead;

    size_t bytesLeft = numBytesToRead - masterCounter;

    if(bytesLeft < chunkSize)
    {
      chunkSize = bytesLeft;
    }
  }

  fclose(f);

  return dataArray;
}

std::shared_ptr<DataStructure> CreateDataStructure()
{
  std::shared_ptr<DataStructure> dataGraph = std::shared_ptr<DataStructure>(new DataStructure);

  dataGraph->makePath(DataPath::FromString("1/2/3/4/5").value());

  DataGroup* group = complex::DataGroup::Create(*dataGraph, "Small IN100");
  DataGroup* scanData = complex::DataGroup::Create(*dataGraph, "EBSD Scan Data", group->getId());

  dataGraph->makePath(DataPath::FromString("Small IN100/EBSD Scan Data").value());

  // Create an Image Geometry grid for the Scan Data
  ImageGeom* imageGeom = ImageGeom::Create(*dataGraph, "Small IN100 Grid", scanData->getId());
  imageGeom->setSpacing({0.25F, 0.25F, 0.25F});
  imageGeom->setOrigin({0.0F, 0.0F, 0.0F});
  complex::SizeVec3 imageGeomDims = {100, 100, 100};
  imageGeom->setDimensions(imageGeomDims); // Listed from slowest to fastest (Z, Y, X)

  std::cout << "Creating Data Structure" << std::endl;
  // Create some DataArrays; The DataStructure keeps a shared_ptr<> to the DataArray so DO NOT put
  // it into another shared_ptr<>
  size_t tupleSize = 1;
  size_t tupleCount = imageGeom->getNumberOfElements();

  std::string filePath = complex2vtk::k_DataDir.str();

  std::string fileName = "ConfidenceIndex.raw";
  ReadFromFile<float>(filePath + fileName, "Confidence Index", dataGraph.get(), tupleCount, tupleSize, imageGeom->getId());

  fileName = "FeatureIds.raw";
  ReadFromFile<int32_t>(filePath + fileName, "FeatureIds", dataGraph.get(), tupleCount, tupleSize, imageGeom->getId());

  fileName = "ImageQuality.raw";
  ReadFromFile<float>(filePath + fileName, "Image Quality", dataGraph.get(), tupleCount, tupleSize, imageGeom->getId());

  fileName = "IPFColors.raw";
  ReadFromFile<uint8_t>(filePath + fileName, "IPF Colors", dataGraph.get(), tupleCount * 3, tupleSize, imageGeom->getId());

  fileName = "Phases.raw";
  ReadFromFile<int32_t>(filePath + fileName, "Phases", dataGraph.get(), tupleCount, tupleSize, imageGeom->getId());

  // Add in another group that is just information about the grid data.
  DataGroup* phaseGroup = complex::DataGroup::Create(*dataGraph, "Phase Data", group->getId());
  tupleSize = 1;
  tupleCount = 2;
  auto laueDataStore = new Int32DataStore({tupleSize}, {tupleCount});
  Int32Array::Create(*dataGraph, "Laue Class", laueDataStore, phaseGroup->getId());

  return dataGraph;
}

VTK_PTR(vtkDataSet)
WrapGeometryV1(const std::shared_ptr<DataStructure>& dataStructure, const DataPath& arrayPath, const DataPath& geomPath)
{
  auto dataArrayObject = dataStructure->getSharedData(arrayPath);
  auto vInt32Array = CV::VtkBridge::wrapDataArray(dataArrayObject);

  // Get Complex Image Geometry
  auto absGeom = dataStructure->getSharedDataAs<AbstractGeometry>(geomPath);
  auto dataset = CV::VtkBridge::wrapGeometry(absGeom);

  // Get the vtkCellData Data from the vtkImageData object and add our wrapped Complex Data Array to the CellData
  vtkCellData* cellData = dataset->GetCellData();
  cellData->AddArray(vInt32Array);
  // Set the Active Scalars for the vtkDataSet
  cellData->SetActiveScalars(dataArrayObject->getName().c_str());

  return dataset;
}

VTK_PTR(vtkDataSet)
WrapGeometryV2(const std::shared_ptr<DataStructure>& dataStructure)
{
  auto wrappedGeoms = CV::VtkBridge::wrapDataStructure(*dataStructure);
  if(wrappedGeoms.size() == 0)
  {
    return nullptr;
  }
  return wrappedGeoms.front();
}

void render(VTK_PTR(vtkDataSet) dataset)
{
  // Create  GrayScale Color Table for the data
  vtkNew<vtkLookupTable> grayScaleLut;
  grayScaleLut->SetHueRange(0, 0);
  grayScaleLut->SetSaturationRange(0, 0);
  grayScaleLut->SetValueRange(0.2, 1.0);
  grayScaleLut->SetNumberOfColors(256);
  grayScaleLut->SetHueRange(0.0, 0.667);
  grayScaleLut->Build();

  // Create a Rainbow Color Table for the data
  vtkNew<vtkLookupTable> rainbowBlueRedLut;
  rainbowBlueRedLut->SetNumberOfColors(256);
  rainbowBlueRedLut->SetHueRange(0.667, 0.0);
  rainbowBlueRedLut->Build();

  // Create a vtkDataSetMapper to map the data
  vtkNew<vtkDataSetMapper> dataSetMapper;
  dataSetMapper->SetLookupTable(rainbowBlueRedLut);
  dataSetMapper->SetScalarRange(1.0, 795.0);
  dataSetMapper->SetInputData(dataset);
  dataSetMapper->Update();

  // Create some named colors for the rendering background
  vtkNew<vtkNamedColors> colors;
  // Set the background color.
  std::array<unsigned char, 4> bkg{{51, 77, 102, 255}};
  colors->SetColor("BkgColor", bkg.data());

  vtkNew<vtkActor> actor;
  actor->SetMapper(dataSetMapper);

  // Visualization
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  //
  // By default the vtkRenderWindowInteractor instantiates an instance
  // of vtkInteractorStyle. vtkInteractorStyle translates a set of events
  // it observes into operations on the camera, actors, and/or properties
  // in the vtkRenderWindow associated with the vtkRenderWinodwInteractor.
  // Here we specify a particular interactor style.
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  renderWindowInteractor->SetInteractorStyle(style);

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

  vtkNew<vtkAxesActor> axes;

  vtkNew<vtkOrientationMarkerWidget> widget;
  double rgba[4]{0.0, 0.0, 0.0, 0.0};
  colors->GetColor("Carrot", rgba);
  widget->SetOutlineColor(rgba[0], rgba[1], rgba[2]);
  widget->SetOrientationMarker(axes);
  widget->SetInteractor(renderWindowInteractor);
  widget->SetViewport(0.0, 0.0, 0.4, 0.4);
  widget->SetEnabled(1);
  // widget->InteractiveOn();

  renderWindow->SetWindowName("ImageDataGeometryFilter");
  renderWindow->Render();
  renderWindowInteractor->Start();
}

int main(int argc, char* argv[])
{
  std::shared_ptr<DataStructure> dataStructure = CreateDataStructure();

  DataPath scanData_DataPath = DataPath({"Small IN100", "EBSD Scan Data"});
  DataPath featureIdsDataPath = DataPath({"Small IN100", "EBSD Scan Data", "Small IN100 Grid", "FeatureIds"});
  DataPath scanDataImageGeomDataPath = DataPath({"Small IN100", "EBSD Scan Data", "Small IN100 Grid"});

  auto imageData = WrapGeometryV1(dataStructure, featureIdsDataPath, scanDataImageGeomDataPath);
  //auto imageData = WrapGeometryV2(dataStructure);

  render(imageData);

  return EXIT_SUCCESS;
}
