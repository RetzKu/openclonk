/*
 * OpenClonk, http://www.openclonk.org
 *
 * Copyright (c) 2005-2009, RedWolf Design GmbH, http://www.clonk.de
 *
 * Portions might be copyrighted by other authors who have contributed
 * to OpenClonk.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * See isc_license.txt for full license and disclaimer.
 *
 * "Clonk" is a registered trademark of Matthes Bender.
 * See clonk_trademark_license.txt for full license.
 */
// Startup screen for non-parameterized engine start (stub)

#include <C4Include.h>
#include <C4StartupMainDlg.h>
#include <C4UpdateDlg.h>
#include <C4Version.h>

#ifndef BIG_C4INCLUDE
#include <C4StartupNetDlg.h>
#include <C4StartupScenSelDlg.h>
#include <C4StartupOptionsDlg.h>
#include <C4StartupAboutDlg.h>
#include <C4StartupPlrSelDlg.h>
#include <C4Startup.h>
#include <C4Game.h>
#include <C4Log.h>
#include <C4Language.h>
#endif


C4StartupMainDlg::C4StartupMainDlg() : C4StartupDlg(NULL) // create w/o title; it is drawn in custom draw proc
	{
	// ctor
	fFirstShown = true;
	// screen calculations
	int iButtonPadding = 2;
	int iButtonHeight = C4GUI_BigButtonHgt;
	C4GUI::ComponentAligner caMain(rcBounds, 0,0,true);
	C4GUI::ComponentAligner caRightPanel(caMain.GetFromLeft(rcBounds.Wdt*2/5), rcBounds.Wdt/26,40+rcBounds.Hgt/8);
	//C4GUI::ComponentAligner caButtons(caRightPanel.GetCentered(caRightPanel.GetWidth(), (iButtonHeight+iButtonPadding) * iButtonCount - iButtonPadding), 0, iButtonPadding);
	C4GUI::ComponentAligner caButtons(caRightPanel.GetAll(), 0, iButtonPadding);
	// main menu buttons
	C4GUI::CallbackButton<C4StartupMainDlg> *btn;
	AddElement(btn = new C4GUI::CallbackButton<C4StartupMainDlg>(LoadResStr("IDS_BTN_LOCALGAME"), caButtons.GetFromTop(iButtonHeight), &C4StartupMainDlg::OnStartBtn));
	btn->SetToolTip(LoadResStr("IDS_DLGTIP_STARTGAME"));
	btn->SetCustomGraphics(&C4Startup::Get()->Graphics.barMainButtons, &C4Startup::Get()->Graphics.barMainButtonsDown);
	pStartButton = btn;
	AddElement(btn = new C4GUI::CallbackButton<C4StartupMainDlg>(LoadResStr("IDS_BTN_NETWORKGAME"), caButtons.GetFromTop(iButtonHeight), &C4StartupMainDlg::OnNetJoinBtn));
	btn->SetToolTip(LoadResStr("IDS_DLGTIP_NETWORKGAME"));
	btn->SetCustomGraphics(&C4Startup::Get()->Graphics.barMainButtons, &C4Startup::Get()->Graphics.barMainButtonsDown);
	AddElement(btn = new C4GUI::CallbackButton<C4StartupMainDlg>(LoadResStr("IDS_DLG_PLAYERSELECTION"), caButtons.GetFromTop(iButtonHeight), &C4StartupMainDlg::OnPlayerSelectionBtn));
	btn->SetToolTip(LoadResStr("IDS_DLGTIP_PLAYERSELECTION"));
	btn->SetCustomGraphics(&C4Startup::Get()->Graphics.barMainButtons, &C4Startup::Get()->Graphics.barMainButtonsDown);
	AddElement(btn = new C4GUI::CallbackButton<C4StartupMainDlg>(LoadResStr("IDS_DLG_OPTIONS"), caButtons.GetFromTop(iButtonHeight), &C4StartupMainDlg::OnOptionsBtn));
	btn->SetToolTip(LoadResStr("IDS_DLGTIP_OPTIONS"));
	btn->SetCustomGraphics(&C4Startup::Get()->Graphics.barMainButtons, &C4Startup::Get()->Graphics.barMainButtonsDown);
	AddElement(btn = new C4GUI::CallbackButton<C4StartupMainDlg>(LoadResStr("IDS_DLG_ABOUT"), caButtons.GetFromTop(iButtonHeight), &C4StartupMainDlg::OnAboutBtn));
	btn->SetToolTip(LoadResStr("IDS_DLGTIP_ABOUT"));
	btn->SetCustomGraphics(&C4Startup::Get()->Graphics.barMainButtons, &C4Startup::Get()->Graphics.barMainButtonsDown);
	AddElement(btn = new C4GUI::CallbackButton<C4StartupMainDlg>(LoadResStr("IDS_DLG_EXIT"), caButtons.GetFromTop(iButtonHeight), &C4StartupMainDlg::OnExitBtn));
	btn->SetToolTip(LoadResStr("IDS_DLGTIP_EXIT"));
	btn->SetCustomGraphics(&C4Startup::Get()->Graphics.barMainButtons, &C4Startup::Get()->Graphics.barMainButtonsDown);
	// list of selected players
	AddElement(pParticipantsLbl = new C4GUI::Label("test", GetClientRect().Wdt*39/40, GetClientRect().Hgt*9/10, ARight, 0xffffffff, &C4GUI::GetRes()->TitleFont, false));
	pParticipantsLbl->SetToolTip(LoadResStr("IDS_DLGTIP_SELECTEDPLAYERS"));
	// player selection shortcut - to be made optional
	UpdateParticipants();
	pParticipantsLbl->SetContextHandler(new C4GUI::CBContextHandler<C4StartupMainDlg>(this, &C4StartupMainDlg::OnPlayerSelContext));
	/*new C4GUI::ContextButton(pParticipantsLbl, true, 0,0);*/
	// key bindings
	C4CustomKey::CodeList keys;
	keys.push_back(C4KeyCodeEx(K_DOWN)); keys.push_back(C4KeyCodeEx(K_RIGHT));
	if (Config.Controls.GamepadGuiControl)
		{
		keys.push_back(C4KeyCodeEx(KEY_Gamepad(0, KEY_JOY_Down))); // right will be done by Dialog already
		}
	pKeyDown = new C4KeyBinding(keys, "StartupMainCtrlNext", KEYSCOPE_Gui,
		new C4GUI::DlgKeyCBEx<C4StartupMainDlg, bool>(*this, false, &C4StartupMainDlg::KeyAdvanceFocus), C4CustomKey::PRIO_CtrlOverride);
	keys.clear(); keys.push_back(C4KeyCodeEx(K_UP)); keys.push_back(C4KeyCodeEx(K_LEFT));
	if (Config.Controls.GamepadGuiControl)
		{
		keys.push_back(C4KeyCodeEx(KEY_Gamepad(0, KEY_JOY_Up))); // left will be done by Dialog already
		}
	pKeyUp = new C4KeyBinding(keys, "StartupMainCtrlPrev", KEYSCOPE_Gui,
		new C4GUI::DlgKeyCBEx<C4StartupMainDlg, bool>(*this, true, &C4StartupMainDlg::KeyAdvanceFocus), C4CustomKey::PRIO_CtrlOverride);
	keys.clear(); keys.push_back(C4KeyCodeEx(K_RETURN));
	pKeyEnter = new C4KeyBinding(keys, "StartupMainOK", KEYSCOPE_Gui,
		new C4GUI::DlgKeyCB<C4StartupMainDlg>(*this, &C4StartupMainDlg::KeyEnterDown, &C4StartupMainDlg::KeyEnterUp), C4CustomKey::PRIO_CtrlOverride);
	keys.clear(); keys.push_back(C4KeyCodeEx(K_F6));
	pKeyEditor = new C4KeyBinding(keys, "StartupMainEditor", KEYSCOPE_Gui,
		new C4GUI::DlgKeyCB<C4StartupMainDlg>(*this, &C4StartupMainDlg::SwitchToEditor, false), C4CustomKey::PRIO_CtrlOverride);
	}

C4StartupMainDlg::~C4StartupMainDlg()
	{
	// dtor
	delete pKeyEnter;
	delete pKeyUp;
	delete pKeyDown;
	delete pKeyEditor;
	}

void C4StartupMainDlg::DrawElement(C4TargetFacet &cgo)
	{
	// inherited
	typedef C4GUI::FullscreenDialog Base;
	Base::DrawElement(cgo);
	// draw logo
	C4Facet &fctLogo = Game.GraphicsResource.fctLogo;
	float fLogoZoom = 1.0f;
	fctLogo.DrawX(cgo.Surface, rcBounds.Wdt *1/21, rcBounds.Hgt/14 - 5, int32_t(fLogoZoom*fctLogo.Wdt), int32_t(fLogoZoom*fctLogo.Hgt));
	// draw version info
	StdStrBuf sVer;
	sVer.Format(LoadResStr("IDS_DLG_VERSION"), C4VERSION);
	if (!Config.Registered())
		{ sVer += " <c ff0000>["; sVer += LoadResStr("IDS_CTL_UNREGISTERED"); sVer += "]</c>"; }
	lpDDraw->TextOut(sVer.getData(), C4GUI::GetRes()->TextFont, 1.0f, cgo.Surface, rcBounds.Wdt*1/40, rcBounds.Hgt/12 + int32_t(fLogoZoom*fctLogo.Hgt) - 10, 0xffffffff, ALeft, true);
	}

C4GUI::ContextMenu *C4StartupMainDlg::OnPlayerSelContext(C4GUI::Element *pBtn, int32_t iX, int32_t iY)
	{
	// preliminary player selection via simple context menu
	C4GUI::ContextMenu *pCtx = new C4GUI::ContextMenu();
	pCtx->AddItem("Add", "Add participant", C4GUI::Ico_None, NULL, new C4GUI::CBContextHandler<C4StartupMainDlg>(this, &C4StartupMainDlg::OnPlayerSelContextAdd));
	pCtx->AddItem("Remove", "Remove participant", C4GUI::Ico_None, NULL, new C4GUI::CBContextHandler<C4StartupMainDlg>(this, &C4StartupMainDlg::OnPlayerSelContextRemove));
	return pCtx;
	}

C4GUI::ContextMenu *C4StartupMainDlg::OnPlayerSelContextAdd(C4GUI::Element *pBtn, int32_t iX, int32_t iY)
	{
	C4GUI::ContextMenu *pCtx = new C4GUI::ContextMenu();
	const char *szFn;
	StdStrBuf sSearchPath(Config.General.UserDataPath);
//	sSearchPath.Format("%s%s", (const char *) Config.General.ExePath, (const char *) Config.General.PlayerPath);
	for (DirectoryIterator i(sSearchPath.getData()); szFn=*i; i++)
		{
		szFn = Config.AtRelativePath(szFn);
		if (*GetFilename(szFn) == '.') continue;
		if (!WildcardMatch(C4CFN_PlayerFiles, GetFilename(szFn))) continue;
		if (!SIsModule(Config.General.Participants, szFn, NULL, false))
			pCtx->AddItem(C4Language::IconvClonk(GetFilenameOnly(szFn)).getData(), "Let this player join in next game", C4GUI::Ico_Player,
				new C4GUI::CBMenuHandlerEx<C4StartupMainDlg, StdCopyStrBuf>(this, &C4StartupMainDlg::OnPlayerSelContextAddPlr, StdCopyStrBuf(szFn)), NULL);
		}
	return pCtx;
	}

C4GUI::ContextMenu *C4StartupMainDlg::OnPlayerSelContextRemove(C4GUI::Element *pBtn, int32_t iX, int32_t iY)
	{
	C4GUI::ContextMenu *pCtx = new C4GUI::ContextMenu();
	char szPlayer[1024+1];
	for (int i = 0; SCopySegment(Config.General.Participants, i, szPlayer, ';', 1024, true); i++)
		if (*szPlayer)
			pCtx->AddItem(GetFilenameOnly(szPlayer), "Remove this player from participation list", C4GUI::Ico_Player, new C4GUI::CBMenuHandlerEx<C4StartupMainDlg, int>(this, &C4StartupMainDlg::OnPlayerSelContextRemovePlr, i), NULL);
	return pCtx;
	}

void C4StartupMainDlg::OnPlayerSelContextAddPlr(C4GUI::Element *pTarget, const StdCopyStrBuf &rsFilename)
	{
	SAddModule(Config.General.Participants, rsFilename.getData());
	UpdateParticipants();
	}

void C4StartupMainDlg::OnPlayerSelContextRemovePlr(C4GUI::Element *pTarget, const int &iIndex)
	{
	char szPlayer[1024+1];
	if (SCopySegment(Config.General.Participants, iIndex, szPlayer, ';', 1024, true))
		SRemoveModule(Config.General.Participants, szPlayer, false);
	UpdateParticipants();
	}

void C4StartupMainDlg::UpdateParticipants()
	{
	// First validate all participants (files must exist)
	std::string strPlayers(Config.General.Participants);
	std::string strPlayer;
	strPlayer.reserve(1025);
	*Config.General.Participants=0;
	for (int i = 0; SCopySegment(strPlayers.c_str(), i, &strPlayer[0], ';', 1024, true); i++)
		{
		const char *szPlayer = strPlayer.c_str();
		std::string strPlayerFile(Config.General.UserDataPath);
		strPlayerFile.append(szPlayer);
		if (!szPlayer || !*szPlayer) continue;
		if (!FileExists(strPlayerFile.c_str())) continue;
		if (!SEqualNoCase(GetExtension(szPlayer), "c4p")) continue; // additional sanity check to clear strange exe-path-only entries in player list?
		SAddModule(Config.General.Participants, szPlayer);
		}
	// Draw selected players - we are currently displaying the players stored in Config.General.Participants.
	// Existence of the player files is not validated and player filenames are displayed directly
	// (names are not loaded from the player core).
	strPlayers = LoadResStr("IDS_DESC_PLRS");
	if (!Config.General.Participants[0])
		strPlayers.append(LoadResStr("IDS_DLG_NOPLAYERSSELECTED"));
	else
		for (int i = 0; SCopySegment(Config.General.Participants, i, &strPlayer[0], ';', 1024, true); i++)
			{
			if (i > 0) strPlayers.append(", ");
			strPlayers.append(C4Language::IconvClonk(GetFilenameOnly(strPlayer.c_str())).getData());
			}
	pParticipantsLbl->SetText(strPlayers.c_str());
	}

void C4StartupMainDlg::OnClosed(bool fOK)
	{
	// if dlg got aborted (by user), quit startup
	// if it got closed with OK, the user pressed one of the buttons and dialog switching has been done already
	if (!fOK) C4Startup::Get()->Exit();
	}

void C4StartupMainDlg::OnStartBtn(C4GUI::Control *btn)
	{
	// no regular game start if no players were selected
	/*if (!*Config.General.Participants)
		{
		StdStrBuf buf(LoadResStrNoAmp("IDS_DLG_STARTGAME"), true);
		GetScreen()->ShowMessageModal(LoadResStr("IDS_MSG_NOPLAYERSELECTED"), buf.getData(), C4GUI::MessageDialog::btnOK, C4GUI::Ico_Notify);
		// let's go to the player selection dlg then instead
		OnPlayerSelectionBtn(NULL);
		return;
		}*/
	// advance to scenario selection screen
	C4Startup::Get()->SwitchDialog(C4Startup::SDID_ScenSel);
	}

void C4StartupMainDlg::OnPlayerSelectionBtn(C4GUI::Control *btn)
	{
	// advance to player selection screen
	C4Startup::Get()->SwitchDialog(C4Startup::SDID_PlrSel);
	}

void C4StartupMainDlg::OnNetJoinBtn(C4GUI::Control *btn)
	{
	// simple net join dlg
	//GetScreen()->ShowDialog(new C4GUI::InputDialog("Enter host IP", "Direct join", C4GUI::Ico_Host, new C4GUI::InputCallback<C4StartupMainDlg>(this, &C4StartupMainDlg::OnNetJoin)), false);
	// advanced net join and host dlg!
	C4Startup::Get()->SwitchDialog(C4Startup::SDID_NetJoin);
	}

void C4StartupMainDlg::OnNetJoin(const StdStrBuf &rsHostAddress)
	{
	// no IP given: No join
	if (!rsHostAddress || !*rsHostAddress.getData()) return;
	// set default startup parameters
	*Game.ScenarioFilename=0;
	SCopy("Objects.c4d", Game.DefinitionFilenames);
	Game.NetworkActive = TRUE;
	Game.fLobby = TRUE;
	Game.fObserve = FALSE;
	SCopy(rsHostAddress.getData(), Game.DirectJoinAddress, sizeof(Game.DirectJoinAddress)-1);
	// start with this set!
	C4Startup::Get()->Start();
	}

void C4StartupMainDlg::OnOptionsBtn(C4GUI::Control *btn)
	{
	// advance to options screen
	C4Startup::Get()->SwitchDialog(C4Startup::SDID_Options);
	}

void C4StartupMainDlg::OnAboutBtn(C4GUI::Control *btn)
	{
	// advance to about screen
	C4Startup::Get()->SwitchDialog(C4Startup::SDID_About);
	}

void C4StartupMainDlg::OnExitBtn(C4GUI::Control *btn)
	{
	// callback: exit button pressed
	C4Startup::Get()->Exit();
	}

void C4StartupMainDlg::OnTODO(C4GUI::Control *btn)
	{
	GetScreen()->ShowMessage("not yet implemented", "2do", C4GUI::Ico_Error);
	}

bool C4StartupMainDlg::KeyEnterDown()
	{
	// just execute selected button: Re-Send as space
	return Game.DoKeyboardInput(K_SPACE, KEYEV_Down, false, false, false, false, this);
	}

bool C4StartupMainDlg::KeyEnterUp()
	{
	// just execute selected button: Re-Send as space
	return Game.DoKeyboardInput(K_SPACE, KEYEV_Up, false, false, false, false, this);
	}

void C4StartupMainDlg::OnShown()
	{

	// New valid registration key found (in working directory or key path)
	if (Config.Registered() && !Config.Security.WasRegistered)
		HandleIncomingKeyfile(Config.GetKeyFilename());
	// Incoming key file from command line
	else if (Application.IncomingKeyfile)
		HandleIncomingKeyfile(Application.IncomingKeyfile.getData());
	// An invalid key file was found: handle it to show warning message
	else if (!Config.Registered() && *Config.GetInvalidKeyFilename())
		HandleIncomingKeyfile(Config.GetInvalidKeyFilename());
	// Clear incoming key info
	Application.IncomingKeyfile.Clear();
	// Update registration status
	Config.Security.WasRegistered = Config.Registered();

	// Incoming update
	if (Application.IncomingUpdate)
		{
		C4UpdateDlg::ApplyUpdate(Application.IncomingUpdate.getData(), false, GetScreen());
		Application.IncomingUpdate.Clear();
		}
	// Manual update by command line or url
	if (Application.CheckForUpdates)
		{
		C4UpdateDlg::CheckForUpdates(GetScreen(), false);
		Application.CheckForUpdates = false;
		}
	// Automatic update
	else
		{
		if (Config.Network.AutomaticUpdate)
			C4UpdateDlg::CheckForUpdates(GetScreen(), true);
		}

	// first start evaluation
	if (Config.General.FirstStart)
		{
		Config.General.FirstStart = false;
		}
	// first thing that's needed is a new player, if there's none - independent of first start
	bool fHasPlayer = false;
	StdStrBuf sSearchPath(Config.General.UserDataPath);
	const char *szFn;
//	sSearchPath.Format("%s%s", (const char *) Config.General.ExePath, (const char *) Config.General.PlayerPath);
	for (DirectoryIterator i(sSearchPath.getData()); szFn=*i; i++)
		{
		szFn = Config.AtRelativePath(szFn);
		if (*GetFilename(szFn) == '.') continue; // ignore ".", ".." and private files (".*")
		if (!WildcardMatch(C4CFN_PlayerFiles, GetFilename(szFn))) continue;
		fHasPlayer = true;
		break;
		}
	if (!fHasPlayer)
		{
		// no player created yet: Create one
		C4GUI::Dialog *pDlg;
		GetScreen()->ShowModalDlg(pDlg=new C4StartupPlrPropertiesDlg(NULL, NULL), true);
		}
	// make sure participants are updated after switching back from player selection
	UpdateParticipants();

	// First show
	if (fFirstShown)
		{
		// Activate the application (trying to prevent flickering half-focus in win32...)
		Application.Activate();
		// Set the focus to the start button (we might still not have the focus after the update-check sometimes... :/)
		SetFocus(pStartButton, false);
		}
	fFirstShown = false;
	}

bool C4StartupMainDlg::SwitchToEditor()
{

#ifdef _WIN32
	// No editor executable available
	if (!FileExists(Config.AtExePath(C4CFN_Editor))) return false;
	// Flag editor launch
	Application.launchEditor = true;
	// Quit
	C4Startup::Get()->Exit();
#endif

	return true;
}

void C4StartupMainDlg::HandleIncomingKeyfile(const char *strIncomingKey)
{
	static bool fWarnedInvalidKeyfile = false;

	// Copy filename parameter to keep it valid
	StdCopyStrBuf strKeyFilename; strKeyFilename = strIncomingKey;

	// Key file doesn't even exist? Do nothing.
	if (!FileExists(strKeyFilename.getData())) return;

	// Try loading registration from the incoming keyfile to verify it
	Config.ClearRegistrationError();
	if (!Config.LoadRegistration(strKeyFilename.getData()))
	{
		// Invalid keyfile: show message (but only once)
		if (!fWarnedInvalidKeyfile)
		{
			StdStrBuf strMessage; strMessage.Format(LoadResStr("IDS_MSG_INVALIDKEY"), Config.GetRegistrationError());
			GetScreen()->ShowMessageModal(strMessage.getData(), LoadResStr("IDS_DLG_REGISTRATION"),	C4GUI::MessageDialog::btnOK, C4GUI::Ico_Error);
		}
		fWarnedInvalidKeyfile = true;
		// Try to reload any existing registration
		Config.LoadRegistration();
		// Bail out
		return;
	}

	// See where the key is coming from
	char strIncomingPath[_MAX_PATH + 1];
	GetParentPath(strKeyFilename.getData(), strIncomingPath);
	AppendBackslash(strIncomingPath);

	// The key is already in the key path: that's how we like it
	if (SEqualNoCase(strIncomingPath, Config.GetKeyPath()))
		{
		// Just say thank you
		GetScreen()->ShowMessageModal(LoadResStr("IDS_CTL_REGISTERED"), LoadResStr("IDS_DLG_REGISTRATION"),	C4GUI::MessageDialog::btnOK, C4GUI::Ico_Notify);
		return;
		}

	// The key is currently in the work dir (ExePath)
	if (SEqualNoCase(strIncomingPath, Config.General.ExePath))
		{
		// Ask whether to move it to the key path (preferred)
		StdStrBuf strMessage; strMessage.Format("%s||%s", LoadResStr("IDS_CTL_REGISTERED"), LoadResStr("IDS_MSG_MOVEKEY"));
		if (GetScreen()->ShowMessageModal(strMessage.getData(), LoadResStr("IDS_DLG_REGISTRATION"),	C4GUI::MessageDialog::btnYesNo, C4GUI::Ico_Confirm))
			{
			// Create key path if it doesn't already exist
			if (!DirectoryExists(Config.GetKeyPath())) CreateDirectory(Config.GetKeyPath(), NULL);
			// Move key into key path
			StdStrBuf strTarget; strTarget.Format("%s%s", Config.GetKeyPath(), GetFilename(strKeyFilename.getData()));
			if (!MoveItem(strKeyFilename.getData(), strTarget.getData()))
				GetScreen()->ShowMessageModal(LoadResStr("IDS_FAIL_MOVE"), LoadResStr("IDS_DLG_REGISTRATION"),	C4GUI::MessageDialog::btnOK, C4GUI::Ico_Error);
			// Update registration
			Config.LoadRegistration();
			}
		return;
		}

	// The key is coming from the outside
	else
		{
		// Say thank you
		GetScreen()->ShowMessageModal(LoadResStr("IDS_CTL_REGISTERED"), LoadResStr("IDS_DLG_REGISTRATION"),	C4GUI::MessageDialog::btnOK, C4GUI::Ico_Notify);
		// Create key path if it doesn't already exist
		if (!DirectoryExists(Config.GetKeyPath())) CreateDirectory(Config.GetKeyPath(), NULL);
		// Now try to copy it into the key path (preferred)
		StdStrBuf strTarget; strTarget.Format("%s%s", Config.GetKeyPath(), GetFilename(strKeyFilename.getData()));
		if (!CopyItem(strKeyFilename.getData(), strTarget.getData()))
			{
			// Failed for some reason, try copy it into the work dir instead (emergency backup)
			strTarget.Format("%s%s", Config.General.ExePath, GetFilename(strKeyFilename.getData()));
			if (!CopyItem(strKeyFilename.getData(), strTarget.getData()))
				GetScreen()->ShowMessageModal(LoadResStr("IDS_FAIL_COPY"), LoadResStr("IDS_DLG_REGISTRATION"),	C4GUI::MessageDialog::btnOK, C4GUI::Ico_Error);
			}
		// Update registration
		Config.LoadRegistration();
		return;
		}

}
