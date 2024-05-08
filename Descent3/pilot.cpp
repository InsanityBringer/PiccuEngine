/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "pilot.h"
#include "mono.h"
#include "renderer.h"
#include "render.h"
#include "ddio.h"
#include "descent.h"
#include "game.h"
#include "CFILE.H"
#include "application.h"
#include "manage.h"
#include "newui.h"
#include "grtext.h"
#include "gamefont.h"
#include "ConfigItem.h"
#include "ctlconfig.h"
#include "hud.h"
#include "stringtable.h"
#include "gametexture.h"
#include "vclip.h"
#include "hlsoundlib.h"
#include "weapon.h"
#include "config.h"
#include "difficulty.h"
#include "PilotPicsAPI.h"
#include "pstring.h"
#include "Mission.h"
#include "mem.h"
#include "polymodel.h"
#include "audiotaunts.h"
#include "streamaudio.h"
#include "ship.h"
#include "dedicated_server.h"
#include "init.h"

//some general defines
#define	IDP_SAVE	10
#define	IDP_CANCEL	11
#define	IDP_APPLY	12

//used for the Pilot file functions
#define PLTWILDCARD	"*.plt"
#define PLTEXTENSION ".plt"
#define DPLTWILDCARD "*.pld"
#define PLTFILELEN	260

#define MAX_AUDIOTAUNTSIZE (32*1024)

// !!!!!(PLEASE VERSION CHECK ON READ TO MAINTAIN PILOT COMPAT.)!!!!
// 0x20	NEW PILOT FILE READ->WRITE
// 0x12 ????
// 0x11 added finished field to mission data -Samir
//  0x10 added mission data - Jeff
//	0xf write out multiple joystick and mouse sensitivities.-Samir
// 0xe write out number of controller functions in file-Samir
// 0xd added pilot picture id
// 0xc added mouse and joy sensitivities -Samir.
// 0xb added invertible axes - Samir
// 0xa Added game window size-Samir
// 0x9 Added weapon selection info -Samir 
//	0x8	Added audio taunt files - Jeffrey
//  0x7 Added ship logo - Jeff
//	0x6 Added default hud mode- Samir
// 0x5 Added hud layout info- Samir
//	0x4 Added read controller flag- Samir.
#define PLTFILEVERSION 0x12

//border size of the UI window
#define UI_BORDERSIZE	20

///////////////////////////////////////////////
// Externals (Globals for the game)

//Game global for current pilot
pilot Current_pilot;
char Default_pilot[_MAX_PATH] = {" "};
ubyte ingame_difficulty = 1;


///////////////////////////////////////////////
// Internals (Globals for the file)

bool DisplayFileDialog(char *path,char *title,char *wildcards,int flags);

//internal function prototypes
bool PltDelete(pilot *Pilot);
void NewPltUpdate(newuiListBox *list,char **flist,int filecount,int selected,char *filename=nullptr, bool presets=false);
bool PilotChoose(pilot *Pilot,bool presets=false);
bool PltCopyKeyConfig(pilot *src,pilot *dest);
bool PltSelectShip(pilot *Pilot);
void ShipSelectCallBack(int c);
void CustomCallBack(int c);
void PilotCopyDefaultControls(pilot *Pilot);

int Pilot_NewRead(pilot *Pilot,bool read_keyconfig,bool read_missiondata);
int Pilot_NewWrite(pilot *Pilot,bool newpilot);

struct tCustomListInfo
{
	newuiListBox *custom_bitmap_list;
	int needed_size;			//size of allocated memory for files
	char *files;				//string list of file names
	// Initializes the struct
	void Init(){
		files = NULL;
		custom_bitmap_list = NULL;
		needed_size = 0;
	}
	// Frees and resets the struct
	void Reset(){
		if(files){
			mem_free(files);
			files = NULL;
		}
		custom_bitmap_list = NULL;
		needed_size = 0;
	}
};

struct tAudioTauntComboBoxes
{
	newuiComboBox *taunt_a,*taunt_b,*taunt_c,*taunt_d;
};

// Deletes the currently selected audio taunt #4
void ShipSelectDeleteTaunt(pilot *Pilot,tCustomListInfo *cust_snds,newuiComboBox *lb,tAudioTauntComboBoxes *taunt_boxex);

// Deletes the currently selected ship logo
void ShipSelectDeleteLogo(tCustomListInfo *cust_bmps,newuiListBox *lb);

// -------------------------------------------------------
// ShowPilotPicDialog
//	Purpose:
//		Displays the dialog to choose a pilot picture for
//	the given pilot.
// -------------------------------------------------------
void ShowPilotPicDialog(pilot *Pilot);


UITextItem *pilot_items=NULL;			//array of UITextItems for use in Pilot listbox
pilot temp;								//pilot in use by the listbox
NewUIGameWindow *PilotDisplayWindow;	//pointer to display_window (needed for listbox callback)
char **filelist;						//list of pilot filenames
int filecount;							//number of pilot filenames found
void PilotListSelectChangeCallback(int index);

////////////////////////////////////////////////////////////////////////////
// Dialog Functions
////////////////////////////////////////////////////////////////////////////
void PilotInitData(pilot *plt);

void PilotShutdown(void)
{
	Current_pilot.flush(false);
	Current_pilot.clean(false);
}

void PilotInit(void)
{
	PilotInitData(&Current_pilot);
	atexit(PilotShutdown);
}

void PilotInitData(pilot *plt)
{
	if(!plt)
	{
		Int3();		
	}else
	{
		plt->clean(true);
	}
}



// VerifyPilotData
//
//	Call this function to check the data that is in the given pilot struct...it will verify that all files
//	listed are available, if they are not, then it will set them to defaults. Returns true if it had to
//	fix the data (you may want to save the pilot immediatly)
bool VerifyPilotData(pilot *Pilot)
{
	if(!Pilot){
		Int3();
		return false;
	}
	Pilot->verify();
	return false;
}

/*
***********************************************************
*/

#define IDP_SELECT			0x50
#define IDP_SEL_PILOTLIST	0x51
#define IDP_DELETE			0x52
#define IDP_ADD				0x53
#define IDP_EDIT			0x54
#define IDP_SHIPCONFIG		0x55
#define IDP_CHOOSEPIC		0x56
#define IDP_CONFIGCONT		0x57
#define IDP_CONFIGKEYB		0x58
#define IDP_COPYCONTROLS	0x59

struct pilot_select_menu
{
	newuiSheet *sheet;
	
	newuiListBox *pilot_list;

// sets the menu up.
	newuiSheet *setup(newuiMenu *menu)
	{
		sheet = menu->AddOption(IDP_SELECT, TXT_PILOTS, NEWUIMENU_MEDIUM);

	// Pilot selection list
		sheet->NewGroup(NULL, 6, 18);
		pilot_list = sheet->AddListBox(232,168,IDP_SEL_PILOTLIST);
		pilot_list->SetSelectChangeCallback(PilotListSelectChangeCallback);

		return sheet;
	};

// retreive values from property sheet here.
	void finish()
	{
		sheet = NULL;
	};

// process
	void process(int res)
	{
	};
};

struct pilot_edit_menu
{
	newuiSheet *sheet;

	int *difficulty;
	bool *profanity;
	bool *audiotaunts;
	char *pilot_name;

	newuiSheet *setup(newuiMenu *menu)
	{
		sheet = menu->AddOption(IDP_EDIT, TXT_CONFIGURE, NEWUIMENU_MEDIUM);

		//pilot name
		sheet->NewGroup(NULL,5,0);
		pilot_name = sheet->AddChangeableText(64);

		// difficulty
		sheet->NewGroup(TXT_PLTDIFFICULT,55, 12);
#if 1 //ndef DEMO		
		difficulty = sheet->AddFirstLongRadioButton(TXT_TRAINEE);
		sheet->AddLongRadioButton(TXT_ROOKIE);
		sheet->AddLongRadioButton(TXT_HOTSHOT);
		sheet->AddLongRadioButton(TXT_ACE);
		sheet->AddLongRadioButton(TXT_INSANE);
		*difficulty = 0;
#else
		difficulty = sheet->AddFirstLongRadioButton(TXT_TRAINEE);
		sheet->AddLongRadioButton(TXT_HOTSHOT);
		*difficulty = 0;
#endif
		sheet->NewGroup(TXT_CONTROLSCONFIG, 55, 93);
		sheet->AddLongButton(TXT_CPYKEYCONF, IDP_COPYCONTROLS);
		sheet->AddLongButton(TXT_CUSTKEYB,IDP_CONFIGKEYB);
		sheet->AddLongButton(TXT_CUSTGAMEC,IDP_CONFIGCONT);

		sheet->NewGroup(TXT_MULTIPLAYERCONFIG, 55, 150);
#if (!defined(OEM) && !defined(DEMO))
		sheet->AddLongButton(TXT_SELPILOTPIC, IDP_CHOOSEPIC);
#endif
		sheet->AddLongButton(TXT_SHIPCUSTOMIZE,IDP_SHIPCONFIG);

		sheet->NewGroup(TXT_MISCELLANEOUS,55,195);
		profanity = sheet->AddLongCheckBox(TXT_PROFFILTER);
		audiotaunts = sheet->AddLongCheckBox(TXT_AUDIOTAUNTS);

		return sheet;
	};

	// retreive values from property sheet here.
	void finish()
	{
		sheet = NULL;
	};

	// process
	void process(int res)
	{
	};
};

struct pilot_add_menu
{
	newuiSheet *sheet;
};

struct
{
	newuiMenu *menu;
	pilot_select_menu *select;
	pilot_edit_menu *edit;
	
	bool initial_call;
	bool all_setup;
}PilotChooseDialogInfo;
pilot working_pilot;

void PilotListSelectChangeCallback(int index)
{
	if(!filecount || !PilotChooseDialogInfo.all_setup)
		return;

	pilot *Pilot = &working_pilot;
	char name[PILOT_STRING_SIZE];
	ubyte difficulty;
	bool profanity,audiotaunts;
	bool in_edit = false;
	char fullpath[MAX_PATH];

	if(PilotChooseDialogInfo.menu->GetCurrentOption()==IDP_EDIT)
	{
		in_edit = true;
		PilotChooseDialogInfo.edit->sheet->Realize();
	}	

	//Pilot has changed...reset all data to new pilot selected
	if(!PilotChooseDialogInfo.initial_call)
	{
		//save out old Pilot file so we can load up the new one
		char filename[PAGENAME_LEN];
		working_pilot.get_filename(filename);

		ddio_MakePath(fullpath, User_directory, filename, nullptr);
		if(cfexist(fullpath))
		{
			if(in_edit)
				PilotChooseDialogInfo.edit->sheet->UpdateReturnValues();

			//only save if the file is already there
			//which keeps us from bringing deleted pilots
			//back from the dead.
			difficulty = *PilotChooseDialogInfo.edit->difficulty;
			profanity = *PilotChooseDialogInfo.edit->profanity;
			audiotaunts = *PilotChooseDialogInfo.edit->audiotaunts;

			Pilot->set_profanity_filter(profanity);
			Pilot->set_difficulty(difficulty);
			Pilot->set_audiotaunts(audiotaunts);
			PltWriteFile(&working_pilot);
			mprintf((0,"Pilot saved\n"));			
		}else
		{
			mprintf((0,"Skipping pilot save...has the old pilot been deleted?\n"));
		}

		working_pilot.clean(true);
	}

	Pilot->set_filename(filelist[index]);
	PltReadFile(Pilot);
	
	// Setup all values
	///////////////////////////////	
	Pilot->get_difficulty(&difficulty);
	Pilot->get_profanity_filter(&profanity);
	Pilot->get_audiotaunts(&audiotaunts);
	*PilotChooseDialogInfo.edit->difficulty = difficulty;
	*PilotChooseDialogInfo.edit->profanity = profanity;
	*PilotChooseDialogInfo.edit->audiotaunts = audiotaunts;
	
	
	if(in_edit)
		PilotChooseDialogInfo.edit->sheet->UpdateChanges();
	Pilot->get_name(name);
	mprintf((0,"Pilot has changed to: %s\n",name));

	if(PilotChooseDialogInfo.edit->pilot_name){
		strncpy(PilotChooseDialogInfo.edit->pilot_name,name,63);
		PilotChooseDialogInfo.edit->pilot_name[63] = '\0';
	}


	PilotChooseDialogInfo.initial_call = false;
}

void selectcb(newuiMenu *menu,short id, void *data)
{
	pilot_select_menu *select = (pilot_select_menu *)data;
	if(id==IDP_SELECT)
	{
		menu->SetFocusOnGadget(select->pilot_list);
	}
}

void PilotSelect(void)
{
	newuiMenu menu;
	pilot_select_menu select;
	pilot_edit_menu edit;
	char fullpath[MAX_PATH];

	PilotChooseDialogInfo.menu = &menu;
	PilotChooseDialogInfo.select = &select;
	PilotChooseDialogInfo.edit = &edit;
	PilotChooseDialogInfo.initial_call = true;
	PilotChooseDialogInfo.all_setup = false;
	filelist = NULL;
	filecount = 0;

	int res=-1;
	bool done = false;

	ddio_MakePath(fullpath, User_directory, Default_pilot, NULL);
	if(cfexist(fullpath)!=CF_NOT_FOUND){
		//ok so the default pilot file is around, mark this as the current pilot
		Current_pilot.set_filename(Default_pilot);
		PltReadFile(&Current_pilot);
	}

	char pfilename[_MAX_FNAME];

	// open menu
	menu.Create();

	select.setup(&menu);						// setup pilot select menu IDP_SELECT
	edit.setup(&menu);							// edit pilot
	menu.AddSimpleOption(IDP_ADD,TXT_ADD);				// add
	menu.AddSimpleOption(IDP_DELETE,TXT_DELETE);		// delete currently selected pilot
	menu.AddSimpleOption(UID_OK, TXT_OK);
	menu.AddSimpleOption(UID_CANCEL,TXT_CANCEL);	
	menu.SetCurrentOption(IDP_SELECT);

	menu.SetOnOptionFocusCB(selectcb,&select);

	PilotChooseDialogInfo.all_setup = true;
	filelist = 	PltGetPilots(&filecount);

	if(!filecount){
		pilot temp_pilot;
		//if there are currently no pilots force player to create a new pilot
		if(PilotCreate(&temp_pilot,true)){
			PltClearList();
			PltGetPilotsFree();
			filelist = 	PltGetPilots(&filecount);
			NewPltUpdate(select.pilot_list,filelist,filecount,filecount-1);

			int index = select.pilot_list->GetCurrentIndex();

			PilotListSelectChangeCallback(index);

			PilotCopyDefaultControls(&working_pilot);

			menu.SetCurrentOption(IDP_EDIT);
		}
	}
	
	Current_pilot.get_filename(pfilename);
	NewPltUpdate(select.pilot_list,filelist,filecount,0,pfilename);

	//if we get here than there is at least one pilot already
	char old_file[MAX_PATH];

	//use this in case they cancel out
	Current_pilot.get_filename(pfilename);
	ddio_MakePath(fullpath, User_directory, pfilename, NULL);
	if(cfexist(fullpath)!=CF_NOT_FOUND){
		strcpy(old_file,fullpath);
	}else{
		old_file[0] = '\0';
	}

	DoWaitMessage(false);

	menu.Open();

	// run menu
	do
	{
		res = menu.DoUI();

		switch(res)
		{
		case IDP_EDIT:
			{
				if(!filecount)
				{
					menu.SetCurrentOption(IDP_SELECT);
				}
			}break;

		case IDP_SEL_PILOTLIST:
		case UID_OK:
			{
				//Start Game
				if(filecount){
					int index = select.pilot_list->GetCurrentIndex();

					PilotListSelectChangeCallback(index);

					char filename[PAGENAME_LEN];
					working_pilot.get_filename(filename);					
					Current_pilot.set_filename(filename);
					PltReadFile(&Current_pilot,true,true);

					char pname[PILOT_STRING_SIZE];
					Current_pilot.get_name(pname);
					mprintf((0,"Pilot To Use: %s\n",pname));

					if(VerifyPilotData(&Current_pilot)){
						//save out updated pilot since it had to be fixed
						mprintf((0,"PILOT: Saving out Pilot info due to bad data in pilot file\n"));
						PltWriteFile(&Current_pilot);
					}

					Current_pilot.get_filename(Default_pilot);

					done = true;

				}else{
					DoMessageBox(TXT_PLTERROR,TXT_NEEDTOCREATE,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
					menu.SetCurrentOption(IDP_SELECT);
				}
			}break;

		case UID_CANCEL:
			{
				// Cancel out
				bool found_old = (cfexist(old_file)!=CF_NOT_FOUND);
				bool display_error;

				if(filecount && found_old)
					display_error = false;
				else
					display_error = true;

				if(filecount)
				{
					int index = select.pilot_list->GetCurrentIndex();
					PilotListSelectChangeCallback(index);

					if(found_old){
						Current_pilot.set_filename(old_file);
						PltReadFile(&Current_pilot,true,true);

						char pname[PILOT_STRING_SIZE];
						Current_pilot.get_name(pname);

						mprintf((0,"Pilot To Use: %s\n",pname));
						if(VerifyPilotData(&Current_pilot)){
							//save out updated pilot since it had to be fixed
							mprintf((0,"PILOT: Saving out Pilot info due to bad data in pilot file\n"));
							PltWriteFile(&Current_pilot);
						}
						done = true;
					}
				}
				
				if(display_error)
				{
					if(filecount>0 && old_file[0]!='\0'){
						DoMessageBox(TXT_PLTERROR,TXT_OLDPILOTNOEXIST,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
					}else{
						DoMessageBox(TXT_PLTERROR,TXT_NEEDTOCREATE,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
					}
				}

			}break;

		case IDP_DELETE:
			//delete a pilot
			if(filecount)
			{				
				pilot temp_pilot;
				char buff[200];
				int tindex=select.pilot_list->GetCurrentIndex();

				PilotListSelectChangeCallback(tindex);

				temp_pilot.set_filename(filelist[tindex]);
				PltReadFile(&temp_pilot);

				char pname[PILOT_STRING_SIZE];
				temp_pilot.get_name(pname);

				sprintf(buff,TXT_PLTOKDEL,pname);
				if(DoMessageBox(TXT_PLTDELCONF,buff,MSGBOX_YESNO,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL))	{
					PltDelete(&temp_pilot);
				}

				PltClearList();
				PltGetPilotsFree();
				filelist = 	PltGetPilots(&filecount);
				if(tindex>=filecount){
					tindex = filecount-1;
				}
				NewPltUpdate(select.pilot_list,filelist,filecount,tindex);
			}
			break;

		case IDP_ADD:
			{
				//Create New Player
				bool go_into_edit = false;
				int cpilotindex = select.pilot_list->GetCurrentIndex();
				pilot temp_pilot;
				if(PilotCreate(&temp_pilot,!filecount)){
					PltClearList();
					PltGetPilotsFree();
					filelist = 	PltGetPilots(&filecount);

					char pfilename[_MAX_FNAME];
					temp_pilot.get_filename(pfilename);
					NewPltUpdate(select.pilot_list,filelist,filecount,filecount-1,pfilename);
					go_into_edit = true;
				}else{
					if(filecount)
						NewPltUpdate(select.pilot_list,filelist,filecount,cpilotindex);
				}

				int index = select.pilot_list->GetCurrentIndex();
				PilotListSelectChangeCallback(index);

				if(go_into_edit)
				{
					PilotCopyDefaultControls(&working_pilot);
				}

				if(go_into_edit)
					menu.SetCurrentOption(IDP_EDIT);
				else
					menu.SetCurrentOption(IDP_SELECT);
			}break;

		case IDP_SHIPCONFIG:
			{
				PltSelectShip(&working_pilot);
			}break;

		case IDP_CHOOSEPIC:
			{
				char pname[PILOT_STRING_SIZE];
				working_pilot.get_name(pname);

				if(PPic_QueryPilot(pname))
				{
					ShowPilotPicDialog(&working_pilot);
				}else
				{
					ushort pid;
					pid = PPIC_INVALID_ID;		
					working_pilot.set_multiplayer_data(NULL,NULL,NULL,&pid);

					DoMessageBox(TXT_PLTERROR,TXT_NOPICSAVAIL,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
				}

			}break;

		case IDP_CONFIGCONT:
		case IDP_CONFIGKEYB:
			{
				if(!filecount){
					break;
				}

				int index = select.pilot_list->GetCurrentIndex();

				//this saves out the pilot we're currently working on
				PilotListSelectChangeCallback(index);

				//read the working pilot into the Current_pilot position
				char pfilename[_MAX_FNAME];
				working_pilot.get_filename(pfilename);

				Current_pilot.set_filename(pfilename);
				PltReadFile(&Current_pilot,true,true);
				//configure the current pilot
				if(res==IDP_CONFIGKEYB)
					CtlConfig(CTLCONFIG_KEYBOARD);
				else
					CtlConfig(CTLCONFIG_CONTROLLER);
				//now save the current_pilot out to disk
				PltWriteFile(&Current_pilot,false);

				PltReadFile(&working_pilot,true,true);
			}break;

		case IDP_COPYCONTROLS:
			{
				pilot s_pil;
				char pfilename[_MAX_FNAME];
				working_pilot.get_filename(pfilename);

				// destroy the current list of pilots and recreate, but 
				// ignoring our current pilot
				PltClearList();
				PltGetPilotsFree();
				filelist = 	PltGetPilots(&filecount,pfilename,1);

				if(filecount>=1)
				{
					if(PilotChoose(&s_pil))
					{						
						char spfilename[_MAX_FNAME];						
						s_pil.get_filename(spfilename);

						if(strcmp(spfilename,pfilename)==0)
						{
							//user choose the same file as what he is configuring
							DoMessageBox(TXT_COPYCONFERR,TXT_COPYCONFERR1,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
						}
						else
						{
							PltCopyKeyConfig(&s_pil,&working_pilot);
						}
					}
				}else
				{
					DoMessageBox(TXT_PLTERROR,TXT_NOPILOTSTOCOPY,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
				}
				
				// Restore the pilot list, ignoring none
				PltClearList();
				PltGetPilotsFree();
				filelist = 	PltGetPilots(&filecount);
			}break;
		}

	}while(!done);

	PltClearList();
	PltGetPilotsFree();
	Current_pilot.get_difficulty(&ingame_difficulty);

	// get settings
	select.finish();
	edit.finish();
	menu.Close();
	menu.Destroy();
}


//////////////////////////////////////////////////////////////
//Brings up the Create A New Pilot Dialog
bool PilotCreate(pilot *Pilot,bool forceselection)
{
	bool done = false;
	bool ret;
	bool to_ret = false;
	char pname[PILOT_STRING_SIZE];

	while(!done)
	{
		pname[0] = 0;
		ret = DoEditDialog(TXT_PLTENTERNAME1,pname,PILOT_STRING_SIZE-1);

		if(!ret)
		{
			if(!forceselection)
			{
				done = true;
				to_ret = false;
			}
		}else
		{
			mprintf((0,"Creating Pilot!"));

			// call this to initialize pilot data for player.
			PilotInitData(Pilot);
			char pfilename[_MAX_FNAME];

			//strip whitespace
			char *ptr = pname + strlen(pname) - 1;
			while( (ptr > pname) && (*ptr==' '||*ptr=='\t') ){ *ptr = '\0'; ptr--; }

			ptr = pname;
			while( *ptr && (*ptr==' '||*ptr=='\t') ) ptr++;

			if(strlen(ptr)==0){
				DoMessageBox(TXT_PLTERROR,TXT_PLTERRORNAME,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
				goto retry;
			}

			Pilot->set_name(ptr);
			strcpy(pfilename,ptr);
			PltMakeFNValid(pfilename);
			strcat(pfilename,PLTEXTENSION);

			Pilot->set_filename(pfilename);			

			RestoreDefaultControls();

			for(int id=0;id<NUM_CONTROLLER_FUNCTIONS;id++)
			{
				ct_type type[2];
				ct_config_data value;
				ubyte flags[2];
				Controller->get_controller_function(Controller_needs[id].id, type, &value, flags);
				Pilot->controls[id].id = Controller_needs[id].id;
				Pilot->controls[id].type[0] = type[0];
				Pilot->controls[id].type[1] = type[1];
				Pilot->controls[id].value = value;
				Pilot->controls[id].flags[0] = flags[0];
				Pilot->controls[id].flags[1] = flags[1];
			}
			switch(PltWriteFile(Pilot,true))
			{
			case PLTW_CFILE_FATAL:
				{
					//real bad error trying to open file to write to
					DoMessageBox(TXT_FILEERROR,TXT_PLTCFILEERR,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
					done = true;
					to_ret = false;	//returning true, that way if they are out of drive space
									//they can't get to the pilot window, and delete some pilots
				}break;

			case PLTW_UNKNOWN_FATAL:
				{
					//real bad error trying to open file to write to
					DoMessageBox(TXT_FILEERROR,TXT_PLTUKNOWNERR,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
					done = true;
					to_ret = false;	//returning true, that way if they are out of drive space
									//they can't get to the pilot window, and delete some pilots
				}break;

			case PLTW_FILE_CANTOPEN:
				{
					//real bad error trying to open file to write to
					DoMessageBox(TXT_FILEERROR,TXT_FILEERRPLT1,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
					done = true;
					to_ret = false;	//returning true, that way if they are out of drive space
									//they can't get to the pilot window, and delete some pilots
				}break;

			case PLTW_NO_FILENAME:
				{
					//No name was given
					DoMessageBox(TXT_PLTERROR,TXT_PLTERRORNAME,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
				}break;

			case PLTW_FILE_EXISTS:
				{
					//pilot already exists so bring up error window
					DoMessageBox(TXT_PLTERROR,TXT_PLTERREXISTS,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
				}break;

			case PLTW_NO_ERROR:
				{
					//File saved ok
					done = true;
					to_ret = true;
				}break;
			}
		}
retry:;

	}	

	return to_ret;
}


///////////////////////////////////////////////////////////////////////////////////
// Helper funktions
///////////////////////////////////////////////////////////////////////////////////
void PilotCopyDefaultControls(pilot *Pilot)
{
	PltClearList();
	PltGetPilotsFree();
	filelist = 	PltGetPilots(&filecount,NULL,2);

	//if(filecount>0 && DoMessageBox(TXT_PRESETCONTROLS,TXT_USEPRESETS,MSGBOX_YESNO))
	if(filecount>0)
	{
		pilot s_pil;

		if(PilotChoose(&s_pil,true))
		{
			char spfilename[_MAX_FNAME];						
			s_pil.get_filename(spfilename);

			if(cfexist(spfilename))
			{
				PltCopyKeyConfig(&s_pil,Pilot);
			}else
			{
				mprintf((0,"%s does not exist...not copying\n",spfilename));
				Int3();
			}
		}
	}

	// Restore the pilot list, ignoring none
	PltClearList();
	PltGetPilotsFree();
	filelist = 	PltGetPilots(&filecount);
}

///////////////////////////////////////////////////////////
// Displays a window to select a pilot
bool PilotChoose(pilot *Pilot,bool presets)
{
#define IDP_PCLIST	19
#define PCHOOSE_WND_H	288
#define PCHOOSE_WND_W	384

	if(filecount==0)
		return false;

	newuiTiledWindow window;
	newuiSheet *sheet;
	newuiListBox *pilot_list;

	bool exit_menu = false;

	if(presets)
	{
		window.Create(TXT_PRESETCONTROLS,0,0,PCHOOSE_WND_W,PCHOOSE_WND_H);
	}else
	{
		window.Create(TXT_PLTCHOOSE,0,0,PCHOOSE_WND_W,PCHOOSE_WND_H);
	}
	sheet = window.GetSheet();

	sheet->NewGroup(NULL,57,0);
	sheet->AddText(TXT_PILOTPICTITLE);
	sheet->AddText(TXT_COPYCONTB);
	sheet->AddText(TXT_COPYCONTOLSC);

	sheet->NewGroup(NULL,7,40);
	pilot_list = sheet->AddListBox(284,100,IDP_PCLIST);
	pilot_list->SetCurrentIndex(0);
	NewPltUpdate(pilot_list,filelist,filecount,0,nullptr, presets);

	sheet->NewGroup(NULL,80,180,NEWUI_ALIGN_HORIZ);
	sheet->AddButton(TXT_OK,UID_OK);
	sheet->AddButton(TXT_CANCEL,UID_CANCEL);

	bool ret = false;

	window.Open();

	while (!exit_menu) 
	{
		int res = window.DoUI();

		// handle all UI results.
		switch (res) 
		{
		case IDP_PCLIST:
		case UID_OK:
		{
			int index = pilot_list->GetCurrentIndex();
			if(index>=0)
			{
				Pilot->set_filename(filelist[index]);
				PltReadFile(Pilot,false);
				ret = true;
				exit_menu = true;
			}
		}break;

		case UID_CANCEL:
			ret = false;
			exit_menu = true;
			break;
		}
	}

	window.Close();
	window.Destroy();

	return ret;
}

///////////////////////////////////////////////////////////
//copies a pilot to another
/***************************************************
bool PilotCopy(pilot *Src,pilot *Dest)
{
	char sname[PILOT_STRING_SIZE];
	char sship[PAGENAME_LEN];
	ubyte sdiff;

	Src->get_name(sname);
	Src->get_ship(sship);
	Src->get_difficulty(&sdiff);

	Dest->set_name(sname);
	Dest->set_ship(sship);
	Dest->set_difficulty(sdiff);

	return PltCopyKeyConfig(Src,Dest);
}
***************************************************/
/////////////////////////////////////////////////////
// Updates the pilot listbox
void NewPltUpdate(newuiListBox *list,char **flist,int filecount,int selected,char *filename, bool presets)
{
	int index;
	pilot tPilot;
	char fullpath[MAX_PATH];

	list->RemoveAll();

	if(pilot_items)
	{
		delete[] pilot_items;
		pilot_items=NULL;
	}

	if(filecount){
		//create UIItems
		char pname[PILOT_STRING_SIZE];

		for(index=0;index<filecount;index++)
		{
			tPilot.set_filename(flist[index]);
			PltReadFile(&tPilot);

			tPilot.get_name(pname);

			list->AddItem(pname);
		}

		list->SetCurrentIndex(selected);
		
		if (filename)
		{
			if (presets)
				ddio_MakePath(fullpath, Base_directory, filename, NULL);
			else
				ddio_MakePath(fullpath, User_directory, filename, NULL);
			if (cfexist(fullpath) != CF_NOT_FOUND) 
			{
				//get the selected pilot from the filename
				mprintf((0, "Looking for Pilot: %s\n", filename));
				for (int d = 0; d < filecount; d++) 
				{
					if (!stricmp(flist[d], filename)) 
					{
						//ok we found the filename that they want as the pilot
						list->SetCurrentIndex(d);
						break;
					}
				}
			}
		}
	}
}

// updates the current pilot's information (level played, mission played, etc)
//	call after every successful mission completion
extern bool IsCheater;
extern int Default_ship_permission;
void CurrentPilotUpdateMissionStatus(bool just_add_data)
{
	//Don't update if it's a multiplayer game
	if(Game_mode&GM_MULTI)
		return;
	//look for the current mission in the player's data
	int index;
	tMissionData mission_to_use;

	index = Current_pilot.find_mission_data(Current_mission.name);

	if(index==-1){

		//this mission doesn't exist for the pilot yet, so add the mission to the pilot
		mprintf((0,"PILOT: New Mission being added to mission data (%s)\n",Current_mission.name));
		mission_to_use.highest_level = 0;
		mission_to_use.finished = false;
		mission_to_use.num_restores = 0;
		mission_to_use.num_saves = 0;
		mission_to_use.ship_permissions = Default_ship_permission;
		strcpy(mission_to_use.mission_name,Current_mission.name);

		Current_pilot.add_mission_data(&mission_to_use);

		index = Current_pilot.find_mission_data(Current_mission.name);
		
		ASSERT(index!=-1);
		
	}else{
		//this pilot has flown this mission before, just update it
		mprintf((0,"PILOT: Updating previously flown mission data (%s)\n",Current_mission.name));

		Current_pilot.get_mission_data(index,&mission_to_use);
	}

	if(!just_add_data)
	{
	// bad place to do this, but this code was added at the last minute.  a long term fix requires more files to fix.
		int max_level;
		for (max_level=0; max_level < Current_mission.num_levels; max_level++)
		{
			if (Current_mission.levels[max_level].flags & LVLFLAG_FINAL) {
				max_level++;
				break;
			}
		}

		if( Current_mission.cur_level>mission_to_use.highest_level && !IsCheater && Current_mission.cur_level < max_level){//cheaters don't win
			ASSERT(!(Game_mode&GM_MULTI));
			mission_to_use.highest_level = Current_mission.cur_level;
			mission_to_use.ship_permissions = Players[0].ship_permissions;
		}

	// if we've reached end of mission, we've finished it.
		if (Current_mission.cur_level == max_level) {
			mission_to_use.finished = true;
		}

		Current_pilot.edit_mission_data(index,&mission_to_use);
	}
	
	PltWriteFile(&Current_pilot,false);
}

// gets highest level flown for mission
int PilotGetHighestLevelAchieved(pilot *Pilot,char *mission_name)
{
	ASSERT(Pilot);
	if(!Pilot)
		return 1;

	int index = Pilot->find_mission_data(mission_name);
	if(index==-1)
		return 0;

	tMissionData data;
	Pilot->get_mission_data(index,&data);

	return data.highest_level;
}

// just like it says
bool HasPilotFinishedMission(pilot* Pilot, const char *mission_name)
{
	ASSERT(Pilot);
	if(!Pilot)
		return false;

	int index = Pilot->find_mission_data((char *)mission_name);
	if(index==-1)
		return false;

	tMissionData data;
	Pilot->get_mission_data(index,&data);
	return data.finished;
}

bool HasPilotFlownMission(pilot *Pilot,const char *mission_name)
{
	ASSERT(Pilot);
	if(!Pilot)
		return false;

	int index = Pilot->find_mission_data((char *)mission_name);
	if(index==-1)
		return false;
	return true;
}

int GetPilotNumSavedGamesForMission(pilot *Pilot,const char *mission_name)
{
	ASSERT(Pilot);
	if(!Pilot)
		return 0;

	int index = Pilot->find_mission_data((char *)mission_name);
	if(index==-1)
		return 0;

	tMissionData data;
	Pilot->get_mission_data(index,&data);
	return data.num_saves;
}

int GetPilotNumRestoredGamesForMission(pilot *Pilot,const char *mission_name)
{
	ASSERT(Pilot);
	if(!Pilot)
		return 0;

	int index = Pilot->find_mission_data((char *)mission_name);
	if(index==-1)
		return 0;

	tMissionData data;
	Pilot->get_mission_data(index,&data);
	return data.num_restores;
}

void IncrementPilotSavedGamesForMission(pilot *Pilot,const char *mission_name)
{
	ASSERT(Pilot);
	if(!Pilot)
		return;

	int index = Pilot->find_mission_data((char *)mission_name);
	if(index==-1)
		return;

	tMissionData data;
	Pilot->get_mission_data(index,&data);
	data.num_saves++;
	Pilot->edit_mission_data(index,&data);
	
	PltWriteFile(Pilot,false);
}

void IncrementPilotRestoredGamesForMission(pilot *Pilot,const char *mission_name)
{
	ASSERT(Pilot);
	if(!Pilot)
		return;

	int index = Pilot->find_mission_data((char *)mission_name);
	if(index==-1)
		return;

	tMissionData data;
	Pilot->get_mission_data(index,&data);
	data.num_restores++;
	Pilot->edit_mission_data(index,&data);
	
	PltWriteFile(Pilot,false);
}

int GetPilotShipPermissions(pilot *Pilot,const char *mission_name)
{
	ASSERT(!(Game_mode&GM_MULTI));
	ASSERT(Pilot);
	if(!Pilot)
		return 1;

	int index = Pilot->find_mission_data((char *)mission_name);
	if(index==-1)
		return 0;

	tMissionData data;
	Pilot->get_mission_data(index,&data);

	return data.ship_permissions;
}

////////////////////////////////////////////////////////////////////////
// Pilot file functions
////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////
//deletes a pilot
bool PltDelete(pilot *Pilot)
{
	char filename[_MAX_PATH];
	char pfilename[_MAX_FNAME];
	char pname[PILOT_STRING_SIZE];

	Pilot->get_filename(pfilename);

	if(pfilename[0]!=0)
	{
		ddio_MakePath(filename, User_directory,pfilename,NULL);
		return (ddio_DeleteFile(pfilename)==1);
	}
	else
	{
		Int3();	//this is odd

		//it shouldn't get to here, but if for some reason filename didn't exist
		//then create the filename from the player name and delete the file
		Pilot->get_name(pname);

		if(pname[0]==0)
			return false;

		PltMakeFNValid(pname);

		strcpy(pfilename,pname);
		strcat(pfilename,PLTEXTENSION);
		ddio_MakePath(filename, User_directory,pfilename,NULL);
		return (ddio_DeleteFile(filename)==1);
	}
}

void PltCreateDedicatedServerPilot(pilot *Pilot)
{
	Pilot->clean(true);
	Pilot->set_name("SERVER");
}

//////////////////////////////////////////////
//writes a pilot out to file
//make sure Pilot->filename and Pilot->name are filled in correctly!!!!
int PltWriteFile(pilot *Pilot,bool newpilot)
{
	if(Dedicated_server)
		return PLTW_NO_ERROR;

	Pilot->write();
	return Pilot->flush(newpilot);
}

////////////////////////////////////////////////////
//reads in a pilot file (fill in pilot name to read in the struct before calling)
//Pilot->filename MUST be filled in (with .plt extension)
void _ReadOldPilotFile(pilot *Pilot,bool keyconfig,bool missiondata);

void PltReadFile(pilot *Pilot,bool keyconfig,bool missiondata)
{
	if(Dedicated_server)
	{
		PltCreateDedicatedServerPilot(Pilot);
		return;
	}

	char filename[_MAX_PATH];
	char pfilename[_MAX_FNAME];
	char ext[_MAX_PATH];
	CFILE *file;
	int filever;

	Pilot->get_filename(pfilename);
	if(pfilename[0]==0)
		return;

	ddio_SplitPath(pfilename, nullptr, nullptr, ext);

	//open and process file
	if (!stricmp(ext, ".pld")) //[ISB] presets are part of the base data
		ddio_MakePath(filename, Base_directory, pfilename, nullptr);
	else
		ddio_MakePath(filename, User_directory, pfilename, nullptr);
	try
	{
		file = cfopen(filename,"rb");
		if(!file)
			return;

		filever = cf_ReadInt(file);
		cfclose(file);
	}catch(...)
	{
		mprintf((0,"File exception has occured\n"));		
		Int3();
		Error(TXT_MAJORPLTERROR);
		return;
	}

	if(filever>=0x20){
		//new pilot files!
		int ret = Pilot->read(!keyconfig,!missiondata);
		switch(ret)
		{
		case PLTR_TOO_NEW:
			Error(TXT_PLTFILETOONEW,pfilename);
			break;
		}

		return;
	}else{
		try
		{
			_ReadOldPilotFile(Pilot,keyconfig,missiondata);
		}catch(...)
		{
			mprintf((0,"File exception has occured\n"));		
			Int3();
			Error(TXT_MAJORPLTERROR);
			return;
		}
	}
}

//////////////////////////////////////////////////////////////
//returns the filelist of pilots available
struct tPGetPilotStruct
{
	char *filename;
	tPGetPilotStruct *next;
};
static char **pltgetname_list = NULL;
static int pltgetname_count = 0;

void PltGetPilotsFree(void)
{
	if(pltgetname_list){
		mem_free(pltgetname_list);
		pltgetname_list = NULL;
	}
	pltgetname_count = 0;
}

char **PltGetPilots(int *count,char *ignore_filename,int display_default_configs)
{
	static char **pname_list = NULL;
	
	char buffer[PLTFILELEN];
	char search[256];
	tPGetPilotStruct *root = NULL,*curr = NULL;
	int loop = 1;

	if(display_default_configs==1)
		loop = 2;

	//clear list
	PltClearList();	
	PltGetPilotsFree();
	(*count) = 0;
	root = NULL;

	for(int loop_count=0;loop_count<loop;loop_count++)
	{
		switch(display_default_configs)
		{
		case 0:
			ASSERT(loop_count==0);
			ddio_MakePath(search, User_directory,PLTWILDCARD,NULL);
			break;
		case 1:
			ddio_MakePath(search, User_directory,(loop_count==0)?PLTWILDCARD:DPLTWILDCARD,NULL);
			break;
		case 2:
			ASSERT(loop_count==0);
			ddio_MakePath(search, Base_directory,DPLTWILDCARD,NULL);
			break;
		default:
			Int3();
			break;
		}
	
		if(ddio_FindFileStart(search,buffer)){

			if(ignore_filename && !stricmp(ignore_filename,buffer))
			{
				mprintf((0,"Getting Pilots...found %s, but ignoring\n",buffer));
			}else
			{
				if(root==NULL)
				{
					root = curr = (tPGetPilotStruct *)mem_malloc(sizeof(tPGetPilotStruct));
					curr->next = NULL;
				}else
				{
					curr->next = (tPGetPilotStruct *)mem_malloc(sizeof(tPGetPilotStruct));
					curr = curr->next;
					curr->next = NULL;
				}

				if(curr)
				{					
					curr->filename = mem_strdup(buffer);

					if(curr->filename)
					{
						(*count)++;
						mprintf((0,"Getting Pilots..found %s\n",buffer));
					}else
					{
						Error(TXT_OUTOFMEMORY);
					}
				}

			}
			
			while(ddio_FindNextFile(buffer)){
				if(ignore_filename && !stricmp(ignore_filename,buffer))
				{
					mprintf((0,"Getting Pilots...found %s, but ignoring\n",buffer));
				}else
				{
					if(root==NULL)
					{
						root = curr = (tPGetPilotStruct *)mem_malloc(sizeof(tPGetPilotStruct));
						curr->next = NULL;
					}else
					{
						curr->next = (tPGetPilotStruct *)mem_malloc(sizeof(tPGetPilotStruct));
						curr = curr->next;
						curr->next = NULL;
					}
					
					if(curr){
						curr->filename = (char *)mem_strdup(buffer);						

						if(curr->filename){						
							(*count)++;
							mprintf((0,"Getting Pilots..found %s\n",buffer));
						}else{
							Error(TXT_OUTOFMEMORY);
						}
					}
				}
			}

		}

		ddio_FindFileClose();
	}

	//now allocate for the real list of char * and move the linked list to that array
	if((*count)>0){
		pltgetname_list = (char **)mem_malloc((*count)*sizeof(char *));
		if(pltgetname_list){
			int end = (*count);
			curr = root;
			tPGetPilotStruct *next;

			for(int i=0;i<end;i++){
				//move all the allocated buffers to this list, than we can free the node
				next = curr->next;

				if(curr){
					pltgetname_list[i] = curr->filename;
				}else{
					pltgetname_list[i] = NULL;
				}

				mem_free(curr);
				curr = next;
			}

		}
	}
	pltgetname_count = (*count);

	mprintf((0,"Found %d pilots\n",(*count)));
	return pltgetname_list;
}

///////////////////////////////////////////////////////////
//clears the file list (MUST CALL TO FREE MEMORY!!!!!!!!!)
void PltClearList(void)
{
	if(!pltgetname_list)
		return;

	int index;

	for(index=0;index<pltgetname_count;index++)
	{
		if(pltgetname_list[index])
		{
			mem_free(pltgetname_list[index]);
			pltgetname_list[index]=NULL;
		}
	}
}

////////////////////////////////////////////////////////////
//given a name, it will create a valid filename
void PltMakeFNValid(char *name)
{
	int whiteindex;
	int end = strlen(name);

	for(whiteindex=0;whiteindex<end;whiteindex++)
	{
		//remove illegal character (sub _ in place)
		switch(name[whiteindex])
		{
		case '?':
		case '*':
		case '\\':
		case '/':
		case '|':
		case '<':
		case '>':
		case ':':
		case '"':
			name[whiteindex]='_';
			break;
		default:
			break;
		}
	}

	//remove leading whitespace
	whiteindex=0;
	while(name[whiteindex]==' ') whiteindex++;

	char temp[PLTFILELEN];
	strcpy(temp,&name[whiteindex]);
	strcpy(name,temp);
}

////////////////////////////////////////////////////////
// Copies the key/joy config of a pilot to another (src must exist!) (Keep up to date with Read/Write file)
// this will also save out the dest, so make sure its filled in before calling function
bool PltCopyKeyConfig(pilot *src,pilot *dest)
{
	//check to make sure src exists
	ASSERT(src);
	ASSERT(dest);

	char pname[PILOT_STRING_SIZE];
	src->get_name(pname);

	if( (pname[0]!=0) && (strlen(pname)>0) && (strcmp(pname," ")!=0) )
	{
		int i;
		for(i=0;i<NUM_CONTROLLER_FUNCTIONS;i++)
		{
			dest->controls[i].id = src->controls[i].id;
			dest->controls[i].type[0] = src->controls[i].type[0];
			dest->controls[i].type[1] = src->controls[i].type[1];
			dest->controls[i].value = src->controls[i].value;
			dest->controls[i].flags[0] = src->controls[i].flags[0];
			dest->controls[i].flags[1] = src->controls[i].flags[1];
		}
		for(i=0;i<N_MOUSE_AXIS;i++)
		{
			dest->mouse_sensitivity[i] = src->mouse_sensitivity[i];
		}
		for(i=0;i<N_JOY_AXIS;i++)
		{
			dest->joy_sensitivity[i] = src->joy_sensitivity[i];
		}
		dest->key_ramping = src->key_ramping;
		dest->read_controller = src->read_controller;
		dest->mouselook_control = src->mouselook_control;

		return true;
	}
	else
		return false;
}

/*
 ************************************************************************
 *		The following section is for the multiplayer ship selection		*
 *		and configuration.												*
 ************************************************************************
 *
*/

//	UI3DWindow class
//	
//	UI class for displaying a polymodel on the UIWindow, rotating 30 deg/sec
class UI3DWindow: public UIStatic
{
public:
	void OnDraw(void);
};

//	UIBmpWindow class
//
//	UI class for displaying a bitmap on a UIWindow
class UIBmpWindow: public UIStatic
{
public:
	UIBmpWindow();
	~UIBmpWindow();
	void SetInfo(bool animated,int handle);
	void OnDraw(void);
private:
	void DrawBorder(void);
	bool animated;
	int bm_handle;
	float start_time;
	UIText text;
	bool created;
};

//	struct ship_pos
//	
//	contains information about the ship to be displayed in the ship configuration
typedef struct{
	matrix last_frame;
	float cam_dist;
	float last_time;
	bool texture_changed;
	int texture_type;
	int texture_id;
	int bm_handle;
	void Init(){
		vm_AnglesToMatrix (&last_frame,0,(65535/2),0);
		last_time = timer_GetTime();
		texture_id = Players[0].custom_texture_handle;
		bm_handle = -1;
	}
}tShipPos;

tShipPos ship_pos;
int ship_model = -1;
char custom_texture[_MAX_PATH];
UIBmpWindow *bmpwindow;

typedef struct{
	int *idlist;				//array of remapping indicies (list index->ship index)
	void Init(){
		idlist = NULL;
	}
	void Reset(){
		if(idlist){
			mem_free(idlist);
			idlist = NULL;
		}
	}
}tShipListInfo;

tCustomListInfo *lp_cust_bmps = NULL;
tShipListInfo *lp_ship_info = NULL;


//	axtoi
//
//Convert a string that represents a hex value into an int (like atoi)
int axtoi(char *p)
{
	int value = 0;

	while( (p) &&(*p))
	{
		*p = toupper(*p);
		if ( (*p>='0') && (*p<='9') )
			value = (value * 16) + ((*p)-'0');
		else if ( (*p>='A') && (*p<='F') )
			value = (value * 16) + (((*p)-'A')+10);
		else
			return 0;
		p++;
	}
	return value;
}

//	StripCRCFileName
//
//	Given a filename that has the _<CRC> in it at the end of the name, it will fill
//	in dest with just the filename part (without the trailing _<CRC>).
void StripCRCFileName(const char *src,char *dest)
{
	ASSERT(src);
	ASSERT(dest);

	int full_strlen = strlen(src);
	char ext[256];
	ddio_SplitPath(src,NULL,NULL,ext);
	int extstrlen = strlen(ext);
	int eon = full_strlen-9-extstrlen;

	if(eon<1){
		//this filename doesn't have a trailing CRC
		strcpy(dest,src);
		return;
	}

	//check to make sure the [eon] is '_'
	if(src[eon]!='_'){
		//it wasn't so it must not have a CRC at the end
		strcpy(dest,src);
		return;
	}

	char hexstring[9];
	strncpy(hexstring,&src[eon+1],8);
	hexstring[8] = '\0';
	unsigned int crc;
	crc = axtoi(hexstring);

	if(crc==0){
		//crc can't be 0?
		strcpy(dest,src);
		return;
	}

	//strip the filename
	strncpy(dest,src,eon);
	dest[eon]='\0';
	strcat(dest,ext);	//put back the extension
}


//	CreateCRCFileName
//
//	Given a filename, this function will open the file, make a CRC of the data
//	and then fill in dest buffer with <filename>_<CRC> where <filename> is the
//	original filename and <CRC> is the converted string of the CRC in hex.
//	returns true on success.
//	dest should be size of src + 9 (for the _ & CRC)
bool CreateCRCFileName(const char *src,char *dest)
{
	ASSERT(src);
	ASSERT(dest);

	if(cfexist(src)!=CF_ON_DISK)
		return false;

	unsigned int crc_value = cf_GetfileCRC((char *)src);
	if(crc_value==0){
		mprintf((0,"CRC WARNING: A CRC of 0 HAS BEEN GENERATED!\n"));
	}
	char hex_string[10];
	sprintf(hex_string,"_%08X",crc_value);

	char ext[256];
	ddio_SplitPath(src,NULL,NULL,ext);

	//now create the full filename
	//first strip any possible CRC on the file
	StripCRCFileName(src,dest);
	dest[strlen(dest)-strlen(ext)] = '\0';	//lose the extension
	strcat(dest,hex_string);
	strcat(dest,ext);						//put extension back
	
	return true;
}

//	CreateCRCFileName
//
//	Given a file, a new filename, it will take the src file, create a new filename, with base as the base
bool CreateCRCFileName(const char *src,char *base,char *newfilename)
{
	ASSERT(src);
	ASSERT(base);
	ASSERT(newfilename);

	if(cfexist(src)!=CF_ON_DISK)
		return false;

	unsigned int crc_value = cf_GetfileCRC((char *)src);
	if(crc_value==0){
		mprintf((0,"CRC WARNING: A CRC of 0 HAS BEEN GENERATED!\n"));
	}
	char hex_string[10];
	sprintf(hex_string,"_%08X",crc_value);

	char ext[256];
	ddio_SplitPath(base,NULL,NULL,ext);

	//now create the full filename
	//first strip any possible CRC on the file
	strcpy(newfilename,base);
	newfilename[strlen(newfilename)-strlen(ext)] = '\0';	//lose the extension
	strcat(newfilename,hex_string);
	strcat(newfilename,ext);						//put extension back
	
	return true;
}


//	FindAllFiles
//	
//	Retrieves the files in the current directory that match the pattern given.	Call FindAllFilesSize() to determine
//	how much memory will be needed for the buffer.
//	pattern	=	wildcard pattern to use to match
//	list	=	buffer of memory that will be filled in with the filenames (each seperated by a \0)
//	size	=	size of memory allocated for list parm
//	filecount = filled in with the number of files it found
int FindAllFiles(const char *pattern,char *list,int size,int *filecount)
{
	int count = 0;
	char filename[_MAX_PATH];
	*filecount = 0;
	int memory = 0;

	if(size<=0)
		return 0;
	if(!list)
		return 0;
	if(!pattern)
		return 0;

	if(ddio_FindFileStart(pattern,filename)){
		count++;
		strcpy(&list[memory],filename);
		memory += strlen(&list[memory]) + 1;
		while( (count<size) && (ddio_FindNextFile(filename)) ){
			strcpy(&list[memory],filename);
			memory += strlen(&list[memory]) + 1;
			count++;
		}
	}
	ddio_FindFileClose();
	*filecount = count;
	return memory;
}

//	FindAllFilesSize
//
//	Looks in the current directory and returns the number of bytes of memory that will be needed
//	for a buffer of filenames (see FindAllFiles()).
//	pattern	=	wildcard pattern to match
//	filecount =	will be filled in with the number of files it finds
int FindAllFilesSize(const char *pattern,int *filecount)
{
	if(!pattern){
		*filecount =0;
		return 0;
	}

	char filename[_MAX_PATH];
	int size = 0;
	int count = 0;

	if(ddio_FindFileStart(pattern,filename)){
		count++;
		size+=strlen(filename) + 1;
		while(ddio_FindNextFile(filename)){
			count++;
			size+=strlen(filename) + 1;
		}
		ddio_FindFileClose();
	}
	*filecount = count;
	return size;
}

//returns the optimal distance to place the camera
float getdist(angle ang,float height)
{
	float s,c,t;
	vm_SinCos (ang,&s,&c);
	t = s/c;	
	return (height/t);
}


//	ImportGraphic
//
//	Takes the graphics file at the given location and imports it into the custom\graphics dir as an scaled ogf
//	pathname	=	full path to source bitmap
//	newfile		=	on return true is the filename of the new bitmap
bool ImportGraphic(char *pathname,char *newfile)
{
	ASSERT(pathname);
	if(cfexist(pathname)!=CF_ON_DISK){
		mprintf((0,"'%s' not found\n",pathname));
		return false;
	}

	int bm_handle = bm_AllocLoadFileBitmap(IGNORE_TABLE(pathname),0);
	if(bm_handle<=BAD_BITMAP_HANDLE){
		return false;
	}

	bm_ChangeSize(bm_handle,64,64);

	char p[_MAX_PATH];
	char filename[_MAX_FNAME],ext[_MAX_EXT];
	char tempfilename[_MAX_PATH];

	// Create a temporary filename, so that we can temporarily save the graphic to this file
	if(!ddio_GetTempFileName(Descent3_temp_directory,"d3i",tempfilename)){
		//there was an error trying to create a temporary filename
		bm_FreeBitmap(bm_handle);
		mprintf((0,"Error creating temp filename\n"));
		return false;
	}

	//save out the file
	if(bm_SaveFileBitmap (tempfilename,bm_handle)==-1){
		mprintf((0,"Error importing\n"));
		bm_FreeBitmap(bm_handle);
		return false;
	}

	bm_FreeBitmap(bm_handle);

	//when we get here we have an .ogf file saved out as tempfilename, we need to load it back up
	char blah[_MAX_PATH];
	StripCRCFileName(pathname,blah);	//make sure all CRC are out of the filename
	ddio_SplitPath(blah,NULL,filename,ext);
	if(!CreateCRCFileName(tempfilename,filename,p)){
		mprintf((0,"Error creating CRC File\n"));
		ddio_DeleteFile (tempfilename);
		return false;
	}

	strcpy(filename,p);
	strcat(filename,".ogf");
	
	ddio_MakePath(p,LocalCustomGraphicsDir,filename,NULL);

	//p contains the real filename
	//tempfilename contains old filename
	bm_handle = bm_AllocLoadFileBitmap(IGNORE_TABLE(tempfilename),0);
	if(bm_handle<=BAD_BITMAP_HANDLE){
		mprintf((0,"Error reloading bitmap for rename\n"));
		ddio_DeleteFile (tempfilename);
		return false;
	}

	if(bm_SaveFileBitmap (p,bm_handle)==-1){
		mprintf((0,"Error importing\n"));
		bm_FreeBitmap(bm_handle);
		ddio_DeleteFile (tempfilename);
		return false;
	}

	bm_FreeBitmap(bm_handle);
	ddio_DeleteFile (tempfilename);

	strcpy(newfile,p);
	return true;
}

//	UpdateGraphicsListbox
//
//	Updates the listbox that contains the custom textures. Make sure all double-pointer arguments passes in
//	are set to NULL is they don't have memory allocated for them yet, because if they're not null this function
//	will try to free up their memory
bool UpdateGraphicsListbox(tCustomListInfo *cust_bmps,newuiListBox *lb,char *selected_name)
{
	ASSERT(lb);
	lb->RemoveAll();

	cust_bmps->Reset();
	cust_bmps->custom_bitmap_list = lb;

	//get a list of custom textures
	int count = 0;
	bool ok_to_get_files;
	int total_files;

	//build list
	char oldpath[_MAX_PATH];

	ddio_GetWorkingDir(oldpath,_MAX_PATH);

	if(ddio_SetWorkingDir(LocalCustomGraphicsDir))
		ok_to_get_files = true;
	else
		ok_to_get_files = false;

	if(ok_to_get_files){
		//Get all the filenames
		int totalsize,size;
		int count,totalcount;
	
		//.oaf
		//.ogf
		count = totalcount = totalsize = total_files = 0;
		
		totalsize += FindAllFilesSize("*.oaf",&count); totalcount+=count;
		totalsize += FindAllFilesSize("*.ogf",&count); totalcount+=count;

		if(totalsize){
			cust_bmps->needed_size = totalsize;
			total_files = totalcount;
			cust_bmps->files = (char *)mem_malloc(totalsize);
			if(cust_bmps->files){
				size = 0;
				count = 0;

				size += FindAllFiles("*.oaf",&cust_bmps->files[size],totalcount,&count); totalcount-=count;
				size += FindAllFiles("*.ogf",&cust_bmps->files[size],totalcount,&count); totalcount-=count;
			}else
				mprintf((0,"Unable to allocate memory for %d bytes\n",totalsize));
		}
	}
	
	count = total_files + 1;	//make room for "None"

	char chosen_name[256],ext[256];
	if(selected_name)
	{
		ddio_SplitPath(selected_name,NULL,chosen_name,ext);
		strcat(chosen_name,ext);
	}else
	{
		*chosen_name = '\0';
	}

	//default "None"
	lb->AddItem(TXT_LBNONE);
	
	int memory_used;
	memory_used = 0;
	for(int i=1;i<count;i++){
		char tempf[_MAX_PATH];
		StripCRCFileName(&cust_bmps->files[memory_used],tempf);
		lb->AddItem(tempf);
		if(!stricmp(&cust_bmps->files[memory_used],chosen_name))
		{
			lb->SetCurrentIndex(i);
			CustomCallBack(i);
		}

		memory_used += strlen(&cust_bmps->files[memory_used])+1;
	}

	// If we are not forcing a selection, choose none
	if(*chosen_name=='\0')
	{
		lb->SetCurrentIndex(0);
		CustomCallBack(0);
	}
	
	ddio_SetWorkingDir(oldpath);
	return true;
}

//	GetCustomSoundFiles
//	
//	Looks in the custom/sound directory and retrieves all the wav files in there.  Returns the size of the buffer
//	needed to get all the needed files.  The filenames are stored in the buffer, seperated by \0, use count to 
//	determine how many were placed in the buffer
//
//	buffer	=	Your buffer passed in, if this is NULL then it just returns a buffer size that is needed
//	size	=	Size of your buffer passed in
//	*count	=	filled in with the number of files that were placed into your buffer
int GetCustomSoundFiles(char *buffer=NULL,int size=0,int *count=NULL);
int GetCustomSoundFiles(char *buffer,int size,int *count)
{
	int c = 0;
	int isize = 0;
	int len,overallsize=0;
	char tempname[_MAX_PATH];
	char oldpath[_MAX_PATH];
	ddio_GetWorkingDir(oldpath,_MAX_PATH);
	ddio_SetWorkingDir(LocalCustomSoundsDir);

	if(!buffer)
		size = 0;

	if(count)
		*count = 0;

	if(ddio_FindFileStart("*.osf",tempname)){
		len = strlen(tempname);
		overallsize += len + 1;
		if(isize+len<size){
			strcpy(buffer,tempname);
			isize += len+1;
			c++;
		}

		while(ddio_FindNextFile(tempname)){
			len = strlen(tempname);
			overallsize += len + 1;
			if(isize+len<size){
				strcpy(&buffer[isize],tempname);
				isize += len+1;
				c++;
			}
		}
	}
	ddio_FindFileClose();
	ddio_SetWorkingDir(oldpath);
	if(count)
		*count = c;
	return overallsize;
}

//	GetStringInList
//
//	Returns a pointer to the string in a string list (<string1>\0<string2>\0...).  Be VERY careful that
//	the index you pass in is a valid string index
//	index	=	Index of string you want
//	list	=	string list (a bunch of strings, seperated by \0)
//	maxsize =	size of string list buffer
//	returns NULL on error (if an error can be detected)
char *GetStringInList(int index,char *list,int maxsize)
{
	int count = 0;
	int p = 0;
	char *pp;
	pp = list;

	while(1){
		if(count==index)
			return pp;

		if(p<maxsize){
			if(*pp=='\0')
				count++;
		}else
			return NULL;		
		p++;
		pp++;
	}
	return NULL;
}

//	UpdateAudioTauntBoxes
//	
//	Resets and updates the list of audio taunt combo boxes
void UpdateAudioTauntBoxes(tCustomListInfo *cust_snds,newuiComboBox *audio1_list,newuiComboBox *audio2_list,newuiComboBox *audio3_list,newuiComboBox *audio4_list,pilot *Pilot)
{
	ASSERT(audio1_list);
	ASSERT(audio2_list);

	audio1_list->RemoveAll();
	audio2_list->RemoveAll();
	audio3_list->RemoveAll();
	audio4_list->RemoveAll();

	// free up allocated memory
	cust_snds->Reset();

	// Get all the audio files and put them into the lists
	cust_snds->needed_size = GetCustomSoundFiles();
	if(cust_snds->needed_size)
		cust_snds->files = (char *)mem_malloc(cust_snds->needed_size);
	else
		cust_snds->files = NULL;

	int audio_count;

	GetCustomSoundFiles(cust_snds->files,cust_snds->needed_size,&audio_count);

	int len;

	//Add <None> to both boxes
	audio1_list->AddItem(TXT_LBNONE);
	audio2_list->AddItem(TXT_LBNONE);
	audio3_list->AddItem(TXT_LBNONE);
	audio4_list->AddItem(TXT_LBNONE);

	int count = 0;

	char paudio1[PAGENAME_LEN],paudio2[PAGENAME_LEN];
	char paudio3[PAGENAME_LEN],paudio4[PAGENAME_LEN];
	Pilot->get_multiplayer_data(NULL,paudio1,paudio2,NULL,paudio3,paudio4);


	for(int i=0;i<audio_count;i++){
		char tfn[_MAX_PATH];
		len = strlen(&cust_snds->files[count]);
		StripCRCFileName(&cust_snds->files[count],tfn);

		audio1_list->AddItem((char *)(tfn));
		audio2_list->AddItem((char *)(tfn));
		audio3_list->AddItem((char *)(tfn));
		audio4_list->AddItem((char *)(tfn));

		if(!stricmp(paudio1,&cust_snds->files[count])){
			//set this as selected index for audio #1
			audio1_list->SetCurrentIndex(i+1);
		}
		if(!stricmp(paudio2,&cust_snds->files[count])){
			//set this as selected index for audio #2
			audio2_list->SetCurrentIndex(i+1);
		}
		if(!stricmp(paudio3,&cust_snds->files[count])){
			//set this as selected index for audio #1
			audio3_list->SetCurrentIndex(i+1);
		}
		if(!stricmp(paudio4,&cust_snds->files[count])){
			//set this as selected index for audio #2
			audio4_list->SetCurrentIndex(i+1);
		}
		count+=len+1;
	}
}

pilot *AudioTauntPilot;
tCustomListInfo *AudioTauntPilotSndInfo;
void audio1changecallback(int index)
{
	if(index==0){
		AudioTauntPilot->set_multiplayer_data(NULL,"");
		return;
	}else{
		index--;
		char *p = GetStringInList(index,AudioTauntPilotSndInfo->files,AudioTauntPilotSndInfo->needed_size);
		if(p){
			AudioTauntPilot->set_multiplayer_data(NULL,p);
		}
	}
}

void audio2changecallback(int index)
{
	if(index==0){
		AudioTauntPilot->set_multiplayer_data(NULL,NULL,"");
		return;
	}else{
		index--;
		char *p = GetStringInList(index,AudioTauntPilotSndInfo->files,AudioTauntPilotSndInfo->needed_size);
		if(p){
			AudioTauntPilot->set_multiplayer_data(NULL,NULL,p);
		}
	}
}

void audio3changecallback(int index)
{
	if(index==0){
		AudioTauntPilot->set_multiplayer_data(NULL,NULL,NULL,NULL,"");
		return;
	}else{
		index--;
		char *p = GetStringInList(index,AudioTauntPilotSndInfo->files,AudioTauntPilotSndInfo->needed_size);
		if(p){
			AudioTauntPilot->set_multiplayer_data(NULL,NULL,NULL,NULL,p);
		}
	}
}

void audio4changecallback(int index)
{
	if(index==0){
		AudioTauntPilot->set_multiplayer_data(NULL,NULL,NULL,NULL,NULL,"");
		return;
	}else{
		index--;
		char *p = GetStringInList(index,AudioTauntPilotSndInfo->files,AudioTauntPilotSndInfo->needed_size);
		if(p){
			AudioTauntPilot->set_multiplayer_data(NULL,NULL,NULL,NULL,NULL,p);
		}
	}
}

#define TAUNT_MENU_W	512
#define TAUNT_MENU_H	384

#define MAX_MULTIPLAYER_TAUNTS	8
#define TAUNT_EDIT_WIDTH			400


void DoPilotTauntScreen(pilot *plt)
{
	newuiTiledWindow taunt_wnd;
	newuiSheet *sheet;
	char *taunt_edit[MAX_PILOT_TAUNTS];
	int cury = 10;
	bool exit_menu=false;
	int i;

	//create window
	taunt_wnd.Create(TXT_MULTIPLAYER_TAUNTS, 0,0, TAUNT_MENU_W, TAUNT_MENU_H);
	sheet = taunt_wnd.GetSheet();

	//Ok/Cancel buttons
	for (i=0;i<MAX_PILOT_TAUNTS;i++)
	{
		char str[70];

		sprintf (str,"%s%d",TXT_TAUNT_NUMBER,i+1);
		sheet->NewGroup(str, 10,cury);

		taunt_edit[i] = sheet->AddEditBox(str, PILOT_TAUNT_SIZE, TAUNT_EDIT_WIDTH);
		strcpy(taunt_edit[i], plt->taunts[i]);
		cury+= 32;
	}


	sheet->NewGroup(NULL, TAUNT_MENU_W-210, TAUNT_MENU_H - 100, NEWUI_ALIGN_HORIZ);
	sheet->AddButton(TXT_OK, UID_OK);
	sheet->AddButton(TXT_CANCEL, UID_CANCEL);

	taunt_wnd.Open();

	while (!exit_menu) 
	{
		int res = taunt_wnd.DoUI();

		// handle all UI results.
		switch (res) 
		{
			case UID_OK:
				for (i=0;i<MAX_PILOT_TAUNTS;i++)
				{
					strcpy(plt->taunts[i], taunt_edit[i]);
				//	taunt_edit[i].GetText(plt->taunts[i],PILOT_TAUNT_SIZE);
				}
				exit_menu = true;
				break;
			case UID_CANCEL:
				exit_menu = true;
				break;
			default:
				break;
		}
	}

	taunt_wnd.Close();
	taunt_wnd.Destroy();
}

bool PltSelectShip(pilot *Pilot)
{
#define IDP_SHPLIST	19
#define ID_GETANIM  20
#define ID_IMPORT	21
#define ID_PLAY1	22
#define ID_PLAY2	23
#define ID_PLAY3	24
#define ID_PLAY4	25

#define ID_IMPORTSOUND	26
#define ID_TAUNTS		27
#define ID_DEL_LOGO		28
#define ID_DEL_TAUNTA	29
#define ID_DEL_TAUNTB	30
#define ID_DEL_TAUNTC	31
#define ID_DEL_TAUNTD	32
#define PSSELECT_WND_H	384
#define PSSELECT_WND_W	512

	//	variable declarations
	newuiTiledWindow window;
	newuiSheet *sheet;
	newuiListBox *custom_list;
	newuiComboBox *ship_list;

	int count;
	char oldt1[PILOT_TAUNT_SIZE],oldt2[PILOT_TAUNT_SIZE];
	char oldt3[PILOT_TAUNT_SIZE],oldt4[PILOT_TAUNT_SIZE];

	DoWaitMessage(true);

	Pilot->get_multiplayer_data(NULL,oldt1,oldt2,NULL,oldt3,oldt4);
	
	tCustomListInfo cust_snds;
	tAudioTauntComboBoxes taunts_lists;
	AudioTauntPilot = Pilot;
	AudioTauntPilotSndInfo = &cust_snds;

	UI3DWindow ship_win;
	UIBmpWindow bmp_win;
	tCustomListInfo	cust_bmps;
	tShipListInfo ship_info;

	int old_bmhandle;
	int old_flags;
	bool exit_menu = false;
	bool ret;

	//	pre-initialize all variables
	cust_bmps.Init();
	cust_snds.Init();

	lp_cust_bmps = &cust_bmps;
	lp_ship_info = &ship_info;

	ship_pos.Init();
	ship_info.Init();
	
	custom_texture[0]='\0';
	old_bmhandle = GameTextures[ship_pos.texture_id].bm_handle;
	old_flags = GameTextures[ship_pos.texture_id].flags;

	//	create UIWindow
	window.Create(TXT_MULTISELECTSHIP,0,0,PSSELECT_WND_W,PSSELECT_WND_H);
	sheet = window.GetSheet();

	sheet->NewGroup(TXT_DEFAULTMSHIPS,0,0);
	ship_list = sheet->AddComboBox(IDP_SHPLIST,0);

	sheet->NewGroup(TXT_CUSTOMTEXTURES,0,36);
	cust_bmps.custom_bitmap_list = custom_list = sheet->AddListBox(200,96,IDP_SHPLIST,0);

	sheet->NewGroup(NULL,230,130);
	sheet->AddButton(TXT_DELETE,ID_DEL_LOGO);

	sheet->NewGroup(TXT_AUDIOTAUNTA,0,155);
	taunts_lists.taunt_a = sheet->AddComboBox(-5,0);

	sheet->NewGroup(TXT_AUDIOTAUNTB,0,192);
	taunts_lists.taunt_b = sheet->AddComboBox(-5,0);

	sheet->NewGroup(TXT_AUDIOTAUNTC,0,229);
	taunts_lists.taunt_c = sheet->AddComboBox(-5,0);

	sheet->NewGroup(TXT_AUDIOTAUNTD,0,266);
	taunts_lists.taunt_d = sheet->AddComboBox(-5,0);

	//Ship window
	ship_win.Create(&window,&UITextItem(""),290,50,180,140,0);

	//Bitmap display of selected logo
	bmpwindow = &bmp_win;
	bmp_win.Create(&window,&UITextItem(""),UI_BORDERSIZE+200,53,42,42,0);

	// Get all the audio files and put them into the lists
	UpdateAudioTauntBoxes(&cust_snds,taunts_lists.taunt_a,taunts_lists.taunt_b,taunts_lists.taunt_c,taunts_lists.taunt_d,Pilot);

	DoWaitMessage(true);

	// Setup hotspots for audio taunt commands
	sheet->NewGroup(NULL,170,163);
	sheet->AddButton(TXT_PLAYSOUND,ID_PLAY1);
	sheet->AddButton(TXT_DELETE,ID_DEL_TAUNTA);
	
	sheet->NewGroup(NULL,170,200);
	sheet->AddButton(TXT_PLAYSOUND,ID_PLAY2);
	sheet->AddButton(TXT_DELETE,ID_DEL_TAUNTB);

	sheet->NewGroup(NULL,170,237);
	sheet->AddButton(TXT_PLAYSOUND,ID_PLAY3);
	sheet->AddButton(TXT_DELETE,ID_DEL_TAUNTC);

	sheet->NewGroup(NULL,170,274);
	sheet->AddButton(TXT_PLAYSOUND,ID_PLAY4);
	sheet->AddButton(TXT_DELETE,ID_DEL_TAUNTD);

	//setup callback function
	custom_list->SetSelectChangeCallback(CustomCallBack);
	ship_list->SetSelectChangeCallback(ShipSelectCallBack);
	taunts_lists.taunt_a->SetSelectChangeCallback(audio1changecallback);
	taunts_lists.taunt_b->SetSelectChangeCallback(audio2changecallback);
	taunts_lists.taunt_c->SetSelectChangeCallback(audio3changecallback);
	taunts_lists.taunt_d->SetSelectChangeCallback(audio4changecallback);

	int i;
	//get all the ship ids
	count = 0;
	for(i=0;i<MAX_SHIPS;i++){
		if(Ships[i].used)
			count++;
	}
	ship_info.idlist = (int *)mem_malloc(sizeof(int)*count);
	if(!ship_info.idlist)
		goto ship_id_err;

	bool found_ship;
	bool shipoktoadd;
	
	found_ship = false;
	count = 0;
	char pship[PAGENAME_LEN];
	Pilot->get_ship(pship);

	for(i=0;i<MAX_SHIPS;i++){
		if(Ships[i].used){
			ship_info.idlist[count] = i;
#ifdef DEMO
			if(strcmpi(Ships[i].name,DEFAULT_SHIP)==0)
			{
#endif
				// Non-Windows versions don't have to check
				if(!stricmp(Ships[i].name,"Black Pyro") && !MercInstalled())
				{
					shipoktoadd = false;
				}
				else
				{
					shipoktoadd = true;
				}
				if(shipoktoadd)
					ship_list->AddItem(Ships[i].name);
#ifdef DEMO
			}
#endif

			if(!stricmp(pship,Ships[i].name)){
				ship_list->SetCurrentIndex(count);
				found_ship = true;
			}
			count++;
		}
	}

	//if we couldn't find the ship than that means that the ship in their pilot file no longer
	//exists and so we must set it to the default ship
	if(!found_ship){
		DoMessageBox(TXT_WARNING,TXT_SHIPSELECTERR,MSGBOX_OK);
		if(count>0){
			//find the ship in the page
			int index = FindShipName(DEFAULT_SHIP);
			if(index==-1){
				mprintf((0,"WARNING: CAN'T FIND DEFAULT SHIP IN TABLE\n"));
			}else{
				//go through all the id's of the ships we found and find the ship (if FindShipName found it, 
				//then we'll have it here somewhere.
				for(int i=0;i<count;i++){
					if(ship_info.idlist[i]==index){
						//set this as the default ship
						ship_list->SetCurrentIndex(i);
						
					}//endif
				}//endfor
			}//end else
		}else{
			//NO SHIPS IN THE TABLE!!!
			mprintf((0,"WARNING: NO SHIPS IN THE TABLE!?\n"));
		}
	}

	//	get a list of custom textures
	char pshiplogo[PAGENAME_LEN];
	Pilot->get_multiplayer_data(pshiplogo);

	DoWaitMessage(true);

	if(!UpdateGraphicsListbox(&cust_bmps,custom_list,pshiplogo))
		goto ship_id_err;

	i = ship_list->GetCurrentIndex();
	ShipSelectCallBack(i);

	//	setup hotspots
	sheet->NewGroup(NULL,250,170);
	sheet->AddLongButton(TXT_IMPORTGRAPHIC,ID_IMPORT);
	sheet->AddLongButton(TXT_IMPORTIFL,ID_GETANIM);
	sheet->AddLongButton(TXT_MULTIPLAYER_TAUNTS,ID_TAUNTS);
	sheet->AddLongButton(TXT_IMPORTSOUND,ID_IMPORTSOUND);
	sheet->NewGroup(NULL,260,281,NEWUI_ALIGN_HORIZ);
	sheet->AddButton(TXT_OK,UID_OK);
	sheet->AddButton(TXT_CANCEL,UID_CANCEL);

	//Ok, all initial setup is finally complete, time to display the window and process input
	ret = false;

	DoWaitMessage(false);

	window.Open();
	while (!exit_menu) 
	{
		int res = window.DoUI();

		// handle all UI results.
		switch (res){
		case UID_OK:
			{
				char tbuf[64];
				ship_list->GetCurrentItem(tbuf,64);

				Pilot->set_ship(tbuf);
				Pilot->set_multiplayer_data(custom_texture);

				//Retrieve Audio taunts
				Pilot->set_multiplayer_data(NULL,"","",NULL,"","");
				
				int index;
				index = taunts_lists.taunt_a->GetCurrentIndex();
				if(index>0 && cust_snds.files){
					char *p = GetStringInList(index-1,cust_snds.files,cust_snds.needed_size);
					if(p){
						Pilot->set_multiplayer_data(NULL,p);
					}
				}
				index = taunts_lists.taunt_b->GetCurrentIndex();
				if(index>0 && cust_snds.files){
					char *p = GetStringInList(index-1,cust_snds.files,cust_snds.needed_size);
					if(p){
						Pilot->set_multiplayer_data(NULL,NULL,p);
					}
				}

				index = taunts_lists.taunt_c->GetCurrentIndex();
				if(index>0 && cust_snds.files){
					char *p = GetStringInList(index-1,cust_snds.files,cust_snds.needed_size);
					if(p){
						Pilot->set_multiplayer_data(NULL,NULL,NULL,NULL,p);
					}
				}

				index = taunts_lists.taunt_d->GetCurrentIndex();
				if(index>0 && cust_snds.files){
					char *p = GetStringInList(index-1,cust_snds.files,cust_snds.needed_size);
					if(p){
						Pilot->set_multiplayer_data(NULL,NULL,NULL,NULL,NULL,p);
					}
				}

				char tempn[PAGENAME_LEN];

				Pilot->get_multiplayer_data(NULL,tempn);
				mprintf((0,"Audio #1: '%s'\n",tempn));

				Pilot->get_multiplayer_data(NULL,NULL,tempn);
				mprintf((0,"Audio #2: '%s'\n",tempn));

				Pilot->get_multiplayer_data(NULL,NULL,NULL,NULL,tempn);
				mprintf((0,"Audio #3: '%s'\n",tempn));

				Pilot->get_multiplayer_data(NULL,NULL,NULL,NULL,NULL,tempn);
				mprintf((0,"Audio #4: '%s'\n",tempn));

				ret = true;
				exit_menu = true;
			}break;
		case UID_CANCEL:
			ret = false;
			exit_menu = true;
			GameTextures[ship_pos.texture_id].bm_handle = old_bmhandle;
			GameTextures[ship_pos.texture_id].flags = old_flags;			
			Pilot->set_multiplayer_data(NULL,oldt1,oldt2,NULL,oldt3,oldt4);
			break;
		case ID_IMPORT:
			{
				char path[_MAX_PATH];
				char newf[_MAX_FNAME];
				char wildcards[100];
				path[0] = '\0';
				strcpy(wildcards,"*.ogf;*.tga;*.pcx;*.iff");
				if(DoPathFileDialog(false,path,TXT_CHOOSE,wildcards,PFDF_FILEMUSTEXIST)){
					if(ImportGraphic(path,newf)){
						//update the listbox
						if(!UpdateGraphicsListbox(&cust_bmps,custom_list,newf))
							goto ship_id_err;
					}else{
						DoMessageBox(TXT_ERROR,TXT_ERRORIMPORT,MSGBOX_OK);
					}
				}
				ddio_SetWorkingDir(path);
			}
			break;
		case ID_GETANIM:
			{
				char path[_MAX_PATH];
				char opath[_MAX_PATH];
				char wildcards[100];
				path[0] = '\0';
				strcpy(opath,path);
				strcpy(wildcards,"*.ifl");
				if(DoPathFileDialog(false,path,TXT_CHOOSE,wildcards,PFDF_FILEMUSTEXIST)){
					int handle = AllocLoadIFLVClip (IGNORE_TABLE(path),SMALL_TEXTURE,1);

					if(handle!=-1){
						//change the file extension
						char f[_MAX_FNAME],newf[_MAX_FNAME+_MAX_EXT],tempf[_MAX_PATH];
						if(!CreateCRCFileName(path,tempf)){
							FreeVClip(handle);
							break;
						}
						ddio_SplitPath(tempf,NULL,f,NULL);
						strcpy(newf,f);
						strcat(newf,".oaf");
						ddio_MakePath(path,LocalCustomGraphicsDir,newf,NULL);
						
						if(SaveVClip (path,handle)==0){
							//error saving
							DoMessageBox(TXT_ERROR,TXT_ERRORIMPORT,MSGBOX_OK);
						}else{
							//all went ok!!!
							DoMessageBox(TXT_SUCCESS,TXT_SUCCESSIMPORT,MSGBOX_OK);
						}
						FreeVClip(handle);

						//check file size...make sure it isn't too big
						CFILE *file = cfopen(path,"rb");						
						if(file)
						{
							if(cfilelength(file)>MAX_AUDIOTAUNTSIZE)
							{
								//file too big!!!!!!
								char message[256];
								sprintf(message,TXT_COMPRESSTOOBIG,MAX_AUDIOTAUNTSIZE/1024);
								DoMessageBox(TXT_WARNING,message,MSGBOX_OK);
							}
							cfclose(file);
						}

						//update the listbox
						if(!UpdateGraphicsListbox(&cust_bmps,custom_list,newf))
							goto ship_id_err;
					}
				}
				ddio_SetWorkingDir(opath);
			}
			break;
		case ID_PLAY1:
			{
				//	Play audio taunt #1 if <None> isn't selected
				char path[_MAX_PATH];
				int index = taunts_lists.taunt_a->GetCurrentIndex();
				if(index>0 && cust_snds.files){
					char *p = GetStringInList(index-1,cust_snds.files,cust_snds.needed_size);
					if(p){
						ddio_MakePath(path,LocalCustomSoundsDir,p,NULL);
						mprintf((0,"Playing: %s\n",path));
						bool cenable = taunt_AreEnabled();
						taunt_Enable(true);
						taunt_PlayTauntFile(path);
						taunt_Enable(cenable);
					}
				}
			}break;
		case ID_PLAY2:
			{
				//	Play audio taunt #2 if <None> isn't selected
				char path[_MAX_PATH];
				int index = taunts_lists.taunt_b->GetCurrentIndex();
				if(index>0 && cust_snds.files){
					char *p = GetStringInList(index-1,cust_snds.files,cust_snds.needed_size);
					if(p){
						ddio_MakePath(path,LocalCustomSoundsDir,p,NULL);
						mprintf((0,"Playing: %s\n",path));
						bool cenable = taunt_AreEnabled();
						taunt_Enable(true);
						taunt_PlayTauntFile(path);
						taunt_Enable(cenable);
					}
				}
			}break;
		case ID_PLAY3:
			{
				//	Play audio taunt #3 if <None> isn't selected
				char path[_MAX_PATH];
				int index = taunts_lists.taunt_c->GetCurrentIndex();
				if(index>0 && cust_snds.files){
					char *p = GetStringInList(index-1,cust_snds.files,cust_snds.needed_size);
					if(p){
						ddio_MakePath(path,LocalCustomSoundsDir,p,NULL);
						mprintf((0,"Playing: %s\n",path));
						bool cenable = taunt_AreEnabled();
						taunt_Enable(true);
						taunt_PlayTauntFile(path);
						taunt_Enable(cenable);
					}
				}
			}break;

		case ID_PLAY4:
			{
				//	Play audio taunt #4 if <None> isn't selected
				char path[_MAX_PATH];
				int index = taunts_lists.taunt_d->GetCurrentIndex();
				if(index>0 && cust_snds.files){
					char *p = GetStringInList(index-1,cust_snds.files,cust_snds.needed_size);
					if(p){
						ddio_MakePath(path,LocalCustomSoundsDir,p,NULL);
						mprintf((0,"Playing: %s\n",path));
						bool cenable = taunt_AreEnabled();
						taunt_Enable(true);
						taunt_PlayTauntFile(path);
						taunt_Enable(cenable);
					}
				}
			}break;

		case ID_TAUNTS:
		{
			DoPilotTauntScreen(Pilot);
			break;
		}
		case ID_IMPORTSOUND:
			{
				//Import the sound, set it's sample to xx.xKhz and xbit depth, attach the CRC to the filename
				//and place in custom/sounds.  Then update the audio taunt combo boxes
				char path[_MAX_PATH];
				char wildcards[100];
				path[0] = '\0';
				strcpy(wildcards,"*.wav");
				if(DoPathFileDialog(false,path,TXT_CHOOSE,wildcards,PFDF_FILEMUSTEXIST))
				{
					char dpath[_MAX_PATH*2];
					char filename[_MAX_PATH];
					char tempfile[_MAX_PATH];

					ddio_SplitPath(path,NULL,filename,NULL);
					strcat(filename,".osf");
					ddio_MakePath(tempfile,LocalCustomSoundsDir,filename,NULL);

					//import the sound	
					mprintf((0,"Importing: '%s'->'%s'\n",path,tempfile));
					if(taunt_ImportWave(path,tempfile)){
						//success

						//check file size...make sure it isn't too big
						CFILE *file = cfopen(tempfile,"rb");
						
						if(file) {

							if(cfilelength(file)>MAX_AUDIOTAUNTSIZE)
							{
								//file too big!!!!!!
								char message[256];
								sprintf(message,TXT_COMPRESSTOOBIG,MAX_AUDIOTAUNTSIZE/1024);
								DoMessageBox(TXT_WARNING,message,MSGBOX_OK);
							}

							cfclose(file);

							if(CreateCRCFileName(tempfile,dpath)){
								//dpath is destination file
								//tempfile is source file
								if(cf_CopyFile(dpath,tempfile,0)){
									//file copied...
									DoMessageBox(TXT_SUCCESS,TXT_AUDIOIMPORTSUC,MSGBOX_OK);
									//delete temp file
									ddio_DeleteFile(tempfile);
									//Put path in the custom\sounds as dpath (convert if needed)
									UpdateAudioTauntBoxes(&cust_snds,taunts_lists.taunt_a,taunts_lists.taunt_b,taunts_lists.taunt_c,taunts_lists.taunt_d,Pilot);
								}else{
									DoMessageBox(TXT_ERROR,TXT_COPYTEMPERR,MSGBOX_OK);
									//delete temp file
									ddio_DeleteFile(tempfile);
								}
							}else{
								//couldn't generate a crc filename
								DoMessageBox(TXT_ERROR,TXT_CANTCREATECRC,MSGBOX_OK);
								//delete temp file
								ddio_DeleteFile(tempfile);
							}
						}//end else (file size)
					}else{
						int imp_err = taunt_GetError();
						char *err = taunt_GetErrorString(imp_err);
						//failure
						DoMessageBox(TXT_ERROR,err,MSGBOX_OK);
					}
				}
				ddio_SetWorkingDir(path);
			}break;

		case ID_DEL_LOGO:
			{
				ShipSelectDeleteLogo(&cust_bmps,custom_list);
			}break;

		case ID_DEL_TAUNTA:
			{
				ShipSelectDeleteTaunt(Pilot,&cust_snds,taunts_lists.taunt_a,&taunts_lists);
			}break;

		case ID_DEL_TAUNTB:
			{
				ShipSelectDeleteTaunt(Pilot,&cust_snds,taunts_lists.taunt_b,&taunts_lists);
			}break;

		case ID_DEL_TAUNTC:
			{
				ShipSelectDeleteTaunt(Pilot,&cust_snds,taunts_lists.taunt_c,&taunts_lists);
			}break;

		case ID_DEL_TAUNTD:
			{
				ShipSelectDeleteTaunt(Pilot,&cust_snds,taunts_lists.taunt_d,&taunts_lists);
			}break;
		}
	}

	
ship_id_err:

	//no use for this since stream files will stop
	//Sound_system.StopAllSounds();

	taunts_lists.taunt_a = NULL;
	taunts_lists.taunt_b = NULL;
	taunts_lists.taunt_c = NULL;
	taunts_lists.taunt_d = NULL;

	cust_snds.Reset();

	cust_bmps.Reset();
	ship_info.Reset();

	if( (ret)&&(ship_pos.bm_handle<=BAD_BITMAP_HANDLE) ){
		GameTextures[ship_pos.texture_id].flags&=~TF_ANIMATED;
		GameTextures[ship_pos.texture_id].flags&=~TF_TEXTURE_32;
		GameTextures[ship_pos.texture_id].flags|=TF_TEXTURE_64;
		GameTextures[ship_pos.texture_id].bm_handle=BAD_BITMAP_HANDLE;
	}

	if(ship_pos.bm_handle>BAD_BITMAP_HANDLE){
		int handle;
		bool anim;
		if(ret){
			handle = old_bmhandle;
			anim = (bool)((old_flags&TF_ANIMATED)!=0);
		}else{
			handle = ship_pos.bm_handle;
			anim = (bool)(ship_pos.texture_type>0);
		}
		if(handle>BAD_BITMAP_HANDLE){
			if(anim)
				FreeVClip(handle);
			else
				bm_FreeBitmap(handle);
		}
	}
	ship_pos.bm_handle = -1;

	window.Close();
	window.Destroy();

	return ret;
}


void CustomCallBack(int c)
{
	char select_bitmap[_MAX_PATH];
	char *sbmp = GetStringInList(c-1,lp_cust_bmps->files,lp_cust_bmps->needed_size);
	if(sbmp)
		strcpy(select_bitmap,sbmp);
	else
		select_bitmap[0] = '\0';

	if(!strcmp(custom_texture,select_bitmap))
		return;

	if(ship_pos.bm_handle>BAD_BITMAP_HANDLE){
		if(ship_pos.texture_type)
			FreeVClip(ship_pos.bm_handle);
		else
			bm_FreeBitmap(ship_pos.bm_handle);
		ship_pos.bm_handle = -1;
	}
	ship_pos.texture_changed = true;

	if(c==0){
		//None selected
		custom_texture[0]='\0';
		mprintf((0,"None selected\n"));
		bmpwindow->SetInfo(false,-1);
		GameTextures[ship_pos.texture_id].flags&=~TF_ANIMATED;
		GameTextures[ship_pos.texture_id].flags&=~TF_TEXTURE_32;
		GameTextures[ship_pos.texture_id].flags|=TF_TEXTURE_64;
		GameTextures[ship_pos.texture_id].bm_handle=BAD_BITMAP_HANDLE;
	}else{
		//Get the filename
		char *p = GetStringInList(c-1,lp_cust_bmps->files,lp_cust_bmps->needed_size);
		if(p)
			strcpy(custom_texture,p);
		else
			custom_texture[0]='\0';
		ship_pos.bm_handle = LoadTextureImage (IGNORE_TABLE(custom_texture),&ship_pos.texture_type,SMALL_TEXTURE,1);
		if(ship_pos.bm_handle>BAD_BITMAP_HANDLE){
			if (ship_pos.texture_type)
				GameTextures[ship_pos.texture_id].flags|=TF_ANIMATED;
			else
				GameTextures[ship_pos.texture_id].flags&=~TF_ANIMATED;

			GameTextures[ship_pos.texture_id].flags&=~TF_TEXTURE_32;
			GameTextures[ship_pos.texture_id].flags|=TF_TEXTURE_64;
					
			GameTextures[ship_pos.texture_id].bm_handle=ship_pos.bm_handle;

			bmpwindow->SetInfo(ship_pos.texture_type?true:false,ship_pos.bm_handle);

			mprintf((0,"Loaded texture (%s).  Type=%d, ID=%d\n",custom_texture,ship_pos.texture_type,ship_pos.texture_id));
		}else
			goto load_texture_err;
	}

	return;

load_texture_err:
	ship_pos.bm_handle = BAD_BITMAP_HANDLE;
	GameTextures[ship_pos.texture_id].flags&=~TF_ANIMATED;
	GameTextures[ship_pos.texture_id].flags&=~TF_TEXTURE_32;
	GameTextures[ship_pos.texture_id].flags|=TF_TEXTURE_64;
	GameTextures[ship_pos.texture_id].bm_handle=BAD_BITMAP_HANDLE;
	strcpy(custom_texture,"");
	mprintf((0,"Unable to load texture\n"));
	bmpwindow->SetInfo(false,-1);
}

extern float Render_zoom;
void ShipSelectCallBack(int c)
{
	if(!lp_ship_info->idlist)
		return;

	float size;
	ship_model = Ships[lp_ship_info->idlist[c]].model_handle;
	if(ship_model==-1){
		mprintf((0,"ship_model is -1\n"));
		Int3();
	}

	DoWaitMessage(true);

	PageInPolymodel (ship_model, OBJ_PLAYER, &size);
	poly_model *pm = GetPolymodelPointer(ship_model);

	ship_pos.cam_dist = pm->anim_size / Render_zoom + 5;

	DoWaitMessage(false);
}

extern char LocalCustomGraphicsDir[TABLE_NAME_LEN];
extern char LocalCustomSoundsDir[TABLE_NAME_LEN];

// Deletes the currently selected ship logo
void ShipSelectDeleteLogo(tCustomListInfo *cust_bmps,newuiListBox *lb)
{
	ASSERT(lb);
	ASSERT(cust_bmps);
	
	int selected_index = lb->GetCurrentIndex();
	char custom_filename[384];
	char custom_logoname[384];
	
	//check for None selected
	if(selected_index==0)
	{
		mprintf((0,"Listbox selected item is None\n"));
		return;
	}

	lb->GetItem(selected_index,custom_logoname,384);

	//Get the filename
	char *p = GetStringInList(selected_index-1,cust_bmps->files,cust_bmps->needed_size);
	if(p)
	{
		strncpy(custom_filename,p,383);
		custom_filename[383] = '\0';
	}else
	{
		mprintf((0,"Listbox selected item not found\n"));
		Int3();
		return;
	}

	//delete custom_filename, we don't want it....
	char buffer[512];
	sprintf(buffer,TXT_PLTOKDEL,custom_logoname);
	if(DoMessageBox(TXT_PLTDELCONF,buffer,MSGBOX_YESNO,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL))
	{
		mprintf((0,"Deleting pilot logo %s (%s)\n",custom_logoname,custom_filename));

		char olddir[_MAX_PATH];
		ddio_GetWorkingDir(olddir,_MAX_PATH);
		ddio_SetWorkingDir(LocalCustomGraphicsDir);
		if(ddio_DeleteFile(custom_filename))
		{
			// Update the list box, select none
			UpdateGraphicsListbox(cust_bmps,lb,NULL);
		}else
		{
			mprintf((0,"Unable to delete file %s\n",custom_filename));
			Int3();
		}
		ddio_SetWorkingDir(olddir);
	}
}

// Deletes the currently selected audio taunt
void ShipSelectDeleteTaunt(pilot *Pilot,tCustomListInfo *cust_snds,newuiComboBox *lb,tAudioTauntComboBoxes *taunt_boxes)
{
	ASSERT(Pilot);
	ASSERT(cust_snds);
	ASSERT(taunt_boxes);

	int selected_index = lb->GetCurrentIndex();
	char custom_filename[384];
	char custom_logoname[384];
	
	//check for None selected
	if(selected_index==0)
	{
		mprintf((0,"Listbox selected item is None\n"));
		return;
	}

	lb->GetItem(selected_index,custom_logoname,384);

	//Get the filename
	char *p = GetStringInList(selected_index-1,cust_snds->files,cust_snds->needed_size);
	if(p)
	{
		strncpy(custom_filename,p,383);
		custom_filename[383] = '\0';
	}else
	{
		mprintf((0,"Listbox selected item not found\n"));
		Int3();
		return;
	}

	//delete custom_filename, we don't want it....
	char buffer[512];
	sprintf(buffer,TXT_PLTOKDEL,custom_logoname);
	if(DoMessageBox(TXT_PLTDELCONF,buffer,MSGBOX_YESNO,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL))
	{
		mprintf((0,"Deleting audio taunt %s (%s)\n",custom_logoname,custom_filename));

		char olddir[_MAX_PATH];
		ddio_GetWorkingDir(olddir,_MAX_PATH);
		ddio_SetWorkingDir(LocalCustomSoundsDir);
		if(ddio_DeleteFile(custom_filename))
		{
			// Update the list boxes, select none
			UpdateAudioTauntBoxes(cust_snds,taunt_boxes->taunt_a,taunt_boxes->taunt_b,taunt_boxes->taunt_c,taunt_boxes->taunt_d,Pilot);
		}else
		{
			mprintf((0,"Unable to delete file %s\n",custom_filename));
			Int3();
		}
		ddio_SetWorkingDir(olddir);
	}
}

void UI3DWindow::OnDraw()
{	
	AudioStream::Frame();

	static polymodel_effect pefx;

	vector viewer_eye = {0,0,0};
	matrix viewer_orient = IDENTITY_MATRIX;
	viewer_eye.z = -ship_pos.cam_dist;

//	3d start frame
	//@@@@@StartFrame(m_X,m_Y,m_X+m_W-1,m_Y+m_H-1);
	g3_StartFrame(&viewer_eye,&viewer_orient,D3_DEFAULT_ZOOM);
	rend_SetFlatColor (0);

	if(ship_model==-1){
		mprintf((0,"Shipmodel is -1\n"));
		return;
	}

	float normalized_time[MAX_SUBOBJECTS];
	float light_scalar,size;
	PageInPolymodel (ship_model, OBJ_PLAYER, &size);
	poly_model *pm = GetPolymodelPointer(ship_model);
	vector view_pos;
	vector light_vec;
	matrix view_orient = IDENTITY_MATRIX;
	matrix rot_mat = IDENTITY_MATRIX;
	matrix final_mat = IDENTITY_MATRIX;

//	draw model.
	SetNormalizedTimeAnim(0, normalized_time, pm);

	view_pos = pm->anim_size_offset;

	//move 30 degrees a sec
	float new_time = timer_GetTime();

	vm_AnglesToMatrix(&rot_mat,0,(new_time - ship_pos.last_time)*(65535/360)*30,0);
	vm_MatrixMul (&view_orient,&rot_mat,&ship_pos.last_frame);
	vm_Orthogonalize(&view_orient);
	ship_pos.last_frame = view_orient;

	
	light_vec.x = 0.0f; 	light_vec.y = -1.0f; 	light_vec.z = -1.0f; 
	light_scalar = 0.8f;
	vm_NormalizeVector(&light_vec);

	rend_SetZBufferState(1);
	rend_SetAlphaType (AT_CONSTANT_TEXTURE);
	rend_SetAlphaValue (255);
	rend_SetLighting (LS_NONE);
	rend_SetColorModel (CM_MONO);
	rend_SetOverlayType (OT_NONE);

	if(ship_pos.bm_handle>BAD_BITMAP_HANDLE){
		if(ship_pos.texture_changed){
			memset(&pefx,0,sizeof(pefx));
			pefx.custom_texture = ship_pos.texture_id;
			pefx.type = PEF_CUSTOM_TEXTURE;
			ship_pos.texture_changed = false;
		}

		SetPolymodelEffect (&pefx);

		DrawPolygonModel(&view_pos, &view_orient, ship_model, normalized_time,0, &light_vec, light_scalar,light_scalar,light_scalar,0xFFFFFFFF,1);
	}else{
		DrawPolygonModel(&view_pos, &view_orient, ship_model, normalized_time,0, &light_vec, light_scalar,light_scalar,light_scalar);
	}

	g3_EndFrame();
	//@@@@@@@@@@@EndFrame();

	UIStatic::OnDraw();

	ship_pos.last_time = new_time;
}

UIBmpWindow::UIBmpWindow()
{
	animated = false;
	bm_handle = -1;
	start_time = timer_GetTime();
	created = false;
}

UIBmpWindow::~UIBmpWindow()
{
}

void UIBmpWindow::SetInfo(bool anim,int handle)
{
	animated = anim;
	bm_handle = handle;
	start_time = timer_GetTime();
}

void UIBmpWindow::DrawBorder(void)
{
	int minx,maxx,miny,maxy;

	minx = 0;
	maxx = m_W-1;
	miny = 0;
	maxy = m_H-1;

	rend_SetZBufferState(0);
	rend_SetFlatColor(GR_RGB(40,255,40));

	rend_DrawLine(minx,miny,maxx,miny);
	rend_DrawLine(minx,maxy,maxx,maxy);
	rend_DrawLine(minx,miny,minx,maxy);
	rend_DrawLine(maxx,miny,maxx,maxy);

	rend_SetZBufferState(1);
}

void UIBmpWindow::OnDraw(void)
{
	int bmh = -1;

	//@@@@@StartFrame(m_Wnd->X(),m_Wnd->Y(),m_Wnd->X()+m_Wnd->W(),m_Wnd->Y()+m_Wnd->H());

	if(bm_handle==-1){
		UIStatic::OnDraw();
		DrawBorder();
		//@@@@@@@@@@@EndFrame();
		return;
	}

	if(animated){
		vclip *vc = &GameVClips[bm_handle];

		float elapsed_time = timer_GetTime() - start_time;
		float interval = 1.0f/10.0f;
		int frames_elapsed = (int)(elapsed_time/interval);
		
		int frame = frames_elapsed % vc->num_frames;
		bmh = vc->frames[frame];
	}else{
		bmh = bm_handle;
	}

	//draw the bitmap
	rend_SetAlphaType (AT_CONSTANT_TEXTURE);
	rend_SetAlphaValue (255);
	rend_SetLighting (LS_NONE);
	rend_SetColorModel (CM_MONO);
	rend_SetOverlayType (OT_NONE);
	rend_SetFiltering (0);

	int real_x,real_y;

	real_x = 5;
	real_y = 5;

	rend_DrawScaledBitmap (real_x,real_y,real_x+31,real_y+31,bmh,0,0,1,1);
	
	rend_SetFiltering (1);

	UIStatic::OnDraw();

	DrawBorder();
	//@@@@@@@EndFrame();
}


struct{
	int blank_bmp;
	int curr_bmp;
	int curr_index;
	ushort size;
	ushort *id_list;
	newuiListBox *listbox;
	UIStatic *bmp_disp;
}PPicDlgInfo;

void ShowPilotPicDialogListCallback(int index)
{
	if(index==PPicDlgInfo.curr_index)
		return;

	int new_idx = index;

	if(PPicDlgInfo.blank_bmp!=PPicDlgInfo.curr_bmp){
		bm_FreeBitmap(PPicDlgInfo.curr_bmp);
		PPicDlgInfo.curr_bmp = -1;
	}

	if(index==0){
		//<none> is selected
		PPicDlgInfo.curr_bmp = PPicDlgInfo.blank_bmp;
	}else{
		if(index<=PPicDlgInfo.size){			
			int handle = PPic_GetBitmapHandle(PPicDlgInfo.id_list[index-1]);
			if(handle<=BAD_BITMAP_HANDLE){
				mprintf((0,"Couldn't get ID#%d's bitmap\n",PPicDlgInfo.id_list[index-1]));
				Int3();
				PPicDlgInfo.curr_bmp = PPicDlgInfo.blank_bmp;
				new_idx = 0;
			}else{
				PPicDlgInfo.curr_bmp = handle;
			}
		}else{			
			mprintf((0,"Invalid index\n"));
			Int3();
			PPicDlgInfo.curr_bmp = PPicDlgInfo.blank_bmp;
			new_idx = 0;
		}
	}

	//change the bitmap
	UIBitmapItem bmp_item(PPicDlgInfo.curr_bmp);
	PPicDlgInfo.bmp_disp->SetTitle(&bmp_item);

	PPicDlgInfo.curr_index = new_idx;
	PPicDlgInfo.listbox->SetCurrentIndex(new_idx);
}

// -------------------------------------------------------
// ShowPilotPicDialog
//	Purpose:
//		Displays the dialog to choose a pilot picture for
//	the given pilot.
// -------------------------------------------------------
void ShowPilotPicDialog(pilot *Pilot)
{
	ASSERT(Pilot);
	if(!Pilot) return;

	char pname[PILOT_STRING_SIZE];

	Pilot->get_name(pname);
	
	ushort num_pilots = PPic_QueryPilot(pname);
	int blank_bmp_handle;

	//only display the dialog if there is a pilot to choose from
	if(num_pilots==0){
		mprintf((0,"No Pilot Pics available for %s\n",pname));
		ushort pid;
		pid = PPIC_INVALID_ID;		
		Pilot->set_multiplayer_data(NULL,NULL,NULL,&pid);

		DoMessageBox(TXT_ERROR,TXT_NOPICSAVAIL,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
		return;
	}

	blank_bmp_handle = bm_AllocBitmap(64,64,0);
	if(blank_bmp_handle<=BAD_BITMAP_HANDLE){
		ushort pid;
		pid = PPIC_INVALID_ID;		
		Pilot->set_multiplayer_data(NULL,NULL,NULL,&pid);

		mprintf((0,"Couldn't alloc bitmap\n"));
		DoMessageBox(TXT_ERROR,TXT_ERRCREATINGDIALOG,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
		return;
	}
	ushort *data = bm_data(blank_bmp_handle,0);
	for(int i=0;i<64*64;i++){
		data[i] = GR_RGB16(0,0,0)|OPAQUE_FLAG;
	}

	// create the dialog
	// -----------------
#define DLG_PPIC_W	320
#define DLG_PPIC_H	256
	newuiTiledWindow hwnd;
	newuiSheet *sheet;
	newuiListBox *list;

	UIBitmapItem bmp_item(blank_bmp_handle);
	UIStatic bmp_disp;
	bool exit_menu = false;
		
	hwnd.Create(TXT_PILOTPICTURE,0,0,DLG_PPIC_W,DLG_PPIC_H);
	sheet = hwnd.GetSheet();

	sheet->NewGroup(NULL,0,0);
	list = sheet->AddListBox(150,150,IDP_SAVE,UILB_NOSORT);
	list->SetSelectChangeCallback(ShowPilotPicDialogListCallback);
	
	sheet->NewGroup(TXT_DISPLAY,175,0);	
	bmp_disp.Create(&hwnd,&bmp_item,215,65,bmp_item.width(),bmp_item.height(),UIF_FIT);

	sheet->NewGroup(NULL,170,145);
	sheet->AddButton(TXT_OK,UID_OK);

	// Initialize PPicDlgInfo data
	// ---------------------------
	ushort *id_list;
	id_list = (ushort *)mem_malloc(num_pilots*sizeof(ushort));

	if(!id_list){
		//out of memory
		mprintf((0,"Out of memory\n"));
		goto clean_up;
	}

	PPicDlgInfo.curr_bmp = PPicDlgInfo.blank_bmp = blank_bmp_handle;
	PPicDlgInfo.curr_index = 0;
	PPicDlgInfo.listbox = list;
	PPicDlgInfo.bmp_disp = &bmp_disp;
	PPicDlgInfo.size = num_pilots;
	PPicDlgInfo.id_list = id_list;

	// fill in dialog data
	// -------------------
	ushort temp_id;
	int idx;
	idx = 0;

	Pilot->get_name(pname);

	if(PPic_FindFirst(pname,&temp_id)){
		id_list[idx] = temp_id;
		idx++;
		while(PPic_FindNext(&temp_id)){
			id_list[idx] = temp_id;
			idx++;
		}
	}
	PPic_FindClose();

	int selected_index;
	selected_index = 0;
	list->AddItem(TXT_NONE);

	char temp_buffer[PILOT_STRING_SIZE+6];
	ushort ppic_id;
	Pilot->get_multiplayer_data(NULL,NULL,NULL,&ppic_id);

	Pilot->get_name(pname);

	if(num_pilots>1)
	{
		for(idx = 0;idx<num_pilots;idx++){
			
			if(ppic_id==id_list[idx])
				selected_index = idx+1;

			Psprintf(temp_buffer,PILOT_STRING_SIZE+6,"%s%d",pname,idx);
			list->AddItem(temp_buffer);
		}
	}else
	{
		if(ppic_id==id_list[idx])
			selected_index = idx+1;

		Psprintf(temp_buffer,PILOT_STRING_SIZE+6,"%s",pname);
		list->AddItem(temp_buffer);
	}

	ShowPilotPicDialogListCallback(selected_index);

	// display dialog
	// --------------	
	hwnd.Open();
	while (!exit_menu){
		int res = hwnd.DoUI();

		// handle all UI results.
		switch (res){
		case UID_OK:
			exit_menu = true;
			break;
		};
	}

	selected_index = list->GetCurrentIndex();
	ushort pid;
	if(selected_index==0){
		pid = PPIC_INVALID_ID;		
	}else{
		pid = id_list[selected_index-1];		
	}
	Pilot->set_multiplayer_data(NULL,NULL,NULL,&pid);

	hwnd.Close();
	hwnd.Destroy();

	

clean_up:
	// free data
	// ---------
	if( (PPicDlgInfo.curr_bmp != PPicDlgInfo.blank_bmp) && (PPicDlgInfo.curr_bmp>BAD_BITMAP_HANDLE))
		bm_FreeBitmap(PPicDlgInfo.curr_bmp);	

	bm_FreeBitmap(blank_bmp_handle);

	if(id_list){
		mem_free(id_list);
	}
}


// "Current Pilot" access functions
void dCurrentPilotName(char *buffer)
{
	Current_pilot.get_name(buffer);
}
ubyte dCurrentPilotDifficulty(void)
{
	ubyte d;
	Current_pilot.get_difficulty(&d);
	return d;
}


/////////////////////////////////////////////////////////////////////
void _ReadOldPilotFile(pilot *Pilot,bool keyconfig,bool missiondata)
{
	char filename[_MAX_PATH];
	char pfilename[_MAX_FNAME];
	char buffer[256];
	ubyte temp_b;
	ushort temp_s;
	int temp_i;
	int filever,i,nctlfuncs;


	Pilot->get_filename(pfilename);	

	//open and process file
	ddio_MakePath(filename,User_directory,pfilename,NULL);
	CFILE *file = cfopen(filename,"rb");
	if(!file)
		return;

	filever = cf_ReadInt(file);

	if(filever<0x12){
		cfclose(file);
		Error(TXT_PLTTOOOLD);
		return;
	}
	
	if(filever>PLTFILEVERSION){
		//we're reading in a version that's newer than we have
		cfclose(file);
		Error(TXT_PLTTOONEW,pfilename);
		return;
	}

	Pilot->clean(true);
	Pilot->set_filename(pfilename);

	cf_ReadString(buffer,PILOT_STRING_SIZE+1,file);
	Pilot->set_name(buffer);

	cf_ReadInt(file);	
	cf_ReadInt(file);

	cf_ReadString(buffer,PAGENAME_LEN+1,file);
	Pilot->set_ship(buffer);

	cf_ReadString(buffer,PAGENAME_LEN+1,file);
	Pilot->set_multiplayer_data(buffer);

	temp_b = cf_ReadByte(file);
	Pilot->set_difficulty(temp_b);

	// load hud info
	temp_b = cf_ReadByte(file);
	Pilot->set_hud_data(&temp_b);

	temp_s = cf_ReadShort(file);
	Pilot->set_hud_data(NULL,&temp_s);
	
	temp_s = cf_ReadShort(file);
	Pilot->set_hud_data(NULL,NULL,&temp_s);

	//read in audio taunts
	cf_ReadString(buffer,PAGENAME_LEN+1,file);
	Pilot->set_multiplayer_data(NULL,buffer);

	cf_ReadString(buffer,PAGENAME_LEN+1,file);
	Pilot->set_multiplayer_data(NULL,NULL,buffer);

	// added in version 0x9
	int n;
	ushort widx;
	n = (int)cf_ReadByte(file);
	for (i = 0; i < n; i++)
	{
		widx = (ushort)cf_ReadShort(file);
		SetAutoSelectPrimaryWpnIdx(i, widx);
	}
	n = (int)cf_ReadByte(file);
	for (i = 0; i < n; i++)
	{
		widx = (ushort)cf_ReadShort(file);
		SetAutoSelectSecondaryWpnIdx(i, widx);
	}

	temp_i = cf_ReadInt(file);
	Pilot->set_hud_data(NULL,NULL,NULL,&temp_i);
	temp_i = cf_ReadInt(file);
	Pilot->set_hud_data(NULL,NULL,NULL,NULL,&temp_i);

	for (i = 0; i < N_MOUSE_AXIS; i++)
		Pilot->mouse_sensitivity[i] = cf_ReadFloat(file);
	for (i = 0; i < N_JOY_AXIS; i++)
		Pilot->joy_sensitivity[i] = cf_ReadFloat(file);


	temp_s = cf_ReadShort(file);
	Pilot->set_multiplayer_data(NULL,NULL,NULL,&temp_s);

	//skip over mission data
	int count = cf_ReadInt(file);
	if(count>0){
		for( i = 0; i < count; i++){
			char dummy[256];
			int length;
			cf_ReadByte(file);
			if (filever >= 0x11) {
				cf_ReadByte(file);
			}
			length = cf_ReadByte(file);
			if (length) {
				cf_ReadBytes((ubyte *)dummy,length,file);
			}
		}
	}

	//key/joy config MUST stay at the end of the file
	Pilot->read_controller = cf_ReadByte(file);

	// needed to properly map functions between pilot versions
	nctlfuncs = (filever >=0xe) ? cf_ReadByte(file) : NUM_CTLFUNCS_DEMOv1_0;

	for (i = 0; i < nctlfuncs; i++)
	{
		int id = cf_ReadInt(file);
		ct_type type[2];
		ct_config_data value;
		int y;

		type[0] = (ct_type)cf_ReadInt(file);
		type[1] = (ct_type)cf_ReadInt(file);
		value = (ct_config_data)cf_ReadInt(file);
		for (y=0; y < nctlfuncs; y++)
			if (Controller_needs[y].id == id) {
				if (type[0] == ctNone)			// do this if there are new functions that don't have ctNone.
					type[0] = Controller_needs[y].ctype[0];
				if (type[1] == ctNone)			// do this if there are new functions that don't have ctNone.
					type[1] = Controller_needs[y].ctype[1];
				break;
			}

		Pilot->controls[y].id = id;
		Pilot->controls[y].type[0] = type[0];
		Pilot->controls[y].type[1] = type[1];
		Pilot->controls[y].value = value;

	// new flags for each controller item.
		if (filever >= 0xb) {
			Pilot->controls[y].flags[0] = (ubyte)cf_ReadByte(file);
			Pilot->controls[y].flags[1] = (ubyte)cf_ReadByte(file);
		}

		if(keyconfig)
			Controller->set_controller_function(Pilot->controls[y].id,Pilot->controls[y].type,Pilot->controls[y].value, Pilot->controls[y].flags);
	}

// fill in remainder of pilot controls array.
	for (;i<NUM_CONTROLLER_FUNCTIONS;i++)
	{
		Controller->get_controller_function(Controller_needs[i].id,Pilot->controls[i].type, &Pilot->controls[i].value, Pilot->controls[i].flags);
	}

	if (Controller) {
		Controller->mask_controllers((Current_pilot.read_controller&READF_JOY)?true:false, 
			(Current_pilot.read_controller&READF_MOUSE)?true:false);
	}

	// Read taunts if there
	for (i=0;i<MAX_PILOT_TAUNTS;i++)
		cf_ReadString(Pilot->taunts[i],PILOT_TAUNT_SIZE,file);

	cfclose(file);
}
