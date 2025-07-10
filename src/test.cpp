#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <iostream>

const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 700;

struct Service {
    std::string label;
    std::string accountName;
    std::string password;
};

struct MultiInputResult {
    bool submitted;
    std::string serviceName;
    std::string accountName;
    std::string password;
};

MultiInputResult getMultipleTextInput(SDL_Renderer* renderer, TTF_Font* font, int maxLen = 20) {
    SDL_StartTextInput();

    std::string inputs[3] = { "", "", "" };
    const char* placeholders[3] = { "Service Name", "Account Name", "Password" };
    int activeInput = 0;
    bool done = false;
    bool canceled = false;

    SDL_Event e;

    SDL_Color boxColor = { 50, 50, 50, 255 };
    SDL_Color borderColor = { 255, 255, 255, 255 };
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Color placeholderColor = { 150, 150, 150, 255 };

    // Define rectangles for input boxes stacked vertically
    SDL_Rect inputRects[3] = {
        { 50, 220, 300, 50 },
        { 50, 290, 300, 50 },
        { 50, 360, 300, 50 }
    };

    while (!done && !canceled) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                canceled = true;
            }
            else if (e.type == SDL_TEXTINPUT) {
                if (inputs[activeInput].size() < (size_t)maxLen) {
                    inputs[activeInput] += e.text.text;
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !inputs[activeInput].empty()) {
                    inputs[activeInput].pop_back();
                }
                else if (e.key.keysym.sym == SDLK_RETURN) {
                    done = true;
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    canceled = true;
                }
                else if (e.key.keysym.sym == SDLK_TAB) {
                    activeInput = (activeInput + 1) % 3;
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw all input boxes with placeholders or input text
        for (int i = 0; i < 3; ++i) {
            // Fill box
            SDL_SetRenderDrawColor(renderer, boxColor.r, boxColor.g, boxColor.b, boxColor.a);
            SDL_RenderFillRect(renderer, &inputRects[i]);

            // Draw border (highlight if active)
            if (i == activeInput) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // yellow highlight
            } else {
                SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
            }
            SDL_RenderDrawRect(renderer, &inputRects[i]);

            // Render text or placeholder
            const std::string& textToRender = (inputs[i].empty()) ? placeholders[i] : inputs[i];
            SDL_Color colorToUse = (inputs[i].empty()) ? placeholderColor : textColor;

            std::string displayText = textToRender;
            // For password field (index 2), mask text with '*'
            if (i == 2 && !inputs[i].empty()) {
                displayText = std::string(inputs[i].size(), '*');
            }

            SDL_Surface* textSurf = TTF_RenderText_Blended(font, displayText.c_str(), colorToUse);
            SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
            SDL_Rect textRect = { inputRects[i].x + 5, inputRects[i].y + 10, textSurf->w, textSurf->h };
            SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
            SDL_FreeSurface(textSurf);
            SDL_DestroyTexture(textTex);
        }

        // Draw instruction at bottom
        const char* instruction = "Press Enter to submit, Esc to cancel, Tab to switch fields";
        SDL_Surface* instrSurf = TTF_RenderText_Blended(font, instruction, textColor);
        SDL_Texture* instrTex = SDL_CreateTextureFromSurface(renderer, instrSurf);
        SDL_Rect instrRect = { 50, 430, instrSurf->w, instrSurf->h };
        SDL_RenderCopy(renderer, instrTex, nullptr, &instrRect);
        SDL_FreeSurface(instrSurf);
        SDL_DestroyTexture(instrTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();

    if (canceled) {
        return { false, "", "", "" };
    } else {
        return { true, inputs[0], inputs[1], inputs[2] };
    }
}


// bool getTextInput(SDL_Renderer* renderer, TTF_Font* font, std::string& output, const std::string& prompt = "", int maxLen = 20) {
//     output.clear();
//     SDL_StartTextInput();

//     SDL_Event e;
//     bool done = false;

//     SDL_Rect inputRect = { 50, 300, 300, 50 };
//     SDL_Color boxColor = { 50, 50, 50, 255 };
//     SDL_Color borderColor = { 255, 255, 255, 255 };
//     SDL_Color textColor = { 255, 255, 255, 255 };

//     SDL_Rect promptRect = { 50, 250, 300, 30 };

//     while (!done) {
//         while (SDL_PollEvent(&e)) {
//             if (e.type == SDL_QUIT) {
//                 SDL_StopTextInput();
//                 return false;
//             }

//             if (e.type == SDL_TEXTINPUT && output.size() < (size_t)maxLen) {
//                 output += e.text.text;
//             }
//             else if (e.type == SDL_KEYDOWN) {
//                 if (e.key.keysym.sym == SDLK_BACKSPACE && !output.empty()) {
//                     output.pop_back();
//                 }
//                 else if (e.key.keysym.sym == SDLK_RETURN) {
//                     done = true;
//                 }
//                 else if (e.key.keysym.sym == SDLK_ESCAPE) {
//                     SDL_StopTextInput();
//                     return false;
//                 }
//             }
//         }

//         // Clear screen
//         SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
//         SDL_RenderClear(renderer);

//         // Draw prompt text
//         if (!prompt.empty()) {
//             SDL_Surface* promptSurf = TTF_RenderText_Blended(font, prompt.c_str(), textColor);
//             SDL_Texture* promptTex = SDL_CreateTextureFromSurface(renderer, promptSurf);
//             SDL_Rect textRect = { promptRect.x, promptRect.y, promptSurf->w, promptSurf->h };
//             SDL_RenderCopy(renderer, promptTex, nullptr, &textRect);
//             SDL_FreeSurface(promptSurf);
//             SDL_DestroyTexture(promptTex);
//         }

//         // Draw input box
//         SDL_SetRenderDrawColor(renderer, boxColor.r, boxColor.g, boxColor.b, boxColor.a);
//         SDL_RenderFillRect(renderer, &inputRect);

//         SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
//         SDL_RenderDrawRect(renderer, &inputRect);

//         // Render typed text
//         if (!output.empty()) {
//             SDL_Surface* textSurf = TTF_RenderText_Blended(font, output.c_str(), textColor);
//             SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
//             SDL_Rect textRect = { inputRect.x + 5, inputRect.y + 10, textSurf->w, textSurf->h };
//             SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
//             SDL_FreeSurface(textSurf);
//             SDL_DestroyTexture(textTex);
//         }

//         SDL_RenderPresent(renderer);
//         SDL_Delay(16);
//     }

//     SDL_StopTextInput();
//     return !output.empty();
// }


bool showServiceDetailsPopup(SDL_Renderer* renderer, TTF_Font* font, Service& service) {
    bool done = false;
    bool deleted = false;

    SDL_Event e;
    SDL_Rect popupRect = { 50, 200, 300, 200 };
    SDL_Rect deleteBtnRect = { popupRect.x + 30, popupRect.y + 140, 100, 40 };
    SDL_Rect copyBtnRect   = { popupRect.x + 170, popupRect.y + 140, 100, 40 };

    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    done = true;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                if (mx >= deleteBtnRect.x && mx <= deleteBtnRect.x + deleteBtnRect.w &&
                    my >= deleteBtnRect.y && my <= deleteBtnRect.y + deleteBtnRect.h) {
                    deleted = true;
                    done = true;
                }
                else if (mx >= copyBtnRect.x && mx <= copyBtnRect.x + copyBtnRect.w &&
                         my >= copyBtnRect.y && my <= copyBtnRect.y + copyBtnRect.h) {
                    SDL_SetClipboardText(service.password.c_str());  // <- Clipboard copy
                    done = true;
                }
                else if (mx < popupRect.x || mx > popupRect.x + popupRect.w ||
                         my < popupRect.y || my > popupRect.y + popupRect.h) {
                    done = true;
                }
            }
        }

        // Draw semi-transparent background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
        SDL_RenderFillRect(renderer, nullptr);

        // Draw popup box
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderFillRect(renderer, &popupRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &popupRect);

        SDL_Color white = { 255, 255, 255, 255 };

        // Service label
        std::string labelText = "Service: " + service.label;
        SDL_Surface* labelSurf = TTF_RenderText_Blended(font, labelText.c_str(), white);
        SDL_Texture* labelTex = SDL_CreateTextureFromSurface(renderer, labelSurf);
        SDL_Rect labelRect = { popupRect.x + 10, popupRect.y + 10, labelSurf->w, labelSurf->h };
        SDL_RenderCopy(renderer, labelTex, nullptr, &labelRect);
        SDL_FreeSurface(labelSurf);
        SDL_DestroyTexture(labelTex);

        // Account
        std::string accountText = "Account: " + service.accountName;
        SDL_Surface* accountSurf = TTF_RenderText_Blended(font, accountText.c_str(), white);
        SDL_Texture* accountTex = SDL_CreateTextureFromSurface(renderer, accountSurf);
        SDL_Rect accountRect = { popupRect.x + 10, popupRect.y + 50, accountSurf->w, accountSurf->h };
        SDL_RenderCopy(renderer, accountTex, nullptr, &accountRect);
        SDL_FreeSurface(accountSurf);
        SDL_DestroyTexture(accountTex);

        // Password
        std::string passwordText = "Password: " + service.password;
        SDL_Surface* passwordSurf = TTF_RenderText_Blended(font, passwordText.c_str(), white);
        SDL_Texture* passwordTex = SDL_CreateTextureFromSurface(renderer, passwordSurf);
        SDL_Rect passwordRect = { popupRect.x + 10, popupRect.y + 90, passwordSurf->w, passwordSurf->h };
        SDL_RenderCopy(renderer, passwordTex, nullptr, &passwordRect);
        SDL_FreeSurface(passwordSurf);
        SDL_DestroyTexture(passwordTex);

        // Delete button
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &deleteBtnRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &deleteBtnRect);

        SDL_Surface* delTextSurf = TTF_RenderText_Blended(font, "Delete", white);
        SDL_Texture* delTextTex = SDL_CreateTextureFromSurface(renderer, delTextSurf);
        SDL_Rect delTextRect = {
            deleteBtnRect.x + (deleteBtnRect.w - delTextSurf->w) / 2,
            deleteBtnRect.y + (deleteBtnRect.h - delTextSurf->h) / 2,
            delTextSurf->w,
            delTextSurf->h
        };
        SDL_RenderCopy(renderer, delTextTex, nullptr, &delTextRect);
        SDL_FreeSurface(delTextSurf);
        SDL_DestroyTexture(delTextTex);

        // Copy button
        SDL_SetRenderDrawColor(renderer, 50, 150, 200, 255);
        SDL_RenderFillRect(renderer, &copyBtnRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &copyBtnRect);

        SDL_Surface* copyTextSurf = TTF_RenderText_Blended(font, "Copy", white);
        SDL_Texture* copyTextTex = SDL_CreateTextureFromSurface(renderer, copyTextSurf);
        SDL_Rect copyTextRect = {
            copyBtnRect.x + (copyBtnRect.w - copyTextSurf->w) / 2,
            copyBtnRect.y + (copyBtnRect.h - copyTextSurf->h) / 2,
            copyTextSurf->w,
            copyTextSurf->h
        };
        SDL_RenderCopy(renderer, copyTextTex, nullptr, &copyTextRect);
        SDL_FreeSurface(copyTextSurf);
        SDL_DestroyTexture(copyTextTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return deleted;
}


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Fixed Size Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("assets/fonts/Oswald-VariableFont_wght.ttf", 16);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return 1;
    }

    std::vector<Service> services;
    int scrollOffset = 0;
    size_t lastServiceI = 0;

    auto addService = [&]() {
        MultiInputResult result = getMultipleTextInput(renderer, font);
        if (result.submitted) {
            services.push_back({ result.serviceName, result.accountName, result.password });
        }
    };


    Service* selectedService = nullptr;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (event.type == SDL_MOUSEWHEEL) {
                if (!((lastServiceI == (services.size() - 1)) && (event.wheel.y < 0))) {
                    scrollOffset -= event.wheel.y * 20;
                }
                if (scrollOffset < 0) scrollOffset = 0;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mx = event.button.x;
                int my = event.button.y;

                SDL_Point mousePoint = { mx, my };
                SDL_Rect addBtnRect = { WINDOW_WIDTH - 160, WINDOW_HEIGHT - 70, 140, 50 };

                if (SDL_PointInRect(&mousePoint, &addBtnRect)) {
                    addService();
                } else {
                    int spacing = 10;
                    int buttonHeight = 50;
                    int buttonWidth = static_cast<int>(WINDOW_WIDTH * 0.5);
                    int xStart = static_cast<int>(WINDOW_WIDTH * 0.1);
                    int yStart = static_cast<int>(WINDOW_HEIGHT * 0.1);

                    for (size_t i = 0; i < services.size(); ++i) {
                        int y = static_cast<int>(i) * (buttonHeight + spacing) - scrollOffset + yStart;
                        SDL_Rect btnRect = { xStart, y, buttonWidth, buttonHeight };
                        if (mx >= btnRect.x && mx <= btnRect.x + btnRect.w &&
                            my >= btnRect.y && my <= btnRect.y + btnRect.h) {
                            // Show popup, delete service if requested
                            bool deleted = showServiceDetailsPopup(renderer, font, services[i]);
                            if (deleted) {
                                services.erase(services.begin() + i);
                                selectedService = nullptr;
                            }
                            break;
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
        SDL_RenderClear(renderer);

        int spacing = 10;
        int buttonHeight = 50;
        int buttonWidth = static_cast<int>(WINDOW_WIDTH * 0.5);
        int xStart = static_cast<int>(WINDOW_WIDTH * 0.1);
        int yStart = static_cast<int>(WINDOW_HEIGHT * 0.1);
        int scrollAreaHeight = WINDOW_HEIGHT - 100;

        // Draw "Services" label
        SDL_Surface* labelSurf = TTF_RenderText_Blended(font, "Services", { 255, 255, 255, 255 });
        SDL_Texture* labelTex = SDL_CreateTextureFromSurface(renderer, labelSurf);
        SDL_Rect labelRect = { xStart + buttonWidth + 20, yStart, labelSurf->w, labelSurf->h };
        SDL_RenderCopy(renderer, labelTex, nullptr, &labelRect);
        SDL_FreeSurface(labelSurf);
        SDL_DestroyTexture(labelTex);

        for (size_t i = 0; i < services.size(); ++i) {
            int y = static_cast<int>(i) * (buttonHeight + spacing) - scrollOffset + yStart;
            if (y + buttonHeight < yStart || y > scrollAreaHeight) continue;
            lastServiceI = i;

            SDL_Rect btnRect = { xStart, y, buttonWidth, buttonHeight };
            SDL_SetRenderDrawColor(renderer, 70, 130, 180, 255);
            SDL_RenderFillRect(renderer, &btnRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &btnRect);

            // Draw label text
            if (!services[i].label.empty()) {
                SDL_Surface* textSurf = TTF_RenderText_Blended(font, services[i].label.c_str(), { 255, 255, 255, 255 });
                SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
                SDL_Rect textRect = {
                    btnRect.x + (btnRect.w - textSurf->w) / 2,
                    btnRect.y + (btnRect.h - textSurf->h) / 2,
                    textSurf->w,
                    textSurf->h
                };
                SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
                SDL_FreeSurface(textSurf);
                SDL_DestroyTexture(textTex);
            }
        }

        // Draw Add Service Button
        SDL_Rect addBtnRect = { WINDOW_WIDTH - 160, WINDOW_HEIGHT - 70, 140, 50 };
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_RenderFillRect(renderer, &addBtnRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &addBtnRect);

        SDL_Surface* addTextSurf = TTF_RenderText_Blended(font, "Add Service", { 255, 255, 255, 255 });
        SDL_Texture* addTextTex = SDL_CreateTextureFromSurface(renderer, addTextSurf);
        SDL_Rect addTextRect = {
            addBtnRect.x + (addBtnRect.w - addTextSurf->w) / 2,
            addBtnRect.y + (addBtnRect.h - addTextSurf->h) / 2,
            addTextSurf->w,
            addTextSurf->h
        };
        SDL_RenderCopy(renderer, addTextTex, nullptr, &addTextRect);
        SDL_FreeSurface(addTextSurf);
        SDL_DestroyTexture(addTextTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

// Note: You need to implement or adapt your getTextInput function that also supports showing a prompt message.
// For example, add an optional prompt argument and render it similarly in the input popup.
