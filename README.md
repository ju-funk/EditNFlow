#
# CEditNFlow
This is a MFC C++ **CEdit**-Template for **N**umber and **Float**, for the types 
- ULONGLONG
- LONGLONG
- long
- float

*The starting point of the project was [CodeGuru CNumEdit](https://www.codeguru.com/cplusplus/creating-a-numeric-edit-box/)*

# 
# Features
Only need the one header-file EditNFlow.hpp 

 1. Can show no value
 2. one Variable for Control and Value
 3. UpdateDate not needed, Value is always up to date
 4. Context-Menu
    1. Handling for the UI View
    2. Copy/Paste (handling the LOCALE_STHOUSAND/LOCALE_SMONDECIMALSEP)
 5. ToolTips
 6. Number inc/dec with mouse-wheel, cursor up/down, controlling the steps
 7. Set Min/Max value (handling for negative value when min is < 0)
 8. Change the control with cursor left/right
 9. Color Handling
10. Control for change event (EN_CHANGE)
11. Controlling the Precision Length, and the decimal seperator (so you can copy values from/to the calculator)
12. Can Set Mouse-Events with Class-Wizard

#
# Integration
<u>In Header-File:</u>
```.cpp
#include "./EditNFlow.hpp

class myClass
{
    .
    .
    .
    CEditNFlow<long> myLongEdit
    CEditNFlow<float> myFloatEdit
     // here you can switch to not view value, default is ---
    CEditNFlow<LONGLONG , true> myEdit; 
    .
    .

}
```
<u>In Source-File:</u>
```.cpp
void myClass::DoDataExchange(CDataExchange* pDX)
{
    .
    .
    DDX_EditNFlow(pDX, IDC_LONG,  myLongEdit);
    DDX_EditNFlow(pDX, IDC_FLOAT, myFloatEdit);
    DDX_EditNFlow(pDX, IDC_EDIT,  myEdit);
}

Access:
 - new value myLongEdit = 1234 or myEdit.SetVal(4321)
 - read value val = myFloatEdit.GetVal() or vall = (long)myLongEdit
 - set not value myEdit = false
 - get is Value valid if(myEdit) a valid value

```
No **UpdateData** is needed

For more details show into the ***EditNFlow.hpp***, example for using look to ViewSDR (coming soon) 


