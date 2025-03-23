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

    CEditNFlow(CEditNFlow const&) = delete;
    CEditNFlow& operator=(CEditNFlow other)  = delete;


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

    T GetValue(void) { return Value;}

    explicit operator bool() const
    {
         return !noValue;
    }

    bool operator !() const
    {
         return noValue;
    }

    bool operator =(bool val)
    {
        assert(val == false);
        SetValue(0, ShowNoVal);
        return val;
    }

    T operator =(T val) 
    {
        SetValue(val, SetView);
        return Value;
    }


    void DataExChg(CDataExchange* pDx)
    {
        if (!pDx->m_bSaveAndValidate)
            SetLast();
    }


protected:
    typename T Value, Min, Max;
    CString myLastValidValue, notSet;
    DWORD myLastSel;
    bool myRejectingChange, noValue, 
         setReset, ValueSet;

private:
    void SetLast(bool reset = false)
    {
        if (::IsWindow(m_hWnd))
        {
            if (reset)
                myLastValidValue = _T("");
            else
            {
                if (setReset)
                    myLastSel = noValue ? 0x10000000 : 0x00010001;
                else
                    myLastSel = myLastValidValue == notSet ? 0x10000000 : GetSel();
                setReset = noValue;
            }
            myRejectingChange = true;
            SetWindowText(myLastValidValue);
            myRejectingChange = false;
            if (!reset)
                SetSel(myLastSel);
        }
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
            val = wcstoul(_String, _EndPtr, 10);
    }

    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        switch (nChar)
        {
        case VK_BACK:
        case VK_DELETE:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_END:
        case VK_HOME:
            if (noValue)
            {
                SetLast(true);
                return;
            }
            break;
            
        case VK_RETURN:
        case 0x0A:               // Shift+Enter (= linefeed)
        case VK_ESCAPE:
        case VK_TAB:
            break;
        case VK_OEM_PERIOD:
        case VK_OEM_COMMA:
            nChar = VK_OEM_PERIOD;
            if constexpr (!std::is_same_v<T, float>)
                return;
            break;
        case VK_OEM_MINUS:
            if constexpr (std::is_same_v<T, ULONGLONG>) 
                return;

            [[fallthrough]]; // fallthrough is explicit
        default:
            break;
        }

        CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
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
template <class T, bool snoValue, class P>
const AFX_MSGMAP* CEditNFlow<T, snoValue, P>::GetMessageMap() const
{
    return GetThisMessageMap();
}
template <class T, bool snoValue, class P>
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
        ON_WM_KEYDOWN()
        ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()



template <class T, bool snoValue, class P>
void DDX_EditNFlow(CDataExchange * pDX, int nIDC, CEditNFlow<T, snoValue, P> & rControl)
{
    DDX_Control(pDX, nIDC, static_cast<CWnd&>(rControl));

    rControl.DataExChg(pDX);
}


