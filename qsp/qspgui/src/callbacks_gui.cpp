// Copyright (C) 2005-2008 Valeriy Argunov (nporep AT mail DOT ru)
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "callbacks_gui.h"

wxString QSPCallBacks::m_gamePath;
QSPFrame *QSPCallBacks::m_frame;
bool QSPCallBacks::m_isHtml;
FMOD_SYSTEM *QSPCallBacks::m_sys;
QSPSounds QSPCallBacks::m_sounds;

void QSPCallBacks::Init(QSPFrame *frame)
{
	m_frame = frame;

	FMOD_System_Create(&m_sys);
	FMOD_System_SetPluginPath(m_sys, "sound");
	FMOD_System_SetOutputByPlugin(m_sys, 0);
	FMOD_System_Init(m_sys, 32, FMOD_INIT_NORMAL, 0);

	QSPSetCallBack(QSP_CALL_SETTIMER, (QSP_CALLBACK)&SetTimer);
	QSPSetCallBack(QSP_CALL_REFRESHINT, (QSP_CALLBACK)&RefreshInt);
	QSPSetCallBack(QSP_CALL_SETINPUTSTRTEXT, (QSP_CALLBACK)&SetInputStrText);
	QSPSetCallBack(QSP_CALL_ISPLAYINGFILE, (QSP_CALLBACK)&IsPlay);
	QSPSetCallBack(QSP_CALL_PLAYFILE, (QSP_CALLBACK)&PlayFile);
	QSPSetCallBack(QSP_CALL_CLOSEFILE, (QSP_CALLBACK)&CloseFile);
	QSPSetCallBack(QSP_CALL_SHOWMSGSTR, (QSP_CALLBACK)&Msg);
	QSPSetCallBack(QSP_CALL_SLEEP, (QSP_CALLBACK)&Sleep);
	QSPSetCallBack(QSP_CALL_GETMSCOUNT, (QSP_CALLBACK)&GetMSCount);
	QSPSetCallBack(QSP_CALL_DELETEMENU, (QSP_CALLBACK)&DeleteMenu);
	QSPSetCallBack(QSP_CALL_ADDMENUITEM, (QSP_CALLBACK)&AddMenuItem);
	QSPSetCallBack(QSP_CALL_SHOWMENU, (QSP_CALLBACK)&ShowMenu);
	QSPSetCallBack(QSP_CALL_INPUTBOX, (QSP_CALLBACK)&Input);
	QSPSetCallBack(QSP_CALL_SHOWIMAGE, (QSP_CALLBACK)&ShowImage);
	QSPSetCallBack(QSP_CALL_SHOWWINDOW, (QSP_CALLBACK)&ShowPane);
	QSPSetCallBack(QSP_CALL_OPENGAMESTATUS, (QSP_CALLBACK)&OpenGameStatus);
	QSPSetCallBack(QSP_CALL_SAVEGAMESTATUS, (QSP_CALLBACK)&SaveGameStatus);
}

void QSPCallBacks::DeInit()
{
	FMOD_System_Close(m_sys);
	FMOD_System_Release(m_sys);
}

bool QSPCallBacks::GetVarValue(QSP_CHAR *name, long *num, QSP_CHAR **str)
{
	if (QSPGetVarValuesCount(name, num) && *num)
	{
		QSPGetVarValues(name, 0, num, str);
		return true;
	}
	return false;
}

void QSPCallBacks::SetTimer(long msecs)
{
	if (msecs)
		m_frame->GetTimer()->Start(msecs);
	else
		m_frame->GetTimer()->Stop();
}

void QSPCallBacks::RefreshInt(QSP_BOOL isRedraw)
{
	static long oldFullRefreshCount = 0;
	long i, numVal;
	bool isScroll, isCanSave;
	QSP_CHAR *strVal, *imgPath;
	// -------------------------------
	UpdateGamePath();
	// -------------------------------
	QSP_CHAR *mainDesc = QSPGetMainDesc();
	QSP_CHAR *varsDesc = QSPGetVarsDesc();
	// -------------------------------
	isScroll = !(GetVarValue(QSP_FMT("DISABLESCROLL"), &numVal, &strVal) && numVal);
	isCanSave = !(GetVarValue(QSP_FMT("NOSAVE"), &numVal, &strVal) && numVal);
	m_isHtml = GetVarValue(QSP_FMT("USEHTML"), &numVal, &strVal) && numVal;
	// -------------------------------
	m_frame->GetVars()->SetIsHtml(m_isHtml, isScroll);
	if (QSPIsVarsDescChanged())
		m_frame->GetVars()->SetText(wxString(varsDesc), isScroll);
	// -------------------------------
	long fullRefreshCount = QSPGetFullRefreshCount();
	if (oldFullRefreshCount != fullRefreshCount)
	{
		isScroll = false;
		oldFullRefreshCount = fullRefreshCount;
	}
	m_frame->GetDesc()->SetIsHtml(m_isHtml, isScroll);
	if (QSPIsMainDescChanged())
		m_frame->GetDesc()->SetText(wxString(mainDesc), isScroll);
	// -------------------------------
	m_frame->GetActions()->SetIsHtml(m_isHtml);
	m_frame->GetActions()->SetIsShowNums(m_frame->GetIsShowHotkeys());
	if (QSPIsActionsChanged())
	{
		long actionsCount = QSPGetActionsCount();
		m_frame->GetActions()->BeginItems();
		for (i = 0; i < actionsCount; ++i)
		{
			QSPGetActionData(i, &imgPath, &strVal);
			m_frame->GetActions()->AddItem(wxString(imgPath), wxString(strVal));
		}
		m_frame->GetActions()->EndItems();
	}
	m_frame->GetActions()->SetSelection(QSPGetSelActionIndex());
	m_frame->GetObjects()->SetIsHtml(m_isHtml);
	if (QSPIsObjectsChanged())
	{
		long objectsCount = QSPGetObjectsCount();
		m_frame->GetObjects()->BeginItems();
		for (i = 0; i < objectsCount; ++i)
		{
			QSPGetObjectData(i, &imgPath, &strVal);
			m_frame->GetObjects()->AddItem(wxString(imgPath), wxString(strVal));
		}
		m_frame->GetObjects()->EndItems();
	}
	m_frame->GetObjects()->SetSelection(QSPGetSelObjectIndex());
	// -------------------------------
	m_frame->ShowPane(ID_BACKPIC,
		GetVarValue(QSP_FMT("BACKIMAGE"), &numVal, &strVal) &&
		strVal && *strVal && m_frame->GetImgBack()->OpenFile(m_gamePath + strVal)
	);
	// -------------------------------
	m_frame->ApplyParams();
	if (isRedraw)
	{
		m_frame->EnableControls(false, true);
		wxTheApp->Yield(true);
		m_frame->EnableControls(true, true);
	}
	m_frame->GetGameMenu()->Enable(ID_SAVEGAMESTAT, isCanSave);
}

void QSPCallBacks::SetInputStrText(QSP_CHAR *text)
{
	m_frame->GetInput()->SetText(wxString(text));
}

QSP_BOOL QSPCallBacks::IsPlay(QSP_CHAR *file)
{
	FMOD_CHANNEL *channel;
	FMOD_BOOL playing = FALSE;
	wxString strFile(file);
	strFile.Replace(wxT("\\"), wxT("/"));
	QSPSounds::iterator elem = m_sounds.find(strFile.Upper());
	if (elem != m_sounds.end())
	{
		channel = ((QSPSound)(elem->second)).Channel;
		FMOD_Channel_IsPlaying(channel, &playing);
	}
	return (playing == TRUE);
}

void QSPCallBacks::CloseFile(QSP_CHAR *file)
{
	if (file)
	{
		wxString strFile(file);
		strFile.Replace(wxT("\\"), wxT("/"));
		QSPSounds::iterator elem = m_sounds.find(strFile.Upper());
		if (elem != m_sounds.end())
		{
			((QSPSound)(elem->second)).Free();
			m_sounds.erase(elem);
		}
	}
	else
	{
		for (QSPSounds::iterator i = m_sounds.begin(); i != m_sounds.end(); ++i)
			((QSPSound)(i->second)).Free();
		m_sounds.clear();
	}
}

void QSPCallBacks::PlayFile(QSP_CHAR *file, long volume)
{
	FMOD_SOUND *newSound;
	FMOD_CHANNEL *newChannel;
	QSPSound snd;
	if (SetVolume(file, volume)) return;
	CloseFile(file);
	wxString strFile(file);
	strFile.Replace(wxT("\\"), wxT("/"));
	if (!FMOD_System_CreateSound(m_sys, wxConvFile.cWX2MB(strFile.wx_str()), FMOD_SOFTWARE | FMOD_CREATESTREAM, 0, &newSound))
	{
		FMOD_System_PlaySound(m_sys, FMOD_CHANNEL_FREE, newSound, FALSE, &newChannel);
		snd.Channel = newChannel;
		snd.Sound = newSound;
		m_sounds.insert(QSPSounds::value_type(strFile.Upper(), snd));
		SetVolume(file, volume);
	}
}

void QSPCallBacks::ShowPane(long type, QSP_BOOL isShow)
{
	switch (type)
	{
	case QSP_WIN_ACTS:
		m_frame->ShowPane(ID_ACTIONS, isShow != QSP_FALSE);
		break;
	case QSP_WIN_OBJS:
		m_frame->ShowPane(ID_OBJECTS, isShow != QSP_FALSE);
		break;
	case QSP_WIN_VARS:
		m_frame->ShowPane(ID_VARSDESC, isShow != QSP_FALSE);
		break;
	case QSP_WIN_INPUT:
		m_frame->ShowPane(ID_INPUT, isShow != QSP_FALSE);
		break;
	}
}

void QSPCallBacks::Sleep(long msecs)
{
	if (m_frame->GetIsQuit()) return;
	bool isSave = m_frame->GetGameMenu()->IsEnabled(ID_SAVEGAMESTAT);
	m_frame->EnableControls(false, true);
	long i, count = msecs / 50, rem = msecs % 50;
	for (i = 0; i < count; ++i)
	{
		wxThread::Sleep(50);
		wxTheApp->Yield(true);
		if (m_frame->GetIsQuit()) break;
	}
	if (!m_frame->GetIsQuit())
	{
		wxThread::Sleep(rem);
		wxTheApp->Yield(true);
	}
	m_frame->EnableControls(true, true);
	m_frame->GetGameMenu()->Enable(ID_SAVEGAMESTAT, isSave);
}

long QSPCallBacks::GetMSCount()
{
	static wxStopWatch stopWatch;
	long ret = stopWatch.Time();
	stopWatch.Start();
	return ret;
}

void QSPCallBacks::Msg(QSP_CHAR *str)
{
	bool oldIsProcessEvents;
	wxMessageDialog msgDialog(m_frame, wxString(str), _("Info"), wxOK | wxICON_INFORMATION);
	oldIsProcessEvents = m_frame->GetIsProcessEvents();
	m_frame->SetIsProcessEvents(false);
	msgDialog.ShowModal();
	m_frame->SetIsProcessEvents(oldIsProcessEvents);
}

void QSPCallBacks::DeleteMenu()
{
	m_frame->DeleteMenu();
}

void QSPCallBacks::AddMenuItem(QSP_CHAR *name, QSP_CHAR *imgPath)
{
	m_frame->AddMenuItem(wxString(name), wxString(imgPath));
}

void QSPCallBacks::ShowMenu()
{
	m_frame->ShowMenu();
}

void QSPCallBacks::Input(QSP_CHAR *text, QSP_CHAR *buffer, long maxLen)
{
	bool oldIsProcessEvents;
	RefreshInt(QSP_FALSE);
	QSPInputDlg *inputDlg = new QSPInputDlg(m_frame,
		wxID_ANY,
		m_frame->GetDesc()->GetBackgroundColour(),
		m_frame->GetDesc()->GetForegroundColour(),
		m_frame->GetDesc()->GetTextFont(),
		_("Input data"),
		wxString(text),
		m_isHtml,
		m_gamePath
	);
	oldIsProcessEvents = m_frame->GetIsProcessEvents();
	m_frame->SetIsProcessEvents(false);
	inputDlg->ShowModal();
	m_frame->SetIsProcessEvents(oldIsProcessEvents);
	#ifdef _UNICODE
		wcsncpy(buffer, inputDlg->GetText().wx_str(), maxLen);
	#else
		strncpy(buffer, inputDlg->GetText().wx_str(), maxLen);
	#endif
	buffer[maxLen] = 0;
	inputDlg->Destroy();
}

void QSPCallBacks::ShowImage(QSP_CHAR *file)
{
	m_frame->ShowPane(ID_VIEWPIC, m_frame->GetImgView()->OpenFile(wxString(file)));
}

void QSPCallBacks::OpenGameStatus()
{
	wxFileDialog dialog(m_frame, _("Select saved game file"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_OPEN);
	if (dialog.ShowModal() == wxID_OK)
		QSPOpenSavedGame((QSP_CHAR *)dialog.GetPath().wx_str(), QSP_FALSE);
}

void QSPCallBacks::SaveGameStatus()
{
	wxFileDialog dialog(m_frame, _("Select file to save"), wxEmptyString, wxEmptyString, _("Saved game files (*.sav)|*.sav"), wxFD_SAVE);
	if (dialog.ShowModal() == wxID_OK)
		QSPSaveGame((QSP_CHAR *)dialog.GetPath().wx_str(), QSP_FALSE);
}

void QSPCallBacks::UpdateGamePath()
{
	wxFileName file(QSPGetQstFullPath());
	m_gamePath = file.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	m_frame->GetDesc()->SetGamePath(m_gamePath);
	m_frame->GetObjects()->SetGamePath(m_gamePath);
	m_frame->GetActions()->SetGamePath(m_gamePath);
	m_frame->GetVars()->SetGamePath(m_gamePath);
}

bool QSPCallBacks::SetVolume(QSP_CHAR *file, long volume)
{
	FMOD_CHANNEL *channel;
	if (!IsPlay(file)) return false;
	wxString strFile(file);
	strFile.Replace(wxT("\\"), wxT("/"));
	QSPSounds::iterator elem = m_sounds.find(strFile.Upper());
	channel = ((QSPSound)(elem->second)).Channel;
	FMOD_Channel_SetVolume(channel, (float)volume / 100);
	return true;
}
