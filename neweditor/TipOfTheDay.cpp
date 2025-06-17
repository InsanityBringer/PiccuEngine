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
 // TipOfTheDay.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "TipOfTheDay.h"
#include "args.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTipOfTheDay dialog


CTipOfTheDay::CTipOfTheDay(CWnd* pParent )
	: CDialog(CTipOfTheDay::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTipOfTheDay)
	m_strTip = _T("");
	m_ShowNextTime = FALSE;
	//}}AFX_DATA_INIT
	m_lpszTips = NULL;
	m_NumTips = 0;
}


CTipOfTheDay::~CTipOfTheDay()
{
//	int i;
	
	if(!m_lpszTips)
		return;

	//Don't do this stuff, because the mem lib will clean up for us before the destructor is called.
	/*
	for(i=0;i<m_NumTips;i++)
	{
		//Clean up
		mem_free(m_lpszTips[i]);		
	}	
	mem_free(m_lpszTips);
	*/
}


void CTipOfTheDay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTipOfTheDay)
	DDX_Text(pDX, IDC_TIP_TEXT, m_strTip);
	DDX_Check(pDX, IDC_SHOWAGAIN, m_ShowNextTime);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTipOfTheDay, CDialog)
	//{{AFX_MSG_MAP(CTipOfTheDay)
	ON_BN_CLICKED(IDC_OUTRAGEWEBPAGE, OnOutragewebpage)
	ON_BN_CLICKED(IDC_NEXTTIP, OnNexttip)
	ON_BN_CLICKED(IDC_PREVTIP, OnPrevtip)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTipOfTheDay message handlers

void CTipOfTheDay::OnOutragewebpage() 
{
	ShellExecute(AfxGetMainWnd()->m_hWnd,"open","http://www.outrage.com/",NULL,NULL,SW_SHOW);		
}

void CTipOfTheDay::OnOK() 
{
	UpdateData(true);

	AfxGetApp()->WriteProfileInt("Defaults","ShowTips",m_ShowNextTime?1:0);
	
	AfxGetApp()->WriteProfileInt("Defaults","NextTipNum",m_CurTip+1);
	
	CDialog::OnOK();
}

BOOL CTipOfTheDay::OnInitDialog() 
{
	
	CString cmdline;
	
	CString path;
	
	char * p = GameArgs[1];//GetCommandLine();

	cmdline = p;

	path = cmdline.Left(cmdline.ReverseFind('\\')+1);

	path += "totd.txt";
	
	ReadTips((char *)LPCSTR(path));


	if(m_NumTips)
	{
		m_CurTip = AfxGetApp()->GetProfileInt("Defaults","NextTipNum",1);

		if(m_CurTip >= m_NumTips)
		{
			m_CurTip = 0;
		}

		CDialog::OnInitDialog();

		m_ShowNextTime = AfxGetApp()->GetProfileInt("Defaults","ShowTips",1)?true:false;

		m_strTip = m_lpszTips[m_CurTip];//"Make sure to check the Outrage web site for\r\nnewer versions of this editor!";

	}
	else
	{
		m_strTip = "Make sure to check the Outrage web site for\r\nnewer versions of this editor!";
	}
	UpdateData(false);

	CWnd *parent_wnd = AfxGetMainWnd();
	
	RECT prect,rect;
	int width,height;
	int pwidth,pheight;
	//Add code here to center the new window
	if(parent_wnd)
	{
		parent_wnd->GetClientRect(&prect);
		pwidth = prect.right - prect.left;
		pheight = prect.bottom - prect.top;
	}

	GetWindowRect(&rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	rect.top = (pheight/2)-(height/2);
	rect.left = (pwidth/2)-(width/2);
	rect.bottom = rect.top + height;
	rect.right = rect.left + width;

	MoveWindow(&rect);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTipOfTheDay::OnNexttip() 
{
	if(!m_lpszTips)
		return;
	m_CurTip++;
	if(m_CurTip >= m_NumTips)
	{
		m_CurTip = 0;
	}
	m_strTip = m_lpszTips[m_CurTip];
	UpdateData(false);
}

void CTipOfTheDay::OnPrevtip() 
{
	if(!m_lpszTips)
		return;
	m_CurTip--;
	if(m_CurTip < 0)
	{
		m_CurTip = m_NumTips-1;
	}
	m_strTip = m_lpszTips[m_CurTip];
	UpdateData(false);
	
}

#define MAX_TIP_LINE_LEN	500
int CTipOfTheDay::CountTips(char *file)
{
	CFILE *cfp;
	char szline[MAX_TIP_LINE_LEN];

	cfp = cfopen(file,"rt");

	if(cfp)
	{
		do
		{
			try
			{
				cf_ReadString(szline,MAX_TIP_LINE_LEN-1,cfp);
			}
			catch(...)
			{
				break;
			}

			if( (szline[0] == '&') && (szline[1] == '&') )
			{
				m_NumTips++;
			}
		}while(!cfeof(cfp));
	}
	else
		return 0;

	cfclose(cfp);
	return m_NumTips;
}

#define TIP_NEWLINE	"\r\n"
int CTipOfTheDay::ReadTips(char *file)
{
	CFILE *cfp;
	char szline[MAX_TIP_LINE_LEN];
	
	CountTips(file);

	if(!m_NumTips)
		return 0;

	m_lpszTips = (char **) mem_malloc(sizeof(char *)*m_NumTips);

	int i;
	
	for(i=0;i<m_NumTips;i++)
	{
		//Allocate just enough for the NULL for now
		m_lpszTips[i] = (char *) mem_malloc(sizeof(char)*1);
		strcpy(m_lpszTips[i],"");
	}

	cfp = cfopen(file,"rt");

	i = 0;
	if(cfp)
	{
		do
		{
			try
			{
				cf_ReadString(szline,MAX_TIP_LINE_LEN-1,cfp);
			}
			catch(...)
			{
				break;
			}

			if( (szline[0] == '&') && (szline[1] == '&') )
			{
				i++;
			}
			else
			{
				//Allocate enough for the new string plus this one plus a CR
				m_lpszTips[i] = (char *) mem_realloc(m_lpszTips[i],strlen(m_lpszTips[i])+strlen(szline)+strlen(TIP_NEWLINE)+1 );
				if(*m_lpszTips[i])
					strcat(m_lpszTips[i],TIP_NEWLINE);
				strcat(m_lpszTips[i],szline);
			}
		}while( (i<m_NumTips) && (!cfeof(cfp)) );
	}
	else
		return 0;

	cfclose(cfp);
	return i;
}
