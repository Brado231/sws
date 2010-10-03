/******************************************************************************
/ ItemParams.cpp
/
/ Copyright (c) 2010 Tim Payne (SWS)
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


#include "stdafx.h"
#include "ItemParams.h"
#include "TrackSel.h"

void TogItemMute(COMMAND_T* = NULL)
{	// Toggle item's mutes on selected tracks
	for (int i = 1; i <= GetNumTracks(); i++)
	{
		MediaTrack* tr = CSurf_TrackFromID(i, false);
		if (*(int*)GetSetMediaTrackInfo(tr, "I_SELECTED", NULL))
			for (int j = 0; j < GetTrackNumMediaItems(tr); j++)
			{
				MediaItem* mi = GetTrackMediaItem(tr, j);
				bool bMute = !*(bool*)GetSetMediaItemInfo(mi, "B_MUTE", NULL);
				GetSetMediaItemInfo(mi, "B_MUTE", &bMute);
			}
	}
	UpdateTimeline();
}

void DelAllItems(COMMAND_T* = NULL)
{
	for (int i = 1; i <= GetNumTracks(); i++)
	{
		MediaTrack* tr = CSurf_TrackFromID(i, false);
		if (*(int*)GetSetMediaTrackInfo(tr, "I_SELECTED", NULL))
			while(GetTrackNumMediaItems(tr))
				DeleteTrackMediaItem(tr, GetTrackMediaItem(tr, 0));
	}
	UpdateTimeline();
	Undo_OnStateChangeEx("Delete all items on selected track(s)", UNDO_STATE_ITEMS, -1);
}

// I rarely want to toggle the loop item section, I just want to reset it!
// So here, check it's state, then set/reset it, and also make sure loop item is on.
void LoopItemSection(COMMAND_T*)
{
	WDL_PtrList<void> items;
	WDL_PtrList<void> sections;
	for (int i = 0; i < CountSelectedMediaItems(0); i++)
	{
		MediaItem* item = GetSelectedMediaItem(0, i);
		items.Add(item);
		MediaItem_Take* take = GetMediaItemTake(item, -1);
		if (take && strcmp(((PCM_source*)GetSetMediaItemTakeInfo(take, "P_SOURCE", NULL))->GetType(), "SECTION") == 0)
			sections.Add(item);
	}

	if (sections.GetSize())
	{	// Some items already are in "section mode", turn it off
		Main_OnCommand(40289, 0); // Unselect all items
		for (int i = 0; i < sections.GetSize(); i++)
		{
			MediaItem* item = (MediaItem*)sections.Get(i);
			GetSetMediaItemInfo(item, "B_UISEL", &g_bTrue);
			Main_OnCommand(40547, 0); // Toggle loop section of item source
			GetSetMediaItemInfo(item, "B_UISEL", &g_bFalse);
		}
		// Restore item selection
		for (int i = 0; i < items.GetSize(); i++)
			GetSetMediaItemInfo((MediaItem*)items.Get(i), "B_UISEL", &g_bTrue);
	}

	// Turn on loop item source
	for (int i = 0; i < items.GetSize(); i++)
		GetSetMediaItemInfo((MediaItem*)items.Get(i), "B_LOOPSRC", &g_bTrue);

	// Turn on loop section
	Main_OnCommand(40547, 0);
}

void MoveItemLeftToCursor(COMMAND_T* = NULL)
{
	double dEditCur = GetCursorPosition();
	// 
	for (int i = 1; i <= GetNumTracks(); i++)
	{
		MediaTrack* tr = CSurf_TrackFromID(i, false);
		for (int j = 0; j < GetTrackNumMediaItems(tr); j++)
		{
			MediaItem* mi = GetTrackMediaItem(tr, j);
			if (*(bool*)GetSetMediaItemInfo(mi, "B_UISEL", NULL))
			{
				GetSetMediaItemInfo(mi, "D_POSITION", &dEditCur);
			}
		}
	}
	UpdateTimeline();
	Undo_OnStateChangeEx("Move selected item(s) left edge to edit cursor", UNDO_STATE_ITEMS, -1);
}

void MoveItemRightToCursor(COMMAND_T* = NULL)
{
	double dEditCur = GetCursorPosition();
	// 
	for (int i = 1; i <= GetNumTracks(); i++)
	{
		MediaTrack* tr = CSurf_TrackFromID(i, false);
		for (int j = 0; j < GetTrackNumMediaItems(tr); j++)
		{
			MediaItem* mi = GetTrackMediaItem(tr, j);
			if (*(bool*)GetSetMediaItemInfo(mi, "B_UISEL", NULL))
			{
				double dNewPos = dEditCur - *(double*)GetSetMediaItemInfo(mi, "D_LENGTH", NULL);
				if (dNewPos >= 0.0)
					GetSetMediaItemInfo(mi, "D_POSITION", &dNewPos);
			}
		}
	}
	UpdateTimeline();
	Undo_OnStateChangeEx("Move selected item(s) right edge to edit cursor", UNDO_STATE_ITEMS, -1);
}

void InsertFromTrackName(COMMAND_T*)
{
	WDL_PtrList<void> tracks;
	for (int i = 0; i < CountSelectedTracks(0); i++)
		tracks.Add(GetSelectedTrack(0, i));

	if (!tracks.GetSize())
		return;

	char cPath[256];
	EnumProjects(-1, cPath, 256);
	char* pSlash = strrchr(cPath, PATH_SLASH_CHAR);
	if (pSlash)
		pSlash[1] = 0;
	
	double dCurPos = GetCursorPosition();
	for (int i = 0; i < tracks.GetSize(); i++)
	{
		ClearSelected();
		MediaTrack* tr = (MediaTrack*)tracks.Get(i);
		GetSetMediaTrackInfo(tr, "I_SELECTED", &g_i1);
		SetLTT();
		WDL_String cFilename;
		cFilename.Set((char*)GetSetMediaTrackInfo(tr, "P_NAME", NULL));
		if (cFilename.GetLength())
		{
			if (FileExists(cFilename.Get()))
				InsertMedia(cFilename.Get(), 0);
			else
			{
				cFilename.Insert(cPath, 0);
				if (FileExists(cFilename.Get()))
					InsertMedia(cFilename.Get(), 0);
			}
			SetEditCurPos(dCurPos, false, false);
		}
	}
	// Restore selected
	for (int i = 0; i < tracks.GetSize(); i++)
		GetSetMediaTrackInfo((MediaTrack*)tracks.Get(i), "I_SELECTED", &g_i1);
}

double QuantizeTime(double dTime, double dGrid, double dMin, double dMax)
{
	double dBeatPos = TimeMap_timeToQN(dTime);
	dBeatPos += dGrid / 2.0;
	dBeatPos -= fmod(dBeatPos, dGrid);
	double dNewTime = TimeMap_QNToTime(dBeatPos);
	while (dNewTime <= dMin)
	{
		dBeatPos += dGrid;
		dNewTime = TimeMap_QNToTime(dBeatPos);
	}
	while (dNewTime >= dMax)
	{
		dBeatPos -= dGrid;
		dNewTime = TimeMap_QNToTime(dBeatPos);
	}
	return dNewTime;
}

// 1: Quant start, keep len
// 2: Quant start, keep end
// 3: Quant end, keep len
// 4: Quant end, keep start
// 5: Quant both
void QuantizeItemEdges(COMMAND_T* t)
{
	// Eventually a dialog?  for now quantize to grid
	double div = *(double*)GetConfigVar("projgriddiv");
	for (int i = 0; i < CountSelectedMediaItems(NULL); i++)
	{
		MediaItem* item = GetSelectedMediaItem(NULL, i);
		double dStart = *(double*)GetSetMediaItemInfo(item, "D_POSITION", NULL);
		double dOffset = *(double*)GetSetMediaItemInfo(item, "D_SNAPOFFSET", NULL);
		double dLen   = *(double*)GetSetMediaItemInfo(item, "D_LENGTH", NULL);
		double dEnd   = dStart + dLen;

		// Correct for snap offset
		dStart += dOffset;
		dLen -= dOffset;

		switch(t->user)
		{
		case 1:
			dStart = QuantizeTime(dStart, div, -DBL_MAX, dEnd);
			break;
		case 2:
			dStart = QuantizeTime(dStart, div, -DBL_MAX, dEnd);
			dLen   = dEnd - dStart;
			break;
		case 3:
			dEnd = QuantizeTime(dEnd, div, dStart, DBL_MAX);
			dStart = dEnd - dLen;
			break;
		case 4:
			dEnd = QuantizeTime(dEnd, div, dStart, DBL_MAX);
			dLen = dEnd - dStart;
			break;
		case 5:
			dStart = QuantizeTime(dStart, div, -DBL_MAX, dEnd);
			dEnd = QuantizeTime(dEnd, div, dStart, DBL_MAX);
			dLen = dEnd - dStart;
			break;
		}

		// Uncorrect for the snap offset
		dStart -= dOffset;
		dLen += dOffset;

		GetSetMediaItemInfo(item, "D_POSITION", &dStart);
		GetSetMediaItemInfo(item, "D_LENGTH",	&dLen);
	}
	UpdateTimeline();
	Undo_OnStateChangeEx(SWS_CMD_SHORTNAME(t), UNDO_STATE_ITEMS, -1);
}

void SetChanModeAllTakes(COMMAND_T* t)
{
	int mode = (int)t->user;
	for (int i = 0; i < CountSelectedMediaItems(NULL); i++)
	{
		MediaItem* item = GetSelectedMediaItem(NULL, i);
		for (int j = 0; j < CountTakes(item); j++)
			GetSetMediaItemTakeInfo(GetTake(item, j), "I_CHANMODE", &mode);
	}

	UpdateTimeline();
	Undo_OnStateChangeEx(SWS_CMD_SHORTNAME(t), UNDO_STATE_ITEMS, -1);
}

void SetPreservePitch(COMMAND_T* t)
{
	bool bPP = t->user ? true : false;
	for (int i = 0; i < CountSelectedMediaItems(NULL); i++)
	{
		MediaItem* item = GetSelectedMediaItem(NULL, i);
		for (int j = 0; j < CountTakes(item); j++)
			GetSetMediaItemTakeInfo(GetTake(item, j), "B_PPITCH", &bPP);
	}

	UpdateTimeline();
	Undo_OnStateChangeEx(SWS_CMD_SHORTNAME(t), UNDO_STATE_ITEMS, -1);
}

void SetPitch(COMMAND_T* t)
{
	for (int i = 0; i < CountSelectedMediaItems(NULL); i++)
	{
		MediaItem* item = GetSelectedMediaItem(NULL, i);
		for (int j = 0; j < CountTakes(item); j++)
		{
			double dPitch = (double)t->user / 100.0;
			if (dPitch != 0.0)
				dPitch += *(double*)GetSetMediaItemTakeInfo(GetTake(item, j), "D_PITCH", NULL);
			GetSetMediaItemTakeInfo(GetTake(item, j), "D_PITCH", &dPitch);
		}
	}

	UpdateTimeline();
	Undo_OnStateChangeEx(SWS_CMD_SHORTNAME(t), UNDO_STATE_ITEMS, -1);
}

// Original code by FNG/fingers
// Modified by SWS to change the rate of all takes and to use snap offset
void NudgePlayrate(COMMAND_T *t)
{
	for(int i = 0; i < CountSelectedMediaItems(NULL); i++)
	{
		MediaItem* item = GetSelectedMediaItem(NULL, i);
		double snapOffset = *(double*)GetSetMediaItemInfo(item, "D_SNAPOFFSET", NULL);
		for (int j = 0; j < CountTakes(item); j++)
		{
			MediaItem_Take* take = GetActiveTake(item);
			double rate  = *(double*)GetSetMediaItemTakeInfo(take, "D_PLAYRATE", NULL);
			double newRate = rate * pow(2.0, 1.0/12.0 / (double)t->user);
			GetSetMediaItemTakeInfo(take, "D_PLAYRATE", &newRate);
			GetSetMediaItemTakeInfo(take, "B_PPITCH", &g_bFalse);
			
			// Take into account the snap offset
			if (snapOffset != 0.0)
			{
				// Rate can be clipped by Reaper, re-read it here
				newRate = *(double*)GetSetMediaItemTakeInfo(take, "D_PLAYRATE", NULL);
				double offset = *(double*)GetSetMediaItemTakeInfo(take, "D_STARTOFFS", NULL);
				offset += (1.0 - newRate / rate) * snapOffset * rate;
				if (offset < 0.0)
					offset = 0.0;
				GetSetMediaItemTakeInfo(take, "D_STARTOFFS", &offset);
			}
		}
	}
	UpdateTimeline();
	Undo_OnStateChangeEx(SWS_CMD_SHORTNAME(t), UNDO_STATE_ITEMS, -1);
}

void CrossfadeSelItems(COMMAND_T* t)
{
	double dFadeLen = fabs(*(double*)GetConfigVar("deffadelen")); // Abs because neg value means "not auto"
	double dEdgeAdj = dFadeLen / 2.0;
	bool bChanges = false;

	for (int iTrack = 1; iTrack <= GetNumTracks(); iTrack++)
	{
		MediaTrack* tr = CSurf_TrackFromID(iTrack, false);
		for (int iItem1 = 0; iItem1 < GetTrackNumMediaItems(tr); iItem1++)
		{
			MediaItem* item1 = GetTrackMediaItem(tr, iItem1);
			if (*(bool*)GetSetMediaItemInfo(item1, "B_UISEL", NULL))
			{
				double dStart1 = *(double*)GetSetMediaItemInfo(item1, "D_POSITION", NULL);
				double dEnd1   = *(double*)GetSetMediaItemInfo(item1, "D_LENGTH", NULL) + dStart1;

				// Try to match up the end with the start of the other selected media items
				for (int iItem2 = 0; iItem2 < GetTrackNumMediaItems(tr); iItem2++)
				{
					MediaItem* item2 = GetTrackMediaItem(tr, iItem2);
					if (item1 != item2 && *(bool*)GetSetMediaItemInfo(item2, "B_UISEL", NULL))
					{
						double dStart2 = *(double*)GetSetMediaItemInfo(item2, "D_POSITION", NULL);
						double dTest = fabs(dEnd1 - dStart2);

						// Need a tolerance for "items are adjacent".
						// Found one case of items after split having diff edges 2.0e-14 apart, hopefully 1.0e-13 is enough (but not too much)
						if (fabs(dEnd1 - dStart2) < 1.0e-13)
						{	// Found a matching item
							// Need to ensure that there's "room" to move the start of the second item back
							// Check all of the takes' start offset before doing any "work"
							int iTake;
							for (iTake = 0; iTake < GetMediaItemNumTakes(item2); iTake++)
								if (dEdgeAdj > *(double*)GetSetMediaItemTakeInfo(GetMediaItemTake(item2, iTake), "D_STARTOFFS", NULL))
									break;
							if (iTake < GetMediaItemNumTakes(item2))
								continue;	// Keep looking

							// We're all good, move the edges around and set the crossfades
							double dLen1 = dEnd1 - dStart1 + dEdgeAdj;
							GetSetMediaItemInfo(item1, "D_LENGTH", &dLen1);
							GetSetMediaItemInfo(item1, "D_FADEOUTLEN_AUTO", &dFadeLen);

							double dLen2 = *(double*)GetSetMediaItemInfo(item2, "D_LENGTH", NULL);
							dStart2 -= dEdgeAdj;
							dLen2   += dEdgeAdj;
							GetSetMediaItemInfo(item2, "D_POSITION", &dStart2);
							GetSetMediaItemInfo(item2, "D_LENGTH", &dLen2);
							GetSetMediaItemInfo(item2, "D_FADEINLEN_AUTO", &dFadeLen);
							double dSnapOffset2 = *(double*)GetSetMediaItemInfo(item2, "D_SNAPOFFSET", NULL);
							if (dSnapOffset2)
							{	// Only adjust the snap offset if it's non-zero
								dSnapOffset2 += dEdgeAdj;
								GetSetMediaItemInfo(item2, "D_SNAPOFFSET", &dSnapOffset2);
							}

							for (iTake = 0; iTake < GetMediaItemNumTakes(item2); iTake++)
							{
								MediaItem_Take* take = GetMediaItemTake(item2, iTake);
								if (take)
								{
									double dOffset = *(double*)GetSetMediaItemTakeInfo(take, "D_STARTOFFS", NULL);
									dOffset -= dEdgeAdj;
									GetSetMediaItemTakeInfo(take, "D_STARTOFFS", &dOffset);
								}
							}
							bChanges = true;
							break;
						}
					}
				}
			}
		}
	}
	if (bChanges)
	{
		UpdateTimeline();
		Undo_OnStateChangeEx(SWS_CMD_SHORTNAME(t), UNDO_STATE_ITEMS, -1);
	}
}

static COMMAND_T g_commandTable[] = 
{
	{ { DEFACCEL, "SWS: Toggle mute of items on selected track(s)" },			"SWS_TOGITEMMUTE",		TogItemMute,			},
	{ { DEFACCEL, "SWS: Delete all items on selected track(s)" },				"SWS_DELALLITEMS",		DelAllItems,			},
	{ { DEFACCEL, "SWS: Loop section of selected item(s)" },					"SWS_LOOPITEMSECTION",	LoopItemSection,		},
	{ { DEFACCEL, "SWS: Move selected item(s) left edge to edit cursor" },		"SWS_ITEMLEFTTOCUR",	MoveItemLeftToCursor,	},
	{ { DEFACCEL, "SWS: Move selected item(s) right edge to edit cursor" },		"SWS_ITEMRIGHTTOCUR",	MoveItemRightToCursor,	},
	{ { DEFACCEL, "SWS: Insert file matching selected track(s) name" },			"SWS_INSERTFROMTN",		InsertFromTrackName,	},
	{ { DEFACCEL, "SWS: Quantize item's start to grid (keep length)" },			"SWS_QUANTITESTART2",	QuantizeItemEdges, NULL, 1 },
	{ { DEFACCEL, "SWS: Quantize item's start to grid (change length)" },		"SWS_QUANTITESTART1",	QuantizeItemEdges, NULL, 2 },
	{ { DEFACCEL, "SWS: Quantize item's end to grid (keep length)" },			"SWS_QUANTITEEND2",		QuantizeItemEdges, NULL, 3 },
	{ { DEFACCEL, "SWS: Quantize item's end to grid (change length)" },			"SWS_QUANTITEEND1",		QuantizeItemEdges, NULL, 4 },
	{ { DEFACCEL, "SWS: Quantize item's edges to grid (change length)" },		"SWS_QUANTITEEDGES",	QuantizeItemEdges, NULL, 5 },
	{ { DEFACCEL, "SWS: Set all takes channel mode to normal" },				"SWS_TAKESCHANMODE0",	SetChanModeAllTakes, NULL, 0 },
	{ { DEFACCEL, "SWS: Set all takes channel mode to reverse stereo" },		"SWS_TAKESCHANMODE1",	SetChanModeAllTakes, NULL, 1 },
	{ { DEFACCEL, "SWS: Set all takes channel mode to mono (downmix)" },		"SWS_TAKESCHANMODE2",	SetChanModeAllTakes, NULL, 2 },
	{ { DEFACCEL, "SWS: Set all takes channel mode to mono (left)" },			"SWS_TAKESCHANMODE3",	SetChanModeAllTakes, NULL, 3 },
	{ { DEFACCEL, "SWS: Set all takes channel mode to mono (right)" },			"SWS_TAKESCHANMODE4",	SetChanModeAllTakes, NULL, 4 },
	{ { DEFACCEL, "SWS: Clear all takes preserve pitch" },						"SWS_TAKESSPREVPITCH",	SetPreservePitch, NULL, 0 },
	{ { DEFACCEL, "SWS: Set all takes preserve pitch" },						"SWS_TAKESSPREVPITCH",	SetPreservePitch, NULL, 1 },
	{ { DEFACCEL, "SWS: Reset all takes' pitch" },								"SWS_TAKESPITCHRESET",	SetPitch, NULL, 0 },
	{ { DEFACCEL, "SWS: Pitch all takes up one cent" },							"SWS_TAKESPITCH_1C",	SetPitch, NULL, 1 },
	{ { DEFACCEL, "SWS: Pitch all takes up one semitone" },						"SWS_TAKESPITCH_1S",	SetPitch, NULL, 100 },
	{ { DEFACCEL, "SWS: Pitch all takes up one octave" },						"SWS_TAKESPITCH_1O",	SetPitch, NULL, 1200 },
	{ { DEFACCEL, "SWS: Pitch all takes down one cent" },						"SWS_TAKESPITCH-1C",	SetPitch, NULL, -1 },
	{ { DEFACCEL, "SWS: Pitch all takes down one semitone" },					"SWS_TAKESPITCH-1S",	SetPitch, NULL, -100 },
	{ { DEFACCEL, "SWS: Pitch all takes down one octave" },						"SWS_TAKESPITCH-1O",	SetPitch, NULL, -1200 },
	{ { DEFACCEL, "SWS: Crossfade adjacent selected items" },					"SWS_CROSSFADE",		CrossfadeSelItems, },

	{ { DEFACCEL, "SWS: Increase item rate by ~0.6% (10 cents) preserving length, clear 'preserve pitch'" },	"FNG_NUDGERATEUP",		NudgePlayrate, NULL, 10 },
	{ { DEFACCEL, "SWS: Decrease item rate by ~0.6% (10 cents) preserving length, clear 'preserve pitch'" },	"FNG_NUDGERATEDOWN",	NudgePlayrate, NULL, -10 },
	{ { DEFACCEL, "SWS: Increase item rate by ~6% (one semitone) preserving length, clear 'preserve pitch'" },	"FNG_INCREASERATE",		NudgePlayrate, NULL, 1 },
	{ { DEFACCEL, "SWS: Decrease item rate by ~6% (one semitone) preserving length, clear 'preserve pitch'"},	"FNG_DECREASERATE",		NudgePlayrate, NULL, -1 },

	{ {}, LAST_COMMAND, }, // Denote end of table
};

int ItemParamsInit()
{
	SWSRegisterCommands(g_commandTable);

	return 1;
}