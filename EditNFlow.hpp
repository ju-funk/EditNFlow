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
/*
   T should be
    - long          (not tested)
    - ULONGLONG
    - LONGLONG
    - float
*/
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
            ValueVaild = VaildValueSet = false;
            notSet = _T("---");
        }
        else
        {
            ValueVaild = VaildValueSet = true;
            notSet = _T("");
        }

        myRejectingChange = setReset =
        bSendMouseMsgs = false;
        myLastSel = 0;
        MenuRes = MenuSub = 0;
        myLastValidValue = notSet;
        Set_MinMax(Min, Max);
        PrecLen = 2;
        TT_On   = TRUE;

        int  ret = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, &TousSep, sizeof(TousSep));
        ret = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONDECIMALSEP, &FlowSep, sizeof(FlowSep));
        DefFloSep = _T('.');
    }

    /*
    *   Set Min Max values
    *    update the ToolTip text
    */
    void SetMinMax(T min, T max)
    {
        Min = min;
        Max = max;
    }

    /*
    *   Set the Precision Length
    *    default is 2  123.45
    */
    void SetPrecisionLen(int len)
    {
        PrecLen = len;
    }

    /*
    *   Set Tool Tip Text
    *    - Add
    *        true  : own text add before default text
    *        false : only set the own text
    *    - Msg
    *        String own message
    *    - Title
    *        set own Title text (Add is only for Msg)
    * 
    *    when Msg == "" and Title = nullptr
    *        disable tool tip
    */
    void SetToolTipText(bool Add, const CString& Msg, const CString* Title = nullptr)
    {
        if (Title != nullptr)
            sTT_Title = Title->GetString();
        else
            sTT_Title = _T("");

        if (Msg != _T(""))
        {
            sTT_AddMsg = _T("");
            if (Add)
                sTT_AddMsg = Msg;
            else
                sTT_Msg    = Msg;
        }
        else
           sTT_Msg = _T("");

        TT_On = (Msg != _T("") || Title != nullptr);

        if (IsWindow(ToolTip.m_hWnd))
            ToolTip.Activate(TT_On);
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
    *   Test send this control for 
    *    - the Mouse-Message
    *    - or Context Menu for COMMAND/COMMAND_UI
    *     return
    *       - true  : the mouse-message is from
    *                 this control
    *       - false : mouse-message is not
    *                 from this control
    */
    bool IsSendMsg() 
    { 
        return bSendMouseMsgs;
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


    /*
    *   Right Mouse Button Handler
    *    when overwrite you can call
    */
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point)
    {
        if ((nFlags & MK_CONTROL) == MK_CONTROL)
            SetNoValid();
        else
            ShowContextMenu(point);
    }


    /*
    *   Set Vaild value to not Valid
    *    call from Right Mouse Button
    */
    void SetNoValid()
    {
        if (IsEdit() && !VaildValueSet)
        {
            if (ValueVaild)
                SetValue(Value, false);
        }
    }

    /*
    *   Handling for the Context-Menu
    *    call from Right Mouse Button
    *     point    : Client coordinate for show (LEFT-ALIGN)
    *     pOwnMenu : own Sub-Menu pointer
    */
    bool ShowContextMenu(CPoint point, CMenu *pOwnMenu = nullptr)
    {
        CMenu SubMenu, cMenu;
        
        if (pOwnMenu == nullptr)
        {

            if (MenuRes > 0)
            {
                if (cMenu.LoadMenu(MenuRes))
                {
                    if (cMenu.GetSubMenu(MenuSub) != nullptr)
                    {
                        if (SubMenu.Attach(cMenu.GetSubMenu(MenuSub)->m_hMenu))
                        {
                            bSendMouseMsgs = true;

                            Send_UI_Command(&SubMenu);
                        }
                        else
                            return false;
                    }
                    else 
                        return false;
                }
                else
                    return false;
            }
            else
            {
                if (!SubMenu.CreatePopupMenu())
                    return false;

                bSendMouseMsgs = true;
            }
        

            // menue-item: copy
            UINT ix = 0;
            UINT nFlags = MF_BYPOSITION | MF_STRING | (ValueVaild ? 0 : MF_GRAYED | MF_DISABLED);
            SubMenu.InsertMenu(ix++, nFlags, My_Copy, _T("Copy"));

            // menue-item: paste
            nFlags = MF_BYPOSITION | MF_STRING | (IsPaste() ? 0 : MF_GRAYED | MF_DISABLED);
            SubMenu.InsertMenu(ix++, nFlags, My_Paste, _T("Paste"));

            if (!VaildValueSet)
            {
                nFlags = MF_BYPOSITION | MF_STRING | (ValueVaild ? 0 : MF_GRAYED | MF_DISABLED);
                SubMenu.InsertMenu(ix++, nFlags, My_SetNoValid, _T("Set to No Value\tCTL-Right-Mouse"));
            }

            nFlags = MF_BYPOSITION | MF_SEPARATOR;
            SubMenu.InsertMenu(ix++, nFlags);
        }
        else
            if (SubMenu.Attach(pOwnMenu->m_hMenu))
            {
                bSendMouseMsgs = true;
                Send_UI_Command(&SubMenu);
            }
            else
                return false;

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
            SetNoValid();
            break;
        default:
            GetParent()->OnCmdMsg(Id, 0, NULL, NULL);
            break;
        }

        bSendMouseMsgs = false;

        return true;
    }

protected:
    typename T Min, Max;
    int      PrecLen;
    CString myLastValidValue, notSet;
    DWORD myLastSel;
    bool myRejectingChange, setReset, VaildValueSet,
         bSendMouseMsgs;
    int MenuRes, MenuSub;
    bool bSendMouseMsgs;
    TCHAR TousSep, FlowSep, DefFloSep;
    CToolTipCtrl ToolTip;
    BOOL         TT_On;
    CString      sToolTip, sTT_Title, sTT_Msg, sTT_AddMsg;

    enum { My_Copy = 35000, My_Paste, My_SetNoValid };

private:
    CEditNFlow(CEditNFlow const&) = delete;
    CEditNFlow& operator=(CEditNFlow other) = delete;

    CString GetTTS_String(bool Title)
    {
        CString tri, triNa, reol, neg;

        if constexpr (ShNotValue)
        {
            tri.Format(_T("Tristate means: Show vaules, and show '%s' when no value is set\n%s"), notSet,
                !IsEdit() ? _T("") : _T("The State 'no value' can you set, when delete all valid characters or\n"
                    "with CTRL-Right-Mouse-Button, or in the Context-Menu\n"));
            triNa = _T("Tristate ");
        }

        if(Min >= 0)
            neg = _T("");
        else
            neg = _T(" - and");

        reol = IsEdit() ? _T("Edit-Input") : _T("Readonly");

        if constexpr (std::is_same_v<T, float>)
        {
            if (Title)
            {
                CString tit;
                tit.Format((_T("%sFloating %s")), triNa.GetString(), reol.GetString());

                return sTT_Title.IsEmpty() ? tit : sTT_Title;
            }
            else
            {
                CString msg, flow, mima;

                mima.Format(_T("Min: %.*f\nMax: %.*f"), PrecLen, Min, PrecLen, Max);

                if(DefFloSep != FlowSep)
                    flow.Format(_T("%c and %c"), FlowSep, DefFloSep);
                else
                    flow.Format(_T("%c"), FlowSep);

                if(IsEdit())
                    msg.Format(_T("Valid character:%s 0 to 9 and %s\n%s\n%s"), neg.GetString(), flow.GetString(), mima.GetString(), tri.GetString());
                else
                    msg = tri;

                sToolTip = sTT_AddMsg + msg;

                return sTT_Msg.IsEmpty() ? sToolTip : sTT_Msg;
            }
        }
        else if constexpr (std::is_same_v<T, LONGLONG>)
        {
            if (Title)
            {
                CString tit;
                tit.Format((_T("%sLong Long %s")), triNa, reol);

                return sTT_Title.IsEmpty() ? tit : sTT_Title;
            }
            else
            {
                CString msg, mima;

                mima.Format(_T("Min:%lli\nMax: %lli"), Min, Max);

                if (IsEdit())
                    msg.Format(_T("Valid character:%s 0 to 9\n%s\n%s"), neg.GetString(), mima.GetString(), tri.GetString());
                else
                    msg = tri;

                sToolTip = sTT_AddMsg + msg;

                return sTT_Msg.IsEmpty() ? sToolTip : sTT_Msg;
            }
        }
        else if constexpr (std::is_same_v<T, ULONGLONG>)
        {
            if (Title)
            {
                CString tit;
                tit.Format((_T("%sUnsigned Long Long %s")), triNa.GetString(), reol.GetString());

                return sTT_Title.IsEmpty() ? tit : sTT_Title;
            }
            else
            {
                CString msg, mima;

                mima.Format(_T("Min:%llu\nMax: %llu"), Min, Max);

                if (IsEdit())
                    msg.Format(_T("Valid character: 0 to 9\n%s\n%s"), mima.GetString(), tri.GetString());
                else
                    msg = tri;

                sToolTip = sTT_AddMsg + msg;

                return sTT_Msg.IsEmpty() ? sToolTip : sTT_Msg;
            }
        }
        else // long
        {
            if (Title)
            {
                CString tit;
                tit.Format((_T("%sLong %s")), triNa.GetString(), reol.GetString());

                return sTT_Title.IsEmpty() ? tit : sTT_Title;
            }
            else
            {
                CString msg, mima;

                mima.Format(_T("Min:%li\nMax: %li"), Min, Max);

                if (IsEdit())
                    msg.Format(_T("Valid character:%s 0 to 9\n%s\n%s"), neg.GetString(), mima.GetString(), tri.GetString());
                else
                    msg = tri;

                sToolTip = sTT_AddMsg + msg;

                return sTT_Msg.IsEmpty() ? sToolTip : sTT_Msg;
            }
        }

        return CString();
    }


    afx_msg BOOL OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
    {
        TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;

        pTTT->lpszText = (LPTSTR)(LPCTSTR)GetTTS_String(false).GetString();

        ToolTip.SetTipBkColor(RGB(192, 192, 192));
        ToolTip.SetTipTextColor(RGB(192, 20, 20));
        ToolTip.SetTitle(TTI_INFO, GetTTS_String(true));

        return TRUE;
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg) override
    {
        if (::IsWindow(ToolTip.m_hWnd) && (pMsg->hwnd == m_hWnd))
            ToolTip.RelayEvent(pMsg);

        return __super::PreTranslateMessage(pMsg);
    }

    virtual void PreSubclassWindow() override
    {
        __super::PreSubclassWindow();
        if (!IsWindow(ToolTip.m_hWnd) && ToolTip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON))
        {
            TOOLINFO ti;
            memset(&ti, 0, sizeof(TOOLINFO));
            ti.cbSize = sizeof(TOOLINFO);
      
            ti.hwnd = m_hWnd;
            ti.uFlags = TTF_IDISHWND;
            ti.uId = (UINT_PTR)m_hWnd;
            ti.lpszText = LPSTR_TEXTCALLBACK;
      
            if (::SendMessage(ToolTip.m_hWnd, TTM_ADDTOOL, 0, (LPARAM)&ti))
            {
                ToolTip.SetMaxTipWidth(SHRT_MAX);
                ToolTip.SetDelayTime(TTDT_RESHOW, 300);
                ToolTip.SetDelayTime(TTDT_AUTOPOP, 150000);
                ToolTip.SetDelayTime(TTDT_INITIAL, 500);
                ToolTip.Activate(TT_On);
            }
        }
    }


    virtual void SetValue(T val, bool ShowState) override
    {
        CVaildValue::SetValue(val, ShowState);

        if (!ShowState)
        {
            myLastValidValue = notSet;
            ValueVaild = VaildValueSet;
        }

        if (ValueVaild)
        {
            CString str, last(myLastValidValue);

            if constexpr (std::is_same_v<T, float>)
                str.Format(_T("%%.%if"), PrecLen);
            else if constexpr (std::is_same_v<T, LONGLONG>)
                str = _T("%lli");
            else if constexpr (std::is_same_v<T, ULONGLONG>)
                str = _T("%llu");
            else
                str = _T("%li");    // fallback to long

            myLastValidValue.Format(str, Value);
            if constexpr (std::is_same_v<T, float>)
                myLastValidValue.Replace(CString(DefFloSep), CString(FlowSep));
            if(last == myLastValidValue)
                return;
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
            Min = -FLT_MAX;
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
            else if(Min >= 0)
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

    virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)  override
    {
        BOOL bHandler = FALSE;

        if (IsEdit() &&
            ((WM_MOUSEFIRST <= message) && (WM_MOUSELAST >= message)) )
        {
            CWnd *pPar = GetParent();

            // test, exist a Mouse-Handler?
            bHandler = reinterpret_cast<CEditNFlow *>(pPar)->FindParentMouseHnd(message);

            bSendMouseMsgs = true;

            if (bHandler)
                pPar->SendMessage(message, wParam, lParam);

            bSendMouseMsgs = false;
        }

        if(!bHandler)
            bHandler = CEdit::OnWndMsg(message, wParam, lParam, pResult);
        return bHandler;
    }


    void Send_UI_Command(CMenu* pMenu)
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

    bool FindParentMouseHnd(UINT nMsg)
    {
        const AFX_MSGMAP_ENTRY *Entry;

        Entry = GetMessageMap()->lpEntries;
        while (Entry->nSig != AfxSig_end)
        {
            if (Entry->nMessage == nMsg)
                return true;

            Entry++;
        }

        return false;
    }

    DECLARE_MESSAGE_MAP()
};


//BEGIN_TEMPLATE_MESSAGE_MAP(CEditNFlow, T, CEdit)
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
    __pragma(warning(disable: 4640)) // message maps can only be called by single threaded message pump 
    static const AFX_MSGMAP_ENTRY _messageEntries[] =
    {
        ON_CONTROL_REFLECT(EN_UPDATE, OnUpdate)
        ON_CONTROL_REFLECT(EN_SETFOCUS, OnEnSetfocus)
        ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
        ON_WM_KEYDOWN()
        ON_WM_RBUTTONDOWN()
        ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNeedText)
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


