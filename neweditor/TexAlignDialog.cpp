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
 // TexAlignDialog.cpp : implementation file
//

#include "stdafx.h"

#include "../editor/HTexture.h"
#include "../editor/RoomUVs.h"

#include "neweditor.h"
#include "TexAlignDialog.h"
#include "ned_Util.h"
#include "RoomFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern short facenums[MAX_FACES_PER_ROOM];
int num_m_faces = 0;

/////////////////////////////////////////////////////////////////////////////
// CTexAlignDialog dialog


CTexAlignDialog::CTexAlignDialog(CWnd* pParent )
	: CDialog(CTexAlignDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTexAlignDialog)
	m_U = 0.0f;
	m_V = 0.0f;
	//}}AFX_DATA_INIT
	m_pPrim = theApp.AcquirePrim();
//	m_bTimerSet = false;
	m_Faces = CURRENT;
}


CTexAlignDialog::CTexAlignDialog(prim *pprim, CWnd* pParent /*=NULL*/)
	: CDialog(CTexAlignDialog::IDD, pParent)
{
	m_U = 0.0f;
	m_V = 0.0f;
	m_pPrim = pprim;
//	m_bTimerSet = false;
	m_Faces = CURRENT;
}


void CTexAlignDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTexAlignDialog)
	DDX_Text(pDX, IDC_EDIT_U, m_U);
	DDX_Text(pDX, IDC_EDIT_V, m_V);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTexAlignDialog, CDialog)
	//{{AFX_MSG_MAP(CTexAlignDialog)
	ON_BN_CLICKED(IDC_UV_APPLY, OnUVApply)
	ON_BN_CLICKED(IDC_UV_DEFAULT, OnUVDefault)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_UV_FLIP_X, OnUVFlipX)
	ON_BN_CLICKED(IDC_UV_FLIP_Y, OnUVFlipY)
	ON_BN_CLICKED(IDC_UV_ROTATE_90, OnUVRotate90)
	ON_EN_KILLFOCUS(IDC_UV_EDIT_STEP, OnKillfocusUVEditStep)
	ON_BN_CLICKED(IDC_UV_ALIGN_MARKED, OnUVAlignMarked)
	ON_BN_CLICKED(IDC_UV_FACE_MAP, OnUVFaceMap)
	ON_BN_CLICKED(IDC_UV_ALIGN_EDGE, OnUVAlignEdge)
	ON_BN_CLICKED(IDC_UV_DOWN, OnUVDown)
	ON_BN_CLICKED(IDC_UV_EXPAND_U, OnUVExpandU)
	ON_BN_CLICKED(IDC_UV_EXPAND_V, OnUVExpandV)
	ON_BN_CLICKED(IDC_UV_LEFT, OnUVLeft)
	ON_BN_CLICKED(IDC_UV_RIGHT, OnUVRight)
	ON_BN_CLICKED(IDC_UV_ROTLEFT, OnUVRotLeft)
	ON_BN_CLICKED(IDC_UV_ROTRIGHT, OnUVRotRight)
	ON_BN_CLICKED(IDC_UV_SHRINK_U, OnUVShrinkU)
	ON_BN_CLICKED(IDC_UV_SHRINK_V, OnUVShrinkV)
	ON_BN_CLICKED(IDC_UV_STRETCHLESS, OnUVStretchLess)
	ON_BN_CLICKED(IDC_UV_STRETCHMORE, OnUVStretchMore)
	ON_BN_CLICKED(IDC_UV_UP, OnUVUp)
	ON_BN_CLICKED(IDC_CURRENTFACE, OnCurrentFace)
	ON_BN_CLICKED(IDC_MARKEDFACES, OnMarkedFaces)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTexAlignDialog message handlers

void CTexAlignDialog::OnUVApply() 
{
	// TODO: Add your control notification handler code here
	CString ustr,vstr;

	GetDlgItemText(IDC_EDIT_U,ustr);
	GetDlgItemText(IDC_EDIT_V,vstr);
	m_U = (float) atof(LPCSTR(ustr));
	m_V = (float) atof(LPCSTR(vstr));
/*
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			UVApply(m_pPrim->roomp,facenums[i],m_pPrim->vert,m_U,m_V);
	else if (m_pPrim->face != -1 && m_pPrim->vert != -1)
		UVApply(m_pPrim->roomp,m_pPrim->face,m_pPrim->vert,m_U,m_V);
	else
		return;
*/
	if (m_pPrim->face != -1 && m_pPrim->vert != -1)
	{
		face *fp = &m_pPrim->roomp->faces[m_pPrim->face];
		fp->face_uvls[m_pPrim->vert].u = m_U;
		fp->face_uvls[m_pPrim->vert].v = m_V;
		RoomChanged(m_pPrim->roomp);
	}
}

void CTexAlignDialog::OnUVDown() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureSlide(m_pPrim->roomp, facenums[i], 0.0f, (float) -1.0*Editor_state.texscale);
	else if (m_pPrim->face != -1)
		HTextureSlide(m_pPrim->roomp, m_pPrim->face, 0.0f, (float) -1.0*Editor_state.texscale);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVLeft() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureSlide(m_pPrim->roomp, facenums[i], (float) -1.0*Editor_state.texscale, 0.0f);
	else if (m_pPrim->face != -1)
		HTextureSlide(m_pPrim->roomp, m_pPrim->face, (float) -1.0*Editor_state.texscale, 0.0f);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVRight() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureSlide(m_pPrim->roomp, facenums[i], (float) 1.0*Editor_state.texscale, 0.0f);
	else if (m_pPrim->face != -1)
		HTextureSlide(m_pPrim->roomp, m_pPrim->face, (float) 1.0*Editor_state.texscale, 0.0f);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVRotLeft() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureRotate(m_pPrim->roomp, facenums[i], (-512*Editor_state.texscale));
	else if (m_pPrim->face != -1)
		HTextureRotate(m_pPrim->roomp, m_pPrim->face, (-512*Editor_state.texscale));
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVRotRight() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureRotate(m_pPrim->roomp, facenums[i], (512*Editor_state.texscale));
	else if (m_pPrim->face != -1)
		HTextureRotate(m_pPrim->roomp, m_pPrim->face, (512*Editor_state.texscale));	
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVStretchLess() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureStretchLess(m_pPrim->roomp, facenums[i], m_pPrim->edge);
	else if (m_pPrim->face != -1 && m_pPrim->edge != -1)
		HTextureStretchLess(m_pPrim->roomp, m_pPrim->face, m_pPrim->edge);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVStretchMore() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureStretchMore(m_pPrim->roomp, facenums[i], m_pPrim->edge);
	else if (m_pPrim->face != -1 && m_pPrim->edge != -1)
		HTextureStretchMore(m_pPrim->roomp, m_pPrim->face, m_pPrim->edge);	
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVUp() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureSlide(m_pPrim->roomp, facenums[i], 0.0f, (float) 1.0*Editor_state.texscale);
	else if (m_pPrim->face != -1)
		HTextureSlide(m_pPrim->roomp, m_pPrim->face, 0.0f, (float) 1.0*Editor_state.texscale);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}


void CTexAlignDialog::OnUVDefault() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureSetDefault(m_pPrim->roomp, facenums[i]);
	else if (m_pPrim->face != -1)
		HTextureSetDefault(m_pPrim->roomp,m_pPrim->face);	
	else
		return;

	RoomChanged(m_pPrim->roomp);
}

void CTexAlignDialog::EnableControls(bool enable)
{
	((CEdit *)GetDlgItem(IDC_CURRENTFACE))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_MARKEDFACES))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_STRETCHLESS))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_STRETCHMORE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_UP))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_DEFAULT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_APPLY))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_DOWN))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_LEFT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_RIGHT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_ROTLEFT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_ROTRIGHT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_EDIT_U))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_EDIT_V))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_FLIP_X))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_FLIP_Y))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_ROTATE_90))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_EDIT_STEP))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_EXPAND_U))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_EXPAND_V))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_SHRINK_U))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_SHRINK_V))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_ALIGN_MARKED))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_FACE_MAP))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_UV_ALIGN_EDGE))->EnableWindow(enable);
}

void CTexAlignDialog::UpdateDialog()
{
	m_pPrim = theApp.AcquirePrim();
	void *wnd = theApp.AcquireWnd();
	(wnd == theApp.m_pFocusedRoomFrm) ? (m_pRoomFrm = (CRoomFrm *) wnd) : (m_pRoomFrm = NULL);

	if (m_pPrim == NULL || m_pPrim->roomp == NULL) // || m_pPrim->face == -1 || m_pPrim->edge == -1 || m_pPrim->vert == -1)
	{
//		EnableTabControls(this->m_hWnd,false,IDC_UV_STRETCHLESS,-1);
		EnableControls(false);
		return;
	}
	else
	{
//		EnableTabControls(this->m_hWnd,true,IDC_UV_STRETCHLESS,-1);
		EnableControls(true);
	}

	CString ustr,vstr,scalestr;

	face *fp;
	roomUVL *uv;

	if (m_pPrim->face != -1)
	{
		fp = &m_pPrim->roomp->faces[m_pPrim->face];

		int vert = m_pPrim->vert;
		if (vert != -1)
			uv = &fp->face_uvls[vert];

		m_U = uv->u;
		m_V = uv->v;
	}
	else
	{
		m_U = 0.00f;
		m_V = 0.00f;
	}

	ustr.Format("%.2f", m_U);
	vstr.Format("%.2f", m_V);

	SetDlgItemText(IDC_EDIT_U,ustr);
	SetDlgItemText(IDC_EDIT_V,vstr);

	scalestr.Format("%.2f",Editor_state.texscale);
	SetDlgItemText(IDC_UV_EDIT_STEP,scalestr);

	// If the focused window is not a room frame, disable the marked faces radio button 
	// and check the current faces radio button
	if (m_pRoomFrm == NULL)
	{
		CheckRadioButton(IDC_CURRENTFACE,IDC_MARKEDFACES,IDC_CURRENTFACE);
		OnCurrentFace();
		((CButton *)GetDlgItem(IDC_MARKEDFACES))->EnableWindow(false);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_MARKEDFACES))->EnableWindow(true);
		// Get list of marked faces
		num_m_faces = m_pRoomFrm->GetMarkedFaces(facenums);
		ASSERT(num_m_faces == m_pRoomFrm->m_Num_marked_faces);
	}
}

void UVApply(room *rp, int facenum, int vertnum, float u, float v)
{
	ASSERT(facenum != -1);
	ASSERT(vertnum != -1);
	if (facenum == -1 || vertnum == -1)
		return;

	face *fp = &rp->faces[facenum];
	int vertnum2;

	roomUVL *uva = &fp->face_uvls[vertnum];
	(vertnum == fp->num_verts-1) ? (vertnum2 = 0) : (vertnum2 = vertnum+1);
	roomUVL *uvb = &fp->face_uvls[vertnum2];

	float oldu = uva->u;
	float oldv = uva->v;
	uva->u = u;
	uva->v = v;
	uvb->u += u - oldu;
	uvb->v += v - oldv;

	AssignUVsToFace(rp, facenum, uva, uvb, vertnum, vertnum2);
}


void CTexAlignDialog::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	theApp.m_pTexAlignDlg = NULL;
	delete this;
}

void CTexAlignDialog::OnCancel() 
{
	// TODO: Add extra cleanup here
	DestroyWindow();
}

void CTexAlignDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	UpdateDialog();	
	// Do not call CDialog::OnPaint() for painting messages
}

void CTexAlignDialog::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}

void CTexAlignDialog::FlipTextureX(int facenum) 
{
	ASSERT(facenum != -1);
	if (facenum == -1)
		return;

	room *rp = m_pPrim->roomp;

	ASSERT(rp->used);

	for (int i=0;i<rp->faces[facenum].num_verts;i++)
		rp->faces[facenum].face_uvls[i].u=1-rp->faces[facenum].face_uvls[i].u;
}

void CTexAlignDialog::FlipTextureY(int facenum) 
{
	ASSERT(facenum != -1);
	if (facenum == -1)
		return;

	room *rp = m_pPrim->roomp;

	ASSERT(rp->used);

	for (int i=0;i<rp->faces[facenum].num_verts;i++)
		rp->faces[facenum].face_uvls[i].v=1-rp->faces[facenum].face_uvls[i].v;
}

void CTexAlignDialog::OnUVFlipX() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			FlipTextureX(facenums[i]);
	else if (m_pPrim->face != -1)
		FlipTextureX(m_pPrim->face);
	else
		return;

	RoomChanged(m_pPrim->roomp);
}

void CTexAlignDialog::OnUVFlipY() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			FlipTextureY(facenums[i]);
	else if (m_pPrim->face != -1)
		FlipTextureY(m_pPrim->face);
	else
		return;

	RoomChanged(m_pPrim->roomp);
}

void CTexAlignDialog::OnUVRotate90() 
{
	// TODO: Add your control notification handler code here
	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			HTextureRotate(m_pPrim->roomp, facenums[i], 0x4000);
	else if (m_pPrim->face != -1)
		HTextureRotate(m_pPrim->roomp, m_pPrim->face, 0x4000);
	else
		return;

	RoomChanged(m_pPrim->roomp);
}

void CTexAlignDialog::OnKillfocusUVEditStep() 
{
	// TODO: Add your control notification handler code here
	char str[20];

	GetDlgItemText(IDC_UV_EDIT_STEP,str,sizeof(str));
	float scale = atof(str);
	if (scale<0)
		scale=0;
	if (scale>64)
		scale=64;
	if (scale != 0) // don't allow 0
		Editor_state.texscale = scale;
	else
		SetDlgItemText(IDC_UV_EDIT_STEP,"1.00");
}

void CTexAlignDialog::ExpandTextureU(int facenum,bool expand) 
{
	ASSERT(facenum != -1);
	if (facenum == -1)
		return;

	face *fp=&m_pPrim->roomp->faces[facenum];

	int i;
	float mid=0;
	float scale;
	(expand) ? (scale = 1.0/D3EditState.texscale) : (scale = D3EditState.texscale);

	for (i=0;i<fp->num_verts;i++)
		mid+=fp->face_uvls[i].u;

	mid/=fp->num_verts;

	for (i=0;i<fp->num_verts;i++)
	{
		float diff=fp->face_uvls[i].u-mid;

		diff*=scale;

		fp->face_uvls[i].u=mid+diff;
	}
}

void CTexAlignDialog::ExpandTextureV(int facenum,bool expand) 
{
	ASSERT(facenum != -1);
	if (facenum == -1)
		return;

	face *fp=&m_pPrim->roomp->faces[facenum];

	int i;
	float mid=0;
	float scale;
	(expand) ? scale = (scale = 1.0/D3EditState.texscale) : (scale = D3EditState.texscale);

	for (i=0;i<fp->num_verts;i++)
		mid+=fp->face_uvls[i].v;

	mid/=fp->num_verts;

	for (i=0;i<fp->num_verts;i++)
	{
		float diff=fp->face_uvls[i].v-mid;

		diff*=scale;

		fp->face_uvls[i].v=mid+diff;
	}
}

void CTexAlignDialog::OnUVExpandU() 
{
	// TODO: Add your control notification handler code here
	if (D3EditState.texscale == 1.0f)
	{
		OutrageMessageBox("This command has no effect when scale is set to 1.0");
		return;
	}

	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			ExpandTextureU(facenums[i],true);
	else if (m_pPrim->face != -1)
		ExpandTextureU(m_pPrim->face,true);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVExpandV() 
{
	// TODO: Add your control notification handler code here
	if (D3EditState.texscale == 1.0f)
	{
		OutrageMessageBox("This command has no effect when scale is set to 1.0");
		return;
	}

	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			ExpandTextureV(facenums[i],true);
	else if (m_pPrim->face != -1)
		ExpandTextureV(m_pPrim->face,true);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVShrinkU() 
{
	// TODO: Add your control notification handler code here
	if (D3EditState.texscale == 1.0f)
	{
		OutrageMessageBox("This command has no effect when scale is set to 1.0");
		return;
	}

	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			ExpandTextureU(facenums[i],false);
	else if (m_pPrim->face != -1)
		ExpandTextureU(m_pPrim->face,false);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

void CTexAlignDialog::OnUVShrinkV() 
{
	// TODO: Add your control notification handler code here
	if (D3EditState.texscale == 1.0f)
	{
		OutrageMessageBox("This command has no effect when scale is set to 1.0");
		return;
	}

	if (m_Faces == MARKED)
		for (int i=0; i<num_m_faces; i++)
			ExpandTextureV(facenums[i],false);
	else if (m_pPrim->face != -1)
		ExpandTextureV(m_pPrim->face,false);
	else
		return;

	RoomChanged(m_pPrim->roomp);
//	if (!m_bTimerSet)
//		SetTexTimer();
}

int FindNeighbor(room *rp,int facenum,int edgenum);

void PropagateFromMarkedFace(room *rp,short *facenums,int num_faces,int facenum,ubyte *face_flags,bool matching_faces_only)
{
	face *fp = &rp->faces[facenum];

	face_flags[facenum] = 1;

	for (int v=0;v<fp->num_verts;v++) {
		int t;

		t = FindNeighbor(rp,facenum,v);

		// If the neighboring face is not marked, skip it
		bool marked = false;
		for (int i=0; i<num_faces; i++)
		{
			if (facenums[i] == t)
			{
				marked = true;
				break;
			}
		}
		if (!marked)
			continue;

		if ((t != -1) && !face_flags[t])
			if (!matching_faces_only || (rp->faces[t].tmap == fp->tmap)) {
				HTexturePropagateToFace(rp,t,rp,facenum);
				PropagateFromMarkedFace(rp,facenums,num_faces,t,face_flags,matching_faces_only);
			}
	}
}


void PropagateToFaces(room *rp,short *facenums,int num_faces)
{
	ubyte face_flags[MAX_FACES_PER_ROOM];

	//Clear flags
	for (int i=0;i<rp->num_faces;i++)
		face_flags[i] = 0;

	//Start recursive function
	PropagateFromMarkedFace(rp,facenums,num_faces,facenums[0],face_flags,false);

	RoomChanged(rp);
}


void CTexAlignDialog::OnUVAlignMarked() 
{
	// TODO: Add your control notification handler code here
	if (m_pPrim->face != -1)
	{
		// Mark the current face (only so that it will be included in the list)
		m_pRoomFrm->MarkFace(m_pPrim->face);

		// Get list of marked faces
		int num_m_faces = m_pRoomFrm->GetMarkedFaces(facenums);
		ASSERT(num_m_faces == m_pRoomFrm->m_Num_marked_faces);

		if (num_m_faces > 1)
			PropagateToFaces(m_pPrim->roomp,facenums,num_m_faces);
		else
			OutrageMessageBox("No marked faces!");

		// Unmark the current face
		m_pRoomFrm->UnMarkFace(m_pPrim->face);
	}
	else
		OutrageMessageBox("No current face!");
}


void CTexAlignDialog::FaceMapTexture(int facenum) 
{
	ASSERT(facenum != -1);
	if (facenum == -1)
		return;

	face *fp=&m_pPrim->roomp->faces[facenum];

	if (fp->num_verts!=4)
	{	
		OutrageMessageBox ("Face must have four vertices to use face mapping.");
		return;
	}

	// Simply square off this face

	fp->face_uvls[0].u=0.0;
	fp->face_uvls[0].v=0.0;

	fp->face_uvls[1].u=1.0;
	fp->face_uvls[1].v=0.0;

	fp->face_uvls[2].u=1.0;
	fp->face_uvls[2].v=1.0;

	fp->face_uvls[3].u=0.0;
	fp->face_uvls[3].v=1.0;
}

void CTexAlignDialog::OnUVFaceMap() 
{
	// TODO: Add your control notification handler code here
	bool nomap = false;

	if (m_Faces == MARKED)
	{
		for (int i=0; i<num_m_faces; i++)
		{
			if (m_pPrim->roomp->faces[facenums[i]].num_verts!=4)
				nomap = true;
		}
		if (nomap)
		{
			OutrageMessageBox("One or more marked faces have more than four vertices. Operation aborted.");
			return;
		}
		for (int i=0; i<num_m_faces; i++)
			FaceMapTexture(facenums[i]);
	}
	else if (m_pPrim->face != -1)
		FaceMapTexture(m_pPrim->face);
	else
		return;

	RoomChanged(m_pPrim->roomp);
}


void CTexAlignDialog::OnUVAlignEdge() 
{
	// TODO: Add your control notification handler code here
	MessageBox("Not yet supported!");
	return;

	RoomChanged(m_pPrim->roomp);
}

/*
void CTexAlignDialog::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	switch (nIDEvent)
	{
	case TIMER_TEXALIGN:
		if ( ((CButton *)GetDlgItem(IDC_UV_DOWN))->GetState()&0x0004 )
			OnUVDown();
		else if ( ((CButton *)GetDlgItem(IDC_UV_LEFT))->GetState()&0x0004 )
			OnUVLeft();
		else if ( ((CButton *)GetDlgItem(IDC_UV_RIGHT))->GetState()&0x0004 )
			OnUVRight();
		else if ( ((CButton *)GetDlgItem(IDC_UV_ROTLEFT))->GetState()&0x0004 )
			OnUVRotLeft();
		else if ( ((CButton *)GetDlgItem(IDC_UV_ROTRIGHT))->GetState()&0x0004 )
			OnUVRotRight();
		else if ( ((CButton *)GetDlgItem(IDC_UV_STRETCHLESS))->GetState()&0x0004 )
			OnUVStretchLess();
		else if ( ((CButton *)GetDlgItem(IDC_UV_STRETCHMORE))->GetState()&0x0004 )
			OnUVStretchMore();
		else if ( ((CButton *)GetDlgItem(IDC_UV_UP))->GetState()&0x0004 )
			OnUVUp();
		else if ( ((CButton *)GetDlgItem(IDC_UV_EXPAND_U))->GetState()&0x0004 )
			OnUVExpandU();
		else if ( ((CButton *)GetDlgItem(IDC_UV_EXPAND_V))->GetState()&0x0004 )
			OnUVExpandV();
		else if ( ((CButton *)GetDlgItem(IDC_UV_SHRINK_U))->GetState()&0x0004 )
			OnUVShrinkU();
		else if ( ((CButton *)GetDlgItem(IDC_UV_SHRINK_V))->GetState()&0x0004 )
			OnUVShrinkV();
		else
		{
			m_bTimerSet = false;
			KillTimer(TIMER_TEXALIGN);
		}
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

void CTexAlignDialog::SetTexTimer()
{
	SetTimer(TIMER_TEXALIGN,100,NULL);
	m_bTimerSet = true;
}
*/


void CTexAlignDialog::OnCurrentFace() 
{
	// TODO: Add your control notification handler code here
	m_Faces = CURRENT;
}

void CTexAlignDialog::OnMarkedFaces() 
{
	// TODO: Add your control notification handler code here
	m_Faces = MARKED;
}



BOOL CTexAlignDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CheckRadioButton(IDC_CURRENTFACE,IDC_MARKEDFACES,IDC_CURRENTFACE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
