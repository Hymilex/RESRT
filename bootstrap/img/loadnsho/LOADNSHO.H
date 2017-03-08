/* LOADNSHO.H */
#define DEFIMGWIDTH  256   /* Default image buffer size */
#define DEFIMGLENGTH 240
#define DEFIMGBPPIXEL  8

/* Menu item IDs */
#define IDM_F_OPEN     600
#define IDM_F_EXIT     610
#define IDM_F_ABOUT    620

/* Dialog Box templates */
#define IDD_ABOUTBOX   400

/* Control IDs */
#define IDC_ABOUT1     410

/* Text controls */
#define IDC_VAL1              320
#define IDC_VAL2              321

/* Program error codes */
#define ERR_BAD_PATH   -51
#define ERR_REG_CLASS  -52
#define ERR_CRT_MANWIN -53
#define ERR_DEFAULT    -54

/* Function declarations */
#ifdef __cplusplus
   extern "C" {
#endif
LRESULT _export WINAPI WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
int _export WINAPI About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Common dialog DLL functions
// int FAR PASCAL GetOpenFileName(OPENFILENAME far *);
#ifdef __cplusplus
 }
#endif

int  InitApplication(void);
int  InitInstance(LPCSTR lpCmdLine, int nCmdShow);
int  InitLoadNSho(HWND hWnd, int width, int length, int bppixel);
int  MenuCommand(HWND hWnd, WPARAM id);
int  load_tif(char *fname, imgdes *image);
WORD appVersion(void);
void CreateFilterString(char *list, char *des);
void set_scroll_ranges(HWND hwnd, imgdes *image);
void SetMainCaption(HWND hwnd, char *szAppName, char *szCapName);
void _cdecl error_handler(HWND hWnd, int errcode, ...);

