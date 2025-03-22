////////////////////////////////////////////////////////////////////////////////////////////////
///// I have derived the basic framework from the CNumEdit class                              //
///// WebPage: https://www.codeguru.com/cplusplus/creating-a-numeric-edit-box/                //
////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once
#include "afxdialogex.h"
#include <type_traits>
#include "pch.h"
#include "ViewSDR.h"


/* 
   T should be
    - long          (not tested)
    - ULONGLONG
    - LONGLONG
    - float

   snoValue = show no Value
    - false -> only show Values
    - true  -> when not set show default '---'

   P send right Mouse Click to this class
*/
template <class T, bool snoValue = false, class P = nullptr_t>
class CEditNFlow : public CEdit
{
public:

    CEditNFlow() 
    {
        myRejectingChange = setReset = false;
        if constexpr (snoValue)
        {
            noValue = ValueSet = true;
            notSet = _T("---");
        }
        else
        {
            noValue = ValueSet = false;
            notSet = _T("");
        }

        Value = 0;
        myLastSel = 0;
        myLastValidValue = notSet;
        Set_MinMax(Min, Max);
    }

    enum ShowState {DoNothing, SetView, ShowNoVal};
    void SetValue(T val, ShowState ShSa = DoNothing)
    {
        Value = val;
        switch (ShSa)
        {
        case DoNothing:
            break;
        case SetView:
            noValue = false;
            break;
        case ShowNoVal:
            Value = 0;
            myLastValidValue = notSet;
            noValue = ValueSet;
            break;
        }

        if (!noValue)
        {
            CString str;

            if constexpr (std::is_same_v<T, float>)
                str = _T("%.3f");
            else if constexpr (std::is_same_v<T, LONGLONG>)
                str = _T("%lli");
            else if constexpr (std::is_same_v<T, ULONGLONG>)
                str = _T("%llu");
            else
                str = _T("%li");    // fallback to long

            myLastValidValue.Format(str, Value);
        }
        SetLast();
    }

    void SetMinMax(T min, T max)
    {
        Min = min;
        Max = max;
    }

    T GetValue(void) { return Value; }

    void DataExChg(CDataExchange* pDx, T& Val)
    {
        bool read = false;
        ShowState ss = DoNothing;
        switch (pDx->m_bSaveAndValidate)
        {
        case 0:
            break;
        case 1:
            read = false;
            break;
        case 2:
            ss = SetView;
            break;
        case 3:
            ss = ShowNoVal;
            break;
        }

        if(read)
            if(noValue)
                Val = 0;
            else
                Val = Value;
        else if(Val != Value || myLastValidValue == notSet)
            SetValue(Val, ss);
    }


protected:
    typename T Value, Min, Max;
    CString myLastValidValue, notSet;
    DWORD myLastSel;
    bool myRejectingChange, noValue, 
         setReset, ValueSet;

private:
    void SetLast(void)
    {
        if(setReset)
            myLastSel = noValue ? 0x10000000 : 0x00010001;
        else
            myLastSel = GetSel();
        setReset = noValue;
        myRejectingChange = true;
        SetWindowText(myLastValidValue);
        myRejectingChange = false;
        SetSel(myLastSel);
    }


    void Set_MinMax(T &Min, T &Max)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            Min = FLT_MIN;
            Max = FLT_MAX;
        }
        else if constexpr (std::is_same_v<T, LONGLONG>)
        {
            Min = MINLONGLONG;
            Max = MAXLONGLONG;
        }
        else if constexpr (std::is_same_v<T, ULONGLONG>)
        {
            Min = 0;
            Max = MAXULONGLONG;
        }
        else
         {
            Min = MINLONG;
            Max = MAXLONG;
        }
    }

protected:
    void OnUpdate()
    {
        if (!myRejectingChange)
        {
            CString aValue;
            GetWindowText(aValue);
            if (aValue.IsEmpty())
            {
                setReset = true;
                SetValue(Value, ShowNoVal);
                return;
            }

            LPTSTR aEndPtr = nullptr;

            errno = 0;

            T value;
            Convert(value, aValue, &aEndPtr);

            if (!(*aEndPtr) && errno != ERANGE)
            {
                if (value < Min)
                    SetValue(Min, SetView);
                else if (value > Max)
                    SetValue(Max, SetView);
                else
                    SetValue(value, SetView);
            }
            else
                SetValue(Value, SetView);
        }
    }

    void Convert(T &val, wchar_t const* _String, wchar_t** _EndPtr)
    {
        if constexpr (std::is_same_v<T, float>)
            val = _tcstof(_String, _EndPtr);
        else if constexpr (std::is_same_v<T, LONGLONG>)
            val = _tcstoi64(_String, _EndPtr, 10);
        else if constexpr (std::is_same_v<T, ULONGLONG>)
            val = _tcstoui64(_String, _EndPtr, 10);
        else
            val = _tstol(_String, _EndPtr, 10);
    }

    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        switch (nChar)
        {
        case VK_BACK:
        case VK_RETURN:
        case 0x0A:               // Shift+Enter (= linefeed)
        case VK_ESCAPE:
        case VK_TAB:
            break;
        case '.':
        case ',':
            nChar = '.';
            if constexpr (!std::is_same_v<T, float>)
                return;
            break;
        case '-':
            if constexpr (std::is_same_v<T, ULONGLONG>) 
                return;

            [[fallthrough]]; // fallthrough is explicit
        default:
            break;
        }

        CEdit::OnChar(nChar, nRepCnt, nFlags);
    }

    afx_msg void OnEnSetfocus()
    {
        if (noValue)
        {
            myLastValidValue = _T("");
            SetLast();
        }
    }

    afx_msg void OnEnKillfocus()
    {
        if (noValue)
            SetValue(Value, ShowNoVal);
    }

    void OnLButtonDblClk(UINT nFlags, CPoint point)
    {
        CEdit::OnLButtonDblClk(nFlags, point);

        if constexpr (!(std::is_same_v<P, nullptr_t>))
            ((P*)GetParent())->ENFLButtonDblClk(this, nFlags, point);
    }

    DECLARE_MESSAGE_MAP()
};


//BEGIN_MESSAGE_MAP((CEditNFlow<T>), CEdit)
PTM_WARNING_DISABLE 
template <class T, bool snoValue = false, class P>
const AFX_MSGMAP* CEditNFlow<T, snoValue, P>::GetMessageMap() const
{
    return GetThisMessageMap();
}
template <class T, bool snoValue = false, class P>
const AFX_MSGMAP* PASCAL CEditNFlow<T, snoValue, P>::GetThisMessageMap()
{
    typedef CEditNFlow<T, snoValue, P> ThisClass;
    typedef CEdit TheBaseClass;
    __pragma(warning(push))
    __pragma(warning(disable: 4640)) /* message maps can only be called by single threaded message pump */
    static const AFX_MSGMAP_ENTRY _messageEntries[] =
    {
        ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
        ON_CONTROL_REFLECT(EN_SETFOCUS, OnEnSetfocus)
        ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
        ON_WM_CHAR()
        ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()





template <class T>
void DDX_EditNFlow(CDataExchange * pDX, int nIDC, CWnd & rControl, T & value)
{
    DDX_Control(pDX, nIDC, rControl);

    ((CEditNFlow<T>*) & rControl)->DataExChg(pDX, value);
}



