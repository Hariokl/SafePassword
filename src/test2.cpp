//TODO: hide all passwords, mb later add a button to reveal password
//TODO: make it so when adding new service/account with the same name, it says that it already exists and not exits adding pop up
//TODO: unite getServiceNameInput and getMultipleTextInput into one function (also structures MultiInputResult and ServiceInputResult unite into one structure)
//TODO: make it, so all buttons, heights, widths and placement is connected to WIDTH and HEIGHT of the window (there should be relativity everywhere to WIDTH and HEIGHT)
//TODO: make this more universal code by adding specified int and char types like int8
//TODO: make a searchbar for services (idk about accounts it seems just like a wasted space in an app)
//TODO: mb make it so even when not selected you can type in searchbar (so when entering the app, you can instatnly write 'github' and it will search it)
//TODO: make this available for linux (?)
//TODO: make a better visuals altogether :D
//TODO: make it so user can't leave account name and service name empty
//TODO: кнопка delete слишком вырежена, мб перенести в главный экран и сделать крестик
//TODO: make it so user can rearange services/accounts however they want
//TODO: to make a better code, mb instead of updating variables every loop, while not update them only when changes are made

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <windows.h>

namespace {
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 700;
const int MAX_CHARACTERS = 20;
const char PATH_SAVE[9] = "save.txt";

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
    Account account;
};

struct ServiceInputResult {
    bool submitted = false;
    std::string label;
};

void saveToFile(const std::vector<Service>& services, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    for (const auto& service : services) {
        for (const auto& account : service.accounts) {
            outFile << service.label << ";" << account.accountName << ";" << account.password << "\n";
        }
        if (service.accounts.size() == 0) {
            outFile << service.label << ";;";
        }
    }
    outFile.close();
}

void loadFromFile(std::vector<Service>& services, const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile) {
        std::cerr << "No existing file to load: " << filename << std::endl;
        return;
    }
    std::unordered_map<std::string, Service*> serviceMap;
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string serviceName, accountName, password;
        if (std::getline(ss, serviceName, ';') &&
            std::getline(ss, accountName, ';') &&
            std::getline(ss, password)) {

            if (serviceMap.find(serviceName) == serviceMap.end()) {
                services.emplace_back(Service{serviceName});
                serviceMap[serviceName] = &services.back();
            }
            serviceMap[serviceName]->accounts.push_back({accountName, password});
        } else if (serviceName.size() != 0) {
            if (serviceMap.find(serviceName) == serviceMap.end()) {
                services.emplace_back(Service{serviceName});
                serviceMap[serviceName] = &services.back();
            }
        }
    }
    inFile.close();
}

}

ServiceInputResult getServiceNameInput(SDL_Renderer* renderer, TTF_Font* font) {
    SDL_StartTextInput();

    std::string inputText;
    bool done = false;
    bool submitted = false;
    const char* placeholder = "Service Name";
    SDL_Event e;

    SDL_Color boxColor = { 50, 50, 50, 255 };
    SDL_Color borderColor = { 255, 255, 255, 255 };
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Color placeholderColor = { 150, 150, 150, 255 };

    // Define rectangles for input boxes stacked vertically
    SDL_Rect inputRect = { 50, 290, 300, 50 };

    while (!done && !submitted) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_TEXTINPUT) {
                if (inputText.size() < (size_t)MAX_CHARACTERS) {
                    inputText += e.text.text;
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && !inputText.empty()) {
                    inputText.pop_back();
                }
                else if (e.key.keysym.sym == SDLK_RETURN) {
                    submitted = true;
                }
                else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    done = true;
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Fill box
        SDL_SetRenderDrawColor(renderer, boxColor.r, boxColor.g, boxColor.b, boxColor.a);
        SDL_RenderFillRect(renderer, &inputRect);

        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &inputRect);
        // Render text or placeholder
        const std::string& textToRender = (inputText.empty()) ? placeholder : inputText;
        SDL_Color colorToUse = (inputText.empty()) ? placeholderColor : textColor;

        std::string displayText = textToRender;

        SDL_Surface* textSurf = TTF_RenderText_Blended(font, displayText.c_str(), colorToUse);
        SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
        SDL_Rect textRect = { inputRect.x + 5, inputRect.y + 10, textSurf->w, textSurf->h };
        SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
        SDL_FreeSurface(textSurf);
        SDL_DestroyTexture(textTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    return { submitted, inputText };
}


MultiInputResult getMultipleTextInput(SDL_Renderer* renderer, TTF_Font* font, int maxLen = 20) {
    SDL_StartTextInput();

    std::string inputs[2] = { "", "" };
    const char* placeholders[2] = {"Account Name", "Password" };
    int activeInput = 0;
    bool done = false;
    bool canceled = false;

    SDL_Event e;

    SDL_Color boxColor = { 50, 50, 50, 255 };
    SDL_Color borderColor = { 255, 255, 255, 255 };
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Color placeholderColor = { 150, 150, 150, 255 };

    // Define rectangles for input boxes stacked vertically
    SDL_Rect inputRects[2] = {
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
                    activeInput = (activeInput + 1) % 2;
                }
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw all input boxes with placeholders or input text
        for (int i = 0; i < 2; ++i) {
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
            // if (i == 1 && !inputs[i].empty()) {
            //     displayText = std::string(inputs[i].size(), '*');
            // }

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
        return { false, "", "" };
    } else {
        return { true, Account{inputs[0], inputs[1]} };
    }
}

bool DeleteConfirmationPopup(SDL_Renderer* renderer, TTF_Font* font, const std::string& message) {
    bool confirmed = false;
    bool waiting = true;

    SDL_Rect popupRect = { WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 75, 300, 150 };
    SDL_Rect yesBtn = { popupRect.x + 30, popupRect.y + 90, 100, 40 };
    SDL_Rect noBtn = { popupRect.x + 170, popupRect.y + 90, 100, 40 };

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color bgColor = { 40, 40, 40, 255 };

    SDL_Event e;
    while (waiting) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                waiting = false;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;

                if (mx >= yesBtn.x && mx <= yesBtn.x + yesBtn.w &&
                    my >= yesBtn.y && my <= yesBtn.y + yesBtn.h) {
                    confirmed = true;
                    waiting = false;
                }

                if (mx >= noBtn.x && mx <= noBtn.x + noBtn.w &&
                    my >= noBtn.y && my <= noBtn.y + noBtn.h) {
                    confirmed = false;
                    waiting = false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(renderer, nullptr);

        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, 255);
        SDL_RenderFillRect(renderer, &popupRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &popupRect);

        SDL_Surface* msgSurf = TTF_RenderText_Blended_Wrapped(font, message.c_str(), white, popupRect.w - 20);
        SDL_Texture* msgTex = SDL_CreateTextureFromSurface(renderer, msgSurf);
        SDL_Rect msgRect = {
            popupRect.x + (popupRect.w - msgSurf->w) / 2,
            popupRect.y + 20,
            msgSurf->w,
            msgSurf->h
        };
        SDL_RenderCopy(renderer, msgTex, nullptr, &msgRect);
        SDL_FreeSurface(msgSurf);
        SDL_DestroyTexture(msgTex);

        // Yes Button
        SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
        SDL_RenderFillRect(renderer, &yesBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &yesBtn);

        SDL_Surface* yesSurf = TTF_RenderText_Blended(font, "Yes", white);
        SDL_Texture* yesTex = SDL_CreateTextureFromSurface(renderer, yesSurf);
        SDL_Rect yesRect = {
            yesBtn.x + (yesBtn.w - yesSurf->w) / 2,
            yesBtn.y + (yesBtn.h - yesSurf->h) / 2,
            yesSurf->w,
            yesSurf->h
        };
        SDL_RenderCopy(renderer, yesTex, nullptr, &yesRect);
        SDL_FreeSurface(yesSurf);
        SDL_DestroyTexture(yesTex);

        // No Button
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &noBtn);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &noBtn);

        SDL_Surface* noSurf = TTF_RenderText_Blended(font, "No", white);
        SDL_Texture* noTex = SDL_CreateTextureFromSurface(renderer, noSurf);
        SDL_Rect noRect = {
            noBtn.x + (noBtn.w - noSurf->w) / 2,
            noBtn.y + (noBtn.h - noSurf->h) / 2,
            noSurf->w,
            noSurf->h
        };
        SDL_RenderCopy(renderer, noTex, nullptr, &noRect);
        SDL_FreeSurface(noSurf);
        SDL_DestroyTexture(noTex);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    return confirmed;
}

bool ServiceDetailsPopup(SDL_Renderer* renderer, TTF_Font* font, Service& service) {
    bool done = false;
    int scrollOffset = 0;
    const int blockHeight = 120;
    const int blockX = 50;
    const int blockWidth = 300;
    bool deleteService = false;
    const Uint8 colorBackground[4] = {0, 0, 0, 255};
    int paddingY = 20;
    int paddingX = 50;
    int btnHeight = 50;
    int btnWidth = 300;

    const int scrollbarWidth = 15;
    bool draggingScrollbar = false;
    int dragOffsetY = 0;

    SDL_Rect addAccountBtn = { paddingX, WINDOW_HEIGHT - paddingY - btnHeight, btnWidth, btnHeight };
    SDL_Rect deleteServiceBtn = { addAccountBtn.x, addAccountBtn.y - addAccountBtn.h - paddingY, addAccountBtn.w, addAccountBtn.h };
    SDL_Event e;
    SDL_Color white = { 255, 255, 255, 255 };

    while (!done) {
        int spacing = 20;
        int xStart = static_cast<int>(WINDOW_WIDTH * 0.1); //
        int yStart = static_cast<int>(WINDOW_HEIGHT * 0.1); //
        int yScrollArea = static_cast<int>(WINDOW_HEIGHT * 0.8); //

        SDL_Rect scrollbarTrack = {
            blockX + blockWidth + 10,  // To the right of account blocks
            yStart,
            scrollbarWidth,
            yScrollArea - yStart
        };
        int contentHeight = static_cast<int>(service.accounts.size()) * (blockHeight + spacing);
        int maxScroll = max(0, contentHeight - (yScrollArea - yStart));
        int thumbHeight = 0;
        if (contentHeight > 0) {
            thumbHeight = max(30, (yScrollArea - yStart) * (yScrollArea - yStart) / contentHeight);
        }
        int thumbY = scrollbarTrack.y + (scrollOffset * (scrollbarTrack.h - thumbHeight)) / max(1, maxScroll);
        SDL_Rect scrollbarThumb = {
            scrollbarTrack.x,
            thumbY,
            scrollbarTrack.w,
            thumbHeight
        };

        SDL_Point mousePoint;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                done = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                done = true;
            }
            else if (e.type == SDL_MOUSEWHEEL && !service.accounts.empty()) {
                scrollOffset -= e.wheel.y * 20;
                scrollOffset = max(0, min(scrollOffset, maxScroll));
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                mousePoint = { mx, my };

                // Start dragging scrollbar
                if (SDL_PointInRect(&mousePoint, &scrollbarThumb)) {
                    draggingScrollbar = true;
                    dragOffsetY = my - scrollbarThumb.y;
                }

                // Add account button
                if (mx >= addAccountBtn.x && mx <= addAccountBtn.x + addAccountBtn.w &&
                    my >= addAccountBtn.y && my <= addAccountBtn.y + addAccountBtn.h) {
                    MultiInputResult result = getMultipleTextInput(renderer, font, 20);
                    if (result.submitted) {
                        service.accounts.push_back(result.account);
                    }
                }

                // Delete service button
                if (mx >= deleteServiceBtn.x && mx <= deleteServiceBtn.x + deleteServiceBtn.w &&
                    my >= deleteServiceBtn.y && my <= deleteServiceBtn.y + deleteServiceBtn.h) {
                    if (DeleteConfirmationPopup(renderer, font, "Are you sure you want to delete this service?")) {
                        deleteService = true;
                        done = true;
                    }
                }

                // Account delete/copy buttons
                for (size_t i = 0; i < service.accounts.size(); ++i) {
                    int y = 80 + static_cast<int>(i) * (blockHeight + spacing) - scrollOffset;
                    SDL_Rect deleteBtn = { 80, y + 60, 80, 30 };
                    SDL_Rect copyBtn = { 200, y + 60, 80, 30 };

                    if (mx >= deleteBtn.x && mx <= deleteBtn.x + deleteBtn.w &&
                        my >= deleteBtn.y && my <= deleteBtn.y + deleteBtn.h) {
                        if (DeleteConfirmationPopup(renderer, font, "Are you sure you want to delete this account?")) {
                            service.accounts.erase(service.accounts.begin() + i);
                        }
                        break;
                    }

                    if (mx >= copyBtn.x && mx <= copyBtn.x + copyBtn.w &&
                        my >= copyBtn.y && my <= copyBtn.y + copyBtn.h) {
                        SDL_SetClipboardText(service.accounts[i].password.c_str());
                        break;
                    }
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                draggingScrollbar = false;
            }
            else if (e.type == SDL_MOUSEMOTION && draggingScrollbar) {
                int my = e.motion.y;
                int newThumbY = my - dragOffsetY;
                newThumbY = max(scrollbarTrack.y, min(scrollbarTrack.y + scrollbarTrack.h - scrollbarThumb.h, newThumbY));

                float scrollRatio = float(newThumbY - scrollbarTrack.y) / float(scrollbarTrack.h - scrollbarThumb.h);
                scrollOffset = int(scrollRatio * maxScroll);
            }
        }
        // Background
        SDL_SetRenderDrawColor(renderer, colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
        SDL_RenderFillRect(renderer, nullptr);

        if (!service.accounts.empty()) {
            for (size_t i = 0; i < service.accounts.size(); ++i) {
                int y = yStart + static_cast<int>(i) * (blockHeight + spacing) - scrollOffset;
                if (y + blockHeight < 0 || y > yScrollArea) continue;

                SDL_Rect blockRect = { blockX, y, blockWidth, blockHeight };
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

                // Draw rectangles to keep it beautifful :)
                SDL_Rect topEdge = { 0, 0, WINDOW_WIDTH, yStart};
                SDL_Rect bottomEdge = { 0, yScrollArea, WINDOW_WIDTH, WINDOW_HEIGHT - yScrollArea};
                // SDL_Rect leftEdge = { 0, 0, blockX, WINDOW_HEIGHT};
                // SDL_Rect rightEdge = { blockX + blockWidth, 0, blockX, WINDOW_HEIGHT };
                SDL_Rect btnsRect = {blockX, yStart, blockWidth, yScrollArea - yStart};
                SDL_SetRenderDrawColor(renderer, colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
                SDL_RenderFillRect(renderer, &topEdge);
                SDL_RenderFillRect(renderer, &bottomEdge);
                // SDL_RenderFillRect(renderer, &leftEdge);
                // SDL_RenderFillRect(renderer, &rightEdge);
                // int yLast = yStart + static_cast<int>(service.accounts.size()) * (blockHeight + spacing) - scrollOffset;
                // if (yLast >= yScrollArea) {
                //     SDL_SetRenderDrawColor(renderer,  colorBackground[0] + 50, colorBackground[1] + 50, colorBackground[2] + 50, colorBackground[3]);
                //     SDL_RenderDrawRect(renderer, &btnsRect);
                // }
                if (contentHeight > (yScrollArea - yStart)) {
                    // Scrollbar background
                    SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
                    SDL_RenderFillRect(renderer, &scrollbarTrack);

                    // Scrollbar thumb
                    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
                    SDL_RenderFillRect(renderer, &scrollbarThumb);
                }
            }
        } else {
            // Delete Service button (no accounts case)
            SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
            SDL_RenderFillRect(renderer, &deleteServiceBtn);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &deleteServiceBtn);

            SDL_Surface* delServText = TTF_RenderText_Blended(font, "Delete Service", white);
            SDL_Texture* delServTex = SDL_CreateTextureFromSurface(renderer, delServText);
            SDL_Rect delServRect = {
                deleteServiceBtn.x + (deleteServiceBtn.w - delServText->w) / 2,
                deleteServiceBtn.y + (deleteServiceBtn.h - delServText->h) / 2,
                delServText->w,
                delServText->h
            };
            SDL_RenderCopy(renderer, delServTex, nullptr, &delServRect);
            SDL_FreeSurface(delServText);
            SDL_DestroyTexture(delServTex);
        }

        // Title
        std::string title = "Service: " + service.label;
        SDL_Surface* titleSurf = TTF_RenderText_Blended(font, title.c_str(), white);
        SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer, titleSurf);
        SDL_Rect titleRect = { 60, 30, titleSurf->w, titleSurf->h };
        SDL_RenderCopy(renderer, titleTex, nullptr, &titleRect);
        SDL_FreeSurface(titleSurf);
        SDL_DestroyTexture(titleTex);

        // Add Account button
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

    return deleteService;
}

// used to be main(), but since I decided to use windows.h to remove console, so it needed to be changed
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    const Uint8 colorBackground[4] = {25, 25, 25, 255};

    SDL_Window* window = SDL_CreateWindow("Fixed Size Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("assets/fonts/Oswald-VariableFont_wght.ttf", 16);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return 1;
    }
    std::vector<Service> services;
    loadFromFile(services, PATH_SAVE);

    auto addService = [&]() {
        ServiceInputResult result = getServiceNameInput(renderer, font);
        if (result.submitted) {
            Service newService;
            newService.label = result.label;
            services.push_back(newService);
        }
    };

    Service* selectedService = nullptr;
    int scrollOffset = 0;
    const int scrollbarWidth = 15;
    bool draggingScrollbar = false;
    int dragOffsetY = 0;
    int scrollbarThumbOffsetY = 0;
    size_t lastServiceI = 0;
    bool running = true;
    SDL_Event event;
    std::string searchQuery;
    bool searchActive = false;
    int accessableButtons = 0;

    SDL_StartTextInput();
    while (running) {
        //TODO: remove from loop all constants
        int xStart = static_cast<int>(WINDOW_WIDTH * 0.15); //
        int yStart = static_cast<int>(WINDOW_HEIGHT * 0.2); //
        int yScrollArea = static_cast<int>(WINDOW_HEIGHT * 0.7); //
        int buttonWidth = WINDOW_WIDTH - 2 * xStart; //
        int buttonHeight = 50;
        int spacing = 10;
        SDL_Rect addBtnRect = { WINDOW_WIDTH - 160, WINDOW_HEIGHT - 70, 140, 50 };
        SDL_Rect searchBarRect = { xStart, yStart - 50, buttonWidth, 40 };

        SDL_Rect scrollbarTrack = {
            xStart + buttonWidth + 10,  // To the right of account blocks
            yStart,
            scrollbarWidth,
            yScrollArea - yStart
        };
        int contentHeight = static_cast<int>(accessableButtons) * (buttonHeight + spacing);
        int maxScroll = max(0, contentHeight - (yScrollArea - yStart));
        int thumbHeight = 0;
        if (contentHeight > 0) {
            thumbHeight = max(30, (yScrollArea - yStart) * (yScrollArea - yStart) / contentHeight);
        }
        int thumbY = scrollbarTrack.y + (scrollOffset * (scrollbarTrack.h - thumbHeight)) / max(1, maxScroll);
        SDL_Rect scrollbarThumb = {
            scrollbarTrack.x,
            thumbY,
            scrollbarTrack.w,
            thumbHeight
        };

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            else if ((event.type == SDL_MOUSEWHEEL) && (!services.empty())) {
                scrollOffset -= event.wheel.y * 20;
                int yLast = static_cast<int>(services.size()) * (buttonHeight + spacing) - scrollOffset + yStart;
                if (yLast < yScrollArea) scrollOffset = scrollOffset + yLast - yScrollArea;
                if (scrollOffset < 0) scrollOffset = 0;
            }

            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;
                SDL_Point mousePoint = { mx, my };

                if (SDL_PointInRect(&mousePoint, &searchBarRect)) {
                    searchActive = true;
                } else {
                    searchActive = false;
                }

                // Start dragging scrollbar
                if (SDL_PointInRect(&mousePoint, &scrollbarThumb)) {
                    draggingScrollbar = true;
                    dragOffsetY = my - scrollbarThumb.y;
                } else if (SDL_PointInRect(&mousePoint, &addBtnRect)) {
                    addService();
                } else {
                    for (size_t i = 0; i < services.size(); ++i) {
                        int y = static_cast<int>(i) * (buttonHeight + spacing) - scrollOffset + yStart;
                        SDL_Rect btnRect = { xStart, y, buttonWidth, buttonHeight };
                        if (mx >= btnRect.x && mx <= btnRect.x + btnRect.w &&
                            my >= btnRect.y && my <= btnRect.y + btnRect.h) {
                            // Show popup, delete service if requested
                            bool deleted = ServiceDetailsPopup(renderer, font, services[i]);
                            if (deleted) {
                                services.erase(services.begin() + i);
                                selectedService = nullptr;
                            }
                            break;
                        }
                    }
                }

            }

            else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                draggingScrollbar = false;
            }

            else if (event.type == SDL_MOUSEMOTION && draggingScrollbar) {
                int my = event.motion.y;
                int newThumbY = my - dragOffsetY;
                newThumbY = max(scrollbarTrack.y, min(scrollbarTrack.y + scrollbarTrack.h - scrollbarThumb.h, newThumbY));

                float scrollRatio = float(newThumbY - scrollbarTrack.y) / float(scrollbarTrack.h - scrollbarThumb.h);
                scrollOffset = int(scrollRatio * maxScroll);
            }

            if (event.type == SDL_TEXTINPUT && searchActive) {
                searchQuery += event.text.text;
            } else if (event.type == SDL_KEYDOWN && searchActive) {
                if (event.key.keysym.sym == SDLK_BACKSPACE && !searchQuery.empty()) {
                    searchQuery.pop_back();
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    searchActive = false;
                    searchQuery.clear();
                }
            }
        }



        SDL_SetRenderDrawColor(renderer, colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
        SDL_RenderClear(renderer);


        // Draw service buttons
        int visibleIndex = 0;
        accessableButtons = 0;
        for (size_t i = 0; i < services.size(); ++i) {
            if (!searchQuery.empty() && services[i].label.find(searchQuery) == std::string::npos) continue;
            ++visibleIndex;
            ++accessableButtons;

            int y = visibleIndex * (buttonHeight + spacing) - scrollOffset + yStart;
            if (y + buttonHeight < yStart || y > yStart + (yScrollArea - yStart)) continue;
            lastServiceI = i;
            SDL_Rect btnRect = { xStart, y, buttonWidth, buttonHeight };

            SDL_SetRenderDrawColor(renderer, 70, 130, 180, 255);
            SDL_RenderFillRect(renderer, &btnRect);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &btnRect);

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


        // Draw rectangles to keep it beautifful :)
        SDL_Rect topEdge = { 0, 0, WINDOW_WIDTH, yStart};
        SDL_Rect bottomEdge = { 0, yScrollArea, WINDOW_WIDTH, WINDOW_HEIGHT - yScrollArea};
        // SDL_Rect leftEdge = { 0, 0, xStart, WINDOW_HEIGHT};
        // SDL_Rect rightEdge = { xStart + buttonWidth, 0, xStart, WINDOW_HEIGHT };
        SDL_Rect btnsRect = {xStart, yStart, buttonWidth, yScrollArea - yStart};
        SDL_SetRenderDrawColor(renderer, colorBackground[0], colorBackground[1], colorBackground[2], colorBackground[3]);
        SDL_RenderFillRect(renderer, &topEdge);
        SDL_RenderFillRect(renderer, &bottomEdge);
        // SDL_RenderFillRect(renderer, &leftEdge);
        // SDL_RenderFillRect(renderer, &rightEdge);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &btnsRect);

        // Draw scrollbar if needed
        if (contentHeight > (yScrollArea - yStart)) {
            // Scrollbar background
            SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
            SDL_RenderFillRect(renderer, &scrollbarTrack);

            // Scrollbar thumb
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
            SDL_RenderFillRect(renderer, &scrollbarThumb);
        }

        //search bar
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &searchBarRect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &searchBarRect);

        // Render search text
        std::string displayText = searchQuery.empty() ? "Search..." : searchQuery;
        SDL_Color textColor = { 255, 255, 255, 255 };
        SDL_Surface* searchSurf = TTF_RenderText_Blended(font, displayText.c_str(), textColor);
        SDL_Texture* searchTex = SDL_CreateTextureFromSurface(renderer, searchSurf);
        SDL_Rect textRect = {
            searchBarRect.x + 10,
            searchBarRect.y + (searchBarRect.h - searchSurf->h) / 2,
            searchSurf->w,
            searchSurf->h
        };
        SDL_RenderCopy(renderer, searchTex, nullptr, &textRect);
        SDL_FreeSurface(searchSurf);
        SDL_DestroyTexture(searchTex);


        // Draw "Services" label
        SDL_Surface* labelSurf = TTF_RenderText_Blended(font, "Services", { 255, 255, 255, 255 });
        SDL_Texture* labelTex = SDL_CreateTextureFromSurface(renderer, labelSurf);
        SDL_Rect labelRect = { xStart + buttonWidth + 20, yStart, labelSurf->w, labelSurf->h };
        SDL_RenderCopy(renderer, labelTex, nullptr, &labelRect);
        SDL_FreeSurface(labelSurf);
        SDL_DestroyTexture(labelTex);

        // Draw Add Service Button
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

    SDL_StopTextInput();

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    saveToFile(services, PATH_SAVE);
    return 0;
}
