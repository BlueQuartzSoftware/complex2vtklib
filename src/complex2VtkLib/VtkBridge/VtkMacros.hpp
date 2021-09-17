#pragma once

#include <vtkSmartPointer.h>

#define VTK_NEW(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

#define VTK_PTR(type) vtkSmartPointer<type>
