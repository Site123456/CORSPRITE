#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <stdexcept>
#include <stdio.h>

// Vulkan test
static VkInstance g_Instance;
static VkPhysicalDevice g_PhysicalDevice;
static VkDevice g_Device;
static uint32_t g_QueueFamily;
static VkQueue g_Queue;
static VkSurfaceKHR g_Surface;

static VkSwapchainKHR g_Swapchain;
static VkFormat g_SwapchainFormat;
static std::vector<VkImage> g_SwapchainImages;
static std::vector<VkImageView> g_SwapchainImageViews;

static VkRenderPass g_RenderPass;
static std::vector<VkFramebuffer> g_Framebuffers;

static VkCommandPool g_CommandPool;
static std::vector<VkCommandBuffer> g_CommandBuffers;

static VkDescriptorPool g_DescriptorPool;

static VkSemaphore g_SemaphoreImageAcquired;
static VkSemaphore g_SemaphoreRenderComplete;
static VkFence g_Fence;

static int g_Width = 1280;
static int g_Height = 720;

// Helpers
static void Check(VkResult r) {
    if (r != VK_SUCCESS) throw std::runtime_error("Vulkan error");
}

// Vulkan setup
static void CreateInstance() {
    VkApplicationInfo app = {};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = "Vulkan ImGui";
    app.apiVersion = VK_API_VERSION_1_1;

    uint32_t extCount;
    const char** ext = glfwGetRequiredInstanceExtensions(&extCount);

    VkInstanceCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &app;
    ci.enabledExtensionCount = extCount;
    ci.ppEnabledExtensionNames = ext;

    Check(vkCreateInstance(&ci, nullptr, &g_Instance));
}

static void PickGPU() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(g_Instance, &count, nullptr);
    std::vector<VkPhysicalDevice> gpus(count);
    vkEnumeratePhysicalDevices(g_Instance, &count, gpus.data());
    g_PhysicalDevice = gpus[0];
}

static void CreateDevice() {
    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> props(count);
    vkGetPhysicalDeviceQueueFamilyProperties(g_PhysicalDevice, &count, props.data());

    for (uint32_t i = 0; i < count; i++)
        if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            g_QueueFamily = i;

    float priority = 1.0f;
    VkDeviceQueueCreateInfo q = {};
    q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    q.queueFamilyIndex = g_QueueFamily;
    q.queueCount = 1;
    q.pQueuePriorities = &priority;

    const char* ext[] = { "VK_KHR_swapchain" };

    VkDeviceCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos = &q;
    ci.enabledExtensionCount = 1;
    ci.ppEnabledExtensionNames = ext;

    Check(vkCreateDevice(g_PhysicalDevice, &ci, nullptr, &g_Device));
    vkGetDeviceQueue(g_Device, g_QueueFamily, 0, &g_Queue);
}

static void CreateSurface(GLFWwindow* window) {
    Check(glfwCreateWindowSurface(g_Instance, window, nullptr, &g_Surface));
}

static void CreateSwapchain() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_PhysicalDevice, g_Surface, &caps);

    g_Width = caps.currentExtent.width;
    g_Height = caps.currentExtent.height;

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_PhysicalDevice, g_Surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g_PhysicalDevice, g_Surface, &formatCount, formats.data());

    g_SwapchainFormat = formats[0].format;

    VkSwapchainCreateInfoKHR ci = {};
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = g_Surface;
    ci.minImageCount = caps.minImageCount + 1;
    ci.imageFormat = g_SwapchainFormat;
    ci.imageColorSpace = formats[0].colorSpace;
    ci.imageExtent = caps.currentExtent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform = caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    ci.clipped = VK_TRUE;

    Check(vkCreateSwapchainKHR(g_Device, &ci, nullptr, &g_Swapchain));

    uint32_t count;
    vkGetSwapchainImagesKHR(g_Device, g_Swapchain, &count, nullptr);
    g_SwapchainImages.resize(count);
    vkGetSwapchainImagesKHR(g_Device, g_Swapchain, &count, g_SwapchainImages.data());
}

static void CreateImageViews() {
    g_SwapchainImageViews.resize(g_SwapchainImages.size());

    for (size_t i = 0; i < g_SwapchainImages.size(); i++) {
        VkImageViewCreateInfo ci = {};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.image = g_SwapchainImages[i];
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = g_SwapchainFormat;
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.levelCount = 1;
        ci.subresourceRange.layerCount = 1;

        Check(vkCreateImageView(g_Device, &ci, nullptr, &g_SwapchainImageViews[i]));
    }
}

static void CreateRenderPass() {
    VkAttachmentDescription att = {};
    att.format = g_SwapchainFormat;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ref = {};
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub = {};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &ref;

    VkRenderPassCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    ci.attachmentCount = 1;
    ci.pAttachments = &att;
    ci.subpassCount = 1;
    ci.pSubpasses = &sub;

    Check(vkCreateRenderPass(g_Device, &ci, nullptr, &g_RenderPass));
}

static void CreateFramebuffers() {
    g_Framebuffers.resize(g_SwapchainImageViews.size());

    for (size_t i = 0; i < g_SwapchainImageViews.size(); i++) {
        VkImageView att[] = { g_SwapchainImageViews[i] };

        VkFramebufferCreateInfo ci = {};
        ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ci.renderPass = g_RenderPass;
        ci.attachmentCount = 1;
        ci.pAttachments = att;
        ci.width = g_Width;
        ci.height = g_Height;
        ci.layers = 1;

        Check(vkCreateFramebuffer(g_Device, &ci, nullptr, &g_Framebuffers[i]));
    }
}

static void CreateCommandPool() {
    VkCommandPoolCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    ci.queueFamilyIndex = g_QueueFamily;

    Check(vkCreateCommandPool(g_Device, &ci, nullptr, &g_CommandPool));
}

static void CreateCommandBuffers() {
    g_CommandBuffers.resize(g_Framebuffers.size());

    VkCommandBufferAllocateInfo ai = {};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = g_CommandPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = (uint32_t)g_CommandBuffers.size();

    Check(vkAllocateCommandBuffers(g_Device, &ai, g_CommandBuffers.data()));
}

static void CreateSync() {
    VkSemaphoreCreateInfo si = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    Check(vkCreateSemaphore(g_Device, &si, nullptr, &g_SemaphoreImageAcquired));
    Check(vkCreateSemaphore(g_Device, &si, nullptr, &g_SemaphoreRenderComplete));

    VkFenceCreateInfo fi = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    Check(vkCreateFence(g_Device, &fi, nullptr, &g_Fence));
}

static void CreateDescriptorPool() {
    VkDescriptorPoolSize pool = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 };

    VkDescriptorPoolCreateInfo ci = {};
    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    ci.maxSets = 1000;
    ci.poolSizeCount = 1;
    ci.pPoolSizes = &pool;

    Check(vkCreateDescriptorPool(g_Device, &ci, nullptr, &g_DescriptorPool));
}

static void RecreateSwapchain(GLFWwindow* window) {
    vkDeviceWaitIdle(g_Device);

    for (auto fb : g_Framebuffers) vkDestroyFramebuffer(g_Device, fb, nullptr);
    for (auto iv : g_SwapchainImageViews) vkDestroyImageView(g_Device, iv, nullptr);
    vkDestroySwapchainKHR(g_Device, g_Swapchain, nullptr);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    if (w == 0 || h == 0) return;

    CreateSwapchain();
    CreateImageViews();
    CreateFramebuffers();
    CreateCommandBuffers();
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Vulkan ImGui", nullptr, nullptr);

    CreateInstance();
    CreateSurface(window);
    PickGPU();
    CreateDevice();
    CreateSwapchain();
    CreateImageViews();
    CreateRenderPass();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateSync();
    CreateDescriptorPool();

    // ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo info = {};
    info.Instance = g_Instance;
    info.PhysicalDevice = g_PhysicalDevice;
    info.Device = g_Device;
    info.QueueFamily = g_QueueFamily;
    info.Queue = g_Queue;
    info.DescriptorPool = g_DescriptorPool;
    info.MinImageCount = 2;
    info.ImageCount = (uint32_t)g_SwapchainImages.size();

    ImGui_ImplVulkan_Init(&info);

    // UI state
    ImVec4 color = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        vkWaitForFences(g_Device, 1, &g_Fence, VK_TRUE, UINT64_MAX);
        vkResetFences(g_Device, 1, &g_Fence);

        uint32_t index;
        VkResult r = vkAcquireNextImageKHR(g_Device, g_Swapchain, UINT64_MAX, g_SemaphoreImageAcquired, VK_NULL_HANDLE, &index);
        if (r == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapchain(window);
            continue;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // UI window
        ImGui::Begin("Outils");
        ImGui::Text("Choisis une couleur :");
        ImGui::ColorEdit3("Couleur", (float*)&color);
        ImGui::End();

        // Circle
        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::GetForegroundDrawList()->AddCircleFilled(center, 80.0f, ImColor(color), 64);

        ImGui::Render();

        VkCommandBuffer cmd = g_CommandBuffers[index];

        VkCommandBufferBeginInfo bi = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkBeginCommandBuffer(cmd, &bi);

        VkClearValue clear;
        clear.color.float32[0] = color.x;
        clear.color.float32[1] = color.y;
        clear.color.float32[2] = color.z;
        clear.color.float32[3] = 1.0f;

        VkRenderPassBeginInfo rp = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        rp.renderPass = g_RenderPass;
        rp.framebuffer = g_Framebuffers[index];
        rp.renderArea.extent.width = g_Width;
        rp.renderArea.extent.height = g_Height;
        rp.clearValueCount = 1;
        rp.pClearValues = &clear;

        vkCmdBeginRenderPass(cmd, &rp, VK_SUBPASS_CONTENTS_INLINE);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);

        VkPipelineStageFlags wait = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo si = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
        si.waitSemaphoreCount = 1;
        si.pWaitSemaphores = &g_SemaphoreImageAcquired;
        si.pWaitDstStageMask = &wait;
        si.commandBufferCount = 1;
        si.pCommandBuffers = &cmd;
        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores = &g_SemaphoreRenderComplete;

        vkQueueSubmit(g_Queue, 1, &si, g_Fence);

        VkPresentInfoKHR pi = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores = &g_SemaphoreRenderComplete;
        pi.swapchainCount = 1;
        pi.pSwapchains = &g_Swapchain;
        pi.pImageIndices = &index;

        r = vkQueuePresentKHR(g_Queue, &pi);
        if (r == VK_ERROR_OUT_OF_DATE_KHR)
            RecreateSwapchain(window);
    }

    vkDeviceWaitIdle(g_Device);
    return 0;
}
