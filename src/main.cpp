#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <windows.h>
#include <algorithm>
#include <windowsx.h>
#include <Lmcons.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

#include <nlohmann/json.hpp>

#include <dxgi.h>
#include <intrin.h>
#include <cstring>
#include <mmsystem.h>

using json = nlohmann::json;

struct CharacterGen {
    // proportions
    int headToBody = 1;
    int bodyType = 0;
    int heightClass = 1;

    // face
    float eyeSize = 0.8f;
    float eyeAngle = 0.2f;
    float chinSharpness = 0.4f;
    float browAngle = 0.1f;
    float blush = 0.5f;

    // colors
    ImVec4 hairMain = ImVec4(1.0f, 0.53f, 0.80f, 1.0f);
    ImVec4 hairAccent = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 eyeColor = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
    ImVec4 skinColor = ImVec4(1.0f, 0.85f, 0.75f, 1.0f);

    // style
    int artStyle = 0;
    int lineWeight = 1;
    int colorStyle = 1;

    // hair
    int baseStyle = 4;
    int bangs = 1;

    // outfit
    int outfit = 0;
    int detailLevel = 2;
    int paletteMood = 0;

    // personality
    int vibe = 3;
    int expression = 0;

    // generated text
    std::string description;
    std::string jsonString;
};

CharacterGen CG;


static float fpsAccum        = 0.0f;
static int   fpsCount        = 0;

static MEMORYSTATUSEX memInfo = { sizeof(memInfo) };
static volatile bool g_closingMic = false;

void UpdateHardwareStats()
{
    GlobalMemoryStatusEx(&memInfo);
}

// Microphone
static std::vector<std::string> micDevices;
static size_t selectedMic  = 0;
static float micLevel      = 0.0f;

static HWAVEIN   g_waveIn  = nullptr;
static short     g_buffer[2048];
static WAVEHDR   g_hdr;
static bool      g_micOpen = false;

static float g_smoothLevel = 0.0f;

void CALLBACK WaveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR, DWORD_PTR dwParam1, DWORD_PTR)
{
    if (g_closingMic) return;
    if (uMsg != WIM_DATA || hwi != g_waveIn) return;

    WAVEHDR* hdr = (WAVEHDR*)dwParam1;
    int samples = hdr->dwBytesRecorded / sizeof(short);

    float peak = 0.0f;
    for (int i = 0; i < samples; ++i)
    {
        float v = fabsf(g_buffer[i] / 32768.0f);
        if (v > peak) peak = v;
    }

    micLevel = peak;
    waveInAddBuffer(g_waveIn, hdr, sizeof(WAVEHDR));
}

void InitMicrophones()
{
    UINT count = waveInGetNumDevs();
    micDevices.clear();

    for (UINT i = 0; i < count; ++i)
    {
        WAVEINCAPSW caps;
        if (waveInGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
        {
            std::wstring ws(caps.szPname);
            micDevices.push_back(std::string(ws.begin(), ws.end()));
        }
    }

    if (!micDevices.empty())
        selectedMic = 0;
}

void CloseMic()
{
    if (!g_micOpen) return;

    g_closingMic = true;

    waveInStop(g_waveIn);
    Sleep(10);
    waveInReset(g_waveIn);

    waveInUnprepareHeader(g_waveIn, &g_hdr, sizeof(g_hdr));
    waveInClose(g_waveIn);

    g_waveIn = nullptr;
    g_micOpen = false;
    micLevel = 0.0f;
    g_smoothLevel = 0.0f;

    g_closingMic = false;
}

bool RefreshMicrophones()
{
    std::vector<std::string> newList;

    UINT count = waveInGetNumDevs();
    for (UINT i = 0; i < count; ++i)
    {
        WAVEINCAPSW caps;
        if (waveInGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR)
        {
            std::wstring ws(caps.szPname);
            newList.push_back(std::string(ws.begin(), ws.end()));
        }
    }

    if (newList == micDevices)
        return false;

    micDevices = newList;

    if (selectedMic >= micDevices.size())
    {
        CloseMic();
        selectedMic = 0;
    }

    return true;
}
void StartMic(int index)
{
    CloseMic();

    UINT count = waveInGetNumDevs();
    if (index < 0 || (UINT)index >= count) return;

    WAVEFORMATEX fmt = {};
    fmt.wFormatTag      = WAVE_FORMAT_PCM;
    fmt.nChannels       = 1;
    fmt.nSamplesPerSec  = 44100;
    fmt.wBitsPerSample  = 16;
    fmt.nBlockAlign     = fmt.nChannels * (fmt.wBitsPerSample / 8);
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

    if (waveInOpen(&g_waveIn, index, &fmt,
                   (DWORD_PTR)WaveInProc, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
        return;

    ZeroMemory(&g_hdr, sizeof(g_hdr));
    g_hdr.lpData         = (LPSTR)g_buffer;
    g_hdr.dwBufferLength = sizeof(g_buffer);

    waveInPrepareHeader(g_waveIn, &g_hdr, sizeof(g_hdr));
    waveInAddBuffer(g_waveIn, &g_hdr, sizeof(g_hdr));
    waveInStart(g_waveIn);

    g_micOpen = true;
}

void UpdateMicLevel()
{
    g_smoothLevel = g_smoothLevel * 0.85f + micLevel * 0.15f;
}

void AutoSelectMicOnStart()
{
    RefreshMicrophones();

    // No devices → nothing to do
    if (micDevices.empty())
    {
        selectedMic = -1;
        return;
    }

    // If selected mic is invalid, auto-select the first one
    if (selectedMic >= micDevices.size())
    {
        selectedMic = 0;
        StartMic(selectedMic);
        return;
    }

    // If selected mic exists but mic is not open → open it
    if (!g_micOpen)
    {
        StartMic(selectedMic);
    }
}

static ImVec4 LerpColor(const ImVec4& a, const ImVec4& b, float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    return ImVec4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    );
}

void DrawWavyMicVisualizer(float level, float width, float height)
{
    const int segments = 48;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float baseY = pos.y + height * 0.5f;
    float startX = pos.x;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec4 green  = ImVec4(0.20f, 0.85f, 0.30f, 1.0f);
    ImVec4 yellow = ImVec4(0.95f, 0.80f, 0.20f, 1.0f);
    ImVec4 red    = ImVec4(0.95f, 0.25f, 0.20f, 1.0f);

    float tLevel = level;
    if (tLevel < 0.0f) tLevel = 0.0f;
    if (tLevel > 1.0f) tLevel = 1.0f;

    float t = (tLevel < 0.5f) ? (tLevel / 0.5f) : ((tLevel - 0.5f) / 0.5f);
    ImVec4 midColor = (tLevel < 0.5f) ? LerpColor(green, yellow, t)
                                      : LerpColor(yellow, red, t);

    float time = ImGui::GetTime();

    for (int i = 0; i < segments; i++)
    {
        float s = (float)i / (segments - 1);
        float x = startX + s * width;

        float wave = sinf(s * 6.28318f * 2.0f + time * 4.0f);
        float amp  = wave * tLevel * (height * 0.45f);

        ImVec4 c = LerpColor(midColor, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), s * 0.18f);
        ImU32 col = ImGui::GetColorU32(c);

        dl->AddLine(
            ImVec2(x, baseY - amp),
            ImVec2(x, baseY + amp),
            col,
            3.0f
        );
    }

    ImGui::Dummy(ImVec2(width, height));
}

void DrawPerformanceTool(ImFont* fontMedium, ImFont* fontLarge)
{
    ImGuiIO& io = ImGui::GetIO();

    RefreshMicrophones();
    UpdateMicLevel();
    UpdateHardwareStats();

    float fps = io.Framerate;
    fpsAccum += fps;
    fpsCount++;

    float avgFPS = fpsAccum / (fpsCount > 0 ? fpsCount : 1);
    if (fpsCount >= 240)
    {
        fpsAccum = fps;
        fpsCount = 1;
    }

    const double gb = 1024.0 * 1024.0 * 1024.0;

    ImVec2 overlaySize(600, 240);
    ImVec2 overlayPos(io.DisplaySize.x - overlaySize.x - 30, 30);

    ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(overlaySize, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("##input_overlay", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground);

    ImVec4 cardBg = ImVec4(0,0,0,0);
    ImVec4 border = ImVec4(0.25f,0.27f,0.30f,0.35f);
    ImVec4 muted  = ImVec4(0.65f,0.70f,0.75f,1.0f);

    float totalWidth = ImGui::GetContentRegionAvail().x;
    float controlWidth = totalWidth * 0.45f;
    float previewWidth = totalWidth - controlWidth - 12; // spacing

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, border);
    ImGui::BeginChild("mic_card", ImVec2(0,200), true);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    ImGui::BeginGroup();
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10,10));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));

        ImGui::BeginChild("controls", ImVec2(controlWidth,140), false);
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
        
        ImGui::Spacing();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
        ImGui::PushFont(fontLarge);
        ImGui::Text("Device I/O");
        ImGui::PopFont();


        bool connected = g_micOpen;
        ImVec4 statusColor = connected ? ImVec4(0.12f,0.45f,0.28f,1.0f) : ImVec4(0.55f,0.18f,0.18f,1.0f);
        const char* statusText = connected ? "Connected" : "Idle";

        ImVec2 textSize = ImGui::CalcTextSize(statusText);
        ImVec2 badgeSize(textSize.x + 24, textSize.y + 18);

        ImGui::SameLine(controlWidth - badgeSize.x);
        ImGui::PushStyleColor(ImGuiCol_Button, statusColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, statusColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, statusColor);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20);
        ImGui::Button(statusText, badgeSize);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::PushFont(fontMedium);
        ImGui::PushStyleColor(ImGuiCol_Text, muted);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
        ImGui::Text("Mic Input for CORSPRITE");
        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::PushFont(fontMedium);
        if (!micDevices.empty())
        {
            const char* current = micDevices[selectedMic].c_str();
            ImGui::PushItemWidth(-1);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
            if (ImGui::BeginCombo("##mic_device_combo", current))
            {
                for (size_t i = 0; i < micDevices.size(); ++i)
                {
                    bool selected = (i == selectedMic);
                    if (ImGui::Selectable(micDevices[i].c_str(), selected))
                    {
                        selectedMic = i;
                        StartMic(i);
                    }
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
        }
        else
        {
            ImGui::TextColored(ImVec4(1,0.4f,0.4f,1),
                "No microphone devices detected");
        }
        ImGui::PopFont();
        
        ImGui::EndChild();
    }
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));
        ImGui::BeginChild("preview", ImVec2(previewWidth,140), false);
        ImGui::PopStyleColor(2);

        ImGui::Spacing();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
        ImGui::PushFont(fontMedium);
        ImGui::Text("Mic preview:");
        ImGui::PopFont();

        float barWidth = ImGui::GetContentRegionAvail().x * 0.96;
        DrawWavyMicVisualizer(g_smoothLevel, barWidth, 40.0f);

        ImGui::PushStyleColor(ImGuiCol_Text, muted);
        ImGui::Text("FPS %.0f   |   RAM %.2f / %.2f GB",
            avgFPS,
            (memInfo.ullTotalPhys - memInfo.ullAvailPhys)/gb,
            memInfo.ullTotalPhys/gb
        );
        ImGui::PopStyleColor();

        ImGui::EndChild();
    }
    ImGui::EndGroup();
    
    ImGui::PushFont(fontMedium);
    ImGui::PushStyleColor(ImGuiCol_Text, muted);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
    ImGui::Text("More advanced option:");
    ImGui::PopStyleColor();
    ImGui::PopFont();
    ImGui::EndChild();
    ImGui::End();
}

bool UIChanged = false;
#define TRACK(x) if (x) UIChanged = true;

void SaveCharacterJSON() {
    json j;

    j["proportions"] = {
        {"head_to_body", CG.headToBody},
        {"body_type", CG.bodyType},
        {"height_class", CG.heightClass}
    };

    j["face"] = {
        {"eye_size", CG.eyeSize},
        {"eye_angle", CG.eyeAngle},
        {"chin_sharpness", CG.chinSharpness},
        {"brow_angle", CG.browAngle},
        {"blush", CG.blush}
    };

    j["colors"] = {
        {"hair_main", {CG.hairMain.x, CG.hairMain.y, CG.hairMain.z}},
        {"hair_accent", {CG.hairAccent.x, CG.hairAccent.y, CG.hairAccent.z}},
        {"eye", {CG.eyeColor.x, CG.eyeColor.y, CG.eyeColor.z}},
        {"skin", {CG.skinColor.x, CG.skinColor.y, CG.skinColor.z}}
    };

    j["style"] = {
        {"art_style", CG.artStyle},
        {"line_weight", CG.lineWeight},
        {"color_style", CG.colorStyle}
    };

    j["hair"] = {
        {"base_style", CG.baseStyle},
        {"bangs", CG.bangs}
    };

    j["outfit"] = {
        {"type", CG.outfit},
        {"detail", CG.detailLevel},
        {"palette", CG.paletteMood}
    };

    j["personality"] = {
        {"vibe", CG.vibe},
        {"expression", CG.expression}
    };

    std::ofstream file("corsprite_generation.json");
    if (file.is_open()) file << j.dump(4);
}

void LoadCharacterJSON() {
    std::ifstream file("corsprite_generation.json");
    if (!file.is_open()) return;

    json j;
    file >> j;

    CG.headToBody = j["proportions"]["head_to_body"];
    CG.bodyType = j["proportions"]["body_type"];
    CG.heightClass = j["proportions"]["height_class"];

    CG.eyeSize = j["face"]["eye_size"];
    CG.eyeAngle = j["face"]["eye_angle"];
    CG.chinSharpness = j["face"]["chin_sharpness"];
    CG.browAngle = j["face"]["brow_angle"];
    CG.blush = j["face"]["blush"];

    auto hm = j["colors"]["hair_main"];
    CG.hairMain = ImVec4(hm[0], hm[1], hm[2], 1.0f);

    auto ha = j["colors"]["hair_accent"];
    CG.hairAccent = ImVec4(ha[0], ha[1], ha[2], 1.0f);

    auto ec = j["colors"]["eye"];
    CG.eyeColor = ImVec4(ec[0], ec[1], ec[2], 1.0f);

    auto sc = j["colors"]["skin"];
    CG.skinColor = ImVec4(sc[0], sc[1], sc[2], 1.0f);

    CG.artStyle = j["style"]["art_style"];
    CG.lineWeight = j["style"]["line_weight"];
    CG.colorStyle = j["style"]["color_style"];

    CG.baseStyle = j["hair"]["base_style"];
    CG.bangs = j["hair"]["bangs"];

    CG.outfit = j["outfit"]["type"];
    CG.detailLevel = j["outfit"]["detail"];
    CG.paletteMood = j["outfit"]["palette"];

    CG.vibe = j["personality"]["vibe"];
    CG.expression = j["personality"]["expression"];
}

void InitCharacterSystem() {
    LoadCharacterJSON();
}

bool CorspriteSliderFloat(const char* label, float* v, float v_min, float v_max, float width = 200.0f)
{
    ImGui::PushID(label);

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, 140);
    ImGui::TextUnformatted(label);
    ImGui::NextColumn();

    ImGui::PushItemWidth(width);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.40f, 0.60f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.50f, 0.70f, 1.0f, 1.0f));

    bool changed = ImGui::SliderFloat("##slider", v, v_min, v_max);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    ImGui::PopItemWidth();
    ImGui::Columns(1);

    ImGui::PopID();
    return changed;
}

bool CorspriteSliderInt(const char* label, int* v, int v_min, int v_max, float width = 200.0f)
{
    ImGui::PushID(label);

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, 140);
    ImGui::TextUnformatted(label);
    ImGui::NextColumn();

    ImGui::PushItemWidth(width);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 6));

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.40f, 0.60f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.50f, 0.70f, 1.0f, 1.0f));

    bool changed = ImGui::SliderInt("##slider", v, v_min, v_max);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(3);

    ImGui::PopItemWidth();
    ImGui::Columns(1);

    ImGui::PopID();
    return changed;
}

void CorspriteCharacterWidget(ImFont* fontMedium, ImFont* fontLarge)
{
    ImGuiIO& io = ImGui::GetIO();

    const float widgetX = io.DisplaySize.x - 400.0f - 20.0f;
    const float widgetWidth = 400.0f;
    const float widgetHeight = io.DisplaySize.y - 480.0f;

    ImGui::SetNextWindowPos(ImVec2(widgetX, io.DisplaySize.y - 620.0f));
    ImGui::SetNextWindowSize(ImVec2(widgetWidth, widgetHeight));

    ImGui::PushFont(fontMedium);

    ImGui::Begin("##CharacterGen", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);

    ImGui::PushFont(fontLarge);
    ImGui::Text("CORSPRITE Character");
    ImGui::PopFont();

    bool changed = false;

    ImGui::Spacing();
    ImGui::SeparatorText("Proportions");

    TRACK(CorspriteSliderInt("Head:Body", &CG.headToBody, 0, 3));
    TRACK(CorspriteSliderInt("Body Type", &CG.bodyType, 0, 3));
    TRACK(CorspriteSliderInt("Height", &CG.heightClass, 0, 2));

    ImGui::Spacing();
    ImGui::SeparatorText("Face");

    TRACK(CorspriteSliderFloat("Eye Size", &CG.eyeSize, 0.0f, 1.0f));
    TRACK(CorspriteSliderFloat("Eye Angle", &CG.eyeAngle, -1.0f, 1.0f));
    TRACK(CorspriteSliderFloat("Chin Sharpness", &CG.chinSharpness, 0.0f, 1.0f));
    TRACK(CorspriteSliderFloat("Brow Angle", &CG.browAngle, -1.0f, 1.0f));
    TRACK(CorspriteSliderFloat("Blush", &CG.blush, 0.0f, 1.0f));

    TRACK(ImGui::ColorEdit4("Eye Color", (float*)&CG.eyeColor));
    TRACK(ImGui::ColorEdit4("Skin Color", (float*)&CG.skinColor));

    ImGui::Spacing();
    ImGui::SeparatorText("Hair");

    TRACK(CorspriteSliderInt("Base Style", &CG.baseStyle, 0, 5));
    TRACK(CorspriteSliderInt("Bangs", &CG.bangs, 0, 3));
    TRACK(ImGui::ColorEdit4("Main Hair", (float*)&CG.hairMain));
    TRACK(ImGui::ColorEdit4("Accent Hair", (float*)&CG.hairAccent));

    ImGui::Spacing();
    ImGui::SeparatorText("Style");

    TRACK(CorspriteSliderInt("Art Style", &CG.artStyle, 0, 4));
    TRACK(CorspriteSliderInt("Line Weight", &CG.lineWeight, 0, 2));
    TRACK(CorspriteSliderInt("Color Style", &CG.colorStyle, 0, 3));

    ImGui::Spacing();
    ImGui::SeparatorText("Outfit");

    TRACK(CorspriteSliderInt("Outfit Type", &CG.outfit, 0, 5));
    TRACK(CorspriteSliderInt("Detail Level", &CG.detailLevel, 0, 2));
    TRACK(CorspriteSliderInt("Palette Mood", &CG.paletteMood, 0, 3));

    ImGui::Spacing();
    ImGui::SeparatorText("Personality");

    TRACK(CorspriteSliderInt("Vibe", &CG.vibe, 0, 5));
    TRACK(CorspriteSliderInt("Expression", &CG.expression, 0, 4));

    if (changed) SaveCharacterJSON();

    ImGui::End();
    ImGui::PopFont();
}

void CorspriteBottomCarousel(ImFont* fontMedium)
{
    ImGuiIO& io = ImGui::GetIO();

    const float leftMargin  = 480.0f;
    const float rightMargin = 540.0f;

    const float elementHeight = 80.0f;
    const float barHeight     = elementHeight + 6.0f;
    const float barWidth      = io.DisplaySize.x - (leftMargin + rightMargin);
    const float barX          = leftMargin;
    const float barY          = io.DisplaySize.y - barHeight - 20.0f;

    const float squareSize = 40.0f;

    // DATA
    static const char* labels[10] =
    {
        "Skin","Hair","Eyes","Lips","Brows",
        "Top","Bottom","Shoes","Accessory","Accent"
    };

    static ImVec4 colors[10] =
    {
        {0.90f,0.75f,0.60f,1.0f},
        {0.20f,0.10f,0.05f,1.0f},
        {0.15f,0.30f,0.80f,1.0f},
        {0.80f,0.20f,0.35f,1.0f},
        {0.10f,0.05f,0.02f,1.0f},
        {0.30f,0.50f,0.90f,1.0f},
        {0.20f,0.60f,0.30f,1.0f},
        {0.10f,0.10f,0.10f,1.0f},
        {0.90f,0.80f,0.10f,1.0f},
        {0.80f,0.40f,0.10f,1.0f}
    };

    // WINDOW
    ImGui::SetNextWindowPos({barX, barY});
    ImGui::SetNextWindowSize({barWidth, barHeight});

    ImGui::PushStyleColor(ImGuiCol_WindowBg, {0.05f,0.05f,0.05f,0.55f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10,0});

    ImGui::PushFont(fontMedium);

    ImGui::Begin("##BottomColorCarousel", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar);

    // SCROLLBAR STYLE
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0,0});

    ImGui::BeginChild("##BottomScroll",
    {barWidth, barHeight},
    false,
    ImGuiWindowFlags_AlwaysHorizontalScrollbar |
    ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 14.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {30,0});

    ImDrawList* dl = ImGui::GetWindowDrawList();

    for (int i = 0; i < 10; i++)
    {
        ImGui::BeginGroup();

        float startX = ImGui::GetCursorPosX();
        float startY = ImGui::GetCursorPosY();

        float totalHeight = squareSize + 22.0f;
        float offsetY = (elementHeight - totalHeight) * 0.5f;

        ImGui::SetCursorPosY(startY + offsetY);

        // Clamp hover color
        ImVec4 hoverColor =
        {
            std::min(colors[i].x + 0.15f, 1.0f),
            std::min(colors[i].y + 0.15f, 1.0f),
            std::min(colors[i].z + 0.15f, 1.0f),
            1.0f
        };

        // ID buffers (no allocations)
        char btnID[32];
        char popupID[32];
        char pickerID[32];

        sprintf(btnID,   "##color%d", i);
        sprintf(popupID, "ColorPopup%d", i);
        sprintf(pickerID,"##picker%d", i);

        ImGui::PushStyleColor(ImGuiCol_Button, colors[i]);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors[i]);

        bool clicked = ImGui::Button(btnID, {squareSize, squareSize});
        bool hovered = ImGui::IsItemHovered();

        ImGui::PopStyleColor(3);

        if (clicked)
            ImGui::OpenPopup(popupID);

        // DRAW OUTLINE
        ImVec2 pMin = ImGui::GetItemRectMin();
        ImVec2 pMax = ImGui::GetItemRectMax();

        float glow = hovered ? 200.0f : 70.0f;
        float thickness = hovered ? 2.8f : 1.3f;

        dl->AddRect(
            {pMin.x-2,pMin.y-2},
            {pMax.x+2,pMax.y+2},
            IM_COL32(255,255,255,(int)glow),
            6.0f,
            0,
            thickness
        );

        // LABEL
        float textWidth = ImGui::CalcTextSize(labels[i]).x;

        ImGui::SetCursorPosX(startX + squareSize*0.5f - textWidth*0.5f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);

        ImGui::TextUnformatted(labels[i]);

        // POPUP POSITION
        ImVec2 popupPos(pMin.x, pMin.y - 310.0f);
        ImGui::SetNextWindowPos(popupPos);

        if (ImGui::BeginPopup(popupID))
        {
            ImGui::SetNextItemWidth(260);

            ImGui::ColorPicker4(
                pickerID,
                (float*)&colors[i],
                ImGuiColorEditFlags_NoSidePreview |
                ImGuiColorEditFlags_NoSmallPreview |
                ImGuiColorEditFlags_DisplayRGB |
                ImGuiColorEditFlags_DisplayHex |
                ImGuiColorEditFlags_PickerHueWheel
            );

            ImGui::EndPopup();
        }

        ImGui::EndGroup();

        if (i < 9)
            ImGui::SameLine();
    }

    ImGui::PopStyleVar(); // ItemSpacing

    ImGui::EndChild();

    ImGui::PopStyleVar(3); // Scrollbar vars

    ImGui::End();

    ImGui::PopFont();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

void OpenURL(const char* url) {
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

// Globals for Win32 subclassing
static WNDPROC g_OriginalWndProc = nullptr;
static HWND    gHwnd             = nullptr;

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_NCHITTEST:
        {
            // If ImGui isn't ready yet, fall back to default behavior
            if (ImGui::GetCurrentContext() == nullptr)
                break;

            // Mouse position in screen coords
            POINT p;
            p.x = GET_X_LPARAM(lParam);
            p.y = GET_Y_LPARAM(lParam);

            // Convert to client coords
            ScreenToClient(hwnd, &p);

            // Feed ImGui the mouse position (so hover tests are correct)
            ImGuiIO& io = ImGui::GetIO();
            io.MousePos = ImVec2((float)p.x, (float)p.y);

            // Check if hovering any ImGui widget or window
            bool hoveringUI =
                ImGui::IsAnyItemHovered() ||
                ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
            bool wantsKeyboard =
                ImGui::GetIO().WantTextInput ||
                ImGui::GetIO().WantCaptureKeyboard;

            if (hoveringUI || wantsKeyboard)
                return HTCLIENT;        // allow mouse + keyboard
            else
                return HTCLIENT;   // pass through mouse, but still allow keyboard (e.g. for global shortcuts)
        }
    }

    return CallWindowProc(g_OriginalWndProc, hwnd, msg, wParam, lParam);
}

static std::unordered_map<ImGuiID, float> g_SwitchAnim;

inline float LerpFloat(float a, float b, float t)
{
    return a + (b - a) * t;
}

bool ToggleSwitch(const char* id, bool* v, ImVec4 onColor, ImVec4 offColor, ImVec4 knobColor)
{
    ImGui::PushID(id);
    ImGuiID switchID = ImGui::GetID("##switch_anim");

    float& anim = g_SwitchAnim[switchID];   // each switch gets its own animation float

    const float width  = 40.0f;
    const float height = 20.0f;
    const float radius = height * 0.5f;

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##switch", ImVec2(width, height));

    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    if (clicked)
        *v = !*v;

    float target = *v ? 1.0f : 0.0f;
    anim = LerpFloat(anim, target, 0.18f);
    ImVec4 bg4 = LerpColor(offColor, onColor, anim);
    ImU32 bg  = ImGui::GetColorU32(bg4);


    if (hovered)
    {
        // Convert packed ImU32 → float RGBA
        ImVec4 col = ImGui::ColorConvertU32ToFloat4(bg);

        col.x = col.x + 0.05f;
        if (col.x > 1.0f) col.x = 1.0f;

        col.y = col.y + 0.05f;
        if (col.y > 1.0f) col.y = 1.0f;

        col.z = col.z + 0.05f;
        if (col.z > 1.0f) col.z = 1.0f;

        // Convert back to ImU32 like why not
        bg = ImGui::GetColorU32(col);

        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }


    ImU32 bgCol   = ImGui::GetColorU32(bg);
    ImU32 knobCol = ImGui::GetColorU32(knobColor);

    ImGui::GetWindowDrawList()->AddRectFilled(
        p,
        ImVec2(p.x + width, p.y + height),
        bgCol,
        radius
    );

    float knobX = p.x + radius + anim * (width - height);

    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(knobX, p.y + radius),
        radius - 2.0f,
        knobCol
    );

    ImGui::PopID();
    return clicked;
}

void SettingsRow(const char* id, const char* title, const char* desc,
                 bool* value, ImFont* fontXS,
                 ImVec4 onColor, ImVec4 offColor, ImVec4 knobColor)
{
    ImGui::PushID(id);

    float fullWidth   = ImGui::GetContentRegionAvail().x;
    float switchWidth = 40.0f;
    float rowHeight   = 78.0f;

    // Reserve the row area
    ImVec2 rowStart = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##row_click", ImVec2(fullWidth, rowHeight));
    bool rowClicked = ImGui::IsItemClicked();

    if (rowClicked)
        *value = !*value;
    
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        

    // Reset cursor to draw content on top of the invisible button
    ImGui::SetCursorScreenPos(rowStart);

    // LEFT — title
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
    ImGui::TextUnformatted(title);

    // LEFT — description
    if (desc && desc[0] != '\0')
    {
        ImGui::PushFont(fontXS);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 1);
        ImGui::TextWrapped(desc);

        ImGui::PopStyleColor();
        ImGui::PopFont();
    }

    // RIGHT — switch
    float switchX = rowStart.x + fullWidth - switchWidth - 10.0f;
    float switchY = rowStart.y + 10.0f;

    ImGui::SetCursorScreenPos(ImVec2(switchX, switchY));
    ToggleSwitch("##switch", value, onColor, offColor, knobColor);

    ImGui::PopID();
    
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
}

void InputRow(const char* id, const char* label, const char* subtext, char* buffer, size_t bufferSize, const char* placeholder, ImFont* fontSmall, ImFont* fontMedium)
{
    ImGui::PushID(id);

    ImGui::PushFont(fontMedium);
    ImGui::TextUnformatted(label);
    ImGui::PopFont();
    ImGui::PushFont(fontSmall);

    if (subtext && subtext[0] != '\0')
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.65f, 0.65f, 0.65f, 1.0f));
        ImGui::TextWrapped("%s", subtext);
        ImGui::PopStyleColor();
    }
    ImGui::PopFont();

    ImGui::Spacing();

    ImVec4 bg        = ImVec4(0.14f, 0.14f, 0.14f, 1.0f);
    ImVec4 bgHover   = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    ImVec4 bgActive  = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
    ImVec4 border    = ImVec4(0.25f, 0.45f, 1.00f, 1.0f);
    ImVec4 borderDim = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_FrameBg,        bg);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bgHover);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  bgActive);
    ImGui::PushStyleColor(ImGuiCol_Border,         borderDim);
    ImGui::PushStyleColor(ImGuiCol_BorderShadow,   ImVec4(0,0,0,0));


    ImGui::PushItemWidth(-1);
    ImGui::InputTextWithHint("##input", placeholder, buffer, bufferSize);
    ImGui::PopItemWidth();

    // Hover cursor
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);

    // Active border highlight
    if (ImGui::IsItemActive())
        ImGui::GetStyle().Colors[ImGuiCol_Border] = border;

    // Restore
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(3);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PopID();
}

void OptionRow(const char* id, const char* title, const char* desc, int* currentIndex, const char* const* options, int optionCount, ImFont* fontXS)
{
    ImGui::PushID(id);

    float fullWidth     = ImGui::GetContentRegionAvail().x;
    float rowHeight     = 76.0f;

    // Full-row click zone
    ImVec2 rowStart = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##row_click", ImVec2(fullWidth, rowHeight));
        
    bool rowClicked = ImGui::IsItemClicked();
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // Reset cursor to draw content on top
    ImGui::SetCursorScreenPos(rowStart);

    // LEFT — title
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
    ImGui::TextUnformatted(title);

    // LEFT — description
    if (desc && desc[0] != '\0')
    {
        ImGui::PushFont(fontXS);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 1);
        ImGui::TextWrapped(desc);

        ImGui::PopStyleColor();
        ImGui::PopFont();
    }
    ImGui::PushFont(fontXS);

    // Measure text width
    ImVec2 textSize = ImGui::CalcTextSize(options[*currentIndex]);

    // Add padding (left + right)
    float dynamicWidth = textSize.x + 20.0f;

    // RIGHT — selector (dynamic width)
    float selectorX = rowStart.x + fullWidth - dynamicWidth - 10.0f;
    float selectorY = rowStart.y - 2.0f;

    ImGui::SetCursorScreenPos(ImVec2(selectorX, selectorY));

    // Draw selector button with dynamic width
    if (ImGui::Button(options[*currentIndex], ImVec2(dynamicWidth, 0)))
        ImGui::OpenPopup("popup");

    ImGui::PopFont();


    // Clicking the row also opens the popup
    if (rowClicked)
        ImGui::OpenPopup("popup");
    // POPUP WINDOW ABOVE — with rounded corners
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 8.0f);   // roundness
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10)); // nicer padding
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    if (ImGui::BeginPopup("popup"))
    {
        for (int i = 0; i < optionCount; i++)
        {
            bool selected = (*currentIndex == i);
            if (ImGui::Selectable(options[i], selected))
            {
                *currentIndex = i;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);

    ImGui::PopID();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
}

bool ModernButton(const char* label, float height = 40.0f)
{
    ImVec4 normal = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    ImVec4 hover  = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);
    ImVec4 active = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
    ImGui::PushStyleColor(ImGuiCol_Button, normal);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);

    bool clicked = ImGui::Button(label, ImVec2(ImGui::GetContentRegionAvail().x, height));

    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    return clicked;
}

// Main entry point
int main()
{
    FreeConsole();
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // No title bar
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // Transparent framebuffer
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // --- GET SCREEN SIZE BEFORE CREATING WINDOW ---
    RECT screen;
    GetWindowRect(GetDesktopWindow(), &screen);

    int screenWidth  = screen.right  - screen.left;
    int screenHeight = screen.bottom - screen.top - 1; // taskbar height compensation (optional)

    // --- CREATE FULLSCREEN-SIZED WINDOW ---
    GLFWwindow* window = glfwCreateWindow(
        screenWidth,
        screenHeight,
        "CORSPRITE client",
        NULL,
        NULL
    );

    if (!window)
        return -1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Win32 handle
    HWND hwnd = glfwGetWin32Window(window);
    gHwnd = hwnd;

    // Always on top
    SetWindowPos(
        hwnd,
        HWND_TOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE
    );

    // Transparent layered overlay window, hidden from taskbar/Alt-Tab
    LONG ex = GetWindowLong(hwnd, GWL_EXSTYLE);
    ex |= WS_EX_TOOLWINDOW;   // hide from Alt-Tab + taskbar
    ex &= ~WS_EX_APPWINDOW;   // prevent taskbar button
    ex |= WS_EX_LAYERED;      // transparency
    ex |= WS_EX_TOPMOST;      // always on top
    SetWindowLong(hwnd, GWL_EXSTYLE, ex);
    SetLayeredWindowAttributes(hwnd, RGB(0,0,0), 255, LWA_ALPHA);

    // Subclass the window for custom hit-testing
    g_OriginalWndProc = (WNDPROC)SetWindowLongPtr(
        hwnd,
        GWLP_WNDPROC,
        (LONG_PTR)OverlayWndProc
    );
    // Initialize microphones
    InitMicrophones();
    AutoSelectMicOnStart();

    // --- INIT IMGUI ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImFont* fontSmallest  = io.Fonts->AddFontFromFileTTF("fonts/NotoSans-VariableFont_wdth,wght.ttf", 12.0f);
    ImFont* fontSmall     = io.Fonts->AddFontFromFileTTF("fonts/NotoSans-VariableFont_wdth,wght.ttf", 18.0f);
    ImFont* fontMedium    = io.Fonts->AddFontFromFileTTF("fonts/NotoSans-VariableFont_wdth,wght.ttf", 22.0f);
    ImFont* fontLarge     = io.Fonts->AddFontFromFileTTF("fonts/NotoSans-VariableFont_wdth,wght.ttf", 38.0f);
    
    io.FontDefault = fontMedium;

    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding   = 6.0f;
    s.FrameRounding    = 5.0f;
    s.GrabRounding     = 5.0f;
    s.WindowBorderSize = 0.0f;
    s.Colors[ImGuiCol_WindowBg].w = 0.90f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // UI state
    ImVec4 circleColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);
    float radius = 3.0f;
    
    float alertHeight = screen.bottom - screen.top - 360.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Image for logo
    GLuint myImageLogoTexture = 0;
    int imgWidth = 0, imgHeight = 0;

    {
        int channels;
        unsigned char* data = stbi_load("images/logo.png", &imgWidth, &imgHeight, &channels, 4);

        glGenTextures(1, &myImageLogoTexture);
        glBindTexture(GL_TEXTURE_2D, myImageLogoTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    
    // Image for banner
    GLuint myBannerTexture = 0;
    int bannerWidth = 0, bannerHeight = 0;

    {
        int channels;
        unsigned char* data = stbi_load("images/banner_main.jpg", &bannerWidth, &bannerHeight, &channels, 4);

        glGenTextures(1, &myBannerTexture);
        glBindTexture(GL_TEXTURE_2D, myBannerTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bannerWidth, bannerHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }


    
    static bool enableAssistant   = true;
    static bool alwaysOnTop       = true;
    static bool showNotifications = true;
    static bool autoSaveConfig    = true;

    static bool interactiveMode = true;
    static bool followCursor    = false;
    static bool idleAnimations  = true;
    
    static int languageIndex = 0;

    
    static char username[128] = "";

    // Only load once at startup
    static bool loaded = false;
    if (!loaded)
    {
        DWORD size = UNLEN + 1;
        GetUserNameA(username, &size);
        loaded = true;
    }
    std::ifstream file("assistant_config.json");
    if (file.is_open())
    {
        json config;
        file >> config;
        file.close();

        // Json parsing with safety checks
        if (config.contains("assistant"))
        {
            auto& a = config["assistant"];

            if (a.contains("enable"))           enableAssistant   = a["enable"];
            if (a.contains("always_on_top"))    alwaysOnTop       = a["always_on_top"];
            if (a.contains("notifications"))    showNotifications = a["notifications"];
            if (a.contains("auto_save"))        autoSaveConfig    = a["auto_save"];

            if (a.contains("interactive_mode")) interactiveMode   = a["interactive_mode"];
            if (a.contains("follow_cursor"))    followCursor      = a["follow_cursor"];
            if (a.contains("idle_animations"))  idleAnimations    = a["idle_animations"];
            if (a.contains("language"))  languageIndex    = a["language"];
            if (a.contains("username") && a["username"].is_string())
            {
                std::string uname = a["username"];
                strncpy(username, uname.c_str(), sizeof(username) - 1);
                username[sizeof(username) - 1] = '\0'; // ensure null-termination
            }

        }

        if (config.contains("indicator"))
        {
            auto& ind = config["indicator"];

            if (ind.contains("color") && ind["color"].is_array() && ind["color"].size() == 3)
            {
                circleColor.x = ind["color"][0];
                circleColor.y = ind["color"][1];
                circleColor.z = ind["color"][2];
            }

            if (ind.contains("radius")) radius = ind["radius"];
        }
    }
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // --- START IMGUI FRAME ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- LEFT PANEL (INTERACTIVE) ---
        ImGuiStyle& s = ImGui::GetStyle();

        // WINDOW SHAPE
        s.WindowRounding   = 18.0f;
        s.FrameRounding    = 10.0f;
        s.GrabRounding     = 10.0f;
        s.ChildRounding    = 14.0f;

        // PADDING
        s.WindowPadding    = ImVec2(20, 20);
        s.FramePadding     = ImVec2(12, 8);
        s.ItemSpacing      = ImVec2(12, 10);
        s.ItemInnerSpacing = ImVec2(8, 6);

        // BORDERS
        s.WindowBorderSize = 0.0f;
        s.FrameBorderSize  = 0.0f;

        // COLORS

        s.Colors[ImGuiCol_FrameBg]          = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
        s.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.25f, 0.25f, 0.25f, 0.90f);
        s.Colors[ImGuiCol_FrameBgActive]    = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

        s.Colors[ImGuiCol_Button]           = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
        s.Colors[ImGuiCol_ButtonHovered]    = ImVec4(0.30f, 0.30f, 0.30f, 0.90f);
        s.Colors[ImGuiCol_ButtonActive]     = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

        s.Colors[ImGuiCol_Header]           = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
        s.Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.35f, 0.35f, 0.35f, 0.90f);
        s.Colors[ImGuiCol_HeaderActive]     = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

        s.Colors[ImGuiCol_Separator]        = ImVec4(0.35f, 0.35f, 0.35f, 0.50f);
        s.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.70f);
        s.Colors[ImGuiCol_SeparatorActive]  = ImVec4(0.60f, 0.60f, 0.60f, 0.80f);

        s.Colors[ImGuiCol_Text]             = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        s.Colors[ImGuiCol_TextDisabled]     = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        ImGuiIO& io = ImGui::GetIO();

        // FULLSCREEN BANNER WINDOW
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        ImGui::Begin("BannerWindow", nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoBackground
        );

        // Window rectangle
        ImVec2 p0 = ImGui::GetWindowPos();
        ImVec2 p1 = ImVec2(p0.x + ImGui::GetWindowSize().x,
                        p0.y + ImGui::GetWindowSize().y);

        // Draw list
        ImDrawList* draw = ImGui::GetWindowDrawList();

        // Clip to window
        draw->PushClipRect(p0, p1, true);

        draw->AddImage(
            (void*)(intptr_t)myBannerTexture,
            p0, p1,
            ImVec2(0, 0), ImVec2(1, 1),
            IM_COL32(255, 255, 255, 0.3f * 255)
        );

        // Add black overlay (0.4 = 40% darkness)
        draw->AddRectFilled(
            p0, p1,
            IM_COL32(0, 0, 0, (int)(0.4f * 255))
        );


        draw->PopClipRect();

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(360, io.DisplaySize.y - 40), ImGuiCond_Always);

        ImGui::Begin("MainUI", NULL,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar
        );
        // Logo
        ImGui::Image(
            (void*)(intptr_t)myImageLogoTexture,
            ImVec2(60, 60)
        );

        ImGui::SameLine();
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 14);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 20);

        ImGui::PushFont(fontLarge);
        ImGui::Text("ORSPRITE");
        ImGui::PopFont();

        ImGui::Separator();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 4));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
        
        ImGui::BeginChild("Windowcontaindata", ImVec2(0, alertHeight), true);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.30f, 0.00f, 0.90f));
        ImGui::BeginChild("AlertBox", ImVec2(0, 76), true);

            ImGui::PushFont(fontSmall);
            ImGui::TextWrapped("Welcome to CORSPRITE!");
            ImGui::TextWrapped("Please read the documentation before using the assistant.");

            ImGui::PopFont();


        ImGui::EndChild();
        ImGui::Spacing();
        if (ModernButton("View Documentation"))
        {
            OpenURL("https://corsprite-docs.vercel.app/");
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
        ImGui::Spacing();
        // Roberto language options
        const char* languageOptions[] =
        {
            "English",
            "Français",
            "Deutsch"
        };

        OptionRow(
            "assistant_language",
            "Language",
            "Select the language used for CORSPRITE assistant responses.",
            &languageIndex,
            languageOptions,
            IM_ARRAYSIZE(languageOptions),
            fontSmall
        );
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        InputRow(
            "username_row",
            "Username",
            "How should your name be called?",
            username,
            128,
            "Your username...",
            fontSmall,
            fontMedium
        );
        ImGui::Spacing();

        // Shared toggle colors
        ImVec4 onColor  = ImVec4(0.10f, 0.45f, 1.00f, 1.0f);
        ImVec4 offColor = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
        ImVec4 knob     = ImVec4(1, 1, 1, 1);

        // ASSISTANT — CORE SETTINGS
        ImGui::Separator();
        ImGui::PushFont(fontMedium);
        ImGui::Text("Assistant");
        ImGui::PopFont();

        ImGui::PushFont(fontSmall);
        ImGui::Text("Global behavior and visibility.");
        ImGui::PopFont();
        SettingsRow(
            "assistant_enable",
            "Enable Assistant",
            "Turns the on‑screen AI assistant on or off.",
            &enableAssistant,
            fontSmall, onColor, offColor, knob
        );

        SettingsRow(
            "assistant_topmost",
            "Always On Top",
            "Keeps the assistant visible above all other windows and games.",
            &alwaysOnTop,
            fontSmall, onColor, offColor, knob
        );

        SettingsRow(
            "assistant_notifications",
            "Notifications",
            "Shows alerts, reminders, and assistant responses.",
            &showNotifications,
            fontSmall, onColor, offColor, knob
        );

        SettingsRow(
            "assistant_autosave",
            "Auto‑Save Settings",
            "Automatically saves configuration changes as you make them.",
            &autoSaveConfig,
            fontSmall, onColor, offColor, knob
        );


        // ASSISTANT — INTERACTION & BEHAVIOR
        
        ImGui::Separator();
        ImGui::PushFont(fontMedium);
        ImGui::Text("Interaction & Behavior");
        ImGui::PopFont();

        ImGui::PushFont(fontSmall);
        ImGui::Text("Customize how the assistant reacts.");
        ImGui::PopFont();

        SettingsRow(
            "assistant_interactive",
            "Interactive Mode",
            "Allows clicking and interacting directly with the assistant.",
            &interactiveMode,
            fontSmall, onColor, offColor, knob
        );

        SettingsRow(
            "assistant_follow_cursor",
            "Follow Cursor",
            "Makes the assistant react to or follow your mouse movement.",
            &followCursor,
            fontSmall, onColor, offColor, knob
        );

        SettingsRow(
            "assistant_idle_anim",
            "Idle Animations",
            "Plays subtle animations when the assistant is idle.",
            &idleAnimations,
            fontSmall, onColor, offColor, knob
        );

        ImGui::Separator();

        ImGui::PushFont(fontMedium);
        ImGui::Text("Indicator (TOP RIGHT)");
        ImGui::PopFont();

        ImGui::PushFont(fontSmall);
        ImGui::Text("Enabled when corsprite is on.");
        ImGui::PopFont();

        ImGui::PushFont(fontSmall);
        ImGui::ColorEdit3("Color", (float*)&circleColor);
        ImGui::SliderFloat("Radius", &radius, 1.0f, 100.0f);
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);


        ImVec4 baseColor   = ImVec4(0.10f, 0.45f, 0.90f, 1.00f);
        ImVec4 hoverColor  = ImVec4(0.15f, 0.55f, 1.00f, 1.00f);
        ImVec4 activeColor = ImVec4(0.05f, 0.35f, 0.80f, 1.00f);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14, 10));
        ImGui::PushStyleColor(ImGuiCol_Button,        baseColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  activeColor);

        ImGui::PushFont(fontMedium);
        bool pressed = ImGui::Button("Apply & Start Client", ImVec2(-1, 45));
        ImGui::PopFont();

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);

        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        if (pressed)
        {
            json config;

            // Json structure 3
            config["assistant"]["enable"]          = enableAssistant;
            config["assistant"]["always_on_top"]   = alwaysOnTop;
            config["assistant"]["notifications"]   = showNotifications;
            config["assistant"]["auto_save"]       = autoSaveConfig;

            config["assistant"]["interactive_mode"] = interactiveMode;
            config["assistant"]["follow_cursor"]    = followCursor;
            config["assistant"]["idle_animations"]  = idleAnimations;
            config["assistant"]["language"]               = languageIndex;
            config["assistant"]["username"]               = std::string(username);

            config["indicator"]["color"] = {
                circleColor.x,
                circleColor.y,
                circleColor.z
            };

            config["indicator"]["radius"] = radius;

            // SAVE TO FILE
            std::ofstream file("assistant_config.json");
            if (file.is_open())
            {
                file << config.dump(4); // pretty print with 4‑space indent
                file.close();
            }

            // Close window after saving
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        
        // Quit button (styled to match your UI)
        ImVec4 quitBase   = ImVec4(0.25f, 0.25f, 0.25f, 0.85f);
        ImVec4 quitHover  = ImVec4(0.35f, 0.35f, 0.35f, 0.95f);
        ImVec4 quitActive = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
        ImGui::PushStyleColor(ImGuiCol_Button,        quitBase);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, quitHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  quitActive);

        ImGui::PushFont(fontMedium);

        // Colors
        ImVec4 normal = ImVec4(0.75f, 0.15f, 0.15f, 1.0f);   // red
        ImVec4 hover  = ImVec4(0.90f, 0.20f, 0.20f, 1.0f);   // brighter red
        ImVec4 active = ImVec4(0.60f, 0.10f, 0.10f, 1.0f);   // darker red

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 10));
        ImGui::PushStyleColor(ImGuiCol_Button, normal);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);

        bool quitPressed = ImGui::Button("Cancel & Quit", ImVec2(-1, 40));

        if (ImGui::IsItemHovered())
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);

        ImGui::PopFont();


        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
        if (quitPressed)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui::Spacing();
        ImGui::Spacing();

        // Footer text (centered)
        ImGui::PushFont(fontMedium);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("© 2026 CORSPRITE").x) * 0.5f);
        ImGui::Text("© 2026 CORSPRITE");
        ImGui::PopFont();

        ImGui::PushFont(fontSmallest);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Version Alpha 0.1").x) * 0.5f);
        ImGui::Text("Version Alpha 0.1");
        ImGui::PopFont();

        ImGui::End();

        
        DrawPerformanceTool(fontMedium, fontLarge);
        CorspriteCharacterWidget(fontMedium, fontLarge);
        CorspriteBottomCarousel(fontMedium);


        // --- DRAW CIRCLE (STATUS DOT) ---
        ImVec2 center(
            io.DisplaySize.x - radius - 10.0f,
            radius + 10.0f
        );

        ImGui::GetForegroundDrawList()->AddCircleFilled(
            center,
            radius,
            ImColor(circleColor),
            64
        );

        ImGui::Render();

        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0, 0, 0, 0.6f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
