/**
  ******************************************************************************
  * @file    ${file_name} 
  * @author  ${user}
  * @version 
  * @date    ${date}
  * @brief   
  ******************************************************************************
  * @attention
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
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "fileBrowser.h"
#include "stm32f4xx.h"
#include "ImageDecoder.h"
#include "AudioDecoders.h"
#include "FSIO.h"
#include "main.h"

/**
  * @brief   Resources
  */
#include "InternalResourceXC32.h"
#include "audio_icon.h"
#include "DroidSansMono.h"

/* Private types ------------------------------------------------------------*/
/**
  * @brief   Structure for thumb drive information
  */
typedef struct _VOLUME_INFO
{
    char    label[12];
    BYTE    valid;
} VOLUME_INFO;

/**
  * @brief   Structure for folder/directory information
  */
typedef struct _FolderElement
{
    char            Name[13];
    XCHAR           XName[13];
    BYTE            blFolder : 1;
    FILE_FORMAT     FmtType;
} FolderElement;

/**
  * @brief   File browser errors
  */
typedef enum _FB_ERROR
{
	NO_ERROR = 0,
	READ_ERROR = 100,
	MEMORY_ERROR = 200,
	UNSUPPORTED_FORMAT = 255
} FB_ERROR;

/* Private constants --------------------------------------------------------*/
/**
  * @brief   Strings
  */
const XCHAR                 DetectingStr[] = {'D','e','t','e','c','t','i','n','g',0};
const XCHAR                 PleaseWaitStr[] = {'P','l','e','a','s','e',' ','W','a','i','t','.','.','.',0};
const XCHAR                 UpArrowStr[] = {'+',0};
const XCHAR                 DownArrowStr[] = {'-',0};
const XCHAR                 Exiting1Str[] = {'N','o',' ','D','r','i','v','e',' ','D','e','t','e','c','t','e','d',0};
const XCHAR                 UnsupportedStr[] = {'U','n','s','u','p','p','o','r','t','e','d',0};
const XCHAR                 FormatStr[] = {'F','o','r','m','a','t',0};
const XCHAR 				StopStr[] = {'S','t','o','p',0};
const XCHAR 				ResumeStr[] = {'R','e','s','u','m','e',0};
const XCHAR 				PauseStr[] = {'P','a','u','s','e',0};
const XCHAR 				ReadStr[] = {'R','e','a','d',0};
const XCHAR 				ErrorStr[] = {'E','r','r','o','r',0};
const XCHAR 				MemoryAllocationStr[] = {'M','e','m','o','r','y',' ','a','l','l','o','c','a','t','i','o','n',0};

/* Private macro ------------------------------------------------------------*/
#define MAX_ELEMENTS            100
#define SLIDERSCROLLDELAY   	50
#define AUTOPLAYDELAY   		1500

#define WAIT_UNTIL_FINISH(x)    while(!x)

/**
  * @brief  Object IDs 
  */
#define ID_JPGLISTBOX   0xFC10                              // List Box ID
#define ID_BTNUP4LB     0xFC11                              // Up Button ID
#define ID_BTNDN4LB     0xFC12                              // Down Button ID
#define ID_SLD4LB       0xFC13                              // Slider ID
#define ID_SLDVOL       0xFC14                              // SliderVol ID

#define ID_BUTTON_A		101
#define ID_BUTTON_B		102
#define ID_BUTTON_C		103
#define ID_BUTTON_D		104

/**
  * @brief  Object dimensions 
  */
#define SCROLLBTNWIDTH  25
#define SCROLLBTNHEIGHT 25

#define LBJPGXPOS       (0)
#define LBJPGYPOS       (0)
#define LBJPGWIDTH      (GetMaxX() - SCROLLBTNWIDTH - 1)    // width		
#define LBJPGHEIGHT     (GetMaxY() - 36)                    // height (36 is taken from the dimension of the navigation control buttons)
#define BTNUP4LBXPOS    (LBJPGXPOS + LBJPGWIDTH + 1)
#define BTNUP4LBYPOS    (LBJPGYPOS)
#define BTNUP4LBWIDTH   (SCROLLBTNWIDTH)
#define BTNUP4LBHEIGHT  (SCROLLBTNHEIGHT)
#define SLD4LBXPOS      (BTNUP4LBXPOS)
#define SLD4LBYPOS      (BTNUP4LBYPOS + SCROLLBTNHEIGHT + 1)
#define SLD4LBWIDTH     (SCROLLBTNWIDTH)
#define SLD4LBHEIGHT    (LBJPGHEIGHT - ((SCROLLBTNHEIGHT + 1) << 1))
#define BTNDN4LBXPOS    (BTNUP4LBXPOS)
#define BTNDN4LBYPOS	(LBJPGHEIGHT-SCROLLBTNHEIGHT)    
#define BTNDN4LBWIDTH   (SCROLLBTNWIDTH)
#define BTNDN4LBHEIGHT  (SCROLLBTNHEIGHT)

#define CTRLBTN_XINDENT         0
#define CTRLBTN_HEIGHT          35
#define CTRLBTN_WIDTH           (((GetMaxX() + 1) - (CTRLBTN_XINDENT * 2)) / 4)
#define CtrlBtnTop()            (GetMaxY() - CTRLBTN_HEIGHT)
#define CtrlBtnBottom()         GetMaxY()
#define CtrlBtnLeft(column)     (((column + 1) * CTRLBTN_XINDENT) + (column * CTRLBTN_WIDTH))
#define CtrlBtnRight(column)    ((column + 1) * (CTRLBTN_XINDENT + CTRLBTN_WIDTH))

#define ID_SLDVOL_X0     CtrlBtnLeft(1)
#define ID_SLDVOL_Y0     CtrlBtnTop()
#define ID_SLDVOL_X      CtrlBtnRight(2)
#define ID_SLDVOL_Y      CtrlBtnBottom()

/* Private variables --------------------------------------------------------*/
FolderElement               aFolderElement[MAX_ELEMENTS];
BYTE                        bStartingElement;
WORD                        bNumElementsInFolder;
BYTE                        blImageOnScreen;
extern __IO DWORD           tick;
BYTE        				bAutoPlay;
BYTE						NextFile;
BYTE        				bFileProcess;

/**
  * @brief   Audio player variables
  */
extern BYTE                 blAudioPlaying;

/**
  * @brief   File system variables
  */
BYTE                        mediaPresent = FALSE;
VOLUME_INFO                 volume;

/**
  * @brief   Pointers to screen objects
  */
BUTTON                      *pBtnUp, *pBtnDn;
BUTTON 						*pBtnA, *pBtnB, *pBtnC, *pBtnD;
LISTBOX                     *pListBox;
SLIDER                      *pSlider;
SLIDER 						*pSliderVol;
XCHAR                       *pFBItemList = NULL;
  
/* Private function prototypes ----------------------------------------------*/
void                        FillNewElements(void);
void                        DisplayErrorInfo(FB_ERROR nError);
WORD 						CreateCtrlButtons(XCHAR *pTextA, XCHAR *pTextB, XCHAR *pTextC, XCHAR *pTextD);
FILE_FORMAT 				getFileExt(SearchRec *rec);
BOOL 						GOLDeleteObj(OBJ_HEADER *object);
BYTE						AutoPlay(void);

/**
  * @brief  
  * @param  
  * @retval 
  */
WORD    Create_fileBrowser(void) {
    BYTE    TextHeight;
    WORD    TextX;

    // Free memory for the objects in the previous linked list and start new list to display
    // the files seen on the media
    GOLFree();

    // initialize the image decoder
    ImageDecoderInit();

    // initialize the screen	
    SetColor(WHITE);
    ClearDevice();

    SetFont((void *) &DroidSansMono_16);
    TextHeight = GetTextHeight((void *) &DroidSansMono_16);

    SetColor(BLACK);
    TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)DetectingStr, (void *) &DroidSansMono_16)) / 2;
    WAIT_UNTIL_FINISH(OutTextXY(TextX, 3 * TextHeight, (XCHAR *)DetectingStr));
    TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)PleaseWaitStr, (void *) &DroidSansMono_16)) / 2;
    WAIT_UNTIL_FINISH(OutTextXY(TextX, 6 * TextHeight, (XCHAR *)PleaseWaitStr));

#if defined(ENABLE_SD_MSD)
    MDD_SDSPI_InitIO();
#endif
    MonitorDriveMedia();

    if(mediaPresent == 0)
    {
        // erase the last line
        SetColor(WHITE);
        TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)PleaseWaitStr, (void *) &DroidSansMono_16)) / 2;
        WAIT_UNTIL_FINISH(OutTextXY(TextX, 6 * TextHeight, (XCHAR *)PleaseWaitStr));

        // replace it with these
        SetColor(BRIGHTRED);
        TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)Exiting1Str, (void *) &DroidSansMono_16)) / 2;
        WAIT_UNTIL_FINISH(OutTextXY(TextX, 6 * TextHeight, (XCHAR *)Exiting1Str));

        DelayMs(1000);
        return (0);
    }

    blImageOnScreen = 0;
    blAudioPlaying = FALSE;
    bAutoPlay = FALSE;
    bFileProcess = FALSE;
    NextFile = 0xFF;

    // create the listbox, slider and buttons that will emulate a
    // list box with controls.
    pListBox = LbCreate
        (
            ID_JPGLISTBOX,
            LBJPGXPOS,
            LBJPGYPOS,
            LBJPGXPOS + LBJPGWIDTH,
            LBJPGYPOS + LBJPGHEIGHT,
            LB_DRAW | LB_SINGLE_SEL,
            pFBItemList,
            NULL
        );

    if(pListBox == NULL)
        return (0);

    pSlider = SldCreate
        (
            ID_SLD4LB,
            SLD4LBXPOS,
            SLD4LBYPOS,
            SLD4LBXPOS + SLD4LBWIDTH,
            SLD4LBYPOS + SLD4LBHEIGHT,
            SLD_DRAW | SLD_VERTICAL | SLD_SCROLLBAR,
            2,
            1,
            1,  // these are temporary fill items will set to proper values
            NULL
        );
    if(pSlider == NULL)
        return (0);

    pBtnUp = BtnCreate
        (
            ID_BTNUP4LB,
            BTNUP4LBXPOS,
            BTNUP4LBYPOS,
            BTNUP4LBXPOS + BTNUP4LBWIDTH,
            BTNUP4LBYPOS + BTNUP4LBHEIGHT,
            0,
            BTN_DRAW,
            NULL,
            (XCHAR *)UpArrowStr,
            NULL
        );

    if(pBtnUp == NULL)
        return (0);

    pBtnDn = BtnCreate
        (
            ID_BTNDN4LB,
            BTNDN4LBXPOS,
            BTNDN4LBYPOS,
            BTNDN4LBXPOS + BTNDN4LBWIDTH,
            BTNDN4LBYPOS + BTNDN4LBHEIGHT,
            0,
            BTN_DRAW,
            NULL,
            (XCHAR *)DownArrowStr,
            NULL
        );

    if(pBtnDn == NULL)
        return (0);

    pSliderVol = SldCreate
        (
			ID_SLDVOL,                                              // object’s ID
			ID_SLDVOL_X0, ID_SLDVOL_Y0, ID_SLDVOL_X, ID_SLDVOL_Y,   // object’s dimension
			SLD_DRAW,                                				// object state after creation
			70,                                                     // range
			1,                                                      // page
			volumeAudio,                                            // initial position
			NULL                                                    // use default style scheme
        );
    if(pSliderVol == NULL)
        return (0);

    // create the control buttons at the bottom of the screen
    if(!CreateCtrlButtons(NULL, NULL, NULL, NULL))
		return (0);
	
    pBtnA = (BUTTON *)GOLFindObject(ID_BUTTON_A);
    pBtnB = (BUTTON *)GOLFindObject(ID_BUTTON_B);
    pBtnC = (BUTTON *)GOLFindObject(ID_BUTTON_C);
    pBtnD = (BUTTON *)GOLFindObject(ID_BUTTON_D);

    GOLDeleteObj(pSliderVol);

    // fill the list box with the file names of images in the media
    FillNewElements();

    // set the first item to be focused
    LbSetFocusedItem(pListBox, 0);

    // successful creation of the JPEG demo screen
    return (1);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
WORD    fileBrowser_MsgCallback(WORD objMsg, OBJ_HEADER *pObj, GOL_MSG *pMsg) {
    LISTITEM    *pItemSel;
    FB_ERROR 	bError;

    // check if an image is being shown
    if(blImageOnScreen == 1)
    {
        // this is the routine to go back and show the list when an
        // image is being shown on the screen or the slide show is
        // currently ongoing
        if(pMsg->uiEvent == EVENT_RELEASE)
        {
            blImageOnScreen = 0;
            GOLRedrawRec(0, 0, GetMaxX(), GetMaxY());
        }

        return (0);
    }

    switch(GetObjID(pObj))
    {
        case ID_BUTTON_A:
            if(objMsg == BTN_MSG_RELEASED)
            {   
				// check if button is pressed
                // do not process if user moved the touch away from the button
                // returning 1 will update the button
                if(pMsg->uiEvent == EVENT_MOVE)
                    return (1);

                // check if an audio file is now playing
                if(blAudioPlaying == TRUE)
                {
                   /* Disable the continuous playback */
                   bAutoPlay = FALSE;

                   AudioPlayer_CallBack();

                   GOLDeleteObj(pSliderVol);
                   GOLAddObject(pBtnB); GOLAddObject(pBtnC);

                   /* Clear audio buttons */
                   BtnSetText(pBtnA, NULL); BtnSetText(pBtnD, NULL);
                   SetState(pBtnA, BTN_DISABLED); SetState(pBtnD, BTN_DISABLED);

                   /* Enable objs */
                   ClrState(pListBox, LB_DISABLED); ClrState(pSlider, SLD_DISABLED);
                   ClrState(pBtnUp, BTN_DISABLED); ClrState(pBtnDn, BTN_DISABLED);

                   GOLRedrawRec(0, 0, GetMaxX(), GetMaxY());

                   return (1);
                }

            }

            return (1);

        case ID_BUTTON_B:
           if(objMsg == BTN_MSG_STILLPRESSED)
           {   // check if button is pressed
               // do not process if user moved the touch away from the button
               // returning 1 wil update the button
               if(pMsg->uiEvent == EVENT_MOVE)
                   return (1);

           }

           return (1);

        case ID_BUTTON_C:
           if(objMsg == BTN_MSG_STILLPRESSED)
           {   // check if button is pressed
               // do not process if user moved the touch away from the button
               // returning 1 wil update the button
               if(pMsg->uiEvent == EVENT_MOVE)
                   return (1);

           }

           return (1);

        case ID_BUTTON_D:
            if(objMsg == BTN_MSG_RELEASED)
            {   // check if button is pressed
                // do not process if user moved the touch away from the button
                // returning 1 wil update the button
                if(pMsg->uiEvent == EVENT_MOVE)
                    return (1);

                // check if an audio file is now playing
                if(blAudioPlaying == TRUE)
                {
                	if(!PauseFlag) {
                		PauseResumeStatus = 0;
                		BtnSetText(pBtnD, ResumeStr);
                		SetState(pBtnD, BTN_DRAW);
                	} else {
                		PauseResumeStatus = 1;
                		BtnSetText(pBtnD, PauseStr);
                		SetState(pBtnD, BTN_DRAW);
                	}
                }
            }

            return (1);

        case ID_JPGLISTBOX:
            if(pMsg->uiEvent == EVENT_MOVE)
            {
                pMsg->uiEvent = EVENT_PRESS;            // change event for listbox

                // Process message by default
                LbMsgDefault(objMsg, (LISTBOX *)pObj, pMsg);

                // Set new list box position
                SldSetPos(pSlider, LbGetCount(pListBox) - LbGetFocusedItem(pListBox) - 1);
                SetState(pSlider, SLD_DRAW_THUMB);
                pMsg->uiEvent = EVENT_MOVE;             // restore event for listbox
            }
            else if(pMsg->uiEvent == EVENT_PRESS)
            {
                // call the message default processing of the list box to select the item
                LbMsgDefault(objMsg, (LISTBOX *)pObj, pMsg);
            }
            else if(pMsg->uiEvent == EVENT_RELEASE)
            {
                // check which item was selected and display appropriate screen
                pItemSel = LbGetSel(pListBox, NULL);    // get the selected item

                /* Sets the starting file for AutoPlay function */
				NextFile = pItemSel->data;

                if(aFolderElement[pItemSel->data].blFolder == 1)
                {
                    if(FSchdir(aFolderElement[pItemSel->data].Name) == 0)
                    {
                        FillNewElements();
                        return (1);
                    }
                }
                else if(aFolderElement[pItemSel->data].FmtType == BMP  || \
						aFolderElement[pItemSel->data].FmtType == JPEG || \
						aFolderElement[pItemSel->data].FmtType == GIF)
                {
                    FSFILE *pFile = FSfopen(aFolderElement[pItemSel->data].Name, "rb");
                    if(pFile != NULL)
                    {
                        blImageOnScreen = 1;
						
                        // initialize the screen	
					    SetColor(BLACK);
    					ClearDevice();

    					bError = ImageDecode
    							(
    								pFile,
    								aFolderElement[pItemSel->data].FmtType,
    								0,
    								0,
    								IMG_SCREEN_WIDTH,
    								IMG_SCREEN_HEIGHT,
    								(IMG_DOWN_SCALE | IMG_ALIGN_CENTER),
    								NULL,
    								NULL
    							);

    					if(bError)
    						DisplayErrorInfo(bError);

                        FSfclose(pFile);
                    }
                }
                else if(aFolderElement[pItemSel->data].FmtType == WAV || \
						aFolderElement[pItemSel->data].FmtType == MP3)
                {
                	/* Enable the continuous playback */
                	bAutoPlay = TRUE;

                	GOLDeleteObj(pBtnB); GOLDeleteObj(pBtnC);
                	GOLAddObject(pSliderVol);

                	/* Set audio buttons */
                	BtnSetText(pBtnA, StopStr); BtnSetText(pBtnD, PauseStr);
                	ClrState(pBtnA, BTN_DISABLED); ClrState(pBtnD, BTN_DISABLED);

                	/* Disable needless objs */
                	SetState(pListBox, LB_DISABLED); SetState(pSlider, SLD_DISABLED);
                	SetState(pBtnUp, BTN_DISABLED); SetState(pBtnDn, BTN_DISABLED);

                	GOLRedrawRec(0, 0, GetMaxX(), GetMaxY());
                }
				else if(aFolderElement[pItemSel->data].FmtType == OTHER)
				{
					DisplayErrorInfo(UNSUPPORTED_FORMAT);
					GOLRedrawRec(0, 0, GetMaxX(), GetMaxY());
				}
            }

            // The message was processed. To avoid other objects processing the 
            // processed message reset the message.
            pMsg->uiEvent = EVENT_INVALID;
            return (0);

        case ID_SLD4LB:
        	
        	if((objMsg == SLD_MSG_INC) || (objMsg == SLD_MSG_DEC)) 
            {   // check slider was touched.

	            // Process message by default
	            SldMsgDefault(objMsg, (SLIDER *)pObj, pMsg);
	
	            // Set new list box position
	            if(LbGetFocusedItem(pListBox) != LbGetCount(pListBox) - SldGetPos(pSlider))
	            {
	                LbSetFocusedItem(pListBox, LbGetCount(pListBox) - SldGetPos(pSlider));
	                SetState(pListBox, LB_DRAW_ITEMS);
	            }
	            
	        }

            // The message was processed
            return (0);

        case ID_BTNUP4LB:                               // slider up button was pressed
            if(objMsg == BTN_MSG_RELEASED)
            {
				// check if we have reached the very first then stay there.
	            if (LbGetFocusedItem(pListBox) == 0)
	                LbSetFocusedItem(pListBox,0);
	            else    
                	LbSetFocusedItem(pListBox,LbGetFocusedItem(pListBox)-1);                
                SetState(pListBox, LB_DRAW_ITEMS);
                SldSetPos(pSlider, SldGetPos(pSlider) + 1);
                SetState(pSlider, SLD_DRAW_THUMB);
            }

            return (1);

        case ID_BTNDN4LB:                               // slider down button was pressed
            if(objMsg == BTN_MSG_RELEASED)
            {
	            // set all items to be not displayed
                pItemSel = pListBox->pItemList;
                while(pItemSel != NULL) {
                	pItemSel->status = 0;
                	pItemSel = pItemSel->pNextItem;
                }	
                LbSetFocusedItem(pListBox, LbGetFocusedItem(pListBox) + 1);
                SetState(pListBox, LB_DRAW_ITEMS);
                SldSetPos(pSlider, SldGetPos(pSlider) - 1);
                SetState(pSlider, SLD_DRAW_THUMB);
            }

            return (1);

        case ID_SLDVOL:

        	if((objMsg == SLD_MSG_INC) || (objMsg == SLD_MSG_DEC))
                {   // check slider was touched.

	            	// Process message by default
	            	SldMsgDefault(objMsg, (SLIDER *)pObj, pMsg);

	            	// check if an audio file is now playing
                    if(blAudioPlaying == TRUE)
                    {
                    	volumeAudio = SldGetPos(pSliderVol);
                    	volTask = SET;
                    }
                }

            // The message was processed
            return (0);
    }

    return (1);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
WORD    fileBrowser_DrawCallback(void) {
    static DWORD    prevTick = 0;   // keeps previous value of tick	
    SHORT           item;

	MonitorDriveMedia();

    // check if media is still present, if not return to main menu
    if(mediaPresent == FALSE)
    {
    	// Free memory used by graphic objects
    	GOLFree();

        SetColor(WHITE);
        ClearDevice();
    	screenState = CREATE_FILEBROWSER;
        return (1);
    }

    // this implements the automatic playback of the multimedia list box contents
    if(bAutoPlay && !bFileProcess)
    {
    	if((tick - prevTick) > AUTOPLAYDELAY)
		{
    		AutoPlay();
			prevTick = tick;
		}
    }
    else
    {
    	// this implements the automatic scrolling of the list box contents
    	// when the up or down buttons are continuously pressed.
    	if((tick - prevTick) > SLIDERSCROLLDELAY)
    	{
    		item = LbGetFocusedItem(pListBox);
    		if(GetState(pBtnUp, BTN_PRESSED))
    		{

    			// redraw only the listbox when the item that is focused has changed
    			LbSetFocusedItem(pListBox, LbGetFocusedItem(pListBox) - 1);
    			if(item != LbGetFocusedItem(pListBox))
    			{
    				SetState(pListBox, LB_DRAW_ITEMS);
    			}

    			SldSetPos(pSlider, SldGetPos(pSlider) + 1);
    			SetState(pSlider, SLD_DRAW_THUMB);
    		}

    		if(GetState(pBtnDn, BTN_PRESSED))
    		{
    			LbSetFocusedItem(pListBox, LbGetFocusedItem(pListBox) + 1);
    			if(item != LbGetFocusedItem(pListBox))
    			{
    				SetState(pListBox, LB_DRAW_ITEMS);
    			}

    			SldSetPos(pSlider, SldGetPos(pSlider) - 1);
    			SetState(pSlider, SLD_DRAW_THUMB);
    		}

    		prevTick = tick;
    	}
    }

	return (1);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void FillNewElements(void) {
    SearchRec   searchRecord;
    WORD        bCounter;
    BYTE        bFound;
    void        *pBitmap;

    bNumElementsInFolder = 0;

    bFound = FindFirst("*", ATTR_DIRECTORY, &searchRecord);
    for(bCounter = 0; bCounter < MAX_ELEMENTS; bCounter++)
    {
        if(bFound == 0)
        {
            BYTE    i;
            if(searchRecord.filename[0] == '.' && searchRecord.filename[1] == '\0')
            {
                bFound = FindNext(&searchRecord);
                continue;
            }

            aFolderElement[bNumElementsInFolder].blFolder = 1;
            for(i = 0; i < 13; i++)
            {
                aFolderElement[bNumElementsInFolder].Name[i] = searchRecord.filename[i];
                aFolderElement[bNumElementsInFolder].XName[i] = searchRecord.filename[i];
            }

            bNumElementsInFolder++;
        }
        else
        {
            break;
        }

        bFound = FindNext(&searchRecord);
    }

    bFound = FindFirst("*.*", ATTR_MASK ^ ATTR_DIRECTORY, &searchRecord);
    for(bCounter = 0; bCounter < MAX_ELEMENTS && bNumElementsInFolder < MAX_ELEMENTS; bCounter++)
    {
        if(bFound == 0)
        {
            BYTE    i;
            aFolderElement[bNumElementsInFolder].blFolder = 0;
			aFolderElement[bNumElementsInFolder].FmtType = getFileExt(&searchRecord);
            for(i = 0; i < 13; i++)
            {
                aFolderElement[bNumElementsInFolder].Name[i] = searchRecord.filename[i];
                aFolderElement[bNumElementsInFolder].XName[i] = searchRecord.filename[i];
            }

            bNumElementsInFolder++;
        }
        else
        {
            break;
        }

        bFound = FindNext(&searchRecord);
    }

    // fill the list box items
    bStartingElement = 0;

    // clear the list first
    LbDelItemsList(pListBox);

    for(bCounter = 0; (bStartingElement + bCounter) < bNumElementsInFolder; bCounter++)
    {

        // set the proper bitmap icon
        if(aFolderElement[bStartingElement + bCounter].blFolder == 1)
        {
            pBitmap = (void *) &FolderIcon;
        }
        else if(aFolderElement[bStartingElement + bCounter].FmtType == JPEG)
        {
            pBitmap = (void *) &JpegIcon;
        }
        else if(aFolderElement[bStartingElement + bCounter].FmtType == BMP)
        {
            pBitmap = (void *) &BitmapIcon;
        }
        else if(aFolderElement[bStartingElement + bCounter].FmtType == MP3)
        {
            pBitmap = (void *) &Music;
        }
		else if(aFolderElement[bStartingElement + bCounter].FmtType == WAV)
        {
            pBitmap = (void *) &Music;
        }
        else
        {
            pBitmap = NULL;
        }

        // assign the item to the list
        if(NULL == LbAddItem(pListBox, NULL, aFolderElement[bStartingElement + bCounter].XName, pBitmap, 0, bCounter))
            break;
        else
        {

            // adjust the slider page and range
            SldSetRange(pSlider, bNumElementsInFolder);
            SldSetPage(pSlider, bNumElementsInFolder >> 1);

            // to completely redraw the object when GOLDraw() is executed.
            SetState(pSlider, DRAW);
            SetState(pListBox, DRAW);
        }
    }
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void DisplayErrorInfo(FB_ERROR nError) {
	WORD    TextX, TextY, TextHeight;

    	SetColor(WHITE);
    	ClearDevice();

    	SetColor(BRIGHTRED);
    	SetFont((void *) &DroidSansMono_16);

    	TextHeight = GetTextHeight((void *) &DroidSansMono_16);
    	TextY = (IMG_SCREEN_HEIGHT - 3 * TextHeight) / 2;

    	switch(nError) {
		case READ_ERROR:
    			TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)ReadStr, (void *) &DroidSansMono_16)) / 2;
    			WAIT_UNTIL_FINISH(OutTextXY(TextX, TextY, (XCHAR *)ReadStr));
    			TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)ErrorStr, (void *) &DroidSansMono_16)) / 2;
    			WAIT_UNTIL_FINISH(OutTextXY(TextX, TextY + 2 * TextHeight, (XCHAR *)ErrorStr));
			break;

		case MEMORY_ERROR:
    			TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)MemoryAllocationStr, (void *) &DroidSansMono_16)) / 2;
    			WAIT_UNTIL_FINISH(OutTextXY(TextX, TextY, (XCHAR *)MemoryAllocationStr));
    			TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)ErrorStr, (void *) &DroidSansMono_16)) / 2;
    			WAIT_UNTIL_FINISH(OutTextXY(TextX, TextY + 2 * TextHeight, (XCHAR *)ErrorStr));
			break;

		case UNSUPPORTED_FORMAT:
    			TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)UnsupportedStr, (void *) &DroidSansMono_16)) / 2;
    			WAIT_UNTIL_FINISH(OutTextXY(TextX, TextY, (XCHAR *)UnsupportedStr));
    			TextX = (IMG_SCREEN_WIDTH - GetTextWidth((XCHAR *)FormatStr, (void *) &DroidSansMono_16)) / 2;
    			WAIT_UNTIL_FINISH(OutTextXY(TextX, TextY + 2 * TextHeight, (XCHAR *)FormatStr));
			break;
    	}

    	DelayMs(800);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void    MonitorDriveMedia(void) {
    BYTE        mediaPresentNow;
    BYTE        mountTries;
    SearchRec   searchRecord;

#if defined ENABLE_USB_MSD	
    mediaPresentNow = USBHostMSDSCSIMediaDetect();
#elif defined ENABLE_SD_MSD
    mediaPresentNow = MDD_MediaDetect();
#endif

#if defined(ENABLE_USB_MSD)
    if( (mediaPresentNow != mediaPresent) && EnumDone )
#else
    if(mediaPresentNow != mediaPresent)
#endif
    {
        if(mediaPresentNow)
        {
            mountTries = 10;
            while(!FSInit() && mountTries)
            	mountTries--;
            if(!mountTries)
            {
                mediaPresent = FALSE;
            }
            else
            {
                mediaPresent = TRUE;

                // Find the volume label.  We need to do this here while we are at the
                // root directory.
                if(!FindFirst("*.*", ATTR_VOLUME, &searchRecord))
                {
                    strcpy(volume.label, searchRecord.filename);
                    volume.valid = TRUE;
                }
            }
        }
        else
        {
            mediaPresent = FALSE;
            volume.valid = FALSE;
        }
    }
#if defined(ENABLE_USB_MSD)
    else if(!EnumDone)
    {
        mediaPresent = FALSE;
        volume.valid = FALSE;
    }
#endif
}

/**
  * @brief  
  * @param  
  * @retval 
  */
WORD CreateCtrlButtons(XCHAR *pTextA, XCHAR *pTextB, XCHAR *pTextC, XCHAR *pTextD) {
	WORD    state;
    BUTTON  *pObj;


    state = BTN_DRAW;
    if(pTextA == NULL)
        state = BTN_DRAW | BTN_DISABLED;
    pObj = BtnCreate
    (
        ID_BUTTON_A,
        CtrlBtnLeft(0),
        CtrlBtnTop(),
        CtrlBtnRight(0),
        CtrlBtnBottom(),
        0,
        state,
        NULL,
        pTextA,
        altScheme
    );
    if (pObj == NULL)
        return (0);

    state = BTN_DRAW;
    if(pTextB == NULL)
        state = BTN_DRAW | BTN_DISABLED;
    pObj = BtnCreate
    (
        ID_BUTTON_B,
        CtrlBtnLeft(1),
        CtrlBtnTop(),
        CtrlBtnRight(1),
        CtrlBtnBottom(),
        0,
        state,
        NULL,
        pTextB,
        altScheme
    );
    if (pObj == NULL)
        return (0);

    state = BTN_DRAW;
    if(pTextC == NULL)
        state = BTN_DRAW | BTN_DISABLED;
    pObj = BtnCreate
    (
        ID_BUTTON_C,
        CtrlBtnLeft(2),
        CtrlBtnTop(),
        CtrlBtnRight(2),
        CtrlBtnBottom(),
        0,
        state,
        NULL,
        pTextC,
        altScheme
    );
    if (pObj == NULL)
        return (0);

    state = BTN_DRAW;
    if(pTextD == NULL)
        state = BTN_DRAW | BTN_DISABLED;
    pObj = BtnCreate
    (
        ID_BUTTON_D,
        CtrlBtnLeft(3),
        CtrlBtnTop(),
        CtrlBtnRight(3),
        CtrlBtnBottom(),
        0,
        state,
        NULL,
        pTextD,
        altScheme
    );
    if (pObj == NULL)
        return (0);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
FILE_FORMAT getFileExt(SearchRec *rec) {
	FILE_FORMAT Ext;

	if     (strstr(rec->filename, ".BMP"))
		Ext = BMP;
	else if(strstr(rec->filename, ".JPG"))
		Ext = JPEG;
	else if(strstr(rec->filename, ".GIF"))
		Ext = GIF;
	else if(strstr(rec->filename, ".WAV"))
		Ext = WAV;
	else if(strstr(rec->filename, ".MP3"))
		Ext = MP3;
	else if(strstr(rec->filename, ".RAW"))
		Ext = RGB;
	else
		Ext = OTHER;

	return(Ext);
}

/**
  * @brief
  * @param
  * @retval
  */
BOOL GOLDeleteObj(OBJ_HEADER *object)
{
    if(!_pGolObjects)
        return (FALSE);

    if(object == _pGolObjects)
    {
        _pGolObjects = (OBJ_HEADER *)object->pNxtObj;
    }
    else
    {
        OBJ_HEADER  *curr;
        OBJ_HEADER  *prev;

        curr = (OBJ_HEADER *)_pGolObjects->pNxtObj;
        prev = (OBJ_HEADER *)_pGolObjects;

        while(curr)
        {
            if(curr == object)
                break;

            prev = (OBJ_HEADER *)curr;
            curr = (OBJ_HEADER *)curr->pNxtObj;
        }

        if(!curr)
            return (FALSE);

        prev->pNxtObj = curr->pNxtObj;
    }

    return (TRUE);
}

/**
  * @brief
  * @param
  * @retval
  */
BYTE AutoPlay(void)
{
    FSFILE    	*pMediaFile;
    FB_ERROR 	bMediaError;

    bFileProcess = TRUE;

    if(aFolderElement[NextFile].FmtType == WAV || \
	   aFolderElement[NextFile].FmtType == MP3)
	{
		pMediaFile = FSfopen(aFolderElement[NextFile].Name, "rb");
		if(pMediaFile != NULL)
		{
			bMediaError = AudioPlayBack(pMediaFile, aFolderElement[NextFile].FmtType);

			if(bMediaError)
				DisplayErrorInfo(bMediaError);

			FSfclose(pMediaFile);
		}

		GOLRedrawRec(0, 0, GetMaxX(), GetMaxY());
	}

	NextFile++;
	if(NextFile >= bNumElementsInFolder)
		NextFile = 1;

    bFileProcess = FALSE;
    return (0);
}
