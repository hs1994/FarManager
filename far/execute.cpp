/*
execute.cpp

"�����������" ��������.

*/

/* Revision: 1.16 02.12.2001 $ */

/*
Modify:
  02.12.2001 SVS
    ! �������� �����. :-(( ������ ��� �������, �� � ��������� �������,
      �� ����� �������� ������ �������, ���� ���� �� ������� ����������, �.�.
      ���� ������ "date" - ����������� ���������� ������� ���.����., ����
      ������ "date.exe", �� ����� �������� � ����������� ������ "date.exe"
      � ��������� ��� ������ ���� ��� � ������.
  30.11.2001 SVS
    ! ����� ������ ����� ���������� ��������� � ����������
  29.11.2001 SVS
    - ���� � �������� ��������� - ������ ����������� ���� ������� � OEM.
  28.11.2001 SVS
    - BugZ#129 �� ����������� ���������� � �������� � ��������
    ! ��������� ��������� � PrepareExecuteModule()
  22.11.2001 SVS
    - ��� ��������� ��� ���� ����� ���������� ����� ��� �� ������� �������:
      >"" Enter
      ��. ����� ����������� ��� �� �������.
    + � Execute() �������� �������� - SetUpDirs "����� ������������� ��������?"
      ��� ��� ��� ��� �� �����, ����� ����� "�������" ����� ���� ���
      ��������� ���������. ��� ����������� ������� �����.
  21.11.2001 SVS
    ! ����������� � ��������� "��������" ���� ���������, � ��� ��
      �������������� IsCommandExeGUI � PrepareExecuteModule (����������
      ����� �������). GetExistAppPaths ������� �� �������������.
  21.11.2001 VVM
    ! ��������� �������� ���������� ��� ������� ��������.
  20.11.2001 SVS
    - BugZ#111 - ��� cd ��: ������������� ����� ����� - ������� �� Upper.
  20.11.2001 SVS
    ! ��������� ���������.
  19.11.2001 SVS
    + GetExistAppPaths() - �������� ���� ���� ���� �� App Paths
    ! ������� IsCommandExeGUI() ������ ���������� ����������� ������ ����
      � ��������� �����
  15.11.2001 OT
    - ����������� ��������� cd c:\ �� �������� ��������� �������
  14.11.2001 SVS
    - ����������� ����������� BugZ#90 - ������ �� �����������
  12.11.2001 SVS
    - BugZ#90: ������ �������� �� ������
  12.11.2001 SVS
    ! ����� 1033 � 1041 �� ������ ������.
  08.11.2001 SVS
    - ��������� ������� (�������� � ���� ���� ���������) ������� (��� ��������)
  31.10.2001 VVM
    + ������� ���������� ������ ��������. ��������� ������� �� ����� "start.exe",
      � ����� CREATE_NEW_CONSOLE
  10.10.2001 SVS
    + ������
*/

#include "headers.hpp"
#pragma hdrstop

#include "filepanels.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "plugin.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "chgprior.hpp"
#include "global.hpp"
#include "cmdline.hpp"
#include "panel.hpp"
#include "fn.hpp"
#include "rdrwdsk.hpp"

// ��������� ����� �� �������� GetFileInfo, �������� ����������� ���������� �
// ���� PE-������
static int IsCommandPEExeGUI(const char *FileName,DWORD *IsPEGUI)
{
  char NameFile[NM];
  HANDLE hFile;
  int Ret=FALSE;
  *IsPEGUI=0;

  if((hFile=CreateFile(FileName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL)) != INVALID_HANDLE_VALUE)
  {
    DWORD FileSizeLow, FileSizeHigh, ReadSize;
    IMAGE_DOS_HEADER dos_head;

    FileSizeLow=GetFileSize(hFile,&FileSizeHigh);

    if(ReadFile(hFile,&dos_head,sizeof(IMAGE_DOS_HEADER),&ReadSize,NULL) &&
       dos_head.e_magic == IMAGE_DOS_SIGNATURE)
    {
      Ret=TRUE;
      /*  ���� �������� ����� �� �������� 18h (OldEXE - MZ) >= 40h,
      �� �������� ����� � 3Ch �������� ��������� ��������� Windows. */
      if (dos_head.e_lfarlc >= 0x40)
      {
        DWORD signature;
        #include <pshpack1.h>
        struct __HDR
        {
           DWORD signature;
           IMAGE_FILE_HEADER _head;
           IMAGE_OPTIONAL_HEADER opt_head;
           // IMAGE_SECTION_HEADER section_header[];  /* actual number in NumberOfSections */
        } header, *pheader;
        #include <poppack.h>

        if(SetFilePointer(hFile,dos_head.e_lfanew,NULL,FILE_BEGIN) != -1)
        {
          // ������ ��������� ���������
          if(ReadFile(hFile,&header,sizeof(struct __HDR),&ReadSize,NULL))
          {
            signature=header.signature;
            pheader=&header;

            if(signature == IMAGE_NT_SIGNATURE) // PE
            {
              *IsPEGUI=header.opt_head.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI;
            }
            else if((WORD)signature == IMAGE_OS2_SIGNATURE) // NE
            {
              ; // NE,  ���...  � ��� ���������� ��� ��� ������?
            }
          }
          else
          {
            ; // ������ ����� � ������� ���������� ��������� ;-(
          }
        }
        else
        {
          ; // ������ �������� ���� ���� � �����, �.�. dos_head.e_lfanew
            // ������ ������� � �������������� ����� (�������� ��� ������
            // ���� DOS-����
        }
      }
      else
      {
        ; // ��� ������� EXE, �� �� �������� EXE
      }
    }
    else
    {
      ; // ��� �� ����������� ���� - � ���� ���� ��������� MZ
        // ��������, NLM-������ ��� ������ ������ :-)
    }
    CloseHandle(hFile);
  }
  return Ret;
}

char* GetShellAction(char *FileName)
{
  char Value[80],*ExtPtr;
  LONG ValueSize;
  if ((ExtPtr=strrchr(FileName,'.'))==NULL)
    return(NULL);
  ValueSize=sizeof(Value);
  if (RegQueryValue(HKEY_CLASSES_ROOT,(LPCTSTR)ExtPtr,(LPTSTR)Value,&ValueSize)!=ERROR_SUCCESS)
    return(NULL);
  strcat(Value,"\\shell");

  HKEY hKey;
  if (RegOpenKey(HKEY_CLASSES_ROOT,Value,&hKey)!=ERROR_SUCCESS)
    return(NULL);

  static char Action[80];
  ValueSize=sizeof(Action);
  if (RegQueryValueEx(hKey,"",NULL,NULL,(unsigned char *)Action,(LPDWORD)&ValueSize)!=ERROR_SUCCESS)
    return(NULL);
  RegCloseKey(hKey);
  return(*Action==0 ? NULL:Action);
}


/*
 ������ PrepareExecuteModule �������� ����� ����������� ������ (� �.�. � ��
 %PATHEXT%). � ������ ������ �������� � Command ������, ������������� ��
 ����������� ������ �� ��������� ��������, �������� ��������� � Dest �
 �������� ��������� ��������� PE �� �������� (����� ��������� �������
 � ��������� ���� � �� ����� ����������).
 � ������ ������� Dest �� �����������!
 Return: TRUE/FALSE/-1 - �����/�� �����/������ ������.
*/
int WINAPI PrepareExecuteModule(char *Command,char *Dest,int DestSize,DWORD *GUIType)
{
  int Ret, I;
  char FileName[4096],FullName[4096], *Ptr;
  int IsQuoted=FALSE;
  int IsExistExt=FALSE;

  // ����� ������� �����! ������� �������,  � ����� ��������� �����.
  static char StdExecuteExt[NM]=".BAT;.CMD;.EXE;.COM;";
  static int PreparePrepareExt=FALSE;

  if(!PreparePrepareExt) // ���������������������� �����
  {
#if 1
    // ���� ���������� %PATHEXT% ��������...
    if((I=GetEnvironmentVariable("PATHEXT",FullName,sizeof(FullName)-1)) != 0)
    {
      FullName[I]=0;
      // ������� ������� �� PATHEXT
      static char const * const StdExecuteExt0[4]={".BAT;",".CMD;",".EXE;",".COM;"};
      for(I=0; I < sizeof(StdExecuteExt0)/sizeof(StdExecuteExt0[0]); ++I)
        ReplaceStrings(FullName,StdExecuteExt0[I],"",-1);
    }

    Ptr=strcat(StdExecuteExt,strcat(FullName,";")); //!!!
#else
    Ptr=StdExecuteExt;
#endif
    StdExecuteExt[strlen(StdExecuteExt)]=0;
    while(*Ptr)
    {
      if(*Ptr == ';')
        *Ptr=0;
      ++Ptr;
    }
    PreparePrepareExt=TRUE;
  }

  /* ����� "����������" �� �������, ������� ������ ����������� ��������,
     ��������, ��������� ���������� ������� ���.����������.
  static char ExcludeCmds[4096];
  static int PrepareExcludeCmds=FALSE;
  if(!PrepareExcludeCmds)
  {
    GetRegKey("System","ExcludeCmds",(char*)ExcludeCmds,"",0);
    PrepareExcludeCmds=TRUE;
  }
  */

  *GUIType=FALSE; // GUIType ������ ������� ���������������� � FALSE
  Ret=FALSE;

  // �������� ��� ������
  if (*Command=='\"')
  {
    OemToChar(Command+1,FileName);
    if ((Ptr=strchr(FileName,'\"'))!=NULL)
      *Ptr=0;
    IsQuoted=TRUE;
  }
  else
  {
    OemToChar(Command,FileName);
    if ((Ptr=strpbrk(FileName," \t/"))!=NULL)
      *Ptr=0;
  }

  if(!*FileName) // ��� ��, ���� ��... �������� �������� :-(
    return -1;

  /* $ 07.09.2001 VVM ���������� ���������� ��������� */
  ExpandEnvironmentStr(FileName,FileName,sizeof(FileName));
  // IsExistExt - ���� ����� ���� (����������), �� ����� �������������� ��
  // �����.
  IsExistExt=strrchr(FileName,'.')!=NULL;

  SetFileApisToANSI();

  {
    char *FilePart;
    char *PtrFName=strrchr(strcpy(FullName,FileName),'.');
    char *WorkPtrFName;
    if(!PtrFName)
      WorkPtrFName=FullName+strlen(FullName);

    char *PtrExt=StdExecuteExt;
    while(*PtrExt) // ������ ������ - � ������� ��������
    {
      if(!PtrFName)
        strcpy(WorkPtrFName,PtrExt);
      if(GetFileAttributes(FullName)!=-1)
      {
        // GetFullPathName - ��� �����, �.�. ���� ������ � date.exe
        // � ������� ��������, �� ������ ������ ������� �� ��������
        // cmd.exe �� ����� �� �������� ������� ���������� date
        GetFullPathName(FullName,sizeof(FullName),FullName,&FilePart);

        Ret=TRUE;
        break;
      }
      PtrExt+=strlen(PtrExt)+1;
    }

    if(!Ret) // ������ ������ - �� �������� SearchPath
    {
      PtrExt=StdExecuteExt;
      while(*PtrExt)
      {
        if(!PtrFName)
          strcpy(WorkPtrFName,PtrExt);
        if(SearchPath(NULL,FullName,PtrExt,sizeof(FullName),FullName,&FilePart))
        {
          Ret=TRUE;
          break;
        }
        PtrExt+=strlen(PtrExt)+1;
      }

      if(!Ret) // ������ ������ - ����� � ������ � "App Paths"
      {
        // � ������ Command ������� ����������� ������ �� ������ ����, �������
        // ������� �� SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
        // ������� ������� � HKCU, ����� - � HKLM
        HKEY hKey;
        HKEY RootFindKey[2]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE};
        PtrExt=StdExecuteExt;
        while(*PtrExt)
        {
          if(!PtrFName)
            strcpy(WorkPtrFName,PtrExt);
          for(I=0; I < sizeof(RootFindKey)/sizeof(RootFindKey[0]); ++I)
          {
            sprintf(FullName,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s",FileName);
            if (RegOpenKeyEx(RootFindKey[I], FullName, 0,KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
            {
              DWORD Type, DataSize=sizeof(FullName);
              RegQueryValueEx(hKey,"", 0, &Type, (LPBYTE)FullName,&DataSize);
              RegCloseKey(hKey);
              Ret=TRUE;
              break;
            }
          }
          if(Ret)
            break;
          PtrExt+=strlen(PtrExt)+1;
        }
      }
    }
  }

  if(Ret) // ��������� "�������" ������
  {
    char TempStr[4096];
    // ������� ��������...
    IsCommandPEExeGUI(FullName,GUIType);
    QuoteSpaceOnly(FullName);
    QuoteSpaceOnly(FileName);
    strncpy(TempStr,Command,sizeof(TempStr)-1);
    CharToOem(FullName,FullName);
    CharToOem(FileName,FileName);
    ReplaceStrings(TempStr,FileName,FullName);
    if(!DestSize)
      DestSize=strlen(TempStr);
    if(Dest && IsExistExt)
      strncpy(Dest,TempStr,DestSize);
  }

  SetFileApisToOEM();
  return(Ret);
}

/* �������-��������� ������� ���������
   ���������� -1 � ������ ������ ���...
*/
int Execute(char *CmdStr,          // ���.������ ��� ����������
            int AlwaysWaitFinish,  // ����� ���������� ��������?
            int SeparateWindow,    // ��������� � ��������� ����?
            int DirectRun,         // ��������� ��������? (��� CMD)
            int SetUpDirs)         // ����� ������������� ��������?
{
  char NewCmdStr[4096];

  // ������������ �� ��������
  Unquote(strcpy(NewCmdStr,CmdStr));
  RemoveExternalSpaces(NewCmdStr);
  // ������ �� ���������
  if(!*NewCmdStr)
  {
    // � ����� ������ ��������� CMD ��� ���������?
    // ���� "��", �� ���� ����� ����� ���� ���������.
    return -1;
  }

  CONSOLE_SCREEN_BUFFER_INFO sbi;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  int Visible,Size;
  int PrevLockCount;
  char ExecLine[1024],CommandName[NM];
  char OldTitle[512];
  DWORD GUIType;
  int ExitCode=0;
  int NT;
  int OldNT;
  DWORD CreateFlags;
  char *CmdPtr;

  /* $ 13.04.2001 VVM
    + ���� CREATE_DEFAULT_ERROR_MODE. ���-�� ��������� ��� ������ */
  CreateFlags=CREATE_DEFAULT_ERROR_MODE;
  /* VVM $ */

  GetCursorType(Visible,Size);
  SetCursorType(TRUE,-1);

  PrevLockCount=ScrBuf.GetLockCount();
  ScrBuf.SetLockCount(0);
  ScrBuf.Flush();

  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  GetConsoleTitle(OldTitle,sizeof(OldTitle));
  memset(&si,0,sizeof(si));
  si.cb=sizeof(si);

  NT=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT;
  OldNT=NT && WinVer.dwMajorVersion<4;

  *CommandName=0;
  GetEnvironmentVariable("COMSPEC",CommandName,sizeof(CommandName));

  if(SetUpDirs)
  {
    Panel *PassivePanel=CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel);
    if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS && PassivePanel->GetType()==FILE_PANEL)
      for (int I=0;CmdStr[I]!=0;I++)
        if (isalpha(CmdStr[I]) && CmdStr[I+1]==':' && CmdStr[I+2]!='\\')
        {
          char SavePath[NM],PanelPath[NM],SetPathCmd[NM];
          GetCurrentDirectory(sizeof(SavePath),SavePath);
          PassivePanel->GetCurDir(PanelPath);
          sprintf(SetPathCmd,"%s /C chdir %s",CommandName,QuoteSpace(PanelPath));
          CreateProcess(NULL,SetPathCmd,NULL,NULL,FALSE,CreateFlags,NULL,NULL,&si,&pi);
          CloseHandle(pi.hThread);
          CloseHandle(pi.hProcess);
          chdir(SavePath);
        }
  }

  CmdPtr=strcpy(NewCmdStr,CmdStr);
  //while (isspace(*CmdPtr))
  //  CmdPtr++;

  // ����� �����������....
  PrepareExecuteModule(NewCmdStr,NewCmdStr,sizeof(NewCmdStr)-1,&GUIType);

  {
    if (DirectRun && !SeparateWindow)
      strcpy(ExecLine,CmdPtr);
    else
    {
      sprintf(ExecLine,"%s /C",CommandName);
      if (!OldNT && (SeparateWindow || GUIType && (NT || AlwaysWaitFinish)))
      {
        strcat(ExecLine," start");
        if (AlwaysWaitFinish)
          strcat(ExecLine," /wait");
        if (NT && *CmdPtr=='\"')
          strcat(ExecLine," \"\"");
      }
      strcat(ExecLine," ");

      char *CmdEnd=CmdPtr+strlen (CmdPtr)-1;
      if (NT && *CmdPtr == '\"' && *CmdEnd == '\"' && strchr (CmdPtr+1, '\"') != CmdEnd)
      {
        strcat (ExecLine, "\"");
        strcat (ExecLine, CmdPtr);
        strcat (ExecLine, "\"");
      }
      else
        strcat(ExecLine,CmdPtr);
    }

    SetFarTitle(CmdPtr);
    FlushInputBuffer();

    /*$ 15.03.2001 SKV
      ���� ��������� ��������� ������� �� ������ � �.�.
    */
    GetConsoleScreenBufferInfo(hConOut,&sbi);
    /* SKV$*/

    ChangeConsoleMode(InitialConsoleMode);

    if (SeparateWindow)
      CreateFlags|=(OldNT)?CREATE_NEW_CONSOLE:DETACHED_PROCESS;

    if (SeparateWindow==2)
    {
      char AnsiLine[NM];
      SHELLEXECUTEINFO si;
      OemToChar(CmdPtr,AnsiLine);

      if (PointToName(AnsiLine)==AnsiLine)
      {
        char FullName[2*NM];
        sprintf(FullName,".\\%s",AnsiLine);
        strcpy(AnsiLine,FullName);
      }

      memset(&si,0,sizeof(si));
      si.cbSize=sizeof(si);
      si.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_FLAG_DDEWAIT;
      si.lpFile=AnsiLine;
      si.lpVerb=GetShellAction((char *)si.lpFile);
      si.nShow=SW_SHOWNORMAL;
      SetFileApisToANSI();
      ExitCode=ShellExecuteEx(&si);
      SetFileApisToOEM();
      pi.hProcess=si.hProcess;
    }
    else
      ExitCode=CreateProcess(NULL,ExecLine,NULL,NULL,0,CreateFlags,
                             NULL,NULL,&si,&pi);

    StartExecTime=clock();
  }

  if (ExitCode)
  {
    if (!SeparateWindow && !GUIType || AlwaysWaitFinish)
    {
      /*$ 12.02.2001 SKV
        ����� ����� ;)
        ��������� ��������� ������� �� ���������������� ��������.
        ������� ������� � System/ConsoleDetachKey
      */
      if(Opt.ConsoleDetachKey>0)
      {
        HANDLE h[2];
        HANDLE hConOut=GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE hConInp=GetStdHandle(STD_INPUT_HANDLE);
        h[0]=pi.hProcess;
        h[1]=hConInp;
        INPUT_RECORD ir[256];
        DWORD rd;

        int vkey=0,ctrl=0;
        TranslateKeyToVK(Opt.ConsoleDetachKey,vkey,ctrl,NULL);
        int alt=ctrl&PKF_ALT;
        int shift=ctrl&PKF_SHIFT;
        ctrl=ctrl&PKF_CONTROL;

        while(WaitForMultipleObjects(2,h,FALSE,INFINITE)!=WAIT_OBJECT_0)
        {
          if(PeekConsoleInput(h[1],ir,256,&rd) && rd)
          {
            int stop=0;
            for(int i=0;i<rd;i++)
            {
              PINPUT_RECORD pir=&ir[i];
              if(pir->EventType==KEY_EVENT)
              {
                if(vkey==pir->Event.KeyEvent.wVirtualKeyCode &&
                  (alt?(pir->Event.KeyEvent.dwControlKeyState&LEFT_ALT_PRESSED || pir->Event.KeyEvent.dwControlKeyState&RIGHT_ALT_PRESSED):!(pir->Event.KeyEvent.dwControlKeyState&LEFT_ALT_PRESSED || pir->Event.KeyEvent.dwControlKeyState&RIGHT_ALT_PRESSED)) &&
                  (ctrl?(pir->Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED || pir->Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED):!(pir->Event.KeyEvent.dwControlKeyState&LEFT_CTRL_PRESSED || pir->Event.KeyEvent.dwControlKeyState&RIGHT_CTRL_PRESSED)) &&
                  (shift?(pir->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED):!(pir->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED))
                  )
                {

                  HICON hSmallIcon=NULL,hLargeIcon=NULL;
                  if(hFarWnd)
                  {
                    hSmallIcon=CopyIcon((HICON)SendMessage(hFarWnd,WM_SETICON,0,(LPARAM)0));
                    hLargeIcon=CopyIcon((HICON)SendMessage(hFarWnd,WM_SETICON,1,(LPARAM)0));
                  }
                  ReadConsoleInput(hConInp,ir,256,&rd);
                  CloseConsole();
                  FreeConsole();
                  AllocConsole();

                  /*$ 17.05.2001 SKV
                    ���� ���� ����� HOTKEY, �� ������ ������ ��� ������.
                  */
                  if(hFarWnd)
                  {
                    SendMessage(hFarWnd,WM_SETHOTKEY,0,(LPARAM)0);
                  }
                  /* SKV$*/


                  /*$ 20.03.2001 SKV
                    ��� ����� ��� ������ :-\
                  */
                  SetConsoleScreenBufferSize(hConOut,sbi.dwSize);
                  SetConsoleWindowInfo(hConOut,TRUE,&sbi.srWindow);
                  SetConsoleScreenBufferSize(hConOut,sbi.dwSize);

                  /* SKV$*/

                  Sleep(100);
                  InitConsole(0);

                  hFarWnd=0;
                  InitDetectWindowedMode();

                  if (hFarWnd)
                  {
                    if(Opt.SmallIcon)
                    {
                      char FarName[NM];
                      GetModuleFileName(NULL,FarName,sizeof(FarName));
                      ExtractIconEx(FarName,0,&hLargeIcon,&hSmallIcon,1);
                    }
                    if (hLargeIcon!=NULL)
                      SendMessage(hFarWnd,WM_SETICON,1,(LPARAM)hLargeIcon);
                    if (hSmallIcon!=NULL)
                      SendMessage(hFarWnd,WM_SETICON,0,(LPARAM)hSmallIcon);
                  }

                  stop=1;
                  break;
                }
              }
            }
            if(stop)break;
          }
          Sleep(100);
        }
      }
      else
      {
        WaitForSingleObject(pi.hProcess,INFINITE);
      }
      /* SKV$*/
    }
    int CurScrX=ScrX,CurScrY=ScrY;
//    ReopenConsole();

//OT    GetVideoMode();
//OT    if (CurScrX!=ScrX || CurScrY!=ScrY)
//OT      CtrlObject->Cp()->SetScreenPositions();
    GetExitCodeProcess(pi.hProcess,(LPDWORD)&ExitCode);
    if (SeparateWindow!=2)
      CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    ScrBuf.FillBuf();
    ScrBuf.SetLockCount(PrevLockCount);
  }
  else
  {
    if (SeparateWindow!=2)
      Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MCannotExecute),
              SeparateWindow==2 ? CmdPtr:ExecLine,MSG(MOk));
    ExitCode=-1;
  }
  SetFarConsoleMode();
  /* $ 05.10.2001 IS
     - ��������
     + �������������� ��������� �������, �.�. SetCursorType ������ �� �������
       ���������� ����� �����������, ������� � ������ ������ ������� �����.
  */
  SetCursorType(Visible,Size);
  SetRealCursorType(Visible,Size);
  /* IS $ */
  if (WinVer.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS &&
      WinVer.dwBuildNumber<=0x4000457)
    WriteInput(VK_F16,SKEY_VK_KEYS);
  /* ���� ���� �������� ������� �������, ��������
     mode con lines=50 cols=100
     �� ��� �� ���� �� ��������� ������� �������.
     ��� ����� ���� ���� ��������� ������ ��� :-)
  */
  GenerateWINDOW_BUFFER_SIZE_EVENT(-1,-1);
  SetConsoleTitle(OldTitle);
  return(ExitCode);
}

int CommandLine::CmdExecute(char *CmdLine,int AlwaysWaitFinish,
                            int SeparateWindow,int DirectRun)
{
  LastCmdPartLength=-1;
  if (!SeparateWindow && CtrlObject->Plugins.ProcessCommandLine(CmdLine))
  {
    /* $ 12.05.2001 DJ
       �������� ������ ���� �������� ������� �������
    */
    if (CtrlObject->Cp()->IsTopFrame())
    {
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      ScrBuf.Flush();
    }
    return(-1);
  }
  int Code;
  /* 21.11.2001 VVM
    ! � ��������� ��� �������� � ����������� ����.
      ����� �� ������ ������� ����� :) */
  {

    // �������� ��������� ������� � ������� ��, ���-�� �� ���������������� �� �������
    // � ����� ����� � ��������������� ����
    int LeftVisible = CtrlObject->Cp()->LeftPanel->IsVisible();
    int RightVisible = CtrlObject->Cp()->RightPanel->IsVisible();
    CtrlObject->Cp()->LeftPanel->SetVisible(FALSE);
    CtrlObject->Cp()->RightPanel->SetVisible(FALSE);

    {
      RedrawDesktop Redraw(TRUE);
      ScrollScreen(1);
      MoveCursor(X1,Y1);
      if (CurDir[0] && CurDir[1]==':')
        chdir(CurDir);
      CmdStr.SetString("");
      if (ProcessOSCommands(CmdLine))
        Code=-1;
      else
        Code=Execute(CmdLine,AlwaysWaitFinish,SeparateWindow,DirectRun);

      int CurX,CurY;
      GetCursorPos(CurX,CurY);
      if (CurY>=Y1-1)
        ScrollScreen(Min(CurY-Y1+2,Opt.ShowKeyBar ? 2:1));
    }

    // ����������� ��������� �������, �������, ���� �������� �
    // ���������� ���� ����� (������, ������, ����...)
    CtrlObject->Cp()->LeftPanel->SetVisible(LeftVisible);
    CtrlObject->Cp()->RightPanel->SetVisible(RightVisible);
    CtrlObject->Cp()->LeftPanel->UpdateIfChanged(1);
    CtrlObject->Cp()->RightPanel->UpdateIfChanged(1);
    CtrlObject->Cp()->Redraw();
  }
  /* VVM $ */
  ScrBuf.Flush();
  return(Code);
}


/* $ 14.01.2001 SVS
   + � ProcessOSCommands ��������� ���������
     "IF [NOT] EXIST filename command"
     "IF [NOT] DEFINED variable command"

   ��� ������� ������������� ��� ��������� ���������� IF`�
   CmdLine - ������ ������ ����
     if exist file if exist file2 command
   Return - ��������� �� "command"
            ������ ������ - ������� �� ���������
            NULL - �� ������� "IF" ��� ������ � �����������, ��������
                   �� exist, � esist ��� ����������� �������.

   DEFINED - ������� EXIST, �� ��������� � ����������� �����

   �������� ������ (CmdLine) �� ��������������!!!
*/
char* WINAPI PrepareOSIfExist(char *CmdLine)
{
  if(!CmdLine || !*CmdLine)
    return NULL;

  char Cmd[1024], *PtrCmd=CmdLine, *CmdStart;
  int Not=FALSE;
  int Exist=0; // ������� ������� ����������� "IF [NOT] EXIST filename command"
               // > 0 - ���� ����� �����������

  /* $ 25.04.2001 DJ
     ��������� @ � IF EXIST
  */
  if (*PtrCmd == '@')
  {
    // ����� @ ������������; �� ������� � ���������� ����� �������
    // ExtractIfExistCommand � filetype.cpp
    PtrCmd++;
    while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd;
  }
  /* DJ $ */
  while(1)
  {
    if (!PtrCmd || !*PtrCmd || memicmp(PtrCmd,"IF ",3))
      break;

    PtrCmd+=3; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;

    if (memicmp(PtrCmd,"NOT ",4)==0)
    {
      Not=TRUE;
      PtrCmd+=4; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
    }

    if (*PtrCmd && !memicmp(PtrCmd,"EXIST ",6))
    {
      PtrCmd+=6; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;

      /* $ 25.04.01 DJ
         ��������� ������� ������ ����� ����� � IF EXIST
      */
      BOOL InQuotes=FALSE;
      while (*PtrCmd)
      {
        if (*PtrCmd == '\"')
          InQuotes = !InQuotes;
        else if (*PtrCmd == ' ' && !InQuotes)
          break;
        PtrCmd++;
      }

      if(PtrCmd && *PtrCmd && *PtrCmd == ' ')
      {
        char ExpandedStr[8192];
        memmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
        Cmd[PtrCmd-CmdStart]=0;
        Unquote(Cmd);
//_SVS(SysLog("Cmd='%s'",Cmd));
        if (ExpandEnvironmentStr(Cmd,ExpandedStr,sizeof(ExpandedStr))!=0)
        {
          char FullPath[8192]="";
          if(!(Cmd[1] == ':' || (Cmd[0] == '\\' && Cmd[1]=='\\')))
          {
            if(CtrlObject)
              CtrlObject->CmdLine->GetCurDir(FullPath);
            else
              GetCurrentDirectory(sizeof(FullPath),FullPath);
            AddEndSlash(FullPath);
          }
          strcat(FullPath,ExpandedStr);
          ConvertNameToFull(FullPath,FullPath, sizeof(FullPath));
          DWORD FileAttr=GetFileAttributes(FullPath);
//_SVS(SysLog("%08X FullPath=%s",FileAttr,FullPath));
          if(FileAttr != (DWORD)-1 && !Not || FileAttr == (DWORD)-1 && Not)
          {
            while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return "";
        }
      }
      /* DJ $ */
    }
    // "IF [NOT] DEFINED variable command"
    else if (*PtrCmd && !memicmp(PtrCmd,"DEFINED ",8))
    {
      PtrCmd+=8; while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd; if(!*PtrCmd) break;
      CmdStart=PtrCmd;
      if(*PtrCmd == '"')
        PtrCmd=strchr(PtrCmd+1,'"');

      if(PtrCmd && *PtrCmd)
      {
        PtrCmd=strchr(PtrCmd,' ');
        if(PtrCmd && *PtrCmd && *PtrCmd == ' ')
        {
          char ExpandedStr[8192];
          memmove(Cmd,CmdStart,PtrCmd-CmdStart+1);
          Cmd[PtrCmd-CmdStart]=0;
          DWORD ERet=GetEnvironmentVariable(Cmd,ExpandedStr,sizeof(ExpandedStr));
//_SVS(SysLog(Cmd));
          if(ERet && !Not || !ERet && Not)
          {
            while(*PtrCmd && isspace(*PtrCmd)) ++PtrCmd;
            Exist++;
          }
          else
            return "";
        }
      }
    }
  }
  return Exist?PtrCmd:NULL;
}
/* SVS $ */


int CommandLine::ProcessOSCommands(char *CmdLine)
{
  Panel *SetPanel;
  int Length;
  SetPanel=CtrlObject->Cp()->ActivePanel;
  if (SetPanel->GetType()!=FILE_PANEL && CtrlObject->Cp()->GetAnotherPanel(SetPanel)->GetType()==FILE_PANEL)
    SetPanel=CtrlObject->Cp()->GetAnotherPanel(SetPanel);
  RemoveTrailingSpaces(CmdLine);
  if (isalpha(CmdLine[0]) && CmdLine[1]==':' && CmdLine[2]==0)
  {
    int NewDisk=toupper(CmdLine[0])-'A';
    setdisk(NewDisk);
    if (getdisk()!=NewDisk)
    {
      char NewDir[10];
      sprintf(NewDir,"%c:\\",NewDisk+'A');
      chdir(NewDir);
      setdisk(NewDisk);
    }
    SetPanel->ChangeDirToCurrent();
    return(TRUE);
  }
  if (strnicmp(CmdLine,"SET ",4)==0)
  {
    char Cmd[1024];
    strcpy(Cmd,CmdLine+4);
    char *Value=strchr(Cmd,'=');
    if (Value==NULL)
      return(FALSE);
    *Value=0;
    if (Value[1]==0)
      SetEnvironmentVariable(Cmd,NULL);
    else
    {
      char ExpandedStr[8192];
      /* $ 17.06.2001 IS
         ! ��������� ExpandEnvironmentStr, �.�. ��� ��������� �������� �
           �������� �������.
         + ������������ ������ ����� SetEnvironmentVariable �� OEM � ANSI
      */
      if (ExpandEnvironmentStr(Value+1,ExpandedStr,sizeof(ExpandedStr))!=0)
      {
        // ���������� ��������� ������ ���� � ANSI???
        OemToChar(ExpandedStr, ExpandedStr);
        SetEnvironmentVariable(Cmd,ExpandedStr);
      }
      /* IS $ */
    }
    return(TRUE);
  }

  if (!memicmp(CmdLine,"REM ",4) || !memicmp(CmdLine,"::",2))
  {
    return TRUE;
  }

  if (!memicmp(CmdLine,"CLS",3))
  {
    SetScreen(0,0,ScrX,ScrY,' ',F_LIGHTGRAY|B_BLACK);
    ScrBuf.ResetShadow();
    ScrBuf.Flush();
    return TRUE;
  }

  /* $ 14.01.2001 SVS
     + � ProcessOSCommands ��������� ���������
       "IF [NOT] EXIST filename command"
       "IF [NOT] DEFINED variable command"
  */
  if (memicmp(CmdLine,"IF ",3)==0)
  {
    char *PtrCmd=PrepareOSIfExist(CmdLine);
    // ����� PtrCmd - ��� ������� �������, ��� IF
    if(PtrCmd && *PtrCmd && CtrlObject->Plugins.ProcessCommandLine(PtrCmd))
    {
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return TRUE;
    }
    return FALSE;
  }
  /* SVS $ */

  if ((strnicmp(CmdLine,"CD",Length=2)==0 || strnicmp(CmdLine,"CHDIR",Length=5)==0) &&
      (isspace(CmdLine[Length]) || CmdLine[Length]=='\\' || strcmp(CmdLine+Length,"..")==0))
  {
    int ChDir=(Length==5);
    while (isspace(CmdLine[Length]))
      Length++;
    if (CmdLine[Length]=='\"')
      Length++;
    char NewDir[NM];
    strcpy(NewDir,&CmdLine[Length]);

    // ������������� ����� ����� �� "���������"
    if(NewDir[1] == ':' && isalpha(NewDir[0]))
      NewDir[0]=toupper(NewDir[0]);

    /* $ 15.11.2001 OT
      ������� ��������� ���� �� ����� "�������" ����������.
      ���� �� ���, �� ����� �������� ������, ��� ��� ���������� ���������
    */
    DWORD DirAtt=GetFileAttributes(NewDir);
    if (DirAtt!=0xffffffff && DirAtt & FILE_ATTRIBUTE_DIRECTORY && PathMayBeAbsolute(NewDir))
    {
      SetPanel->SetCurDir(NewDir,TRUE);
      return TRUE;
    }
    /* OT $ */

    if (CtrlObject->Plugins.ProcessCommandLine(NewDir))
    {
      CmdStr.SetString("");
      GotoXY(X1,Y1);
      mprintf("%*s",X2-X1+1,"");
      Show();
      return(TRUE);
    }

    char *ChPtr=strrchr(NewDir,'\"');
    if (ChPtr!=NULL)
      *ChPtr=0;
    if (SetPanel->GetType()==FILE_PANEL && SetPanel->GetMode()==PLUGIN_PANEL)
    {
      SetPanel->SetCurDir(NewDir,ChDir);
      return(TRUE);
    }
    char ExpandedDir[8192];
    if (ExpandEnvironmentStr(NewDir,ExpandedDir,sizeof(ExpandedDir))!=0)
      if (chdir(ExpandedDir)==-1)
        return(FALSE);
    SetPanel->ChangeDirToCurrent();
    if (!SetPanel->IsVisible())
      SetPanel->SetTitle();
    return(TRUE);
  }
  return(FALSE);
}
