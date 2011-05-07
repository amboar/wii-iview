/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_filebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "treebrowser.h"

/**
 * Constructor for the GuiTreeBrowser class.
 */
GuiTreeBrowser::GuiTreeBrowser(int w, int h)
{
	width = w;
	height = h;
	numEntries = 0;
	selectedItem = 0;
	selectable = true;
	listChanged = true; // trigger an initial list update
	focus = 0; // allow focus

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);

	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	bgTreeSelection = new GuiImageData(bg_file_selection_png);
	bgTreeSelectionImg = new GuiImage(bgTreeSelection);
	bgTreeSelectionImg->SetParent(this);
	bgTreeSelectionImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgTreeSelectionEntry = new GuiImageData(bg_file_selection_entry_png);
	treeFolder = new GuiImageData(folder_png);

	scrollbar = new GuiImageData(scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(0, 30);

	arrowDown = new GuiImageData(scrollbar_arrowdown_png);
	arrowDownImg = new GuiImage(arrowDown);
	arrowDownOver = new GuiImageData(scrollbar_arrowdown_over_png);
	arrowDownOverImg = new GuiImage(arrowDownOver);
	arrowUp = new GuiImageData(scrollbar_arrowup_png);
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOver = new GuiImageData(scrollbar_arrowup_over_png);
	arrowUpOverImg = new GuiImage(arrowUpOver);
	scrollbarBox = new GuiImageData(scrollbar_box_png);
	scrollbarBoxImg = new GuiImage(scrollbarBox);
	scrollbarBoxOver = new GuiImageData(scrollbar_box_over_png);
	scrollbarBoxOverImg = new GuiImage(scrollbarBoxOver);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	arrowUpBtn->SetPosition(0, -2);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetClickable(false);
	arrowUpBtn->SetHoldable(true);
	arrowUpBtn->SetTrigger(trigHeldA);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetClickable(false);
	arrowDownBtn->SetHoldable(true);
	arrowDownBtn->SetTrigger(trigHeldA);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);

	scrollbarBoxBtn = new GuiButton(scrollbarBoxImg->GetWidth(), scrollbarBoxImg->GetHeight());
	scrollbarBoxBtn->SetParent(this);
	scrollbarBoxBtn->SetImage(scrollbarBoxImg);
	scrollbarBoxBtn->SetImageOver(scrollbarBoxOverImg);
	scrollbarBoxBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarBoxBtn->SetMinY(0);
	scrollbarBoxBtn->SetMaxY(130);
	scrollbarBoxBtn->SetSelectable(false);
	scrollbarBoxBtn->SetClickable(false);
	scrollbarBoxBtn->SetHoldable(true);
	scrollbarBoxBtn->SetTrigger(trigHeldA);

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		nodeListText[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		nodeListText[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		nodeListText[i]->SetPosition(5,0);
		nodeListText[i]->SetMaxWidth(512);

		nodeListBg[i] = new GuiImage(bgTreeSelectionEntry);
		nodeListFolder[i] = new GuiImage(treeFolder);

		nodeList[i] = new GuiButton(512,30);
		nodeList[i]->SetParent(this);
		nodeList[i]->SetLabel(nodeListText[i]);
		nodeList[i]->SetImageOver(nodeListBg[i]);
		nodeList[i]->SetPosition(2,30*i+3);
		nodeList[i]->SetTrigger(trigA);
		nodeList[i]->SetSoundClick(btnSoundClick);
	}
}

/**
 * Destructor for the GuiTreeBrowser class.
 */
GuiTreeBrowser::~GuiTreeBrowser()
{
	delete arrowUpBtn;
	delete arrowDownBtn;
	delete scrollbarBoxBtn;

	delete bgTreeSelectionImg;
	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;
	delete scrollbarBoxImg;
	delete scrollbarBoxOverImg;

	delete bgTreeSelection;
	delete bgTreeSelectionEntry;
	delete treeFolder;
	delete scrollbar;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;
	delete scrollbarBox;
	delete scrollbarBoxOver;

	delete btnSoundOver;
	delete btnSoundClick;
	delete trigHeldA;
	delete trigA;

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		delete nodeListText[i];
		delete nodeList[i];
		delete nodeListBg[i];
		delete nodeListFolder[i];
	}
}

void GuiTreeBrowser::SetFocus(int f)
{
	focus = f;

	for(int i=0; i<FILE_PAGESIZE; i++)
		nodeList[i]->ResetState();

	if(f == 1)
		nodeList[selectedItem]->SetState(STATE_SELECTED);
}

void GuiTreeBrowser::ResetState()
{
	state = STATE_DEFAULT;
	stateChan = -1;
	selectedItem = 0;

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		nodeList[i]->ResetState();
	}
}

void GuiTreeBrowser::TriggerUpdate()
{
	listChanged = true;
}

/**
 * Draw the button on screen
 */
void GuiTreeBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	bgTreeSelectionImg->Draw();

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		nodeList[i]->Draw();
	}

	scrollbarImg->Draw();
	arrowUpBtn->Draw();
	arrowDownBtn->Draw();
	scrollbarBoxBtn->Draw();

	this->UpdateEffects();
}

void GuiTreeBrowser::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t)
		return;

	int position = 0;
	int positionWiimote = 0;

	arrowUpBtn->Update(t);
	arrowDownBtn->Update(t);
	scrollbarBoxBtn->Update(t);

        char scrollbarIsHeld = scrollbarBoxBtn->GetState() == STATE_HELD;
        char scrollbarRequired = treeBrowser.currentNode->numChildren > FILE_PAGESIZE;
        char tIsScrollTrigger = scrollbarBoxBtn->GetStateChan() == t->chan;
        char tIsValid = t->wpad->ir.valid;
	// move the file listing to respond to wiimote cursor movement
	if(scrollbarIsHeld && scrollbarRequired && tIsScrollTrigger && tIsValid)
	{
		scrollbarBoxBtn->SetPosition(0,0);
		positionWiimote = t->wpad->ir.y - 60 - scrollbarBoxBtn->GetTop();

		if(positionWiimote < scrollbarBoxBtn->GetMinY())
                {
			positionWiimote = scrollbarBoxBtn->GetMinY();
                }
                else if(positionWiimote > scrollbarBoxBtn->GetMaxY())
                {
			positionWiimote = scrollbarBoxBtn->GetMaxY();
                }

		treeBrowser.pageIndex = (positionWiimote * treeBrowser.currentNode->numChildren)/130.0 - selectedItem;

		if(treeBrowser.pageIndex <= 0)
		{
			treeBrowser.pageIndex = 0;
		}
		else if(treeBrowser.pageIndex+FILE_PAGESIZE >= treeBrowser.currentNode->numChildren)
		{
			treeBrowser.pageIndex = treeBrowser.currentNode->numChildren - FILE_PAGESIZE;
		}
		listChanged = true;
		focus = false;
	}

	if(arrowDownBtn->GetState() == STATE_HELD && arrowDownBtn->GetStateChan() == t->chan)
	{
		t->wpad->btns_h |= WPAD_BUTTON_DOWN;
		if(!this->IsFocused())
			((GuiWindow *)this->GetParent())->ChangeFocus(this);
	}
	else if(arrowUpBtn->GetState() == STATE_HELD && arrowUpBtn->GetStateChan() == t->chan)
	{
		t->wpad->btns_h |= WPAD_BUTTON_UP;
		if(!this->IsFocused())
			((GuiWindow *)this->GetParent())->ChangeFocus(this);
	}

	// pad/joystick navigation
	if(!focus)
	{
		goto endNavigation; // skip navigation
		listChanged = false;
	}

	if(t->Right())
	{
		if(treeBrowser.pageIndex < treeBrowser.currentNode->numChildren && treeBrowser.currentNode->numChildren > FILE_PAGESIZE)
		{
			treeBrowser.pageIndex += FILE_PAGESIZE;
			if(treeBrowser.pageIndex+FILE_PAGESIZE >= treeBrowser.currentNode->numChildren)
				treeBrowser.pageIndex = treeBrowser.currentNode->numChildren-FILE_PAGESIZE;
			listChanged = true;
		}
	}
	else if(t->Left())
	{
		if(treeBrowser.pageIndex > 0)
		{
			treeBrowser.pageIndex -= FILE_PAGESIZE;
			if(treeBrowser.pageIndex < 0)
				treeBrowser.pageIndex = 0;
			listChanged = true;
		}
	}
	else if(t->Down())
	{
		if(treeBrowser.pageIndex + selectedItem + 1 < treeBrowser.currentNode->numChildren)
		{
			if(selectedItem == FILE_PAGESIZE-1)
			{
				// move list down by 1
				treeBrowser.pageIndex++;
				listChanged = true;
			}
			else if(nodeList[selectedItem+1]->IsVisible())
			{
				nodeList[selectedItem]->ResetState();
				nodeList[++selectedItem]->SetState(STATE_SELECTED, t->chan);
			}
		}
	}
	else if(t->Up())
	{
		if(selectedItem == 0 &&	treeBrowser.pageIndex + selectedItem > 0)
		{
			// move list up by 1
			treeBrowser.pageIndex--;
			listChanged = true;
		}
		else if(selectedItem > 0)
		{
			nodeList[selectedItem]->ResetState();
			nodeList[--selectedItem]->SetState(STATE_SELECTED, t->chan);
		}
	}

	endNavigation:

	for(int i=0; i<FILE_PAGESIZE; i++)
	{
		if(listChanged || numEntries != treeBrowser.currentNode->numChildren)
		{
			if(treeBrowser.pageIndex+i < treeBrowser.currentNode->numChildren)
			{
				if(nodeList[i]->GetState() == STATE_DISABLED)
					nodeList[i]->SetState(STATE_DEFAULT);

				nodeList[i]->SetVisible(true);

				nodeListText[i]->SetText(treeBrowser.currentNode->children[treeBrowser.pageIndex+i].displayname);

                                nodeList[i]->SetIcon(NULL);
                                nodeListText[i]->SetPosition(10,0);
			}
			else
			{
				nodeList[i]->SetVisible(false);
				nodeList[i]->SetState(STATE_DISABLED);
			}
		}

		if(i != selectedItem && nodeList[i]->GetState() == STATE_SELECTED)
			nodeList[i]->ResetState();
		else if(focus && i == selectedItem && nodeList[i]->GetState() == STATE_DEFAULT)
			nodeList[selectedItem]->SetState(STATE_SELECTED, t->chan);

		int currChan = t->chan;

		if(t->wpad->ir.valid && !nodeList[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
			t->chan = -1;

		nodeList[i]->Update(t);
		t->chan = currChan;

		if(nodeList[i]->GetState() == STATE_SELECTED)
		{
			selectedItem = i;
			treeBrowser.selIndex = treeBrowser.pageIndex + i;
		}

		if(selectedItem == i)
			nodeListText[i]->SetScroll(SCROLL_HORIZONTAL);
		else
			nodeListText[i]->SetScroll(SCROLL_NONE);
	}

	// update the location of the scroll box based on the position in the file list
	if(positionWiimote > 0)
	{
		position = positionWiimote; // follow wiimote cursor
	}
	else
	{
		position = 130*(treeBrowser.pageIndex + FILE_PAGESIZE/2.0) / (treeBrowser.currentNode->numChildren*1.0);

		if(treeBrowser.pageIndex/(FILE_PAGESIZE/2.0) < 1)
			position = 0;
		else if((treeBrowser.pageIndex+FILE_PAGESIZE)/(FILE_PAGESIZE*1.0) >= (treeBrowser.currentNode->numChildren)/(FILE_PAGESIZE*1.0))
			position = 130;
	}

	scrollbarBoxBtn->SetPosition(0,position+36);

	listChanged = false;
	numEntries = treeBrowser.currentNode->numChildren;

	if(updateCB)
		updateCB(this);
}
