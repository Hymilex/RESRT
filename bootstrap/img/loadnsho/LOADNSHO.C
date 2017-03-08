/* LOADNSHO.C -- Minimal Windows program using the Victor Library for 
	32-bit Windows
*/
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <vicdefs.h>
#include <string.h>
#include "loadnsho.h"

/******************** Global Variables ***************************/
imgdes Image;           // Image descriptor
HWND hWndMain;          // Main window handle
HINSTANCE hInstance;       // Instance handle
HPALETTE HPalette;      // Logical palette handle
int xScreen, yScreen;   // Screen dimensions
HCURSOR hHourGlass, hSaveCursor; // Cursor handles
char *szAppName  = "LOADNSHO";
// File load variables
char szFileName[96]; // Holds fully qualified pathname
char szRealFile[16]; // Holds just the filename without path

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance,
   LPSTR lpszCmdLine, int nCmdShow)
{
   MSG msg;

   hInstance = hInst; // Set global instance handle
   // Determine screen dimensions
   xScreen = GetSystemMetrics(SM_CXSCREEN);
   yScreen = GetSystemMetrics(SM_CYSCREEN);
   if(hPrevInstance == FALSE) {
      if(InitApplication() == FALSE) {
         // Registering one of the windows failed
         error_handler(0, ERR_REG_CLASS);
         return(FALSE);
         }
      }
   // Create this instance's window
   if(InitInstance(lpszCmdLine, nCmdShow) == FALSE) {
      // Could not create main window
      error_handler(0, ERR_CRT_MANWIN);
      return(FALSE);
      }
   while(GetMessage(&msg, NULL, 0, 0)) { // Until the WM_QUIT message arrives
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      }
   return(msg.wParam);
}

// Our main window proc
LRESULT _export WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int rcode = NO_ERROR;

   switch(msg) {
      case WM_COMMAND:         // Menu item selected!
         return(MenuCommand(hWnd, wParam));

      case WM_PAINT:         // Repaint a portion of the window!
			{
		   PAINTSTRUCT ps;   // Holds PAINT info
		   HDC hdc;
			int xpos, ypos;

         hdc = BeginPaint(hWnd, &ps);
         // Display the hourglass cursor
         hSaveCursor = SetCursor(hHourGlass);
         if(HPalette)   // Delete palette object allocated by viewimage()
            DeleteObject(HPalette);
		   // Get the position of the upper left corner in the image buffer
   		xpos = GetScrollPos(hWnd, SB_HORZ);
   		ypos = GetScrollPos(hWnd, SB_VERT);
      	rcode = viewimageex(hWnd, hdc, &HPalette, xpos, ypos,
		      &Image, 0, 0, VIEWDITHER);
         SetCursor(hSaveCursor);  // Restore the cursor
         EndPaint(hWnd, &ps);
         // Handle any errors
		   if(rcode != NO_ERROR)
	         error_handler(hWnd, rcode);
         break;
			}

      case WM_VSCROLL:      /* Vert scroll bar clicked on! */
			{
		   RECT rect;
   		int bound, iMax, iMin, iPos, dn; // Scrolling variables
			UINT code = wParam;
			int pos = LOWORD(lParam);

         /* Calculate new vertical scroll position */
         GetScrollRange(hWnd, SB_VERT, &iMin, &iMax);
         iPos = GetScrollPos(hWnd, SB_VERT);
         GetClientRect(hWnd, &rect);
         switch(code) {
            case SB_LINEDOWN:      dn =  rect.bottom / 16 + 1;
               break;
            case SB_LINEUP:        dn = -rect.bottom / 16 + 1;
               break;
            case SB_PAGEDOWN:      dn =  rect.bottom / 2  + 1;
               break;
            case SB_PAGEUP:        dn = -rect.bottom / 2  + 1;
               break;
            case SB_THUMBPOSITION: dn = pos - iPos;
               break;
            default:               dn = 0;
            }
         // Limit scrolling to current scroll range
         bound = (iPos + dn < iMin) ? iMin :
                 (iPos + dn > iMax) ? iMax : iPos + dn;
         if((dn = bound - iPos) != 0) {
            ScrollWindow(hWnd, 0, -dn, 0, 0);
            SetScrollPos(hWnd, SB_VERT, iPos + dn, TRUE);
            }
         break;
			}

      case WM_HSCROLL:      // Horiz scroll bar clicked on!
			{
		   RECT rect;
		   int bound, iMax, iMin, iPos, dn; // Scrolling variables
			UINT code = wParam;
			int pos = LOWORD(lParam);

         // Calculate new horizontal scroll position
         GetScrollRange(hWnd, SB_HORZ, &iMin, &iMax);
         iPos = GetScrollPos(hWnd, SB_HORZ);
         GetClientRect(hWnd, &rect);
         switch(code) {
            case SB_LINEDOWN:      dn =  rect.right / 16 + 1;
               break;
            case SB_LINEUP:        dn = -rect.right / 16 + 1;
               break;
            case SB_PAGEDOWN:      dn =  rect.right / 2  + 1;
               break;
            case SB_PAGEUP:        dn = -rect.right / 2  + 1;
               break;
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION: dn = pos - iPos;
               break;
            default:                 dn = 0;
            }
         // Limit scrolling to current scroll range
         bound = (iPos + dn < iMin) ? iMin :
                 (iPos + dn > iMax) ? iMax : iPos + dn;
         if((dn = bound - iPos) != 0) {
            ScrollWindow(hWnd, -dn, 0, 0, 0);
            SetScrollPos(hWnd, SB_HORZ, iPos + dn, TRUE);
            }
         break;
			}

      case WM_DESTROY:      // Window is being destroyed!
         // Perform any necessary cleanup functions before exiting the program
         if(HPalette)
            DeleteObject(HPalette);
         freeimage(&Image);   // Free the allocated image
         PostQuitMessage(0);  // Quit the application
         break;

      default:
         return(DefWindowProc(hWnd, msg, wParam, lParam));
      }
   return(0L);
}      // End of WndProc

// Command selected from a menu
int MenuCommand(HWND hWnd, WPARAM id)
{
   // Description string and file type for GetOpenFileName()
   static char Loadlist[] = "TIFF~*.TIF~";
   int nRc, rcode=NO_ERROR;
   char filterStr[128]; /* Construct the "TIFF\0*.TIF\0" string here */
   OPENFILENAME ofn;
   char realFname[128];

   switch(id) {
      case IDM_F_ABOUT: // "About" menu item selected!
   		DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, (DLGPROC)About);
         break;

      case IDM_F_EXIT:   // "Exit" menu item selected!
         DestroyWindow(hWnd);
         PostQuitMessage(0);  // Quit the application 
         break;

      case IDM_F_OPEN:  // "Open file" menu item selected!
         // Zero the OpenFileName struct
         memset(&ofn, 0, sizeof(OPENFILENAME));
         // Initialize the needed struct members
         ofn.lStructSize = sizeof(OPENFILENAME);
         ofn.hwndOwner = hWnd;

         // Construct a file filter string of the form "TIFF\0*.TIF\0";
         CreateFilterString(Loadlist, filterStr);
         ofn.lpstrFilter = filterStr;
         ofn.nFilterIndex = 1L;

         /* Fill in the initializing string for the filename edit control.
            On return, this buffer contains the fully qualified
            pathname of the file to open
         */
         *szFileName = 0;
         ofn.lpstrFile = szFileName;
         ofn.nMaxFile = sizeof(szFileName);
         ofn.lpstrFileTitle = realFname; // Receives filename.ext without path
         ofn.nMaxFileTitle = sizeof(realFname);

         ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
         // Set default file type
         ofn.lpstrDefExt = "TIF";

         nRc = GetOpenFileName((OPENFILENAME far *)&ofn);
         if(nRc != 0) { // Fully qualified filename is in szFileName
            if((rcode = load_tif(szFileName, &Image)) == NO_ERROR) {
               // Save the filename
               lstrcpy(szRealFile, realFname);
               // Update the main window caption
               SetMainCaption(hWnd, szAppName, szRealFile);
               /* Add scroll bars if needed */
               set_scroll_ranges(hWnd, &Image);
               // Invalidate rect to update the screen
               InvalidateRect(hWnd, 0, TRUE);
               }
            else  // Handle any errors
               error_handler(hWnd, rcode, (LPSTR)szFileName);
            }
         break;

      default:
         break;
      }
   return(TRUE);
}

// Set new vertical and horizontal scroll positions for the window
void set_scroll_ranges(HWND hWnd, imgdes *image)
{
   static int flag;
   RECT rc;
   int vert_range, horiz_range, j;

   if(flag == 0) { // Enter this function only if we're not already here
      flag++;
      for(j=0; j<2; j++) {
         GetClientRect(hWnd, &rc);
         horiz_range = (UINT)image->bmh->biWidth - rc.right;
         if(horiz_range < 0)
            horiz_range = 0;
         SetScrollRange(hWnd, SB_HORZ, 0, horiz_range, TRUE);

         vert_range = (UINT)image->bmh->biHeight - rc.bottom;
         if(vert_range < 0)
            vert_range = 0;
         SetScrollRange(hWnd, SB_VERT, 0, vert_range, TRUE);

         if(GetScrollPos(hWnd, SB_HORZ) > horiz_range ||
            GetScrollPos(hWnd, SB_VERT) > vert_range)
            InvalidateRect(hWnd, 0, TRUE);
         }
      flag--;
      }
}

/* Create the main window and the file handling window.
   Returns TRUE if successful, else FALSE.
*/
int InitInstance(LPCSTR lpCmdLine, int nCmdShow)
{
   hWndMain = CreateWindow(szAppName, // Window class name
      szAppName,               // Window's title
      WS_CAPTION      |         // Title and Min/Max
      WS_SYSMENU      |         // Add system menu box
      WS_MINIMIZEBOX  |         // Add minimize box
      WS_MAXIMIZEBOX  |         // Add maximize box
      WS_THICKFRAME   |         // Thick sizeable frame
      WS_CLIPCHILDREN |         // Don't draw in child windows areas
      WS_OVERLAPPED,
      0, 0, xScreen, yScreen,   // Use full screen
      0,                        // Parent window's handle
      0,                        // Default to Class Menu
      hInstance,               // Instance of window
      0);                     // Create struct for WM_CREATE
   if(hWndMain) {
      // Get an hourglass cursor to use during file transfers, etc.
      hHourGlass = LoadCursor(NULL, IDC_WAIT);
      if(InitLoadNSho(hWndMain, DEFIMGWIDTH, DEFIMGLENGTH, DEFIMGBPPIXEL) != NO_ERROR)
         PostMessage(hWndMain, WM_DESTROY, 0, 0);
      ShowWindow(hWndMain, nCmdShow); // Display main window
      UpdateWindow(hWndMain);
      return(TRUE);
      }
   return(FALSE);
}

// Initialize instance specific variables
int InitLoadNSho(HWND hWnd, int width, int length, int bppixel)
{
   int rcode;
   HDC hdc;

   // Allocate our image and palette buffer
   if((rcode = allocimage(&Image, width, length, bppixel)) == NO_ERROR) {
      zeroimage(0, &Image);   // Zero image buffer
      // Write main window caption
      lstrcpy(szRealFile, "untitled"); // File has not yet been loaded
      SetMainCaption(hWnd, szAppName, szRealFile);

      hdc = GetDC(hWnd);
      // Create and realize a logical palette
      victowinpal(&Image, &HPalette);
      HPalette = SelectPalette(hdc, HPalette, 0);
      RealizePalette(hdc);
      // Remove logical palette before releasing DC
      HPalette = SelectPalette(hdc, HPalette, 0);
      ReleaseDC(hWnd, hdc);
      }
   error_handler(hWnd, rcode);   // Handle any errors
   return(rcode);
}

/* Register the window classes. Returns TRUE if successful, else FALSE. */
int InitApplication(void)
{
   WNDCLASS wc;   // Struct to define a window class

   wc.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
   wc.lpfnWndProc = WndProc;
   // Extra storage for Class and Window objects
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hInstance;
   wc.hIcon = LoadIcon(hInstance, szAppName);
   wc.hCursor = LoadCursor(NULL, IDC_ARROW);
   // Create brush for erasing background
   wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
   wc.lpszMenuName = szAppName;   // Menu Name is App Name
   wc.lpszClassName = szAppName;   // Class Name is App Name
   return(RegisterClass(&wc));
}

// Set main window caption
void SetMainCaption(HWND hwnd, char *AppName, char *CapName)
{
#define CAPTION_BUFFER_SIZE 80
   char szCaption[CAPTION_BUFFER_SIZE];

   wsprintf(szCaption, "%s - %s (%d x %d x %d)",
      (LPSTR)AppName, (LPSTR)CapName,
      (UINT)Image.bmh->biWidth, (UINT)Image.bmh->biHeight, Image.bmh->biBitCount);
   SetWindowText(hwnd, szCaption);
}

// Load a TIF image file
int load_tif(char *fname, imgdes *image)
{
   TiffData tdat;      // Reserve space for struct
   int rcode;

   // Make sure file exists and get its dimensions
   if((rcode = tiffinfo(fname, &tdat)) == NO_ERROR) {
      // Release the current image buffer
      freeimage(image);
      // Try to allocate the new image buffer to the file dimensions
      if((rcode = allocimage(image, tdat.width, tdat.length, 
      	tdat.vbitcount)) == NO_ERROR)
         // Load the image file
         rcode = loadtif(fname, image);
      }
   return(rcode);
}

/* Fill a buffer with string to use in the Open common dialog boxes. 
	NOTE: des must be large enough to hold the entire filter string 
	since no size checking is performed.
*/
void CreateFilterString(char *templateStr, // Filter string template
   char *filterStr)  // Store the constructed filter string here
{
	int j, len, repChar;

   // Construct a file filter string in filterStr
	lstrcpy(filterStr, templateStr);
   len = lstrlen(filterStr); // Get length of template string
   repChar = filterStr[len - 1]; // Get char to replace
   // Replace repChar's in filter string with \0's
   // as required by the common dialog proc
   for(j=0; j<len; j++) {
      if(filterStr[j] == repChar)
         filterStr[j] = '\0';
      }
}

/* Return LoadNSho version as an integer. The high-order word
   contains the major version number. The low-order word contains
   the minor version number as a two-digit decimal number.
*/
WORD appVersion(void)
{
	char szFullPath[256];
	VS_FIXEDFILEINFO *pvsfi;
	DWORD dwHandle;
	LPVOID pbData;
	HGLOBAL hMem;
	DWORD dwBufsiz;
	UINT uLen;
	WORD rval = 0;	// Assume failure

	// Get module's name
	GetModuleFileName(hInstance, szFullPath, sizeof(szFullPath));
	// Get the size of the version data struct
	if((dwBufsiz = GetFileVersionInfoSize((LPSTR)szFullPath, &dwHandle)) != 0) {
		// Allocate space for version data
		if((hMem = GlobalAlloc(GMEM_MOVEABLE, dwBufsiz)) != 0) {
			pbData = GlobalLock(hMem);
			if(GetFileVersionInfo(szFullPath, dwHandle, dwBufsiz, pbData) != 0 &&
         	VerQueryValue(pbData, "\\", (VOID FAR * FAR *)&pvsfi, &uLen) != 0)
				rval = (((UCHAR)HIWORD(pvsfi->dwFileVersionMS)) << 8) |
					((UCHAR)LOWORD(pvsfi->dwFileVersionMS));
			// Free memory
			GlobalUnlock(hMem);
			GlobalFree(hMem);
			}
		}
	return(rval);
}

/* Processes messages for "About" dialog box */
int _export WINAPI About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
#define ABOUTSTR_BUFFER_SIZE 128
   char buff[ABOUTSTR_BUFFER_SIZE];
   WORD version;

   (void)lParam;
   switch(msg) {
      case WM_INITDIALOG:
         // Display LoadNSho version
         version = appVersion();
         wsprintf(buff, "LoadNSho version %d.%02d",
            HIBYTE(version), LOBYTE(version));
         SetDlgItemText(hDlg, IDC_VAL1, buff);

         // Display Victor Library version
         version = Victorversion();
         wsprintf(buff, "Victor Library version %d.%02d",
            HIBYTE(version), LOBYTE(version));
         SetDlgItemText(hDlg, IDC_VAL2, buff);
         return(TRUE);

      case WM_COMMAND:
         if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, TRUE);
            return(TRUE);
            }
         break;
      }
   return(FALSE);
}


/* Error handler for various functions. If error_list returns a vaild
   address, display an error message. Errors are listed in VICDEFS.H.
*/
void _cdecl error_handler(HWND hWnd, int errcode, ...)
{
   static int inErrflag;
   LPSTR arg_ptr;
   char szErrstr[64], szBuff[128];   // Handle up to 128 chars

   if(errcode != NO_ERROR) {    // Ignore an error code of NO_ERROR
      // Enter this function only if we're not already here
      if(inErrflag == 0) {
         inErrflag++;         // Set "already here" flag
         if(outrange(ERR_DEFAULT, errcode, NO_ERROR))
            errcode = ERR_DEFAULT;   // Set any invalid error codes to default
         MessageBeep(0);
         // Load string from resource file
         LoadString(hInstance, errcode, szErrstr, sizeof(szErrstr));
         arg_ptr = (LPSTR)&errcode + sizeof(errcode);
         wvsprintf(szBuff, szErrstr, (LPSTR)arg_ptr);
         MessageBox(hWnd, szBuff, szAppName, MB_OK | MB_ICONSTOP);
         inErrflag--;         // Clear "already here" flag
         }
      }
}
