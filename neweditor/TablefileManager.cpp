/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */
 


// TablefileManager.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "TablefileManager.h"
#include "ned_Tablefile.h"
#include "globals.h"
#include "Tableview.h"
#include "HogBrowser.h"	

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTablefileManager dialog


CTablefileManager::CTablefileManager(CWnd* pParent /*=NULL*/)
	: CDialog(CTablefileManager::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTablefileManager)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_Base_tablefile = NULL;
	m_Mission_tablefile = NULL;
	m_Module_tablefile = NULL;
}


void CTablefileManager::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTablefileManager)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTablefileManager, CDialog)
	//{{AFX_MSG_MAP(CTablefileManager)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BROWSE_A, OnBrowseA)
	ON_BN_CLICKED(IDC_BROWSE_B, OnBrowseB)
	ON_BN_CLICKED(IDC_BROWSE_C, OnBrowseC)
	ON_BN_CLICKED(IDC_REMOVE_A, OnRemoveA)
	ON_BN_CLICKED(IDC_REMOVE_B, OnRemoveB)
	ON_BN_CLICKED(IDC_REMOVE_C, OnRemoveC)
	ON_BN_CLICKED(IDC_DISPLAY_ALL, OnDisplayAll)
	ON_BN_CLICKED(IDC_DISPLAY_A, OnDisplayA)
	ON_BN_CLICKED(IDC_DISPLAY_B, OnDisplayB)
	ON_BN_CLICKED(IDC_DISPLAY_C, OnDisplayC)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_HELP_BUTTON, OnHelp)
	ON_BN_CLICKED(IDC_BROWSE_HOG_A, OnBrowseHogA)
	ON_BN_CLICKED(IDC_BROWSE_HOG_B, OnBrowseHogB)
	ON_BN_CLICKED(IDC_BROWSE_HOG_C, OnBrowseHogC)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTablefileManager message handlers

void CTablefileManager::OnOK() 
{
	CDialog::OnOK();
}

void CTablefileManager::OnCancel() 
{

	CDialog::OnCancel();
}

BOOL CTablefileManager::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int index;

	for(index = 0; index<MAX_LOADED_TABLE_FILES; index++ )
	{
		if(Ntbl_loaded_table_files[index].used)
		{
			switch(Ntbl_loaded_table_files[index].type)
			{
			case TABLE_FILE_BASE:
				ASSERT(!m_Base_tablefile);
				m_Base_tablefile = mem_strdup(Ntbl_loaded_table_files[index].identifier);
				break;
			case TABLE_FILE_MISSION:
				ASSERT(!m_Mission_tablefile);
				m_Mission_tablefile = mem_strdup(Ntbl_loaded_table_files[index].identifier);
				break;
			case TABLE_FILE_MODULE:
				ASSERT(!m_Module_tablefile);
				m_Module_tablefile = mem_strdup(Ntbl_loaded_table_files[index].identifier);
				break;
			default:
				Int3();
				break;
			}
		}
	}

	UpdateDialog();

	return TRUE;
}

void CTablefileManager::UpdateDialog(void)
{
	CEdit *edit;
	CButton *button;
	bool enabled;

	// Base
	edit = (CEdit *)GetDlgItem(IDC_EDIT_A);	
	edit->SetWindowText(m_Base_tablefile?m_Base_tablefile:"");
	enabled = (bool)(m_Base_tablefile!=NULL);

	edit->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_REMOVE_A);
	button->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_DISPLAY_A);
	button->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_BROWSE_A);
	button->EnableWindow(true);
	button = (CButton *)GetDlgItem(IDC_BROWSE_HOG_A);
	button->EnableWindow(true);

	// Mission
	edit = (CEdit *)GetDlgItem(IDC_EDIT_B);	
	edit->SetWindowText(m_Mission_tablefile?m_Mission_tablefile:"");
	enabled = (bool)(m_Mission_tablefile!=NULL);

	edit->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_REMOVE_B);
	button->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_DISPLAY_B);
	button->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_BROWSE_B);
	button->EnableWindow(true);
	button = (CButton *)GetDlgItem(IDC_BROWSE_HOG_B);
	button->EnableWindow(true);

	// Module
	edit = (CEdit *)GetDlgItem(IDC_EDIT_C);	
	edit->SetWindowText(m_Module_tablefile?m_Module_tablefile:"");
	enabled = (bool)(m_Module_tablefile!=NULL);

	edit->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_REMOVE_C);
	button->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_DISPLAY_C);
	button->EnableWindow(enabled);
	button = (CButton *)GetDlgItem(IDC_BROWSE_C);
	button->EnableWindow(true);
	button = (CButton *)GetDlgItem(IDC_BROWSE_HOG_C);
	button->EnableWindow(true);

	button = (CButton *)GetDlgItem(IDOK);
	button->EnableWindow(true);

	enabled = (bool)(m_Base_tablefile!=NULL || m_Mission_tablefile!=NULL || m_Module_tablefile!=NULL);
	button = (CButton *)GetDlgItem(IDC_DISPLAY_ALL);
	button->EnableWindow(enabled);
}

void CTablefileManager::DisableAll(void)
{
	CEdit *edit;
	CButton *button;

	// Base
	edit = (CEdit *)GetDlgItem(IDC_EDIT_A);	
	edit->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_REMOVE_A);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_DISPLAY_A);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_BROWSE_A);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_BROWSE_HOG_A);
	button->EnableWindow(false);

	// Mission
	edit = (CEdit *)GetDlgItem(IDC_EDIT_B);	
	edit->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_REMOVE_B);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_DISPLAY_B);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_BROWSE_B);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_BROWSE_HOG_B);
	button->EnableWindow(false);

	// Module
	edit = (CEdit *)GetDlgItem(IDC_EDIT_C);	
	edit->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_REMOVE_C);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_DISPLAY_C);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_BROWSE_C);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDC_BROWSE_HOG_C);
	button->EnableWindow(false);

	button = (CButton *)GetDlgItem(IDC_DISPLAY_ALL);
	button->EnableWindow(false);
	button = (CButton *)GetDlgItem(IDOK);
	button->EnableWindow(false);
}

void CTablefileManager::ChangeTablefile(char *filename,int type)
{
	int ret;

	switch(type)
	{
	case TABLE_FILE_BASE:
		{
			//check to see if we are removing a tablefile, or replacing/adding

			if(filename)//replacing/adding
			{
				//check to make sure we aren't just adding the same tablefile
				if(m_Base_tablefile && !stricmp(m_Base_tablefile,filename))
				{
					//we're adding the same table file...ignore request
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}

				//check to make sure we aren't adding the table file that clashes
				if(m_Module_tablefile && !stricmp(m_Module_tablefile,filename))
				{
					//clash
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}
				if(m_Mission_tablefile && !stricmp(m_Mission_tablefile,filename))
				{
					//clash
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}

				//check for the table files we have to remove, before we add them...ick, this
				//one means all loaded ones have to be removed, since it is the lowest base
				if(m_Module_tablefile)
				{
					//remove the module tablefile
					ntbl_DeleteTableFilePages(m_Module_tablefile);
				}
				if(m_Mission_tablefile)
				{
					//remove the mission tablefile
					ntbl_DeleteTableFilePages(m_Mission_tablefile);
				}
				if(m_Base_tablefile)
				{
					//remove the base tablefile
					ntbl_DeleteTableFilePages(m_Base_tablefile);

					//remove the memory allocated for it
					mem_free(m_Base_tablefile);
					m_Base_tablefile = NULL;
				}

				//now add the table files back in the correct order
				m_Base_tablefile = mem_strdup(filename);
				if(!m_Base_tablefile)
				{
					//out of memory
					mprintf((0,"OUT OF MEMORY\n"));
					Int3();
				}else
				{
					//add it in
					ret = ntbl_LoadTableFile(m_Base_tablefile);
					if(ret>=0)
					{
						Ntbl_loaded_table_files[ret].type = TABLE_FILE_BASE;
					}
				}

				if(m_Mission_tablefile)
				{
					//add it in
					ret = ntbl_LoadTableFile(m_Mission_tablefile);
					if(ret>=0)
					{
						Ntbl_loaded_table_files[ret].type = TABLE_FILE_MISSION;
					}
				}


				if(m_Module_tablefile)
				{
					//add it in
					ret = ntbl_LoadTableFile(m_Module_tablefile);
					if(ret>=0)
					{
						Ntbl_loaded_table_files[ret].type = TABLE_FILE_MODULE;
					}
				}

			}else
			{
				//we are just removing the base table file, so we just have to delete it without
				//deleting any of the others
				if(m_Base_tablefile)
				{
					//remove the base tablefile
					ntbl_DeleteTableFilePages(m_Base_tablefile);

					//remove the memory allocated for it
					mem_free(m_Base_tablefile);
					m_Base_tablefile = NULL;
				}
			}

		}break;
	case TABLE_FILE_MISSION:
		{
			//check to see if we are removing a tablefile, or replacing/adding

			if(filename)//replacing/adding
			{
				//check to make sure we aren't just adding the same tablefile
				if(m_Mission_tablefile && !stricmp(m_Mission_tablefile,filename))
				{
					//we're adding the same table file...ignore request
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}

				//check to make sure we aren't adding the table file that clashes
				if(m_Module_tablefile && !stricmp(m_Module_tablefile,filename))
				{
					//clash
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}
				if(m_Base_tablefile && !stricmp(m_Base_tablefile,filename))
				{
					//clash
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}

				//check for the table files we have to remove, before we add them...
				if(m_Module_tablefile)
				{
					//remove the module tablefile
					ntbl_DeleteTableFilePages(m_Module_tablefile);
				}
				if(m_Mission_tablefile)
				{
					//remove the mission tablefile
					ntbl_DeleteTableFilePages(m_Mission_tablefile);

					//remove the memory allocated for it
					mem_free(m_Mission_tablefile);
					m_Mission_tablefile = NULL;
				}

				//now add the table files back in the correct order
				m_Mission_tablefile = mem_strdup(filename);
				if(!m_Mission_tablefile)
				{
					//out of memory
					mprintf((0,"OUT OF MEMORY\n"));
					Int3();
				}else
				{
					//add it in
					ret = ntbl_LoadTableFile(m_Mission_tablefile);
					if(ret>=0)
					{
						Ntbl_loaded_table_files[ret].type = TABLE_FILE_MISSION;
					}
				}

				if(m_Module_tablefile)
				{
					//add it in
					ret = ntbl_LoadTableFile(m_Module_tablefile);
					if(ret>=0)
					{
						Ntbl_loaded_table_files[ret].type = TABLE_FILE_MODULE;
					}
				}

			}else
			{
				//we are just removing the base table file, so we just have to delete it without
				//deleting any of the others
				if(m_Mission_tablefile)
				{
					//remove the base tablefile
					ntbl_DeleteTableFilePages(m_Mission_tablefile);

					//remove the memory allocated for it
					mem_free(m_Mission_tablefile);
					m_Mission_tablefile = NULL;
				}
			}

		}break;
	case TABLE_FILE_MODULE:
		{
			//check to see if we are removing a tablefile, or replacing/adding

			if(filename)//replacing/adding
			{
				//check to make sure we aren't just adding the same tablefile
				if(m_Module_tablefile && !stricmp(m_Module_tablefile,filename))
				{
					//we're adding the same table file...ignore request
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}

				//check to make sure we aren't adding the table file that clashes
				if(m_Mission_tablefile && !stricmp(m_Mission_tablefile,filename))
				{
					//clash
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}
				if(m_Base_tablefile && !stricmp(m_Base_tablefile,filename))
				{
					//clash
					char buffer[1024];
					sprintf(buffer,"%s already exists in the Tablefile hierarchy",filename);
					MessageBox(buffer,"Error");					
					return;
				}

				//check for the table files we have to remove, before we add them...
				if(m_Module_tablefile)
				{
					//remove the module tablefile
					ntbl_DeleteTableFilePages(m_Module_tablefile);

					//remove the memory allocated for it
					mem_free(m_Module_tablefile);
					m_Module_tablefile = NULL;
				}

				//now add the table files back in the correct order
				m_Module_tablefile = mem_strdup(filename);
				if(!m_Module_tablefile)
				{
					//out of memory
					mprintf((0,"OUT OF MEMORY\n"));
					Int3();
				}else
				{
					//add it in
					ret = ntbl_LoadTableFile(m_Module_tablefile);
					if(ret>=0)
					{
						Ntbl_loaded_table_files[ret].type = TABLE_FILE_MODULE;
					}
				}

			}else
			{
				//we are just removing the base table file, so we just have to delete it without
				//deleting any of the others
				if(m_Module_tablefile)
				{
					//remove the base tablefile
					ntbl_DeleteTableFilePages(m_Module_tablefile);

					//remove the memory allocated for it
					mem_free(m_Module_tablefile);
					m_Module_tablefile = NULL;
				}
			}
		}break;
	}

}

void CTablefileManager::OnClose() 
{
	if(m_Base_tablefile)
	{
		mem_free(m_Base_tablefile);
		m_Base_tablefile = NULL;
	}

	if(m_Mission_tablefile)
	{
		mem_free(m_Mission_tablefile);
		m_Mission_tablefile = NULL;
	}

	if(m_Module_tablefile)
	{
		mem_free(m_Module_tablefile);
		m_Module_tablefile = NULL;
	}
	
	CDialog::OnClose();
}

void CTablefileManager::OnBrowseA() 
{
	char filter[] = "Descent 3 Tablefiles (*.gam)|*.gam|All Files (*.*)|*.*||";
	CFileDialog dlg(true,".ext",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,filter);
	if(dlg.DoModal()==IDOK)
	{
		DisableAll();
		ChangeTablefile(dlg.GetPathName().GetBuffer(0),TABLE_FILE_BASE);
		UpdateDialog();	
	}	
}

void CTablefileManager::OnBrowseB() 
{
	char filter[] = "Descent 3 Tablefiles (*.gam)|*.gam|All Files (*.*)|*.*||";
	CFileDialog dlg(true,".ext",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,filter);
	if(dlg.DoModal()==IDOK)
	{
		DisableAll();
		ChangeTablefile(dlg.GetPathName().GetBuffer(0),TABLE_FILE_MISSION);
		UpdateDialog();	
	}	
}

void CTablefileManager::OnBrowseC() 
{
	char filter[] = "Descent 3 Tablefiles (*.gam)|*.gam|All Files (*.*)|*.*||";
	CFileDialog dlg(true,".ext",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,filter);
	if(dlg.DoModal()==IDOK)
	{
		DisableAll();
		ChangeTablefile(dlg.GetPathName().GetBuffer(0),TABLE_FILE_MODULE);
		UpdateDialog();	
	}	
}

void CTablefileManager::OnRemoveA() 
{
	DisableAll();
	ChangeTablefile(NULL,TABLE_FILE_BASE);
	UpdateDialog();	
}

void CTablefileManager::OnRemoveB() 
{
	DisableAll();
	ChangeTablefile(NULL,TABLE_FILE_MISSION);
	UpdateDialog();	
}

void CTablefileManager::OnRemoveC() 
{
	DisableAll();
	ChangeTablefile(NULL,TABLE_FILE_MODULE);
	UpdateDialog();	
}

void CTablefileManager::OnDisplayAll() 
{
	CTableView dlgTableView("Tablefile Pages In Use",this);
	
	CTableTextureView TexturePage;
	CTableObjectView ObjectPage;
	CTableSoundsView SoundsPage;
	CTableDoorView DoorPage;

	TexturePage.SetFilter(-1);
	ObjectPage.SetFilter(-1);
	SoundsPage.SetFilter(-1);
	DoorPage.SetFilter(-1);

	dlgTableView.AddPage(&ObjectPage);
	dlgTableView.AddPage(&TexturePage);
	dlgTableView.AddPage(&SoundsPage);
	dlgTableView.AddPage(&DoorPage);

	dlgTableView.DoModal();	
}


void CTablefileManager::OnDisplayA() 
{
	CTableView dlgTableView("Base Tablefile Pages In Use",this);
	
	CTableTextureView TexturePage;
	CTableObjectView ObjectPage;
	CTableSoundsView SoundsPage;
	CTableDoorView DoorPage;

	TexturePage.SetFilter(TABLE_FILE_BASE);
	ObjectPage.SetFilter(TABLE_FILE_BASE);
	SoundsPage.SetFilter(TABLE_FILE_BASE);
	DoorPage.SetFilter(TABLE_FILE_BASE);

	dlgTableView.AddPage(&ObjectPage);
	dlgTableView.AddPage(&TexturePage);
	dlgTableView.AddPage(&SoundsPage);
	dlgTableView.AddPage(&DoorPage);

	dlgTableView.DoModal();		
}

void CTablefileManager::OnDisplayB() 
{
	CTableView dlgTableView("Mission Tablefile Pages In Use",this);
	
	CTableTextureView TexturePage;
	CTableObjectView ObjectPage;
	CTableSoundsView SoundsPage;
	CTableDoorView DoorPage;

	TexturePage.SetFilter(TABLE_FILE_MISSION);
	ObjectPage.SetFilter(TABLE_FILE_MISSION);
	SoundsPage.SetFilter(TABLE_FILE_MISSION);
	DoorPage.SetFilter(TABLE_FILE_MISSION);

	dlgTableView.AddPage(&ObjectPage);
	dlgTableView.AddPage(&TexturePage);
	dlgTableView.AddPage(&SoundsPage);
	dlgTableView.AddPage(&DoorPage);

	dlgTableView.DoModal();		
}

void CTablefileManager::OnDisplayC() 
{
	CTableView dlgTableView("Module Tablefile Pages In Use",this);
	
	CTableTextureView TexturePage;
	CTableObjectView ObjectPage;
	CTableSoundsView SoundsPage;
	CTableDoorView DoorPage;

	TexturePage.SetFilter(TABLE_FILE_MODULE);
	ObjectPage.SetFilter(TABLE_FILE_MODULE);
	SoundsPage.SetFilter(TABLE_FILE_MODULE);
	DoorPage.SetFilter(TABLE_FILE_MODULE);

	dlgTableView.AddPage(&ObjectPage);
	dlgTableView.AddPage(&TexturePage);
	dlgTableView.AddPage(&SoundsPage);
	dlgTableView.AddPage(&DoorPage);

	dlgTableView.DoModal();		
}

void CTablefileManager::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if(m_Base_tablefile)
	{
		mem_free(m_Base_tablefile);
		m_Base_tablefile = NULL;
	}

	if(m_Mission_tablefile)
	{
		mem_free(m_Mission_tablefile);
		m_Mission_tablefile = NULL;
	}

	if(m_Module_tablefile)
	{
		mem_free(m_Module_tablefile);
		m_Module_tablefile = NULL;
	}	
}

void CTablefileManager::OnHelp() 
{
	// TODO: Add your control notification handler code here
	// ::CreateProcess(NULL, "c:/windows/winhlp32.exe -i cntx_tablefile c:/descent3/main/neweditor/debug/d3edit.hlp", NULL, NULL, FALSE, NULL, NULL, NULL, NULL, NULL);
	HINSTANCE hresult = ::ShellExecute(AfxGetMainWnd()->m_hWnd, NULL, "d3edit.hlp", "-i cntx_tablefile", "c:/descent3/main/neweditor/debug", 0); // "winhlp32 -i cntx_tablefile d3edit.hlp"
}
void CTablefileManager::OnBrowseHogA() 
{
	CHogBrowser dlg("Descent 3 Tablefiles (*.gam)|*.gam||");
	if(dlg.DoModal()==IDOK)
	{
		char filename[_MAX_PATH];
		if(dlg.GetFilename(filename,_MAX_PATH))
		{
			DisableAll();
			ChangeTablefile(filename,TABLE_FILE_BASE);
			UpdateDialog();	
		}
	}
}

void CTablefileManager::OnBrowseHogB() 
{
	CHogBrowser dlg("Descent 3 Tablefiles (*.gam)|*.gam||");
	if(dlg.DoModal()==IDOK)
	{
		char filename[_MAX_PATH];
		if(dlg.GetFilename(filename,_MAX_PATH))
		{
			DisableAll();
			ChangeTablefile(filename,TABLE_FILE_MISSION);
			UpdateDialog();	
		}
	}
}

void CTablefileManager::OnBrowseHogC() 
{
	CHogBrowser dlg("Descent 3 Tablefiles (*.gam)|*.gam||");
	if(dlg.DoModal()==IDOK)
	{
		char filename[_MAX_PATH];
		if(dlg.GetFilename(filename,_MAX_PATH))
		{
			DisableAll();
			ChangeTablefile(filename,TABLE_FILE_MODULE);
			UpdateDialog();	
		}
	}
}
