// P_OVERRIDE_ACTIVE_CONFIG=remote monado-service

#include <SDL3/SDL.h>
// SPDX-License-Identifier: BSL-1.0
/*!
 * @author Adrian Przekwas <adrian.v.przekwas@gmail.com>
 */

#include "data_sender.h"
#include "structs.h"
#include "movement.h"
#include "math_helper.h"
#include "main_window.h"
#include "settings.h"

#include <SDL3/SDL_main.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#include <iostream>
#include <memory>
#include <unistd.h>
#include <vector>

int main()
{
    auto config_dir = getConfigDir();
    auto config = loadConfig(config_dir);

    std::vector<SDL_Gamepad*> gamepads;

    /* SDL window part */
    bool running = true;
    Uint64 old_ticks = 0;
    int width = 640;
    int height = 300;
    float base_mouse_x = 0.0f;
    float base_mouse_y = 0.0f;

    bool mouse_kb_grabbed = false; // if not grabbed ImGui will get events, otherwise they will be used to change movement

    InputConsumer inputConsumer;
    inputConsumer = hmd;

    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)){
        std::cerr << "SDL could not initialize! SDL error: " << SDL_GetError() << std::endl;
        return -1;
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow("remote-mndset", (int)(width * main_scale), (int)(height * main_scale), window_flags);

    if (window == nullptr){
        std::cerr << "Window could not be created! SDL error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_SetWindowAlwaysOnTop(window, true);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Create GPU Device
    SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB,true,nullptr);
    if (gpu_device == nullptr)
    {
        std::cerr << "Error: SDL_CreateGPUDevice(): " << SDL_GetError() << std::endl;
        return -1;
    }
    // Claim window for GPU Device
    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window))
    {
        std::cerr << "Error: SDL_ClaimWindowForGPUDevice(): " << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_SetGPUSwapchainParameters(gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

    /* ImGui graphical interface part */
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLGPU(window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = gpu_device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    ImGui_ImplSDLGPU3_Init(&init_info);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    /* Movement objects and settings */
    std::unique_ptr<Movement> hmdMov = std::make_unique<Movement>(); // HMD movement
    std::unique_ptr<Movement> leftMov = std::make_unique<Movement>(); // left controller
    std::unique_ptr<Movement> rightMov = std::make_unique<Movement>(); // right controller
    hmdMov->updateConfigValues(config.hmd_lin_vel, config.hmd_ang_vel,
                               config.mouse_sens, config.gamepad_axis_sens, config.gamepad_dead_zone);
    leftMov->updateConfigValues(config.controller_lin_vel, config.controller_ang_vel,
                                config.mouse_sens, config.gamepad_axis_sens, config.gamepad_dead_zone);
    rightMov->updateConfigValues(config.controller_lin_vel, config.controller_ang_vel,
                                 config.mouse_sens, config.gamepad_axis_sens, config.gamepad_dead_zone);

    /* TCP data part */
    std::unique_ptr<DataSender> dataSender = std::make_unique<DataSender>();
    r_remote_data data{};
    r_remote_data old_data{}; // for velocity calculation
    // set some valid value for starting orientation
    const xrt_quat default_quat = {0.0f, 0.0f, 0.0f, 1.0f};

    // HMD
    data.head.center.orientation = default_quat;
    // starting HMD position for human standing a few steps back
    // X+ Right, Y+ Up, Z+ to camera
    data.head.center.position = {0.0f, 1.7f, 1.0f};

    // controllers, poses relative to HMD
    xrt_pose left_rel, right_rel;
    left_rel.orientation = default_quat;
    left_rel.position = {-0.3f, -0.3f, -0.4f};
    right_rel.orientation = default_quat;
    right_rel.position = {0.3f, -0.3f, -0.4f};
    // final poses
    data.left.pose.orientation = default_quat;
    data.right.pose.orientation = default_quat;
    data.left.active = true;
    data.right.active =  true;

    while (running) {
        /* SDL event and data sending*/

        // It seems that SDL_WarpMouseInWindow breaks V-sync, this forces loop to be at least 5 ms
        while (SDL_GetTicks() - old_ticks < 5) {
            SDL_Delay(1);
        }
        Uint64 ticks = SDL_GetTicks();
        old_ticks = ticks; // End of the workaround

        hmdMov->updateTicks(ticks);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {  // poll until all events are handled

            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED){
                running = false;
            }
            else if (event.type == SDL_EVENT_GAMEPAD_ADDED) {
                int device_index = event.gdevice.which;
                SDL_Gamepad* newGamepad = SDL_OpenGamepad(device_index);
                if (newGamepad) {
                    gamepads.push_back(newGamepad);
                    std::cout << "Gamepad connected: " << SDL_GetGamepadName(newGamepad)
                              << " (ID: " << device_index << ")" << std::endl;
                }
            } else if (event.type == SDL_EVENT_GAMEPAD_REMOVED) {
                int instance_id = event.gdevice.which;
                for (auto it = gamepads.begin(); it != gamepads.end(); ++it) {
                    if (SDL_GetGamepadID(*it) == instance_id) {
                        std::cout << "Gamepad disconnected " << SDL_GetGamepadName(*it) << std::endl;
                        SDL_CloseGamepad(*it);
                        gamepads.erase(it);
                        break;
                    }
                }
            }
            else {
                // ImGui will get acces to keyboard and mouse after the Esc key is hit
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.key == SDLK_ESCAPE) {
                        mouse_kb_grabbed = false;
                    }
                }
                if (mouse_kb_grabbed){
                    SDL_Keymod mod = SDL_GetModState();  // Get the current key modifier state for the keyboard (SHIFT, CTRL etc.)
                    if (mod & SDL_KMOD_LSHIFT) {
                        leftMov->passKeyboardEvent(event);
                        inputConsumer = left_controller;
                    } else if (mod & SDL_KMOD_LALT){
                        rightMov->passKeyboardEvent(event);
                        inputConsumer = right_controller;
                    } else {
                        inputConsumer = hmd;
                        hmdMov->passKeyboardEvent(event);
                    }
                } else {
                    ImGui_ImplSDL3_ProcessEvent(&event);
                }
            }
        }

        if (mouse_kb_grabbed) {
            float x_rel, y_rel;
            SDL_MouseButtonFlags mouse_flags = SDL_GetRelativeMouseState(&x_rel, &y_rel);
            switch (inputConsumer) {
            case hmd:
                hmdMov->passMouseRelativePos(x_rel, y_rel);
                break;
            case left_controller:
                data.left.a_click = (mouse_flags & SDL_BUTTON_MASK(SDL_BUTTON_X1)) != 0;
                data.left.b_click = (mouse_flags & SDL_BUTTON_MASK(SDL_BUTTON_X2)) != 0;
                leftMov->passMouseRelativePos(x_rel, y_rel);
                break;
            case right_controller:
                data.right.a_click = (mouse_flags & SDL_BUTTON_MASK(SDL_BUTTON_X1)) != 0;
                data.right.b_click = (mouse_flags & SDL_BUTTON_MASK(SDL_BUTTON_X2)) != 0;
                rightMov->passMouseRelativePos(x_rel, y_rel);
                break;
            }
            // left mouse button simulates left controller trigger, right, right controller trigger
            if (mouse_flags & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
                data.left.trigger_value = xrt_vec1 {1.0f};
                data.left.trigger_click = true;
            } else {
                data.left.trigger_value = xrt_vec1 {0.0f};
                data.left.trigger_click = false;
            }
            if (mouse_flags & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
                data.right.trigger_value = xrt_vec1 {1.0f};
                data.right.trigger_click = true;
            } else {
                data.right.trigger_value = xrt_vec1 {0.0f};
                data.right.trigger_click = false;
            }

            SDL_WarpMouseInWindow(window, base_mouse_x, base_mouse_y);
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }

        // if user decides to use keyboard and mouse, disable gamepad input
        if (!gamepads.empty() && !mouse_kb_grabbed) {
            SDL_Gamepad* gamepad = gamepads[0];
            if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)) {
                inputConsumer = left_controller;
            } else if (SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) {
                inputConsumer = right_controller;
            } else {
                inputConsumer = hmd;
            }
            switch (inputConsumer) {
            case hmd:
                hmdMov->passGamepadState(*gamepad);
                break;
            case left_controller:
                data.left.a_click = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
                data.left.b_click = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST);
                leftMov->passGamepadState(*gamepad);
                break;
            case right_controller:
                data.right.a_click = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_SOUTH);
                data.right.b_click = SDL_GetGamepadButton(gamepad, SDL_GAMEPAD_BUTTON_EAST);
                rightMov->passGamepadState(*gamepad);
                break;
            }
            float left_trig = static_cast<float>(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER)) / gamepad_axis_range;
            float right_trig = static_cast<float>(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER)) / gamepad_axis_range;
            data.left.trigger_value = xrt_vec1 {left_trig};
            if (left_trig > gamepad_click_threshold) {
                data.left.trigger_click = true;
            } else {
                data.left.trigger_click = false;
            }
            data.right.trigger_value = xrt_vec1 {right_trig};
            if (right_trig > gamepad_click_threshold) {
                data.right.trigger_click = true;
            } else {
                data.right.trigger_click = false;
            }
        }
        // HMD, center is a xrt_pose
        hmdMov->updatePose(data.head.center);

        // controllers following HMD
        leftMov->updatePose(left_rel);
        data.left.pose = poseMult(data.head.center, left_rel);
        leftMov->updateVelocity(data.left.pose, old_data.left.pose,
                                data.left.linear_velocity, data.left.angular_velocity);
        rightMov->updatePose(right_rel);
        data.right.pose = poseMult(data.head.center, right_rel);
        rightMov->updateVelocity(data.right.pose, old_data.right.pose,
                                data.right.linear_velocity, data.right.angular_velocity);

        /* Sending all the data */
        dataSender->sendData(data);

        /* ImGui rendering */

        // Start the Dear ImGui frame
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        /* Main Window creation (every frame)*/
        WindowState w_state{};
        w_state.connect_button_clicked = dataSender->isSocketOpened();
        w_state.grab_button_clicked = mouse_kb_grabbed;
        w_state.iCons = inputConsumer;
        w_state.config = config;
        if (!gamepads.empty()) {
            w_state.has_gamepad = true;
            w_state.gamepad_name = SDL_GetGamepadName(gamepads[0]);
            int batt_percent;
            SDL_PowerState powerState = SDL_GetGamepadPowerInfo(gamepads[0], &batt_percent);
            if (powerState == SDL_POWERSTATE_ON_BATTERY || powerState == SDL_POWERSTATE_CHARGING
                || powerState == SDL_POWERSTATE_CHARGED){
                w_state.batt = batt_percent;
            }
        }

        drawMainWindow(w_state); // window with all widgets

        if (w_state.connect_button_clicked && !dataSender->isSocketOpened()) {
            dataSender->openSocket(w_state.config.server_ip);
        }
        if (!w_state.connect_button_clicked && dataSender->isSocketOpened()) {
            dataSender->closeSocket();
        }
        // mouse just grabbed, set where cursor will be locked
        if (!mouse_kb_grabbed && w_state.grab_button_clicked)   {
            SDL_GetMouseState(&base_mouse_x, &base_mouse_y);
            // read realitve position once and do nothing, to avoid position jump at start
            float x_rel, y_rel;
            SDL_GetRelativeMouseState(&x_rel, &y_rel);
        }
        mouse_kb_grabbed = w_state.grab_button_clicked;

        config = w_state.config;

        hmdMov->updateConfigValues(config.hmd_lin_vel, config.hmd_ang_vel,
                                   config.mouse_sens, config.gamepad_axis_sens, config.gamepad_dead_zone);
        leftMov->updateConfigValues(config.controller_lin_vel, config.controller_ang_vel,
                                    config.mouse_sens, config.gamepad_axis_sens, config.gamepad_dead_zone);
        rightMov->updateConfigValues(config.controller_lin_vel, config.controller_ang_vel,
                                     config.mouse_sens, config.gamepad_axis_sens, config.gamepad_dead_zone);

        old_data = data;

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device); // Acquire a GPU command buffer

        SDL_GPUTexture* swapchain_texture;
        SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, nullptr, nullptr); // Acquire a swapchain texture

        if (swapchain_texture != nullptr && !is_minimized) {
            // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
            ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

            // Setup and start a render pass
            SDL_GPUColorTargetInfo target_info = {};
            target_info.texture = swapchain_texture;
            target_info.clear_color = SDL_FColor { clear_color.x, clear_color.y, clear_color.z, clear_color.w };
            target_info.load_op = SDL_GPU_LOADOP_CLEAR;
            target_info.store_op = SDL_GPU_STOREOP_STORE;
            target_info.mip_level = 0;
            target_info.layer_or_depth_plane = 0;
            target_info.cycle = false;
            SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

            // Render ImGui
            ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);
            SDL_EndGPURenderPass(render_pass);
        }

        // Submit the command buffer
        SDL_SubmitGPUCommandBuffer(command_buffer);
    }

    /* Save configuration */
    saveConfig(config_dir, config);

    /* Terminate SDL and ImGui */
    for (const auto& gamepad : gamepads) {
        SDL_CloseGamepad(gamepad);
    }
    SDL_WaitForGPUIdle(gpu_device);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();
    SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
    SDL_DestroyGPUDevice(gpu_device);
    SDL_DestroyWindow(window);
    window = nullptr;
    SDL_Quit();

    return 0;
}

