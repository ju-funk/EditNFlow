////////////////////////////////////////////////////////////////////////////////////////////////
///// I have derived the basic framework from the CNumEdit class                              //
///// WebPage: https://www.codeguru.com/cplusplus/creating-a-numeric-edit-box/                //
////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once
#include "afxdialogex.h"
#include <type_traits>
#include "pch.h"
#include "ViewSDR.h"


//// T should be
/////  - long          (not tested)
/////  - ULONGLONG
/////  - LONGLONG
/////  - float
template <class T>
class CEditNFlow : public CEdit
{
public:

    CEditNFlow() 
    {
        myRejectingChange = false; 
        Value = 0;
        myLastSel = 0;
        myLastValidValue = _T("0");
    }

    void SetValue(T val)
    {
        Value = val;
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
        SetLast();
    }

    T GetValue(void) { return Value; }

    void DataExChg(CDataExchange* pDx, T& Val)
    {
        if(pDx->m_bSaveAndValidate)
            Val = Value;
        else
            SetValue(Val);
    }

    void SetLast(void)
    {

        myRejectingChange = true;
        SetWindowText(myLastValidValue);
        myRejectingChange = false;
    }


protected:
    typename T Value;
    CString myLastValidValue;
    UINT myLastSel;
    bool myRejectingChange;

protected:
    void OnUpdate()
    {
        if (!myRejectingChange)
        {
            CString aValue;
            GetWindowText(aValue);
            if (aValue.IsEmpty())
            {
                aValue = _T("!");
                myLastValidValue = _T("0");

            }
            LPTSTR aEndPtr = nullptr;

            errno = 0;

            Convert(Value, aValue, &aEndPtr);

            if (!(*aEndPtr) && errno != ERANGE)
            {
                myLastValidValue = aValue;
            }
            else
            {
                SetLast();
                SetSel(myLastSel);
            }
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
        case '-':
            if (constexpr (std::is_same_v<T, ULONGLONG>) || 
               constexpr (std::is_same_v<T, long>))
                return;

        default:

            myLastSel = GetSel();

            break;

        }

        CEdit::OnChar(nChar, nRepCnt, nFlags);

    }

    DECLARE_MESSAGE_MAP()
};


//BEGIN_MESSAGE_MAP((CEditNFlow<T>), CEdit)
    PTM_WARNING_DISABLE 
        template <class T>
    const AFX_MSGMAP* CEditNFlow<T>::GetMessageMap() const
    {
        return GetThisMessageMap();
    }
    template <class T>
    const AFX_MSGMAP* PASCAL CEditNFlow<T>::GetThisMessageMap()
    {
        typedef CEditNFlow<T> ThisClass;
        typedef CEdit TheBaseClass;
        __pragma(warning(push))
            __pragma(warning(disable: 4640)) /* message maps can only be called by single threaded message pump */
            static const AFX_MSGMAP_ENTRY _messageEntries[] =
        {

            ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
            ON_WM_CHAR()
   END_MESSAGE_MAP()


template <class T>
void DDX_EditNFlow(CDataExchange * pDX, int nIDC, CWnd & rControl, T & value)
{
    DDX_Control(pDX, nIDC, rControl);

    ((CEditNFlow<T>*) & rControl)->DataExChg(pDX, value);
}

// void DDX_EditNFlow(CDataExchange * pDX, int nIDC, CWnd & rControl, LONGLONG & value);