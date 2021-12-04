#include "complex2VtkLib/VtkBridge/VtkBridge.hpp"
#include "Utilities.hpp"


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
  std::shared_ptr<DataStructure> dataStructure = std::shared_ptr<DataStructure>(new DataStructure);

  CreateEbsdTestDataStructure(dataStructure);

  DataPath smallIN100_DataPath = DataPath({k_SmallIN100});
  DataPath scanDataPath = smallIN100_DataPath.createChildPath(k_EbsdScanData);
  DataPath confidenceIndexDataPath = scanDataPath.createChildPath(k_ConfidenceIndex);
  DataPath featureIdsDataPath = scanDataPath.createChildPath(k_FeatureIds);
  DataPath imageQualityDataPath = scanDataPath.createChildPath(k_ImageQuality);
  DataPath phaseDataPath = scanDataPath.createChildPath(k_Phases);
  DataPath ipfColorsDataPath = scanDataPath.createChildPath(k_IpfColors);

  DataPath scanDataImageGeomDataPath =scanDataPath.createChildPath(k_SmallIn100ImageGeom);
  auto imageData = WrapGeometryV1(dataStructure, featureIdsDataPath, scanDataImageGeomDataPath);
  //auto imageData = WrapGeometryV2(dataStructure);

  render(imageData);

  return EXIT_SUCCESS;
}
