#pragma once
#include "afxdialogex.h"
#include <type_traits>
#include "pch.h"
#include "ViewSDR.h"


template <class T>
class CEditNFlow : public CEdit
{
public:

    CEditNFlow() 
    {
        myRejectingChange = false; 
        Value = 0;
        myLastSel = 0;
        myLastValidValue = _T("");
    }

    void SetValue(T val)
    {
        Value = val;
        CString str(_T("%llu"));

        if constexpr (std::is_same_v<T, float>)
            str = _T("%.3f");

        myLastValidValue.Format(str, Value);
        SetLast();
    }

    T GetValue(void) { return Value; }

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

    void Convert(float &val, wchar_t const* _String, wchar_t** _EndPtr)
    {
        val = _tcstof(_String, _EndPtr);
    }

    void Convert(UINT64 &val, wchar_t const* _String, wchar_t** _EndPtr)
    {
        val = _tcstoui64(_String, _EndPtr, 10);
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
        case '-':
            break;

        default:

            myLastSel = GetSel();

            GetWindowText(myLastValidValue);

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
