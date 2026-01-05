#define kWindowStandardFloatingAttributes ((1<<0)|(1<<3))
#define kWindowStandardHandlerAttribute (1<<25)
#define kDocumentWindowClass 6
#define kEventClassWindow 'wind'
#define kEventWindowDrawContent 2
#define kEventWindowActivated 5
#define kEventWindowBoundsChanged 27
#define eventNotHandledErr -9874

typedef void *EventHandlerCallRef;
typedef void *EventRef;
uint32_t GetEventClass(EventRef);
uint32_t GetEventKind(EventRef);

typedef int OSStatus;
OSStatus CreateNewWindow(unsigned, unsigned, Rect *, WindowRef *);
typedef ProcPtr EventHandlerUPP;

typedef struct {
  uint32_t eventClass, eventKind;
} EventTypeSpec;

CGrafPtr GetWindowPort(WindowRef);
void RunApplicationEventLoop(void);
OSStatus InvalWindowRect(WindowRef, Rect *);
OSStatus GetEventParameter(EventRef, uint32_t, uint32_t, void *, uint32_t, uint32_t *, void *);
OSStatus InstallEventHandler(void *, void *, uint32_t, EventTypeSpec *, void *, void *);
void *NewEventHandlerUPP(void *);
void *GetWindowEventTarget(WindowRef);
