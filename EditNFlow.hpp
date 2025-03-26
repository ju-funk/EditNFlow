////////////////////////////////////////////////////////////////////////////////////////////////
///// I have derived the basic framework from the CNumEdit class                              //
///// WebPage: https://www.codeguru.com/cplusplus/creating-a-numeric-edit-box/                //
////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once
#include "afxdialogex.h"
#include <type_traits>
#include "pch.h"
#include "ViewSDR.h"


///////////////////////////////////////////////////////////////////////////////////////////
//  template Class CValidValue
//  Handling tristate of a Value T
//    it should be a valid Value or
//    no Value is set, in this case 
//    the Value is not valid
///////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class CVaildValue
{
public:
    CVaildValue()
    {
        ValueVaild = true;
        Value      = 0;
    }

/* 
    *   Get only the current value back
    *    - return T

    *     no test for vaild value, see next operator
    */
    T GetVal(void) { return Value; }

    /*
    *   Get the state is value vaild
    *    - return
    *        true  : value is valid
    *        false : value is not vaild
    */
    explicit operator bool() const
    {
        return ValueVaild;
    }

    /*
    *   Get the state is not value vaild
    *    - return
    *        false : value is vaild
    *        true  : vaild is not value
    */
    bool operator !() const
    {
        return !ValueVaild;
    }

    /*
    *   Change valid value 
    *    - val
    *        true  : set to no vaild value
    *        false : is forbitten, see next operator
    */
    bool operator =(bool val)
    {
        assert(val == true);
        SetValue(0, !val);
        return val;
    }

    /*
    *   Set Value to vaild value
    *    - val
    *         set vaild value
    */
    T operator =(T val)
    {
        SetValue(val, true);
        return Value;
    }

    /*
    *   CAST Get only the current value back
    *    - return T

    *     no test for vaild value
    */
    explicit operator T()
    {
        return Value;
    }


protected:
    typename T Value;
    bool  ValueVaild;


protected:
    virtual void SetValue(T val, bool ShowState)
    {
        Value = val;
        ValueVaild = ShowState;
        if (!ShowState)
            Value = 0;
    }

};


///////////////////////////////////////////////////////////////////////////////////////////
//  template Class CEditNFlow
// 
//  CEdit class expand to handling with different Value types
//    and tristate of the value see above
///////////////////////////////////////////////////////////////////////////////////////////

/* 
   T should be
    - long          (not tested)
    - ULONGLONG
    - LONGLONG
    - float

   ShNotValue = show not Value
    - false -> show Values
    - true  -> when no vaild value, set show to '---'
*/
template <class T, bool ShNotValue = false>
class CEditNFlow : public CEdit, public CVaildValue<T>
{
public:

    CEditNFlow() : CVaildValue()
    {
        if constexpr (ShNotValue)
        {
            ValueVaild = ValueSet = false;
            notSet = _T("---");
        }
        else
        {
            ValueVaild = ValueSet = true;
            notSet = _T("");
        }

        myRejectingChange = setReset =
        bSendMouseMsgs = false;
        myLastSel = 0;
        MenuRes = MenuSub = 0;
        myLastValidValue = notSet;
        pCurrentCtl = nullptr;
        Set_MinMax(Min, Max);
    }

    /*
    *   Set Min Max values
    */
    void SetMinMax(T min, T max)
    {
        Min = min;
        Max = max;
    }

    /*
    * By Mouse Right Click, can set the own Context-Menu
    * 
    *   it is not indepent with SendMouseMsgs
    * 
    *    - menuRes : int ID for the Resource Menu
    *    - menuSub : int ID for the Submenu of the 
    *                Resource Menu
    */
    void SetContextMenu(int menuRes, int menuSub)
    {
        MenuRes = menuRes;
        MenuSub = menuSub;
    }

    /*
    *   Send the Mouse Message to Parent-Dialog,
    *   Set Variable for the Current CEditNFlow-Object
    *    - On
    *       true  : send Mouse Messages to the Parent Window
    *       false : not Send Mouse Messages to Parent Window
    *    - pcurrEdit
    *       When a Mouse or a Context-Menu-Message send to
    *       Parent Window, this Varible has the Sending 
    *       CEditNFlow-Object, when needed
    */
    template <class TT, bool BT>
    void SendMouseMsgs(bool On = true, CEditNFlow<TT, BT>** pCurrEdit = nullptr)
    {
        bSendMouseMsgs = On;
        pCurrentCtl = reinterpret_cast<CEditNFlow<T, ShNotValue>**>(pCurrEdit);
        SetCurrCtrl();
    }

    /*
    *   The Value is always the current
    *     but update the view
    */
    void DataExChg(CDataExchange* pDx)
    {
        if (!pDx->m_bSaveAndValidate)
            SetLast();
    }

    /*
    *  Using the operator= 
    *     from base class CVaildValue
    */
    using CVaildValue<T>::operator=;

protected:
    typename T Min, Max;
    CString myLastValidValue, notSet;
    DWORD myLastSel;
    bool myRejectingChange, setReset, ValueSet;
    int MenuRes, MenuSub;
    CEditNFlow<T, ShNotValue>** pCurrentCtl;
    bool bSendMouseMsgs;

private:
    CEditNFlow(CEditNFlow const&) = delete;
    CEditNFlow& operator=(CEditNFlow other) = delete;


    virtual void SetValue(T val, bool ShowState) override
    {
        CVaildValue::SetValue(val, ShowState);

        if (!ShowState)
        {
            myLastValidValue = notSet;
            ValueVaild = ValueSet;
        }

        if (ValueVaild)
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


    void SetLast(bool reset = false)
    {
        if (::IsWindow(m_hWnd))
        {
            if (reset)
                myLastValidValue = _T("");
            else
            {
                if (setReset)
                    myLastSel = !ValueVaild ? 0x10000000 : 0x00010001;
                else
                    myLastSel = myLastValidValue == notSet ? 0x10000000 : GetSel();
                setReset = !ValueVaild;
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
                SetValue(Value, false);
                return;
            }

            LPTSTR aEndPtr = nullptr;

            errno = 0;

            T value;
            Convert(value, aValue, &aEndPtr);

            if (!(*aEndPtr) && errno != ERANGE)
            {
                if (value < Min)
                    SetValue(Min, true);
                else if (value > Max)
                    SetValue(Max, true);
                else
                    SetValue(value, true);
            }
            else
                SetValue(Value, true);
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

    bool IsEdit()
    {
        return (GetStyle() & ES_READONLY) == 0 && IsWindowEnabled();
    }

    void SetCurrCtrl(CEditNFlow<T, ShNotValue>* This = nullptr)
    {
        if (pCurrentCtl != nullptr)
            *pCurrentCtl = This;
    }

    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
    {
        if(!IsEdit())
            return;

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
            if (!ValueVaild)
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
        if (!ValueVaild && IsEdit())
            SetLast(true);
    }

    afx_msg void OnEnKillfocus()
    {
        if (!ValueVaild && IsEdit())
            SetValue(Value, false);
    }

    afx_msg void OnRButtonDown(UINT nFlags, CPoint point)
    {
        if (MenuRes > 0)
        {
            CMenu men;
            men.LoadMenuW(MenuRes);
            CMenu* pmen = men.GetSubMenu(MenuSub);

            SetCurrCtrl(this);

            DisableItems(pmen);

            ClientToScreen(&point);
            int Id = pmen->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, GetParent());

            if(Id > 0)
                GetParent()->OnCmdMsg(Id, 0, NULL, NULL);

            SetCurrCtrl();
        }
        else
            CEdit::OnRButtonDown(nFlags, point);
    }

    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)  override
    {
        if (bSendMouseMsgs && (m_hWnd != nullptr) &&
            ((WM_MOUSEFIRST <= message) && (WM_MOUSELAST >= message)) )
        {
            SetCurrCtrl(this);

            GetParent()->SendMessage(message, wParam, lParam);

            SetCurrCtrl();
        }
        return CEdit::OnWndMsg(message, wParam, lParam, pResult);
    }



    void DisableItems(CMenu* pMenu)
    {
        ENSURE_VALID(pMenu);

        // check the enabled state of various menu items

        CCmdUI state;
        state.m_pMenu = pMenu;

        state.m_nIndexMax = pMenu->GetMenuItemCount();
        for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
            state.m_nIndex++)
        {
            state.m_nID = pMenu->GetMenuItemID(state.m_nIndex);
            if (state.m_nID == 0)
                continue; // menu separator or invalid cmd - ignore it

            ASSERT(state.m_pOther == NULL);
            ASSERT(state.m_pMenu != NULL);
            if (state.m_nID == (UINT)-1)
            {
                // possibly a popup menu, route to first item of that popup
                state.m_pSubMenu = pMenu->GetSubMenu(state.m_nIndex);
                if (state.m_pSubMenu == NULL ||
                    (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
                    state.m_nID == (UINT)-1)
                {
                    continue;       // first item of popup can't be routed to
                }
                state.DoUpdate(GetParent(), FALSE);    // popups are never auto disabled
            }
            else
            {
                // normal menu item
                // Auto enable/disable if frame window has 'm_bAutoMenuEnable'
                //    set and command is _not_ a system command.
                state.m_pSubMenu = NULL;
                state.DoUpdate(GetParent(), state.m_nID < 0xF000);
            }
        }
    }

    DECLARE_MESSAGE_MAP()
};


//BEGIN_MESSAGE_MAP((CEditNFlow<T, ShNotValue>), CEdit)
PTM_WARNING_DISABLE 
template <class T, bool ShNotValue>
const AFX_MSGMAP* CEditNFlow<T, ShNotValue>::GetMessageMap() const
{
    return GetThisMessageMap();
}
template <class T, bool ShNotValue>
const AFX_MSGMAP* PASCAL CEditNFlow<T, ShNotValue>::GetThisMessageMap()
{
    typedef CEditNFlow<T, ShNotValue> ThisClass;
    typedef CEdit TheBaseClass;
    __pragma(warning(push))
    __pragma(warning(disable: 4640)) /* message maps can only be called by single threaded message pump */
    static const AFX_MSGMAP_ENTRY _messageEntries[] =
    {
        ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
        ON_CONTROL_REFLECT(EN_SETFOCUS, OnEnSetfocus)
        ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
        ON_WM_KEYDOWN()
        ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



/*
*   Connect the Control to Resource ID
*    - pDX      : from MFC
*    - nIDC     : Resource ID
*    - rControl : Control variable
*/
template <class T, bool ShNotValue>
void DDX_EditNFlow(CDataExchange * pDX, int nIDC, CEditNFlow<T, ShNotValue> & rControl)
{
    DDX_Control(pDX, nIDC, dynamic_cast<CWnd&>(rControl));

    rControl.DataExChg(pDX);
}


