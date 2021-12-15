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
  using ComplexArrayPtr = std::shared_ptr<complex::DataArray<T>>;
  using ValueType = T;
  using Superclass2 = vtkGenericDataArray<CV::Array<T>, T>;

  vtkAbstractTypeMacro(CV::Array<T>, Superclass2);

  static inline const std::string MissingArrayName = "[Missing Array]";

  static CV::Array<T>* New()
  {
    return new CV::Array<T>();
  }

  Array()
  : Superclass()
  , m_DataArray(nullptr)
  {
    SetComplexArray(nullptr);
  }

  Array(const ComplexArrayPtr& dataArr)
  : Superclass()
  , m_DataArray(dataArr)
  {
    SetComplexArray(dataArr);
  }

  virtual ~Array() = default;

  void SetComplexArray(const ComplexArrayPtr& dataArray)
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
   * @brief Returns the value for the given ID.
   * @param valueIdx
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
   * @brief Sets the value for the given ID.
   * @param valueIdx
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
   * @brief Copies the tuple pointer for the given index.
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
      tuple[i] = dataStore->getValue(elementIndex + i);
    }
  }

  /**
   * @brief Sets the tuple pointer for the given index.
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
      dataStore->setValue(elementIndex + i, tuple[i]);
    }
  }

  /**
   * @brief Returns the component specified by tuple and component ID.
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
    return value;
  }

  /**
   * @brief Sets the component specified by tuple and component ID.
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
   * @brief Allocates space for a given number of tuples. Old data should not
   * be preserved.
   * @param numTuples
   * @return bool
   */
  inline bool AllocateTuples(vtkIdType numTuples)
  {
    throw std::runtime_error("CV::Array::AllocateTuples() does not have an underlying complex::DataArray");

    if(m_DataArray == nullptr)
    {
      return false;
    }

    //m_DataArray->getDataStore()->reshapeTuples({static_cast<size_t>(numTuples)});
    return true;
  }

  /**
   * @brief Allocates space for a given number of tuples.  Old data should be
   * preserved.
   * @param numTuples
   * @return bool
   */
  inline bool ReallocateTuples(vtkIdType numTuples)
  {
    //throw std::runtime_error("CV::Array::ReallocateTuples() does not have an underlying complex::DataArray");

    if(m_DataArray == nullptr)
    {
      return false;
    }

    this->NumberOfComponents = m_DataArray->getNumberOfComponents();
    this->Size =numTuples * this->NumberOfComponents;
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
    return new Array(m_DataArray);
  }

private:
  ComplexArrayPtr m_DataArray;
};
} // namespace CV
