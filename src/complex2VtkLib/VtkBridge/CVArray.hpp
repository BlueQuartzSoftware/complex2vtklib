#pragma once

#include <memory>

#include "vtkGenericDataArray.h"

#include "complex/DataStructure/DataArray.hpp"

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
class COMPLEX2VTKLIB_EXPORT Array : public vtkGenericDataArray<Array<T>, T>
{
public:
  using ComplexArrayPtr = std::shared_ptr<complex::DataArray<T>>;
  using ValueType = T;
  using Superclass = typename vtkGenericDataArray<Array<T>, T>;

  vtkTypeMacro(Array, Superclass);

  Array(const ComplexArrayPtr& dataArr)
  : vtkGenericDataArray<Array<T>, T>()
  , m_DataArray(dataArr)
  {
    if(dataArr != nullptr)
    {
      SetName(dataArr->getName().c_str());
    }
  }

  virtual ~Array() = default;

  /**
   * @brief Returns the value for the given ID.
   * @param valueIdx
   * @return T
   */
  inline ValueType GetValue(vtkIdType valueIdx) const
  {
    if(!m_DataArray)
    {
      return {};
    }
    return m_DataArray->at(valueIdx);
  }

  /**
   * @brief Sets the value for the given ID.
   * @param valueIdx
   * @param value
   */
  inline void SetValue(vtkIdType valueIdx, ValueType value)
  {
    if(!m_DataArray)
    {
      return;
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
    if(!m_DataArray)
    {
      tuple = nullptr;
      return;
    }

    auto dataStore = m_DataArray->getDataStore();
    const size_t numComps = dataStore->getNumComponents();
    const size_t tupleIndex = numComps * tupleIdx;
    for(size_t i = 0; i < numComps; i++)
    {
      tuple[i] = dataStore->getValue(tupleIndex + i);
    }
  }

  /**
   * @brief Sets the tuple pointer for the given index.
   * @param tupleIdx
   * @param tuple
   */
  inline void SetTypedTuple(vtkIdType tupleIdx, const ValueType* tuple)
  {
    if(!m_DataArray)
    {
      return;
    }

    auto dataStore = m_DataArray->getDataStore();
    const size_t numComps = dataStore->getNumComponents();
    const size_t tupleIndex = numComps * tupleIdx;
    for(size_t i = 0; i < numComps; i++)
    {
      dataStore->setValue(tupleIndex + i, tuple[i]);
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
    if(!m_DataArray)
    {
      return {};
    }

    const auto numComps = m_DataArray->getNumComponents();
    const auto tuplePos = numComps * tupleIdx;
    return m_DataArray->at(tuplePos + compIdx);
  }

  /**
   * @brief Sets the component specified by tuple and component ID.
   * @param tupleIdx
   * @param compIdx
   * @param value
   */
  inline void SetTypedComponent(vtkIdType tupleIdx, int compIdx, ValueType value)
  {
    if(!m_DataArray)
    {
      return;
    }

    const auto numComps = m_DataArray->getNumComponents();
    const auto tuplePos = numComps * tupleIdx;
    (*m_DataArray)[tuplePos + compIdx] = value;
  }

  /**
   * @brief Allocates space for a given number of tuples. Old data should not
   * be preserved.
   * @param numTuples
   * @return bool
   */
  inline bool AllocateTuples(vtkIdType numTuples)
  {
    if(m_DataArray == nullptr)
    {
      return false;
    }

    m_DataArray->getDataStore()->resizeTuples(numTuples);
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
    if(m_DataArray == nullptr)
    {
      return false;
    }

    m_DataArray->getDataStore()->resizeTuples(numTuples);
    return true;
  }

private:
  ComplexArrayPtr m_DataArray;
};
} // namespace CV
