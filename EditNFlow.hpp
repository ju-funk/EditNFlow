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
            ValueVaild = NoVaildValueSet = false;
            notSet = _T("---");
        }
        else
        {
            ValueVaild = NoVaildValueSet = true;
            notSet = _T("");
        }

        myRejectingChange = setReset =
        bSendMouseMsgs = false;
        myLastSel = 0;
        MenuRes = MenuSub = 0;
        myLastValidValue = notSet;
        pCurrentCtl = nullptr;
        Set_MinMax(Min, Max);

        int  ret = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, &TousSep, sizeof(TousSep));
        ret = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONDECIMALSEP, &FlowSep, sizeof(FlowSep));
        DefFloSep = _T('.');
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
    *               but not send the Right-Button-Down 
    *       false : not Send Mouse Messages to Parent Window
    *               but always send the Right-Button-Double
    *    - pcurrEdit
    *       When a Mouse or a Context-Menu-Message send to
    *       Parent Window, this Varible has the Sending 
    *       CEditNFlow-Object, when needed
    * 
    *       the tip with own template came from perplexity.ai
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
    * 
    *   the tip came from perplexity.ai
    */
    using CVaildValue<T>::operator=;

protected:
    typename T Min, Max;
    CString myLastValidValue, notSet;
    DWORD myLastSel;
    bool myRejectingChange, setReset, NoVaildValueSet;
    int MenuRes, MenuSub;
    CEditNFlow<T, ShNotValue>** pCurrentCtl;
    bool bSendMouseMsgs;
    TCHAR TousSep, FlowSep, DefFloSep;
    enum { My_Copy = 35000, My_Paste, My_SetNoValid };

private:
    CEditNFlow(CEditNFlow const&) = delete;
    CEditNFlow& operator=(CEditNFlow other) = delete;


    virtual void SetValue(T val, bool ShowState) override
    {
        CVaildValue::SetValue(val, ShowState);

        if (!ShowState)
        {
            myLastValidValue = notSet;
            ValueVaild = NoVaildValueSet;
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
            if constexpr (std::is_same_v<T, float>)
                myLastValidValue.Replace(CString(DefFloSep), CString(FlowSep));
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

    CString GetClipBoardStr()
    {
        CString str(_T(""));
        COleDataObject cSource;

        auto convert = [&](CLIPFORMAT cf) -> void
        {
            HGLOBAL hData = cSource.GetGlobalData(cf);
            if (hData == nullptr)
                return;

            if (cf == CF_TEXT)
            {
                char *pData = (char *) ::GlobalLock(hData);
                str = pData;
            }
            else
            {
                wchar_t *pData = (wchar_t *) ::GlobalLock(hData);
                str = pData;
            }

            ::GlobalUnlock(hData);
        };

        if (!cSource.AttachClipboard())
            return str;

        if (cSource.IsDataAvailable(CF_UNICODETEXT))
            convert(CF_UNICODETEXT);
        else if(cSource.IsDataAvailable(CF_TEXT))
            convert(CF_TEXT);

        return str;
    }

    bool TestPrepareClipStr(T& val)
    {
        val = 0;
        CString str = GetClipBoardStr();

        if constexpr (std::is_same_v<T, float>)
        {
            CString str1(str);
            if(str1.Remove(TousSep) > 1 || str.Find(FlowSep) > -1)
                str.Remove(TousSep);

            str.Replace(CString(FlowSep), CString(DefFloSep));
        }
        else
        {
            str.Remove(TousSep);

            int id = str.Find(FlowSep);
            if(id > 0)
                str.Delete(id, str.GetLength());
        }

        return Convert(val, str);
    }

    bool IsPaste(void)
    {
        COleDataObject cSource;
        cSource.AttachClipboard();

        bool ret = cSource.IsDataAvailable(CF_TEXT) || cSource.IsDataAvailable(CF_UNICODETEXT);

        cSource.Release();

        T val;
        return ret && IsEdit() && TestPrepareClipStr(val);
    }

    void EditCopy(void)
    {
        if (ValueVaild)
        {
            CString cStr(myLastValidValue);
            int stch, ench, len;
            COleDataSource* pSource = new COleDataSource;

            if constexpr (std::is_same_v<T, float>)
                cStr.Replace(CString(FlowSep), CString(DefFloSep));

            GetSel(stch, ench);
            len = ench - stch;
            if (len > 0)
                cStr = myLastValidValue.Mid(stch, len);
            else
                len = cStr.GetLength();
            ++len;

            HGLOBAL	hMem = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, len * sizeof(TCHAR));
            if (hMem != nullptr)
            {
                TCHAR* pchData = (TCHAR*)GlobalLock(hMem);
                if (pchData != nullptr)
                {
                    CString::CopyChars(pchData, len, cStr.GetBuffer(), len - 1);
                    ::GlobalUnlock(hMem);

                    pSource->CacheGlobalData(sizeof(TCHAR) == 2 ? CF_UNICODETEXT : CF_TEXT, hMem);
                    pSource->SetClipboard();
                }
                else
                {
                    ::GlobalUnlock(hMem);
                    delete pSource;
                }
            }
            else
                delete pSource;
        }
    }

    void EditPaste(void)
    {
        T val;

        if (IsEdit() && TestPrepareClipStr(val))
            SetValue(val, true);
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


            T value;
            if (Convert(value, aValue))
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

    bool Convert(T &val, CString &_String)
    {
        LPTSTR _EndPtr = nullptr;
        errno = 0;

        if constexpr (std::is_same_v<T, float>)
        {
            _String.Replace(CString(FlowSep), CString(DefFloSep));
            val = _tcstof(_String, &_EndPtr);
        }
        else if constexpr (std::is_same_v<T, LONGLONG>)
            val = _tcstoi64(_String, &_EndPtr, 10);
        else if constexpr (std::is_same_v<T, ULONGLONG>)
            val = _tcstoui64(_String, &_EndPtr, 10);
        else
            val = wcstoul(_String, &_EndPtr, 10);

        return (!(*_EndPtr) && errno != ERANGE);
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
        else if(ValueVaild)
            SetSel(0);
    }

    afx_msg void OnRButtonDown(UINT nFlags, CPoint point)
    {
        if((nFlags & MK_CONTROL) == MK_CONTROL)
            SetNoValid(nFlags, point);
        else
            ShowContextMenu(point);
    }

    void SetNoValid(UINT nFlags, CPoint point)
    {
        KillTimer(1);
        
        if (IsEdit() && !NoVaildValueSet)
        {
            if (ValueVaild)
                SetValue(Value, false);

            SetCurrCtrl(this);

            GetParent()->SendMessage(WM_RBUTTONDBLCLK, nFlags, MAKELPARAM(point.x, point.y));

            SetCurrCtrl();
        }
    }

    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)  override
    {
        if (bSendMouseMsgs && (m_hWnd != nullptr) &&
            ((WM_MOUSEFIRST <= message) && (WM_MOUSELAST >= message)) )
        {
            SetCurrCtrl(this);

            if(message != WM_RBUTTONDBLCLK && message != WM_RBUTTONDOWN)
                GetParent()->SendMessage(message, wParam, lParam);

            SetCurrCtrl();
        }
        return CEdit::OnWndMsg(message, wParam, lParam, pResult);
    }


    void ShowContextMenu(CPoint point)
    {
        CMenu SubMenu, cMenu;

        if (MenuRes > 0)
        {
            cMenu.LoadMenu(MenuRes);
            SubMenu.Attach(cMenu.GetSubMenu(MenuSub)->m_hMenu);
       
            SetCurrCtrl(this);
       
            DisableItems(&SubMenu);
        }
        else
        {
            if (!SubMenu.CreatePopupMenu())
                return;
        }


        // menue-item: copy
        UINT ix = 0; 
        UINT nFlags = MF_BYPOSITION | MF_STRING | (ValueVaild ? 0 : MF_GRAYED | MF_DISABLED);
        SubMenu.InsertMenu(ix++, nFlags, My_Copy, _T("Copy"));
     
        // menue-item: paste
        nFlags = MF_BYPOSITION | MF_STRING |( IsPaste() ? 0 : MF_GRAYED | MF_DISABLED);
        SubMenu.InsertMenu(ix++, nFlags, My_Paste, _T("Paste"));
     
        if (!NoVaildValueSet)
        {
            nFlags = MF_BYPOSITION | MF_STRING | (ValueVaild ? 0 : MF_GRAYED | MF_DISABLED);
            SubMenu.InsertMenu(ix++, nFlags, My_SetNoValid, _T("Set to No Value\tCTL-Right-Mouse"));
        }

        nFlags = MF_BYPOSITION | MF_SEPARATOR;
        SubMenu.InsertMenu(ix++, nFlags);


        ClientToScreen(&point);
        int Id = SubMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, GetParent());
        switch (Id)
        {
        case 0:
            break;
        case My_Copy:
            EditCopy();
            break;
        case My_Paste:
            EditPaste();
            break;
        case My_SetNoValid:
            SetNoValid(0, CPoint());
            break;
        default:
            GetParent()->OnCmdMsg(Id, 0, NULL, NULL);
            break;
        }

        SetCurrCtrl();
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


