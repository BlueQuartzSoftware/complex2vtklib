

#include "complex2VtkLib/VtkBridge/CVArray.hpp"

#include "Utilities.hpp"

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkDataSetMapper.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkLookupTable.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkConeSource.h>
#include <vtkImageData.h>
#include <vtkProperty.h>
#include <vtkPolyDataWriter.h>
#include <vtkLongArray.h>



#include <iostream>
#include <string>


struct NXVtkRenderViewObjects
{
  vtkNew<vtkRenderWindow>                    renderWindow;
  vtkNew<vtkRenderer>                        renderer;
  vtkNew<vtkRenderWindowInteractor>          renderWindowInteractor;
  vtkNew<vtkInteractorStyleTrackballCamera>  interactorStyle;
  vtkNew<vtkNamedColors>                     backgroundColor;
  vtkNew<vtkOrientationMarkerWidget>         orientationMarker;

};

struct NXVtkRenderProperties
{
  std::shared_ptr<complex::DataObject>        complexGeometry;
  std::shared_ptr<complex::UInt64Array>       complexFaceConnectivity; // Used only for Node Based Geometries
  vtkSmartPointer<vtkDataSet>                 dataSet;
  vtkSmartPointer<vtkAlgorithm>               algorithm;
  vtkSmartPointer<vtkMapper>                  mapper;
  vtkSmartPointer<vtkActor>                   actor;
  vtkSmartPointer<vtkLookupTable>             cellColors;
  vtkSmartPointer<vtkLookupTable>             pointColors;

};

using NXVtkRenderPropertiesPtr = std::shared_ptr<NXVtkRenderProperties>;


class NXVtkRenderView
{
public:
  NXVtkRenderView(std::shared_ptr<DataStructure>& dataStructure)
  : m_DataStructure(dataStructure)
  {}
  ~NXVtkRenderView() = default;

  void initRenderWindow() const
  {
    // Visualization
    m_NxVtkRenderObjects.renderWindow->AddRenderer(m_NxVtkRenderObjects.renderer);
    m_NxVtkRenderObjects.renderWindowInteractor->SetRenderWindow(m_NxVtkRenderObjects.renderWindow);
    //
    // By default the vtkRenderWindowInteractor instantiates an instance
    // of vtkInteractorStyle. vtkInteractorStyle translates a set of events
    // it observes into operations on the camera, actors, and/or properties
    // in the vtkRenderWindow associated with the vtkRenderWinodwInteractor.
    // Here we specify a particular interactor style.
    m_NxVtkRenderObjects.renderWindowInteractor->SetInteractorStyle(m_NxVtkRenderObjects.interactorStyle);

    // Create some named colors for the rendering background
    vtkNew<vtkNamedColors> colors;
    // Set the background color.
    std::array<unsigned char, 4> bkg{{51, 77, 102, 255}};
    colors->SetColor("BkgColor", bkg.data());
    m_NxVtkRenderObjects.renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

    // Create the Orientation Axes and add it to the render window
    vtkNew<vtkAxesActor> axes;
    double rgba[4]{0.0, 0.0, 0.0, 0.0};
    colors->GetColor("Carrot", rgba);
    m_NxVtkRenderObjects.orientationMarker->SetOutlineColor(rgba[0], rgba[1], rgba[2]);
    m_NxVtkRenderObjects.orientationMarker->SetOrientationMarker(axes);
    m_NxVtkRenderObjects.orientationMarker->SetInteractor(m_NxVtkRenderObjects.renderWindowInteractor);
    m_NxVtkRenderObjects.orientationMarker->SetViewport(0.0, 0.0, 0.4, 0.4);
    m_NxVtkRenderObjects.orientationMarker->SetEnabled(1);

  }


  void startRendering() const
  {
    m_NxVtkRenderObjects.renderWindowInteractor->Start();
  }

  void addGeometry(const DataPath& geometryPath)
  {
    std::shared_ptr<complex::DataObject> dataObjectShrdPtr = m_DataStructure->getSharedDataAs<complex::DataObject>(geometryPath);
    if(dataObjectShrdPtr != nullptr)
    {
      complex::DataObject::DataObjectType dataObjectType = dataObjectShrdPtr->getDataObjectType();
      switch(dataObjectType)
      {
      case complex::DataObject::DataObjectType::ImageGeom:
        addImageGeometry(dataObjectShrdPtr);
        break;
      case DataObject::DataObjectType::DataObject:
        break;
      case DataObject::DataObjectType::DynamicListArray:
        break;
      case DataObject::DataObjectType::ScalarData:
        break;
      case DataObject::DataObjectType::BaseGroup:
        break;
      case DataObject::DataObjectType::AbstractMontage:
        break;
      case DataObject::DataObjectType::DataGroup:
        break;
      case DataObject::DataObjectType::IDataArray:
        break;
      case DataObject::DataObjectType::DataArray:
        break;
      case DataObject::DataObjectType::AbstractGeometry:
        break;
      case DataObject::DataObjectType::VertexGeom:
        break;
      case DataObject::DataObjectType::EdgeGeom:
        break;
      case DataObject::DataObjectType::AbstractGeometryGrid:
        break;
      case DataObject::DataObjectType::RectGridGeom:
        break;
      case DataObject::DataObjectType::AbstractGeometry2D:
        break;
      case DataObject::DataObjectType::QuadGeom:
        break;
      case DataObject::DataObjectType::TriangleGeom:
        addAbstractGeometry2D<complex::TriangleGeom>(dataObjectShrdPtr);
        break;
      case DataObject::DataObjectType::AbstractGeometry3D:
        break;
      case DataObject::DataObjectType::HexahedralGeom:
        break;
      case DataObject::DataObjectType::TetrahedralGeom:
        break;
      case DataObject::DataObjectType::Unknown:
        break;
      }
    }
  }

  template <typename AbstractGeometry2DType>
  void addAbstractGeometry2D(std::shared_ptr<DataObject>& complexDataObject)
  {
    std::shared_ptr<AbstractGeometry2DType> complexNodeGeometry2D = std::dynamic_pointer_cast<AbstractGeometry2DType>(complexDataObject);
    const complex::DataStructure* dataStructure = complexNodeGeometry2D->getDataStructure();
    const size_t vertCount = complexNodeGeometry2D->getNumberOfVertices();

    // ******************* This section wraps the existing vertices into a vtkPolyObject
    using CVFloatArray = CV::Array<float>;
    std::shared_ptr<complex::Float32Array> vertDataArray = m_DataStructure->getSharedDataAs<complex::Float32Array>(complexNodeGeometry2D->getVertListId());
    vtkDataArray* complexVertData = CV::VtkBridge::wrapDataArray(vertDataArray);
    vtkNew<vtkPoints> complexPoints;
    complexPoints->SetData(complexVertData);
    vtkNew<vtkPointSet> complexPointSet;
    complexPointSet->SetPoints(complexPoints);
    vtkNew<vtkPolyData> polyData;
    polyData->SetPoints(complexPoints);

    vtkNew<vtkCellArray> complexVertArray;
    for(vtkIdType i = 0; i < vertCount; i++)
    {
      complexVertArray->InsertCellPoint(i);
    }
    polyData->SetVerts(complexVertArray);

    // ******************* This section wraps the existing connectivity list into a vtkPolyObject
    std::shared_ptr<complex::UInt64Array> complexFaceConnectivity = m_DataStructure->getSharedDataAs<complex::UInt64Array>(complexNodeGeometry2D->getTriangleArrayId());
    complex::UInt64DataStore* dataStore = complexFaceConnectivity->template getIDataStoreAs<DataStore<uint64_t>>();

    // This is ugly but about the only way to reuse the existing connectivity array
    vtkNew<vtkLongArray> faceConnectivity;
    faceConnectivity->SetArray(reinterpret_cast<long*>(dataStore->data()), complexFaceConnectivity->getSize(), 1);

    vtkNew<vtkCellArray> complexCellArray;
    complexCellArray->SetData(AbstractGeometry2DType::k_NumVerts, faceConnectivity);
    polyData->SetPolys(complexCellArray);

    complex::LinkedGeometryData& linkedGeometryData = complexNodeGeometry2D->getLinkedGeometryData();

    // Wrap the Vertex Data
    std::set<complex::DataPath> dataPaths = linkedGeometryData.getVertexDataPaths();
    for(const auto& dataPath : dataPaths)
    {
      complex::DataObject::IdType objectId = dataStructure->getId(dataPath).value();
      vtkDataArray* wrappedArray = CV::VtkBridge::wrapDataArray(*dataStructure, objectId);
      if(wrappedArray == nullptr)
      {
        continue;
      }
      if(vertCount != wrappedArray->GetNumberOfTuples())
      {
        continue;
      }

      vtkPointData* pointData = polyData->GetPointData();
      pointData->AddArray(wrappedArray);
      pointData->SetActiveScalars(wrappedArray->GetName());
    }

    // Wrap the Face Data
    dataPaths = linkedGeometryData.getFaceDataPaths();
    const size_t faceCount = complexNodeGeometry2D->getNumberOfFaces();
    for(const auto& dataPath : dataPaths)
    {
      complex::DataObject::IdType objectId = dataStructure->getId(dataPath).value();
      vtkDataArray* wrappedArray = CV::VtkBridge::wrapDataArray(*dataStructure, objectId);
      if(wrappedArray == nullptr)
      {
        continue;
      }
      if(faceCount != wrappedArray->GetNumberOfTuples())
      {
        continue;
      }

      vtkCellData* cellData = polyData->GetCellData();
      cellData->AddArray(wrappedArray);
      cellData->SetActiveScalars(wrappedArray->GetName());
    }

    //**************************************************************************
    // Hook up all the vtk objects that are needed to render the vtkDataSet in 3D
    NXVtkRenderPropertiesPtr renderProperties = std::make_shared<NXVtkRenderProperties>();
    renderProperties->complexGeometry = complexDataObject;
    renderProperties->dataSet = polyData;
    renderProperties->complexFaceConnectivity = complexFaceConnectivity; // THis is needed to hold a ref to the shared_ptr<>

    // Create a Rainbow Color Table for the data
    // This color table would ideally be created by a user through a GUI.
    renderProperties->cellColors = vtkNew<vtkLookupTable>();
    renderProperties->cellColors->SetNumberOfColors(256);
    renderProperties->cellColors->SetHueRange(0.667, 0.0);
    renderProperties->cellColors->Build();

    renderProperties->pointColors = vtkNew<vtkLookupTable>();
    renderProperties->pointColors->SetNumberOfColors(256);
    renderProperties->pointColors->SetHueRange(0.667, 0.0);
    renderProperties->pointColors->Build();

    // Create a vtkPolyDataMapper to map the data
    vtkNew<vtkPolyDataMapper> mapper;
    renderProperties->mapper = mapper;
    mapper->SetLookupTable(renderProperties->cellColors);
    mapper->SetScalarRange(0.0, 1.0);
    mapper->SetInputData(vtkPolyData::SafeDownCast(polyData));
    mapper->Update();

    // Create the vtkActor that will represent the complex Geometry
    renderProperties->actor = vtkNew<vtkActor>();
    renderProperties->actor->SetMapper(renderProperties->mapper);
    renderProperties->actor->GetProperty()->SetPointSize(20.0F);
    m_NxVtkRenderObjects.renderer->AddActor(renderProperties->actor);
    m_RenderProperties.push_back(renderProperties);
#if 1
    vtkNew<vtkPolyDataWriter> polyDataWriter;
    polyDataWriter->SetFileName("/tmp/polydata.vtk");
    polyDataWriter->SetFileTypeToASCII();
    polyDataWriter->SetInputData(polyData);
    polyDataWriter->Write();

#endif

    /*
    complex::Point3Dd pCenter = complexNodeGeometry2D->getParametricCenter();
    complex::BoundingBox<double> boundingBox = complexNodeGeometry2D->getBoundingBox();
    std::array<double, 3> boxCenter = boundingBox.center();

    vtkCamera* activeCamera = m_NxVtkRenderObjects.renderer->GetActiveCamera();
    activeCamera->SetFocalPoint(boxCenter.data());
    activeCamera->SetPosition(1.0, 0.0, 0.0 );
    activeCamera->SetViewUp(0.0, 0.0, 1.0);
    m_NxVtkRenderObjects.renderer->ResetCamera();
    */
  }


/**
 * @brief
 * @param dataObject
 */
  void addImageGeometry(const std::shared_ptr<complex::DataObject>& complexDataObject)
  {
    std::shared_ptr<complex::ImageGeom> complexImageGeom = std::dynamic_pointer_cast<complex::ImageGeom>(complexDataObject);
    // Convert the complex Geometry Object to a Wrapped Vtk Object (vtkDataSet)
    SizeVec3 igDims = complexImageGeom->getDimensions();
    FloatVec3 igOrigin = complexImageGeom->getOrigin();
    FloatVec3 igSpacing = complexImageGeom->getSpacing();

    vtkNew<vtkImageData> wrappedVtkDataset;
    wrappedVtkDataset->SetDimensions(igDims[0] + 1, igDims[1] + 1, igDims[2] + 1);
    wrappedVtkDataset->SetOrigin(igOrigin[0], igOrigin[1], igOrigin[2]);
    wrappedVtkDataset->SetSpacing(igSpacing[0], igSpacing[1], igSpacing[2]);

    const complex::DataStructure* dataStructure = complexImageGeom->getDataStructure();
    const size_t geomTupleCount = complexImageGeom->getNumberOfElements();
    complex::LinkedGeometryData& complexCellData = complexImageGeom->getLinkedGeometryData();
    std::set<complex::DataPath> dataPaths = complexCellData.getCellDataPaths();
    for(const auto& dataPath : dataPaths)
    {
      complex::DataObject::IdType objectId = dataStructure->getId(dataPath).value();
      vtkDataArray* wrappedArray = CV::VtkBridge::wrapDataArray(*dataStructure, objectId);
      if(wrappedArray == nullptr)
      {
        continue;
      }
      if(geomTupleCount != wrappedArray->GetNumberOfTuples())
      {
        continue;
      }

      vtkCellData* cellData = wrappedVtkDataset->GetCellData();
      cellData->AddArray(wrappedArray);
      cellData->SetActiveScalars(wrappedArray->GetName());
    }

    //**************************************************************************
    // Hook up all the vtk objects that are needed to render the vtkDataSet in 3D
    NXVtkRenderPropertiesPtr renderProperties = std::make_shared<NXVtkRenderProperties>();
    renderProperties->dataSet = wrappedVtkDataset;
    renderProperties->complexGeometry = complexImageGeom;

    // Create a Rainbow Color Table for the data
    // This color table would ideally be created by a user through a GUI.
    renderProperties->cellColors = vtkNew<vtkLookupTable>();
    renderProperties->cellColors->SetNumberOfColors(256);
    renderProperties->cellColors->SetHueRange(0.667, 0.0);
    renderProperties->cellColors->Build();

    renderProperties->pointColors = vtkNew<vtkLookupTable>();
    renderProperties->pointColors->SetNumberOfColors(256);
    renderProperties->pointColors->SetHueRange(0.667, 0.0);
    renderProperties->pointColors->Build();

    // Create a vtkDataSetMapper to map the data
    vtkNew<vtkDataSetMapper> mapper;
    renderProperties->mapper = mapper;
    mapper->SetLookupTable(renderProperties->cellColors);
    mapper->SetScalarRange(0.0, 1.0);
    mapper->SetInputData(wrappedVtkDataset);
    mapper->Update();

    // Create the vtkActor that will represent the complex Geometry
    renderProperties->actor = vtkNew<vtkActor>();
    renderProperties->actor->SetMapper(renderProperties->mapper);
    m_NxVtkRenderObjects.renderer->AddActor(renderProperties->actor);
    m_RenderProperties.push_back(renderProperties);

    complex::Point3Dd pCenter = complexImageGeom->getParametricCenter();
    complex::BoundingBox<double> boundingBox = complexImageGeom->getBoundingBox();
    std::array<double, 3> boxCenter = boundingBox.center();

    vtkCamera* activeCamera = m_NxVtkRenderObjects.renderer->GetActiveCamera();
    activeCamera->SetFocalPoint(boxCenter.data());
    activeCamera->SetPosition(1.0, 0.0, 0.0 );
    activeCamera->SetViewUp(0.0, 0.0, 1.0);
    m_NxVtkRenderObjects.renderer->ResetCamera();
  }


  void setActiveScalars(size_t activeGeometryIndex, const DataPath& dataPath)
  {
    NXVtkRenderPropertiesPtr renderProperties = m_RenderProperties[activeGeometryIndex];
    vtkCellData* cellData = renderProperties->dataSet->GetCellData();
    int result = cellData->SetActiveScalars(dataPath.getTargetName().c_str());
    vtkDataArray* dataArray = cellData->GetScalars(dataPath.getTargetName().c_str());
    std::array<double, 2> dataRange = {0.0, 1.0};
    dataArray->GetRange(dataRange.data());
    renderProperties->mapper->SetScalarRange(dataRange[0], dataRange[1]);
    renderProperties->mapper->Update();
  }

  void setMapScalars(size_t activeGeometryIndex, bool mapScalars)
  {
    NXVtkRenderPropertiesPtr renderProperties = m_RenderProperties[activeGeometryIndex];

    if(mapScalars)
    {
      renderProperties->mapper->SetLookupTable(renderProperties->cellColors);
    }
    else
    {
      renderProperties->mapper->SetColorModeToDirectScalars();
    }
  }

  NXVtkRenderViewObjects* renderObjects()
  {
    return &m_NxVtkRenderObjects;
  }

private:
  std::shared_ptr<DataStructure> m_DataStructure;
  NXVtkRenderViewObjects m_NxVtkRenderObjects;
  std::vector<NXVtkRenderPropertiesPtr> m_RenderProperties;
};

void ImportStlFile(std::shared_ptr<DataStructure>& dataStructure, NXVtkRenderView& nxVtkRenderView, size_t geomIndex)
{
  ImportStlFile(dataStructure);

  std::string triangleGeometryName = "[Triangle Geometry]";

  DataPath geometryPath = DataPath({k_LevelZero, triangleGeometryName});
  std::string triangleFaceDataGroupName = "Face Data";
  std::string triangleAreasName = "Triangle Areas";

  DataPath triangleAreasDataPath = geometryPath.createChildPath(triangleFaceDataGroupName).createChildPath(triangleAreasName);

  // Set the Cell Data of the ImageGeometry
  AbstractGeometry* geometry = dataStructure->getDataAs<AbstractGeometry>(geometryPath);
  geometry->getLinkedGeometryData().addFaceData(triangleAreasDataPath);

  nxVtkRenderView.addGeometry(geometryPath);
  nxVtkRenderView.setActiveScalars(geomIndex, triangleAreasDataPath);
  nxVtkRenderView.setMapScalars(geomIndex, true);
}

void CreateImageGeometry(std::shared_ptr<DataStructure>& dataStructure, NXVtkRenderView& nxVtkRenderView,
                         StringLiteral topLevel, FloatVec3& origin, size_t geoIndex)
{
  CreateEbsdTestDataStructure(dataStructure, topLevel);

  DataPath smallIN100_DataPath = DataPath({topLevel});
  DataPath scanDataPath = smallIN100_DataPath.createChildPath(k_EbsdScanData);
  DataPath confidenceIndexDataPath = scanDataPath.createChildPath(k_ConfidenceIndex);
  DataPath featureIdsDataPath = scanDataPath.createChildPath(k_FeatureIds);
  DataPath imageQualityDataPath = scanDataPath.createChildPath(k_ImageQuality);
  DataPath phaseDataPath = scanDataPath.createChildPath(k_Phases);
  DataPath ipfColorsDataPath = scanDataPath.createChildPath(k_IpfColors);

  DataPath scanDataImageGeomDataPath = scanDataPath.createChildPath(k_SmallIn100ImageGeom);

  // Set the Cell Data of the ImageGeometry
  AbstractGeometry* geometry = dataStructure->getDataAs<AbstractGeometry>(scanDataImageGeomDataPath);
  ImageGeom* imageGeom = dataStructure->getDataAs<ImageGeom>(scanDataImageGeomDataPath);
  imageGeom->setOrigin(origin);
  geometry->getLinkedGeometryData().addCellData(confidenceIndexDataPath);
  geometry->getLinkedGeometryData().addCellData(featureIdsDataPath);
  geometry->getLinkedGeometryData().addCellData(imageQualityDataPath);
  geometry->getLinkedGeometryData().addCellData(phaseDataPath);
  geometry->getLinkedGeometryData().addCellData(ipfColorsDataPath);

  nxVtkRenderView.addGeometry(scanDataImageGeomDataPath);
  nxVtkRenderView.setActiveScalars(geoIndex, ipfColorsDataPath);
  nxVtkRenderView.setMapScalars(geoIndex, false);
}

int main(int argc, char* argv[])
{
  std::cout << "VtkDemo Starting... " << std::endl;
  std::shared_ptr<DataStructure> dataStructure = std::shared_ptr<DataStructure>(new DataStructure);
  size_t geoIndex = 0;
  // Instantiate our VtkRendering Class
  NXVtkRenderView nxVtkRenderView(dataStructure);
  nxVtkRenderView.initRenderWindow();
  FloatVec3 origin = {40.0F, 0.0F, 0.0F};
  CreateImageGeometry(dataStructure, nxVtkRenderView, complex::Constants::k_SmallIN100, origin, geoIndex++);

  origin = {30.0F, 30.0F, 30.0F};
  CreateImageGeometry(dataStructure, nxVtkRenderView, complex::Constants::k_SmallIN1002, origin, geoIndex++);

  ImportStlFile(dataStructure, nxVtkRenderView, geoIndex++);


  // This next line will start the interactive rendering and not return until the vtkWindow is closed.
  nxVtkRenderView.startRendering();

  return EXIT_SUCCESS;
}
