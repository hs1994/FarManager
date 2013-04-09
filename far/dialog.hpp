#pragma once

/*
dialog.hpp

����� ������� Dialog.

������������ ��� ����������� ��������� ��������.
�������� ����������� �� ������ Frame.
*/
/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "frame.hpp"
#include "vmenu.hpp"
#include "bitflags.hpp"
#include "synchro.hpp"
#include "macro.hpp"

class History;

// ����� �������� ������ �������
enum DIALOG_MODES
{
	DMODE_INITOBJECTS           =0x00000001, // �������� ����������������?
	DMODE_CREATEOBJECTS         =0x00000002, // ������� (Edit,...) �������?
	DMODE_WARNINGSTYLE          =0x00000004, // Warning Dialog Style?
	DMODE_DRAGGED               =0x00000008, // ������ ���������?
	DMODE_ISCANMOVE             =0x00000010, // ����� �� ������� ������?
	DMODE_ALTDRAGGED            =0x00000020, // ������ ��������� �� Alt-�������?
	DMODE_SMALLDIALOG           =0x00000040, // "�������� ������"
	DMODE_DRAWING               =0x00001000, // ������ ��������?
	DMODE_KEY                   =0x00002000, // ���� ������� ������?
	DMODE_SHOW                  =0x00004000, // ������ �����?
	DMODE_MOUSEEVENT            =0x00008000, // ����� �������� MouseMove � ����������?
	DMODE_RESIZED               =0x00010000, //
	DMODE_ENDLOOP               =0x00020000, // ����� ����� ��������� �������?
	DMODE_BEGINLOOP             =0x00040000, // ������ ����� ��������� �������?
	DMODE_ISMENU                =0x00080000, // ������ �������� ����������� VMenu2
	DMODE_NODRAWSHADOW          =0x00100000, // �� �������� ����?
	DMODE_NODRAWPANEL           =0x00200000, // �� �������� ��������?
	DMODE_FULLSHADOW            =0x00400000,
	DMODE_NOPLUGINS             =0x00800000,
	DMODE_KEEPCONSOLETITLE      =0x10000000, // �� �������� ��������� �������
	DMODE_CLICKOUTSIDE          =0x20000000, // ���� ������� ���� ��� �������?
	DMODE_MSGINTERNAL           =0x40000000, // ���������� Message?
	DMODE_OLDSTYLE              =0x80000000, // ������ � ������ (�� 1.70) �����
};

//#define DIMODE_REDRAW       0x00000001 // ��������� �������������� ���������� �����?

#define MakeDialogItemsEx(Data,Item) \
	DialogItemEx Item[ARRAYSIZE(Data)]; \
	ItemToItemEx(Data,Item,ARRAYSIZE(Data));

// ���������, ����������� ������������� ��� DIF_AUTOMATION
// �� ������ ����� - ����������� - ����������� ������ � ��������� ��� CheckBox
struct DialogItemAutomation
{
	WORD ID;                    // ��� ����� ��������...
	FARDIALOGITEMFLAGS Flags[3][2];          // ...��������� ��� ��� �����
	// [0] - Unchecked, [1] - Checked, [2] - 3Checked
	// [][0] - Set, [][1] - Skip
};

/*
��������� ���� ������� ������� - ��������� �������������.
��� �������� ��� FarDialogItem
*/
struct DialogItemEx: public FarDialogItem
{
	int ListPos;
	string strHistory;
	string strMask;
	string strData;

	int ID;
	BitFlags IFlags;
	std::vector<DialogItemAutomation> Auto;
	void *ObjPtr;
	VMenu *ListPtr;
	class DlgUserControl *UCData;
	intptr_t SelStart;
	intptr_t SelEnd;

	DialogItemEx() {Clear();}

	void Clear()
	{
		ClearStruct(*static_cast<FarDialogItem*>(this));

		ListPos=0;
		strHistory.Clear();
		strMask.Clear();
		strData.Clear();
		ID=0;
		IFlags.ClearAll();
		Auto.clear();
		UserData=0;
		ObjPtr=nullptr;
		ListPtr=nullptr;
		UCData=nullptr;
		SelStart=0;
		SelEnd=0;
	}

	DialogItemEx &operator=(const FarDialogItem &Other)
	{
		*static_cast<FarDialogItem*>(this) = Other;
		return *this;
	}

	void Indent(int Delta)
	{
		X1 += Delta;
		X2 += Delta;
	}

	bool AddAutomation(int id,
		FARDIALOGITEMFLAGS UncheckedSet,FARDIALOGITEMFLAGS UncheckedSkip,
		FARDIALOGITEMFLAGS CheckedSet,FARDIALOGITEMFLAGS CheckedSkip,
		FARDIALOGITEMFLAGS Checked3Set,FARDIALOGITEMFLAGS Checked3Skip)
	{
		DialogItemAutomation Item;
		Item.ID=id;
		Item.Flags[0][0]=UncheckedSet;
		Item.Flags[0][1]=UncheckedSkip;
		Item.Flags[1][0]=CheckedSet;
		Item.Flags[1][1]=CheckedSkip;
		Item.Flags[2][0]=Checked3Set;
		Item.Flags[2][1]=Checked3Skip;
		Auto.push_back(Item);
		return true;
	}
};

class DlgEdit;
class ConsoleTitle;
class Plugin;
class Dialog;

class DialogOwner
{
public:
	intptr_t Handler(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
};
typedef intptr_t (DialogOwner::*MemberHandlerFunction)(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
typedef intptr_t (*StaticHandlerFunction)(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);

class Dialog: public Frame
{
public:
	template<class T, typename Y, class D>
	Dialog(T* OwnerClass, Y HandlerFunction, void* InitParam, D* SrcItem, size_t SrcItemCount):
		bInitOK(false)
	{
		static_assert(std::is_member_function_pointer<Y>::value, "Handler is not a member function");
		Construct(SrcItem, SrcItemCount, reinterpret_cast<DialogOwner*>(OwnerClass), reinterpret_cast<MemberHandlerFunction>(HandlerFunction), nullptr, InitParam);
	}
	Dialog(DialogItemEx *SrcItem, size_t SrcItemCount, StaticHandlerFunction DlgProc=nullptr,void* InitParam=nullptr);
	Dialog(const FarDialogItem *SrcItem, size_t SrcItemCount, StaticHandlerFunction DlgProc=nullptr,void* InitParam=nullptr);
	virtual ~Dialog();

	virtual int ProcessKey(int Key);
	virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
	virtual void Show();
	virtual void Hide();
	virtual void SetExitCode(int Code);
	virtual int GetTypeAndName(string &strType, string &strName);
	virtual int GetType() { return MODALTYPE_DIALOG; }
	virtual const wchar_t *GetTypeName() {return L"[Dialog]";};
	virtual MACROMODEAREA GetMacroMode();
	virtual int FastHide();
	virtual void ResizeConsole();
	virtual void SetPosition(int X1,int Y1,int X2,int Y2);
	virtual void FastShow() {ShowDialog();}

	bool InitOK() const {return bInitOK;}
	void GetDialogObjectsData();
	void SetDialogMode(DWORD Flags) { DialogMode.Set(Flags); }
	bool CheckDialogMode(DWORD Flags) const { return DialogMode.Check(Flags)!=FALSE; }
	// ����� ��� ����������� �������
	void AdjustEditPos(int dx,int dy);
	int IsMoving() const {return DialogMode.Check(DMODE_DRAGGED);}
	void SetModeMoving(bool IsMoving) { DialogMode.Change(DMODE_ISCANMOVE,IsMoving);}
	int  GetModeMoving() const {return DialogMode.Check(DMODE_ISCANMOVE);}
	void SetDialogData(void* NewDataDialog);
	void* GetDialogData() const {return DataDialog;}
	void InitDialog();
	void Process();
	void SetPluginOwner(Plugin* NewPluginAddress) {PluginOwner = ((NewPluginAddress == INVALID_HANDLE_VALUE)? nullptr : NewPluginAddress);}
	Plugin* GetPluginOwner() const {return PluginOwner;}
	void SetHelp(const string& Topic);
	void ShowHelp();
	int Done() const { return DialogMode.Check(DMODE_ENDLOOP); }
	void ClearDone();
	intptr_t CloseDialog();
	// For MACRO
	const std::vector<std::unique_ptr<DialogItemEx>>& GetAllItem() const { return Items; }
	size_t GetDlgFocusPos() const {return FocusPos;}
	int SetAutomation(WORD IDParent,WORD id, FARDIALOGITEMFLAGS UncheckedSet,FARDIALOGITEMFLAGS UncheckedSkip, FARDIALOGITEMFLAGS CheckedSet,FARDIALOGITEMFLAGS CheckedSkip,
		FARDIALOGITEMFLAGS Checked3Set=DIF_NONE,FARDIALOGITEMFLAGS Checked3Skip=DIF_NONE);

	intptr_t DlgProc(intptr_t Msg,intptr_t Param1,void* Param2);
	BOOL IsInited();
	bool ProcessEvents();
	void SetId(const GUID& Id);
	const GUID& GetId() const {return Id;}
	intptr_t SendMessage(intptr_t Msg,intptr_t Param1,void* Param2);
	intptr_t DefProc(intptr_t Msg,intptr_t Param1,void* Param2);

protected:
	size_t InitDialogObjects(size_t ID=(size_t)-1);

private:
	void Construct(DialogItemEx* SrcItem, size_t SrcItemCount, DialogOwner* OwnerClass, MemberHandlerFunction HandlerFunction, StaticHandlerFunction DlgProc=nullptr, void* InitParam=nullptr);
	void Construct(const FarDialogItem* SrcItem, size_t SrcItemCount, DialogOwner* OwnerClass, MemberHandlerFunction HandlerFunction, StaticHandlerFunction DlgProc=nullptr, void* InitParam=nullptr);
	void Init(DialogOwner* OwnerClass, MemberHandlerFunction HandlerFunction, StaticHandlerFunction DlgProc, void* InitParam);
	virtual void DisplayObject();
	void DeleteDialogObjects();
	int LenStrItem(size_t ID, const string& lpwszStr);
	int LenStrItem(size_t ID);
	void ShowDialog(size_t ID=(size_t)-1);  //    ID=-1 - ���������� ���� ������
	intptr_t CtlColorDlgItem(FarColor Color[4], size_t ItemPos, FARDIALOGITEMTYPES Type, bool Focus, bool Default,FARDIALOGITEMFLAGS Flags);
	/* $ 28.07.2000 SVS
		+ �������� ����� ����� ����� ����� ����������.
		    ������� �������� ��� ����, ����� ���������� DMSG_KILLFOCUS & DMSG_SETFOCUS
	*/
	void ChangeFocus2(size_t SetFocusPos);
	size_t ChangeFocus(size_t FocusPos,int Step,int SkipGroup);
	BOOL SelectFromEditHistory(DialogItemEx *CurItem,DlgEdit *EditLine,const string& HistoryName,string &strStr);
	int SelectFromComboBox(DialogItemEx *CurItem,DlgEdit*EditLine,VMenu *List);
	int AddToEditHistory(DialogItemEx* CurItem, const string& AddStr);
	void ProcessLastHistory(DialogItemEx *CurItem, int MsgIndex);  // ��������� DIF_USELASTHISTORY
	int ProcessHighlighting(int Key,size_t FocusPos,int Translate);
	int CheckHighlights(WORD Chr,int StartPos=0);
	void SelectOnEntry(size_t Pos,BOOL Selected);
	void CheckDialogCoord();
	BOOL GetItemRect(size_t I,SMALL_RECT& Rect);
	bool ItemHasDropDownArrow(const DialogItemEx *Item) const;
	const wchar_t *GetDialogTitle();
	BOOL SetItemRect(size_t ID,SMALL_RECT *Rect);
	void SetDropDownOpened(int Status) { DropDownOpened=Status; }
	int GetDropDownOpened() const { return DropDownOpened; }
	void ProcessCenterGroup();
	size_t ProcessRadioButton(size_t);
	int ProcessOpenComboBox(FARDIALOGITEMTYPES Type,DialogItemEx *CurItem,size_t CurFocusPos);
	int ProcessMoveDialog(DWORD Key);
	int Do_ProcessTab(int Next);
	int Do_ProcessNextCtrl(bool, bool IsRedraw=true);
	int Do_ProcessFirstCtrl();
	int Do_ProcessSpace();
	void SetComboBoxPos(DialogItemEx* Item=nullptr);
	void CalcComboBoxPos(DialogItemEx* CurItem, intptr_t ItemCount, int &X1, int &Y1, int &X2, int &Y2);
	void ProcessKey(int Key, size_t ItemPos);

	bool bInitOK;               // ������ ��� ������� ���������������
	class Plugin* PluginOwner;       // ������, ��� ������������ HelpTopic
	size_t FocusPos;               // ������ �������� ����� ������� � ������
	size_t PrevFocusPos;           // ������ �������� ����� ������� ��� � ������
	int IsEnableRedraw;         // ��������� ����������� �������? ( 0 - ���������)
	BitFlags DialogMode;        // ����� �������� ������ �������
	void* DataDialog;        // ������, ������������� ��� ����������� ���������� ������� (������������� ����� ��������, ���������� � �����������)
	std::vector<std::unique_ptr<DialogItemEx>> Items; // ������ ��������� �������
	DialogItemEx* SavedItems; // ���������������� ������ ��������� �������
	ConsoleTitle *OldTitle;     // ���������� ���������
	MACROMODEAREA PrevMacroMode;          // ���������� ����� �����
	DialogOwner *OwnerClass;
	union
	{
		MemberHandlerFunction DialogHandler;
		StaticHandlerFunction RealDlgProc;      // ������� ��������� �������
	};
	// ���������� ��� ����������� �������
	int OldX1,OldX2,OldY1,OldY2;
	string HelpTopic;
	int DropDownOpened;// �������� ������ ���������� � �������: TRUE - ������, FALSE - ������.
	CriticalSection CS;
	int RealWidth, RealHeight;
	GUID Id;
	bool IdExist;
	MOUSE_EVENT_RECORD PrevMouseRecord;

	friend class History;
	friend class DlgEdit;
};

bool IsKeyHighlighted(const string& Str,int Key,int Translate,int AmpPos=-1);
void ItemToItemEx(const FarDialogItem *Data, DialogItemEx *Item, size_t Count, bool Short = false);

intptr_t PluginDialogProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);

class PluginDialog: public Dialog
{
public:
	PluginDialog(const FarDialogItem *SrcItem, size_t SrcItemCount, FARWINDOWPROC DlgProc, void* InitParam):
		Dialog(SrcItem, SrcItemCount, DlgProc? PluginDialogProc : nullptr, InitParam),
		m_Proc(DlgProc)
	{}
	FARWINDOWPROC Proc() {return m_Proc;}

private:
	FARWINDOWPROC m_Proc;
};
