/******************************************************************************
/ SnM_VWnd.h
/
/ Copyright (c) 2012 Jeffos
/ http://www.standingwaterstudios.com/reaper
/
/ Permission is hereby granted, free of charge, to any person obtaining a copy
/ of this software and associated documentation files (the "Software"), to deal
/ in the Software without restriction, including without limitation the rights to
/ use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/ of the Software, and to permit persons to whom the Software is furnished to
/ do so, subject to the following conditions:
/ 
/ The above copyright notice and this permission notice shall be included in all
/ copies or substantial portions of the Software.
/ 
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/ OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/ NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/ HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/ WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/ FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
/ OTHER DEALINGS IN THE SOFTWARE.
/
******************************************************************************/

//#pragma once

#ifndef _SNM_VWND_H_
#define _SNM_VWND_H_

#include "SnM_Dlg.h"

#define SNM_DEF_VWND_X_STEP			12

class SNM_ImageVWnd : public WDL_VWnd {
public:
	SNM_ImageVWnd(LICE_IBitmap* _img = NULL) : WDL_VWnd() { SetImage(_img); }
	virtual ~SNM_ImageVWnd() {}
	virtual const char *GetType() { return "SNM_ImageVWnd"; }
	virtual int GetWidth();
	virtual int GetHeight();
	virtual void SetImage(LICE_IBitmap* _img) { m_img = _img; }
	virtual void OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);
protected:
	LICE_IBitmap* m_img;
};

class SNM_AddDelButton : public WDL_VWnd {
public:
	SNM_AddDelButton() : WDL_VWnd() { m_add=true; m_en=true; }
	virtual ~SNM_AddDelButton() {}
	virtual const char *GetType() { return "SNM_AddDelButton"; }
	virtual void OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);
	virtual void SetAdd(bool _add) { m_add=_add; }
	virtual void SetEnabled(bool _en) { m_en=_en; }
	virtual int OnMouseDown(int xpos, int ypos) { return m_en?1:0;	}
	virtual void OnMouseUp(int xpos, int ypos) { if (m_en) SendCommand(WM_COMMAND,GetID(),0,this); }
protected:
	bool m_add, m_en;
};

class SNM_MiniAddDelButtons : public WDL_VWnd {
public:
	SNM_MiniAddDelButtons() : WDL_VWnd()
	{
		m_btnPlus.SetID(0xF666); // temp ids.. SetIDs() must be used
		m_btnMinus.SetID(0xF667);
		m_btnPlus.SetAdd(true);
		m_btnMinus.SetAdd(false); 
		AddChild(&m_btnPlus);
		AddChild(&m_btnMinus);
	}
	virtual ~SNM_MiniAddDelButtons() { RemoveAllChildren(false); }
	virtual const char *GetType() { return "SNM_MiniAddDelButtons"; }
	virtual void SetIDs(int _id, int _addId, int _delId) { SetID(_id); m_btnPlus.SetID(_addId); m_btnMinus.SetID(_delId); }
	virtual void SetPosition(const RECT* _r)
	{
		m_position = *_r; 
		RECT r1 = {0, 0, 9, 9};
		m_btnPlus.SetPosition(&r1);
		RECT r2 = {0, 10, 9, 19};
		m_btnMinus.SetPosition(&r2);
	}
protected:
	SNM_AddDelButton m_btnPlus, m_btnMinus;
};

class SNM_ToolbarButton : public WDL_VirtualIconButton {
public:
	SNM_ToolbarButton() : WDL_VirtualIconButton() {}
	virtual ~SNM_ToolbarButton() {}
	virtual const char *GetType() { return "SNM_ToolbarButton"; }
	virtual void SetGrayed(bool grayed) { WDL_VirtualIconButton::SetGrayed(grayed); if (grayed) m_pressed=0; } // avoid stuck overlay when mousedown leads to grayed button
	virtual void GetPositionPaintOverExtent(RECT *r) { *r=m_position; }
	virtual void OnPaintOver(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect);
};

void SNM_SkinButton(WDL_VirtualIconButton* _btn, WDL_VirtualIconButton_SkinConfig* _skin, const char* _text);
void SNM_SkinToolbarButton(SNM_ToolbarButton* _btn, const char* _text);
bool SNM_AddLogo(LICE_IBitmap* _bm, const RECT* _r, int _x, int _h);
bool SNM_AutoVWndPosition(WDL_VWnd* _comp, WDL_VWnd* _tiedComp, const RECT* _r, int* _x, int _y, int _h, int _xRoom = SNM_DEF_VWND_X_STEP);

#endif
