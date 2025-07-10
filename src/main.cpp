#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <iostream>

const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 700;

struct Account {
    std::string accountName;
    std::string password;

};

struct Service {
    std::string label;
    std::vector<Account> accounts;

};

struct MultiInputResult {
    bool submitted;
    std::string serviceName;
    Account account;
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
        return { true, inputs[0], Account{inputs[1], inputs[2]} };
    }
}

bool showServiceDetailsPopup(SDL_Renderer* renderer, TTF_Font* font, Service& service) {
    bool done = false;
    int scrollOffset = 0;
    const int blockHeight = 120;
    const int spacing = 10;

    SDL_Event e;
    SDL_Color white = { 255, 255, 255, 255 };

    while (!done) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                done = true;
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                scrollOffset -= e.wheel.y * 20;
                if (scrollOffset < 0) scrollOffset = 0;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                // Check Delete and Copy buttons for each account
                for (size_t i = 0; i < service.accounts.size(); ++i) {
                    int y = 80 + static_cast<int>(i) * (blockHeight + spacing) - scrollOffset;
                    SDL_Rect deleteBtn = { 80, y + 60, 80, 30 };
                    SDL_Rect copyBtn = { 200, y + 60, 80, 30 };

                    if (mx >= deleteBtn.x && mx <= deleteBtn.x + deleteBtn.w &&
                        my >= deleteBtn.y && my <= deleteBtn.y + deleteBtn.h) {
                        service.accounts.erase(service.accounts.begin() + i);
                        break; // break to avoid invalid vector access after erase
                    }

                    if (mx >= copyBtn.x && mx <= copyBtn.x + copyBtn.w &&
                        my >= copyBtn.y && my <= copyBtn.y + copyBtn.h) {
                        SDL_SetClipboardText(service.accounts[i].password.c_str());
                    }
                }

                // Add Account button position
                int addBtnY = 80 + static_cast<int>(service.accounts.size()) * (blockHeight + spacing) - scrollOffset;
                SDL_Rect addAccountBtn = { 50, addBtnY, 300, 50 };

                if (mx >= addAccountBtn.x && mx <= addAccountBtn.x + addAccountBtn.w &&
                    my >= addAccountBtn.y && my <= addAccountBtn.y + addAccountBtn.h) {
                    // Open multi input popup
                    MultiInputResult result = getMultipleTextInput(renderer, font, 20);
                    if (result.submitted) {
                        // Add the new account to the service
                        service.accounts.push_back(result.account);
                    }
                }
            }
        }

        // Draw background
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, nullptr);

        // Draw title
        std::string title = "Service: " + service.label;
        SDL_Surface* titleSurf = TTF_RenderText_Blended(font, title.c_str(), white);
        SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
        SDL_Rect titleRect = { 60, 30, titleSurf->w, titleSurf->h };
        SDL_RenderCopy(renderer, titleTex, nullptr, &titleRect);
        SDL_FreeSurface(titleSurf);
        SDL_DestroyTexture(titleTex);

        // Draw each account block
        for (size_t i = 0; i < service.accounts.size(); ++i) {
            int y = 80 + static_cast<int>(i) * (blockHeight + spacing) - scrollOffset;
            if (y + blockHeight < 0 || y > 700) continue; // Skip off-screen

            SDL_Rect blockRect = { 50, y, 300, blockHeight };
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &blockRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &blockRect);

            std::string accountStr = "Account: " + service.accounts[i].accountName;
            std::string passwordStr = "Password: " + service.accounts[i].password;

            SDL_Surface* accSurf = TTF_RenderText_Blended(font, accountStr.c_str(), white);
            SDL_Texture* accTex = SDL_CreateTextureFromSurface(renderer, accSurf);
            SDL_Rect accRect = { blockRect.x + 10, blockRect.y + 10, accSurf->w, accSurf->h };
            SDL_RenderCopy(renderer, accTex, nullptr, &accRect);
            SDL_FreeSurface(accSurf);
            SDL_DestroyTexture(accTex);

            SDL_Surface* passSurf = TTF_RenderText_Blended(font, passwordStr.c_str(), white);
            SDL_Texture* passTex = SDL_CreateTextureFromSurface(renderer, passSurf);
            SDL_Rect passRect = { blockRect.x + 10, blockRect.y + 35, passSurf->w, passSurf->h };
            SDL_RenderCopy(renderer, passTex, nullptr, &passRect);
            SDL_FreeSurface(passSurf);
            SDL_DestroyTexture(passTex);

            // Draw Delete button
            SDL_Rect deleteBtn = { blockRect.x + 30, blockRect.y + 70, 80, 30 };
            SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
            SDL_RenderFillRect(renderer, &deleteBtn);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &deleteBtn);

            SDL_Surface* delSurf = TTF_RenderText_Blended(font, "Delete", white);
            SDL_Texture* delTex = SDL_CreateTextureFromSurface(renderer, delSurf);
            SDL_Rect delRect = {
                deleteBtn.x + (deleteBtn.w - delSurf->w) / 2,
                deleteBtn.y + (deleteBtn.h - delSurf->h) / 2,
                delSurf->w,
                delSurf->h
            };
            SDL_RenderCopy(renderer, delTex, nullptr, &delRect);
            SDL_FreeSurface(delSurf);
            SDL_DestroyTexture(delTex);

            // Draw Copy button
            SDL_Rect copyBtn = { blockRect.x + 150, blockRect.y + 70, 80, 30 };
            SDL_SetRenderDrawColor(renderer, 50, 150, 200, 255);
            SDL_RenderFillRect(renderer, &copyBtn);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &copyBtn);

            SDL_Surface* copySurf = TTF_RenderText_Blended(font, "Copy", white);
            SDL_Texture* copyTex = SDL_CreateTextureFromSurface(renderer, copySurf);
            SDL_Rect copyRect = {
                copyBtn.x + (copyBtn.w - copySurf->w) / 2,
                copyBtn.y + (copyBtn.h - copySurf->h) / 2,
                copySurf->w,
                copySurf->h
            };
            SDL_RenderCopy(renderer, copyTex, nullptr, &copyRect);
            SDL_FreeSurface(copySurf);
            SDL_DestroyTexture(copyTex);
        }

        // Draw Add Account button at bottom of list
        int addBtnY = 80 + static_cast<int>(service.accounts.size()) * (blockHeight + spacing) - scrollOffset;
        SDL_Rect addAccountBtn = { 50, addBtnY, 300, 50 };
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_RenderFillRect(renderer, &addAccountBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &addAccountBtn);

        SDL_Surface* addTextSurf = TTF_RenderText_Blended(font, "Add Account", white);
        SDL_Texture* addTextTex = SDL_CreateTextureFromSurface(renderer, addTextSurf);
        SDL_Rect addTextRect = {
            addAccountBtn.x + (addAccountBtn.w - addTextSurf->w) / 2,
            addAccountBtn.y + (addAccountBtn.h - addTextSurf->h) / 2,
            addTextSurf->w,
            addTextSurf->h
        };
        SDL_RenderCopy(renderer, addTextTex, nullptr, &addTextRect);
        SDL_FreeSurface(addTextSurf);
        SDL_DestroyTexture(addTextTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return true;
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
            services.push_back(Service{ result.serviceName, std::vector<Account>{result.account}});
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
