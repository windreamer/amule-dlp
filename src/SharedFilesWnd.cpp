//
// This file is part of the aMule Project.
//
// Copyright (c) 2003-2008 aMule Team ( admin@amule.org / http://www.amule.org )
// Copyright (c) 2002-2008 Merkur ( devs@emule-project.net / http://www.emule-project.net )
//
// Any parts of this program derived from the xMule, lMule or eMule project,
// or contributed by third-party developers are copyrighted by their
// respective authors.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA
//

#include <wx/config.h>
#include <wx/gauge.h>		// Do_not_auto_remove (win32)

#include "SharedFilesWnd.h"	// Interface declarations
#include "SharedFilesCtrl.h"
#include "SharedFilePeersListCtrl.h"
#include "muuli_wdr.h"		// Needed for ID_SHFILELIST
#include "KnownFileList.h"	// Needed for CKnownFileList
#include "KnownFile.h"		// Needed for CKnownFile
#include "amule.h"			// Needed for theApp


BEGIN_EVENT_TABLE(CSharedFilesWnd, wxPanel)
	EVT_LIST_ITEM_SELECTED( ID_SHFILELIST,	CSharedFilesWnd::OnItemSelectionChanged )
	EVT_LIST_ITEM_DESELECTED( ID_SHFILELIST,	CSharedFilesWnd::OnItemSelectionChanged )
	EVT_BUTTON( ID_BTNRELSHARED,			CSharedFilesWnd::OnBtnReloadShared )
 	EVT_BUTTON(ID_SHAREDCLIENTTOGGLE,		CSharedFilesWnd::OnToggleClientList)
	
	EVT_SPLITTER_SASH_POS_CHANGING(ID_SHARESSPLATTER, CSharedFilesWnd::OnSashPositionChanging)
END_EVENT_TABLE()

CSharedFilesWnd::CSharedFilesWnd( wxWindow* pParent )
	: wxPanel(pParent, -1)
{
	wxSizer* content = sharedfilesDlg(this, true);
	content->Show(this, true);

	m_bar_requests	= CastChild( wxT("popbar"), wxGauge );
	m_bar_accepted	= CastChild( wxT("popbarAccept"), wxGauge );
	m_bar_transfer	= CastChild( wxT("popbarTrans"), wxGauge );
	sharedfilesctrl = CastChild( wxT("sharedFilesCt"), CSharedFilesCtrl );
	peerslistctrl   = CastChild( ID_SHAREDCLIENTLIST, CSharedFilePeersListCtrl );
	wxASSERT(sharedfilesctrl);
	wxASSERT(peerslistctrl);
	m_prepared		= false;
	
	m_splitter = 0;	
	
	wxConfigBase *config = wxConfigBase::Get();
	
	// Check if the clientlist is hidden
	bool show = true;
	config->Read( wxT("/GUI/SharedWnd/ShowClientList"), &show, true );
	peerslistctrl->SetShowing(show);
	// Load the last used splitter position
	m_splitter = config->Read( wxT("/GUI/SharedWnd/Splitter"), 463l );
}


CSharedFilesWnd::~CSharedFilesWnd()
{
	wxConfigBase *config = wxConfigBase::Get();

	if ( !peerslistctrl->GetShowing() ) {
		// Save the splitter position
		config->Write( wxT("/GUI/SharedWnd/Splitter"), m_splitter );
	
		// Save the visible status of the list
		config->Write( wxT("/GUI/SharedWnd/ShowClientList"), false );
	} else {
		wxSplitterWindow* splitter = CastChild( wxT("sharedsplitterWnd"), wxSplitterWindow );
		
		// Save the splitter position
		config->Write( wxT("/GUI/SharedWnd/Splitter"), splitter->GetSashPosition() );		
		
		// Save the visible status of the list
		config->Write( wxT("/GUI/SharedWnd/ShowClientList"), true );
	}	
}


void CSharedFilesWnd::SelectionUpdated()
{
	if (!sharedfilesctrl->IsSorting()) {
		uint64 lTransferred = theApp->knownfiles->transferred;
		uint32 lAccepted = theApp->knownfiles->accepted;
		uint32 lRequested = theApp->knownfiles->requested;
		m_bar_requests->SetRange( lRequested );
		m_bar_accepted->SetRange( lAccepted );
		m_bar_transfer->SetRange( lTransferred / 1024 );
		
		CKnownFileVector fileVector;
		
		if ( !sharedfilesctrl->GetSelectedItemCount() ) {
			// Requests
			m_bar_requests->SetValue( 0 );
			CastChild(IDC_SREQUESTED, wxStaticText)->SetLabel( wxT("- / -") );

			// Accepted requets
			m_bar_accepted->SetValue( 0 );
			CastChild(IDC_SACCEPTED, wxStaticText)->SetLabel( wxT("- / -") );

			// Transferred
			m_bar_transfer->SetValue( 0 );
			CastChild(IDC_STRANSFERRED, wxStaticText)->SetLabel( wxT("- / -") );
			
		} else {
			// Create a total statistic for the selected item(s)
			uint32 session_requests = 0;
			uint32 session_accepted = 0;
			uint64 session_transferred = 0;
			uint32 all_requests = 0;
			uint32 all_accepted = 0;
			uint64 all_transferred = 0;
			
			long index = -1;

			fileVector.reserve(sharedfilesctrl->GetSelectedItemCount());
			
			while ( (index = sharedfilesctrl->GetNextItem( index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED )) != -1) {
				if ( sharedfilesctrl->GetItemState( index, wxLIST_STATE_SELECTED ) ) {
					CKnownFile* file = (CKnownFile*)sharedfilesctrl->GetItemData( index );
					wxASSERT(file);
					
					session_requests   += file->statistic.GetRequests();
					session_accepted   += file->statistic.GetAccepts();
					session_transferred += file->statistic.GetTransferred();
			
					all_requests   += file->statistic.GetAllTimeRequests();
					all_accepted   += file->statistic.GetAllTimeAccepts();
					all_transferred += file->statistic.GetAllTimeTransferred();
					
					fileVector.push_back(file);
				}
			};

			std::sort(fileVector.begin(), fileVector.end());			
			
			// Store text lengths, and layout() when the texts have grown
			static uint32 lReq = 0, lAcc = 0, lTrans = 0;
			// Requests
			session_requests = session_requests > lRequested ? lRequested : session_requests;
			m_bar_requests->SetValue( session_requests );
			wxString labelReq = CFormat(wxT("%d / %d")) % session_requests % all_requests;
			CastChild(IDC_SREQUESTED, wxStaticText)->SetLabel(labelReq);
		
			// Accepted requets
			session_accepted = session_accepted > lAccepted ? lAccepted : session_accepted;
			m_bar_accepted->SetValue( session_accepted );
			wxString labelAcc = CFormat(wxT("%d / %d")) % session_accepted % all_accepted;
			CastChild(IDC_SACCEPTED, wxStaticText)->SetLabel(labelAcc);

			// Transferred
			session_transferred = session_transferred > lTransferred ? lTransferred : session_transferred;
			m_bar_transfer->SetValue( session_transferred / 1024 );
			wxString labelTrans = CastItoXBytes( session_transferred ) + wxT(" / ") + CastItoXBytes( all_transferred );
			CastChild(IDC_STRANSFERRED, wxStaticText)->SetLabel(labelTrans);

			if (labelReq.Len() > lReq || labelAcc.Len() > lAcc || labelTrans.Len() > lTrans) {
				lReq = labelReq.Len();
				lAcc = labelAcc.Len();
				lTrans = labelTrans.Len();
				s_sharedfilespeerHeader->Layout();
			}
		}
	
		this->peerslistctrl->ShowSources(fileVector);
		
		Refresh();
		Layout();
	}
}


void CSharedFilesWnd::OnBtnReloadShared( wxCommandEvent& WXUNUSED(evt) )
{
	theApp->sharedfiles->Reload();
#ifndef CLIENT_GUI
	// remote gui will update display when data is back
	SelectionUpdated();
#endif
}


void CSharedFilesWnd::OnItemSelectionChanged(wxListEvent& evt)
{
	SelectionUpdated();

	evt.Skip();
}


void CSharedFilesWnd::RemoveAllSharedFiles() {
	sharedfilesctrl->DeleteAllItems();
	sharedfilesctrl->ShowFilesCount();
	peerslistctrl->DeleteAllItems();
	SelectionUpdated();
}

void CSharedFilesWnd::Prepare()
{
	if (m_prepared) {
		return;
	}
	m_prepared = true;
	wxSplitterWindow* splitter = CastChild( wxT("sharedsplitterWnd"), wxSplitterWindow );
	int height = splitter->GetSize().GetHeight();
	int header_height = s_sharedfilespeerHeader->GetSize().GetHeight();	
	
	if ( m_splitter ) {
		// Some sanity checking
		if ( m_splitter < 90 ) {
			m_splitter = 90;
		} else if ( m_splitter > height - header_height * 2 ) {
			m_splitter = height - header_height * 2;
		}
		splitter->SetSashPosition( m_splitter );
		m_splitter = 0;
	}

	if ( !peerslistctrl->GetShowing() ) {
		// use a toggle event to close it (calculate size, change button)
		peerslistctrl->SetShowing( true );	// so it will be toggled to false
		wxCommandEvent evt1;
		OnToggleClientList( evt1 );
	}
}

void CSharedFilesWnd::OnToggleClientList(wxCommandEvent& WXUNUSED(evt))
{
	wxSplitterWindow* splitter = CastChild( wxT("sharedsplitterWnd"), wxSplitterWindow );
	wxBitmapButton*   button = CastChild( ID_SHAREDCLIENTTOGGLE, wxBitmapButton );
	
	if ( !peerslistctrl->GetShowing() ) {
		splitter->SetSashPosition( m_splitter );
		m_splitter = 0;
		
		peerslistctrl->SetShowing( true );
		
		button->SetBitmapLabel( amuleDlgImages( 10 ) );
		button->SetBitmapFocus( amuleDlgImages( 10 ) );
		button->SetBitmapSelected( amuleDlgImages( 10 ) );
		button->SetBitmapHover( amuleDlgImages( 10 ) );
	} else {
		peerslistctrl->SetShowing( false );
	
		m_splitter = splitter->GetSashPosition();
	
		// Add the height of the listctrl to the top-window
		int height = peerslistctrl->GetSize().GetHeight()
					 + splitter->GetWindow1()->GetSize().GetHeight();
	
		splitter->SetSashPosition( height );

		button->SetBitmapLabel( amuleDlgImages( 11 ) );
		button->SetBitmapFocus( amuleDlgImages( 11 ) );
		button->SetBitmapSelected( amuleDlgImages( 11 ) );
		button->SetBitmapHover( amuleDlgImages( 11 ) );
	}
}

void CSharedFilesWnd::OnSashPositionChanging(wxSplitterEvent& evt)
{
	if ( evt.GetSashPosition() < 90 ) {
		evt.SetSashPosition( 90 );
	} else {
		wxSplitterWindow* splitter = wxStaticCast( evt.GetEventObject(), wxSplitterWindow);
		wxCHECK_RET(splitter, wxT("ERROR: NULL splitter in CSharedFilesWnd::OnSashPositionChanging"));
		
		int height = splitter->GetSize().GetHeight();
		int header_height = s_sharedfilespeerHeader->GetSize().GetHeight();	
		int mousey = wxGetMousePosition().y - splitter->GetScreenRect().GetTop();

		if ( !peerslistctrl->GetShowing() ) {
			// lower window hidden
			if ( height - mousey < header_height * 2 ) {
				// no moving down if already hidden
				evt.Veto();
			} else {
				// show it
				m_splitter = mousey;	// prevent jumping if it was minimized and is then shown by dragging the sash
				wxCommandEvent evt1;
				OnToggleClientList( evt1 );
			}
		} else {
			// lower window showing
			if ( height - mousey < header_height * 2 ) {
				// hide it
				wxCommandEvent evt1;
				OnToggleClientList( evt1 );
			} else {
				// normal resize
				// If several events queue up, setting the sash to the current mouse position
				// will speed up things and make sash moving more smoothly.
				evt.SetSashPosition( mousey );
			}
		}
	}
}
// File_checked_for_headers
