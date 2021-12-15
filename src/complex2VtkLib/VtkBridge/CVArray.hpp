#pragma once

#include <memory>

#include "vtkGenericDataArray.h"
#include "vtkSetGet.h"

#include "complex/DataStructure/DataArray.hpp"
#include "complex/DataStructure/DataStore.hpp"

#include "complex2VtkLib/complex2VtkLib_export.hpp"

namespace CV
{
/**
 * @class CV::Array
 * @brief The CVArray class serves as a wrapper around a complex DataArray to
 * make it available for use in VTK without duplicating the underlying data.
 * @tparam T
 */
template <class T>
class Array : public vtkGenericDataArray<CV::Array<T>, T>
{
public:
  using ComplexArrayType = complex::DataArray<T>;
  using ComplexArrayPointerType = std::shared_ptr<ComplexArrayType>;
  using ValueType = T;
  using Superclass2 = vtkGenericDataArray<CV::Array<T>, T>;

  vtkAbstractTypeMacro(CV::Array<T>, Superclass2);

  static inline const std::string MissingArrayName = "[Missing Array]";

  /**
   * @brief Creates a new instance of CV::Array. This is required of vtkObject derived classes
   * @return
   */
  static CV::Array<T>* New()
  {
    return new CV::Array<T>();
  }

  /**
   * @brief
   */
  Array()
  : Superclass()
  , m_DataArray(nullptr)
  {
    SetComplexArray(nullptr);
  }

  /**
   * @brief
   * @param dataArr
   */
  Array(const ComplexArrayPointerType& dataArr)
  : Superclass()
  , m_DataArray(dataArr)
  {
    SetComplexArray(dataArr);
  }

  Array(const Array&) = delete;
  Array(Array&&) noexcept = delete;
  Array& operator=(const Array&) = delete;
  Array& operator=(Array&&) noexcept = delete;

  virtual ~Array() = default;

  /**
   * @brief
   * @param dataArray
   */
  void SetComplexArray(const ComplexArrayPointerType& dataArray)
  {
    m_DataArray = dataArray;
    if(dataArray == nullptr)
    {
      SetName(MissingArrayName.c_str());
    }
    else
    {
      SetName(dataArray->getName().c_str());
      this->NumberOfComponents = m_DataArray->getNumberOfComponents();
      this->Size = m_DataArray->getNumberOfTuples() * this->NumberOfComponents;
      this->MaxId = this->Size - 1;
    }
  }

  /**
   * @brief
   * @param name
   */
  void SetName(const char* name) override
  {
    Superclass::SetName(name);
    if(nullptr == m_DataArray)
    {
      throw std::runtime_error("CV::Array::SetName() does not have an underlying complex::DataArray");
    }
    m_DataArray->rename(name);
  }

  /**
   * @brief Get the value at valueIdx.
   *
   * Note: GetTypedComponent is preferred over this method. It is faster for SOA arrays,
   * and shows equivalent performance for AOS arrays when NumberOfComponents is known
   * to the compiler (See vtkAssume.h).
   * @param valueIdx assumes AOS ordering.
   * @return T
   */
  inline ValueType GetValue(vtkIdType valueIdx) const
  {
    if(nullptr == m_DataArray)
    {
      throw std::runtime_error("CV::Array::GetValue() does not have an underlying complex::DataArray");
    }
    return (*m_DataArray)[valueIdx];
  }

  /**
   * @brief Set the value at valueIdx to value.
   *
   * Note: SetTypedComponent is preferred over this method. It is faster for SOA arrays, and shows equivalent performance for
   *  AOS arrays when NumberOfComponents is known to the compiler (See vtkAssume.h).
   * @param valueIdx assumes AOS ordering.
   * @param value
   */
  inline void SetValue(vtkIdType valueIdx, ValueType value)
  {
    if(nullptr == m_DataArray)
    {
      throw std::runtime_error("CV::Array::SetValue() does not have an underlying complex::DataArray");
    }
    (*m_DataArray)[valueIdx] = value;
  }

  /**
   * @brief Copy the tuple at tupleIdx into tuple.
   *
   * Note:GetTypedComponent is preferred over this method. The overhead of copying the tuple is significant compared to the
   *    more performant component-wise access methods, which typically optimize to raw memory access.
   * @param tupleIdx
   * @param tuple
   */
  inline void GetTypedTuple(vtkIdType tupleIdx, ValueType* tuple) const
  {
    if(nullptr == m_DataArray)
    {
      throw std::runtime_error("CV::Array::GetTypedTuple() does not have an underlying complex::DataArray");
    }
    auto dataStore = m_DataArray->getDataStore();
    const size_t numComps = dataStore->getNumberOfComponents();
    const size_t elementIndex = tupleIdx * numComps;
    for(size_t i = 0; i < numComps; i++)
    {
      tuple[i] = (*dataStore)[elementIndex + i];
    }
  }

  /**
   * @brief Set this array's tuple at tupleIdx to the values in tuple.
   *
   * Note: SetTypedComponent is preferred over this method. The overhead of copying
   * the tuple is significant compared to the more performant component-wise access
   * methods, which typically optimize to raw memory access.
   * @param tupleIdx
   * @param tuple
   */
  inline void SetTypedTuple(vtkIdType tupleIdx, const ValueType* tuple)
  {
    if(nullptr == m_DataArray)
    {
      throw std::runtime_error("CV::Array::SetTypedTuple() does not have an underlying complex::DataArray");
    }
    auto dataStore = m_DataArray->getDataStore();
    const size_t numComps = dataStore->getNumberOfComponents();
    const size_t elementIndex = tupleIdx * numComps;
    for(size_t i = 0; i < numComps; i++)
    {
      (*dataStore)[elementIndex + i] = tuple[i];
    }
  }

  /**
   * @brief Get component compIdx of the tuple at tupleIdx. This is typically the fastest way to access array data.
   * @param tupleIdx
   * @param compIdx
   * @return T
   */
  inline ValueType GetTypedComponent(vtkIdType tupleIdx, int compIdx) const
  {
    if(nullptr == m_DataArray)
    {
      throw std::runtime_error("CV::Array::GetTypedComponent() does not have an underlying complex::DataArray");
    }
    const auto elementIndex = tupleIdx * this->NumberOfComponents;
    ValueType value = (*m_DataArray)[elementIndex + compIdx];
    //   std::cout << " * GetTypedComponent() " << tupleIdx << ":" << compIdx  << " = " << value << std::endl;
    return value;
  }

  /**
   * @brief Set component compIdx of the tuple at tupleIdx to value. This is typically the fastest way to set array data.
   * @param tupleIdx
   * @param compIdx
   * @param value
   */
  inline void SetTypedComponent(vtkIdType tupleIdx, int compIdx, ValueType value)
  {
    if(nullptr == m_DataArray)
    {
      throw std::runtime_error("CV::Array::SetTypedComponent() does not have an underlying complex::DataArray");
    }
    const auto elementIndex = tupleIdx * this->NumberOfComponents;
    (*m_DataArray)[elementIndex + compIdx] = value;
  }

  /**
   * @brief Allocates space for a given number of tuples. Old data should *NOT* be preserved.
   * If numTuples == 0, all data is freed.
   * @param numTuples
   * @return bool
   */
  inline bool AllocateTuples(vtkIdType numTuples)
  {
    // If the underlying m_DataArray is null then we are hosed... just return false but VTK is going to die quickly after this.
    if(m_DataArray == nullptr)
    {
      return false;
    }

    // If there is no difference in size, then just return true
    if(m_DataArray->getNumberOfTuples() == numTuples)
    {
      return true;
    }

    // Now swap the vtkDataArrays
    m_DataArray = createNewDataArray(numTuples);

    // Now update the vtkGenericDataArray internal values
    this->NumberOfComponents = m_DataArray->getNumberOfComponents();
    this->Size = m_DataArray->getNumberOfTuples() * this->NumberOfComponents;
    this->MaxId = this->Size - 1;

    return true;
  }

  /**
   * @brief Allocates space for a given number of tuples.  Old data *WILL* be preserved.
   * If numTuples == 0, all data is freed.
   * @param numTuples
   * @return bool
   */
  inline bool ReallocateTuples(vtkIdType numTuples)
  {
    if(m_DataArray == nullptr)
    {
      throw std::runtime_error("CV::Array::ReallocateTuples() does not have an underlying complex::DataArray");
    }

    // Now swap the vtkDataArrays
    ComplexArrayPointerType copyOfDataArrayPtr = createNewDataArray(numTuples);

    // Copy the previous data over to the new underlying array. This could be done faster possibly.
    for(size_t idx = 0; idx < numTuples; idx++)
    {
      (*copyOfDataArrayPtr.get())[idx] = (*m_DataArray)[idx];
    }

    // Now swap the vtkDataArrays
    m_DataArray = copyOfDataArrayPtr;

    // Now update the vtkGenericDataArray internal values
    this->NumberOfComponents = m_DataArray->getNumberOfComponents();
    this->Size = numTuples * this->NumberOfComponents;
    this->MaxId = this->Size - 1;

    return true;
  }

  void* GetVoidPointer(vtkIdType valueIdx) override
  {
    auto dataStore = dynamic_cast<complex::DataStore<T>*>(m_DataArray->getDataStore());
    if(nullptr == dataStore)
    {
      return nullptr;
    }
    return dataStore->data();
  }

protected:
  vtkObjectBase* NewInstanceInternal() const override
  {
    ComplexArrayPointerType copyOfDataArrayPtr = createNewDataArray(0);
    return new Array(copyOfDataArrayPtr);
  }

private:
  ComplexArrayPointerType m_DataArray;

  /**
   * @brief Creates and returns a new DataArray<T> with a new DataStore<T>
   * @param numTuples The number of tuples to create in the DataStore<T>.
   * @return
   */
  ComplexArrayPointerType createNewDataArray(vtkIdType numTuples) const
  {
    // Create a brand-new instance of DataArray<T> with its own underlying DataStore of the same
    // type as the original DataArray<T> instance
    using DataStoreType = complex::DataStore<T>;
    using SharedDataStoreType = std::shared_ptr<DataStoreType>;
    std::shared_ptr<complex::DataStore<T>> dataStore = SharedDataStoreType(new DataStoreType({static_cast<size_t>(numTuples)}, {static_cast<size_t>(this->NumberOfComponents)}));
    // Shallow Copy from the previous DataArray to get an instance of DataArray
    ComplexArrayType* copyOfDataArray = dynamic_cast<ComplexArrayType*>(m_DataArray->shallowCopy());
    ComplexArrayPointerType copyOfDataArrayPtr = std::shared_ptr<ComplexArrayType>(copyOfDataArray);
    // Set the newly created DataStore into the just created DataArray, replacing its DataStore with this new one.
    copyOfDataArrayPtr->setDataStore(dataStore);

    return copyOfDataArrayPtr;
  }
};
} // namespace CV
