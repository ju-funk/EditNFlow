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
    - long
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
    T GetVal(void) const 
    { 
        return Value;
    }

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
    *   Change to not valid value 
    *    - val
    *        true  : set to no vaild value
    *        false : set to no vaild value 
    *    change to valid, see next operator
    */
    bool operator =(bool val)
    {
        SetValue(0, false);
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
    explicit operator T() const
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
            notSet.Empty();
        }

        myRejectingChange = 
        bMouseMsgsAct     = false;
        bSend_ENChange    = true;
        myLastSel = 0;
        MenuRes = MenuSub = 0;
        myLastValidValue = notSet;
        Set_MinMax(Min, Max);
        PrecLen    = 2;
        TT_On      = TRUE;
        TT_Short   = FALSE;
        showMinMax = false;
        IncStp     = 1; 
        IncStpSh   = 10;
        IncStpCt   = 100;
        IncStpShCt = 1000;
        clText     = ::GetSysColor(COLOR_WINDOW);
        clBack     = ::GetSysColor(COLOR_BACKGROUND);
        clTextRO   = ::GetSysColor(COLOR_WINDOWTEXT);
        clBackRO   = ::GetSysColor(COLOR_GRAYTEXT) + RGB(41, 41, 41);
        clTextNV   = ::GetSysColor(COLOR_WINDOW);
        clBackNV   = ::GetSysColor(COLOR_GRAYTEXT);
        bTrans     = bTransNV = bTransRO = false;

        int  ret = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, &TousSep, sizeof(TousSep));
        ret = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONDECIMALSEP, &FlowSep, sizeof(FlowSep));
        DefFloSep = _T('.');
    }


    /*
    *   Set new Value in depend of
    *    - Event_ENChange
    *        true  : When the value is new or, ValidVal = false
    *                the EN_CHANGE-Event send to Parent Dialog
    *        false : When value is new or ValidVal = false 
    *                the EN_CHANGE-Event is not send to Parent
    *                for this value, it is only for this call
    *    - ValidVal
    *        true  : It display the val
    *        false : Show the not vaild Value (---)
    *       when the 'not vaild Vaule' is not set this paramter
    *       ignore
    */
    void SetVal(T val, bool Event_ENChange = true, bool ValidVal = true)
    {
        bSend_ENChange = Event_ENChange;
        if(!ValidVal)
            SetNoValid();
        else
            SetValue(val, true);

        bSend_ENChange = true;
    }

    /*
    *   Set Min Max values
    *    only show ToolTip when set
    */
    void SetMinMax(T min, T max)
    {
        Min = min;
        Max = max;
        showMinMax = true;
    }

    /*
    *   Set the Precision Length
    *    default is 2 e.g. 123.45
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
    *        set own Title text
    *        when Add=false and Title=nullptr then no title
    *         but it can switch over context menu
    * 
    *    when Msg == "" and Title = nullptr
    *        disable tool tip
    */
    void SetToolTipText(bool Add, const CString& Msg, const CString* Title = nullptr)
    {
        if (Title != nullptr)
            sTT_Title = Title->GetString();
        else
        {
            sTT_Title.Empty();
            TT_Short = !Add;
        }

        if (!Msg.IsEmpty())
        {
            sTT_AddMsg.Empty();
            if (Add)
                sTT_AddMsg = Msg;
            else
                sTT_Msg    = Msg;
        }
        else
           sTT_Msg.Empty();

        TT_On = (!Msg.IsEmpty() || Title != nullptr);

        if (IsWindow(ToolTip.m_hWnd))
            ToolTip.Activate(TT_On);
    }

    /*
    * By Mouse Right Click, can set the own Context-Menu
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
    *   Set the Increment/´Decremet steps
    *
    *    - incStp     = Step when no other key is press
    *    - incStpSh   = Step when Shift key is press
    *    - incStpCt   = Step when Control key is press
    *    - incStpSHCt = Step when Shift+Control key ist press
    */
    void SetIncDecSteps(T incStp, T incStpSh, T incStpCt, T incStpShCt)
    {
        IncStp     = incStp;
        IncStpSh   = incStpSh;
        IncStpCt   = incStpCt;
        IncStpShCt = incStpShCt;
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
        return bMouseMsgsAct;
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
    *   Set the Colors
    * 
    *    - Id    = which color would be change
    *              Cl_Normal, Cl_ReadOnly, Cl_NoVaild
    *    - Txt   = change the text color or set
    *              CL_NoChange for not change
    *    - Bkg   = change the background color or set
    *              CL_NoChange for not change
    *    - trans = when true it is transparent
    */
    enum CL_Indent {Cl_Normal, Cl_ReadOnly, Cl_NoVaild, CL_NoChange = (COLORREF)-1 };
    void SetColors(CL_Indent Id,  COLORREF Txt, COLORREF Bkg, bool trans = false)
    {
        auto setcol = [](COLORREF& scol, COLORREF col) -> void
        {
            scol = col == CL_NoChange ? scol : col;
        };

        switch (Id)
        {
        case Cl_Normal:
            setcol(clText, Txt);
            setcol(clBack, Bkg);
            bTrans = trans;
            break;
        case Cl_ReadOnly:
            setcol(clTextRO, Txt);
            setcol(clBackRO, Bkg);
            bTransRO  = trans;
            break;
        case Cl_NoVaild:
            setcol(clTextNV, Txt);
            setcol(clBackNV, Bkg);
            bTransNV  = trans;
            break;
        }
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
    *    - show context menu
    *    - when Press Control-Key set to 
    *      'no valid value' when active
    */
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point)
    {
        if ((nFlags & MK_CONTROL) == MK_CONTROL)
        {
            if(IsEdit())
                SetNoValid();
        }
        else
            ShowContextMenu(point);
    }

    /*
    *   Mouse Wheel Handler
    *    - Increment/Decrement Value
    *    - combine with Shift and/or Control 
    *    see to SetIncDecSteps
    */
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        MSG Msg;
        memset(&Msg, 0, sizeof(Msg));

        Msg.message = WM_LBUTTONDOWN;
        Msg.hwnd    = m_hWnd;

        if (IsEdit())
        {
            if (::IsWindow(ToolTip.m_hWnd))
                ToolTip.RelayEvent(&Msg);

            IncDecrement(zDelta > 0);
        }

        return __super::OnMouseWheel(nFlags, zDelta, pt);
    }

    /*
    *   Set Vaild value to not Valid
    *    call from Right Mouse Button
    */
    void SetNoValid()
    {
        if (IsEdit(false) && !VaildValueSet)
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
        CMenu SubMenu, cMenu, SubTool;
        
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
                            bMouseMsgsAct = true;

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

                bMouseMsgsAct = true;
            }
        
            auto getNFlag = [](bool state, UINT ntrue = 0, UINT nfalse = MF_GRAYED | MF_DISABLED) -> UINT
            {
                UINT fl = MF_BYPOSITION;
                if(ntrue == 0)
                    fl |= MF_STRING;
                if(state)
                    fl |= ntrue;
                else
                    fl |= nfalse;

                return fl;
            };


            // menue-item: copy
            UINT ix = 0;
            SubMenu.InsertMenu(ix++, getNFlag(ValueVaild), My_Copy, _T("Copy"));

            // menue-item: paste
            SubMenu.InsertMenu(ix++, getNFlag(IsPaste()), My_Paste, _T("Paste"));

            if (!VaildValueSet && IsEdit())
                SubMenu.InsertMenu(ix++, getNFlag(ValueVaild), My_SetNoValid, _T("Set to No Value\tCTL-Right-Mouse"));

            if (SubTool.CreatePopupMenu())
            {
                SubTool.InsertMenu(0, MF_BYPOSITION | MF_STRING, My_TT_Toggle, _T("Tooltip"));
                SubTool.CheckMenuItem(0, getNFlag(TT_On, MF_CHECKED, MF_UNCHECKED));
                SubTool.InsertMenu(1, MF_BYPOSITION | MF_STRING, My_TT_Short, _T("Short Tooltip"));
                SubTool.CheckMenuItem(1, getNFlag(TT_Short, MF_CHECKED, MF_UNCHECKED));

                SubMenu.InsertMenu(ix++, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)SubTool.m_hMenu, _T("ToolTip"));
            }

            SubMenu.InsertMenu(ix++, MF_BYPOSITION | MF_SEPARATOR);
        }
        else
            if (SubMenu.Attach(pOwnMenu->m_hMenu))
            {
                bMouseMsgsAct = true;
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
        case My_TT_Short:
            TT_Short = !TT_Short;
            break;
        case My_TT_Toggle:
            TT_On = !TT_On;
            ToolTip.Activate(TT_On);
            break;
        default:
            GetParent()->OnCmdMsg(Id, 0, NULL, NULL);
            break;
        }

        bMouseMsgsAct = false;

        return true;
    }

protected:
    typename T Min, Max, IncStp, IncStpSh, IncStpCt, IncStpShCt;
    int      PrecLen;
    CString myLastValidValue, notSet;
    DWORD myLastSel;
    bool myRejectingChange, VaildValueSet, showMinMax,
         bMouseMsgsAct, bSend_ENChange, 
         bTrans, bTransNV, bTransRO;
    int MenuRes, MenuSub;
    TCHAR TousSep, FlowSep, DefFloSep;
    CToolTipCtrl ToolTip;
    BOOL         TT_On, TT_Short;
    CString      sToolTip, sTT_Title, sTT_Msg, sTT_AddMsg;
    COLORREF     clText, clBack, clTextRO, clBackRO,
                 clTextNV, clBackNV;
    CBrush       retBrush;


    enum { My_Copy = 35000, My_Paste, My_SetNoValid, My_TT_Short, My_TT_Toggle };

private:
    CEditNFlow(CEditNFlow const&) = delete;
    CEditNFlow& operator=(CEditNFlow) = delete;

    CString& GetTTS_String(bool Title)
    {
        CString tri, triNa, reol, neg, whee;
        sToolTip.Empty();

        if constexpr (ShNotValue)
        {
            tri.Format(_T("Tristate means: Show vaules, and show '%s' when no value is set\n%s"), notSet,
                !IsEdit() ? _T("") : _T("The State 'no value' can you set, when delete all valid characters or\n"
                    "with CTRL-Right-Mouse-Button, or in the Context-Menu\n"));
            triNa = _T("Tristate ");
        }

        if(Min >= 0)
            neg.Empty();
        else
            neg = _T(" - and");

        reol = IsEdit() ? _T("Edit-Input") : _T("Readonly");
        if (!TT_Short)
            whee = _T(" (Inc- or Decement with Mouse Wheel or\n \
                       Cursor-Up/Down, different steps with shift and/or control)");

        if constexpr (std::is_same_v<T, float>)
        {
            if (Title)
            {
                if (TT_Short)
                {
                    sToolTip.Empty();
                    return sToolTip;
                }

                sToolTip.Format((_T("%sFloating %s")), triNa.GetString(), reol.GetString());

                return sTT_Title.IsEmpty() ? sToolTip : sTT_Title;
            }
            else
            {
                CString msg, flow, mima;

                if(showMinMax && IsEdit())
                    mima.Format(_T("Min: %.*f\nMax: %.*f"), PrecLen, Min, PrecLen, Max);

                if(DefFloSep != FlowSep)
                    flow.Format(_T("%c or %c"), FlowSep, DefFloSep);
                else
                    flow.Format(_T("%c"), FlowSep);

                if (!TT_Short)
                {
                    if (IsEdit())
                        msg.Format(_T("Valid character:%s 0 to 9 and %s%s\n%s\n%s"), neg.GetString(), flow.GetString(), whee.GetString(), mima.GetString(), tri.GetString());
                    else
                        msg = tri;
                }
                else
                    msg = mima;

                sToolTip = sTT_AddMsg + msg;

                return sTT_Msg.IsEmpty() ? sToolTip : sTT_Msg;
            }
        }
        else if constexpr (std::is_same_v<T, LONGLONG>)
        {
            if (Title)
            {
                if (TT_Short)
                {
                    sToolTip.Empty();
                    return sToolTip;
                }

                sToolTip.Format((_T("%sLong Long %s")), triNa.GetString(), reol.GetString());

                return sTT_Title.IsEmpty() ? sToolTip : sTT_Title;
            }
            else
            {
                CString msg, mima;

                if (showMinMax && IsEdit())
                    mima.Format(_T("Min:%lli\nMax: %lli"), Min, Max);

                if (!TT_Short)
                {
                    if (IsEdit())
                        msg.Format(_T("Valid character:%s 0 to 9%s\n%s\n%s"), neg.GetString(), whee.GetString(), mima.GetString(), tri.GetString());
                    else
                        msg = tri;
                }
                else
                    msg = mima;

                sToolTip = sTT_AddMsg + msg;

                return sTT_Msg.IsEmpty() ? sToolTip : sTT_Msg;
            }
        }
        else if constexpr (std::is_same_v<T, ULONGLONG>)
        {
            if (Title)
            {
                if (TT_Short)
                {
                    sToolTip.Empty();
                    return sToolTip;
                }

                sToolTip.Format((_T("%sUnsigned Long Long %s")), triNa.GetString(), reol.GetString());

                return sTT_Title.IsEmpty() ? sToolTip : sTT_Title;
            }
            else
            {
                CString msg, mima;
                
                if (showMinMax && IsEdit())
                    mima.Format(_T("Min:%llu\nMax: %llu"), Min, Max);

                if (!TT_Short)
                {
                    if (IsEdit())
                        msg.Format(_T("Valid character: 0 to 9%s\n%s\n\n%s"), whee.GetString(), mima.GetString(), tri.GetString());
                    else
                        msg = tri;
                }
                else
                    msg = mima;

                sToolTip = sTT_AddMsg + msg;

                return sTT_Msg.IsEmpty() ? sToolTip : sTT_Msg;
            }
        }
        else // long
        {
            if (Title)
            {
                if (TT_Short)
                {
                    sToolTip.Empty();
                    return sToolTip;
                }

                sToolTip.Format((_T("%sLong %s")), triNa.GetString(), reol.GetString());

                return sTT_Title.IsEmpty() ? sToolTip : sTT_Title;
            }
            else
            {
                CString msg, mima;

                if (showMinMax && IsEdit())
                    mima.Format(_T("Min:%li\nMax: %li"), Min, Max);

                if (!TT_Short)
                {
                    if (IsEdit())
                        msg.Format(_T("Valid character:%s 0 to 9%s\n%s\n%s"), neg.GetString(), whee.GetString(), mima.GetString(), tri.GetString());
                    else
                        msg = tri;
                }
                else
                    msg = mima;

                sToolTip = sTT_AddMsg + msg;

                return sTT_Msg.IsEmpty() ? sToolTip : sTT_Msg;
            }
        }

        return sToolTip;
    }


    afx_msg BOOL OnToolTipNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
    {
        LPNMTTDISPINFO pTTT = (LPNMTTDISPINFO)pNMHDR;

        ToolTip.SetTitle(TTI_INFO, GetTTS_String(true));
        pTTT->lpszText = const_cast<LPTSTR>(GetTTS_String(false).GetString());

        ToolTip.SetTipBkColor(RGB(192, 192, 192));
        ToolTip.SetTipTextColor(RGB(192, 20, 20));

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
        if (!IsWindow(ToolTip.m_hWnd) && ToolTip.Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_USEVISUALSTYLE))
        {
            TOOLINFO ti;
            memset(&ti, 0, sizeof(TOOLINFO));
            ti.cbSize = sizeof(TOOLINFO);
      
            ti.hwnd = m_hWnd; // in ToolTip.AddTool(this), set here the Parent window
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
        if (!ShowState)
        {
            myLastValidValue = notSet;
            ValueVaild = VaildValueSet;
        }
        else
            ValueVaild = true;

        if (ValueVaild)
        {
            CString str, last(myLastValidValue);

            if (val < Min)
                val = Min;
            else if (val > Max)
                val = Max;
        
            Value = val;

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
            if (last == myLastValidValue)
            {
                bSend_ENChange = true;
                return;
            }
        }
        else
            __super::SetValue(val, ValueVaild);

        SetLast();
    }


    void SetLast(bool reset = false)
    {
        if (::IsWindow(m_hWnd))
        {
            if (reset)
            {
                myLastValidValue.Empty();
                myLastSel        = 0x00010001;
            }
            else
                myLastSel = myLastValidValue == notSet ? 0x10000000 : GetSel();

            myRejectingChange = true;
            SetWindowText(myLastValidValue);
            myRejectingChange = false;
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
        CString str;
        COleDataObject cSource;
        str.Empty();

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
            CString sValue;
            GetWindowText(sValue);
            if (sValue.IsEmpty())
            {
                ValueVaild = VaildValueSet;
                SetLast(true);
                return;
            }

            T val;
            if (Convert(val, sValue))
                SetValue(val, true);
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

    bool IsEdit(bool ReadOnly = true)
    {
        if(ReadOnly)
            return m_hWnd != nullptr && IsWindowEnabled() && (GetStyle() & ES_READONLY) == 0;

        return m_hWnd != nullptr && IsWindowEnabled();
    }


    void IncDecrement(bool inc)
    {
        int shct = (GetAsyncKeyState(VK_SHIFT  ) & 0x8000) != 0 ? 1 : 0;
        shct    |= (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0 ? 2 : 0;
        T stp = IncStp;

        switch (shct)
        {
        case 1:
            stp = IncStpSh;
            break;
        case 2:
            stp = IncStpCt;
            break;
        case 3:
            stp = IncStpShCt;
            break;
        }

        Value += inc ? stp : stp * -1;
        SetValue(Value, true);
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
        case VK_END:
        case VK_HOME:
            if (!ValueVaild)
            {
                SetLast(true);
                return;
            }
            break;

        case VK_UP:
        case VK_DOWN:
            IncDecrement(nChar == VK_UP);
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
            if (!ValueVaild)
                SetLast(true);
            break;
        }

        __super::OnKeyDown(nChar, nRepCnt, nFlags);
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

    afx_msg void OnEnChange()
    {
        if(bSend_ENChange)
            GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), EN_CHANGE), (LPARAM)GetParent()->GetSafeHwnd());
        else
            bSend_ENChange = true;
    }

    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor)
    {
        retBrush.Detach();


        if (!ValueVaild)
        {  // not valid view
            if(bTransNV)
                pDC->SetBkMode(TRANSPARENT);
            else
                pDC->SetBkMode(OPAQUE);

            pDC->SetBkColor(clBackNV);
            pDC->SetTextColor(clTextNV);
            retBrush.CreateSolidBrush(clBackNV);
        }
        else
        {
            if (IsEdit())
            {
                if (bTrans)
                    pDC->SetBkMode(TRANSPARENT);
                else
                    pDC->SetBkMode(OPAQUE);

                pDC->SetBkColor(clBack);
                pDC->SetTextColor(clText);
                retBrush.CreateSolidBrush(clBack);
            }
            else
            {   // readonly
                if (bTransRO)
                    pDC->SetBkMode(TRANSPARENT);
                else
                    pDC->SetBkMode(OPAQUE);

                pDC->SetBkColor(clBackRO);
                pDC->SetTextColor(clTextRO);
                retBrush.CreateSolidBrush(clBackRO);
            }
        }

        return (HBRUSH)retBrush;
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

            bMouseMsgsAct = true;

            if (bHandler)
                pPar->SendMessage(message, wParam, lParam);

            bMouseMsgsAct = false;
        }

        if(!bHandler)
            bHandler = __super::OnWndMsg(message, wParam, lParam, pResult);
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
#define ENF_TEMPLATE_MESSAGE_MAP(T)                                        \
PTM_WARNING_DISABLE                                                        \
template <class T, bool ShNotValue>                                        \
const AFX_MSGMAP* CEditNFlow<T, ShNotValue>::GetMessageMap() const         \
{                                                                          \
    return GetThisMessageMap();                                            \
}                                                                          \
template <class T, bool ShNotValue>                                        \
const AFX_MSGMAP* PASCAL CEditNFlow<T, ShNotValue>::GetThisMessageMap()    \
{                                                                          \
    typedef CEditNFlow<T, ShNotValue> ThisClass;                           \
    typedef CEdit TheBaseClass;                                            \
    __pragma(warning(push))                                                \
    __pragma(warning(disable: 4640)) /* message maps single threaded */    \
    static const AFX_MSGMAP_ENTRY _messageEntries[] =                      \
    {                                                                      \

ENF_TEMPLATE_MESSAGE_MAP(T)
    ON_CONTROL_REFLECT(EN_UPDATE,    OnUpdate)
    ON_CONTROL_REFLECT(EN_SETFOCUS,  OnEnSetfocus)
    ON_CONTROL_REFLECT(EN_KILLFOCUS, OnEnKillfocus)
    ON_CONTROL_REFLECT(EN_CHANGE,    OnEnChange)
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0,    OnToolTipNeedText)
    ON_WM_CTLCOLOR_REFLECT()

    ON_WM_KEYDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_MOUSEWHEEL()
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


