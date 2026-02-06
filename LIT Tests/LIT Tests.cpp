#include <iostream>
#include <LITemplates/func/delegates.h>
#include <LITemplates/pointers/Auto.h>
#include <LITemplates/types/vectortypes.h>
#include <LITemplates/datastructs/dynamicarray.h>
void bufPrintf(const char* buff) {
    printf("%s", buff);
}
static void InitLegit() {
    legit::LITLogger::OutputToFunction(bufPrintf);
}
class SampleObject {
public:
    SampleObject() {
        printf("[SAMPLEOBJECT] Creating Object Default Initialization\n");
    }
    SampleObject(int operation) {
        printf("[SAMPLEOBJECT] Creating Object Integer Constructor\n");
        this->Value = operation;
    }
    SampleObject(const SampleObject& copy) {
        printf("[SAMPLEOBJECT] Copying Object using Copy Constructor\n");
        this->Value = copy.Value;
    }
    SampleObject& operator=(const SampleObject& copy) {
        printf("[SAMPLEOBJECT] Copying Object using Copy Equals\n");
        this->Value = copy.Value;
        return *this;
    }
    SampleObject(SampleObject&& move) noexcept {
        printf("[SAMPLEOBJECT] Moving Object using Move Constructor\n");
        this->Value = move.Value;
        move.Value = 0;
    }
    SampleObject& operator=(SampleObject&& move) noexcept {
        printf("[SAMPLEOBJECT] Moving object using Move Equals\n");
        this->Value = move.Value;
        move.Value = 0;
        return *this;
    }
    ~SampleObject() {
        printf("[SAMPLEOBJECT] Deleting Object\n");
    }
    bool operator==(const SampleObject& o) {
        if (o.Value == Value) return true;
        return false;
    }
    int Value{};
};
#include <Windows.h>
#include <windowsx.h>

class ApplicationWindowProcessor {
    struct sWindowInformation {
        bool IsCloseRequested;
        bool IsQuitRequested;
        bool WasWindowJustCreated;
        unsigned long WindowWidth, WindowHeight;
    };
public:
    static constexpr char IGNORE_RESPONSE = 0;
    static void AddWindowProc(WNDPROC proc) {
        pProc.emplace_back(proc);
    }
    static void Update() {
        MSG pMsg{};
        while (PeekMessage(&pMsg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&pMsg);
            DispatchMessageW(&pMsg);
            if (pMsg.message == WM_QUIT) {
                WindowInfo.IsQuitRequested = true;
            }
        }
    }
    static LRESULT CALLBACK MainWindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam) {
        switch (m) {
            case WM_NCCREATE:
                WindowInfo.WasWindowJustCreated = true;
                break;
            case WM_CREATE: {
                    CREATESTRUCT* pStruct = (CREATESTRUCT*)lParam;
                    WindowInfo.WindowHeight = pStruct->cy;
                    WindowInfo.WindowWidth = pStruct->cx;
                    WindowInfo.WasWindowJustCreated = false;
                    break;
                }
            case WM_CLOSE:
                WindowInfo.IsCloseRequested = true;
                break;
            case WM_SIZE:
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                WindowInfo.WindowWidth = width;
                WindowInfo.WindowHeight = height;
                break;
        }
        for (const auto& a : pProc) {
            LRESULT result = a(wnd, m, wParam, lParam);

            if (result != IGNORE_RESPONSE)
                return result;
        }
        return DefWindowProc(wnd, m, wParam, lParam);
    }
    static sWindowInformation& Get() {
        return WindowInfo;
    }
private:
    static inline sWindowInformation WindowInfo{};
    static inline std::vector<WNDPROC> pProc;
};

namespace legit {
    class ioMouse {
    public:
        friend class ioHandler;
        using IOMouseType = int;
        using DeltaType = int;
        using MouseNormalized = float;
        DeltaType GetDeltaX() const {
            return this->MouseXDelta;
        }
        DeltaType GetDeltaY() const {
            return this->MouseYDelta;
        }
        IOMouseType GetX() const {
            return this->MouseX;
        }
        IOMouseType GetY() const {
            return this->MouseY;
        }
        MouseNormalized GetNormX() const {
            return MouseNormalizedX;
        }
        MouseNormalized GetNormY() const {
            return MouseNormalizedY;
        }
    private:
        DeltaType MouseXDelta;
        DeltaType MouseYDelta;
        IOMouseType MouseX;
        IOMouseType MouseY;
        MouseNormalized MouseNormalizedX;
        MouseNormalized MouseNormalizedY;
    };
    class ioHandler {
    public:
        static void SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY) {
            SetMousePrivate(pMouse, DeltaX, DeltaY, MouseX, MouseY, 0, 0);
        }
        static void SetMousePrivate(ioMouse* pMouse, ioMouse::DeltaType DeltaX, ioMouse::DeltaType DeltaY, ioMouse::IOMouseType MouseX, ioMouse::IOMouseType MouseY, ioMouse::MouseNormalized NormalizedX, ioMouse::MouseNormalized NormalizedY) {
            pMouse->MouseX = MouseX;
            pMouse->MouseY = MouseY;
            pMouse->MouseXDelta = DeltaX;
            pMouse->MouseYDelta = DeltaY;
            pMouse->MouseNormalizedX = NormalizedX;
            pMouse->MouseNormalizedY = NormalizedY;
        }
    private:
    };
}
class CIOBasis {
public:
    static void Init() {
        ApplicationWindowProcessor::AddWindowProc(WindowProc);
    }
    static LRESULT WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam) {
        switch (m) {
            case WM_MOUSEMOVE:
                MouseX = GET_X_LPARAM(lParam);
                MouseY = GET_Y_LPARAM(lParam);
                break;
        }
        return 0;
    }
    static legit::ioMouse::IOMouseType GetMouseX() {
        return MouseX;
    }
    static legit::ioMouse::IOMouseType GetMouseY() {
        return MouseY;
    }
private:
    static inline legit::ioMouse::IOMouseType MouseX = 0, MouseY = 0;
};
class CRawInput {
public:
    static bool IsNotAllowedToProcess(UINT msg, WPARAM wParam) {
        return msg != WM_INPUT || GetWParamCode(wParam) == RIM_INPUTSINK;
    }
    static LRESULT WindowProc(HWND wnd, UINT m, WPARAM wParam, LPARAM lParam) {
        if (IsNotAllowedToProcess(m, wParam)) {
            return 0;
        }
        HRESULT hResult{};
        UINT dwSize{};
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        std::vector<RAWINPUT> pRaw;
        pRaw.resize(dwSize);
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, pRaw.data(), &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
            printf("get rawinputdata didn't return right size\n");
        }
        RAWINPUT* raw = pRaw.data();
        if (pRaw.data()->header.dwType == RIM_TYPEKEYBOARD) {
            //printf(" Kbd: make=%04x Flags:%04x Reserved:%04x ExtraInformation:%08x, msg=%04x VK=%04x \n", raw->data.keyboard.MakeCode, raw->data.keyboard.Flags, raw->data.keyboard.Reserved, raw->data.keyboard.ExtraInformation, raw->data.keyboard.Message, raw->data.keyboard.VKey);
        } else if (raw->header.dwType == RIM_TYPEMOUSE) {
			CRawInput::MouseX += raw->data.mouse.lLastX;
			CRawInput::MouseY += raw->data.mouse.lLastY;
        }
        return 0;
    }
    static void Init() {
        ApplicationWindowProcessor::AddWindowProc(WindowProc);

        RAWINPUTDEVICE Rid[2];

        Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
        Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
        Rid[0].dwFlags = 0;    // adds mouse and also ignores legacy mouse messages
        Rid[0].hwndTarget = 0;

        Rid[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
        Rid[1].usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
        Rid[1].dwFlags = 0;    // adds keyboard and also ignores legacy keyboard messages
        Rid[1].hwndTarget = 0;

        if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE) {

        }
        else {
            printf("Successfully registered RawInputDevices\n");
        }
    }
    static void Update() {

    }
    static void Shutdown() {

    }
    static int MouseDeltaX() {
        return MouseX;
    }
    static int MouseDeltaY() {
        return MouseY;
    }
private:
    static char GetWParamCode(WPARAM wpar) {
        return GET_RAWINPUT_CODE_WPARAM(wpar);
    }
    static inline int MouseX = 0;
    static inline int MouseY = 0;
};
class CInput{
public:
    static void Init() {
        CIOBasis::Init();
        CRawInput::Init();
        mMouse = new legit::ioMouse();
    }
    static const legit::ioMouse& GetMouse() {
        return *mMouse;
    }
    static void Update() {
        CRawInput::Update();
        float x = float(CIOBasis::GetMouseX()) / float(ApplicationWindowProcessor::Get().WindowWidth); 
        float y = float(CIOBasis::GetMouseY()) / float(ApplicationWindowProcessor::Get().WindowHeight);
        legit::ioHandler::SetMousePrivate(mMouse, CRawInput::MouseDeltaX(), CRawInput::MouseDeltaY(), CIOBasis::GetMouseX(), CIOBasis::GetMouseY(), x, y);
        printf("Deltas(%d %d) Position(%d %d) NormedPos(%f, %f)\n", mMouse->GetDeltaX(), mMouse->GetDeltaY(), mMouse->GetX(), mMouse->GetY(), mMouse->GetNormX(), mMouse->GetNormY());
    }
    static void Shutdown() {
        delete mMouse;
        CRawInput::Shutdown();
    }
private:
    static inline legit::ioMouse* mMouse = nullptr;
};
template<typename T> class Castable {
public:
    Castable(T* pointer) {
        this->Pointer = pointer;
    }
    template<typename K>
    K GetAs() {
        return (K)(this->Pointer);
    }
    T* GetPointer() {
        return this->Pointer;
    }
private:
    T* Pointer;
};
class CSystem {
public:
    static Castable<void> GetInstance() {
        return GetModuleHandle(NULL);
    }
private:
};
class CAppl {
public:
    static void Init() {
        const wchar_t CLASS_NAME[] = L"WindowClass";
        WNDCLASS wc = { };
        wc.lpfnWndProc = ApplicationWindowProcessor::MainWindowProc;
        wc.hInstance = CSystem::GetInstance().GetAs<HINSTANCE>();
        wc.lpszClassName = CLASS_NAME;
        
        auto res = RegisterClassW(&wc);
        if (!res) {
            printf("Window Register Class failed, %d\n", GetLastError());
            __debugbreak();
        }
        HWND hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            L"Learn to Program Windows",    // Window text
            WS_OVERLAPPEDWINDOW,            // Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

            NULL,       // Parent window    
            NULL,       // Menu
            CSystem::GetInstance().GetAs<HINSTANCE>(),  // Instance handle
            NULL        // Additional application data
        );
        if (hwnd == NULL) {
            printf("Failed to create window %d\n", GetLastError());
            __debugbreak();
            return;
        }
        ShowWindow(hwnd, SW_SHOW);
        CInput::Init();
    }
    static void Update() {
        while (!ApplicationWindowProcessor::Get().IsCloseRequested) {
            ApplicationWindowProcessor::Update();
            CInput::Update();
        }
    }
    static void Shutdown() {
        CInput::Shutdown();
    }
private:
    void* Window = nullptr;
};

int main()
{
    CAppl::Init();
    CAppl::Update();
    CAppl::Shutdown();
    return 0;
}
